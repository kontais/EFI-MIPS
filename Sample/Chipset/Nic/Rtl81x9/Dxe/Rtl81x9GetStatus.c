/*++
Copyright (c) 2004 - 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module name:
  get_status.c

Abstract:

Revision history:
  2000-Feb-03 M(f)J   Genesis.
--*/

#include "Rtl81x9.h"
#include "Rtl81x9Dev.h"

EFI_STATUS
EFIAPI
Rtl81x9Snp_GetStatus (
  IN EFI_SIMPLE_NETWORK_PROTOCOL * This,
  OUT UINT32                     *InterruptStatusPtr OPTIONAL,
  OUT VOID                       **TransmitBufferListPtr OPTIONAL
  )
/*++

Routine Description:
 This is the SNP interface routine for getting the status
 this routine basically retrieves Drv structure, checks the Drv state and
 calls the pxe_getstatus routine to actually get the undi status

Arguments:
 This  - context pointer
 InterruptStatusPtr - a non null pointer gets the interrupt status
 TransmitBufferListPtrs - a non null ointer gets the list of pointers of previously 
              transmitted buffers whose transmission was completed 
              asynchrnously.
              
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
  case EfiSimpleNetworkInitialized:
    break;

  case EfiSimpleNetworkStopped:
    return EFI_NOT_STARTED;

  case EfiSimpleNetworkStarted:
    return EFI_DEVICE_ERROR;

  default:
    return EFI_DEVICE_ERROR;
  }

  if (InterruptStatusPtr == NULL && TransmitBufferListPtr == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  return Rtl81x9GetStatus (Drv, InterruptStatusPtr, TransmitBufferListPtr);
}
