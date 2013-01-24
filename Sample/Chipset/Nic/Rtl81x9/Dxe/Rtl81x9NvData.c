/*++
Copyright (c) 2012, kontais                                               
Copyright (c) 2004 - 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module name:
    Rtl81x9NvData.c

Abstract:

--*/

#include "Rtl81x9.h"
#include "Rtl81x9Dev.h"

EFI_STATUS
EFIAPI
Rtl81x9Snp_NvData (
  IN EFI_SIMPLE_NETWORK_PROTOCOL *This,
  IN BOOLEAN                     Read,
  IN UINTN                       RegOffset,
  IN UINTN                       NumBytes,
  IN OUT VOID                    *BufferPtr
  )
/*++

Routine Description:
 This is an interface call provided by SNP.
 It does the basic checking on the input parameters and retrieves snp structure
 and then calls the read_nvdata() call which does the actual reading

Arguments:
  This      - context pointer
  Read      - true for reading and false for writing
  RegOffset - eeprom register relative to the base
  NumBytes  - how many bytes to read
  BufferPtr - address of memory to read into

Returns:

--*/
{
  RTL81X9_SNP_DRIVER  *Snp;

  Snp = RTL81X9_PRIVATE_FROM_EFI_SIMPLE_NETWORK (This);
  
  switch (Snp->Mode.State) {
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
  // Return error if non-volatile memory variables are not valid.
  //
  if (Snp->Mode.NvRamSize == 0 || Snp->Mode.NvRamAccessSize == 0) {
    return EFI_UNSUPPORTED;
  }
  //
  // Check for invalid parameter combinations.
  //
  if ((NumBytes == 0) ||
      (BufferPtr == NULL) ||
      (RegOffset >= Snp->Mode.NvRamSize) ||
      (RegOffset + NumBytes > Snp->Mode.NvRamSize) ||
      (NumBytes % Snp->Mode.NvRamAccessSize != 0) ||
      (RegOffset % Snp->Mode.NvRamAccessSize != 0)
      ) {
    return EFI_INVALID_PARAMETER;
  }
  //
  // check the implementation flags of undi if we can write the nvdata!
  //
  if (Read) {
    return Rtl81x9EepromRead (Snp, RegOffset, NumBytes, BufferPtr);
  } else {
    return Rtl81x9EepromWrite (Snp, RegOffset, NumBytes, BufferPtr);
  }  
}
