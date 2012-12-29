/*++

Copyright (c) 2009 - 2011, kontais
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

Module Name:

  BonitoSecInit.s

Abstract:

  Bonito PowerOn init.

--*/



#include "Mips.h"
#include "BonitoReg.h"
#include "SecPci.h"
#include "RegInit.h"
#include "BonitoSecInit.h"


  .text
  .align 3
  .set noreorder

  .globl BonitoSecInit
  .ent   BonitoSecInit
  .type  BonitoSecInit, @function


BonitoSecInit:
/*++

Invoke:

  BRANCH_CALL(BonitoSecInit)

Routine Description:

  None

Arguments:

  None

Returns:

  None

--*/

	la	  a0, BonitoSecInitData
	bal	  RegInit
	nop

  BRANCH_RETURN(BonitoSecInit)
  
  .end BonitoSecInit

  .globl BonitoRomSpeedUp
  .ent   BonitoRomSpeedUp
  .type  BonitoRomSpeedUp, @function

BonitoRomSpeedUp:
/*++

Invoke:

  BRANCH_CALL(BonitoRomSpeedUp)

Routine Description:

  None

Arguments:

  None

Returns:

  None

--*/
  //
  // ROM SpeedUp
  //
  li    t2, PHYS_TO_CACHED(LOONGSON_LIOCFG)
  lw    t1, 0(t2)
  li    a0, 0xffffff83
  and   t1, a0
  ori   t1, 0xc
  sw    t1, 0(t2)
  nop

  li    t2, PHYS_TO_CACHED(LOONGSON_LIOCFG)
  lw    t1, 0(t2)
  li    a0, 0xffffe0ff
  and   t1, a0
  ori   t1, 0x0f00
  sw    t1, 0(t2)

  BRANCH_RETURN(BonitoRomSpeedUp)

  .end BonitoRomSpeedUp


  .set reorder

  .data
  .align 2
  
BonitoSecInitData:

  /* bonito endianess */
  BONITO_BIC(LOONGSON_PONCFG,LOONGSON_PONCFG_CPUBIGEND)
  BONITO_BIC(LOONGSON_GENCFG,LOONGSON_GENCFG_BYTESWAP|LOONGSON_GENCFG_MSTRBYTESWAP)

  
  BONITO_BIS(LOONGSON_PONCFG, LOONGSON_PONCFG_IS_ARBITER)

  /*
   * In certain situations it is possible for the Bonito ASIC
   * to come up with the PCI registers uninitialised, so do them here
   */
  BONITO_INIT(BONITO_PCI_REG(0x08),(PCI_CLASS_BRIDGE << PCI_CLASS_SHIFT) | (PCI_SUBCLASS_BRIDGE_HOST << PCI_SUBCLASS_SHIFT))
  BONITO_INIT(BONITO_PCI_REG(0x04),  PCI_STATUS_PERR_CLR| PCI_STATUS_SERR_CLR| PCI_STATUS_MABORT_CLR| PCI_STATUS_MTABORT_CLR| PCI_STATUS_TABORT_CLR| PCI_STATUS_MPERR_CLR)
  BONITO_INIT(BONITO_PCI_REG(0x0C), 255)
  BONITO_INIT(BONITO_PCI_REG(0x10), 0)
  BONITO_INIT(BONITO_PCI_REG(0x14), 0)
  BONITO_INIT(BONITO_PCI_REG(0x18), 0)
  BONITO_INIT(BONITO_PCI_REG(0x30), 0)
  BONITO_INIT(BONITO_PCI_REG(0x3C), 0)
  BONITO_INIT(LOONGSON_PCI_HIT0_SEL_L,0x8000000c)
  BONITO_INIT(LOONGSON_PCI_HIT0_SEL_H,0xffffffff)
  BONITO_BIS(BONITO_PCI_REG(0x04),  PCI_STATUS_PERRRESPEN)
  BONITO_BIS(BONITO_PCI_REG(0x04), PCI_COMMAND_IO_ENABLE|PCI_COMMAND_MEM_ENABLE|PCI_COMMAND_MASTER_ENABLE)  
  BONITO_BIC(LOONGSON_GENCFG, 0x80)

  /* Set debug mode */
  BONITO_BIS(LOONGSON_GENCFG, LOONGSON_GENCFG_DEBUGMODE)
  REG_INIT_EXIT(0)
  
