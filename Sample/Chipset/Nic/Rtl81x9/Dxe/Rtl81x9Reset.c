/*++
Copyright (c) 2004 - 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module name:
  reset.c

Abstract:

Revision history:
  2000-Feb-09 M(f)J   Genesis.
--*/

#include "Rtl81x9.h"
#include "Rtl81x9Dev.h"

EFI_STATUS
EFIAPI
Rtl81x9Snp_Reset (
  IN EFI_SIMPLE_NETWORK_PROTOCOL *This,
  IN BOOLEAN                     ExtendedVerification
  )
/*++

Routine Description:
 This is the SNP interface routine for resetting the NIC
 this routine basically retrieves Drv structure, checks the Drv state and
 calls the pxe_reset routine to actually do the reset!

Arguments:
 This - context pointer
 ExtendedVerification - not implemented

Returns:

--*/
{
  RTL81X9_SNP_DRIVER  *Drv;

  //
  // Resolve Warning 4 unreferenced parameter problem
  //
  ExtendedVerification = 0;

  if (This == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Drv = RTL81X9_PRIVATE_FROM_EFI_SIMPLE_NETWORK (This);

  if (Drv == NULL) {
    return EFI_DEVICE_ERROR;
  }

  switch (Drv->Mode.State) {
  case EfiSimpleNetworkInitialized:
    break;

  case EfiSimpleNetworkStopped:
    return EFI_NOT_STARTED;

  case EfiSimpleNetworkStarted:
    return EFI_DEVICE_ERROR;

  default:
    return EFI_DEVICE_ERROR;
  }

  return Rtl81x9Reset (Drv);
}

