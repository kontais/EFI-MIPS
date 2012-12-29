/*++

Copyright (c) 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  LinuxLib.c

Abstract:

  Setup gLinux global 

--*/

#include "Efi2Linux.h"
#include "EfiLinuxLib.h"
#include "EfiDriverLib.h"
#include "EfiHobLib.h"
#include EFI_GUID_DEFINITION (Hob)

EFI_LINUX_THUNK_PROTOCOL *gLinux;

EFI_GUID                  mEfiHobListGuid = EFI_HOB_LIST_GUID;

EFI_STATUS
EfiInitializeLinuxDriverLib (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
/*++

Routine Description:

  Intialize gLinux and initialize debug console. 

Arguments:

  (Standard EFI Image entry - EFI_IMAGE_ENTRY_POINT)

Returns: 

  None

--*/
// TODO:    ImageHandle - add argument and description to function comment
// TODO:    SystemTable - add argument and description to function comment
// TODO:    EFI_NOT_FOUND - add return value to function comment
// TODO:    EFI_NOT_FOUND - add return value to function comment
// TODO:    EFI_SUCCESS - add return value to function comment
{
  EFI_STATUS  Status;
  VOID        *HobList;
  VOID        *Pointer;

  Status = EfiLibGetSystemConfigurationTable (&mEfiHobListGuid, &HobList);
  if (EFI_ERROR (Status)) {
    return EFI_NOT_FOUND;
  }

  ASSERT (NULL != HobList);
  Status = GetNextGuidHob (&HobList, &gEfiLinuxThunkProtocolGuid, &Pointer, NULL);
  if (EFI_ERROR (Status)) {
    return EFI_NOT_FOUND;
  }

  gLinux = (VOID *) (*(UINTN *) (Pointer));
  return EFI_SUCCESS;
}
