/*++
Copyright (c) 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module name:
  WaitForPacket.c

Abstract:
  Event handler to check for available packet.

--*/

#include "Rtl81x9.h"

VOID
EFIAPI
Rtl81x9WaitForPacketNotify (
  EFI_EVENT Event,
  VOID      *SnpPtr
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  UINT32             RxStatus;
  RTL81X9_SNP_DRIVER *Drv;

  //
  // Do nothing if either parameter is a NULL pointer.
  //
  if (Event == NULL || SnpPtr == NULL) {
    return ;
  }

  Drv = (RTL81X9_SNP_DRIVER *) SnpPtr;
  
  //
  // Do nothing if the SNP interface is not initialized.
  //
  switch (Drv->Mode.State) {
  case EfiSimpleNetworkInitialized:
    break;

  case EfiSimpleNetworkStopped:
  case EfiSimpleNetworkStarted:
  default:
    return ;
  }

  Rtl81x9RxStatus (Drv, &RxStatus);

  if (RxStatus) {
    gBS->SignalEvent (Event);
  }

}

