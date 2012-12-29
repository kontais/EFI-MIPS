/*-
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Portions copyright (c) 1999, 2000
 * Intel Corporation.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 * 
 *    This product includes software developed by the University of
 *    California, Berkeley, Intel Corporation, and its contributors.
 * 
 * 4. Neither the name of University, Intel Corporation, or their respective
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS, INTEL CORPORATION AND
 * CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
 * BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS,
 * INTEL CORPORATION OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef lint
#if 0
static char sccsid[] = "@(#)hexsyntax.c	8.2 (Berkeley) 5/4/95";
#endif
static const char rcsid[] =
	"$Id: hexsyntax.c,v 1.1.1.1 2006/05/30 06:01:28 hhzhou Exp $";
#endif /* not lint */

#include <atk_libc.h>
#include <sys/types.h>

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include "hexdump.h"

off_t skip;				/* bytes to skip */

void
newsyntax(argcp, argvp)
	int *argcp;
	char ***argvp;
{
	extern enum _vflag vflag;
	extern FS *fshead;
	extern int length;
	int ch, argc, argcount;
	char *p, **argv;


        argc = *argcp;
        argcount = *argcp;
	argv = *argvp;
	if ((p = rindex(argv[0], 'h')) != NULL &&
	    strcmp(p, "hd") == 0) {
		/* "Canonical" format, implies -C. */
		add("\"%08.8_Ax\n\"");
		add("\"%08.8_ax  \" 8/1 \"%02x \" \"  \" 8/1 \"%02x \" ");
		add("\"  |\" 16/1 \"%_p\" \"|\\n\"");
	}
	while ((ch = getopt(argc, argv, "bcCde:f:n:os:vx")) != -1)
		switch (ch) {
		case 'b':
			add("\"%07.7_Ax\n\"");
			add("\"%07.7_ax \" 16/1 \"%03o \" \"\\n\"");
                        --argcount;
			break;
		case 'c':
			add("\"%07.7_Ax\n\"");
			add("\"%07.7_ax \" 16/1 \"%3_c \" \"\\n\"");
                        --argcount;
			break;
		case 'C':
			add("\"%08.8_Ax\n\"");
			add("\"%08.8_ax  \" 8/1 \"%02x \" \"  \" 8/1 \"%02x \" ");
			add("\"  |\" 16/1 \"%_p\" \"|\\n\"");
                        --argcount;
			break;
		case 'd':
			add("\"%07.7_Ax\n\"");
			add("\"%07.7_ax \" 8/2 \"  %05u \" \"\\n\"");
                        --argcount;
			break;
		case 'e':
			add(optarg);
                        --argcount;
			break;
		case 'f':
			addfile(optarg);
                        --argcount;
			break;
		case 'n':
			if ((length = atoi(optarg)) < 0)
				errx(1, "%s: bad length value", optarg);
                        --argcount;
			break;
		case 'o':
			add("\"%07.7_Ax\n\"");
			add("\"%07.7_ax \" 8/2 \" %06o \" \"\\n\"");
                        --argcount;
			break;
		case 's':
			if ((skip = strtol(optarg, &p, 0)) < 0)
				errx(1, "%s: bad skip value", optarg);
			switch(*p) {
			case 'b':
				skip = LIBC_MultU64x32(skip, 512);
				break;
			case 'k':
				skip = LIBC_MultU64x32(skip,  1024);
				break;
			case 'm':
				skip = LIBC_MultU64x32(skip, 1048576);
				break;
			}
                        --argcount;
			break;
		case 'v':
			vflag = ALL;
                        --argcount;
			break;
		case 'x':
			add("\"%07.7_Ax\n\"");
			add("\"%07.7_ax \" 8/2 \"   %04x \" \"\\n\"");
                        --argcount;
			break;
		case '?':
			usage();
		}

	if (!fshead) {
		add("\"%07.7_Ax\n\"");
		add("\"%07.7_ax \" 8/2 \"%04x \" \"\\n\"");
	}

        *argcp = argcount;
	*argvp += optind;


}

void
usage()
{
	(void)fprintf(stderr, "%s\n%s\n%s\n%s\n",
"usage: hexdump [-bcCdovx] [-e fmt] [-f fmt_file] [-n length]",
"               [-s skip] [file ...]",
"       hd      [-bcdovx]  [-e fmt] [-f fmt_file] [-n length]",
"               [-s skip] [file ...]");
	exit(1);
}
