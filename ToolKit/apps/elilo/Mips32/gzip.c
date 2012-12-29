/*
 *  Copyright (C) 2001-2002 Hewlett-Packard Co.
 *	Contributed by Stephane Eranian <eranian@hpl.hp.com>
 *
 *  Copyright (C) 2001 Silicon Graphics, Inc.
 *      Contributed by Brent Casavant <bcasavan@sgi.com>
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
 */

#include <efi.h>
#include <efilib.h>

#include "elf.h"
#include "elilo.h"
#include "gzip.h"
#include "private.h"

#define LD_NAME L"gzip_ia32"

#define memzero(s, n)	Memset((VOID *)(s), 0, (n))
#define memcpy(a,b,n)	Memcpy((VOID *)(a),(b),(n))

/* size of output buffer */
#define WSIZE 0x8000		/* Window size must be at least 32k, */
				/* and a power of two */
/* size of input buffer */
#define INBUFSIZE 0x8000

/*
 * gzip declarations
 */

#define OF(args)  args
#define FUNC_STATIC static

typedef unsigned char  uch;
typedef unsigned short ush;
typedef unsigned long  ulg;


typedef struct segment {
	unsigned long addr;	/* start address */
       	unsigned long offset;	/* file offset   */
       	unsigned long size;	/* file size     */
	unsigned long bss_sz;	/* BSS size      */
	UINT8	flags;	/* indicates whether to load or not */
} segment_t;

#define CHUNK_FL_VALID		0x1
#define CHUNK_FL_LOAD		0x2

#define CHUNK_CAN_LOAD(n)	chunks[(n)].flags |= CHUNK_FL_LOAD
#define CHUNK_NO_LOAD(n)	chunks[(n)].flags &= ~CHUNK_FL_LOAD
#define CHUNK_IS_LOAD(n)	(chunks[(n)].flags & CHUNK_FL_LOAD)

#define CHUNK_VALIDATE(n)	chunks[(n)].flags |= CHUNK_FL_VALID
#define CHUNK_INVALIDATE(n)	chunks[(n)].flags = 0
#define CHUNK_IS_VALID(n)	(chunks[(n)].flags & CHUNK_FL_VALID)

/*
 * static parameters to gzip helper functions
 * we cannot use paramters because API was not
 * designed that way
 */
static segment_t *chunks;	/* holds the list of segments */
static segment_t *cur_chunk;
static UINTN nchunks;
static UINTN chunk;                 /* current segment */
static UINTN input_fd;
static VOID *kernel_entry, *kernel_base, *kernel_end;

static uch *inbuf;		/* input buffer (compressed data) */
static uch *window;    		/* output buffer (uncompressed data) */
static unsigned long file_offset;	/* position in the file */

static unsigned insize = 0;  /* valid bytes in inbuf */
static unsigned inptr  = 0;   /* index of next byte to be processed in inbuf */
static unsigned outcnt = 0;  /* bytes in output buffer */

/* gzip flag byte */
#define ASCII_FLAG   0x01 /* bit 0 set: file probably ASCII text */
#define CONTINUATION 0x02 /* bit 1 set: continuation of multi-part gzip file */
#define EXTRA_FIELD  0x04 /* bit 2 set: extra field present */
#define ORIG_NAME    0x08 /* bit 3 set: original file name present */
#define COMMENT      0x10 /* bit 4 set: file comment present */
#define ENCRYPTED    0x20 /* bit 5 set: file is encrypted */
#define RESERVED     0xC0 /* bit 6,7:   reserved */

#define get_byte()  (inptr < insize ? inbuf[inptr++] : fill_inbuf())

/* Diagnostic functions */
#ifdef INFLATE_DEBUG
#  define Assert(cond,msg) {if(!(cond)) error(msg);}
int stderr;
#  define Trace(x) Print(L"line %d:\n", __LINE__);
#  define Tracev(x) {if (verbose) Print(L"line %d:\n", __LINE__) ;}
#  define Tracevv(x) {if (verbose>1) Print(L"line %d:\n", __LINE__)  ;}
#  define Tracec(c,x) {if (verbose && (c))  Print(L"line %d:\n", __LINE__) ;}
#  define Tracecv(c,x) {if (verbose>1 && (c))  Print(L"line %d:\n", __LINE__) ;}
#else
#  define Assert(cond,msg)
#  define Trace(x)
#  define Tracev(x)
#  define Tracevv(x)
#  define Tracec(c,x)
#  define Tracecv(c,x)
#endif

