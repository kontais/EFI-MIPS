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

#ifdef CONFIG_CHOOSER_SIMPLE
#include "choosers/simple.h"
#endif

#ifdef CONFIG_CHOOSER_TEXTMENU
#include "choosers/textmenu.h"
#endif

static chooser_t *choosers_tab[]={
#ifdef CONFIG_CHOOSER_SIMPLE
	&simple_chooser,
#endif
#ifdef CONFIG_CHOOSER_TEXTMENU
	&textmenu_chooser,
#endif
	NULL
};
/*
 * The intent of this module is to provide a mechanism by which alternate
 * choosers can be installed. Developers can write new choosers and
 * add them to the list. They will be probe and the best match
 * will be started first. It should be possible to switch to another
 * chooser using a key combination. There is a default simple text-based
 * chooser that must always be present.
 *
 * Currently you can specify a chooser via "-c name" when you invoke elilo,
 * or via "chooser=name" in the config file.  If the specified chooser
 * probes ok it will be selected, otherwise the first one that probes ok
 * will be used.
 *
 * XXX: at this time, not all chooser functionalities are implemented.
 *
 */
chooser_func_t *kernel_chooser;

INTN
init_chooser(EFI_HANDLE dev)
{
	chooser_t **p;
	CHAR16 *chooser_name = L"none";

	kernel_chooser = NULL;

	for (p=choosers_tab; *p; p++) {

		VERB_PRT(4, Print(L"trying chooser %s\n", (*p)->chooser_name));

		if ((*p)->chooser_probe(dev) == 0) {
			/*
			 * if that's the one we asked for, then go for it
			 */
			if (!StrCmp(elilo_opt.chooser, (*p)->chooser_name)) {
				kernel_chooser = (*p)->chooser_func;
				chooser_name = (*p)->chooser_name;
				break;
			}

			if (kernel_chooser == NULL) {
				kernel_chooser = (*p)->chooser_func;
				chooser_name = (*p)->chooser_name;
			}
		}
	}

	if (kernel_chooser) {
		VERB_PRT(2, Print(L"selected chooser %s\n", chooser_name));
		return 0;
	}

	ERR_PRT((L"No chooser selected. Impossible to proceed"));
	return -1;
}

INTN
exist_chooser(CHAR16 *name)
{
	chooser_t **p;

	for (p=choosers_tab; *p; p++) {
		if (!StrCmp(name, (*p)->chooser_name)) return 0;
	}
	return -1;
}

