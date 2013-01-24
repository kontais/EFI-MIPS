/*++
Copyright (c) 2012, kontais               
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  Unaligned.s

Abstract:


--*/

#include "Mips.h"

  .text
  .align 3
  .set   noreorder

/*------------------------------------------------------------------------------
 * EFI_STATUS
 * UnalignedAccess (
 *   IN UINT32        OpCode,
 *   IN UINT32        Addr,
 *   IN OUT UINT32    *Data
 *   )
 * ;
 * 
 *-----------------------------------------------------------------------------*/
  .globl UnalignedAccess
  .ent   UnalignedAccess
  .type  UnalignedAccess, @function

UnalignedAccess:
  li    t0, 35     // OP_LW
  bne   t0, a0, 1f
  nop
  lwl   v0, 3(a1)
  lwr   v0, 0(a1)
  b     2f
  sw    v0, 0(a2)
1:
  li    t0, 43      // OP_SW
  bne   t0, a0, 1f
  nop
  lw    v0, 0(a2)
  swl   v0, 3(a1)
  swr   v0, (a1)
  b     2f
  nop
1:
  li    t0, 37     // OP_LHU
  bne   t0, a0, 1f
  nop
  lbu   v0, 1(a1)
  lbu   v1, 0(a1)
  sll   v0, 0x8
  or    v0, v1
  b     2f
  sw    v0, 0(a2)
1:
  li    t0, 41      // OP_SH
  bne   t0, a0, 1f
  nop
  lw    v0, 0(a2)
  sb    v0, 0(a1)
  srl   v0, 0x08
  sb    v0, 1(a1)
  b     2f
  nop
1:
  li    t0, 33      // OP_LH
  bne   t0, a0, 3f
  nop
  lb    v0, 1(a1)
  lbu   v1, 0(a1)
  sll   v0, 0x8
  or    v0, v1
  sw    v0, 0(a2)
2:
  jr    ra
  move  v0, zero      // EFI_SUCCESS

3:
  lui   v0, 0x8000
  jr    ra
  ori   v0, v0, 3     // EFI_UNSUPPORTED

  .end UnalignedAccess   

  
