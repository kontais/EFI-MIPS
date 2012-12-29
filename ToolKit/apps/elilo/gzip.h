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

#ifndef __GZIP_H__
#define __GZIP_H__

int gunzip_image(memdesc_t *);
int gunzip_kernel(fops_fd_t, kdesc_t *);

/* gzip flag byte */
#define ASCII_FLAG   0x01 /* bit 0 set: file probably ASCII text */
#define CONTINUATION 0x02 /* bit 1 set: continuation of multi-part gzip file */
#define EXTRA_FIELD  0x04 /* bit 2 set: extra field present */
#define ORIG_NAME    0x08 /* bit 3 set: original file name present */
#define COMMENT      0x10 /* bit 4 set: file comment present */
#define ENCRYPTED    0x20 /* bit 5 set: file is encrypted */
#define RESERVED     0xC0 /* bit 6,7:   reserved */

/*
 * check for valid gzip signature
 * return:
 *      0 : valid gzip archive
 *      -1: invalid gzip archive
 */
static inline int
gzip_probe(unsigned char *buf, unsigned long size)
{
    if (size < 4) return -1;

    if (buf[0] != 037 ||
        ((buf[1] != 0213) && (buf[1] != 0236))) return -1;

    /* We only support method #8, DEFLATED */
    if (buf[2] != 8) return -1;
    
    if ((buf[3] & ENCRYPTED) != 0) return -1;

    if ((buf[3] & CONTINUATION) != 0) return -1;

    if ((buf[3] & RESERVED) != 0) return -1;

    return 0;
}

#endif /* __GZIP_H__ */
