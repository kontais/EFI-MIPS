/*++

Copyright (c) 2011, kontais
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  CpuIoPei.c
   
Abstract:


--*/

#include "Tiano.h"
#include "Pei.h"
#include "SimpleCpuIoLib.h"

#include EFI_PPI_DEFINITION (CpuIo)

UINT8
PeiCpuIoRead8 (
  IN EFI_PEI_SERVICES    **PeiServices,
  IN PEI_CPU_IO_PPI      * This,
  IN UINT64              Address
  )
{
  return IoRead8 (Address);
}

UINT16
PeiCpuIoRead16 (
  IN EFI_PEI_SERVICES    **PeiServices,
  IN PEI_CPU_IO_PPI      * This,
  IN UINT64              Address
  )
{
  return IoRead16 (Address);
}

UINT32
PeiCpuIoRead32 (
  IN EFI_PEI_SERVICES    **PeiServices,
  IN PEI_CPU_IO_PPI      * This,
  IN UINT64              Address
  )
{
  return IoRead32 (Address);
}

UINT64
PeiCpuIoRead64 (
  IN EFI_PEI_SERVICES    **PeiServices,
  IN PEI_CPU_IO_PPI      * This,
  IN UINT64              Address
  )
{
  // Not support
  EFI_BREAKPOINT ();
}

VOID
PeiCpuIoWrite8 (
  IN EFI_PEI_SERVICES    **PeiServices,
  IN PEI_CPU_IO_PPI      * This,
  IN UINT64              Address,
  IN UINT8               Data
  )
{
  IoWrite8 (Address, Data);
}

VOID
PeiCpuIoWrite16 (
  IN EFI_PEI_SERVICES    **PeiServices,
  IN PEI_CPU_IO_PPI      * This,
  IN UINT64              Address,
  IN UINT16              Data
  )
{
  IoWrite16 (Address, Data);
}

VOID
PeiCpuIoWrite32 (
  IN EFI_PEI_SERVICES    **PeiServices,
  IN PEI_CPU_IO_PPI      * This,
  IN UINT64              Address,
  IN UINT32              Data
  )
{
  IoWrite32 (Address, Data);
}

VOID
PeiCpuIoWrite64 (
  IN EFI_PEI_SERVICES    **PeiServices,
  IN PEI_CPU_IO_PPI      * This,
  IN UINT64              Address,
  IN UINT64              Data
  )
{
  // Not support
  EFI_BREAKPOINT ();
}

UINT8
PeiCpuMemRead8 (
  IN EFI_PEI_SERVICES    **PeiServices,
  IN PEI_CPU_IO_PPI      * This,
  IN UINT64              Address
  )
{
  return MemRead8 (Address);
}

UINT16
PeiCpuMemRead16 (
  IN EFI_PEI_SERVICES    **PeiServices,
  IN PEI_CPU_IO_PPI      * This,
  IN UINT64              Address
  )
{
  return MemRead16 (Address);
}

UINT32
PeiCpuMemRead32 (
  IN EFI_PEI_SERVICES    **PeiServices,
  IN PEI_CPU_IO_PPI      * This,
  IN UINT64              Address
  )
{
  return MemRead32 (Address);
}

UINT64
PeiCpuMemRead64 (
  IN EFI_PEI_SERVICES    **PeiServices,
  IN PEI_CPU_IO_PPI      * This,
  IN UINT64              Address
  )
{
  return MemRead64 (Address);
}

VOID
PeiCpuMemWrite8 (
  IN EFI_PEI_SERVICES    **PeiServices,
  IN PEI_CPU_IO_PPI      * This,
  IN UINT64              Address,
  IN UINT8               Data
  )
{
  MemWrite8 (Address, Data);
}

VOID
PeiCpuMemWrite16 (
  IN EFI_PEI_SERVICES    **PeiServices,
  IN PEI_CPU_IO_PPI      * This,
  IN UINT64              Address,
  IN UINT16              Data
  )
{
  MemWrite16 (Address, Data);
}

