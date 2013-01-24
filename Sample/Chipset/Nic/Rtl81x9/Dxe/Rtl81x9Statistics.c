/*++
Copyright (c) 2004 - 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module name:
  statistics.c

Abstract:

Revision history:
  2000-Feb-17 M(f)J   Genesis.
--*/

#include "Rtl81x9.h"

EFI_STATUS
EFIAPI
Rtl81x9Snp_Statistics (
  IN EFI_SIMPLE_NETWORK_PROTOCOL * This,
  IN BOOLEAN                     ResetFlag,
  IN OUT UINTN                   *StatTableSizePtr OPTIONAL,
  IN OUT EFI_NETWORK_STATISTICS  * StatTablePtr OPTIONAL
  )
/*++

Routine Description:
 This is the SNP interface routine for getting the NIC's statistics.
 this routine basically retrieves Drv structure, checks the Drv state and
 calls the pxe_ routine to actually do the 

Arguments:
  This  - context pointer
  ResetFlag - true to reset the NIC's statistics counters to zero.
  StatTableSizePtr - pointer to the statistics table size
  StatTablePtr - pointer to the statistics table 
  
Returns:

--*/
{
  RTL81X9_SNP_DRIVER        *Drv;
  PXE_DB_STATISTICS *db;
  UINT64            *stp;
  UINT64            mask;
  UINTN             size;
  UINTN             n;

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
  // if we are not resetting the counters, we have to have a valid stat table
  // with >0 size. if no reset, no table and no size, return success.
  //
  if (!ResetFlag && StatTableSizePtr == NULL) {
    return StatTablePtr ? EFI_INVALID_PARAMETER : EFI_SUCCESS;
  }

  if (ResetFlag) {
    return EFI_SUCCESS;
  }

  if (StatTablePtr == NULL) {
    *StatTableSizePtr = sizeof (EFI_NETWORK_STATISTICS);
    return EFI_BUFFER_TOO_SMALL;
  }

  EfiZeroMem (StatTablePtr, sizeof (EFI_NETWORK_STATISTICS));

  return EFI_SUCCESS;
}
