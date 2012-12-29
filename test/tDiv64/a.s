/*++

Copyright (c) 2009, kontais                                                                   
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  DivU64x32.s

Abstract:


--*/

#include "Mips.h"
#include "AsmMacro.h"

  .text
  .align 2
  .globl DivU64x32
  .ent   DivU64x32
  .type  DivU64x32, @function

DivU64x32:
  .set noreorder

  dsll  a0, 32
  dsrl  a0, 32
  dsll  a1, 32
  daddu a1, a0

  ddivu v0, a1, a2
  mfhi  t0

  daddu v1, v0, zero
  dsrl  v1, 32

  beqz  a3, 1f
  nop
  sw    t0, 0(a3)
  nop
1:

  jr  ra
  nop

  .set reorder
  .end DivU64x32

  .align 2
  .globl MultU64x32
  .ent   MultU64x32
  .type  MultU64x32, @function

MultU64x32:

  .set noreorder

  dsll  a0, 32
  dsrl  a0, 32
  dsll  a1, 32
  daddu a1, a0

  dmultu a1, a2
  mflo  v0

  daddu v1, v0, zero
  dsrl  v1, 32

  jr  ra
  nop

  .set reorder
  .end MultU64x32

  .align 2
  .globl LShiftU64
  .ent   LShiftU64
  .type  LShiftU64, @function

LShiftU64:

  .set noreorder

  dsll  a0, 32
  dsrl  a0, 32
  dsll  a1, 32
  daddu a1, a0

  dsll  v0, a1, a2

  daddu v1, v0, zero
  dsrl  v1, 32

  jr  ra
  nop

  .set reorder
  .end LShiftU64

  .align 2
  .globl RShiftU64
  .ent   RShiftU64
  .type  RShiftU64, @function

RShiftU64:

  .set noreorder

  dsll  a0, 32
  dsrl  a0, 32
  dsll  a1, 32
  daddu a1, a0

  dsrl  v0, a1, a2

  daddu v1, v0, zero
  dsrl  v1, 32

  jr  ra
  nop

  .set reorder
  .end RShiftU64

  .align 2
  .globl Log2
  .ent   Log2
  .type  Log2, @function

Log2:

  .set noreorder

  li    v0, 0xff

  or    t0, a0, a1
  beqz  t0, 3f
  nop

  dsll  a0, 32
  dsrl  a0, 32
  dsll  a1, 32
  daddu a1, a0

  move  v0, zero

1:
  dsrl  a0, a1, 32
  or    a0, a1
  beqz  a0, 2f
  nop
  dsrl  a1, a1, 1
  add   v0, 1
  b     1b
  nop
  
2:
  addiu   v0, -1

3:

  jr  ra
  nop

  .set reorder
  .end Log2

  .align 2
  .globl GetPowerOfTwo
  .ent   GetPowerOfTwo
  .type  GetPowerOfTwo, @function

GetPowerOfTwo:

  .set noreorder

  move  v0, zero
  move  v1, zero

  or    t0, a0, a1
  beqz  t0, 5f
  nop

  dsll  a0, 32
  dsrl  a0, 32
  dsll  a1, 32
  daddu a1, a0

  move  v0, zero

1:
  dsrl  a0, a1, 32
  or    a0, a1
  beqz  a0, 2f
  nop
  dsrl  a1, a1, 1
  addiu v0, 1
  b     1b
  nop
  
2:
  daddu a1, 1
  addu  v0, -1

3:
  beqz  v0, 4f
  nop
  dsll  a1, 1
  addiu v0, -1
  b  3b
  nop

4:

  daddu  v0, a1, zero
  daddu  v1, a1, zero
  dsrl   v1, 32

5:

  jr  ra
  nop

  .set reorder
  .end GetPowerOfTwo

