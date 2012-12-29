/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  ver.h

Abstract:

  declares interface functions

Revision History

--*/

#ifndef _VER_H
#define _VER_H
//
//
//
#define EFI_VER_GUID \
  { \
    0xbe5924a7, 0x1cbd, 0x488b, 0xb1, 0x16, 0xad, 0xbe, 0x79, 0x72, 0x79, 0xb2 \
  }

VOID
DisplayExtendedVersionInfo (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  );
#endif
