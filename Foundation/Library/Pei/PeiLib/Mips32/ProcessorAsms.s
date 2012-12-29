/*++

Copyright (c) 1998-2004 Opsycon AB (www.opsycon.se)
Copyright (c) 2009 Miodrag Vallat.
Copyright (c) 2009, kontais               
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  ProcessorAsms.s

Abstract:

  Core stack switching routine, invoked when real system memory is
   discovered and installed.

--*/

#include "Mips.h"

#define EFI_SUCCESS  0
#define EFI_WARN_RETURN_FROM_LONG_JUMP  5  // EFIWARN (5)

  .text
  .align 3
  .globl TransferControlSetJump
  .ent   TransferControlSetJump
  .type  TransferControlSetJump, @function

TransferControlSetJump:
/*++
EFI_STATUS
TransferControlSetJump (
  IN EFI_PEI_TRANSFER_CONTROL_PROTOCOL  *This,
  IN VOID                               *Context
  )

Routine Description:

  This routine implements the MIPS32 variant of the SetJump call.  Its
  responsibility is to store system state information for a possible
  subsequent LongJump. Regsiter save reference to EfiJump.h.

Arguments:

  a0   This
  a1   Pointer to CPU context save buffer.
  
Returns:

  EFI_SUCCESS
  
--*/
  .set noreorder

  sw    ra,  REGSZ * 0 (a1)    // ra
  sw    sp,  REGSZ * 1 (a1)    // sp
  sw    s0,  REGSZ * 2 (a1)    // s0
  sw    s1,  REGSZ * 3 (a1)    // s1
  sw    s2,  REGSZ * 4 (a1)    // s2
  sw    s3,  REGSZ * 5 (a1)    // s3
  sw    s4,  REGSZ * 6 (a1)    // s4
  sw    s5,  REGSZ * 7 (a1)    // s5
  sw    s6,  REGSZ * 8 (a1)    // s6
  sw    s7,  REGSZ * 9 (a1)    // s7
  sw    fp, REGSZ * 10 (a1)    // fp/s8
  sw    gp, REGSZ * 11 (a1)    // gp
  
  jr    ra
  li    v0, EFI_SUCCESS       //  delay slot, SetJump return value

  .set reorder
  .end TransferControlSetJump


  .globl TransferControlLongJump
  .ent   TransferControlLongJump
  .type  TransferControlLongJump, @function

TransferControlLongJump:
/*++
EFI_STATUS
TransferControlLongJump (
  IN EFI_PEI_TRANSFER_CONTROL_PROTOCOL  *This,
  IN VOID                               *Context
  )

Routine Description:

  This routine implements the MIPS32 variant of the LongJump call.  Its
  responsibility is restore the system state to the Context Buffer and
  pass control back. Regsiter restore reference to EfiJump.h.

Arguments:

  a0   This
  a1   Pointer to CPU context save buffer.
  
Returns:

  EFI_WARN_RETURN_FROM_LONG_JUMP
  
--*/
  .set noreorder


  lw    ra,  REGSZ * 0 (a1)    // ra
  lw    sp,  REGSZ * 1 (a1)    // sp
  lw    s0,  REGSZ * 2 (a1)    // s0
  lw    s1,  REGSZ * 3 (a1)    // s1
  lw    s2,  REGSZ * 4 (a1)    // s2
  lw    s3,  REGSZ * 5 (a1)    // s3
  lw    s4,  REGSZ * 6 (a1)    // s4
  lw    s5,  REGSZ * 7 (a1)    // s5
  lw    s6,  REGSZ * 8 (a1)    // s6
  lw    s7,  REGSZ * 9 (a1)    // s7
  lw    fp, REGSZ * 10 (a1)    // fp/s8
  lw    gp, REGSZ * 11 (a1)    // gp
  
  jr    ra
  li    v0, EFI_WARN_RETURN_FROM_LONG_JUMP   // LongJmp return value

  .set reorder
  .end TransferControlLongJump


  .globl SwitchStacks
  .ent   SwitchStacks
  .type  SwitchStacks, @function

SwitchStacks:
/*++
VOID
SwitchStacks (
  VOID  *EntryPoint,
  UINTN Parameter,
  VOID  *NewStack,
  VOID  *NewBsp
  )

Routine Description:

  Routine for PEI switching stacks.

Arguments:

    EntryPoint      - Pointer to the location to enter
    Parameter       - Parameter to pass in
    NewStack        - New Location of the stack
    NewBsp          - New BSP
  
Returns:

  None
--*/
  .set noreorder

  // Save ra,fp to stack. Save sp to fp.
  addiu sp, sp, -32
  sw    ra, 28(sp)
  sw    fp, 24(sp)
  move  fp, sp
  
  move  v0, a0    // v0 = a0  EntryPoint
  move  t1, a1    // t1 = a1  Parameter
  move  sp, a2    // sp = a2  NewStack
  move  t3, a3    // t3 = a3  NewBsp
  
  addu  sp, -STAND_ARG_SIZE
  
  // EntryPoint(Parameter)
  move  a0, t1
  jalr  v0
  nop
  
  // Restore sp, ra, fp.
  move  sp, fp
  lw    ra, 28(sp)
  lw    fp, 24(sp)
  addiu sp, sp, 32
  
  // Return
  jr  ra
  nop

  .set reorder

  .end SwitchStacks






  .globl SwitchIplStacks
  .ent   SwitchIplStacks
  .type  SwitchIplStacks, @function

SwitchIplStacks:
/*++
VOID
SwitchIplStacks (
  VOID  *EntryPoint,
  UINTN Parameter1,
  UINTN Parameter2,
  VOID  *NewStack,
  VOID  *NewBsp
  )

Routine Description:

  This allows the caller to switch the stack and goes to the new entry point

Arguments:

    EntryPoint              - Pointer to the location to enter
    Parameter1/Parameter2   - Parameter to pass in
    NewStack                - New Location of the stack
    NewBsp                  - New BSP
  
Returns:

  None
--*/
  .set noreorder

  // Save ra,fp to stack. Save sp to fp.
  addiu sp,sp,-32
  sw    ra, 28(sp)
  sw    fp, 24(sp)
  move  fp, sp
  
  move  v0, a0              // v0 = a0  EntryPoint
  move  t1, a1              // t1 = a1  Parameter1
  move  t2, a2              // t2 = a2  Parameter2
  move  sp, a3              // sp = a3  NewStack
  addu  sp, -STAND_ARG_SIZE
  lw    t4, 48(fp)          // t4 = 48(fp) NewBsp
  
  // EntryPoint(Parameter)
  move  a0, t1
  move  a1, t2
  jalr  v0
  nop
  
  // Restore sp, ra, fp.
  move  sp, fp
  lw    ra, 28(sp)
  lw    fp, 24(sp)
  addiu sp, sp,32
  
  // Return
  jr    ra
  nop

  .set reorder

  .end SwitchIplStacks

