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

#include "elilo.h"
#include "loader.h"

extern loader_ops_t plain_loader;
extern loader_ops_t gzip_loader;

static loader_ops_t *ldops_list;

loader_ops_t *
loader_probe(CHAR16 *kname)
{
	loader_ops_t *ops;
	UINTN n = 0;

	for (ops= ldops_list; ops; ops = ops->next) {
		n++;
		VERB_PRT(3, Print(L"Probing loader: %s\n", ops->ld_name));
		if (ops->ld_probe(kname) == 0) {
			return ops;
		}
	}
	if (!n) {
		ERR_PRT((L"No loaders registered"));
	}
	return NULL;
}

INTN
loader_register(loader_ops_t *ldops)
{
	if (ldops == NULL) return -1;

	/* cheap sanity check */
	if (ldops->next) {
		ERR_PRT((L"loader %s is already registered", ldops->ld_name));
		return -1;
	}

	ldops->next = ldops_list;
	ldops_list  = ldops;

	VERB_PRT(3, Print(L"New loader registered: %s\n", ldops->ld_name));

	return 0;
}

