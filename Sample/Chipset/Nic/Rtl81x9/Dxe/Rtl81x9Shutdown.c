/*++
Copyright (c) 2004 - 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module name:
  Rtl81x9Shutdown.c

Abstract:

Revision history:
  2000-Feb-14 M(f)J   Genesis.
--*/

#include "Rtl81x9.h"
#include "Rtl81x9Dev.h"

EFI_STATUS
EFIAPI
Rtl81x9Snp_Shutdown (
  IN EFI_SIMPLE_NETWORK_PROTOCOL *This
  )
/*++

Routine Description:
 This is the SNP interface routine for shutting down the interface
 This routine basically retrieves Snp structure, checks the SNP state and
 calls the Rtl81x9Shutdown routine to actually do the undi shutdown

Arguments:
  This  - context pointer

Returns:

--*/
{
  RTL81X9_SNP_DRIVER  *Drv;
  EFI_STATUS          Status;

  //
  //
  //
  if (This == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Drv = RTL81X9_PRIVATE_FROM_EFI_SIMPLE_NETWORK (This);

  if (Drv == NULL) {
    return EFI_DEVICE_ERROR;
  }
  //
  //
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
  //
  //
  Status                          = Rtl81x9Shutdown (Drv);

  Drv->Mode.State                 = EfiSimpleNetworkStarted;
  Drv->Mode.ReceiveFilterSetting  = 0;

  Drv->Mode.MCastFilterCount      = 0;
  Drv->Mode.ReceiveFilterSetting  = 0;
  EfiZeroMem (Drv->Mode.MCastFilter, sizeof Drv->Mode.MCastFilter);
  EfiCopyMem (
    &Drv->Mode.CurrentAddress,
    &Drv->Mode.PermanentAddress,
    sizeof (EFI_MAC_ADDRESS)
    );

  gBS->CloseEvent (Drv->Snp.WaitForPacket);

  return Status;
}
