/*++
Copyright (c) 2004 - 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module name:
    receive.c

Abstract:

Revision history:
    2000-Feb-03 M(f)J   Genesis.
--*/

#include "Rtl81x9.h"
#include "Rtl81x9Dev.h"

EFI_STATUS
EFIAPI
Rtl81x9Snp_Receive (
  IN EFI_SIMPLE_NETWORK_PROTOCOL * This,
  OUT UINTN                      *HeaderSizePtr OPTIONAL,
  IN OUT UINTN                   *BuffSizePtr,
  OUT VOID                       *BufferPtr,
  OUT EFI_MAC_ADDRESS            * SourceAddrPtr OPTIONAL,
  OUT EFI_MAC_ADDRESS            * DestinationAddrPtr OPTIONAL,
  OUT UINT16                     *ProtocolPtr OPTIONAL
  )
/*++

Routine Description:
 This is the SNP interface routine for receiving network data.
 this routine basically retrieves Drv structure, checks the Drv state and
 calls the pxe_receive routine to actually do the receive!

Arguments:
  This  - context pointer
  HeaderSizePtr - optional parameter and is a pointer to the header portion of
                the data received.
  BuffSizePtr - is a pointer to the length of the buffer on entry and contains
                the length of the received data on return
  BufferPtr   - pointer to the memory for the received data
  SourceAddrPtr    - optional parameter, is a pointer to contain the source
                ethernet address on return
  DestinationAddrPtr   - optional parameter, is a pointer to contain the destination
                ethernet address on return
  ProtocolPtr    - optional parameter, is a pointer to contain the protocol type
                from the ethernet header on return

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

  if ((BuffSizePtr == NULL) || (BufferPtr == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  if (!Drv->Mode.ReceiveFilterSetting) {
    return EFI_DEVICE_ERROR;
  }

  return Rtl81x9Receive (
          Drv,
          BufferPtr,
          BuffSizePtr,
          HeaderSizePtr,
          SourceAddrPtr,
          DestinationAddrPtr,
          ProtocolPtr
          );
}
