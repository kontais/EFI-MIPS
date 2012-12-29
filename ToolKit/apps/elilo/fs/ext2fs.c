/*
 *  Copyright (C) 2001-2003 Hewlett-Packard Co.
 *	Contributed by Stephane Eranian <eranian@hpl.hp.com>
 *
 * This file is part of the ELILO, the EFI Linux boot loader.
 *
 *  ELILO is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  ELILO is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with ELILO; see the file COPYING.  If not, write to the Free
 *  Software Foundation, 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 * Please check out the elilo.txt for complete documentation on how
 * to use this program.
 *
 * The ext2 code in this file is derived from aboot-0.7 (the Linux/Alpha
 * bootloader) and credits are attributed to:
 *
 * This file has been ported from the DEC 32-bit Linux version
 * by David Mosberger (davidm@cs.arizona.edu).
 */

#include <efi.h>
#include <efilib.h>

#include "elilo.h"

#define FS_NAME L"ext2fs"

#ifdef PATH_MAX
#error You must have included some Linux header file by error
#endif

#define PATH_MAX		4095
#define EXT2FS_PATH_MAXLEN	PATH_MAX

#include "fs/ext2fs.h"
#include "fs/ext2_private.h"

/*
 * should we decide to spin off ext2, let's say to a boottime driver
 * we would have to change this
 */
#define EXT2FS_MEMTYPE	EfiLoaderData

/* ext2 file size is __u32 */
#define EXT2_FILESIZE_MAX	(0x100000000UL)

/* 
 * Number of simultaneous open files. This needs to be high because
 * directories are kept open while traversing long path names.
 */
#define MAX_OPEN_FILES		32 

typedef struct inode_table_entry {
	struct	ext2_inode	inode;
	int			inumber;
	int			free;
	unsigned short		old_mode;

	UINT32			pos;	/* current position in file ext2 uses __u32 !*/
} inode_entry_t;


typedef struct {
	struct ext2_super_block sb;
	struct ext2_group_desc *gds;
	struct ext2_inode *root_inode;
	int ngroups;
	int directlim;			/* Maximum direct blkno */
	int ind1lim;			/* Maximum single-indir blkno */
	int ind2lim;			/* Maximum double-indir blkno */
	int ptrs_per_blk;		/* ptrs/indirect block */
	CHAR8 blkbuf[EXT2_MAX_BLOCK_SIZE];
	int cached_iblkno;
	CHAR8 iblkbuf[EXT2_MAX_BLOCK_SIZE];
	int cached_diblkno;
	CHAR8 diblkbuf[EXT2_MAX_BLOCK_SIZE];
	long blocksize;


	inode_entry_t inode_table[MAX_OPEN_FILES];

	/* fields added to fit the protocol construct */
	EFI_BLOCK_IO *blkio;
	UINT32	     mediaid;
	EFI_HANDLE   dev;
} ext2fs_priv_state_t;


typedef union {
	ext2fs_interface_t pub_intf;
	struct {
		ext2fs_interface_t  pub_intf;
		ext2fs_priv_state_t priv_data;
	} ext2fs_priv;
} ext2fs_t;

#define FS_PRIVATE(n)	(&(((ext2fs_t *)n)->ext2fs_priv.priv_data))

typedef union {
	EFI_HANDLE *dev;
	ext2fs_t  *intf;
} dev_tab_t;

static dev_tab_t *dev_tab;	/* holds all devices we found */
static UINTN ndev;		/* how many entries in dev_tab */


static EFI_GUID Ext2FsProtocol = EXT2FS_PROTOCOL;

