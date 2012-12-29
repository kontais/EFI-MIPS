/*
 *  Copyright (C) 2001-2003 Hewlett-Packard Co.
 *	Contributed by Stephane Eranian <eranian@hpl.hp.com>
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
#include "fileops.h"

/*
 * import filesystems
 */
#ifdef CONFIG_LOCALFS
#include "glue_localfs.h"
#endif
#ifdef CONFIG_NETFS
#include "glue_netfs.h"
#endif

#ifdef CONFIG_EXT2FS
#include "glue_ext2fs.h"
#endif

/*
 * table of device naming schemes.
 * we will probe each one in sequential order and stop at first match.
 */
extern devname_scheme_t simple_devname_scheme;

static devname_scheme_t *devname_scheme_tab[]={
	&simple_devname_scheme,
	NULL
};


/*
 * Once we are able to create Boot Services drivers we won't need
 * this hack and we'll be able to load the driver from the boot manager
 * or EFI shell. So for now we explicitely invoke the probe routine
 * of each driver that we know about
 */

typedef EFI_STATUS (fops_fixups_t)(EFI_HANDLE dev, device_t *d);

/*
 * List of filesystem we currently support
 */


static fileops_fs_t *fs_tab[]={
#ifdef CONFIG_LOCALFS
	&localfs_glue, 
#endif
#ifdef CONFIG_EXT2FS
	&ext2fs_glue,
#endif
#ifdef CONFIG_NETFS
	&netfs_glue, 
#endif
	NULL
};

static device_t *dev_tab;
static UINTN ndev, ndev_boot;
static device_t *boot_dev;

typedef struct _fdesc_t {
	struct _fdesc_t *next;	/* pointer to next free (when in free list) */
	fileops_t	*fops;	/* pointer to filesystem-specific glue interface */
	UINTN		fh; 	/* file descriptor from lower level protocol */
} fdesc_t;

#define FILEOPS_FD_MAX	16

static fdesc_t fd_tab[FILEOPS_FD_MAX];
static fdesc_t *free_fd;

static devname_scheme_t *current_devname_scheme;

#define FDESC_IDX(f)	(fops_fd_t)(f-fd_tab)
#define FDESC_INVALID_FD(fd)	(fd >= FILEOPS_FD_MAX || fd_tab[(fd)].fops == NULL)
#define FDESC_F(fd)		(fd_tab+(fd))

static fdesc_t *
fd_alloc(VOID)
{
	fdesc_t *tmp = NULL;

	if (free_fd == NULL) {
		ERR_PRT((L"out of file descriptor"));
	} else {
		tmp = free_fd;
		free_fd = free_fd->next;
	}
	return tmp;
}

static VOID
fd_free(fdesc_t *fd)
{
	if (fd == NULL) {
		ERR_PRT((L"invalid fd"));
	}
	fd->fops = NULL; /* mark as invalid */
	fd->next = free_fd;
	free_fd = fd;
}

static fileops_t *
glue_filesystem(EFI_GUID *proto, EFI_HANDLE dev, fops_fs_glue_t glue)
{
	fileops_t *fops;
	VOID *intf = NULL;
	EFI_STATUS status;

	status = BS->HandleProtocol (dev, proto, &intf);
	if (EFI_ERROR(status)) {
		ERR_PRT((L"unable to locate %g: should not happen", proto));
		return NULL; /* should not happen */
	}
	fops = (fileops_t *)alloc(sizeof(*fops), EfiLoaderData);
	if (fops == NULL) {
		ERR_PRT((L"failed to allocate fileops"));
		return NULL; /* XXX uninstall protocol */
	}
	Memset(fops, 0, sizeof(*fops));

	fops->dev = dev;

	/* initialize private interface now */
	glue(fops, intf);

	return fops;
}

