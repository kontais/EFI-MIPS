/*++

Copyright (c) 2005 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:
    PcatPciRootBridgeIo.c
    
Abstract:

    EFI PC AT PCI Root Bridge Io Protocol

Revision History

--*/

#include "PcatPciRootBridge.h"
#include "Pci22.h"
#include "ArchDefs.h"   // For CACHED_TO_PHYS
#include "CpuMips32.h"  // For MipsCacheSync
//
// Protocol Member Function Prototypes
//
EFI_STATUS
PcatRootBridgeIoPollMem ( 
  IN  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL        *This,
  IN  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH  Width,
  IN  UINT64                                 Address,
  IN  UINT64                                 Mask,
  IN  UINT64                                 Value,
  IN  UINT64                                 Delay,
  OUT UINT64                                 *Result
  );
  
EFI_STATUS
PcatRootBridgeIoPollIo ( 
  IN  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL        *This,
  IN  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH  Width,
  IN  UINT64                                 Address,
  IN  UINT64                                 Mask,
  IN  UINT64                                 Value,
  IN  UINT64                                 Delay,
  OUT UINT64                                 *Result
  );
  
EFI_STATUS
PcatRootBridgeIoMemRead (
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL        *This,
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH  Width,
  IN     UINT64                                 Address,
  IN     UINTN                                  Count,
  IN OUT VOID                                   *Buffer
  );

EFI_STATUS
PcatRootBridgeIoMemWrite (
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL        *This,
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH  Width,
  IN     UINT64                                 Address,
  IN     UINTN                                  Count,
  IN OUT VOID                                   *Buffer
  );

EFI_STATUS
PcatRootBridgeIoIoRead (
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL        *This,
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH  Width,
  IN     UINT64                                 UserAddress,
  IN     UINTN                                  Count,
  IN OUT VOID                                   *UserBuffer
  );

EFI_STATUS
PcatRootBridgeIoIoWrite (
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL        *This,
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH  Width,
  IN     UINT64                                 UserAddress,
  IN     UINTN                                  Count,
  IN OUT VOID                                   *UserBuffer
  );

EFI_STATUS
PcatRootBridgeIoCopyMem (
  IN EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL        *This,
  IN EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH  Width,
  IN UINT64                                 DestAddress,
  IN UINT64                                 SrcAddress,
  IN UINTN                                  Count
  );

EFI_STATUS
PcatRootBridgeIoPciRead (
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL        *This,
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH  Width,
  IN     UINT64                                 Address,
  IN     UINTN                                  Count,
  IN OUT VOID                                   *Buffer
  );

EFI_STATUS
PcatRootBridgeIoPciWrite (
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL        *This,
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH  Width,
  IN     UINT64                                 Address,
  IN     UINTN                                  Count,
  IN OUT VOID                                   *Buffer
  );

EFI_STATUS
PcatRootBridgeIoMap (
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL            *This,
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_OPERATION  Operation,
  IN     VOID                                       *HostAddress,
  IN OUT UINTN                                      *NumberOfBytes,
  OUT    EFI_PHYSICAL_ADDRESS                       *DeviceAddress,
  OUT    VOID                                       **Mapping
  );

EFI_STATUS
PcatRootBridgeIoUnmap (
  IN EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL  *This,
  IN VOID                             *Mapping
  );

EFI_STATUS
PcatRootBridgeIoAllocateBuffer (
  IN  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL  *This,
  IN  EFI_ALLOCATE_TYPE                Type,
  IN  EFI_MEMORY_TYPE                  MemoryType,
  IN  UINTN                            Pages,
  OUT VOID                             **HostAddress,
  IN  UINT64                           Attributes
  );

EFI_STATUS
PcatRootBridgeIoFreeBuffer (
  IN  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL  *This,
  IN  UINTN                            Pages,
  OUT VOID                             *HostAddress
  );

EFI_STATUS
PcatRootBridgeIoFlush (
  IN EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL  *This
  );

EFI_STATUS
PcatRootBridgeIoGetAttributes (
  IN  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL  *This,
  OUT UINT64                           *Supported,
  OUT UINT64                           *Attributes
  );

EFI_STATUS
PcatRootBridgeIoSetAttributes (
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL  *This,
  IN     UINT64                           Attributes,
  IN OUT UINT64                           *ResourceBase,
  IN OUT UINT64                           *ResourceLength 
  ); 

