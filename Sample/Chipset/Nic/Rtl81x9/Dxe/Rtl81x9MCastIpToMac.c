/*++
Copyright (c) 2004 - 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module name:
  mcast_ip_to_mac.c

Abstract:

Revision history:
  2000-Feb-17 M(f)J   Genesis.
--*/

#include "Rtl81x9.h"
#include "Rtl81x9Dev.h"

EFI_STATUS
EFIAPI
Rtl81x9Snp_MCastIpToMac (
  IN EFI_SIMPLE_NETWORK_PROTOCOL *This,
  IN BOOLEAN                     IPv6,
  IN EFI_IP_ADDRESS              *IP,
  OUT EFI_MAC_ADDRESS            *MAC
  )
/*++

Routine Description:
 This is the SNP interface routine for converting a multicast IP address to
 a MAC address.
 this routine basically retrieves Drv structure, checks the Drv state and
 calls the pxe_ip2mac routine to actually do the conversion

Arguments:
  This  - context pointer
  IPv6  - flag to indicate if This is an ipv6 address
  IP    - multicast IP address
  MAC   - pointer to hold the return MAC address

Returns:

--*/
{
  RTL81X9_SNP_DRIVER  *Drv;

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

  if (IP == NULL || MAC == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  return Rtl81x9MCastIpToMac (Drv, IPv6, IP, MAC);
}