INTN
fops_split_path(CHAR16 *path, CHAR16 *dev)
{
	CHAR16 *p, *d = dev;
	UINTN len = FILEOPS_DEVNAME_MAXLEN;
#	define CHAR_COLON	L':'

	p = StrChr(path, CHAR_COLON);
	if (p == NULL) {
		*dev = CHAR_NULL; 
		return 0;
	}
	/* copy device part of the name */
	for (d = dev ; path != p ;) {
		if (--len == 0) return -1; /* too long */
		*d++ = *path++;
	}
	*d = CHAR_NULL;
	return 0;
}

static device_t *
find_device_byname(CHAR16 *dev)
{
	UINTN i;
	for(i=0; i < ndev; i++) {
		if (!StrCmp(dev, dev_tab[i].name)) return dev_tab+i;
	}
	return NULL;
}


/*
 * This function is used to walk through the device list and get names.
 * arguments:
 * 	- pidx = opaque cookie returned by previous call (must be zero for initial call)
 * 	- type is the type of filesystem to look for, e.g., vfat or ext2fs. NULL means ANY
 * 	- maxlen is the size in bytes of the returned string buffer
 * 	- idx is the opaque cookie returned (can be reused by next call)
 * 	- name a string buffer to hold the name of the next matching device
 *	- dev will receive the EFI_HANDLE corresponding to the device  if not NULL
 *
 * return:
 * 	- EFI_INVALID_PARAMETER: when NULL values are passed for required arguments.
 * 	- EFI_NOT_FOUND: when reached the end of the list of invalid cookie
 * 	- EFI_BUFFER_TOO_SMALL is device name does not fit into string buffer
 * 	- EFI_SUCCESS otherwise
 */
EFI_STATUS
fops_get_next_device(UINTN pidx, CHAR16 *type, UINTN maxlen, UINTN *idx, CHAR16 *name, EFI_HANDLE *dev)
{
	UINTN i;

	if (name == NULL || idx == NULL) return EFI_INVALID_PARAMETER;

	for(i=pidx; i < ndev; i++) {
		if (type == NULL || !StrCmp(dev_tab[i].fops->name, type)) goto found;
	}
	return EFI_NOT_FOUND;
found:
	if (StrLen(dev_tab[i].name) >= maxlen) {
		*idx = pidx; /* will restart from same place! */
		return EFI_BUFFER_TOO_SMALL;
	}
	StrCpy(name, dev_tab[i].name);
	*idx = i+1;
	if (dev) *dev = dev_tab[i].dev; 

	return EFI_SUCCESS;
}

EFI_STATUS
fops_get_device_handle(CHAR16 *name, EFI_HANDLE *dev)
{
	UINTN i;

	if (name == NULL || dev == NULL) return EFI_INVALID_PARAMETER;

	for(i=0; i < ndev; i++) {
		if (!StrCmp(dev_tab[i].name, name)) goto found;
	}
	return EFI_NOT_FOUND;
found:
	*dev = dev_tab[i].dev; 

	return EFI_SUCCESS;
}


EFI_STATUS
fops_open(CHAR16 *fullname, fops_fd_t *fd)
{
	fdesc_t *f;
	EFI_STATUS status;
	fileops_t *fops;
	device_t *dev;
	CHAR16 dev_name[FILEOPS_DEVNAME_MAXLEN];
	CHAR16 *name;


	if (fd == NULL || fullname == NULL || *fullname == CHAR_NULL || fops_split_path(fullname, dev_name) == -1) 
		return EFI_INVALID_PARAMETER;

	DBG_PRT((L"fops_open(%s), dev:%s:", fullname ? fullname : L"(NULL)", dev_name));

	name = fullname;
	if (dev_name[0] == CHAR_NULL) {
		if (boot_dev == NULL) return EFI_INVALID_PARAMETER;
		fops = boot_dev->fops;
	} else {
		if ((dev=find_device_byname(dev_name)) == NULL) return EFI_INVALID_PARAMETER;

		name += StrLen(dev_name)+1;

		fops = dev->fops;
	}

	if (fops == NULL) return EFI_INVALID_PARAMETER;

	if ((f=fd_alloc()) == NULL) return EFI_OUT_OF_RESOURCES;

	DBG_PRT((L"dev:%s: fullname:%s: name:%s: f=%d", dev_name, fullname, name, f - fd_tab));

	status = fops->open(fops->intf, name, &f->fh);
	if (EFI_ERROR(status)) goto error;

	f->fops = fops;

	*fd = FDESC_IDX(f);

	return EFI_SUCCESS;
error:
	fd_free(f);
	return status;
}

