/*++

Copyright (c) 2011, kontais
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  PciCfg.c
   
Abstract:

  Tiano PEIM to provide the platform support functionality within Windows

--*/

#include "Tiano.h"
#include "Pei.h"
#include EFI_PPI_DEFINITION (PciCfg)
#include "PciCfgLib.h"

EFI_STATUS
PciCfgRead (
  IN EFI_PEI_SERVICES         **PeiServices,
  IN PEI_PCI_CFG_PPI          * This,
  IN PEI_PCI_CFG_PPI_WIDTH    Width,
  IN UINT64                   Address,
  IN OUT VOID                 *Buffer
  )
{
  UINT32 RdData;
  UINT32 Shift;

  if (Width >= PeiPciCfgWidthUint64) {
    return EFI_UNSUPPORTED;
  }

  Shift = ((UINT32)Address & 3) << 3;

  RdData = BonitoPciRead(Address);
  RdData = RdData >> Shift;
  
  switch (Width) {
    case PeiPciCfgWidthUint8:
      *(UINT8*)Buffer = (UINT8)RdData;
      break;
    case PeiPciCfgWidthUint16:
      *(UINT16*)Buffer = (UINT16)RdData;
      break;
    case PeiPciCfgWidthUint32:
      *(UINT32*)Buffer = (UINT32)RdData;
      break;
    default:
      return EFI_INVALID_PARAMETER;
  }
  return EFI_SUCCESS;
}

EFI_STATUS
PciCfgWrite (
  IN EFI_PEI_SERVICES         **PeiServices,
  IN PEI_PCI_CFG_PPI          * This,
  IN PEI_PCI_CFG_PPI_WIDTH    Width,
  IN UINT64                   Address,
  IN OUT VOID                 *Buffer
  )
{
  UINT32 RdData;
  UINT32 WrData;
  UINT32 Mask;
  UINT32 Shift;

  if (Width >= PeiPciCfgWidthUint64) {
    return EFI_UNSUPPORTED;
  }

  Shift = (1 << Width) << 3;
  Mask  = ~(0xffffffff << Shift);

  Shift = ((UINT32)Address & 3) << 3;

  RdData = BonitoPciRead(Address);
  RdData = RdData & Mask;
  
  switch (Width) {
    case PeiPciCfgWidthUint8:
      WrData = *(UINT8*) Buffer;
      break;
    case PeiPciCfgWidthUint16:
      WrData = *(UINT16*) Buffer;
      break;
    case PeiPciCfgWidthUint32:
      WrData = *(UINT32*) Buffer;
      break;
    default:
      return EFI_INVALID_PARAMETER;
  }

  WrData = (WrData << Shift) | RdData;
  BonitoPciWrite (Address, WrData);      

  return EFI_SUCCESS;
}

EFI_STATUS
PciCfgModify (
  IN EFI_PEI_SERVICES         **PeiServices,
  IN PEI_PCI_CFG_PPI          * This,
  IN PEI_PCI_CFG_PPI_WIDTH    Width,
  IN UINT64                   Address,
  IN UINTN                    SetBits,
  IN UINTN                    ClearBits
  )
{
  EFI_STATUS  Status;
  UINTN       Data;

  if (Width >= PeiPciCfgWidthUint64) {
    return EFI_UNSUPPORTED;
  }

  Status = PciCfgRead (
             PeiServices,
             This,
             Width,
             Address,
             &Data
             );

  Data = SetBits | (Data & (~ClearBits));

  Status = PciCfgWrite (
             PeiServices,
             This,
             Width,
             Address,
             &Data
             );

  return Status;
}

//
// Module globals
//
PEI_PCI_CFG_PPI mPciCfgPpi = 
{
  PciCfgRead, PciCfgWrite, PciCfgModify
};

EFI_PEI_PPI_DESCRIPTOR  mPpiListPciCfg = {
  (EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
  &gPeiPciCfgPpiInServiceTableGuid,
  &mPciCfgPpi
};


EFI_STATUS
EFIAPI
InitializePciCfg (
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

  Status = (*PeiServices)->InstallPpi(PeiServices, &mPpiListPciCfg);
  (*PeiServices)->PciCfg = &mPciCfgPpi;

  return Status;
}

