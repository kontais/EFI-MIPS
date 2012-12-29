/*
 * Copyright (c) 1999, 2000
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
 *    This product includes software developed by Intel Corporation and
 *    its contributors.
 * 
 * 4. Neither the name of Intel Corporation or its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY INTEL CORPORATION AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL INTEL CORPORATION OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */


#include "encoding_big5.h"
#include "tounicodemap_big5.h"


static  int
_big5_check(c)
	u_int c;
{
	c &= 0xff;
	return ((c >= 0xa1 && c <= 0xfe) ? 2 : 1);
}

rune_t
big5_sgetrune(string, n, result)
	const char *string;
	size_t n;
	char const **result;
{
	rune_t rune = 0;
	int len;

	if (n < 1 || (len = _big5_check(*string)) > (int)n) {
		if (result)
			*result = string;
		return (_INVALID_RUNE);
	}
	while (--len >= 0)
		rune = (rune << 8) | ((u_int)(*string++) & 0xff);
	if (result)
		*result = string;
//	return map_big5_to_unicode (rune);
return rune;
}

int
big5_sputrune(c, string, n, result)
	rune_t c;
	char *string, **result;
	size_t n;
{
	//c = map_unicode_to_big5(c);
	
	if (c & 0x8000) {
		if (n >= 2) {
			string[0] = (c >> 8) & 0xff;
			string[1] = c & 0xff;
			if (result)
				*result = string + 2;
			return (2);
		}
	}
	else {
		if (n >= 1) {
			*string = c & 0xff;
			if (result)
				*result = string + 1;
			return (1);
		}
	}
	if (result)
		*result = string;
	return (0);
	
}

int
big5_getmbcurmax(void)
{
	return 2;
}

_ToUnicodeMap* 
big5_getunicodemap(void)
{
	return to_unicode_map;
}

int 
big5_getunicodemapentrycount(void)
{
	return to_unicode_map_entrycount;
}

/*

int
_BIG5_init(rl)
	_RuneLocale *rl;
{
	rl->sgetrune = _BIG5_sgetrune;
	rl->sputrune = _BIG5_sputrune;
	_CurrentRuneLocale = rl;
	__mb_cur_max = 2;
	_CurrentMap = (_ToUnicodeMap *) big5_to_unicode_map;
	_CurrentMapEntryCount = big5_to_unicode_map_entrycount;

	return (0);
}

*/
