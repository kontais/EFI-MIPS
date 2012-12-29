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
#include <efi.h>
#include <efilib.h>

#include "glue_ext2fs.h"
#include "fs/ext2fs.h"
#include "strops.h"


static INTN glue(fileops_t *this, VOID *intf);

/* object exported to fileops */

fileops_fs_t ext2fs_glue = { EXT2FS_PROTOCOL , glue, ext2fs_install, ext2fs_uninstall};

static EFI_STATUS
ext2fs_infosize(ext2fs_interface_t *this, fops_fd_t fd, UINT64 *sz)
{
	ext2fs_stat_t st;
	EFI_STATUS status;

	if (this == NULL || sz == NULL) return EFI_INVALID_PARAMETER;

	status = this->ext2fs_fstat(this, fd, &st);
	if (status != EFI_SUCCESS) return status;

	*sz  = (UINT64)st.st_size;

	return EFI_SUCCESS;
}

static INTN
glue(fileops_t *fp, VOID *intf)
{
	ext2fs_interface_t *ext2fs = (ext2fs_interface_t *)intf;

	/* record underlying interface */
	fp->intf = intf;

	fp->open     = (fops_open_t)ext2fs->ext2fs_open;
	fp->read     = (fops_read_t)ext2fs->ext2fs_read;
	fp->close    = (fops_close_t)ext2fs->ext2fs_close;
	fp->infosize = (fops_infosize_t)ext2fs_infosize; /* local override */
	fp->seek     = (fops_seek_t)ext2fs->ext2fs_seek;

	/* fill out the name of the underlying file system */
	ext2fs->ext2fs_name(ext2fs, fp->name, FILEOPS_NAME_MAXLEN);

	return 0;
}