static int  fill_inbuf(void);
static void flush_window(void);
static void error(char *m);
static long bytes_out;
static void error(char *m);
static int error_return;

static void *
gzip_malloc(int size)
{
	return (void *)alloc(size, 0);
}

static void
gzip_free(void *where)
{	
	return free(where);
}

#include "inflate.c"

/*
 * Fill the input buffer and return the first byte in it. This is called
 * only when the buffer is empty and at least one byte is really needed.
 */
int
fill_inbuf(void)
{
	UINTN expected, nread;
	EFI_STATUS status;

	expected = nread = INBUFSIZE;

	status = fops_read(input_fd, inbuf, &nread);
	if (EFI_ERROR(status)) {
    		error("elilo: Read failed");
	}
#ifdef DEBUG_GZIP
	DBG_PRT((L"%s : read %d bytes of %d bytes\n", LD_NAME, nread, expected));
#endif

	insize = nread;
	inptr = 1;

	return inbuf[0];
}

/* ===========================================================================
 * Write the output window window[0..outcnt-1] and update crc and bytes_out.
 * (Used for the decompressed data only.)
 */

/*
 * Run a set of bytes through the crc shift register.  If s is a NULL
 * pointer, then initialize the crc shift register contents instead.
 * Return the current crc in either case.
 *
 * Input:
 *	S	pointer to bytes to pump through.
 *	N	number of bytes in S[].
 */
unsigned long
updcrc(unsigned char *s, unsigned n)
{
	register unsigned long c;
	/* crc is defined in inflate.c */

	if (!s) {
		c = 0xffffffffL;
	} else {
		c = crc;
		while (n--) {
			c = crc_32_tab[((int)c ^ (*s++)) & 0xff] ^ (c >> 8);
		}
	}
	crc = c;
	return c ^ 0xffffffffUL;       /* (instead of ~c for 64-bit machines) */
}


/*
 * Clear input and output buffers
 */
void
clear_bufs(void)
{
	outcnt = 0;
	inptr = 0;
	chunk = 0;
	cur_chunk = NULL;
	file_offset = 0;
}


static INTN
is_valid_header(Elf32_Ehdr *ehdr)
{
	UINT16 type, machine;

	type    = ehdr->e_type;
	machine = ehdr->e_machine;
	
	VERB_PRT(3, Print(L"class=%d type=%d data=%d machine=%d\n", 
		ehdr->e_ident[EI_CLASS],
		type,
		ehdr->e_ident[EI_DATA],
		machine));

	return    ehdr->e_ident[EI_MAG0]  == 0x7f 
	       && ehdr->e_ident[EI_MAG1]  == 'E'
	       && ehdr->e_ident[EI_MAG2]  == 'L'
	       && ehdr->e_ident[EI_MAG3]  == 'F'
	       && ehdr->e_ident[EI_CLASS] == ELFCLASS32 
	       && type                    == ET_EXEC	/* must be executable */
	       && machine                 == EM_386 ? 0 : -1;
}

/*
 * will invalidate loadble segments which overlap with others
 */ 
void
check_overlap(int i)
{
	int j;
	unsigned long iend = chunks[i].addr + chunks[i].size;

	for(j=0; j < nchunks; j++) {
		if (j ==i) continue;
		if (chunks[i].addr >= chunks[j].addr && iend < (chunks[j].addr + chunks[j].size)) {
			DBG_PRT((L"%s : segment %d fully included in segment %d\n", LD_NAME, i, j));
			CHUNK_INVALIDATE(i); /* nullyify segment */
			break;
		}
	}
}

void
analyze_chunks(void)
{
	INTN i;

	for (i=0; i < nchunks; i++) {
		if (CHUNK_IS_VALID(i) && !CHUNK_IS_LOAD(i)) 
			check_overlap(i);
	}
}


/*
 * The decompression code calls this function after decompressing the
 * first block of the object file.  The first block must contain all
 * the relevant header information.
 */
