/*++
Copyright (c) 2004 - 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module name:
  initialize.c

Abstract:

Revision history:
  2000-Feb-09 M(f)J   Genesis.
--*/

#include "Rtl81x9.h"
#include "Rtl81x9Dev.h"

VOID
EFIAPI
SnpWaitForPacketNotify (
  IN EFI_EVENT  Event,
  IN VOID       *SnpPtr
  );

EFI_STATUS
EFIAPI
Rtl81x9Snp_Initialize (
  IN EFI_SIMPLE_NETWORK_PROTOCOL *This,
  IN UINTN                       ExtraRxBufferSize OPTIONAL,
  IN UINTN                       ExtraTxBufferSize OPTIONAL
  )
/*++

Routine Description:
 This is the SNP interface routine for initializing the interface
 this routine basically retrieves Drv structure, checks the Drv state and
 calls the pxe_initialize routine to actually do the undi initialization

Arguments:
 This  - context pointer
 extra_rx_buffer_size - optional parameter, indicates extra space for rx_buffers
 extra_tx_buffer_size - optional parameter, indicates extra space for tx_buffers

Returns:

--*/
{
  EFI_STATUS          EfiStatus;
  RTL81X9_SNP_DRIVER  *Drv;

  //
  //
  //
  if (This == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Drv = RTL81X9_PRIVATE_FROM_EFI_SIMPLE_NETWORK (This);

  if (Drv == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  //
  //
  //
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
  //
  //
  //
  EfiStatus = gBS->CreateEvent (
                    EFI_EVENT_NOTIFY_WAIT,
                    EFI_TPL_NOTIFY,
                    &Rtl81x9WaitForPacketNotify,
                    Drv,
                    &Drv->Snp.WaitForPacket
                    );

  if (EFI_ERROR (EfiStatus)) {
    Drv->Snp.WaitForPacket = NULL;
    return EFI_DEVICE_ERROR;
  }
  //
  //
  //
  Drv->Mode.MCastFilterCount      = 0;
  Drv->Mode.ReceiveFilterSetting  = 0;
  EfiZeroMem (Drv->Mode.MCastFilter, sizeof Drv->Mode.MCastFilter);
  EfiCopyMem (
    &Drv->Mode.CurrentAddress,
    &Drv->Mode.PermanentAddress,
    sizeof (EFI_MAC_ADDRESS)
    );

  if (Drv->Mode.MediaPresentSupported) {
    if (Rtl81x9Init (Drv, PXE_OPFLAGS_INITIALIZE_DETECT_CABLE) == EFI_SUCCESS) {
      Drv->Mode.MediaPresent = TRUE;
      return EFI_SUCCESS;
    }
  }

  Drv->Mode.MediaPresent  = FALSE;

  EfiStatus               = Rtl81x9Init (Drv, PXE_OPFLAGS_INITIALIZE_DO_NOT_DETECT_CABLE);

  if (EFI_ERROR (EfiStatus)) {
    gBS->CloseEvent (Drv->Snp.WaitForPacket);
  }

  return EfiStatus;
}

