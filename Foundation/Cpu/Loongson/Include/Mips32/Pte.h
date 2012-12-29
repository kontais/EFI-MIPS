/*  $OpenBSD: pte.h,v 1.10 2009/12/07 19:05:57 miod Exp $  */

/*
 * Copyright (c) 1988 University of Utah.
 * Copyright (c) 1992, 1993
 *  The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * the Systems Programming Group of the University of Utah Computer
 * Science Department and Ralph Campbell.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *  from: Utah Hdr: pte.h 1.11 89/09/03
 *  from: @(#)pte.h  8.1 (Berkeley) 6/10/93
 */
#ifndef _PTE_H_
#define _PTE_H_

#ifndef PAGE_SHIFT
#define  PAGE_SHIFT       12
#waring  "PAGE_SHIFT use default value 12!"
#endif

#define  TLB_ENTRY_SIZE   64

/* entryhi values */
#if PAGE_SHIFT == 12
#define  PG_SVPN    0xfffff000  /* Software page no mask */
#define  PG_HVPN    0xffffe000  /* Hardware page no mask */
#define  PG_HVPN_SHIFT 13       /* Hardware page no mask */
#define  PG_ODDPG  0x00001000   /* Odd even pte entry */
#elif PAGE_SHIFT == 14
#define  PG_SVPN    0xffffc000  /* Software page no mask */
#define  PG_HVPN    0xffff8000  /* Hardware page no mask */
#define  PG_HVPN_SHIFT 15       /* Hardware page no mask */
#define  PG_ODDPG   0x00004000  /* Odd even pte entry */
#endif

#define  PG_ASID    0x000000ff  /* Address space ID */

/* entrylo values */
#define  PG_RO    0x40000000  /* SW */
#define  PG_G     0x00000001   /* HW */
#define  PG_V     0x00000002
#define  PG_NV    0x00000000
#define  PG_M     0x00000004

#define  PG_SIZE_4K    0x00000000
#define  PG_SIZE_16K   0x00006000
#define  PG_SIZE_64K   0x0001e000
#define  PG_SIZE_256K  0x0007e000
#define  PG_SIZE_1M    0x001fe000
#define  PG_SIZE_4M    0x007fe000
#define  PG_SIZE_16M   0x01ffe000

#if PAGE_SHIFT == 12
#define  TLB_PAGE_MASK  PG_SIZE_4K
#elif PAGE_SHIFT == 14
#define  TLB_PAGE_MASK  PG_SIZE_16K
#endif

#endif /* _PTE_H_ */