VOID
PeiCpuMemWrite32 (
  IN EFI_PEI_SERVICES    **PeiServices,
  IN PEI_CPU_IO_PPI      * This,
  IN UINT64              Address,
  IN UINT32              Data
  )
{
  MemWrite32 (Address, Data);
}

VOID
PeiCpuMemWrite64 (
  IN EFI_PEI_SERVICES    **PeiServices,
  IN PEI_CPU_IO_PPI      * This,
  IN UINT64              Address,
  IN UINT64              Data
  )
{
  MemWrite64 (Address, Data);
}

VOID
PeiCpuIoWidthInc (
  IN PEI_CPU_IO_PPI_WIDTH  Width,
  OUT UINTN                *SrcInc,
  OUT UINTN                *DstInc
  )
{
  *SrcInc = 1 << (Width % 4);
  *DstInc = 1 << (Width % 4);
  switch (Width) {
    case PeiCpuIoWidthFifoUint8:
    case PeiCpuIoWidthFifoUint16:
    case PeiCpuIoWidthFifoUint32:
    case PeiCpuIoWidthFifoUint64:
      *DstInc = 0;
      break;
    case PeiCpuIoWidthFillUint8:
    case PeiCpuIoWidthFillUint16:
    case PeiCpuIoWidthFillUint32:
    case PeiCpuIoWidthFillUint64:
      *SrcInc = 0;
      break;
    default:
      break;
  }
}

EFI_STATUS
PeiCpuIoRead (
  IN EFI_PEI_SERVICES      **PeiServices,
  IN PEI_CPU_IO_PPI        * This,
  IN PEI_CPU_IO_PPI_WIDTH  Width,
  IN UINT64                Address,
  IN UINTN                 Count,
  IN OUT VOID              *Buffer
  )
{
  UINTN Index;
  UINTN SrcInc;
  UINTN DstInc;

  PeiCpuIoWidthInc(Width, &SrcInc, &DstInc);

  switch (Width) {
    case PeiCpuIoWidthUint8:
    case PeiCpuIoWidthFifoUint8:
    case PeiCpuIoWidthFillUint8:
      for (Index = 0; Index < Count; Index++, Address += SrcInc, Buffer = (UINT8*)Buffer + DstInc) {
        *(UINT8*)Buffer = PeiCpuIoRead8 (PeiServices, This, Address);
      }
      break;
    case PeiCpuIoWidthUint16:
    case PeiCpuIoWidthFifoUint16:
    case PeiCpuIoWidthFillUint16:
      for (Index = 0; Index < Count; Index++, Address += SrcInc, Buffer = (UINT8*)Buffer + DstInc) {
        *(UINT16*)Buffer = PeiCpuIoRead16 (PeiServices, This, Address);
      }
      break;
    case PeiCpuIoWidthUint32:
    case PeiCpuIoWidthFifoUint32:
    case PeiCpuIoWidthFillUint32:
      for (Index = 0; Index < Count; Index++, Address += SrcInc, Buffer = (UINT8*)Buffer + DstInc) {
        *(UINT32*)Buffer = PeiCpuIoRead32 (PeiServices, This, Address);
      }
      break;
    case PeiCpuIoWidthUint64:
    case PeiCpuIoWidthFifoUint64:
    case PeiCpuIoWidthFillUint64:
      for (Index = 0; Index < Count; Index++, Address += SrcInc, Buffer = (UINT8*)Buffer + DstInc) {
        *(UINT64*)Buffer = PeiCpuIoRead64 (PeiServices, This, Address);
      }
      break;
    default:
      break;
  }
}

