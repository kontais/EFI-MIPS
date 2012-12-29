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

#include "glue_netfs.h"
#include "fs/netfs.h"
#include "strops.h"

#include "elilo.h"
#include "vars.h"

/*
 * disable this if you only want the default config file (elilo.conf)
 * and not the ip-address based first file attempt
 */

static INTN glue(fileops_t *this, VOID *intf);

/* object exported to fileops */
fileops_fs_t netfs_glue = { NETFS_PROTOCOL  , glue, netfs_install, netfs_uninstall};
	

#define NETFS_DEFAULT_KERNEL		L"vmlinux"
#define NETFS_DEFAULT_CONFIG		L"elilo.conf"
#define NETFS_DEFAULT_SERVER_TYPE	EFI_PXE_BASE_CODE_BOOT_TYPE_REDHAT_BOOT


static CHAR16 netfs_default_path[FILENAME_MAXLEN];


/*
 * Pxe Discovery protocol layers
 * Layer 0 is used to download the boot loader
 */
#define NETFS_CONFIG_LAYER	1
#define NETFS_KERNEL_LAYER	2

static CHAR16 *hexa=L"0123456789ABCDEF";

static VOID
convert_mac2hex(UINT8 *hw_addr,INTN l, CHAR16 *str)
{
 	UINTN i;

	for (i=0 ; i < l; i++) {
		str[3*i] = hexa[(hw_addr[i] & 0xf0)>>4];
		str[3*i+1] = hexa[hw_addr[i] & 0x0f];
		str[3*i+2] = ':';
	}
	str[3*l-1]='\0';
}

static VOID
convert_ip2hex(UINT8 *ip, INTN l, CHAR16 *str)
{
	UINTN i;

	for(i=0; i < l; i++) {
		str[2*i]   = hexa[(ip[i] & 0xf0)>>4];
		str[2*i+1] = hexa[ip[i] & 0x0f];
	}
}

static VOID
convert_ip2decstr(UINT8 *ip, INTN l, CHAR16 *str)
{
	UINTN i, j;
	UINTN v, val;

	for(i=0, j=0; i < l; i++) {
		val = ip[i];
		v = val / 100;
		if (v) {
			str[j++] = L'0'+v;
		}
		val = val % 100;
		v = val / 10;
		if (v || ip[i] >= 100) {
			str[j++] = L'0'+v;
		}

		v = val % 10;
		str[j++] = L'0'+v;
		if (i < l-1) str[j++] = L'.';
	}
	str[j] = CHAR_NULL;
}

static int
netfs_set_default_path(netfs_interface_t *netfs, netfs_info_t *info)
{
 	INTN len;

 	StrnCpy(netfs_default_path, info->bootfile, FILENAME_MAXLEN);

	len = StrLen(netfs_default_path) - 1;

 	while (len >= 0) {
 		if (netfs_default_path[len] == CHAR_SLASH || netfs_default_path[len] == CHAR_BACKSLASH) break;
		len--;
 	}
 	netfs_default_path[len+1] = CHAR_NULL;
 
	DBG_PRT((L"netfs_default_path=%s\n", netfs_default_path));

 	return EFI_SUCCESS;
}

