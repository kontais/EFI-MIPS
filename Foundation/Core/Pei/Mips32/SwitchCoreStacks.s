/*++

Copyright (c) 2009, kontais                                                                   
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  SwitchCoreStacks.s

Abstract:

  Core stack switching routine, invoked when real system memory is
   discovered and installed.

--*/

#include "Mips.h"

  .text
  .align 3
  .globl SwitchCoreStacks
  .ent   SwitchCoreStacks
  .type  SwitchCoreStacks, @function

SwitchCoreStacks:
/*++
VOID
SwitchCoreStacks (
  IN VOID  *EntryPoint,
  IN UINTN Parameter1,
  IN UINTN Parameter2,
  IN VOID  *NewStack
  )

Routine Description:

  Routine for PEI switching stacks.

Arguments:

  EntryPoint - Entry point with new stack.
  Parameter1 - First parameter for entry point.
  Parameter2 - Second parameter for entry point.
  NewStack   - Pointer to new stack.
  
Returns:

  None
--*/
  .set noreorder
  .set nomacro

  // Save ra,fp to stack. Save sp to fp.
  addiu sp, sp, -32
  sw    ra, 28(sp)
  sw    fp, 24(sp)
  move  fp, sp
  
  move  v0, a0    // v0 = a0  EntryPoint
  move  t1, a1    // t1 = a1  Parameter1
  move  t2, a2    // t2 = a2  Parameter2
  move  sp, a3    // sp = a3  NewStack
  addu  sp, -STAND_ARG_SIZE
  
  // EntryPoint(Parameter1,Parameter2)
  move  a0, t1
  move  a1, t2
  jalr  v0
  nop
  
  // Restore sp, ra, fp.
  move  sp, fp
  lw    ra, 28(sp)
  lw    fp, 24(sp)
  addiu sp, sp, 32
  
  // Return
  jr    ra
  nop

  .set macro
  .set reorder

  .end SwitchCoreStacks