int
first_block (const unsigned char *buf, long blocksize)
{
	Elf32_Ehdr *elf;
	Elf32_Phdr *phdrs;
	UINTN total_size, pages;
	UINTN low_addr, max_addr;
	UINTN offs = 0;
	UINT16 phnum;
	UINTN paddr, memsz;
	INTN i;

	elf  = (Elf32_Ehdr *)buf;
	
	if (is_valid_header(elf) == -1) 
		return -1;

	offs  = elf->e_phoff;
	phnum = elf->e_phnum;

	VERB_PRT(3, { 
		Print(L"Entry point "PTR_FMT"\n", elf->e_entry);
		Print(L"%d program headers\n", phnum);
		Print(L"%d segment headers\n", elf->e_shnum);
	});

	if (offs + phnum * sizeof(*phdrs) > (unsigned) blocksize) {
		ERR_PRT((L"%s : ELF program headers not in first block (%d)\n", LD_NAME, offs));
		return -1;
	}

	kernel_entry = (VOID *)(elf->e_entry & PADDR_MASK);

	phdrs = (Elf32_Phdr *) (buf + offs);
	low_addr = ~0;
	max_addr = 0;

	/*
	 * allocate chunk table
	 * Convention: a segment that does not need loading will
	 * have chunk[].addr = 0.
	 */
	chunks = (void *)alloc(sizeof(struct segment)*phnum, 0);
    	if (chunks == NULL) {
        	ERR_PRT((L"%s : failed alloc chunks %r\n", LD_NAME));
		return -1;
    	}
	nchunks = phnum;
	/*
	 * find lowest and higest virtual addresses
	 * don't assume FULLY sorted !
	 */
	for (i = 0; i < phnum; ++i) {
		/* 
		 * record chunk no matter what because no load may happen
		 * anywhere in archive, not just as the last segment
		 */
		paddr = (phdrs[i].p_paddr & PADDR_MASK);
		memsz = phdrs[i].p_memsz,

		chunks[i].addr   = paddr;
		chunks[i].offset = phdrs[i].p_offset;
		chunks[i].size   = phdrs[i].p_filesz;
		chunks[i].bss_sz = phdrs[i].p_memsz - phdrs[i].p_filesz;

		CHUNK_VALIDATE(i);

		if (phdrs[i].p_type != PT_LOAD) {
			CHUNK_NO_LOAD(i); /* mark no load chunk */
			DBG_PRT((L"%s : skipping segment %d\n", LD_NAME, i));
			continue;
		}

		CHUNK_CAN_LOAD(i); /* mark no load chunk */

		VERB_PRT(3, 
		Print(L"\n%s : segment %d vaddr ["PTR_FMT"-"PTR_FMT"] offset %d filesz %d "
			"memsz=%d bss_sz=%d\n",
			LD_NAME, 1+i, chunks[i].addr, chunks[i].addr+phdrs[i].p_filesz, 
			chunks[i].offset, chunks[i].size, memsz, chunks[i].bss_sz));
		
		if (paddr < low_addr) 
			low_addr = paddr;
		if (paddr + memsz > max_addr) 
			max_addr = paddr + memsz;
	}

	if (low_addr & (EFI_PAGE_SIZE - 1)) {
		ERR_PRT((L"%s : low_addr not page aligned "PTR_FMT"\n", LD_NAME, low_addr));
		goto error;
	}
	analyze_chunks();

	DBG_PRT((L"%s : %d program headers entry=" PTR_FMT "\nlowest_addr="PTR_FMT" highest_addr="PTR_FMT"\n", 
			LD_NAME,
			phnum, kernel_entry, low_addr, max_addr));

	total_size = (UINTN)max_addr - (UINTN)low_addr;
	pages = EFI_SIZE_TO_PAGES(total_size);

	/*
	 * Record end of kernel for initrd
	 */
	kernel_base = (void *)low_addr;
	kernel_end  = (void *)(low_addr + (pages << EFI_PAGE_SHIFT));

	/* allocate memory for the kernel */
	if (alloc_kmem((void *)low_addr, pages) == -1) {
		ERR_PRT((L"%s : AllocatePages(%d, "PTR_FMT") for kernel failed\n", 
			LD_NAME, pages, low_addr));
		ERR_PRT((L"%s : Could not load kernel at "PTR_FMT"\n", LD_NAME, low_addr));
		ERR_PRT((L"%s : Bailing\n", LD_NAME));
		goto error;
	}
	return 0;
error:
	if (chunks) 
		free(chunks);
	return -1;
}