static EFI_STATUS
netfs_setdefaults(VOID *intf, config_file_t *config, CHAR16 *kname, UINTN maxlen, CHAR16 *devpath)
{
	netfs_interface_t *netfs = (netfs_interface_t *)intf;
	netfs_info_t info;
	EFI_STATUS status;
	UINT8 *ipaddr;
	UINTN m;
	CHAR16 ip_var[64], str[64];
	UINT8 *ip;

	if (config == NULL || kname == NULL || maxlen < 1) return EFI_INVALID_PARAMETER;

	netfs->netfs_getinfo(netfs, &info);

	m      = info.using_ipv6 ? 16 : 4;
	ipaddr = info.using_ipv6 ? info.cln_ipaddr.v6.Addr: info.cln_ipaddr.v4.Addr;

	convert_ip2decstr(ipaddr, m, ip_var);
	set_var(VAR_NETFS_IPADDR, ip_var);

	ip = info.using_ipv6 ? info.netmask.v6.Addr: info.netmask.v4.Addr;
	convert_ip2decstr(ip, m, str);
	set_var(VAR_NETFS_NETMASK, str);

	ip = info.using_ipv6 ? info.gw_ipaddr.v6.Addr: info.gw_ipaddr.v4.Addr;
	convert_ip2decstr(ip, m, str);
	set_var(VAR_NETFS_GATEWAY, str);

	set_var(VAR_NETFS_HOSTNAME, info.hostname);
	set_var(VAR_NETFS_DOMAINAME, info.domainame);

	if (info.using_pxe) {
		DBG_PRT((L"netfs_setdefaults: using_pxe"));

		status = netfs->netfs_query_layer(netfs, 0, NETFS_CONFIG_LAYER, maxlen, config[0].fname);
		if (EFI_ERROR(status)) {
			StrnCpy(config[0].fname, NETFS_DEFAULT_CONFIG, maxlen-1);
			config[0].fname[maxlen-1] = CHAR_NULL;
		}

		status = netfs->netfs_query_layer(netfs, 0, NETFS_KERNEL_LAYER, maxlen, kname);
		if (EFI_ERROR(status)) {
			StrnCpy(kname, NETFS_DEFAULT_KERNEL, maxlen-1);
			kname[maxlen-1] = CHAR_NULL;
		}
	} else {
#ifdef ENABLE_MACHINE_SPECIFIC_NETCONFIG

#  if defined(CONFIG_ia64)
#    define CONFIG_ARCH_EXTENSION L"-ia64.conf\0"
#    define EXTENSION_LENGTH 11
#  elif defined (CONFIG_Mips32)
#    define CONFIG_ARCH_EXTENSION L"-ia32.conf\0"
#    define EXTENSION_LENGTH 11
#  elif defined (CONFIG_x86_64)
#    define CONFIG_ARCH_EXTENSION L"-x86_64.conf\0"
#    define EXTENSION_LENGTH 13
#  else
#    error "You need to specfy your default arch config file"
#  endif            

#  define CONFIG_EXTENSION L".conf\0"

		DBG_PRT((L"netfs_setdefaults: machine specific (!using_pxe)"));
		/*
		 * will try machine/subnet specific files first.
		 * the filenames are constructed based on the IP(v4) address
		 */
		convert_ip2hex(ipaddr, m, str);
		StrnCpy(config[0].fname, str, maxlen-1);
		StrnCpy(config[0].fname+8, CONFIG_EXTENSION, 6);

		StrnCpy(config[1].fname, str, maxlen-1);
		StrnCpy(config[1].fname+6, CONFIG_ARCH_EXTENSION, EXTENSION_LENGTH);

		StrnCpy(config[2].fname, str, maxlen-1);
		StrnCpy(config[2].fname+6, CONFIG_EXTENSION, 6);

		StrnCpy(config[3].fname, str, maxlen-1);
		StrnCpy(config[3].fname+4, CONFIG_ARCH_EXTENSION, EXTENSION_LENGTH);

		StrnCpy(config[4].fname, str, maxlen-1);
		StrnCpy(config[4].fname+4, CONFIG_EXTENSION, 6);
                
		StrnCpy(config[5].fname, str, maxlen-1);
		StrnCpy(config[5].fname+2, CONFIG_ARCH_EXTENSION, EXTENSION_LENGTH);

		StrnCpy(config[6].fname, str, maxlen-1);
		StrnCpy(config[6].fname+2, CONFIG_EXTENSION, 6);

		/* use the MAC address as a possible file name as well */
		convert_mac2hex(info.hw_addr,6,str);
		StrnCpy(config[7].fname, str, maxlen-1);
		StrnCpy(config[7].fname+17, CONFIG_EXTENSION, 6);

#else
		StrnCpy(config[0].fname, NETFS_DEFAULT_CONFIG, maxlen-1);
		config[0].fname[maxlen-1] = CHAR_NULL;
#endif
		StrnCpy(kname, NETFS_DEFAULT_KERNEL, maxlen-1);
		kname[maxlen-1] = CHAR_NULL;

		/*
		 * extract bootloader path prefix to be used for 
		 * the config file (and possibly the other files we 
		 * need to download)
		 */
		netfs_set_default_path(netfs, &info);
	}
	return EFI_SUCCESS;
}

static EFI_STATUS
netfs_getdefault_path(CHAR16 *path, UINTN maxlen)
{
	if (maxlen <= StrLen(netfs_default_path)) return EFI_BUFFER_TOO_SMALL;

	StrCpy(path, netfs_default_path);
	return EFI_SUCCESS;
}


static EFI_STATUS 
glue_open(VOID *intf, CHAR16 *name, fops_fd_t *fd)
{
	netfs_interface_t *netfs = (netfs_interface_t *)intf;
	CHAR16 fullname[FILENAME_MAXLEN];

	if (name[0] != CHAR_SLASH && name[0] != CHAR_BACKSLASH && netfs_default_path[0] != CHAR_NULL) {
		if (StrLen(netfs_default_path) + StrLen(name) + 1 >= FILENAME_MAXLEN)
			return EFI_INVALID_PARAMETER;

		StrCpy(fullname, netfs_default_path);
		StrCat(fullname, name);
		name = fullname;
	}
	return netfs->netfs_open(intf, name, fd);
}

static INTN
glue(fileops_t *fp, VOID *intf)
{
	netfs_interface_t *netfs = (netfs_interface_t *)intf;

	/* record underlying interface */
	fp->intf        = intf;

	fp->open            = glue_open;
	fp->read            = (fops_read_t)netfs->netfs_read;
	fp->close           = (fops_close_t)netfs->netfs_close;
	fp->infosize        = (fops_infosize_t)netfs->netfs_infosize;
	fp->seek            = (fops_seek_t)netfs->netfs_seek;
	fp->setdefaults     = (fops_setdefaults_t)netfs_setdefaults;
	fp->getdefault_path = (fops_getdefault_path_t)netfs_getdefault_path;
	fp->intf	    = intf;

	/* fill out the name of the underlying file system */
	netfs->netfs_name(netfs, fp->name, FILEOPS_NAME_MAXLEN);

	return 0;
}