EFI_STATUS
PcatRootBridgeIoConfiguration (
  IN  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL  *This,
  OUT VOID                             **Resources
  );

//
// Private Function Prototypes
//
STATIC 
EFI_STATUS
EFIAPI
PcatRootBridgeIoMemRW (
  IN  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH  Width,
  IN  UINTN                                  Count,
  IN  BOOLEAN                                InStrideFlag,
  IN  PTR                                    In,
  IN  BOOLEAN                                OutStrideFlag,
  OUT PTR                                    Out
  );

EFI_STATUS
PcatRootBridgeIoConstructor (
	IN EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL  *Protocol,
  IN UINTN                            SegmentNumber
  )
/*++

Routine Description:

    Contruct the Pci Root Bridge Io protocol

Arguments:

    Protocol - protocol to initialize
    
Returns:

    None

--*/
{
  Protocol->ParentHandle   = NULL;

  Protocol->PollMem        = PcatRootBridgeIoPollMem;
  Protocol->PollIo         = PcatRootBridgeIoPollIo;

  Protocol->Mem.Read       = PcatRootBridgeIoMemRead;
  Protocol->Mem.Write      = PcatRootBridgeIoMemWrite;

  Protocol->Io.Read        = PcatRootBridgeIoIoRead;
  Protocol->Io.Write       = PcatRootBridgeIoIoWrite;

  Protocol->CopyMem        = PcatRootBridgeIoCopyMem;

  Protocol->Pci.Read       = PcatRootBridgeIoPciRead;
  Protocol->Pci.Write      = PcatRootBridgeIoPciWrite;

  Protocol->Map            = PcatRootBridgeIoMap;
  Protocol->Unmap          = PcatRootBridgeIoUnmap;

  Protocol->AllocateBuffer = PcatRootBridgeIoAllocateBuffer;
  Protocol->FreeBuffer     = PcatRootBridgeIoFreeBuffer;

  Protocol->Flush          = PcatRootBridgeIoFlush;

  Protocol->GetAttributes  = PcatRootBridgeIoGetAttributes;
  Protocol->SetAttributes  = PcatRootBridgeIoSetAttributes;

  Protocol->Configuration  = PcatRootBridgeIoConfiguration;

  Protocol->SegmentNumber  = (UINT32)SegmentNumber;

  return EFI_SUCCESS;
}

EFI_STATUS
PcatRootBridgeIoPollMem ( 
  IN  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL        *This,
  IN  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH  Width,
  IN  UINT64                                 Address,
  IN  UINT64                                 Mask,
  IN  UINT64                                 Value,
  IN  UINT64                                 Delay,
  OUT UINT64                                 *Result
  )
{
  EFI_STATUS  Status;
  UINT64      NumberOfTicks;
  UINTN       Remainder;

  if (Result == NULL) {
    return EFI_INVALID_PARAMETER;
  }


  if (Width < 0 || Width > EfiPciWidthUint64) {
    return EFI_INVALID_PARAMETER;
  }
  //
  // No matter what, always do a single poll.
  //
  Status = This->Mem.Read (This, Width, Address, 1, Result);
  if ( EFI_ERROR(Status) ) {
    return Status;
  }    
  if ( (*Result & Mask) == Value ) {
    return EFI_SUCCESS;
  }

  if ( Delay != 0 ) {

    NumberOfTicks = DivU64x32 (Delay, 100, &Remainder);
    if ( Remainder !=0 ) {
      NumberOfTicks += 1;
    }
    NumberOfTicks += 1;
  
    while ( NumberOfTicks ) {

      gBS->Stall (10);

      Status = This->Mem.Read (This, Width, Address, 1, Result);
      if ( EFI_ERROR(Status) ) {
        return Status;
      }
    
      if ( (*Result & Mask) == Value ) {
        return EFI_SUCCESS;
      }

      NumberOfTicks -= 1;
    }
  }
  return EFI_TIMEOUT;
}
  
