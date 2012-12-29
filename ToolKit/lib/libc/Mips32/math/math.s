/*++

Copyright (c) 2009, kontais                                                                   
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  Math.s

Abstract:


--*/

#include "RegDef.h"

  .text
  .align 2
  .globl LIBC_DivU64x32
  .ent   LIBC_DivU64x32
  .type  LIBC_DivU64x32, @function

LIBC_DivU64x32:
/*
UINT64
LIBC_DivU64x32 (
    IN UINT64   Dividend,
    IN UINTN    Divisor,
    OUT UINTN   *Remainder OPTIONAL
    )
 divide 64bit by 32bit and get a 64bit result
 N.B. only works for 31bit divisors!!
*/
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
  .end LIBC_DivU64x32

  .align 2
  .globl LIBC_MultU64x32
  .ent   LIBC_MultU64x32
  .type  LIBC_MultU64x32, @function

LIBC_MultU64x32:
/*
UINT64
LIBC_MultU64x32 (
    IN UINT64   Multiplicand,
    IN UINTN    Multiplier
    )
 Multiple 64bit by 32bit and get a 64bit result
*/
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
  .end LIBC_MultU64x32

  .align 2
  .globl LIBC_DivU64x64
  .ent   LIBC_DivU64x64
  .type  LIBC_DivU64x64, @function

LIBC_DivU64x64:
/*
UINT64
LIBC_DivU64x64 (
  IN  UINT64   Dividend,
  IN  UINT64   Divisor,
  OUT UINT64   *Remainder OPTIONAL,
  OUT UINT32   *Error
  )
*/
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
  j      LIBC_DivU64x64Exit
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
  
LIBC_DivU64x64Exit:

  move   sp, s8
  lw     s8, 4(sp)
  addiu  sp, sp, 8
  jr     ra
  nop

  .set reorder
  .end LIBC_DivU64x64

  .align 2
  .globl LIBC_MulU64x64
  .ent   LIBC_MulU64x64
  .type  LIBC_MulU64x64, @function

LIBC_MulU64x64:
/*++
UINT64 
LIBC_MulU64x64 (
  UINT64 Value1, 
  UINT64 Value2, 
  UINT64 *ResultHigh
  )


Routine Description:
  
  Multiply two unsigned 64-bit values.

Arguments:

  Value1      - first value to multiply
  Value2      - value to multiply by Value1
  ResultHigh  - result to flag overflows

Returns:

  Value1 * Value2

Note:

  The 128-bit result is the concatenation of *ResultHigh and the return value 
  The product fits in 64 bits if *ResultHigh == 0x0000000000000000

  Method 1. Use four 32-bit multiplications:
      a * b = 2^64 * a_h * b_h + 2^32 * (a_h * b_l + b_h * a_l) + a_l * b_l
  Method 2. An alternative using only three multiplications is:
      a * b = (2^64 + 2^32) * a_h * b_h + 
               2^32 * (a_h - a_l) * (b_h - b_l) + (2^32 + 1) * a_l * b_l
  The first method was implemented, because of the overhead in the second one

--*/

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
  .end LIBC_MulU64x64

  .align 2
  .globl LIBC_LShiftU64
  .ent   LIBC_LShiftU64
  .type  LIBC_LShiftU64, @function

LIBC_LShiftU64:
/*
UINT64
LIBC_LShiftU64 (
    IN UINT64   Operand,
    IN UINTN    Count
    )
Left shift 64bit by 32bit and get a 64bit result
*/
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
  .end LIBC_LShiftU64

  .align 2
  .globl LIBC_RShiftU64
  .ent   LIBC_RShiftU64
  .type  LIBC_RShiftU64, @function

LIBC_RShiftU64:
/*
UINT64
LIBC_RShiftU64 (
    IN UINT64   Operand,
    IN UINTN    Count
    )
 Right shift 64bit by 32bit and get a 64bit result
*/
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
  .end LIBC_RShiftU64

