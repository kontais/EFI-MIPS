/*
 *  Copyright (C) 2001-2003 Hewlett-Packard Co.
 *	Contributed by Stephane Eranian <eranian@hpl.hp.com>
 *	Contributed by Chris Ahna <christopher.j.ahna@intel.com>
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
#include "config.h"
#include "private.h"

typedef struct {
	UINTN legacy_free_boot;
} ia32_global_config_t;


static ia32_global_config_t ia32_gconf;

static config_option_t sysdeps_global_options[]={
	{OPT_BOOL,	OPT_GLOBAL, L"legacy-free",	NULL,	NULL,	&ia32_gconf.legacy_free_boot}
};


/*
 * IA-32 operations that need to be done only once and just before 
 * entering the main loop of the loader
 * Return:
 * 	 0 if sucessful
 * 	-1 otherwise (will abort execution)
 */
INTN
sysdeps_preloop_actions(EFI_HANDLE dev, CHAR16 **argv, INTN argc, INTN index, EFI_HANDLE image)
{
	return 0;
}
	
#define IA32_CMDLINE_OPTIONS	L""

CHAR16 *
sysdeps_get_cmdline_opts(VOID)
{
	return IA32_CMDLINE_OPTIONS;
}

INTN
sysdeps_getopt(INTN c, INTN optind, CHAR16 *optarg)
{
	return -1;
}

VOID
sysdeps_print_cmdline_opts(VOID)
{
}


INTN
ia32_use_legacy_free_boot(VOID)
{
	return ia32_gconf.legacy_free_boot ? 1 : 0;
}

INTN
sysdeps_register_options(VOID)
{
	INTN ret;

	ret = register_config_options(sysdeps_global_options, 
				      sizeof(sysdeps_global_options)/sizeof(config_option_t),
				      OPTIONS_GROUP_GLOBAL);
#if 0
	/* no per image options yet */
	if (ret == -1 ) return ret;

	ret = register_config_options(sysdeps_image_options, 
				      sizeof(sysdeps_image_options)/sizeof(config_option_t),
				      OPTIONS_GROUP_IMAGE);
#endif

	return ret;
}
