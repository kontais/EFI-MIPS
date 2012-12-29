/*++

Copyright (c) 2009, kontais                                                                   
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  DivU64x64.s

Abstract:


--*/

#include "Mips.h"
#include "AsmMacro.h"

  .text
  .align 2
  .globl DivU64x64
  .ent   DivU64x64
  .type  DivU64x64, @function

DivU64x64:
  .set noreorder


  addiu  sp, sp, -8
  sw     s8, 4(sp)
  move   s8, sp


  sw     a0, 8(s8)
  sw     a1, 12(s8)
  sw     a2, 16(s8)
  sw     a3, 20(s8)
  
  //
  // *Error = 0
  //
  lw     v0, 28(s8)
  sw     zero, 0(v0)
  
  //
  // Divisor == 0
  //
  lw     v0, 16(s8)
  lw     v1, 20(s8)
  or     v0, v0,v1
  bnez   v0, DivisorNoZero
  nop
  
  //
  // *Error = 1
  //
  lw     v0, 28(s8)
  li     v1, 1
  sw     v1, 0(v0)
  
  //
  // Remainder ?= NULL
  //
  lw     v0, 24(s8)
  beqz   v0, RemainderZero
  nop
  
  // Remainder != NULL, Remainder = 0x8000000000000000;
  lw     a0, 24(s8)
  lui    v1, 0x8000
  move   v0, zero
  sw     v0, 0(a0)
  sw     v1, 4(a0)

  // Remainder == NULL, Return = 0x8000000000000000;
RemainderZero:
  lui    v1, 0x8000
  move   v0, zero
  j      DivU64x64Exit
  nop
  
DivisorNoZero:

  lw     a0, 8(s8)
  lw     a1, 12(s8)
  lw     a2, 16(s8)
  lw     a3, 20(s8)
  nop
  
  
  dsll   a0, 32
  dsrl   a0, 32
  dsll   a1, 32
  daddu  a1, a0

  dsll   a2, 32
  dsrl   a2, 32
  dsll   a3, 32
  daddu  a3, a2

  ddivu  a1, a3

  //
  // Remainder ?= NULL
  //
  lw     v0, 24(s8)
  beqz   v0, SaveLow
  nop
  
  //
  // Save Remainder
  //
  mfhi   a0
  daddu  a1, a0, zero
  dsrl   a1, 32
  lw     a2, 24(s8)
  sw     a0, 0(a2)
  sw     a1, 4(a2)  
  
SaveLow:
  //
  // Low
  //
  mflo   v0
  daddu  v1, v0, zero
  dsrl   v1, 32
  
DivU64x64Exit:

  move   sp, s8
  lw     s8, 4(sp)
  addiu  sp, sp, 8
  jr     ra
  nop

  .set reorder
  .end DivU64x64

  .align 2
  .globl MultU64x64
  .ent   MultU64x64
  .type  MultU64x64, @function

MultU64x64:

  .set noreorder

  addiu  sp, sp,-8
  sw     s8, 4(sp)
  move   s8, sp

  dsll   a0, 32
  dsrl   a0, 32
  dsll   a1, 32
  daddu  a1, a0

  dsll   a2, 32
  dsrl   a2, 32
  dsll   a3, 32
  daddu  a3, a2

  dmultu a1, a3

  //
  // Hi
  //
  mfhi   a0
  daddu  a1, a0, zero
  dsrl   a1, 32
  lw     a2, 24(s8)
  sw     a0, 0(a2)
  sw     a1, 4(a2)

  //
  // Low
  //
  mflo   v0
  daddu  v1, v0, zero
  dsrl   v1, 32

  move   sp, s8
  lw     s8, 4(sp)
  addiu  sp, sp, 8
  jr     ra
  nop

  .set reorder
  .end MultU64x64

