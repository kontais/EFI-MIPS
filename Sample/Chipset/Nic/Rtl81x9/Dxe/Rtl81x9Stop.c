/*++
Copyright (c) 2004 - 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module name:
  stop.c

Abstract:

Revision history:
  2000-Feb-09 M(f)J   Genesis.
--*/

#include "Rtl81x9.h"
#include "Rtl81x9Dev.h"

EFI_STATUS
EFIAPI
Rtl81x9Snp_Stop (
  IN EFI_SIMPLE_NETWORK_PROTOCOL *This
  )
/*++

Routine Description:
 This is the SNP interface routine for stopping the interface.
 This routine basically retrieves snp structure, checks the SNP state and
 calls the Rtl81x9Stop routine to actually stop the undi interface

Arguments:
  This  - context pointer

Returns:

--*/
{
  RTL81X9_SNP_DRIVER  *Drv;

  if (This == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Drv = RTL81X9_PRIVATE_FROM_EFI_SIMPLE_NETWORK (This);

  if (Drv == NULL) {
    return EFI_DEVICE_ERROR;
  }

  switch (Drv->Mode.State) {
  case EfiSimpleNetworkStarted:
    break;

  case EfiSimpleNetworkStopped:
    return EFI_NOT_STARTED;

  case EfiSimpleNetworkInitialized:
    return EFI_DEVICE_ERROR;

  default:
    return EFI_DEVICE_ERROR;
  }

  return Rtl81x9Stop (Drv);
}
