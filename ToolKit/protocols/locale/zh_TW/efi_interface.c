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

#include "locale_protocol.h"
#include "encoding_big5.h"
#include "ctype_big5.h"
#include "collate_big5.h"
#include "time_big5.h"

static char*
getlocale (
	void
	)
{
	return "zh_TW";
}

static rune_t
sgetrune (
	IN		const char	*string,
	IN		size_t		n,
	IN OUT	char const	**result
	)
{
	return big5_sgetrune(string, n, result);
}


static int
sputrune (
	IN		rune_t		c,
	IN		char		*string,
	IN		size_t		n,
	IN OUT	char		**result
	)
{
	return big5_sputrune(c, string, n, result);
}

static int
getmbcurmax (
	void
	)
{
	return big5_getmbcurmax();
}

static _ToUnicodeMap*
getunicodemap (
	void
	)
{
	return big5_getunicodemap();
}

static int
getunicodemapentrycount (
	void
	)
{
	return big5_getunicodemapentrycount();
}

static UINT8* 
getctypedata (
    void
    )
{
	return CtypeData;
}

static int 
getctypedatasize (
    void
    )
{
	return CtypeDataSize;
}

static UINT8* 
getcollatedata (
    void
    )
{
	return CollateData;
}

static int 
getcollatedatasize (
    void
    )
{
	return CollateDataSize;
}

static UINT8* 
gettimedata (
    void
    )
{
	return TimeData;
}

static int 
gettimedatasize (
    void
    )
{
	return TimeDataSize;
}

static EFI_STATUS
setinvalidrune(
    rune_t val
    )
{
    invalid_rune = val;
    return (EFI_SUCCESS);
}

EFI_STATUS
InitLocaleInterface (
	VOID
	)
{
	return (EFI_SUCCESS);
}

//
//  The EFI locale protocol Interface Table
//
EFI_LOCALE_INTERFACE EfiLocaleInterface = {

	EFI_LOCALE_INTERFACE_REVISION,

	getlocale,
	sgetrune,
	sputrune,
	getmbcurmax,
	getunicodemap,
	getunicodemapentrycount,
	getctypedata,
	getctypedatasize,
	getcollatedata,
	getcollatedatasize,
	gettimedata,
	gettimedatasize,
	setinvalidrune
};