/*
 * Determine which chunk in the Elf file will be coming out of the expand
 * code next.
 */
static void
nextchunk(void)
{
	int i;
	segment_t *cp;

	cp = NULL;
	for(i=0; i < nchunks; i++) {

		if (!CHUNK_IS_VALID(i) || !CHUNK_IS_LOAD(i)) continue;

		if (file_offset > chunks[i].offset) continue;

		if (cp == NULL || chunks[i].offset < cp->offset) cp = &chunks[i];
	}
	cur_chunk = cp;
}


/*
 * Write the output window window[0..outcnt-1] holding uncompressed
 * data and update crc.
 */
void
flush_window(void)
{
	static const CHAR8 helicopter[4] = { '|' , '/' , '-' , '\\' };
	static UINTN heli_count;
	struct segment *cp;
	unsigned char	*src, *dst;
	long	cnt;

	if (!outcnt) return;
#ifdef DEBUG_GZIP
	DBG_PRT((L"%s : flush_window outnct=%d file_offset=%d\n", LD_NAME, outcnt, file_offset));
#endif

	Print(L"%c\b",helicopter[heli_count++%4]);

	updcrc(window, outcnt);

	/* first time, we extract the headers */
	if (!bytes_out) {
		if (first_block(window, outcnt) < 0) 
			error("invalid exec header"); 
		nextchunk();
	}

	bytes_out += outcnt;
	src = window;
tail:
	/* check if user wants to abort */
	if (check_abort() == EFI_SUCCESS) goto load_abort;

	cp = cur_chunk;
	if (cp == NULL || file_offset + outcnt <= cp->offset) {
		file_offset += outcnt;
		return;
	}

	/* Does this window begin before the current chunk? */
	if (file_offset < cp->offset) {
		unsigned long skip = cp->offset - file_offset;

		src         += skip;
		file_offset += skip;
		outcnt      -= skip;
	}
	dst = (unsigned char *)cp->addr + (file_offset - cp->offset);
	cnt = cp->offset + cp->size - file_offset;
	if (cnt > outcnt) 
		cnt = outcnt;

	Memcpy(dst, src, cnt);

	file_offset += cnt;
	outcnt      -= cnt;
	src         += cnt;

	/* See if we are at the end of this chunk */
	if (file_offset == cp->offset + cp->size) {
		if (cp->bss_sz) {
			dst = (unsigned char *)cp->addr + cp->size;
			Memset(dst, 0, cp->bss_sz);
		}
		nextchunk();
		/* handle remaining bytes */
		if (outcnt) 
			goto tail; 
	}
	return;
load_abort:
	free_kmem();
	error_return = ELILO_LOAD_ABORTED;
}

static void
error(char *x)
{
	ERR_PRT((L"%s : %a", LD_NAME, x));
	/* will eventually exit with error from gunzip() */
}

INT32
decompress_kernel(VOID)
{
	INT32 ret;

	clear_bufs();
	makecrc();
	Print(L"Uncompressing Linux... ");
	ret = gunzip();
	if (ret == 0) 
		Print(L"done\n");
	return ret == 0 ? 0 : -1;
}

int
gunzip_kernel(fops_fd_t fd, kdesc_t *kd)
{
	int ret = -1;

	error_return = ELILO_LOAD_ERROR;
	
	window = (void *)alloc(WSIZE, 0);
    	if (window == NULL) {
        	ERR_PRT((L"%s : allocate output window failed\n", LD_NAME));
		return -1;
    	}

	inbuf = (void *)alloc(INBUFSIZE, 0);
    	if (inbuf == NULL) {
        	ERR_PRT((L"%s : allocate input window failedr\n", LD_NAME));
		goto error;
    	}
	input_fd   = fd;
	insize     = 0;
	bytes_out  = 0;

	ret = decompress_kernel();
error:
	if (window) free(window);
	if (inbuf) free(inbuf);

	if (ret == 0) {
		kd->kentry = kernel_entry;
		kd->kend   = kernel_end;
		kd->kstart = kernel_base;
		error_return = ELILO_LOAD_SUCCESS;
	}
	return error_return;
}