static INTN
read_bytes(EFI_BLOCK_IO *blkio, UINT32 mediaid, UINTN offset, VOID *addr, UINTN size)
{
	EFI_STATUS status;
	UINT8 *buffer;
	UINTN count, buffer_size;
	EFI_LBA base;
	INTN ret  = EFI_INVALID_PARAMETER;

	base        = offset / blkio->Media->BlockSize;
	count       = (size + blkio->Media->BlockSize -1) / blkio->Media->BlockSize;
	buffer_size = count * blkio->Media->BlockSize;

	DBG_PRT((L"readblock(%x, %d,%d) base=%d count=%d", blkio, offset, size, base, count));

	/*
	 * slow but avoid large buffer on the stack
	 */
	buffer = (UINT8 *)alloc(buffer_size, EfiLoaderData);
	if (buffer == NULL) {
		ERR_PRT((L"cannot allocate ext2fs buffer size=%d", buffer_size));
		return ret;
	}

	DBG_PRT((L"readblock(PTR_FMT ", %d, %ld, %d, " PTR_FMT ")", blkio, mediaid, base, buffer_size, buffer));

	status = blkio->ReadBlocks (blkio, mediaid, base, buffer_size, buffer); 
	if (EFI_ERROR(status)) {
		ERR_PRT((L"readblock(%d,%d)=%r", base, buffer_size, status));
		goto error;
	}

	DBG_PRT((L"readblock(%d,%d)->%r", offset, buffer_size, status));

	Memcpy(addr, buffer+(offset-(base*blkio->Media->BlockSize)), size);

	ret = 0;

error:
	free(buffer);

	return ret;
}

/*
 * Read the specified inode from the disk and return it to the user.
 * Returns NULL if the inode can't be read...
 */
static struct ext2_inode *
ext2_iget(ext2fs_priv_state_t *e2fs, int ino)
{
	int i;
	struct ext2_inode *ip;
	struct inode_table_entry *itp = 0;
	int group;
	long offset;

	ip = 0;
	for (i = 0; i < MAX_OPEN_FILES; i++) {
		DBG_PRT((L"ext2_iget: looping, entry %d inode %d free %d",
		       i, e2fs->inode_table[i].inumber, e2fs->inode_table[i].free));
		if (e2fs->inode_table[i].free) {
			itp = &e2fs->inode_table[i];
			ip = &itp->inode;
			break;
		}
	}
	if (!ip) {
		ERR_PRT((L"ext2_iget: no free inodes"));
		return NULL;
	}


	group = (ino-1) / e2fs->sb.s_inodes_per_group;

	DBG_PRT((L" itp-inode_table=%d bg_inode_table=%d  group=%d ino=%d\n", (UINTN)(itp-e2fs->inode_table),
			(UINTN)(e2fs->gds[group].bg_inode_table), (UINTN)group, (UINTN)ino));

	offset = ((long) e2fs->gds[group].bg_inode_table * e2fs->blocksize)
		+ (((ino - 1) % EXT2_INODES_PER_GROUP(&e2fs->sb)) * EXT2_INODE_SIZE(&e2fs->sb));
	
	DBG_PRT((L"ext2_iget: reading %d bytes at offset %d"
	       " ((%d * %d) + ((%d) %% %d) * %d) "
	       "(inode %d -> table %d)", 
	       sizeof(struct ext2_inode), 
	       (UINTN)offset,
	       (UINTN)e2fs->gds[group].bg_inode_table, (UINTN)e2fs->blocksize,
	       (UINTN)(ino - 1), (UINTN)EXT2_INODES_PER_GROUP(&e2fs->sb), EXT2_INODE_SIZE(&e2fs->sb),
	       (UINTN)ino, (UINTN) (itp - e2fs->inode_table)));

	if (read_bytes(e2fs->blkio, e2fs->mediaid, offset, ip, sizeof(struct ext2_inode))) {
		ERR_PRT((L"ext2_iget: read error"));
		return NULL;
	}
	
	DBG_PRT((L"mode=%x uid=%d size=%d gid=%d links=%d flags=%d",
					(UINTN)ip->i_mode,
					(UINTN)ip->i_uid,
					(UINTN)ip->i_size,
					(UINTN)ip->i_gid,
					(UINTN)ip->i_flags));

	itp->free = 0;
	itp->inumber = ino;
	itp->old_mode = ip->i_mode;

	return ip;
}


/*
 * Release our hold on an inode.  Since this is a read-only application,
 * don't worry about putting back any changes...
 */
static void 
ext2_iput(ext2fs_priv_state_t *e2fs, struct ext2_inode *ip)
{
	struct inode_table_entry *itp;

	/* Find and free the inode table slot we used... */
	itp = (struct inode_table_entry *)ip;

	DBG_PRT((L"ext2_iput: inode %d table %d", itp->inumber, (int) (itp - e2fs->inode_table)));

	itp->inumber = 0;
	itp->free = 1;
}


/*
 * Map a block offset into a file into an absolute block number.
 * (traverse the indirect blocks if necessary).  Note: Double-indirect
 * blocks allow us to map over 64Mb on a 1k file system.  Therefore, for
 * our purposes, we will NOT bother with triple indirect blocks.
 *
 * The "allocate" argument is set if we want to *allocate* a block
 * and we don't already have one allocated.
 */
static int 
ext2_blkno(ext2fs_priv_state_t *e2fs, struct ext2_inode *ip, int blkoff)
{
	unsigned int *lp;
	unsigned int *ilp;
	unsigned int *dlp;
	int blkno;
	int iblkno;
	int diblkno;
	long offset;

	ilp = (unsigned int *)e2fs->iblkbuf;
	dlp = (unsigned int *)e2fs->diblkbuf;
	lp = (unsigned int *)e2fs->blkbuf;

	/* If it's a direct block, it's easy! */
	if (blkoff <= e2fs->directlim) {
		return ip->i_block[blkoff];
	}

	/* Is it a single-indirect? */
	if (blkoff <= e2fs->ind1lim) {
		iblkno = ip->i_block[EXT2_IND_BLOCK];

		if (iblkno == 0) {
			return 0;
		}

		/* Read the indirect block */
		if (e2fs->cached_iblkno != iblkno) {
			offset = iblkno * e2fs->blocksize;
			if (read_bytes(e2fs->blkio, e2fs->mediaid, offset, e2fs->iblkbuf, e2fs->blocksize)) {
				ERR_PRT((L"ext2_blkno: error on iblk read"));
				return 0;
			}
			e2fs->cached_iblkno = iblkno;
		}

		blkno = ilp[blkoff-(e2fs->directlim+1)];
		return blkno;
	}

	/* Is it a double-indirect? */
	if (blkoff <= e2fs->ind2lim) {
		/* Find the double-indirect block */
		diblkno = ip->i_block[EXT2_DIND_BLOCK];

		if (diblkno == 0) {
			return 0;
		}

		/* Read in the double-indirect block */
		if (e2fs->cached_diblkno != diblkno) {
			offset = diblkno * e2fs->blocksize;
			if (read_bytes(e2fs->blkio, e2fs->mediaid, offset,  e2fs->diblkbuf, e2fs->blocksize)) {
				ERR_PRT((L"ext2_blkno: err reading dindr blk"));
				return 0;
			}
			e2fs->cached_diblkno = diblkno;
		}

		/* Find the single-indirect block pointer ... */
		iblkno = dlp[(blkoff - (e2fs->ind1lim+1)) / e2fs->ptrs_per_blk];

		if (iblkno == 0) {
			return 0;
		}

		/* Read the indirect block */
    
		if (e2fs->cached_iblkno != iblkno) {
			offset = iblkno * e2fs->blocksize;
			if (read_bytes(e2fs->blkio, e2fs->mediaid, offset, e2fs->iblkbuf, e2fs->blocksize)) {
				ERR_PRT((L"ext2_blkno: err on iblk read"));
				return 0;
			}
			e2fs->cached_iblkno = iblkno;
		}

		/* Find the block itself. */
		blkno = ilp[(blkoff-(e2fs->ind1lim+1)) % e2fs->ptrs_per_blk];
		return blkno;
	}

	if (blkoff > e2fs->ind2lim) {
		ERR_PRT((L"ext2_blkno: block number too large: %d", blkoff));
		return 0;
	}
	return -1;
}


static int
ext2_breadi(ext2fs_priv_state_t *e2fs, struct ext2_inode *ip, long blkno, long nblks, CHAR8 *buffer)
{ 
	long dev_blkno, ncontig, offset, nbytes, tot_bytes;

	tot_bytes = 0;
	if ((blkno+nblks)*e2fs->blocksize > ip->i_size)
		nblks = (ip->i_size + e2fs->blocksize) / e2fs->blocksize - blkno;

	while (nblks) {
		/*
		 * Contiguous reads are a lot faster, so we try to group
		 * as many blocks as possible:
		 */
		ncontig = 0; nbytes = 0;
		dev_blkno = ext2_blkno(e2fs,ip, blkno);
		do {
			++blkno; ++ncontig; --nblks;
			nbytes += e2fs->blocksize;
		} while (nblks &&
			 ext2_blkno(e2fs, ip, blkno) == dev_blkno + ncontig);

		if (dev_blkno == 0) {
			/* This is a "hole" */
			Memset(buffer, 0, nbytes);
		} else {
			/* Read it for real */
			offset = dev_blkno*e2fs->blocksize;
			DBG_PRT((L"ext2_bread: reading %d bytes at offset %d", nbytes, offset));

			if (read_bytes(e2fs->blkio, e2fs->mediaid, offset, buffer, nbytes)) {
				ERR_PRT((L"ext2_bread: read error"));
				return -1;
			}
		}
		buffer    += nbytes;
		tot_bytes += nbytes;
	}
	return tot_bytes;
}

static struct ext2_dir_entry_2 *
ext2_readdiri(ext2fs_priv_state_t *e2fs, struct ext2_inode *dir_inode, int rewind)
{
	struct ext2_dir_entry_2 *dp;
	static int diroffset = 0, blockoffset = 0;

	/* Reading a different directory, invalidate previous state */
	if (rewind) {
		diroffset = 0;
		blockoffset = 0;
		/* read first block */
		if (ext2_breadi(e2fs, dir_inode, 0, 1, e2fs->blkbuf) < 0)
			return NULL;
	}

	DBG_PRT((L"ext2_readdiri: blkoffset %d diroffset %d len %d", blockoffset, diroffset, dir_inode->i_size));

	if (blockoffset >= e2fs->blocksize) {
		diroffset += e2fs->blocksize;
		if (diroffset >= dir_inode->i_size)
			return NULL;
		ERR_PRT((L"ext2_readdiri: reading block at %d", diroffset));
		/* assume that this will read the whole block */
		if (ext2_breadi(e2fs, dir_inode, diroffset / e2fs->blocksize, 1, e2fs->blkbuf) < 0) return NULL;
		blockoffset = 0;
	}

	dp = (struct ext2_dir_entry_2 *) (e2fs->blkbuf + blockoffset);
	blockoffset += dp->rec_len;

#if 0
	Print(L"ext2_readdiri: returning %x = ");
	{ INTN i; for(i=0; i < dp->name_len; i++) Print(L"%c", (CHAR16)dp->name[i]); Print(L"\n");}
#endif
	return dp;
}

/*
 * the string 'name' is modified by this call as per the parsing that
 * is done in strtok_simple()
 */
static struct ext2_inode *
ext2_namei(ext2fs_priv_state_t *e2fs, CHAR8 *name)
{
	CHAR8 *component;
	struct ext2_inode *dir_inode;
	struct ext2_dir_entry_2 *dp;
	int next_ino;

	/* start at the root: */
	if (!e2fs->root_inode)
		e2fs->root_inode = ext2_iget(e2fs, EXT2_ROOT_INO);
	dir_inode = e2fs->root_inode;
	if (!dir_inode)
	  return NULL;

	component = strtok_simple(name, '/');
	while (component) {
		int component_length;
		int rewind = 0;
		/*
		 * Search for the specified component in the current
		 * directory inode.
		 */
		next_ino = -1;
		component_length = strlena(component);

		DBG_PRT((L"ext2_namei: component = %a", component));

		/* rewind the first time through */
		while ((dp = ext2_readdiri(e2fs, dir_inode, !rewind++))) {
			if ((dp->name_len == component_length) &&
			    (strncmpa(component, dp->name,
				     component_length) == 0))
			{
				/* Found it! */
				DBG_PRT((L"ext2_namei: found entry %a", component));
				next_ino = dp->inode;
				break;
			}
			DBG_PRT((L"ext2_namei: looping"));
		}
	
		DBG_PRT((L"ext2_namei: next_ino = %d", next_ino));

		/*
		 * At this point, we're done with this directory whether
		 * we've succeeded or failed...
		 */
		if (dir_inode != e2fs->root_inode) ext2_iput(e2fs, dir_inode);

		/*
		 * If next_ino is negative, then we've failed (gone
		 * all the way through without finding anything)
		 */
		if (next_ino < 0) {
			return NULL;
		}

		/*
		 * Otherwise, we can get this inode and find the next
		 * component string...
		 */
		dir_inode = ext2_iget(e2fs, next_ino);
		if (!dir_inode)
		  return NULL;

		component = strtok_simple(NULL, '/');
	}

	/*
	 * If we get here, then we got through all the components.
	 * Whatever we got must match up with the last one.
	 */
	return dir_inode;
}


/*
 * Read block number "blkno" from the specified file.
 */
static int 
ext2_bread(ext2fs_priv_state_t *e2fs, int fd, long blkno, long nblks, char *buffer)
{
	struct ext2_inode * ip;
	ip = &e2fs->inode_table[fd].inode;
	return ext2_breadi(e2fs, ip, blkno, nblks, buffer);
}

#if 0
/*
 * Note: don't mix any kind of file lookup or other I/O with this or
 * you will lose horribly (as it reuses blkbuf)
 */
static const char *
ext2_readdir(ext2fs_priv_state_t *e2fs, int fd, int rewind)
{
	struct ext2_inode * ip = &e2fs->inode_table[fd].inode;
	struct ext2_dir_entry_2 * ent;
	if (!S_ISDIR(ip->i_mode)) {
		ERR_PRT((L"fd %d (inode %d) is not a directory (mode %x)",
		       fd, e2fs->inode_table[fd].inumber, ip->i_mode));
		return NULL;
	}
	ent = ext2_readdiri(e2fs, ip, rewind);
	if (ent) {
		ent->name[ent->name_len] = '\0';
		return ent->name;
	} else { 
		return NULL;
	}
}
#endif

static int 
ext2_fstat(ext2fs_priv_state_t *e2fs, int fd, ext2fs_stat_t *buf)
{
	struct ext2_inode * ip = &e2fs->inode_table[fd].inode;

	Memset(buf, 0, sizeof(*buf));

	/* fill in relevant fields */
	buf->st_ino = e2fs->inode_table[fd].inumber;
	buf->st_mode = ip->i_mode;
	buf->st_nlink = ip->i_links_count;
	buf->st_uid = ip->i_uid;
	buf->st_gid = ip->i_gid;
	buf->st_size = ip->i_size;
	buf->st_atime = ip->i_atime;
	buf->st_mtime = ip->i_mtime;
	buf->st_ctime = ip->i_ctime;

	return 0; /* NOTHING CAN GO WROGN! */
}

static EFI_STATUS
ext2fs_fstat(ext2fs_interface_t *this, UINTN fd, ext2fs_stat_t *st)
{
	ext2fs_priv_state_t *e2fs;

	if (this == NULL || fd > MAX_OPEN_FILES || st == NULL) return EFI_INVALID_PARAMETER;

	e2fs = FS_PRIVATE(this);

	ext2_fstat(e2fs, fd, st);

	return EFI_SUCCESS;
}

static EFI_STATUS
ext2fs_seek(ext2fs_interface_t *this, UINTN fd, UINT64 newpos)
{
	ext2fs_priv_state_t *e2fs;

	if (this == NULL || fd > MAX_OPEN_FILES || newpos >= EXT2_FILESIZE_MAX) return EFI_INVALID_PARAMETER;

	e2fs = FS_PRIVATE(this);
	if (newpos > (UINT64)e2fs->inode_table[fd].inode.i_size) return EFI_INVALID_PARAMETER;

	e2fs->inode_table[fd].pos = newpos;

	return EFI_SUCCESS;
}

static EFI_STATUS
ext2fs_read(ext2fs_interface_t *this, UINTN fd, VOID *buf, UINTN *size)
{
	ext2fs_priv_state_t *e2fs;
	UINTN count, nc, bofs, bnum, pos;
	EFI_STATUS ret = EFI_INVALID_PARAMETER;
	CHAR8 *block;

	if (this == NULL || size == NULL || buf == NULL || fd > MAX_OPEN_FILES) return EFI_INVALID_PARAMETER;

	e2fs = FS_PRIVATE(this);

	count = MIN(*size, e2fs->inode_table[fd].inode.i_size - e2fs->inode_table[fd].pos);

	if (count == 0)  {
		*size = 0;
		return EFI_SUCCESS;
	}
	block = e2fs->blkbuf;

	*size = 0;

	pos = e2fs->inode_table[fd].pos;  
	DBG_PRT((L"size=%d i_size=%d count=%d pos=%ld", *size,e2fs->inode_table[fd].inode.i_size, count, pos));
	while (count) {
		bnum = pos / e2fs->blocksize;
		bofs = pos % e2fs->blocksize;
		nc   = MIN(count, e2fs->blocksize - bofs);

		DBG_PRT((L"bnum =%d bofs=%d nc=%d *size=%d", bnum, bofs, nc, *size));

		if (ext2_bread(e2fs, fd, bnum, 1, block) == -1) goto error;
#if 0		
		{ int i; char *p = block+bofs; 
			for(i=MIN(nc, 64); i>=0 ; i--, p++) {
				if (i % 16 == 0) Print(L"\n");
				Print(L"%02x ", (UINTN)*p & 0xff);
			}
		}
#endif

		Memcpy(buf, block+bofs, nc);
		count -= nc;
		pos   += nc;
		buf   += nc;
		*size += nc;
	}

	e2fs->inode_table[fd].pos += *size;
	ret = EFI_SUCCESS;
error:
	DBG_PRT((L"*size=%d ret=%r", *size, ret));
	return ret;
}

static struct ext2_inode *
ext2_follow_link(ext2fs_priv_state_t *e2fs, struct ext2_inode * from, const char * base)
{
	char *linkto;

	if (from->i_blocks) {
		linkto = e2fs->blkbuf;
		if (ext2_breadi(e2fs, from, 0, 1, e2fs->blkbuf) == -1)
			return NULL;
		DBG_PRT((L"long link!"));
	} else {
		linkto = (char*)from->i_block;
	}
	DBG_PRT((L"symlink to %s", linkto));

	/* Resolve relative links */
	if (linkto[0] != '/') {
		char *end = strrchra(base, '/');
		if (end) {
			//char fullname[(end - base + 1) + strlena(linkto) + 1];
			char fullname[EXT2FS_PATH_MAXLEN];

			if (((end - base + 1) + strlena(linkto) + 1) >= EXT2FS_PATH_MAXLEN) {
				Print(L"%s: filename too long, can't resolve\n", __FUNCTION__);
				return NULL;
			}

			strncpya(fullname, base, end - base + 1);
			fullname[end - base + 1] = '\0';
			strcata(fullname, linkto);
			DBG_PRT((L"resolved to %s", fullname));
			return ext2_namei(e2fs, fullname);
		} else {
			/* Assume it's in the root */
			return ext2_namei(e2fs, linkto);
		}
	} else {
		return ext2_namei(e2fs, linkto);
	}
}

static int
ext2_open(ext2fs_priv_state_t *e2fs, char *filename)
{
	/*
	 * Unix-like open routine.  Returns a small integer (actually
	 * an index into the inode table...
	 */
	struct ext2_inode * ip;

	ip = ext2_namei(e2fs, filename);
	if (ip) {
		struct inode_table_entry *itp;

		while (S_ISLNK(ip->i_mode)) {
			ip = ext2_follow_link(e2fs, ip, filename);
			if (!ip) return -1;
		}
		itp = (struct inode_table_entry *)ip;
		return itp - e2fs->inode_table;
	} else
		return -1;
}

static void ext2_close(ext2fs_priv_state_t *e2fs, int fd)
{
	/* blah, hack, don't close the root inode ever */
	if (&e2fs->inode_table[fd].inode != e2fs->root_inode)
		ext2_iput(e2fs, &e2fs->inode_table[fd].inode);
}

static EFI_STATUS
ext2fs_close(ext2fs_interface_t *this, UINTN fd)
{
	ext2fs_priv_state_t *e2fs;

	if (this == NULL || fd > MAX_OPEN_FILES) return EFI_INVALID_PARAMETER;

	e2fs = FS_PRIVATE(this);

	ext2_close(e2fs, fd);

	return EFI_SUCCESS;
}

static EFI_STATUS
ext2fs_open(ext2fs_interface_t *this, CHAR16 *name, UINTN *fd)
{
	ext2fs_priv_state_t *e2fs;
	CHAR8 filename[EXT2FS_PATH_MAXLEN]; /* XXX: kind of big for a stack object */
	INTN tmp;

	DBG_PRT((L"name:%s fd=%x", name, fd));

	if (this == NULL || name == NULL || fd == NULL || StrLen(name) >=EXT2FS_PATH_MAXLEN) return EFI_INVALID_PARAMETER;

	e2fs = FS_PRIVATE(this);

	/*
	 * XXX: for security reasons, we may have to force a prefix like /boot to all filenames
	 */
	StrnXCpy(filename, name, EXT2FS_PATH_MAXLEN);
	
	DBG_PRT((L"ASCII name:%a UTF-name:%s", filename, name));

	tmp = ext2_open(e2fs, filename);
	if (tmp != -1) {
		*fd = (UINTN)tmp;
		e2fs->inode_table[tmp].pos = 0; /* reset file position */
	}

	DBG_PRT((L"name: %s fd=%d tmp=%d", name, *fd, tmp));

	return tmp == -1 ? EFI_NOT_FOUND : EFI_SUCCESS;
}

static EFI_STATUS
ext2fs_name(ext2fs_interface_t *this, CHAR16 *name, UINTN maxlen)
{
	if (name == NULL || maxlen < 1) return EFI_INVALID_PARAMETER;

	StrnCpy(name, FS_NAME, maxlen-1);

	name[maxlen-1] = CHAR_NULL;

	return EFI_SUCCESS;
}

static INTN
ext2fs_init_state(ext2fs_t *ext2fs, EFI_HANDLE dev, EFI_BLOCK_IO *blkio, struct ext2_super_block *sb)
{
	ext2fs_priv_state_t *e2fs = FS_PRIVATE(ext2fs);
	UINTN i;
	EFI_STATUS status;

	Memset(ext2fs, 0, sizeof(*ext2fs));

	e2fs->dev     = dev;
	e2fs->blkio   = blkio;
	e2fs->mediaid = blkio->Media->MediaId;

	/* fools gcc builtin memcpy */
	Memcpy(&e2fs->sb, sb, sizeof(*sb));

	e2fs->ngroups = (sb->s_blocks_count - sb->s_first_data_block + EXT2_BLOCKS_PER_GROUP(sb) - 1) / EXT2_BLOCKS_PER_GROUP(sb);

	e2fs->gds = (struct ext2_group_desc *)alloc(e2fs->ngroups * sizeof(struct ext2_group_desc), EXT2FS_MEMTYPE);
	if (e2fs->gds == NULL) {
		ERR_PRT((L"failed to allocate gds"));
		return EFI_OUT_OF_RESOURCES;
	}
	
	e2fs->blocksize = EXT2_BLOCK_SIZE(sb);

	DBG_PRT((L"gds_size=%d gds_offset=%d ngroups=%d blocksize=%d",
				e2fs->ngroups * sizeof(struct ext2_group_desc), 
				e2fs->blocksize * (EXT2_MIN_BLOCK_SIZE/e2fs->blocksize + 1),
				e2fs->ngroups, (UINTN)e2fs->blocksize));

	/* read in the group descriptors (immediately follows superblock) */
	status = read_bytes(blkio, e2fs->mediaid, e2fs->blocksize * (EXT2_MIN_BLOCK_SIZE/e2fs->blocksize + 1),
			e2fs->gds, e2fs->ngroups * sizeof(struct ext2_group_desc));
	if (EFI_ERROR(status)) {
		ERR_PRT((L"cannot read gds: %r", status));
		free(e2fs->gds);
		return EFI_INVALID_PARAMETER;
	}
#if 0
	{ int i; char *p = (char *)e2fs->gds;
		for(i=e2fs->ngroups*sizeof(*e2fs->gds); i ; i--, p++) {
			if (i % 16 == 0) Print(L"\n");
			Print(L"%02x ", (UINTN)*p & 0xff);

		}
	}
#endif

	e2fs->cached_diblkno = -1;
	e2fs->cached_iblkno  = -1;

	/* initialize the inode table */
	for (i = 0; i < MAX_OPEN_FILES; i++) {
		e2fs->inode_table[i].free = 1;
		e2fs->inode_table[i].inumber = 0;
	}
	/* clear the root inode pointer (very important!) */
	e2fs->root_inode = NULL;

	/*
	 * Calculate direct/indirect block limits for this file system
	 * (blocksize dependent):
	ext2_blocksize = EXT2_BLOCK_SIZE(&sb);
	 */
	e2fs->directlim    = EXT2_NDIR_BLOCKS - 1;
	e2fs->ptrs_per_blk = e2fs->blocksize/sizeof(unsigned int);
	e2fs->ind1lim      = e2fs->ptrs_per_blk + e2fs->directlim;
	e2fs->ind2lim      = (e2fs->ptrs_per_blk * e2fs->ptrs_per_blk) + e2fs->directlim;

	ext2fs->pub_intf.ext2fs_name     = ext2fs_name;
	ext2fs->pub_intf.ext2fs_open     = ext2fs_open;
	ext2fs->pub_intf.ext2fs_read     = ext2fs_read;
	ext2fs->pub_intf.ext2fs_close    = ext2fs_close;
	ext2fs->pub_intf.ext2fs_seek     = ext2fs_seek;
	ext2fs->pub_intf.ext2fs_fstat    = ext2fs_fstat;

	return EFI_SUCCESS;
}


static EFI_STATUS
ext2fs_install_one(EFI_HANDLE dev, VOID **intf)
{
	struct ext2_super_block sb;
	long sb_block = 1;
	EFI_STATUS status;
	EFI_BLOCK_IO *blkio;
	ext2fs_t *ext2fs;

	status = BS->HandleProtocol (dev, &Ext2FsProtocol, (VOID **)&ext2fs);
	if (status == EFI_SUCCESS) {
		ERR_PRT((L"Warning: found existing %s protocol on device", FS_NAME));
		goto found;
	}
	
	status = BS->HandleProtocol (dev, &BlockIoProtocol, (VOID **)&blkio);
	if (EFI_ERROR(status)) return EFI_INVALID_PARAMETER;
	
	VERB_PRT(5,
		{ EFI_DEVICE_PATH *dp; CHAR16 *str;
		  dp  = DevicePathFromHandle(dev);
		  str = DevicePathToStr(dp);
		  Print(L"dev:%s\nLogical partition: %s  BlockSize: %d WriteCaching: %s \n", str, 
			  blkio->Media->LogicalPartition ? L"Yes": L"No",
			  blkio->Media->BlockSize,
			  blkio->Media->WriteCaching ? L"Yes":L"No");
		  FreePool(str);
		});
	if (blkio->Media->LogicalPartition == FALSE) return EFI_INVALID_PARAMETER;

#if 0
	/*
	 * Used to be necessary on some older versions of EFI to avoid getting
	 * stuck. Now can cause problems with some SCSI controllers when enabled. 
	 * Does not seem necessary with EFI 12.38
	 */
	blkio->Reset(blkio, FALSE);
#endif

	status = read_bytes(blkio, blkio->Media->MediaId, sb_block * EXT2_MIN_BLOCK_SIZE, &sb, sizeof(sb));
	if (EFI_ERROR(status)) {
		DBG_PRT((L"cannot read superblock: %r", status));
		return EFI_INVALID_PARAMETER;
	}
	
	if (sb.s_magic != EXT2_SUPER_MAGIC) {
		DBG_PRT((L"bad magic "PTR_FMT"\n", sb.s_magic));
		return EFI_INVALID_PARAMETER;
	}
	
	ext2fs = (ext2fs_t *)alloc(sizeof(*ext2fs), EXT2FS_MEMTYPE);
	if (ext2fs == NULL) return EFI_OUT_OF_RESOURCES;

	status = ext2fs_init_state(ext2fs, dev, blkio, &sb);
	if (status != EFI_SUCCESS) {
		free(ext2fs);
		return status;
	}

	status = LibInstallProtocolInterfaces(&dev, &Ext2FsProtocol, ext2fs, NULL);
	if (EFI_ERROR(status)) {
		ERR_PRT((L"Cannot install %s protocol: %r", FS_NAME, status));
		free(ext2fs);
		return status;
	}
found:
	if (intf) *intf = (VOID *)ext2fs;

	VERB_PRT(3,
		{ EFI_DEVICE_PATH *dp; CHAR16 *str;
		  dp  = DevicePathFromHandle(dev);
		  str = DevicePathToStr(dp);
		  Print(L"dev:%s %s detected\n", str, FS_NAME);
		  FreePool(str);
		});

	return EFI_SUCCESS;
}

EFI_STATUS
ext2fs_install(VOID)
{
	UINTN size = 0;
	UINTN i;
	EFI_STATUS status;
	VOID *intf;

	BS->LocateHandle (ByProtocol, &BlockIoProtocol, NULL, &size, NULL);
	if (size == 0) return EFI_UNSUPPORTED; /* no device found, oh well */

	DBG_PRT((L"size=%d", size));

	dev_tab = (dev_tab_t *)alloc(size, EfiLoaderData);
	if (dev_tab == NULL) {
		ERR_PRT((L"failed to allocate handle table"));
		return EFI_OUT_OF_RESOURCES;
	}
	
	status = BS->LocateHandle (ByProtocol, &BlockIoProtocol, NULL, 
				&size, (VOID **)dev_tab);
	if (status != EFI_SUCCESS) {
		ERR_PRT((L"failed to get handles: %r", status));
		free(dev_tab);
		return status;
	}
	ndev = size / sizeof(EFI_HANDLE);

	for(i=0; i < ndev; i++) {
		intf = NULL;
		ext2fs_install_one(dev_tab[i].dev, &intf);
		/* override device handle with interface pointer */
		dev_tab[i].intf = intf;
	}

	return EFI_SUCCESS;
}
	
EFI_STATUS
ext2fs_uninstall(VOID)
{
	
	ext2fs_priv_state_t *e2fs;
	EFI_STATUS status;
	UINTN i;

	for(i=0; i < ndev; i++) {
		if (dev_tab[i].intf == NULL) continue;
		e2fs = FS_PRIVATE(dev_tab[i].intf);
		status = BS->UninstallProtocolInterface (e2fs->dev, &Ext2FsProtocol, dev_tab[i].intf);
		if (EFI_ERROR(status)) {
			ERR_PRT((L"Uninstall %s error: %r", FS_NAME, status));
			continue;
		}
		VERB_PRT(3,
			{ EFI_DEVICE_PATH *dp; CHAR16 *str;
		  	dp  = DevicePathFromHandle(e2fs->dev);
		  	str = DevicePathToStr(dp);
		  	Print(L"uninstalled %s on %s\n", FS_NAME, str);
		  	FreePool(str);
			});
		free(dev_tab[i].intf);
	}
	if (dev_tab) free(dev_tab);

	return EFI_SUCCESS;
}
