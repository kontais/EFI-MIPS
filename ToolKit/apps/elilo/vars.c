/*
 *  Copyright (C) 2001-2003 Hewlett-Packard Co.
 *	Contributed by Stephane Eranian <eranian@hpl.hp.com>
 *
 *  Copyright (C) 2001 Silicon Graphics, Inc.
 *	Contributed by Brent Casavant <bcasavan@sgi.com>
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

/*
 * A variable name is 1 character long and case sensitive. So 
 * we actually have 52 (26*2) possible variables.
 */
#define MAX_VARIABLES		(26<<1)
#define MAX_VARIABLE_LENGTH	128
#define VAR_IDX(a)		(((a) >= 'a' && (a) <= 'z') ? 26-'a'+(a) : (a)-'A')
#define IDX_VAR(i)		((i) < 26 ? 'A'+(i) : 'a'+ ((i)-26))

typedef struct {
	CHAR16	value[MAX_VARIABLE_LENGTH];
} elilo_var_t;

static elilo_var_t vars[MAX_VARIABLES];	/* set of variables */

INTN
set_var(CHAR16 v, CHAR16 *value)
{
	/* invalid variable name */
	if (v < 'A' || (v > 'Z' && v < 'a') || v > 'z') return -1;

	StrCpy(vars[VAR_IDX(v)].value, value);
	return 0;
}

CHAR16 *
get_var(CHAR16 v)
{
	/* invalid variable name */
	if (v < L'A' || (v > L'Z' && v < L'a') || v > L'z') return NULL;

	return vars[VAR_IDX(v)].value;
}


VOID
print_vars(VOID)
{
	INTN i;
	UINTN cnt = 0;

	for(i=0; i < MAX_VARIABLES; i++) {
		if (vars[i].value[0]) {
			cnt++;
			Print(L"%c = \"%s\"\n", IDX_VAR(i), vars[i].value);
		}
	}
	if (cnt == 0) Print(L"no variable defined\n");
}


INTN
subst_vars(CHAR16 *in, CHAR16 *out, INTN maxlen)
{
	/* 
	 * we cannot use \\ for the despecialization character because
	 * it is also used as a path separator in EFI.
	 */
#define DSPEC_CHAR	L'&'
	INTN i, l, j, cnt;
	INTN m = 0, d = 0;
	CHAR16 *val;

	if (in == NULL || out == NULL || maxlen <= 1) return -1;

	l = StrLen(in);

	maxlen--;

	for (i=0, j=0;i < l; i++) {
		cnt = 1;
		val = in+i;

		if (*val == DSPEC_CHAR  && d == 0) {
			d = 1; 
			continue;
		}
		if(m == 1) {
		    m = 0;
		    val = get_var(*val);

		    if (val == NULL) continue;

		    cnt = StrLen(val);

		} else if (*val == L'%' && d == 0) {
			m = 1;
			continue;
		}
		d = 0;
		while (j < maxlen && cnt) {
			out[j++] = *val++;
			cnt--;
		}
		if (j == maxlen) break;
	}
	out[j] = CHAR_NULL;

	return 0;
}