EFI_STATUS
PeiCpuIoWrite (
  IN EFI_PEI_SERVICES      **PeiServices,
  IN PEI_CPU_IO_PPI        * This,
  IN PEI_CPU_IO_PPI_WIDTH  Width,
  IN UINT64                Address,
  IN UINTN                 Count,
  IN OUT VOID              *Buffer
  )
{
  UINTN Index;
  UINTN SrcInc;
  UINTN DstInc;

  PeiCpuIoWidthInc(Width, &SrcInc, &DstInc);

  switch (Width) {
    case PeiCpuIoWidthUint8:
    case PeiCpuIoWidthFifoUint8:
    case PeiCpuIoWidthFillUint8:
      for (Index = 0; Index < Count; Index++, Buffer = (UINT8*)Buffer + SrcInc, Address += DstInc) {
        PeiCpuIoWrite8 (PeiServices, This, Address, *(UINT8*)Buffer);
      }
      break;
    case PeiCpuIoWidthUint16:
    case PeiCpuIoWidthFifoUint16:
    case PeiCpuIoWidthFillUint16:
      for (Index = 0; Index < Count; Index++, Buffer = (UINT8*)Buffer + SrcInc, Address += DstInc) {
        PeiCpuIoWrite16 (PeiServices, This, Address, *(UINT16*)Buffer);
      }
      break;
    case PeiCpuIoWidthUint32:
    case PeiCpuIoWidthFifoUint32:
    case PeiCpuIoWidthFillUint32:
      for (Index = 0; Index < Count; Index++, Buffer = (UINT8*)Buffer + SrcInc, Address += DstInc) {
        PeiCpuIoWrite32 (PeiServices, This, Address, *(UINT32*)Buffer);
      }
      break;
    case PeiCpuIoWidthUint64:
    case PeiCpuIoWidthFifoUint64:
    case PeiCpuIoWidthFillUint64:
      for (Index = 0; Index < Count; Index++, Buffer = (UINT8*)Buffer + SrcInc, Address += DstInc) {
        PeiCpuIoWrite64 (PeiServices, This, Address, *(UINT64*)Buffer);
      }
      break;
    default:
      break;
  }
}

EFI_STATUS
PeiCpuMemRead (
  IN EFI_PEI_SERVICES      **PeiServices,
  IN PEI_CPU_IO_PPI        * This,
  IN PEI_CPU_IO_PPI_WIDTH  Width,
  IN UINT64                Address,
  IN UINTN                 Count,
  IN OUT VOID              *Buffer
  )
{
  UINTN Index;
  UINTN SrcInc;
  UINTN DstInc;

  PeiCpuIoWidthInc(Width, &SrcInc, &DstInc);

  switch (Width) {
    case PeiCpuIoWidthUint8:
    case PeiCpuIoWidthFifoUint8:
    case PeiCpuIoWidthFillUint8:
      for (Index = 0; Index < Count; Index++, Address += SrcInc, Buffer = (UINT8*)Buffer + DstInc) {
        *(UINT8*)Buffer = PeiCpuMemRead8 (PeiServices, This, Address);
      }
      break;
    case PeiCpuIoWidthUint16:
    case PeiCpuIoWidthFifoUint16:
    case PeiCpuIoWidthFillUint16:
      for (Index = 0; Index < Count; Index++, Address += SrcInc, Buffer = (UINT8*)Buffer + DstInc) {
        *(UINT16*)Buffer = PeiCpuMemRead16 (PeiServices, This, Address);
      }
      break;
    case PeiCpuIoWidthUint32:
    case PeiCpuIoWidthFifoUint32:
    case PeiCpuIoWidthFillUint32:
      for (Index = 0; Index < Count; Index++, Address += SrcInc, Buffer = (UINT8*)Buffer + DstInc) {
        *(UINT32*)Buffer = PeiCpuMemRead32 (PeiServices, This, Address);
      }
      break;
    case PeiCpuIoWidthUint64:
    case PeiCpuIoWidthFifoUint64:
    case PeiCpuIoWidthFillUint64:
      for (Index = 0; Index < Count; Index++, Address += SrcInc, Buffer = (UINT8*)Buffer + DstInc) {
        *(UINT64*)Buffer = PeiCpuMemRead64 (PeiServices, This, Address);
      }
      break;
    default:
      break;
  }
}