EFI_STATUS
fops_read(fops_fd_t fd, VOID *buf, UINTN *size)
{
	fdesc_t *f;

	if (buf == NULL || FDESC_INVALID_FD(fd) || size == NULL) return EFI_INVALID_PARAMETER;

	f = FDESC_F(fd);

	return f->fops->read(f->fops->intf, f->fh, buf, size);
}

EFI_STATUS
fops_close(fops_fd_t fd)
{
	fdesc_t *f;
	EFI_STATUS status;

	if (FDESC_INVALID_FD(fd)) return EFI_INVALID_PARAMETER;

	f = FDESC_F(fd);

	status = f->fops->close(f->fops->intf, f->fh);

	fd_free(f);

	return status;
}

EFI_STATUS
fops_infosize(fops_fd_t fd, UINT64 *size)
{
	fdesc_t *f;

	if (FDESC_INVALID_FD(fd) || size == NULL) return EFI_INVALID_PARAMETER;

	f = FDESC_F(fd);

	return f->fops->infosize(f->fops->intf, f->fh, size);
}

EFI_STATUS
fops_seek(fops_fd_t fd, UINT64 newpos)
{
	fdesc_t *f;

	if (FDESC_INVALID_FD(fd)) return EFI_INVALID_PARAMETER;

	f = FDESC_F(fd);

	return f->fops->seek(f->fops->intf, f->fh, newpos);
}

EFI_STATUS
fops_setdefaults(struct config_file *defconf, CHAR16 *kname, UINTN maxlen, CHAR16 *devpath)
{
    INTN i;

/*
 * The first default config file is architecture dependent. This is useful
 * in case of network booting where the server is used for both types of
 * architectures.
 */
#if defined(CONFIG_ia64)
#define FILEOPS_ARCH_DEFAULT_CONFIG	L"elilo-ia64.conf"
#elif defined (CONFIG_Mips32)
#define FILEOPS_ARCH_DEFAULT_CONFIG	L"elilo-ia32.conf"
#elif defined (CONFIG_x86_64)
#define FILEOPS_ARCH_DEFAULT_CONFIG    L"elilo-x86_64.conf"
#else
#error "You need to specfy your default arch config file"
#endif

/* 
 * last resort config file. Common to all architectures
 */
#define FILEOPS_DEFAULT_CONFIG	L"elilo.conf"

#define FILEOPS_DEFAULT_KERNEL		L"vmlinux"

#ifdef ELILO_DEBUG
	if (defconf == NULL || kname == NULL) return EFI_INVALID_PARAMETER;
#endif

        for (i=0; i<MAX_DEFAULT_CONFIGS; i++) {
            defconf[i].fname[0] = CHAR_NULL;
        }

	if (boot_dev == NULL || boot_dev->fops == NULL) {
		if (boot_dev == NULL) 
			Print(L"Warning boot device not recognized\n");
		else
			Print(L"Unknown filesystem on boot device\n");

		Print(L"Using builtin defaults for kernel and config file\n");

		StrnCpy(kname, FILEOPS_DEFAULT_KERNEL, maxlen-1);
	}
        else {
                boot_dev->fops->setdefaults(boot_dev->fops->intf, defconf, kname, maxlen, devpath);
        }
        i=0; while (i<MAX_DEFAULT_CONFIGS && defconf[i].fname[0] != CHAR_NULL) i += 1;
#ifdef ELILO_DEBUG
        if ((i+3) >= MAX_DEFAULT_CONFIGS) {
            Print(L"ERROR: i = %d, MAX_DEFAULT_CONFIGS is not large enough\n", i);
            return EFI_INVALID_PARAMETER;
        }
#endif
        StrnCpy(defconf[i].fname, FILEOPS_ARCH_DEFAULT_CONFIG, maxlen-1);
        StrnCpy(defconf[i+1].fname, FILEOPS_DEFAULT_CONFIG, maxlen-1);

#ifdef ELILO_DEBUG
        VERB_PRT(3,Print(L"Default config filename list:\n"));
        for (i=0; i<MAX_DEFAULT_CONFIGS; i++) {
                if (defconf[i].fname[0] == CHAR_NULL) { break; }
                        VERB_PRT(3,Print(L"\t%s\n", defconf[i].fname));
        }
#endif
        
	return EFI_SUCCESS;
}

