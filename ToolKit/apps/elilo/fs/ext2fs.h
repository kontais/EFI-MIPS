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
 */
#ifndef __EXT2FS_H__
#define __EXT2FS_H__

INTERFACE_DECL(_ext2fs_interface_t);

/*
 * simplified stat structure
 * XXX: need to cleanup types !
 */
typedef struct {
	unsigned long	st_ino;
	unsigned long	st_nlink;
	unsigned int	st_mode;
	unsigned int	st_uid;
	unsigned int	st_gid;
	unsigned long	st_size;
	unsigned long	st_atime;
	unsigned long	st_mtime;
	unsigned long	st_ctime;
} ext2fs_stat_t;


typedef struct _ext2fs_interface_t {
	EFI_STATUS (*ext2fs_name)(struct _ext2fs_interface_t *this, CHAR16 *name, UINTN maxlen);
	EFI_STATUS (*ext2fs_open)(struct _ext2fs_interface_t *this, CHAR16 *name, UINTN *fd);
	EFI_STATUS (*ext2fs_read)(struct _ext2fs_interface_t *this, UINTN fd, VOID *buf, UINTN *size);
	EFI_STATUS (*ext2fs_close)(struct _ext2fs_interface_t *this, UINTN fd);
	EFI_STATUS (*ext2fs_fstat)(struct _ext2fs_interface_t *this, UINTN fd, ext2fs_stat_t *st);
	EFI_STATUS (*ext2fs_seek)(struct _ext2fs_interface_t *this, UINTN fd, UINT64 newpos);
} ext2fs_interface_t;

#define EXT2FS_PROTOCOL \
    { 0x6ea924f6, 0xc9f2, 0x4331, {0x83, 0x54, 0x19, 0xd0, 0x17, 0x50, 0xd9, 0xc7} }

extern EFI_STATUS ext2fs_install(VOID);
extern EFI_STATUS ext2fs_uninstall(VOID);


#endif /* __EXT2FS_H__ */
