/*
 * Simplistic getopt() function for EFI
 *
 * This function provides the basic functionality of the POSIX getopt() function.
 * No long options are supported.
 *
 * This code  is meant for EFI programs and therefore deals with Unicode characters.
 *
 * Copyright (C) 2000 Hewlett-Packard Co
 * Copyright (C) 2000 Stephane Eranian <eranian@hpl.hp.com>
 *
 * ELILO is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * ELILO is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ELILO; see the file COPYING.  If not, write to the Free
 * Software Foundation, 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#include <efi.h>
#include <efilib.h>


#define DASH	(CHAR16)'-'
#define COLON	(CHAR16)':'
#define EOS	(CHAR16)'\0'
#define BADCH	(INTN)'?'
#define BADARG	(INTN)':'

extern CHAR16 * StrChr(IN const CHAR16 *s, CHAR16 c);

CHAR16 *Optarg;
INTN    Optind = 1;
INTN    Optopt;

/*
 * This simple version of getopt supports:
 * 	- option with no argument (no :)
 * 	- option with REQUIRED argument (single :)
 * it does not support:
 *	- long options
 * 	- optional arguments to options
 * 	- optreset
 */
INTN
Getopt(INTN argc, CHAR16 *const argv[], const CHAR16 *optstring)
{
	static CHAR16 *cur_chr = NULL;
	CHAR16 *opt;

	if (Optind >= argc) { /* no option or end of argument list */
		cur_chr = NULL;
		return -1;
	}
	if (cur_chr == NULL || *cur_chr == EOS) {
		cur_chr = argv[Optind];
		if (*cur_chr++ != DASH) { /* missing DASH */
			cur_chr = NULL;
			return -1;
		}
		if (*cur_chr == DASH) {
			cur_chr = NULL;
			Optind++;
			return -1; /* -- case, we're done */
		}
	}
	Optopt = *cur_chr++;
	opt = StrChr(optstring, Optopt);
	if (!opt) {
		Print(L"%s: illegal option -- %c\n", argv[0], Optopt);
		if (*cur_chr == EOS) Optind++;
		return BADCH;
	} 
	if (*(opt+1) != COLON) {
		Optarg = NULL;
		if (*cur_chr == EOS) Optind++;
	} else {
		if (*cur_chr) {
			Optarg = cur_chr;
		} else if ( ++Optind >= argc ) {
			   Print(L"%s: option `%s' requires an argument\n", argv[0], argv[Optind-1]),
			   cur_chr = NULL;
			   return BADARG;
		} else {
			Optarg = argv[Optind];
		}
		Optind++;
	}
	return Optopt;
}