EFI_STATUS
fops_getdefault_path(CHAR16 *path, UINTN maxlen)
{
	if (path == NULL || maxlen == 0) return EFI_INVALID_PARAMETER;

	/*
	 * if underlying filesystem implements the call, then we call
	 * otherwise we return an empty string
	 */
	if (boot_dev->fops->getdefault_path) 
		return boot_dev->fops->getdefault_path(path, maxlen);

	path[0] = CHAR_NULL;

	return EFI_SUCCESS;
}

CHAR16 *
fops_bootdev_name(VOID)
{
	return boot_dev ? boot_dev->name : L"not supported";
}

static INTN
add_dev_tab(EFI_GUID *proto, EFI_HANDLE boot_handle, UINTN size, fops_fs_glue_t glue)
{
	EFI_STATUS status;
	EFI_HANDLE *tab;
	UINTN i;
	static UINTN idx;

	if (size == 0) return 0;

	tab = (EFI_HANDLE *)alloc(size, EfiLoaderData);
	if (tab == NULL) {
		ERR_PRT((L"failed to allocate handle table"));
		return -1;
	}

	Memset(tab, 0, size);

	/*
	 * get the actual device handles now
	 */
	status = BS->LocateHandle (ByProtocol, proto, NULL, &size, tab);
	if (status != EFI_SUCCESS) {
		ERR_PRT((L"failed to get handles for proto %g size=%d: %r", proto, size, status));
		free(tab);
		return -1;
	}
	size /= sizeof(EFI_HANDLE);

	for(i=0; i < size; i++) {
		dev_tab[idx].dev  = tab[i];
		dev_tab[idx].fops = glue_filesystem(proto, tab[i], glue);

		if (tab[i] == boot_handle) boot_dev = dev_tab+idx;

		/* count the ones we recognized */
		if (dev_tab[idx].fops) ndev_boot++;

		/* assign a generic name for now */
		dev_tab[idx].name[0] = L'd';
		dev_tab[idx].name[1] = L'e';
		dev_tab[idx].name[2] = L'v';
		dev_tab[idx].name[3] = L'0' + idx/100;
		dev_tab[idx].name[4] = L'0' + (idx%100)/10;
		dev_tab[idx].name[5] = L'0' + (idx%100) % 10;
		dev_tab[idx].name[6] = CHAR_NULL;
 
#ifdef ELILO_DEBUG
		if (elilo_opt.debug) {
			EFI_DEVICE_PATH *dp;
			CHAR16 *str, *str2;
				
			str = NULL;
			dp = DevicePathFromHandle(dev_tab[idx].dev);
			if (dp)	str = DevicePathToStr(dp);

			str2 = str == NULL ? L"Unknown" : str;

			DBG_PRT((L"%s : %-8s : %s", dev_tab[idx].name,
				(dev_tab[idx].fops ? dev_tab[idx].fops->name: L"N/A"), str2));

			if (str) FreePool(str);
		}
#endif

		idx++;
	}
	free(tab);

	/* remember actual number of bootable devices */
	ndev = idx;

	return 0;
}

