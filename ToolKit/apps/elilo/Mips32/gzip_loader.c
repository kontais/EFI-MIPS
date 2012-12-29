/*
 *  Copyright (C) 2001-2002 Hewlett-Packard Co.
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

#include "elilo.h"
#include "loader.h"
#include "gzip.h"

#define LD_NAME L"gzip_ia32"

static INTN
gzip_probe_format(CHAR16 *kname)
{
	UINT8 buf[4];
	EFI_STATUS status;
	INTN ret = -1;
	UINTN size;
	fops_fd_t fd;

	status = fops_open(kname, &fd);
	if (EFI_ERROR(status)) return -1;

	size = sizeof(buf);
	status = fops_read(fd, buf, &size);

	if (EFI_ERROR(status) || size != sizeof(buf)) goto error;

	ret = gzip_probe(buf, sizeof(buf));
error:
	fops_close(fd);
	return ret;
}


static INTN
gzip_load_kernel(CHAR16 *kname, kdesc_t *kd)
{
	EFI_STATUS status;
	INT32 ret;
	fops_fd_t fd;

	status = fops_open(kname, &fd);
	if (EFI_ERROR(status)) return ELILO_LOAD_ERROR;

	ret = gunzip_kernel(fd, kd);
       
	fops_close(fd);

	return ret; /* could be success, error, or abort */
}

loader_ops_t gzip_loader={
	NULL,
	LD_NAME,
	gzip_probe_format,
	gzip_load_kernel
};
