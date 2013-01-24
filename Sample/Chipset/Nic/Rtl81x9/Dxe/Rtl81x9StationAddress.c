/*++
Copyright (c) 2004 - 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module name:
  station_address.c

Abstract:

Revision history:
  2000-Feb-17 M(f)J   Genesis.
--*/

#include "Rtl81x9.h"


EFI_STATUS
EFIAPI
Rtl81x9Snp_StationAddress (
  IN EFI_SIMPLE_NETWORK_PROTOCOL * This,
  IN BOOLEAN                     ResetFlag,
  IN EFI_MAC_ADDRESS             * NewMacAddr OPTIONAL
  )
/*++

Routine Description:
 This is the SNP interface routine for changing the NIC's mac address.
 this routine basically retrieves Drv structure, checks the Drv state and
 calls the above routines to actually do the work

Arguments:
 This  - context pointer
 NewMacAddr - pointer to a mac address to be set for the nic, if This is NULL
              then this routine resets the mac address to the NIC's original
              address.
 ResetFlag - If true, the mac address will change to NIC's original address

Returns:

--*/
{
  RTL81X9_SNP_DRIVER  *Drv;
  EFI_STATUS          Status;

  //
  // Get pointer to Drv driver instance for *This.
  //
  if (This == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Drv = RTL81X9_PRIVATE_FROM_EFI_SIMPLE_NETWORK (This);

  if (Drv == NULL) {
    return EFI_DEVICE_ERROR;
  }
  //
  // Return error if the Drv is not initialized.
  //
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
  // Check for invalid parameter combinations.
  //
  if (!ResetFlag && NewMacAddr == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (ResetFlag) {
    Status = Rtl81x9SetStationAddress (Drv, NULL);
  } else {
    Status = Rtl81x9SetStationAddress (Drv, NewMacAddr);

  }

  return Status;
}
