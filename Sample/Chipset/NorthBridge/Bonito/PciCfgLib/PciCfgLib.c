/*++

Copyright (c) 2011, kontais                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  PciCfgLib.c
  
Abstract:
  
--*/

#include "EfiCommon.h"
#include "Mips.h"
#include "BonitoReg.h"
#include "Pci.h"

#define RESERVED_DEV  (11)

VOID
PciBreakAddress (
  IN  UINT64   Address, 
  OUT UINT32   *Bus, 
  OUT UINT32   *Dev,
  OUT UINT32   *Fun,
  OUT UINT32   *Reg
  )
{
  *Bus = (((UINTN)Address) >> 24) & 0xFF;
  *Dev = (((UINTN)Address) >> 16) & 0xFF;
  *Fun = (((UINTN)Address) >> 8)  & 0xFF;
  *Reg = (((UINTN)Address) >> 0)  & 0xFF;  
}

VOID
BonitoPciGetAddressType (
  IN  UINT64   Address, 
  OUT UINT32   *CfgAddr, 
  OUT UINT32   *CfgType
  )
{
  UINT32 Bus, Dev, Fun, Reg;

  PciBreakAddress (Address, &Bus, &Dev, &Fun, &Reg);

  if (Bus == 0) {
    *CfgAddr = (1 << (Dev + RESERVED_DEV)) | (Fun << 8) | Reg;
    *CfgType = 0x00000000;
  } else {
    *CfgAddr = (Bus << 16) | (Dev << RESERVED_DEV) | (Fun << 8) | Reg;
    *CfgType = 0x00010000;
  }
}

UINT32
BonitoPciRead (
  IN  UINT64  Address
  )
{
  UINT32 Data;
  UINT32 Addr;
  UINT32 Type;
  UINT32 AbortBit;

  BonitoPciGetAddressType(Address, &Addr, &Type);  

  AbortBit = EFI_PCI_STATUS_MASTER_ABORT | EFI_PCI_STATUS_MASTER_TARGET_ABORT;

  // clear aborts
  REGVAL(BONITO_PCI_REG(PCI_COMMAND_OFFSET)) |= AbortBit;

  // high 16 bits of address go into PciMapCfg register
  REGVAL(BONITO_PCIMAP_CFG) = (Addr >> 16) | Type;
  (VOID)REGVAL(BONITO_PCIMAP_CFG);

  // low 16 bits of address are offset into config space
  Data = REGVAL(BONITO_PCICFG_BASE | (Addr & 0xfffc));
  
  // check for error
  if (REGVAL(BONITO_PCI_REG(PCI_COMMAND_OFFSET)) & AbortBit) {
    REGVAL(BONITO_PCI_REG(PCI_COMMAND_OFFSET)) |= AbortBit;
    Data = (UINT32) -1;
  }

  return Data;
}

VOID
BonitoPciWrite (
  IN  UINT64  Address,
  IN  UINT32  Data
  )
{
  UINT32 Addr;
  UINT32 Type;
  UINT32 AbortBit;

  AbortBit = EFI_PCI_STATUS_MASTER_ABORT | EFI_PCI_STATUS_MASTER_TARGET_ABORT;

  BonitoPciGetAddressType(Address, &Addr, &Type);

  // clear aborts
  REGVAL(BONITO_PCI_REG(PCI_COMMAND_OFFSET)) |= AbortBit;

  // high 16 bits of address go into PciMapCfg register
  REGVAL(BONITO_PCIMAP_CFG) = (Addr >> 16) | Type;
  (VOID)REGVAL(BONITO_PCIMAP_CFG);

  // low 16 bits of address are offset into config space
  REGVAL(BONITO_PCICFG_BASE | (Addr & 0xfffc)) = Data;

  // clear error
  if (REGVAL(BONITO_PCI_REG(PCI_COMMAND_OFFSET)) & AbortBit) {
    REGVAL(BONITO_PCI_REG(PCI_COMMAND_OFFSET)) |= AbortBit;
  }
  
  return;
}

