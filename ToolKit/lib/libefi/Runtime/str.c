/*++

Copyright (c)  1999 - 2002 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

    str.c

Abstract:

    String runtime functions


Revision History

--*/

#include "lib.h"

INTN
RUNTIMEFUNCTION
RtStrCmp (
    IN CHAR16   *s1,
    IN CHAR16   *s2
    )
// compare strings
{
    while (*s1) {
        if (*s1 != *s2) {
            break;
        }

        s1 += 1;
        s2 += 1;
    }

    return *s1 - *s2;
}

VOID
RUNTIMEFUNCTION
RtStrCpy (
    IN CHAR16   *Dest,
    IN CHAR16   *Src
    )
// copy strings
{
    while (*Src) {
        *(Dest++) = *(Src++);
    }
    *Dest = 0;
}

VOID
RUNTIMEFUNCTION
RtStrCat (
    IN CHAR16   *Dest,
    IN CHAR16   *Src
    )
{   
    RtStrCpy(Dest+StrLen(Dest), Src);
}

UINTN
RUNTIMEFUNCTION
RtStrLen (
    IN CHAR16   *s1
    )
// string length
{
    UINTN        len;
    
    for (len=0; *s1; s1+=1, len+=1) ;
    return len;
}

UINTN
RUNTIMEFUNCTION
RtStrSize (
    IN CHAR16   *s1
    )
// string size
{
    UINTN        len;
    
    for (len=0; *s1; s1+=1, len+=1) ;
    return (len + 1) * sizeof(CHAR16);
}

UINT8
RUNTIMEFUNCTION
RtBCDtoDecimal(
    IN  UINT8 BcdValue
    )
{
    UINTN   High, Low;

    High    = BcdValue >> 4;
    Low     = BcdValue - (High << 4);

    return ((UINT8)(Low + (High * 10)));
}


UINT8
RUNTIMEFUNCTION
RtDecimaltoBCD (
    IN  UINT8 DecValue
    )
{
    UINTN   High, Low;

    High    = DecValue / 10;
    Low     = DecValue - (High * 10);

    return ((UINT8)(Low + (High << 4)));
}