EFI_STATUS
PcatRootBridgeIoPollIo ( 
  IN  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL        *This,
  IN  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH  Width,
  IN  UINT64                                 Address,
  IN  UINT64                                 Mask,
  IN  UINT64                                 Value,
  IN  UINT64                                 Delay,
  OUT UINT64                                 *Result
  )
{
  EFI_STATUS  Status;
  UINT64      NumberOfTicks;
  UINTN       Remainder;

  if (Result == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (Width < 0 || Width > EfiPciWidthUint64) {
    return EFI_INVALID_PARAMETER;
  }
  //
  // No matter what, always do a single poll.
  //
  Status = This->Io.Read (This, Width, Address, 1, Result);
  if ( EFI_ERROR(Status) ) {
    return Status;
  }    
  if ( (*Result & Mask) == Value ) {
    return EFI_SUCCESS;
  }

  if (Delay != 0) {

    NumberOfTicks = DivU64x32 (Delay, 100, &Remainder);
    if ( Remainder !=0 ) {
      NumberOfTicks += 1;
    }
    NumberOfTicks += 1;
  
    while ( NumberOfTicks ) {

      gBS->Stall(10);
    
      Status = This->Io.Read (This, Width, Address, 1, Result);
      if ( EFI_ERROR(Status) ) {
        return Status;
      }
    
      if ( (*Result & Mask) == Value ) {
        return EFI_SUCCESS;
      }

      NumberOfTicks -= 1;
    }
  }
  return EFI_TIMEOUT;
}

BOOLEAN
PcatRootBridgeMemAddressValid (
  IN PCAT_PCI_ROOT_BRIDGE_INSTANCE  *PrivateData,
  IN UINT64                         Address
  )
{
  if ((Address >= PrivateData->PciExpressBaseAddress) && (Address < PrivateData->PciExpressBaseAddress + 0x10000000)) {
    return TRUE;
  }
  if ((Address >= PrivateData->MemBase) && (Address < PrivateData->MemLimit)) {
    return TRUE;
  }

  return FALSE;
}

EFI_STATUS
PcatRootBridgeIoMemRead (
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL        *This,
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH  Width,
  IN     UINT64                                 Address,
  IN     UINTN                                  Count,
  IN OUT VOID                                   *Buffer
  )
{
  PCAT_PCI_ROOT_BRIDGE_INSTANCE  *PrivateData;
  UINTN                          AlignMask;
  PTR                            In;
  PTR                            Out;

  if ( Buffer == NULL ) {
    return EFI_INVALID_PARAMETER;
  }
  
  PrivateData = DRIVER_INSTANCE_FROM_PCI_ROOT_BRIDGE_IO_THIS(This);

  if (!PcatRootBridgeMemAddressValid (PrivateData, Address)) {
    return EFI_INVALID_PARAMETER;
  }

  AlignMask = (1 << (Width & 0x03)) - 1;
  if (Address & AlignMask) {
    return EFI_INVALID_PARAMETER;
  }

  Address += PrivateData->PhysicalMemoryBase;

  In.buf  = Buffer;
  Out.buf = (VOID *)(UINTN) Address;
  if (Width >= EfiPciWidthUint8 && Width <= EfiPciWidthUint64) {
    return PcatRootBridgeIoMemRW (Width, Count, TRUE, In, TRUE, Out);
  }
  if (Width >= EfiPciWidthFifoUint8 && Width <= EfiPciWidthFifoUint64) {
    return PcatRootBridgeIoMemRW (Width, Count, TRUE, In, FALSE, Out);
  }
  if (Width >= EfiPciWidthFillUint8 && Width <= EfiPciWidthFillUint64) {
    return PcatRootBridgeIoMemRW (Width, Count, FALSE, In, TRUE, Out);
  }

  return EFI_INVALID_PARAMETER;
}

EFI_STATUS
PcatRootBridgeIoMemWrite (
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL        *This,
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH  Width,
  IN     UINT64                                 Address,
  IN     UINTN                                  Count,
  IN OUT VOID                                   *Buffer
  )
{
  PCAT_PCI_ROOT_BRIDGE_INSTANCE *PrivateData;
  UINTN  AlignMask;
  PTR    In;
  PTR    Out;

  if ( Buffer == NULL ) {
    return EFI_INVALID_PARAMETER;
  }
  
  PrivateData = DRIVER_INSTANCE_FROM_PCI_ROOT_BRIDGE_IO_THIS(This);

  if (!PcatRootBridgeMemAddressValid (PrivateData, Address)) {
    return EFI_INVALID_PARAMETER;
  }

  AlignMask = (1 << (Width & 0x03)) - 1;
  if (Address & AlignMask) {
    return EFI_INVALID_PARAMETER;
  }

  Address += PrivateData->PhysicalMemoryBase;

  In.buf  = (VOID *)(UINTN) Address;
  Out.buf = Buffer;
  if (Width >= EfiPciWidthUint8 && Width <= EfiPciWidthUint64) {
    return PcatRootBridgeIoMemRW (Width, Count, TRUE, In, TRUE, Out);
  }
  if (Width >= EfiPciWidthFifoUint8 && Width <= EfiPciWidthFifoUint64) {
    return PcatRootBridgeIoMemRW (Width, Count, FALSE, In, TRUE, Out);
  }
  if (Width >= EfiPciWidthFillUint8 && Width <= EfiPciWidthFillUint64) {
    return PcatRootBridgeIoMemRW (Width, Count, TRUE, In, FALSE, Out);
  }

  return EFI_INVALID_PARAMETER;
}

EFI_STATUS
PcatRootBridgeIoCopyMem (
  IN EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL        *This,
  IN EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH  Width,
  IN UINT64                                 DestAddress,
  IN UINT64                                 SrcAddress,
  IN UINTN                                  Count
  )

{
  EFI_STATUS  Status;
  BOOLEAN     Direction;
  UINTN       Stride;
  UINTN       Index;
  UINT64      Result;

  if (Width < 0 || Width > EfiPciWidthUint64) {
    return EFI_INVALID_PARAMETER;
  }       

  if (DestAddress == SrcAddress) {
    return EFI_SUCCESS;
  }

  Stride = 1 << Width;

  Direction = TRUE;
  if ((DestAddress > SrcAddress) && (DestAddress < (SrcAddress + Count * Stride))) {
    Direction   = FALSE;
    SrcAddress  = SrcAddress  + (Count-1) * Stride;
    DestAddress = DestAddress + (Count-1) * Stride;
  }

  for (Index = 0;Index < Count;Index++) {
    Status = PcatRootBridgeIoMemRead (
               This,
               Width,
               SrcAddress,
               1,
               &Result
               );
    if (EFI_ERROR (Status)) {
      return Status;
    }
    Status = PcatRootBridgeIoMemWrite (
               This,
               Width,
               DestAddress,
               1,
               &Result
               );
    if (EFI_ERROR (Status)) {
      return Status;
    }
    if (Direction) {
      SrcAddress  += Stride;
      DestAddress += Stride;
    } else {
      SrcAddress  -= Stride;
      DestAddress -= Stride;
    }
  }
  return EFI_SUCCESS;
}

EFI_STATUS
PcatRootBridgeIoPciRead (
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL        *This,
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH  Width,
  IN     UINT64                                 Address,
  IN     UINTN                                  Count,
  IN OUT VOID                                   *Buffer
  )
{
  if (Buffer == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  return PcatRootBridgeIoPciRW (This, FALSE, Width, Address, Count, Buffer);
}

EFI_STATUS
PcatRootBridgeIoPciWrite (
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL        *This,
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH  Width,
  IN     UINT64                                 Address,
  IN     UINTN                                  Count,
  IN OUT VOID                                   *Buffer
  )
{
  if (Buffer == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  
  return PcatRootBridgeIoPciRW (This, TRUE, Width, Address, Count, Buffer);
}

EFI_STATUS
PcatRootBridgeIoMap (
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL            *This,
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_OPERATION  Operation,
  IN     VOID                                       *HostAddress,
  IN OUT UINTN                                      *NumberOfBytes,
  OUT    EFI_PHYSICAL_ADDRESS                       *DeviceAddress,
  OUT    VOID                                       **Mapping
  )

{
  EFI_STATUS            Status;
  EFI_PHYSICAL_ADDRESS  PhysicalAddress;
  MAP_INFO              *MapInfo;
  MAP_INFO_INSTANCE    *MapInstance;
  PCAT_PCI_ROOT_BRIDGE_INSTANCE *PrivateData;

  if ( HostAddress == NULL || NumberOfBytes == NULL || 
       DeviceAddress == NULL || Mapping == NULL ) {
    
    return EFI_INVALID_PARAMETER;
  }

  PrivateData = DRIVER_INSTANCE_FROM_PCI_ROOT_BRIDGE_IO_THIS(This);
  
  //
  // Perform a fence operation to make sure all memory operations are flushed
  //
  MEMORY_FENCE();

  //
  // Initialize the return values to their defaults
  //
  *Mapping = NULL;

  //
  // Make sure that Operation is valid
  //
  if (Operation < 0 || Operation >= EfiPciOperationMaximum) {
    return EFI_INVALID_PARAMETER;
  }

  switch (Operation) {
  case EfiPciOperationBusMasterCommonBuffer:
    //
    // This config must be change according to Loongson 2F address window registers setting
    // PCI 0x80000000 to RAM 0x00000000 is power on default
    // PCI (0x0000_0000_8000_0000 - 0x0000_0000_8fff_ffff) -> DDR 
    // (0x0000_0000_0000_0000 - 0x0000_0000_ffff_ffff)  PCI WIN0
    //
    PhysicalAddress = (EFI_PHYSICAL_ADDRESS)(CACHED_TO_PHYS((UINTN)HostAddress));
    
    if ((PhysicalAddress + *NumberOfBytes) > 0x10000000)  {
      return EFI_UNSUPPORTED;
    }
    else {
      //
      // The transfer is below 256M
      //
      *DeviceAddress = PhysicalAddress + 0x80000000;
    }
    break;
  //
  // HostAddres is pci view pci space
  // DeviceAddress is cpu view pci memory space
  //
  case EfiPciOperationBusMasterRead:
    *DeviceAddress = PrivateData->PhysicalMemoryBase + HostAddress;
    break;
    
  default:
    break;
  }

  //
  // Perform a fence operation to make sure all memory operations are flushed
  //
  MEMORY_FENCE();

  return EFI_SUCCESS;
}

EFI_STATUS
PcatRootBridgeIoUnmap (
  IN EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL  *This,
  IN VOID                             *Mapping
  )

{
  MAP_INFO    *MapInfo;
  PCAT_PCI_ROOT_BRIDGE_INSTANCE *PrivateData;
  EFI_LIST_ENTRY *Link;

  //
  // Perform a fence operation to make sure all memory operations are flushed
  //
  MEMORY_FENCE();

  PrivateData = DRIVER_INSTANCE_FROM_PCI_ROOT_BRIDGE_IO_THIS(This);
  //
  // See if the Map() operation associated with this Unmap() required a mapping buffer.
  // If a mapping buffer was not required, then this function simply returns EFI_SUCCESS.
  //
  if (Mapping != NULL) {
    return EFI_UNSUPPORTED;
  }

  //
  // Perform a fence operation to make sure all memory operations are flushed
  //
  MEMORY_FENCE();

  return EFI_SUCCESS;
}

EFI_STATUS
PcatRootBridgeIoAllocateBuffer (
  IN  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL  *This,
  IN  EFI_ALLOCATE_TYPE                Type,
  IN  EFI_MEMORY_TYPE                  MemoryType,
  IN  UINTN                            Pages,
  OUT VOID                             **HostAddress,
  IN  UINT64                           Attributes
  )
{
  EFI_STATUS            Status;
  EFI_PHYSICAL_ADDRESS  PhysicalAddress;

  //
  // Validate Attributes
  //
  if (Attributes & EFI_PCI_ATTRIBUTE_INVALID_FOR_ALLOCATE_BUFFER) {
    return EFI_UNSUPPORTED;
  }

  //
  // Check for invalid inputs
  //
  if (HostAddress == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // The only valid memory types are EfiBootServicesData and EfiRuntimeServicesData
  //
  if (MemoryType != EfiBootServicesData && MemoryType != EfiRuntimeServicesData) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Limit allocations to memory below 4GB
  //
  PhysicalAddress = (EFI_PHYSICAL_ADDRESS)(0xffffffff);

  Status = gBS->AllocatePages (AllocateMaxAddress, MemoryType, Pages, &PhysicalAddress);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  *HostAddress = (VOID *)(UINTN)PhysicalAddress;

  return EFI_SUCCESS;
}

EFI_STATUS
PcatRootBridgeIoFreeBuffer (
  IN  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL  *This,
  IN  UINTN                            Pages,
  OUT VOID                             *HostAddress
  )

{

  if( HostAddress == NULL ){
  	 return EFI_INVALID_PARAMETER;
  } 
  return gBS->FreePages ((EFI_PHYSICAL_ADDRESS)(UINTN)HostAddress, Pages);
}

EFI_STATUS
PcatRootBridgeIoFlush (
  IN EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL  *This
  )

{
  //
  // Perform a fence operation to make sure all memory operations are flushed
  //
  MEMORY_FENCE();

  MipsCacheSync();

  return EFI_SUCCESS;
}

EFI_STATUS
PcatRootBridgeIoGetAttributes (
  IN  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL  *This,
  OUT UINT64                           *Supported,  OPTIONAL
  OUT UINT64                           *Attributes
  )

{
  PCAT_PCI_ROOT_BRIDGE_INSTANCE *PrivateData;

  PrivateData = DRIVER_INSTANCE_FROM_PCI_ROOT_BRIDGE_IO_THIS(This);

  if (Attributes == NULL && Supported == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Supported is an OPTIONAL parameter.  See if it is NULL
  //
  if (Supported) {
    //
    // This is a generic driver for a PC-AT class system.  It does not have any
    // chipset specific knowlegde, so none of the attributes can be set or 
    // cleared.  Any attempt to set attribute that are already set will succeed, 
    // and any attempt to set an attribute that is not supported will fail.
    //
    *Supported = PrivateData->Attributes;
  }

  //
  // Set Attrbutes to the attributes detected when the PCI Root Bridge was initialized
  //
  
  if (Attributes) {
    *Attributes = PrivateData->Attributes;
  }
  
   
  return EFI_SUCCESS;
}

EFI_STATUS
PcatRootBridgeIoSetAttributes (
  IN     EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL  *This,
  IN     UINT64                           Attributes,
  IN OUT UINT64                           *ResourceBase,
  IN OUT UINT64                           *ResourceLength 
  )

{
  PCAT_PCI_ROOT_BRIDGE_INSTANCE   *PrivateData;
  
  PrivateData = DRIVER_INSTANCE_FROM_PCI_ROOT_BRIDGE_IO_THIS(This);

  //
  // This is a generic driver for a PC-AT class system.  It does not have any
  // chipset specific knowlegde, so none of the attributes can be set or 
  // cleared.  Any attempt to set attribute that are already set will succeed, 
  // and any attempt to set an attribute that is not supported will fail.
  //
  if (Attributes & (~PrivateData->Attributes)) {
    return EFI_UNSUPPORTED;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
PcatRootBridgeIoConfiguration (
  IN  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL  *This,
  OUT VOID                             **Resources
  )

{
  PCAT_PCI_ROOT_BRIDGE_INSTANCE   *PrivateData;
  
  PrivateData = DRIVER_INSTANCE_FROM_PCI_ROOT_BRIDGE_IO_THIS(This);

  *Resources = PrivateData->Configuration;

  return EFI_SUCCESS;
}

//
// Internal function
//

STATIC 
EFI_STATUS
EFIAPI
PcatRootBridgeIoMemRW (
  IN  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH  Width,
  IN  UINTN                                  Count,
  IN  BOOLEAN                                InStrideFlag,
  IN  PTR                                    In,
  IN  BOOLEAN                                OutStrideFlag,
  OUT PTR                                    Out
  )
/*++

Routine Description:

  Private service to provide the memory read/write

Arguments:

  Width of the Memory Access
  Count of the number of accesses to perform

Returns:

  Status

  EFI_SUCCESS           - Successful transaction
  EFI_INVALID_PARAMETER - Unsupported width and address combination

--*/
{
  UINTN  Stride;
  UINTN  InStride;
  UINTN  OutStride;


  Width     = Width & 0x03;
  Stride    = 1 << Width;
  InStride  = InStrideFlag  ? Stride : 0;
  OutStride = OutStrideFlag ? Stride : 0;

  //
  // Loop for each iteration and move the data
  //
  switch (Width) {
  case EfiPciWidthUint8:
    for (;Count > 0; Count--, In.buf += InStride, Out.buf += OutStride) {
      MEMORY_FENCE();
      *In.ui8 = *Out.ui8;
      MEMORY_FENCE();
    }
    break;
  case EfiPciWidthUint16:
    for (;Count > 0; Count--, In.buf += InStride, Out.buf += OutStride) {
      MEMORY_FENCE();
      *In.ui16 = *Out.ui16;
      MEMORY_FENCE();
    }
    break;
  case EfiPciWidthUint32:
    for (;Count > 0; Count--, In.buf += InStride, Out.buf += OutStride) {
      MEMORY_FENCE();
      *In.ui32 = *Out.ui32;
      MEMORY_FENCE();
    }
    break;
  default:
    return EFI_INVALID_PARAMETER;
  }

  return EFI_SUCCESS;
}

