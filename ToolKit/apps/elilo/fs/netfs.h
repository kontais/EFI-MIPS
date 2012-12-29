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

#ifndef __NETLFS_H__
#define __NETLFS_H__

#include <efi.h>
#include <efilib.h>

#define NETFS_BOOTFILE_MAXLEN	256

typedef struct {
	EFI_IP_ADDRESS cln_ipaddr;
	EFI_IP_ADDRESS srv_ipaddr;
	EFI_IP_ADDRESS netmask;
	EFI_IP_ADDRESS gw_ipaddr;
	UINT8		hw_addr[16];
	CHAR16		hostname[255];	/* 255 limitation of DHCP protocol */
	CHAR16		domainame[255];	/* 255 limitation of DHCP protocol */
	CHAR16		bootfile[NETFS_BOOTFILE_MAXLEN];  /* name of file downloaded (BOOTP/DHCP) */
	BOOLEAN		using_pxe;
	BOOLEAN		started;
	BOOLEAN		using_ipv6;
} netfs_info_t;


INTERFACE_DECL(_netfs_interface_t);

typedef struct _netfs_interface_t {
	EFI_STATUS (*netfs_name)(struct _netfs_interface_t *this, CHAR16 *name, UINTN maxlen);
	EFI_STATUS (*netfs_open)(struct _netfs_interface_t *this, CHAR16 *name, UINTN *fd);
	EFI_STATUS (*netfs_read)(struct _netfs_interface_t *this, UINTN fd, VOID *buf, UINTN *size);
	EFI_STATUS (*netfs_close)(struct _netfs_interface_t *this, UINTN fd);
	EFI_STATUS (*netfs_infosize)(struct _netfs_interface_t *this, UINTN fd, UINT64 *size);
	EFI_STATUS (*netfs_seek)(struct _netfs_interface_t *this, UINTN fd, UINT64 newpos);
	EFI_STATUS (*netfs_query_layer)(struct _netfs_interface_t *this, UINT16 server_type, UINT16 layer, UINTN maxlen, CHAR16 *str);
	EFI_STATUS (*netfs_getinfo)(struct _netfs_interface_t *this, netfs_info_t *info);
} netfs_interface_t;

#define NETFS_PROTOCOL \
    { 0x6746de4f, 0xcc1e, 0x4c5f, {0xb7, 0xfb, 0x85, 0x6a, 0x5d, 0x69, 0x0f, 0x06} }

extern EFI_STATUS netfs_install(VOID);
extern EFI_STATUS netfs_uninstall(VOID);

#endif /* __NETFS_H__ */
