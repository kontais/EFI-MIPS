/*++

Copyright (c) 2011, kontais                                                                   
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  RegInit.s

Abstract:


--*/

#include "Mips.h"
#include "RegInit.h"

  .text
  .align 2
  .globl RegInit
  .ent  RegInit
  .type  RegInit, @function

RegInit:
/*++

Invoke: UINT32 RegInit(UINT32 RegInitTable);

Input:  a0  UINT32 RegInitTable  - Register Init Data Table

Output: None

Return: Exit Status

Saves:

Modifies:

Description: Device Register Init for assemble code.

--*/
  .set noreorder

  lw    t3, REG_INIT_ITEM_FIELD0(a0)
  lw    t0, REG_INIT_ITEM_FIELD1(a0)
  and   t4, t3, REG_INIT_OP_MASK

  //
  // Exit Opration
  //
  bne   t4, REG_INIT_OP_EXIT,  8f
  nop
  move   v0, t0
  b     RegInitExit
  nop

8:
  //
  // Delay Opration
  //
  bne   t4, REG_INIT_OP_DELAY, 8f
  nop

1:
  subu  t0, 1
  bnez  t0, 1b
  nop
  b     RegInitNextItem 
  nop

8:
  //
  // Read Opration
  //
  bne   t4, REG_INIT_OP_READ, 8f
  nop
  and   t4, t3, REG_INIT_MOD_MASK

  // Byte read
  bne   t4, REG_INIT_MOD_BYTE, 1f
  nop
  lbu   t5, 0(t0)
  b     RegInitNextItem
  nop

  // Halfword read
1:
  bne   t4, REG_INIT_MOD_HALFWORD, 1f
  nop
  lhu   t5, 0(t0)
  b     RegInitNextItem
  nop

  // Word read
1:
  bne   t4, REG_INIT_MOD_WORD, 1f
  nop
  lw    t5, 0(t0)
  b     RegInitNextItem
  nop
1:
  // Not support DWORD read
  b     RegInitError
  nop

8:
  //
  // Write Opration
  //
  bne   t4, REG_INIT_OP_WRITE, 8f
  nop
  lw    t1, REG_INIT_ITEM_FIELD2(a0)
  and   t4, t3, REG_INIT_MOD_MASK

  bne   t4, REG_INIT_MOD_BYTE, 1f
  nop
  sb    t1, 0(t0)
  b     RegInitNextItem
  nop
1:
  bne   t4, REG_INIT_MOD_HALFWORD, 1f
  nop
  sh    t1, 0(t0)
  b     RegInitNextItem
  nop
1:
  bne   t4, REG_INIT_MOD_WORD, 1f
  nop
  sw    t1, 0(t0)
  b     RegInitNextItem
  nop
1:
  // Not support DWORD write
  b     RegInitError
  nop 

8:
  //
  // Read Modify Write Opration
  //
  bne    t4, REG_INIT_OP_RMW, 8f
  nop
  lw     t1, REG_INIT_ITEM_FIELD2(a0)
  lw     t2, REG_INIT_ITEM_FIELD3(a0)
  and    t4, t3, REG_INIT_MOD_MASK

  bne    t4, REG_INIT_MOD_BYTE, 1f
  nop
  lbu    t4, 0(t0)
  and    t4, t1
  or     t4, t2
  sb     t4, 0(t0)
  b      RegInitNextItem
  nop
1:
  bne    t4, REG_INIT_MOD_HALFWORD, 1f
  nop
  lhu    t4, 0(t0)
  and    t4, t1
  or     t4, t2
  sh     t4, 0(t0)
  b      RegInitNextItem
  nop
1:
  bne    t4, REG_INIT_MOD_WORD, 1f
  nop
  lw     t4, 0(t0)
  and    t4, t1
  or     t4, t2
  sw     t4, 0(t0)
  b      RegInitNextItem
  nop
1:
  // Not support DWORD RMW
  b      RegInitError
  nop

8:
  //
  // Wait Opration
  //
  bne    t4, REG_INIT_OP_WAIT, 8f
  nop
  lw     t1, REG_INIT_ITEM_FIELD2(a0)
  lw     t2, REG_INIT_ITEM_FIELD3(a0)
  and    t4, t3, REG_INIT_MOD_MASK
  
  bne    t4, REG_INIT_MOD_BYTE, 1f
  nop
3:
  lbu    t4, 0(t0)
  and    t4, t1
  bne    t4, t2, 3b
  nop
  b      RegInitNextItem
  nop
1: 
  bne    t4, REG_INIT_MOD_HALFWORD, 1f
  nop
3:
  lhu    t4, 0(t0)
  and    t4, t1
  bne    t4, t2, 3b
  nop
  b      RegInitNextItem
  nop
1:
  bne    t4, REG_INIT_MOD_WORD, 1f
  nop
3:
  lw     t4, 0(t0)
  and    t4, t1
  bne    t4, t2, 3b
  nop
  b      RegInitNextItem
  nop
1:
  // Not support DWORD 
  b      RegInitError
  nop

RegInitNextItem:
  addu  a0, REG_INIT_ITEM_SIZE
  b     RegInit 
  nop

8:
RegInitError:
  lw    v0, REG_INIT_ITEM_FIELD0(a0)
  nop

RegInitExit:
  jr    ra
  nop


  .set reorder

  .end RegInit


