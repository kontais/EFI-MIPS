/*++

Copyright (c) 2007, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  verEbc.c
  
Abstract:

  Part of shell app "ver".



Revision History

--*/

#include "EfiShellLib.h"
#include "ver.h"
#include EFI_PROTOCOL_DEFINITION (Ebc)

//
//
//
VOID
DisplayExtendedVersionInfo (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
  EFI_EBC_PROTOCOL *Ebc;
  EFI_STATUS       Status;
  UINT64           Version;

  Print (L"\n");

  Status = BS->LocateProtocol (
                 &gEfiEbcProtocolGuid,
                 NULL,
                 &Ebc
                 );
  if (EFI_ERROR (Status)) {
    return ;
  }

  Status = Ebc->GetVersion (Ebc, &Version);
  if (EFI_ERROR (Status)) {
    return ;
  }

  Print (L"EBC VM Version: 0x%x.0x%x\r\n", (UINTN)(UINT16)(Version >> 16), (UINTN)(UINT16)Version);

  return ;
}
