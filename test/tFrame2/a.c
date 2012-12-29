/*++

Copyright (c) 2009, kontais                                                                   
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  SecMain.s

Abstract:


--*/

#include "regdef.h"
#include "regnum.h"
#include "AsmMacro.h"
#include "cpu.h"

  .text
  .align 3
  .globl SecMain
  .ent   SecMain
  .type  SecMain, @function

SecMain:

  .set noreorder
  .set nomacro

  nop
  nop

  .set macro
  .set reorder

  .end SecMain


//NNON_LEAF(k_intr, FRAMESZ(KERN_EXC_FRAME_SIZE), ra)
  .align 3
  .globl k_intr
  .ent   k_intr
  .type  k_intr, @function
k_intr:

  .set  noat
  PTR_SUB k0, sp, FRAMESZ(KERN_EXC_FRAME_SIZE)
  SAVE_CPU(k0, CF_RA_OFFS)
  .set  at
  move  sp, k0      # Already on kernel stack
  LA  gp, _gp
  and t0, a1, ~(SR_COP_1_BIT | SR_EXL | SR_INT_ENAB | SR_KSU_MASK)
  mtc0  t0, CP0_Status
  ITLBNOPFIX
  ITLBNOPFIX      # XXX necessary?
  PTR_S a0, 0(sp)
  jal interrupt
  PTR_S a3, CF_RA_OFFS + KERN_REG_SIZE(sp)

  PTR_L a0, CF_RA_OFFS + KERN_REG_SIZE(sp)
  .set  noat
  RESTORE_CPU(sp, CF_RA_OFFS)
  PTR_ADDU sp, sp, FRAMESZ(KERN_EXC_FRAME_SIZE)
  sync
  eret
  .set  at
END(k_intr)

