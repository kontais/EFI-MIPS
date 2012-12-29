/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  ver64.c
  
Abstract:

  Part of shell app "ver".



Revision History

--*/

#include "EfiShellLib.h"
#include "LibSalPal.h"
#include "Salapi.h"
#include "ver.h"

//
//
//
VOID
DisplayExtendedVersionInfo (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
  SAL_ST_ENTRY_POINT_DESCRIPTOR *SalEpDesc;
  EFI_STATUS                    Status;
  UINT16                        SalSpecRev;
  UINT16                        SalARev;
  UINT16                        SalBRev;
  UINT64                        Value;
  //
  //
  //
  Print (L"\n");
  Status = LibGetSalVersionInfo (&SalSpecRev, &SalARev, &SalBRev);
  if (EFI_ERROR (Status)) {
    Print (L"ver: Sal system table not found\r\n");
    return ;
  }

  Print (L"Sal System Table found\r\n");

  Print (L"SAL Specification Revision    %d.%d\n", SalSpecRev >> 8, SalSpecRev & 0xff);
  Print (L"SAL A Version                 %d.%d\n", SalARev >> 8, SalARev & 0xff);
  Print (L"SAL B Version                 %d.%d\n", SalBRev >> 8, SalBRev & 0xff);

  SalEpDesc = NULL;
  SalEpDesc = (SAL_ST_ENTRY_POINT_DESCRIPTOR *) LibSearchSalSystemTable (EFI_SAL_ST_ENTRY_POINT);

  if (SalEpDesc == NULL) {
    Print (L"ver: Sal entry point not found\r\n");
    return ;
  }

  if (SalEpDesc->SalProcEntry == 0) {
    Print (L"ver: Sal entry point is invalid\r\n");
  }

  Print (L"\n");
  Print (L"Sal Entry point is          %8x\r\n", SalEpDesc->SalProcEntry);
  Print (L"Sal GP is                   %8x\r\n", SalEpDesc->SalGlobalDataPointer);

  if (SalEpDesc->PalProcEntry == 0) {
    Print (L"ver: PAL entry point invalid\r\n");
  } else {
    Print (L"Pal Entry point is          %8x\r\n", SalEpDesc->PalProcEntry);
  }
  //
  //
  //
  Status = LibGetSalIoPortMapping (&Value);
  if (!EFI_ERROR (Status)) {
    Print (L"IO Base address is          %8x\r\n", Value);
  }

  Status = LibGetSalIpiBlock (&Value);
  if (!EFI_ERROR (Status)) {
    Print (L"IPI block address is        %8x\r\n", Value);
  }

  Status = LibGetSalWakeupVector (&Value);
  if (!EFI_ERROR (Status)) {
    Print (L"SAL wakeup vector is        %8x\r\n", Value);
  }

  return ;
}
