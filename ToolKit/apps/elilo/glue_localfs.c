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

#include "fs/localfs.h"
#include "glue_localfs.h"
#include "strops.h"
#include "elilo.h"

static INTN glue(fileops_t *this, VOID *intf);

/* object exported to fileops */

fileops_fs_t localfs_glue = { LOCALFS_PROTOCOL, glue, localfs_install, localfs_uninstall};


static CHAR16 localfs_default_path[FILENAME_MAXLEN];

/*
 * remove /.\ pattern from path name:
 *
 * Example 1:  I am in fs0:\efi\debian, which contains elilo.efi, and I
 * have typed 'elilo' at the efishell prompt.  set_default_path() gets
 * called with string "\EFI\debian/.\elilo.efi". The final path name
 * must then be set to "\EFI\debian\".
 * 
 * Example 2:  I am in fs0:\ and type 'efi\debian\elilo' at the shell
 * prompt.  set_default_path() is called with "\/.\efi\debian\elilo.efi",
 * the path must then be set to "\efi\debian\".
 *
 * Example 3:  I am in fs0:\efi and type '\efi\debian\elilo'.
 * set_default_path() is called with "\efi\debian\elilo.efi", the
 * path is "\efi\debian".
 */
static VOID
set_default_path(CHAR16 *sptr)
{
#define is_sep(h)	(h == CHAR_SLASH || h == CHAR_BACKSLASH)
	CHAR16 *dptr, *last_sep = NULL;
	UINTN len = FILENAME_MAXLEN - 1;
	UINTN last_was_sep = 0;
	CHAR16 c;

	dptr = localfs_default_path;

	while (len-- && *sptr) {
		c = sptr[0];

		if (is_sep(c)) {
			if (last_was_sep) {
				sptr++;
				continue;
			}
			c = CHAR_BACKSLASH;
			last_was_sep = 1;
			last_sep = dptr;
		} else {
			last_was_sep = 0;
		}
		*dptr++ = c;
		sptr++;
	}
	if (last_sep)
		*++last_sep = CHAR_NULL;
	else
		*dptr = CHAR_NULL;

	DBG_PRT((L"localfs_default_path=%s\n", localfs_default_path));
}


/*
 * The following glue functions are the only ones which need
 * to know about the way the underlying interface is working
 */
#define LOCALFS_DEFAULT_KERNEL	L"vmlinux"
#define LOCALFS_DEFAULT_CONFIG	L"elilo.conf"
static EFI_STATUS
localfs_setdefaults(VOID *this, config_file_t *config, CHAR16 *kname, UINTN maxlen, CHAR16 *devpath)
{
	StrnCpy(kname, LOCALFS_DEFAULT_KERNEL, maxlen-1);
	kname[maxlen-1] = CHAR_NULL;

	StrnCpy(config[0].fname, LOCALFS_DEFAULT_CONFIG, maxlen-1);
	config[0].fname[maxlen-1] = CHAR_NULL;

	set_default_path(devpath);

	return EFI_SUCCESS;
}

static EFI_STATUS
localfs_getdefault_path(CHAR16 *path, UINTN maxlen)
{
	if (maxlen <= StrLen(localfs_default_path)) return EFI_BUFFER_TOO_SMALL;

	StrCpy(path, localfs_default_path);
	return EFI_SUCCESS;
}

/*
 * If the supplied path is a relative path, then prepend the path to
 * the elilo.efi executable.  This ensures that elilo will look in
 * its own directory for its config file, kernel images, etc, rather
 * than the root directory of the disk.  Also * convert forward slashes
 * into backward slashes.
 */
static EFI_STATUS 
glue_open(VOID *intf, CHAR16 *name, fops_fd_t *fd)
{
	CHAR16 *p;
	localfs_interface_t *localfs = (localfs_interface_t *)intf;
	CHAR16 fullname[FILENAME_MAXLEN];

	/*
	 * XXX: modification to passed argument (name)
	 */
	for (p= name; *p != CHAR_NULL; p++) {
		if (*p == CHAR_SLASH) *p = CHAR_BACKSLASH;
	}
	if (name[0] != CHAR_BACKSLASH && localfs_default_path[0] != CHAR_NULL) {
		if (StrLen(localfs_default_path) + StrLen(name) + 1 >= FILENAME_MAXLEN)
			return EFI_INVALID_PARAMETER;

		StrCpy(fullname, localfs_default_path);
		StrCat(fullname, name);
		name = fullname;
	}
	return localfs->localfs_open(intf, name, fd);
}

static INTN
glue(fileops_t *fp, VOID *intf)
{
	localfs_interface_t *localfs = (localfs_interface_t *)intf;

	fp->open            = glue_open;
	fp->read            = (fops_read_t)localfs->localfs_read;
	fp->close           = (fops_close_t)localfs->localfs_close;
	fp->infosize        = (fops_infosize_t)localfs->localfs_infosize;
	fp->seek            = (fops_seek_t)localfs->localfs_seek;
	fp->setdefaults     = (fops_setdefaults_t)localfs_setdefaults;
	fp->getdefault_path = (fops_getdefault_path_t)localfs_getdefault_path;
	fp->intf	    = intf;

	/* fill out the name of the underlying file system */
	localfs->localfs_name(localfs, fp->name, FILEOPS_NAME_MAXLEN);

	return 0;
}
