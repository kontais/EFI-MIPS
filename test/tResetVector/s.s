/*++

Copyright (c) 2009, kontais                                                                   
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  sw.s

Abstract:

  Stack switching routine for Linux emulation.

--*/

#include "ArchDefs.h"

  .text
  .align 3
  .global sw
  .ent    sw
  .type   sw, @function

sw:
 /*------------------------------------------------------------------------------
 *  VOID
 *  sw (
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

  lui   v0, 0x8765
  ori   v0, v0, 0x4321
  jal   v0
  nop

  .set reorder

  .end sw
