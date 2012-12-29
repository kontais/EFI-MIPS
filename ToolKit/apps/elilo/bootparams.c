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

/*
 * Initialize the generic part of the boot parameters and call the architecture specific
 * subroutine.
 * Output:
 * 	cookie: the memory map cookie to use with ExitBootServices()
 * Return:
 * 	NULL: if an error occured
 * 	bp  : the address of the bootparams otherwise (opaque type)
 */
VOID *
create_boot_params(CHAR16 *args, memdesc_t *initrd, memdesc_t *vmcode, UINTN *cookie)
{
/* 
 * XXX: need cleanup
 * See ia32 code for explanation on why it is so big.
 */
#define	BOOT_PARAM_MEMSIZE	16384 	
	UINTN bpsize, cmdline_size;
	boot_params_t *bp;
	CHAR8 *cp;
	CHAR16 ch;

	/*
	 * Allocate runtime services memory to hold memory descriptor table and
	 * command line arguments and fetch memory map:
	 *
	 * arg_size = number of character in ASCII form
	 */
	cmdline_size = StrLen(args) + 1;
	bpsize       = sizeof(boot_params_t) + cmdline_size;

	if (bpsize > BOOT_PARAM_MEMSIZE) {
		ERR_PRT((L"BOOT_PARAM_MEMSIZE too small, need at least %d bytes", bpsize));
		return NULL;
	}


	/*
	 * Allocate memory for boot parameters.
	 * This CANNOT be EfiLoaderData or EfiLoaderCode as the kernel
	 * frees this region when initializing.
	 * FIXME:  Is this a bug?  (since the memory type *is* EfiLoaderData)
	 */

	bp = (boot_params_t *)alloc(BOOT_PARAM_MEMSIZE, EfiLoaderData);
	if (bp == NULL) {
		ERR_PRT((L"can't allocate boot params"));
		return 0;
	}

	VERB_PRT(3, Print(L"boot params @ " PTR_FMT "\n", bp));

/* XXX: need to fix this for 3.5 */
#ifdef CONFIG_ia64
       cp = ((CHAR8 *)bp) + BOOT_PARAM_MEMSIZE - cmdline_size;
#elif defined CONFIG_Mips32 || CONFIG_x86_64
       cp = ((CHAR8 *)bp) + BOOT_PARAM_MEMSIZE - 2048;
#endif

	/* 
	 * clear entire buffer. The boot param structure is bigger than
	 * needs be but this allows the kernel bootparam structure to grow
	 * (up to BOOT_PARAM_MEMSIZE) without having to worry about fixing the bootloader.
	 * By convention between the laoder and the kernel, the value 0 means 
	 * don't care or not set.
	 */
	Memset(bp, 0, BOOT_PARAM_MEMSIZE);

	U2ascii(args, cp, cmdline_size);

	if (sysdeps_create_boot_params(bp, cp, initrd, vmcode, cookie) == -1) return 0;

	/*
	 * Convert kernel command line args from UNICODE to ASCII and put them where
	 * the kernel expects them:
	 */
	while (1) {
		ch = *args++;
		if (!ch) break;
		*cp++ = ch;
	}
	*cp++ = '\0';

	return bp;
}

VOID
free_boot_params(VOID *bp)
{
	boot_params_t *real_bp = (boot_params_t *)bp;

	sysdeps_free_boot_params(real_bp);

	free(real_bp);
}

