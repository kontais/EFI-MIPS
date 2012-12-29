/*++

Copyright (c) 2012, kontais                                                                   
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  PcatIoHooks.c

Abstract:


--*/
#include "PcatPciRootBridge.h"
#include "Pci22.h"
#include "BonitoReg.h"
#include "ArchDefs.h"

#include "Cs5536Lib.h"
#include "PciCfgLib.h"


UINT32 BonitoPciCfgRead(UINT64 UserAddress)
{
  UINT32 Data;
  
  if (EFI_SUCCESS == Cs5536PciRead(UserAddress, &Data)) {
    return Data;
  }

  return BonitoPciRead(UserAddress);
}

VOID BonitoPciCfgWrite(UINT64 UserAddress, UINT32 Data)
{
  if (EFI_SUCCESS == Cs5536PciWrite(UserAddress, Data)) {
    return ;
  }

  BonitoPciWrite(UserAddress, Data);
}


EFI_STATUS
BonitoRootBridgeIoPciRW (
  IN BOOLEAN                                Write,
  IN EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH  Width,
  IN UINT64                                 UserAddress,
  IN UINTN                                  Count,
  IN OUT VOID                               *UserBuffer
  )
{
  UINT32    InStride;
  UINT32    OutStride;
  UINT32    Data;
  UINT32    RdData;
  UINT32    WrData;
  UINT32    Shift;
  UINT32    Mask;
  UINT64    Mask64;
  PTR       Buffer;

  if (Width < 0 || Width >= EfiPciWidthMaximum) {
    return EFI_INVALID_PARAMETER;
  }
  
  if ((Width & 0x03) >= EfiPciWidthUint64) {
    return EFI_INVALID_PARAMETER;
  }
  
  InStride    = 1 << (Width & 0x03);
  OutStride   = InStride;
  if (Width >= EfiCpuIoWidthFifoUint8 && Width <= EfiCpuIoWidthFifoUint64) {
    InStride = 0;
  }

  if (Width >= EfiCpuIoWidthFillUint8 && Width <= EfiCpuIoWidthFillUint64) {
    OutStride = 0;
  }

  Width = Width & 0x03;
  Shift = (1 << Width) << 3;

  Mask64 = 0xffffffff;
  Mask64 = ~(Mask64 << Shift);
  
  Mask  = (UINT32) Mask64;
 
  while (Count) {
    Buffer.buf = UserBuffer;
    Shift = ((UINT32)UserAddress & 3) << 3;
    if (Write) {
      RdData = BonitoPciCfgRead(UserAddress);
      Mask = ~(Mask << Shift);
      RdData = RdData & Mask;
      
      switch (Width) {
        case EfiPciWidthUint8:
          WrData = *Buffer.ui8;
          break;
        case EfiPciWidthUint16:
          WrData = *Buffer.ui16;
          break;
        case EfiPciWidthUint32:
          WrData = *Buffer.ui32;
          break;
      }
      
      WrData = (WrData << Shift) | RdData;
      BonitoPciCfgWrite (UserAddress, WrData);      
    } else {
    
      RdData = BonitoPciCfgRead(UserAddress);
      WrData = RdData >> Shift;
      
      switch (Width) {
        case EfiPciWidthUint8:
          *Buffer.ui8 = (UINT8)WrData;
          break;
        case EfiPciWidthUint16:
          *Buffer.ui16 = (UINT16)WrData;
          break;
        case EfiPciWidthUint32:
          *Buffer.ui32 = WrData;
          break;
      }
    }
    
    UserAddress += InStride;
    UserBuffer = ((UINT8 *)UserBuffer) + OutStride;
    Count -= 1;
  }
  
  return EFI_SUCCESS;
}

