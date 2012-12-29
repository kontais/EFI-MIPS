/*
 *  Copyright (C) 2001-2003 Hewlett-Packard Co.
 *	Contributed by Stephane Eranian <eranian@hpl.hp.com>
 *
 * This file is part of the ELILO, the EFI Linux boot loader.
 *
 *  GNU EFI is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  GNU EFI is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GNU EFI; see the file COPYING.  If not, write to the Free
 *  Software Foundation, 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 * Please check out the elilo.txt for complete documentation on how
 * to use this program.
 */

#ifndef __FILEOPS_H__
#define __FILEOPS_H__

#define FILEOPS_NAME_MAXLEN	32	/* length of filesystem name */

/*
 * upper-level interface used by the bootloader
 */
typedef UINTN	fops_fd_t;

/* Forward declaration: */
struct config_file;

extern EFI_STATUS fops_open(CHAR16 *name, fops_fd_t *fd);
extern EFI_STATUS fops_read(fops_fd_t fd,VOID *buf, UINTN *size);
extern EFI_STATUS fops_close(fops_fd_t fd);
extern EFI_STATUS fops_infosize(fops_fd_t fd, UINT64 *size);
extern EFI_STATUS fops_seek(fops_fd_t fd, UINT64 newpos);
extern EFI_STATUS fops_setdefaults(struct config_file *defconf, CHAR16 *kname, UINTN maxlen, CHAR16 *devpath);
extern EFI_STATUS fops_getdefault_path(CHAR16 *path, UINTN maxlen);
extern CHAR16     *fops_bootdev_name(VOID);


/*
 * fileops interface used by underlying filesystems layer
 */

typedef EFI_STATUS (*fops_open_t)(VOID *intf, CHAR16 *name, fops_fd_t *fd);
typedef EFI_STATUS (*fops_read_t)(VOID *intf, fops_fd_t fd, VOID *buf, UINTN *size);
typedef	EFI_STATUS (*fops_close_t)(VOID *intf, fops_fd_t fd);
typedef	EFI_STATUS (*fops_infosize_t)(VOID *intf, fops_fd_t fd, UINT64 *size);
typedef	EFI_STATUS (*fops_seek_t)(VOID *intf, fops_fd_t fd, UINT64 newpos);
typedef	EFI_STATUS (*fops_setdefaults_t)(VOID *intf, struct config_file *defconfs, CHAR16 *kname, UINTN maxlen, CHAR16 *devpath);
typedef	EFI_STATUS (*fops_getdefault_path_t)(CHAR16 *path, UINTN maxlen);

typedef struct {
	VOID *intf;	/* pointer to underlying interface */

	fops_open_t		open;
	fops_read_t		read;
	fops_close_t		close;
	fops_infosize_t		infosize;
	fops_seek_t		seek;
	fops_setdefaults_t	setdefaults;
	fops_getdefault_path_t	getdefault_path;

	EFI_HANDLE dev;	/* handle on device on which proto is installed */
	CHAR16 name[FILEOPS_NAME_MAXLEN];
} fileops_t;

/* 
 * used to register a new filsystem
 */
typedef INTN (*fops_fs_glue_t)(fileops_t *this, VOID *intf);
typedef EFI_STATUS (*fops_fs_install_t)(VOID);
typedef EFI_STATUS (*fops_fs_uninstall_t)(VOID);

typedef struct {
	EFI_GUID		proto;	   /* GUID of filesystem */
	fops_fs_glue_t		glue;	   /* glue routine */
	fops_fs_install_t	install;   /* to go away with real EFI drivers */
	fops_fs_uninstall_t	uninstall; /* to go away with real EFI drivers */
} fileops_fs_t;


/*
 * device description
 */
#define FILEOPS_DEVNAME_MAXLEN	16

typedef struct {
	EFI_HANDLE  		dev;
	fileops_t		*fops;
	CHAR16	    		name[FILEOPS_DEVNAME_MAXLEN];
} device_t;

extern INTN init_devices(EFI_HANDLE boot_handle);
extern INTN close_devices(VOID);
extern VOID print_devices(VOID);
extern EFI_STATUS fops_get_next_device(UINTN pidx, CHAR16 *type, UINTN maxlen, UINTN *idx, CHAR16 *name, EFI_HANDLE *dev);
extern INTN fops_split_path(CHAR16 *path, CHAR16 *dev);
extern EFI_STATUS fops_get_device_handle(CHAR16 *name, EFI_HANDLE *dev);

/*
 * device naming schemes
 *
 * Interface:
 * 	the scheme() function arguments are:
 * 		- the list of detected bootable device
 * 		- the number of entries in that table as argument
 *
 * 	There is no expected return value. If the scheme() cannot perform
 * 	its tasks, then IT MUST NOT OVERWRITE the generic naming scheme (devXXX) that
 * 	is ALWAYS installed by default. Partial modifications are possible, although
 * 	not recommended.
 */
typedef struct {
	CHAR16	*name;
	INTN	(*install_scheme)(device_t *tab, UINTN ndev);
} devname_scheme_t;

#endif /* __FILEOPS_H__ */
