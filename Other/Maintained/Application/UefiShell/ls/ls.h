/*++
  
Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  ls.h

Abstract:

  declares interface functions

Revision History

--*/

#ifndef _EFI_SHELL_LS_H_
#define _EFI_SHELL_LS_H_

#include "EfiShellLib.h"

//
// This is the generated header file which includes whatever needs to be exported (strings + IFR)
//
#include STRING_DEFINES_FILE

extern UINT8          STRING_ARRAY_NAME[];

#define EFI_LS_GUID \
  { \
    0x4884f436, 0x3bbd, 0x4d8f, \
    { \
      0x86, 0xd4, 0x8b, 0xf, 0x4a, 0x6a, 0x4d, 0x15 \
    } \
  }

extern EFI_HII_HANDLE HiiLsHandle;
extern EFI_GUID       EfiLsGuid;

typedef enum {
  TYPE_DIRECTORY,
  TYPE_EXECUTABLE,
  TYPE_UNKNOWN
} SHELL_FILE_TYPE;

EFI_STATUS
Listing (
  IN EFI_FILE_HANDLE      ParentHandle,
  IN CHAR16               *ParentPath,
  IN CHAR16               *Pattern
  );

EFI_STATUS
FindMatchItem (
  IN EFI_FILE_HANDLE   ParentHandle,
  IN CHAR16            *ParentPath,
  IN CHAR16            *Pattern,
  IN BOOLEAN           MatchStratage,
  IN OUT BOOLEAN       *Something,
  IN EFI_FILE_INFO     *Info
  );

CHAR16 *
CleanupAsFatLfn (
  CHAR16  *Name
  );

CHAR16 *
GetPattern (
  IN CHAR16 *Path
  );

BOOLEAN
IsMatch (
  EFI_FILE_INFO  *Info,
  CHAR16         *Pattern,
  BOOLEAN        BracketLiteral
  );

EFI_STATUS
MainProc (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable,
  IN UINT16               VHlpToken
  );

#endif