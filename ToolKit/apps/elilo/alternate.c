/*
 *  Copyright (C) 2001-2003 Hewlett-Packard Co.
 *	Contributed by Stephane Eranian <eranian@hpl.hp.com>
 *	Contributed by Fenghua Yu <fenghua.yu@intel.com>
 *	Contributed by Bibo Mao <bibo.mao@intel.com>
 *	Contributed by Chandramouli Narayanan<mouli@linux.intel.com>
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

#define ELILO_ALTK_VAR	L"EliloAlt"

/*
 * we use a NULL GUID for now because it is more convenient
 */
static EFI_GUID altk_guid={0,};

/*
 * Check for the existence of an EFI variable named EliloAlt.
 * It contains the name of an alternate kernel (and possible options) 
 * to boot ONLY once. 
 * The variable is then deleted. This ensures that next reboot won't
 * pick it up. 
 *
 * The content of the variable is assumed to be Unicode string. It is
 * preferrably NULL terminated but the code can deal with this as well.
 *
 * The size of the buffer MUST be expressed in bytes (not CHAR16).
 *
 * Return:
 *	- 0 if successful
 *	- -1 in case nothing happened (no variable found)
 *	  Please note that no fatal error is reported by this function
 */
INTN
alternate_kernel(CHAR16 *buffer, UINTN size)
{
	EFI_STATUS status;
	INTN ret = -1;

	/*
	 * size of the buffer is expressed in bytes
	 *
	 * we reserve one Unicode for CHAR_NULL (forced), so
	 * the buffer must be at least 2 more bytes to accomodate one Unicode
	 */
	if (size < 4) return -1;

	/*
	 * reserve for CHAR_NULL
	 */
	size-=2;

	/*
	 * Look if the variable exists. 
	 * We may fail because:
	 * 	- the variable does not exist
	 * 	- our buffer size is too small.
	 */
	status = RT->GetVariable (ELILO_ALTK_VAR, &altk_guid, NULL, &size, buffer);
	if (EFI_ERROR(status)) {
		DBG_PRT((L"cannot access variable %s: %r", ELILO_ALTK_VAR, status));

		/* if buffer is too small, erase variable because it's unuseable */
		if (status == EFI_BUFFER_TOO_SMALL) goto delete_var;

		return -1;
	}

	/*
	 * sanity check
	 *
	 * verify that size (expressed in bytes) is multiple of 2. If 
	 * not then it can't possibly be representing a UNICODE string
	 * (2bytes/character).
	 *
	 * We also consider empty as useless (2nd test).
	 */
	if (size & 0x1) {
		Print(L"invalid content for %s variable, erasing variable\n", ELILO_ALTK_VAR);
		goto delete_var;
	}

	/* We also consider empty as useless (2nd test) */
	if (size == 2) goto delete_var;

	buffer[size] = CHAR_NULL;

	VERB_PRT(2, Print(L"found alternate variable %s : %s\n", ELILO_ALTK_VAR, buffer));

	ret = 0;
delete_var:
	status = RT->SetVariable (ELILO_ALTK_VAR, &altk_guid, 0, 0, NULL);
	if (EFI_ERROR(status)) {
		ERR_PRT((L"cannot erase variable %s", ELILO_ALTK_VAR));
	}
	return ret;
}
