/*++
Copyright (c) 2004 - 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module name:
  
    transmit.c

Abstract:

Revision history:
  2000-Feb-03 M(f)J   Genesis.
--*/

#include "Rtl81x9.h"
#include "Rtl81x9Dev.h"

EFI_STATUS
EFIAPI
Rtl81x9Snp_Transmit (
  IN EFI_SIMPLE_NETWORK_PROTOCOL * This,
  IN UINTN                       MacHeaderSize,
  IN UINTN                       BufferLength,
  IN VOID                        *BufferPtr,
  IN EFI_MAC_ADDRESS             * SourceAddrPtr OPTIONAL,
  IN EFI_MAC_ADDRESS             * DestinationAddrPtr OPTIONAL,
  IN UINT16                      *ProtocolPtr OPTIONAL
  )
/*++

Routine Description:
 This is the SNP interface routine for transmitting a packet. This routine 
 basically retrieves the Drv structure, checks the Drv state and calls
 pxe_fill_header and pxe_transmit calls to complete the transmission.
 
Arguments:
 This - pointer to Drv driver context
 MacHeaderSize - size of the memory at MacHeaderPtr
 BufferLength - Size of data in the BufferPtr
 BufferPtr - data buffer pointer
 SourceAddrPtr - address of the source mac address buffer
 DestinationAddrPtr - address of the destination mac address buffer
 ProtocolPtr - address of the protocol type
 
Returns:
 EFI_SUCCESS - if successfully completed the undi call
 Other - error return from undi call.
 
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

  if (BufferPtr == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (BufferLength < Drv->Mode.MediaHeaderSize) {
    return EFI_BUFFER_TOO_SMALL;
  }

  //
  // if the MacHeaderSize is non-zero, we need to fill up the header and for that
  // we need the destination address and the protocol
  //
  if (MacHeaderSize != 0) {
    if (MacHeaderSize != Drv->Mode.MediaHeaderSize || DestinationAddrPtr == 0 || ProtocolPtr == 0) {
      return EFI_INVALID_PARAMETER;
    }

    Status = Rtl81x9FillHeader (
              Drv,
              BufferPtr,
              MacHeaderSize,
              (UINT8 *) BufferPtr + MacHeaderSize,
              BufferLength - MacHeaderSize,
              DestinationAddrPtr,
              SourceAddrPtr,
              ProtocolPtr
              );

    if (Status != EFI_SUCCESS) {
      return Status;
    }
  }

  return Rtl81x9Transmit (Drv, BufferPtr, BufferLength);
}
