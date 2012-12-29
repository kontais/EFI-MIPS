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

#ifndef __STROPS_H__
#define __STROPS_H__

extern CHAR16 *StrChr(IN const CHAR16 *s, const CHAR16 c);
extern CHAR16 *StrnCpy(OUT CHAR16 *dst, IN const CHAR16 *src, UINTN count);
extern CHAR8 *StrnXCpy(OUT CHAR8 *dst, IN const CHAR16 *src, UINTN count);

extern CHAR8 *strtok_simple(CHAR8 *in, CHAR8 c);
extern CHAR8 *strrchra(IN const CHAR8 *s, const INTN c);
extern CHAR8 *strcata(IN CHAR8 *dst,IN CHAR8 *src);
extern CHAR8 *strchra(IN const CHAR8 *s, IN const CHAR8 c);
extern CHAR8 *strcpya(CHAR8 *dst, const CHAR8 *src);
extern CHAR8 *strncpya(OUT CHAR8 *dst, IN const CHAR8 *src, IN UINTN size);
extern VOID  U2ascii(CHAR16 *in, CHAR8 *out, UINTN maxlen);

#endif /* __LOADER_H__ */
