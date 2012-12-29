/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  wait.c
  
Abstract: 

  Shell Environment batch wait command

Revision History

--*/

#include "shelle.h"

EFI_STATUS
EFIAPI
SEnvNoUse (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
/*++

Routine Description:

  Built-in shell command "wait" to wait for specified microseconds.

Arguments:

  ImageHandle - The image handle
  SystemTable - The system table

Returns:

--*/
{
  return EFI_SUCCESS;
}
