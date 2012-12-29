/*
 *  Copyright (C) 2001-2003 Hewlett-Packard Co.
 *	Contributed by Stephane Eranian <eranian@hpl.hp.com>
 *
 *  Copyright (C) 2001 Silicon Graphics, Inc.
 *      Contributed by Brent Casavant <bcasavan@sgi.com>
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

#ifndef __ELILO_VARS_H__
#define __ELILO_VARS_H__
/*
 * This file contains the list of defined variables.
 * It is expected that every module which uses a variable add its entry
 * here.
 * The syntax for the name is: VAR_modulename_meaning L'X'
 * where:
 * 	- modulename: a string representing the module that uses the variable
 * 	- meaning   : a string representing the meaning of the variable for the module
 * 	- X         : the variable name [A-Za-z]
 */

/* from glue_netfs.c */
#define VAR_NETFS_IPADDR	L'I'	/* the IP address obtained by DHCP/PXE */
#define VAR_NETFS_NETMASK	L'M'	/* the netmask obtained by DHCP/PXE */
#define VAR_NETFS_GATEWAY	L'G'	/* the gateway obtained by DHCP/PXE */
#define VAR_NETFS_HOSTNAME	L'H'	/* the hostname obtained by DHCP/PXE */
#define VAR_NETFS_DOMAINAME	L'D'	/* the domain name obtained by DHCP/PXE */

extern INTN set_var(CHAR16 v, CHAR16 *value);
extern CHAR16 * get_var(CHAR16 v);
extern VOID print_vars(VOID);
extern INTN subst_vars(CHAR16 *in, CHAR16 *out, INTN maxlen);

#endif /* __ELILO_VARS_H__ */