static INTN
probe_devname_schemes(device_t *dev_tab, INTN ndev)
{
	devname_scheme_t **p;

	for (p = devname_scheme_tab; *p ; p++) {
		if ((*p)->install_scheme(dev_tab, ndev) == 0) goto found;
	}
	ERR_PRT((L"No devname schemes worked, using builtin\n"));
	return -1;
found:
	VERB_PRT(3, Print(L"devname scheme: %s\n", (*p)->name));
	current_devname_scheme = *p;
	return 0;
}

static INTN
find_filesystems(EFI_HANDLE boot_handle)
{
	UINTN size, total = 0;
	fileops_fs_t **fs;

	/*
	 * 1st pass, figure out how big a table we need
	 */
	for(fs = fs_tab; *fs; fs++) {
		size = 0;
		BS->LocateHandle (ByProtocol, &(*fs)->proto, NULL, &size, NULL);
		total += size;
	}
	if (total == 0) {
		ERR_PRT((L"No useable filesystem found"));
		return -1;
	}
	total /= sizeof(EFI_HANDLE);
	DBG_PRT((L"found %d filesystems", total));

	dev_tab = (device_t *)alloc(total*sizeof(device_t), EfiLoaderData);
	if (dev_tab == NULL) {
		ERR_PRT((L"failed to allocate handle table"));
		return -1;
	}

	Memset(dev_tab, 0, total*sizeof(device_t));

	/* 
	 * do a 2nd pass to initialize the table now
	 */
	for(fs = fs_tab; *fs; fs++) {
		size = 0;

		BS->LocateHandle (ByProtocol, &(*fs)->proto, NULL, &size, NULL);
		if (size == 0) continue;

		add_dev_tab(&(*fs)->proto, boot_handle, size, (*fs)->glue);
	}
	probe_devname_schemes(dev_tab, ndev);

	return 0;
}

static INTN
fileops_init(VOID)
{
	UINTN i;

	for (i=0; i < FILEOPS_FD_MAX-1; i++) {
		fd_tab[i].next = &fd_tab[i+1];
	}
	fd_tab[i].next = NULL;
	free_fd	       = fd_tab;

	return 0;
}

/*
 * both functions will go away once we go with boottime drivers
 */
static INTN
install_filesystems(VOID)
{
	fileops_fs_t **fs;

	for(fs = fs_tab; *fs; fs++) (*fs)->install();

	return 0;
}

static INTN
uninstall_filesystems(VOID)
{
	fileops_fs_t **fs;

	for(fs = fs_tab; *fs; fs++) (*fs)->uninstall();

	return 0;
}

INTN
init_devices(EFI_HANDLE boot_handle)
{
	/* simulate driver installation */
	install_filesystems();

	/*
	 * now let's do our part
	 */
	fileops_init();

	return find_filesystems(boot_handle);
}

INTN
close_devices(VOID)
{
	INTN i;

	for(i=0; i < FILEOPS_FD_MAX; i++) {
		fops_close(i);
	}
	free(dev_tab);

	/*
	 * simulate driver removal
	 */
	uninstall_filesystems();

	return 0;
}

VOID
print_devices(VOID)
{
	UINTN idx;
	EFI_DEVICE_PATH *dp;
	CHAR16 *str, *str2;

	for(idx=0; idx< ndev; idx++) {
		str = NULL;

		dp = DevicePathFromHandle(dev_tab[idx].dev);
		if (dp)	str = DevicePathToStr(dp);

		str2 = str == NULL ? L"Unknown" : str;

		Print(L"%8s : %-6s : %s\n", dev_tab[idx].name,
				(dev_tab[idx].fops ? dev_tab[idx].fops->name: L"N/A"), str2);

		if (str) FreePool(str);
	}
	Print(L"%d devices available for booting\n", ndev_boot);

	if (boot_dev == NULL) {
		Print(L"boot device not detected\n");
	} else {
		Print(L"boot device %s: %s\n", boot_dev->name,
				(boot_dev->fops ? boot_dev->fops->name : L"No file access"));
	}
}
