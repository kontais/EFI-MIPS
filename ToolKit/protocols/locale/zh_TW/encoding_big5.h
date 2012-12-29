/*
 * Copyright (c) 1999, 2000
 * Intel Corporation.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 
 * 3. All advertising materials mentioning features or use of this software must
 *    display the following acknowledgement:
 * 
 *    This product includes software developed by Intel Corporation and its
 *    contributors.
 * 
 * 4. Neither the name of Intel Corporation or its contributors may be used to
 *    endorse or promote products derived from this software without specific
 *    prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY INTEL CORPORATION AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL INTEL CORPORATION OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */

#ifndef _ENCODING_BIG5_H_
#define _ENCODING_BIG5_H_

#include <efi.h>
#include <runetype.h>

extern UINT8   CtypeData[];


typedef	unsigned int	u_int;

//
//The value of 'invalid_rune' is to be set by setinvalidrune() which is
//called in clients of this protocol during the setlocale process.
//The value of 'invalid_rune' is only relevant to the machine byte order
//and the ctype data provided by this protocol,
//so multiple clients (always in one machine) will set the same value
//to 'invalid_rune', which has an equivalent result as a read-only 
//'invalid_rune' once it is set by certain client.
//Then it is safe to have multiple clients
//

rune_t  invalid_rune;

#define _INVALID_RUNE   (invalid_rune)

rune_t	big5_sgetrune(const char *string, size_t n, char const **result);
int big5_sputrune(rune_t c, char *string, size_t n, char **result);
int big5_getmbcurmax(void);
_ToUnicodeMap* big5_getunicodemap(void);
int big5_getunicodemapentrycount(void);

#endif /* _ENCODING_BIG5_H_ */
