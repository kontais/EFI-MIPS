/*++

Copyright (c) 2009, kontais                                                                   
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  memset.s

Abstract:

  This is the code that supports MIPS32-optimized SetMem service

--*/

#include "Mips.h"

  .text
  .align 2
  .globl memset
  .ent   memset
  .type  memset, @function

memset:
/*++

Invoke: VOID *memset(VOID *Buffer, UINTN Count, UINT8 Value);

Input:  a0   VOID   *Buffer - Pointer to buffer to clear
        a1   UINTN  Count   - Number of bytes to clear
        a2   UINT8  Value   - Value to write

Output: None.

Saves:

Modifies:

Description:  This function is an optimized set-memory function.

Notes:  This function tries to zero memory 4 bytes at a time. As a result,
        it first picks up any misaligned bytes, then words, before getting
        in the main loop that does the 4-byte clears.

--*/
  .set noreorder
  .set nomacro

  move t2, a1
  move t5, a0
  move t6, a2


  // 扩展到4个字节
  andi t6, 0xff
  move t7, t6
  sll  t7, 8
  or   t7, t6
  move t6, t7
  sll  t6, 16
  or   t6, t7

  // 填写开始不是4字节对齐的地址
_StartByteSet:
  move t0, t5
  andi t0, 3
  beqz t0, _SetBlocks      // 已经是对齐的
  nop
  beqz t2, _SetMemDone
  nop

  // 字节存
  sb t6, 0(t5)
  addiu t5, 1
  addiu t2, -1
  b _StartByteSet
  nop

_SetBlocks:

  // 计算有多少个32字节
  move t3, t2
  srl  t2, 5            // 转换为32字节的个数
  sll  t2, 5            // 再转为字节个数
  subu t3, t2           // t3保存着余数
  srl  t2, 5            // t2保存着32的倍数

  beqz t2, _SetRemaining
  nop
  
_B:
  sw t6, 0(t5)       // 存32字节
  sw t6, 4(t5)
  sw t6, 8(t5)
  sw t6, 12(t5)
  sw t6, 16(t5)
  sw t6, 20(t5)
  sw t6, 24(t5)
  sw t6, 28(t5)

  addiu t5, 32
  addiu t2, -1
  bnez t2, _B
  nop

_SetRemaining:

  // 余下不足32字节地址
  move t2, t3            // t3为除32的余数
  srl  t2, 2

_SetRemainWord:
  beqz t2, _SetRemainByte
  nop
  sw t6, 0(t5)
  addiu t5, 4
  addiu t2, -1
  b _SetRemainWord
  nop

_SetRemainByte: 
  move t2, t3
  andi t2, 3
  
_Byte:                    // 最后几个字节
  beqz t2, _SetMemDone
  nop
  sb t6,0(t5)
  addiu t5, 1
  addiu t2, -1
  b _Byte
  nop

_SetMemDone:
  move v0, a0
  jr ra
  nop

  .set macro
  .set reorder

  .end memset
