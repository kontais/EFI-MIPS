/*++
 
Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  MemCommonPart.h 

Abstract:

  declares interface functions

Revision History

--*/

#ifndef _EFI_SHELL_MEM_COMMONPART_H_
#define _EFI_SHELL_MEM_COMMONPART_H_

#include "EfiShellLib.h"
#include EFI_PROTOCOL_DEFINITION (PciRootBridgeIo)

#include STRING_DEFINES_FILE

extern UINT8          STRING_ARRAY_NAME[];

//
//
//
#define EFI_MEM_GUID \
  { \
    0xa2d36078, 0x4b03, 0x45c9, 0xad, 0xa9, 0x81, 0x23, 0xd, 0xa8, 0xb0, 0x73  \
  }

extern EFI_HII_HANDLE HiiMemHandle;
extern EFI_GUID       EfiMemGuid;

EFI_STATUS
MemMainProcOld (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
MemMainProc (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

VOID
EFIMemStructsPrint (
  IN  VOID                    *Buffer,
  IN  UINTN                   BlockSize,
  IN  UINT64                  BlockAddress,
  IN  EFI_BLOCK_IO_PROTOCOL   *BlkIo
  );

#endif