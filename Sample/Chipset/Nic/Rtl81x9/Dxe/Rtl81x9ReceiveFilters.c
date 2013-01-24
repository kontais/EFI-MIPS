/*++
Copyright (c) 2004 - 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module name:
  receive_filters.c

Abstract:

Revision history:
  2000-Feb-17 M(f)J   Genesis.
--*/

#include "Rtl81x9.h"
#include "Rtl81x9Dev.h"

EFI_STATUS
EFIAPI
Rtl81x9Snp_ReceiveFilters (
  IN EFI_SIMPLE_NETWORK_PROTOCOL * This,
  IN UINT32                      EnableFlags,
  IN UINT32                      DisableFlags,
  IN BOOLEAN                     ResetMCastList,
  IN UINTN                       MCastAddressCount OPTIONAL,
  IN EFI_MAC_ADDRESS             * MCastAddressList OPTIONAL
  )
/*++

Routine Description:
 This is the SNP interface routine for reading/enabling/disabling the
 receive filters.
 this routine basically retrieves Drv structure, checks the Drv state and
 checks the parameter validity, calls one of the above routines to actually
 do the work

Arguments:
  This  - context pointer
  EnableFlags - bit mask for enabling the receive filters
  DisableFlags - bit mask for disabling the receive filters
  ResetMCastList - boolean flag to reset/delete the multicast filter list
  MCastAddressCount - multicast address count for a new multicast address list
  MCastAddressList  - list of new multicast addresses

Returns:

--*/
{
  RTL81X9_SNP_DRIVER  *Drv;
  EFI_STATUS          Status;

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
  //
  // check if we are asked to enable or disable something that the UNDI
  // does not even support!
  //
  if ((EnableFlags &~Drv->Mode.ReceiveFilterMask) != 0) {
    return EFI_INVALID_PARAMETER;
  }

  if ((DisableFlags &~Drv->Mode.ReceiveFilterMask) != 0) {
    return EFI_INVALID_PARAMETER;
  }

  if (ResetMCastList) {
    DisableFlags |= EFI_SIMPLE_NETWORK_RECEIVE_MULTICAST & Drv->Mode.ReceiveFilterMask;
    MCastAddressCount = 0;
    MCastAddressList  = NULL;
  } else {
    if (MCastAddressCount != 0) {
      if (MCastAddressCount > Drv->Mode.MaxMCastFilterCount) {
        return EFI_INVALID_PARAMETER;
      }

      if (MCastAddressList == NULL) {
        return EFI_INVALID_PARAMETER;
      }
    }
  }

  if (EnableFlags == 0 && DisableFlags == 0 && !ResetMCastList && MCastAddressCount == 0) {
    return EFI_SUCCESS;
  }

  if ((EnableFlags & EFI_SIMPLE_NETWORK_RECEIVE_MULTICAST) != 0 && MCastAddressCount == 0) {
    return EFI_INVALID_PARAMETER;
  }

  if ((EnableFlags != 0) || (MCastAddressCount != 0)) {
    Status = Rtl81x9ReceiveFilterEnable (
              Drv,
              EnableFlags,
              MCastAddressCount,
              MCastAddressList
              );

    if (Status != EFI_SUCCESS) {
      return Status;
    }
  }

  if ((DisableFlags != 0) || ResetMCastList) {
    Status = Rtl81x9ReceiveFilterDisable (Drv, DisableFlags, ResetMCastList);

    if (Status != EFI_SUCCESS) {
      return Status;
    }
  }

  return Rtl81x9ReceiveFilterRead (Drv);
}
