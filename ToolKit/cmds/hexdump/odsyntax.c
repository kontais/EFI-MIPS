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
static char sccsid[] = "@(#)odsyntax.c	8.2 (Berkeley) 5/4/95";
#endif
static const char rcsid[] =
	"$Id: odsyntax.c,v 1.1.1.1 2006/05/30 06:01:29 hhzhou Exp $";
#endif /* not lint */

#include <atk_libc.h>
#include <sys/types.h>

#include <ctype.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "hexdump.h"

int deprecated;

static void odoffset __P((int, char ***));
static void odprecede __P((void));

void
oldsyntax(argcp, argvp)
	int *argcp;
	char ***argvp;
{
	extern enum _vflag vflag;
	extern FS *fshead;
	int ch, argc, argcount;
	char **argv;

	deprecated = 1;
        argc = *argcp;
        argcount = *argcp;
	argv = *argvp;
	while ((ch = getopt(argc, argv, "aBbcDdeFfHhIiLlOoPpswvXx")) != -1)
		switch (ch) {
		case 'a':
			odprecede();
			add("16/1 \"%3_u \" \"\\n\"");
                        --argcount;
			break;
		case 'B':
		case 'o':
			odprecede();
			add("8/2 \" %06o \" \"\\n\"");
                        --argcount;
			break;
		case 'b':
			odprecede();
			add("16/1 \"%03o \" \"\\n\"");
                        --argcount;
			break;
		case 'c':
			odprecede();
			add("16/1 \"%3_c \" \"\\n\"");
                        --argcount;
			break;
		case 'd':
			odprecede();
			add("8/2 \"  %05u \" \"\\n\"");
                        --argcount;
			break;
		case 'D':
			odprecede();
			add("4/4 \"     %010u \" \"\\n\"");
                        --argcount;
			break;
		case 'e':		/* undocumented in od */
		case 'F':
			odprecede();
			add("2/8 \"          %21.14e \" \"\\n\"");
                        --argcount;
			break;

		case 'f':
			odprecede();
			add("4/4 \" %14.7e \" \"\\n\"");
                        --argcount;
			break;
		case 'H':
		case 'X':
			odprecede();
			add("4/4 \"       %08x \" \"\\n\"");
                        --argcount;
			break;
		case 'h':
		case 'x':
			odprecede();
			add("8/2 \"   %04x \" \"\\n\"");
                        --argcount;
			break;
		case 'I':
		case 'L':
		case 'l':
			odprecede();
			add("4/4 \"    %11d \" \"\\n\"");
                        --argcount;
			break;
		case 'i':
			odprecede();
			add("8/2 \" %6d \" \"\\n\"");
                        --argcount;
			break;
		case 'O':
			odprecede();
			add("4/4 \"    %011o \" \"\\n\"");
                        --argcount;
			break;
		case 'v':
			vflag = ALL;
                        --argcount;
			break;
		case 'P':
		case 'p':
		case 's':
		case 'w':
		case '?':
		default:
			warnx("od(1) has been deprecated for hexdump(1)");
			if (ch != '?')
				warnx("hexdump(1) compatibility doesn't support the -%c option%s",
				    ch, ch == 's' ? "; see strings(1)" : "");
			usage();
		}

	if (!fshead) {
		add("\"%07.7_Ao\n\"");
		add("\"%07.7_ao  \" 8/2 \"%06o \" \"\\n\"");
	}

	argc -= optind;
	*argvp += optind;

	if (argc)
		odoffset(argc, argvp);
        *argcp = argcount;
}

static void
odoffset(argc, argvp)
	int argc;
	char ***argvp;
{
	extern off_t skip;
	unsigned char *p, *num, *end;
	int base;

	/*
	 * The offset syntax of od(1) was genuinely bizarre.  First, if
	 * it started with a plus it had to be an offset.  Otherwise, if
	 * there were at least two arguments, a number or lower-case 'x'
	 * followed by a number makes it an offset.  By default it was
	 * octal; if it started with 'x' or '0x' it was hex.  If it ended
	 * in a '.', it was decimal.  If a 'b' or 'B' was appended, it
	 * multiplied the number by 512 or 1024 byte units.  There was
	 * no way to assign a block count to a hex offset.
	 *
	 * We assume it's a file if the offset is bad.
	 */
	p = (unsigned char*)(argc == 1 ? (*argvp)[0] : (*argvp)[1]);

	if (*p != '+' && (argc < 2 ||
	    (!isdigit(p[0]) && (p[0] != 'x' || !isxdigit(p[1])))))
		return;

	base = 0;
	/*
	 * skip over leading '+', 'x[0-9a-fA-f]' or '0x', and
	 * set base.
	 */
	if (p[0] == '+')
		++p;
	if (p[0] == 'x' && isxdigit(p[1])) {
		++p;
		base = 16;
	} else if (p[0] == '0' && p[1] == 'x') {
		p += 2;
		base = 16;
	}

	/* skip over the number */
	if (base == 16)
		for (num = p; isxdigit(*p); ++p);
	else
		for (num = p; isdigit(*p); ++p);

	/* check for no number */
	if (num == p)
		return;

	/* if terminates with a '.', base is decimal */
	if (*p == '.') {
		if (base)
			return;
		base = 10;
	}

	skip = strtol(num, (char **)&end, base ? base : 8);

	/* if end isn't the same as p, we got a non-octal digit */
	if (end != p) {
		skip = 0;
		return;
	}

	if (*p)
		if (*p == 'B') {
			skip = LIBC_MultU64x32(skip, 1024);
			++p;
		} else if (*p == 'b') {
			skip = LIBC_MultU64x32(skip, 512);
			++p;
		}

	if (*p) {
		skip = 0;
		return;
	}

	/*
	 * If the offset uses a non-octal base, the base of the offset
	 * is changed as well.  This isn't pretty, but it's easy.
	 */
#define	TYPE_OFFSET	7
	if (base == 16) {
		fshead->nextfu->fmt[TYPE_OFFSET] = 'x';
		fshead->nextfs->nextfu->fmt[TYPE_OFFSET] = 'x';
	} else if (base == 10) {
		fshead->nextfu->fmt[TYPE_OFFSET] = 'd';
		fshead->nextfs->nextfu->fmt[TYPE_OFFSET] = 'd';
	}

	/* Terminate file list. */
	(*argvp)[1] = NULL;
}

static void
odprecede()
{
	static int first = 1;

	if (first) {
		first = 0;
		add("\"%07.7_Ao\n\"");
		add("\"%07.7_ao  \"");
	} else
		add("\"         \"");
}
