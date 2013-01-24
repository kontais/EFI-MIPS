/*++
Copyright (c) 2004 - 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module name:
  start.c

Abstract:

Revision history:
  2000-Feb-07 M(f)J   Genesis.
--*/

#include "Rtl81x9.h"

EFI_STATUS
EFIAPI
Rtl81x9Snp_Start (
  IN EFI_SIMPLE_NETWORK_PROTOCOL *This
  )
/*++

Routine Description:
 This is the SNP interface routine for starting the interface
 this routine basically retrieves Drv structure, checks the Drv state and
 calls the pxe_start routine to actually do start undi interface

Arguments:
  This  - context pointer

Returns:
  EFI_INVALID_PARAMETER - "This" is Null
                        - No Drv driver can be extracted from "This"
  EFI_ALREADY_STARTED   - The state of Drv is EfiSimpleNetworkStarted
                          or EfiSimpleNetworkInitialized
  EFI_DEVICE_ERROR      - The state of Drv is other than EfiSimpleNetworkStarted,
                          EfiSimpleNetworkInitialized, and EfiSimpleNetworkStopped
  EFI_SUCCESS           - UNDI interface is succesfully started
  Other                 - Error occurs while calling pxe_start function.
  
--*/
{
  RTL81X9_SNP_DRIVER  *Drv;
  EFI_STATUS          Status;
  UINTN               Index;

  if (This == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Drv = RTL81X9_PRIVATE_FROM_EFI_SIMPLE_NETWORK (This);

  if (Drv == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  switch (Drv->Mode.State) {
  case EfiSimpleNetworkStopped:
    break;

  case EfiSimpleNetworkStarted:
  case EfiSimpleNetworkInitialized:
    return EFI_ALREADY_STARTED;

  default:
    return EFI_DEVICE_ERROR;
  }

  Status = Rtl81x9Start (Drv);
  if (Status != EFI_SUCCESS) {
    return Status;
  }

#if 0
  //
  // clear the map_list in Drv structure
  //
  for (Index = 0; Index < MAX_MAP_LENGTH; Index++) {
    Drv->map_list[Index].virt       = 0;
    Drv->map_list[Index].map_cookie = 0;
  }
#endif

  Drv->Mode.MCastFilterCount = 0;

  return Status;
}