EFI_STATUS
PeiCpuMemWrite (
  IN EFI_PEI_SERVICES      **PeiServices,
  IN PEI_CPU_IO_PPI        * This,
  IN PEI_CPU_IO_PPI_WIDTH  Width,
  IN UINT64                Address,
  IN UINTN                 Count,
  IN OUT VOID              *Buffer
  )
{
  UINTN Index;
  UINTN SrcInc;
  UINTN DstInc;

  PeiCpuIoWidthInc(Width, &SrcInc, &DstInc);

  switch (Width) {
    case PeiCpuIoWidthUint8:
    case PeiCpuIoWidthFifoUint8:
    case PeiCpuIoWidthFillUint8:
      for (Index = 0; Index < Count; Index++, Buffer = (UINT8*)Buffer + SrcInc, Address += DstInc) {
        PeiCpuMemWrite8 (PeiServices, This, Address, *(UINT8*)Buffer);
      }
      break;
    case PeiCpuIoWidthUint16:
    case PeiCpuIoWidthFifoUint16:
    case PeiCpuIoWidthFillUint16:
      for (Index = 0; Index < Count; Index++, Buffer = (UINT8*)Buffer + SrcInc, Address += DstInc) {
        PeiCpuMemWrite16 (PeiServices, This, Address, *(UINT16*)Buffer);
      }
      break;
    case PeiCpuIoWidthUint32:
    case PeiCpuIoWidthFifoUint32:
    case PeiCpuIoWidthFillUint32:
      for (Index = 0; Index < Count; Index++, Buffer = (UINT8*)Buffer + SrcInc, Address += DstInc) {
        PeiCpuMemWrite32 (PeiServices, This, Address, *(UINT32*)Buffer);
      }
      break;
    case PeiCpuIoWidthUint64:
    case PeiCpuIoWidthFifoUint64:
    case PeiCpuIoWidthFillUint64:
      for (Index = 0; Index < Count; Index++, Buffer = (UINT8*)Buffer + SrcInc, Address += DstInc) {
        PeiCpuMemWrite64 (PeiServices, This, Address, *(UINT64*)Buffer);
      }
      break;
    default:
      break;
  }
}

//
// Module globals
//
PEI_CPU_IO_PPI mCpuIoPpi = 
{
  {PeiCpuMemRead, PeiCpuMemWrite},
  {PeiCpuIoRead,  PeiCpuIoWrite},
  PeiCpuIoRead8,
  PeiCpuIoRead16,
  PeiCpuIoRead32,
  PeiCpuIoRead64,
  PeiCpuIoWrite8,
  PeiCpuIoWrite16,
  PeiCpuIoWrite32,
  PeiCpuIoWrite64,
  PeiCpuMemRead8,
  PeiCpuMemRead16,
  PeiCpuMemRead32,
  PeiCpuMemRead64,
  PeiCpuMemWrite8,
  PeiCpuMemWrite16,
  PeiCpuMemWrite32,
  PeiCpuMemWrite64
};

EFI_PEI_PPI_DESCRIPTOR  mPpiListCpuIo = {
  (EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
  &gPeiCpuIoPpiInServiceTableGuid,
  &mCpuIoPpi
};


EFI_STATUS
EFIAPI
InitializeCpuIoPei (
  IN EFI_FFS_FILE_HEADER       *FfsHeader,
  IN EFI_PEI_SERVICES          **PeiServices
  )
/*++

Routine Description:

  Peform the boot mode determination logic

Arguments:

  PeiServices - General purpose services available to every PEIM.
    
Returns:

  Status -  EFI_SUCCESS if the boot mode could be set

--*/
// TODO:    FfsHeader - add argument and description to function comment
{
  EFI_STATUS  Status;

  Status = (*PeiServices)->InstallPpi(PeiServices, &mPpiListCpuIo);
  (*PeiServices)->CpuIo = &mCpuIoPpi;

  return Status;
}
