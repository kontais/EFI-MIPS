/*++

Copyright (c) 2009, kontais                                                                   
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  SecSwitchStacks.s

Abstract:

  Stack switching routine for Linux emulation.

--*/

#include "Mips.h"

  .text
  .align 3
  .global SecSwitchStacks
  .ent    SecSwitchStacks
  .type   SecSwitchStacks, @function

SecSwitchStacks:
 /*------------------------------------------------------------------------------
 *  VOID
 *  SecSwitchStacks (
 *    IN VOID                       *EntryPoint,
 *    IN EFI_PEI_STARTUP_DESCRIPTOR *PeiStartup,
 *    IN VOID                       *NewStack,
 *    IN VOID                       *NewBsp
 *    )
 *  
 *   Routine Description:
 *   
 *     Stack switching routine for Winnt emulation.
 *  
 *   Arguments:
 *  
 *     EntryPoint - Entry point with new stack.
 *     PeiStartup - PEI startup descriptor for new entry point.
 *     NewStack   - Pointer to new stack.
 *     NewBsp     - Pointer to new BSP.
 *    
 *   Returns:
 *  
 *     None
 *  
 *----------------------------------------------------------------------------*/
  .set noreorder

  // Save ra,fp to stack. Save sp to fp.
  addiu sp, sp, -32
  sw    ra, 28(sp)
  sw    fp, 24(sp)
  move  fp, sp
  
  move  v0, a0    // v0 = a0  EntryPoint
  move  t1, a1    // t1 = a1  PeiStartup
  move  sp, a2    // sp = a2  NewStack
  move  t3, a3    // t3 = a3  NewBsp
  
  addu  sp, -STAND_ARG_SIZE
  
  // EntryPoint(PeiStartup)
  move  a0, t1
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

  .set reorder

  .end SecSwitchStacks
