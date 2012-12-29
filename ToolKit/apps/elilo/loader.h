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

#ifndef __LOADER_H__
#define __LOADER_H__

#include "fileops.h"

typedef struct __loader_ops_t {
	struct __loader_ops_t *next;
	CHAR16 *ld_name;
	INTN (*ld_probe)(CHAR16 *kname);
	INTN (*ld_load_kernel)(CHAR16 *kname, kdesc_t *kd);
} loader_ops_t;

extern loader_ops_t *loader_probe(CHAR16 *kname);
extern INTN loader_register(loader_ops_t *ldops);

#endif /* __LOADER_H__ */
