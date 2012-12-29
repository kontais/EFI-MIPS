/*
 *  Copyright (C) 2001-2003 Hewlett-Packard Co.
 *	Contributed by Stephane Eranian <eranian@hpl.hp.com>
 *  Copyright (C) 2006-2009 Intel Corporation
 *	Contributed by Fenghua Yu <fenghua.yu@intel.com>
 *	Contributed by Bibo Mao <bibo.mao@intel.com>
 *	Contributed by Chandramouli Narayanan <mouli@linux.intel.com>
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
#include "fs/localfs.h"

/*
 * LocalFS is just a shim layer on top of the 
 * FileSystem Protocol which gives access to FAT32,16,12
 */
#define FS_NAME L"vfat"


typedef struct {
	EFI_HANDLE	dev;		/* device we're attached to */
	EFI_FILE_HANDLE	volume;		/* root of volume */
} localfs_priv_state_t;

#define LOCALFS_F2FD(f)		((UINTN)(f))
#define LOCALFS_FD2F(fd)	((EFI_FILE_HANDLE)(fd))

typedef union {
	localfs_interface_t	pub_intf;
	struct {
		localfs_interface_t	pub_intf;
		localfs_priv_state_t	priv_data;
	} localfs_priv;
} localfs_t;

#define FS_PRIVATE(n)	(&(((localfs_t *)n)->localfs_priv.priv_data))


static EFI_GUID	LocalFsProtocol = LOCALFS_PROTOCOL;

/*
 * let's be clean here
 */
typedef union {
	EFI_HANDLE dev;
	localfs_t  *intf;
} dev_tab_t;

static dev_tab_t *dev_tab;	/* holds all devices we found */
static UINTN ndev;		/* how many entries in dev_tab */

static EFI_STATUS
localfs_name(localfs_interface_t *this, CHAR16 *name, UINTN maxlen)
{
	if (name == NULL || maxlen < 1) return EFI_INVALID_PARAMETER;

	StrnCpy(name, FS_NAME, maxlen-1);

	name[maxlen-1] = CHAR_NULL;

	return EFI_SUCCESS;
}


static EFI_STATUS
localfs_open(localfs_interface_t *this, CHAR16 *name, UINTN *fd)
{
	localfs_priv_state_t *lfs;
	EFI_STATUS 		status;
	EFI_FILE_HANDLE		fh;

	if (this == NULL || name == NULL || fd == NULL) return EFI_INVALID_PARAMETER;

	lfs = FS_PRIVATE(this);

	DBG_PRT((L"localfs_open on %s\n", name));

	status = lfs->volume->Open (lfs->volume, &fh, name, EFI_FILE_MODE_READ, (UINT64)0);
	if (status == EFI_SUCCESS) {
		*fd = LOCALFS_F2FD(fh);
	} 
	return status;
}

static EFI_STATUS
localfs_read(localfs_interface_t *this, UINTN fd, VOID *buf, UINTN *size)
{
	localfs_priv_state_t *lfs;

	if (this == NULL || fd == 0 || buf == NULL || size == NULL) return EFI_INVALID_PARAMETER;

	lfs = FS_PRIVATE(this);

	return lfs->volume->Read (LOCALFS_FD2F(fd), size, buf);
}

static EFI_STATUS
localfs_close(localfs_interface_t *this, UINTN fd)
{
	localfs_priv_state_t *lfs;

	if (this == NULL || fd == 0) return EFI_INVALID_PARAMETER;

	lfs = FS_PRIVATE(this);

	return lfs->volume->Close (LOCALFS_FD2F(fd));
}

static EFI_STATUS
localfs_infosize(localfs_interface_t *this, UINTN fd, UINT64 *sz)
{
	localfs_priv_state_t *lfs;
	EFI_FILE_INFO	     *info;

	if (this == NULL || fd == 0 || sz == NULL) return EFI_INVALID_PARAMETER;

	lfs = FS_PRIVATE(this);

	info = LibFileInfo(LOCALFS_FD2F(fd));
	if (info == NULL) return EFI_UNSUPPORTED;

	*sz = info->FileSize;

	BS->FreePool (info);

	return EFI_SUCCESS;
}

static EFI_STATUS
localfs_seek(localfs_interface_t *this, UINTN fd, UINT64 newpos)
{
	localfs_priv_state_t *lfs;

	if (this == NULL || fd == 0) return EFI_INVALID_PARAMETER;

	lfs = FS_PRIVATE(this);

	return lfs->volume->SetPosition (LOCALFS_FD2F(fd), newpos);
}

