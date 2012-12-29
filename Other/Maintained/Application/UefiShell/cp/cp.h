/*++
 
Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  CpCommonPart.h

Abstract:

  declares interface functions

Revision History

--*/

#ifndef _EFI_SHELL_CP_H_
#define _EFI_SHELL_CP_H_

#include "EfiShellLib.h"

//
// This is the generated header file which includes whatever needs to be exported (strings + IFR)
//
#include STRING_DEFINES_FILE

#define EFI_CPY_GUID \
  { \
    0xe85745f9, 0x36cf, 0x4d64, 0xaf, 0x48, 0x24, 0xdf, 0xd3, 0x7b, 0x27, 0xd0 \
  }
#define CP_BLOCK_SIZE (1024 * 1024)

extern UINT8          STRING_ARRAY_NAME[];

extern EFI_HII_HANDLE HiiCpyHandle;
extern EFI_GUID       EfiCpyGuid;

//
// Function Declarations
//
EFI_STATUS
CPMainProcNew (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable,
  IN UINT16               VHlpToken
  );

EFI_STATUS
CpCopy (
  IN EFI_FILE_HANDLE      SrcHandle,
  IN EFI_FILE_HANDLE      DstHandle,
  IN CHAR16               *SrcPath,
  IN CHAR16               *DstPath,
  IN BOOLEAN              DstExists
  );

EFI_STATUS
CPMainProcOld (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

#endif