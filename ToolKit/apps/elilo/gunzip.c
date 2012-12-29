/*
 *  Copyright (C) 2005 Hewlett-Packard Development Company, L.P.
 *	Contributed by Alex Williamson <alex.williamson@hp.com>
 *
 *  Copyright (C) 2001-2003 Hewlett-Packard Co.
 *	Contributed by Stephane Eranian <eranian@hpl.hp.com>
 *
 *  Copyright (C) 2001 Silicon Graphics, Inc.
 *      Contributed by Brent Casavant <bcasavan@sgi.com>
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

#include "gzip.h"

#define LD_NAME L"gunzip"

#define memzero(s, n)	Memset((VOID *)(s), 0, (n))
#define memcpy(a,b,n)	Memcpy((VOID *)(a),(b),(n))

/* size of output buffer */
#define WSIZE 0x8000		/* Window size must be at least 32k, */
				/* and a power of two */
/*
 * gzip declarations
 */
#define OF(args)  args
#define FUNC_STATIC static

typedef unsigned char  uch;
typedef unsigned short ush;
typedef unsigned long  ulg;

/*
 * static parameters to gzip helper functions
 * we cannot use paramters because API was not
 * designed that way
 */
static uch *inbuf;		/* input buffer (compressed data) */
static uch *window;    		/* output buffer (uncompressed data) */

static VOID *outbuf;
unsigned char *outptr;
static unsigned long outsize;

static unsigned inptr  = 0;   /* index of next byte to be processed in inbuf */
static unsigned outcnt = 0;  /* bytes in output buffer */

#define get_byte()  inbuf[inptr++]

/* Diagnostic functions */
#ifdef INFLATE_DEBUG
#  define Assert(cond,msg) {if(!(cond)) error(msg);}
int stderr;
#  define Trace(x) Print(L"line %d:\n", __LINE__);
#  define Tracev(x) {if (verbose) Print(L"line %d:\n", __LINE__) ;}
#  define Tracevv(x) {if (verbose>1) Print(L"line %d:\n", __LINE__)  ;}
#  define Tracec(c,x) {if (verbose && (c))  Print(L"line %d:\n", __LINE__) ;}
#  define Tracecv(c,x) {if (verbose>1 && (c))  Print(L"line %d:\n", __LINE__) ;}
#else
#  define Assert(cond,msg)
#  define Trace(x)
#  define Tracev(x)
#  define Tracevv(x)
#  define Tracec(c,x)
#  define Tracecv(c,x)
#endif

static void flush_window(void);
static void error(char *m);
static long bytes_out;

static void error(char *m);

#define gzip_malloc(size) (void *)alloc(size, 0)
#define gzip_free(where) free(where)

#include "inflate.c"

/*
 * Run a set of bytes through the crc shift register.  If s is a NULL
 * pointer, then initialize the crc shift register contents instead.
 * Return the current crc in either case.
 *
 * Input:
 *	S	pointer to bytes to pump through.
 *	N	number of bytes in S[].
 */
static void
updcrc(unsigned char *s, unsigned n)
{
	register unsigned long c;
	/* crc is defined in inflate.c */

	c = crc;
	while (n--) {
		c = crc_32_tab[((int)c ^ (*s++)) & 0xff] ^ (c >> 8);
	}
	crc = c;
	return;
}

/*
 * Clear input and output buffers
 */
static void
clear_bufs(void)
{
	outcnt = 0;
	inptr = 0;
}

/*
 * Write the output window window[0..outcnt-1] holding uncompressed
 * data and update crc.
 */
void
flush_window(void)
{
	/*
	 * We'll end up relying on the CRC check and size check failing
	 * if there's actually more data than we expect.
	 */
	if (!outcnt || bytes_out + outcnt > outsize)
		return;

	updcrc(window, outcnt);

	Memcpy(outptr, window, outcnt);
	outptr += outcnt;
	bytes_out += outcnt;

	outcnt = 0;
}

static void
error(char *x)
{
	ERR_PRT((L"%s : %a", LD_NAME, x));
	/* will eventually exit with error from gunzip() */
}

static INT32
decompress(VOID)
{
	INT32 ret;

	clear_bufs();
	makecrc();
	Print(L"Uncompressing... ");
	ret = gunzip();
	if (ret == 0) Print(L"done\n");
	return ret == 0 ? 0 : -1;
}

int
gunzip_image(memdesc_t *image)
{
	UINTN pgcnt;

	inbuf = image->start_addr;

	/* 
	 * Last 4 bytes of gzip'd image indicates the uncompressed size
	 */
	outsize = inbuf[image->size - 1] << 24 | inbuf[image->size - 2] << 16 |
	          inbuf[image->size - 3] << 8 | inbuf[image->size - 4];

	pgcnt = EFI_SIZE_TO_PAGES(outsize);

	outbuf = alloc_pages(pgcnt, EfiLoaderData, AllocateAnyPages, 0);
    	if (outbuf == NULL) {
        	ERR_PRT((L"%s : allocate output buffer failed\n", LD_NAME));
		return -1;
    	}
	outptr = outbuf;

	window = (void *)alloc(WSIZE, 0);
	if (window == NULL) {
		ERR_PRT((L"%s : allocate output window failed\n", LD_NAME));
		free(outbuf);
		return -1;
	}

	bytes_out  = 0;

	if (decompress() != 0) {
		free(window);
		free(outbuf);
		return ELILO_LOAD_ERROR;
	}

	free(window);
	free(image->start_addr);

	image->start_addr = outbuf;
	image->size = outsize;
	image->pgcnt = pgcnt;

	return ELILO_LOAD_SUCCESS;
}
