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
  .set noreorder

  .align 3
  .globl SecMain
  .ent   SecMain
  .type  SecMain, @function

SecMain:

  .set nomacro

  li v0, 100
  jr ra
  nop
  nop

  .set macro

  .end SecMain


  .align 3
  .globl k_intr
  .ent   k_intr
  .type  k_intr, @function
k_intr:

  li a0, 1
  li a1, 2
  li a2, 9
  li a3, 3
  li v0, 100
  li v1, 200

  .set  noat
  sub k0, sp, FRAMESZ(KERN_EXC_FRAME_SIZE)
  SAVE_CPU_T(k0, CF_RA_OFFS)
  .set  at

  // New Stack
  move  sp, k0
  
  la  gp, _gp

  // Disable Interrutp, Set Kernel Level etc.
  //and t0, a1, ~(SR_COP_1_BIT | SR_EXL | SR_INT_ENAB | SR_KSU_MASK)
  //mtc0  t0, CP0_Status
  //ITLBNOPFIX
  

  // a0 set in SAVE_CPU
  jal interrupt
  nop

  li v0, 300
  
  .set  noat
  RESTORE_CPU_T(sp, CF_RA_OFFS)
  addu sp, sp, FRAMESZ(KERN_EXC_FRAME_SIZE)

#if 1
  jr ra
#else
  //sync
  //eret
#endif

  .set  at

  .end k_intr

