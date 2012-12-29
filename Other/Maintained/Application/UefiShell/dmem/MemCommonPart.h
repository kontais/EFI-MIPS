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

#ifndef _EFI_MEM_COMMON_PART_H_
#define _EFI_MEM_COMMON_PART_H_

#include "EfiShellLib.h"
#include EFI_PROTOCOL_DEFINITION (PciRootBridgeIo)

#include STRING_DEFINES_FILE
extern UINT8          STRING_ARRAY_NAME[];

//
//
//
#define EFI_DMEM_GUID \
  { \
    0xb7831e0d, 0x7120, 0x47d2, 0x9b, 0xfb, 0x40, 0xe6, 0x13, 0xa3, 0xf8, 0x5e \
  }

extern EFI_HII_HANDLE HiiDmemHandle;
extern EFI_GUID       EfiDmemGuid;

EFI_STATUS
DmemMainProcOld (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
DmemMainProc (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

VOID
EFIStructsPrint (
  IN  VOID                    *Buffer,
  IN  UINTN                   BlockSize,
  IN  UINT64                  BlockAddress,
  IN  EFI_BLOCK_IO_PROTOCOL   *BlkIo
  );

#endif