static VOID
localfs_init_state(localfs_t *localfs, EFI_HANDLE dev, EFI_FILE_HANDLE volume)
{
	localfs_priv_state_t *lfs = FS_PRIVATE(localfs);

	/* need to do some init here on localfs_intf */
	Memset(localfs, 0, sizeof(*localfs));

	localfs->pub_intf.localfs_name        = localfs_name;
	localfs->pub_intf.localfs_open        = localfs_open;
	localfs->pub_intf.localfs_read        = localfs_read;
	localfs->pub_intf.localfs_close       = localfs_close;
	localfs->pub_intf.localfs_infosize    = localfs_infosize;
	localfs->pub_intf.localfs_seek        = localfs_seek;

	lfs->dev    = dev;
	lfs->volume = volume;
}

static EFI_STATUS
localfs_install_one(EFI_HANDLE dev, VOID **intf)
{

	EFI_STATUS		status;
	localfs_t 		*localfs;
    	EFI_FILE_IO_INTERFACE	*volume;
	EFI_FILE_HANDLE		volume_fh;

	status = BS->HandleProtocol (dev, &LocalFsProtocol, (VOID **)&localfs);
	if (status == EFI_SUCCESS) {
		ERR_PRT((L"Warning: found existing %s protocol on device", FS_NAME));
		goto found;
	}
	
	status = BS->HandleProtocol (dev, &FileSystemProtocol, (VOID **)&volume);
	if (EFI_ERROR(status)) return EFI_INVALID_PARAMETER;

	status = volume->OpenVolume (volume, &volume_fh);
	if (EFI_ERROR(status)) {
		ERR_PRT((L"cannot open volume"));
		return status;
	}

	localfs = (localfs_t *)alloc(sizeof(*localfs), EfiLoaderData);
	if (localfs == NULL) {
		ERR_PRT((L"failed to allocate %s", FS_NAME));
		return EFI_OUT_OF_RESOURCES;
	}
	localfs_init_state(localfs, dev, volume_fh);

	status = LibInstallProtocolInterfaces(&dev, &LocalFsProtocol, localfs, NULL);
	if (EFI_ERROR(status)) {
		ERR_PRT((L"Cannot install %s protocol: %r", FS_NAME, status));
		free(localfs);
		return status;
	}
found:
	if (intf) *intf = (VOID *)localfs;

	VERB_PRT(3,
		{ EFI_DEVICE_PATH *dp; CHAR16 *str;
		  dp  = DevicePathFromHandle(dev);
		  str = DevicePathToStr(dp);
		  Print(L"attached %s to %s\n", FS_NAME, str);
		  BS->FreePool (str);
		});

	return EFI_SUCCESS;
}
	
EFI_STATUS
localfs_install(VOID)
{
	UINTN size = 0;
	UINTN i;
	EFI_STATUS status;
	VOID *intf;

	BS->LocateHandle (ByProtocol, &FileSystemProtocol, NULL, &size, NULL);
	if (size == 0) return EFI_UNSUPPORTED; /* no device found, oh well */

	DBG_PRT((L"size=%d", size));

	dev_tab = (dev_tab_t *)alloc(size, EfiLoaderData);
	if (dev_tab == NULL) {
		ERR_PRT((L"failed to allocate handle table"));
		return EFI_OUT_OF_RESOURCES;
	}
	status = BS->LocateHandle (ByProtocol, &FileSystemProtocol, NULL, &size, (VOID **)dev_tab);
	if (status != EFI_SUCCESS) {
		ERR_PRT((L"failed to get handles: %r", status));
		free(dev_tab);
		return status;
	}
	ndev = size / sizeof(EFI_HANDLE);

	for(i=0; i < ndev; i++) {
		intf = NULL;
		localfs_install_one(dev_tab[i].dev, &intf);
		/* override device handle with interface pointer */
		dev_tab[i].intf = intf;
	}

	return EFI_SUCCESS;
}
	
EFI_STATUS
localfs_uninstall(VOID)
{
	
	localfs_priv_state_t *lfs;
	EFI_STATUS status;
	UINTN i;

	for(i=0; i < ndev; i++) {
		if (dev_tab[i].intf == NULL) continue;
		lfs = FS_PRIVATE(dev_tab[i].intf);
		status = BS->UninstallProtocolInterface (lfs->dev, &LocalFsProtocol, dev_tab[i].intf);
		if (EFI_ERROR(status)) {
			ERR_PRT((L"Uninstall %s error: %r", FS_NAME, status));
			continue;
		}
		VERB_PRT(3,
			{ EFI_DEVICE_PATH *dp; CHAR16 *str;
		  	dp  = DevicePathFromHandle(lfs->dev);
		  	str = DevicePathToStr(dp);
		  	Print(L"uninstalled %s on %s\n", FS_NAME, str);
			BS->FreePool (str);
			});
		free(dev_tab[i].intf);
	}
	if (dev_tab) free(dev_tab);

	return EFI_SUCCESS;
}


