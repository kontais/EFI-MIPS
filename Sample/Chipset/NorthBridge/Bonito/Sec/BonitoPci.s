/*++

Copyright (c) 2011, kontais
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

Module Name:

   BonitoPci.s

Abstract:

  Bonito PCI routine.

--*/



#include "Mips.h"
#include "BonitoReg.h"
#include "BonitoPci.h"


  .text
  .align 3
  .set noreorder

  .globl PciRead32
  .ent   PciRead32
  .type  PciRead32, @function


PciRead32:
/*++

Invoke:

  UINT32 PciRead32(UINT32 Address)

Routine Description:

  Bonito internal device pci read

Arguments:

  Address - BONITO_PCI_CFG_ADDR

Returns:

  Value to read

--*/

  srl   v0, a0, 16
  li    v1, PHYS_TO_UNCACHED(BONITO_PCIMAP_CFG)
  sw    v0, (v1)
  lw    zero, (v1)

  li    v1, PHYS_TO_UNCACHED(BONITO_PCICFG_BASE)
  andi  v0, a0, 0xfffc
  or    v1, v0
  lw    v0, (v1)
  nop

  jr    ra
  nop

  .end PciRead32

  .globl PciWrite32
  .ent   PciWrite32
  .type  PciWrite32, @function


PciWrite32:
/*++

Invoke:

  UINT32 PciWrite32(UINT32 Address, UINT32 Data)

Routine Description:

  Bonito internal device pci write

Arguments:

  Address - BONITO_PCI_CFG_ADDR
  Data    - data to write

Returns:

  None

--*/

  srl   v0, a0, 16
  li    v1, PHYS_TO_UNCACHED(BONITO_PCIMAP_CFG)
  sw    v0, (v1)
  lw    zero, (v1)

  li    v1, PHYS_TO_UNCACHED(BONITO_PCICFG_BASE)
  andi  v0, a0, 0xfffc
  or    v1, v0
  sw    a1, (v1)
  nop

  jr    ra
  nop

  .end PciWrite32

  .set reorder


