/*
 *  Copyright (C) 1999-2003 Hewlett-Packard Co.
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

//#define CHAR_NULL	(CHAR16)'\0'

CHAR16 *
StrChr(IN const CHAR16 *s, IN const CHAR16 c)
{
        for(; *s != c; ++s)  if (*s == CHAR_NULL) return NULL;

        return (CHAR16 *)s;
}

CHAR16 *
StrnCpy(OUT CHAR16 *dst, IN const CHAR16 *src, IN UINTN size)
{
	CHAR16 *res = dst;

	while (size && size-- && (*dst++ = *src++) != CHAR_NULL);
	/*
	 * does the null padding
	 */
	while (size && size-- > 0) *dst++ = CHAR_NULL;

	return res;
}

CHAR8 *
StrnXCpy(OUT CHAR8 *dst, IN const CHAR16 *src, IN UINTN size)
{
	CHAR8 *res = dst;

	while (size && size-- && (*dst++ = (CHAR8)*src++) != '\0');
	/*
	 * does the null padding
	 */
	while (size && size-- > 0) *dst++ = '\0';

	return res;
}
	
VOID
U2ascii(CHAR16 *in, CHAR8 *out, UINTN maxlen)
{
	while(maxlen-- > 1 && (*out++ = *in++));
	*out = '\0';
}
	
CHAR8 *
strncpya(OUT CHAR8 *dst, IN const CHAR8 *src, IN UINTN size)
{
	CHAR8 *res = dst;

	while (size && size-- && (*dst++ = *src++) != '\0');
	/*
	 * does the null padding
	 */
	while (size && size-- > 0) *dst++ = '\0';

	return res;
}

CHAR8 *
strcpya(CHAR8 *dst, const CHAR8 *src)
{
	CHAR8 *tmp = dst;

    while (*src) {
        *(dst++) = *(src++);
    }
    *dst = 0;

    return tmp;

}

CHAR8 *
strchra(IN const CHAR8 *s, IN const CHAR8 c)
{
        for(; *s != c; ++s)   
                if (*s == 0) return NULL;
        return (CHAR8 *)s;
}

CHAR8 *
strcata(IN CHAR8 *dst,IN CHAR8 *src)
{   
    return strcpya(dst+strlena(dst), src);
}

CHAR8 *
strrchra(IN const CHAR8 *s, const INTN c)
{
  CHAR8 *found, *p, ch = (CHAR8)c;

  /* Since strchr is fast, we use it rather than the obvious loop.  */

  if (ch == '\0') return strchra(s, '\0');

  found = NULL;
  while ((p = strchra(s, ch)) != NULL)
    {
      found = p;
      s = p + 1;
    }

  return (CHAR8 *) found;
}

CHAR8 *
strtok_simple(CHAR8 *in, CHAR8 c)
{
	static CHAR8 *last;
	CHAR8 *tmp;

	if (in == NULL) in = last;

	if (in == NULL) return NULL;

	if (*in == c) in++;

	tmp = strchra(in, c);
	if (tmp) {
		*tmp = '\0';
		last = tmp+1;
	} else {
		last = NULL;
	}
	return in;
}
	
VOID
ascii2U(CHAR8 *in, CHAR16 *out, UINTN maxlen)
{
	while(maxlen-- > 1 && (*out++ = *in++));

	*out = (CHAR16)0;
}


