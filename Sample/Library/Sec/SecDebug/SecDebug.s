/*++

Copyright (c) 2011, kontais                                                                   
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  SecDebug.s

Abstract:

  Sec Debug Print.

--*/



#include "Mips.h"

  .text
  .align 3
  .set noreorder

  .globl SecOutputByte
  .weak  SecOutputByte
  .ent   SecOutputByte
  .type  SecOutputByte,   @function


  

/*-------------------------------------------------------
 *  函数名:  SecOutputByte
 *  调用  :  void SecOutputByte(char c);
 *  寄存使用:
 *        使用a0,v0,v1. 确保不使用 a1,a2,a3
 *------------------------------------------------------*/
SecOutputByte:

  jr     ra
  nop  
  
 .end SecOutputByte

  .globl SecOutputString
  .ent   SecOutputString
  .type  SecOutputString, @function

/*-------------------------------------------------
 *  函数名:  SecOutputString
 *  调用  :  void SecOutputString(char * s);
 *
 *-----------------------------------------------*/
SecOutputString:

  // 保存ra到a2, 确保 SecOutputByte不使用a1,a2,a3
  move  a2, ra
  move  a1, a0
  lbu   a0, 0(a1)
  nop
1:
  beqz  a0, 2f
  nop
  bal   SecOutputByte
  addiu a1, 1
  b     1b
  lbu   a0, 0(a1)  // branch槽填充加载指令
2:
  jr    a2
  nop

  .end SecOutputString

  .globl SecOutputHex
  .ent   SecOutputHex
  .type  SecOutputHex,    @function

/*-------------------------------------------------
 *  函数名:  SecOutputHex
 *  调用  :  void SecOutputHex(int Hex);
 *  寄存器:
 *           使用a0,a1,a2,a3,v0
 *-----------------------------------------------*/
SecOutputHex:

  // 保存ra到a2, 确保 SecOutputByte不使用a1,a2,a3
  move  a2, ra
  move  a1, a0
  li    a3, 8

1:
  rol   a0, a1, 4
  move  a1, a0       // save in a1

  and   a0, 0xf
  add   a0, 0x30
  sltu  v0, a0, 0x3a // 0x0a + 0x30
  bnez  v0, 2f
  nop
  addu  a0, 0x27     // 0x0a + 0x30 + 0x27 = 0x61('a')
2:
  bal   SecOutputByte
  nop

  addu  a3, -1
  bnez  a3, 1b
  nop

  jr    a2
  nop

  .end SecOutputHex

  .set reorder

