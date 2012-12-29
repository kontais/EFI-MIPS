/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

    Str.h

Abstract:
    Information about the string operation functions



Revision History

--*/
#ifndef _SHELL_STRING_H
#define _SHELL_STRING_H

#define ESCAPE_CHAR 0x03

INTN
StrCmp (
  IN CHAR16   *s1,
  IN CHAR16   *s2
  );

INTN
StrnCmp (
  IN CHAR16       *s1,
  IN CHAR16       *s2,
  IN UINTN        len
  );

VOID
StrCpy (
  IN CHAR16   *Dest,
  IN CHAR16   *Src
  );

VOID
StrCat (
  IN CHAR16   *Dest,
  IN CHAR16   *Src
  );

UINTN
StrLen (
  IN CHAR16   *s1
  );

VOID
StrLwr (
  IN CHAR16   *Str
  );

VOID
StrUpr (
  IN CHAR16   *Str
  );

UINTN
Xtoi (
  IN CHAR16    *str
  );

UINTN
Atoi (
  IN CHAR16    *str
  );

INTN
StriCmp (
  IN CHAR16   *s1,
  IN CHAR16   *s2
  );

UINTN
StrSize (
  IN CHAR16   *s1
  );

CHAR16                          *
StrDuplicate (
  IN CHAR16   *Src
  );

UINTN
strlena (
  IN CHAR8    *s1
  );

UINTN
strcmpa (
  IN CHAR8    *s1,
  IN CHAR8    *s2
  );

UINTN
strncmpa (
  IN CHAR8    *s1,
  IN CHAR8    *s2,
  IN UINTN    len
  );

BOOLEAN
StrSubCmp (
  IN CHAR16   *s1,
  IN CHAR16   *s2,
  IN UINTN    len
  );

CHAR16                          *
StrChr (
  IN  CHAR16  *Str,
  IN  CHAR16  c
  );

UINTN
StrStr (
  IN  CHAR16  *Str,
  IN  CHAR16  *Pat
  );

VOID
StrTrim (
  IN OUT CHAR16   *str,
  IN     CHAR16   c
  );

VOID
StrTrimRight (
  IN OUT CHAR16   *str,
  IN     CHAR16   c
  );

INTN
StrnCmp (
  IN CHAR16                         *s1,
  IN CHAR16                         *s2,
  IN UINTN                          len
  );

INTN
StriCmp (
  IN CHAR16                         *s1,
  IN CHAR16                         *s2
  );

VOID
StrnCpy (
  OUT CHAR16                   *Dst,
  IN  CHAR16                   *Src,
  IN  UINTN                    Length
  );

BOOLEAN
HasWildcards (
  IN CHAR16 *Pattern
  );

UINT64
StrToUIntegerBase (
  IN  CHAR16      *Str,
  IN  UINTN       Base,
  OUT EFI_STATUS  *Status
  );

UINT64
StrToUInteger (
  IN  CHAR16      *Str,
  OUT EFI_STATUS  *Status
  );

UINT64
StrToUInt (
  CHAR16      *Str,
  UINTN       Base,
  EFI_STATUS  *Status
  );

BOOLEAN
MetaMatch (
  IN CHAR16   *String,
  IN CHAR16   *Pattern
  );

BOOLEAN
MetaiMatch (
  IN CHAR16                         *String,
  IN CHAR16                         *Pattern
  );

BOOLEAN
EFIAPI
LibStubMetaiMatch (
  IN EFI_UNICODE_COLLATION_PROTOCOL   *This,
  IN CHAR16                           *String,
  IN CHAR16                           *Pattern
  );

INTN
EFIAPI
LibStubStriCmp (
  IN EFI_UNICODE_COLLATION_PROTOCOL   *This,
  IN CHAR16                           *s1,
  IN CHAR16                           *s2
  );

VOID
EFIAPI
LibStubStrLwr (
  IN EFI_UNICODE_COLLATION_PROTOCOL   *This,
  IN CHAR16                           *Str
  );

VOID
EFIAPI
LibStubStrUpr (
  IN EFI_UNICODE_COLLATION_PROTOCOL   *This,
  IN CHAR16                           *Str
  );

#endif