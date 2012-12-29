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

#include "RegDef.h"

#define REGSZ 4

  .text
  .align 3
  .globl setjmp
  .ent   setjmp
  .type  setjmp, @function

setjmp:
/*++
int
setjmp( jmp_buf env )

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

  sw    ra,  REGSZ * 0 (a0)    // ra
  sw    sp,  REGSZ * 1 (a0)    // sp
  sw    s0,  REGSZ * 2 (a0)    // s0
  sw    s1,  REGSZ * 3 (a0)    // s1
  sw    s2,  REGSZ * 4 (a0)    // s2
  sw    s3,  REGSZ * 5 (a0)    // s3
  sw    s4,  REGSZ * 6 (a0)    // s4
  sw    s5,  REGSZ * 7 (a0)    // s5
  sw    s6,  REGSZ * 8 (a0)    // s6
  sw    s7,  REGSZ * 9 (a0)    // s7
  sw    fp, REGSZ * 10 (a0)    // fp/s8
  sw    gp, REGSZ * 11 (a0)    // gp

  move  v0, zero
  jr    ra
  nop

  .set reorder
  .end setjmp


  .globl longjmp
  .ent   longjmp
  .type  longjmp, @function

longjmp:
/*++
VOID
longjmp ( jmp_buf env, int val )

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


  lw    ra,  REGSZ * 0 (a0)    // ra
  lw    sp,  REGSZ * 1 (a0)    // sp
  lw    s0,  REGSZ * 2 (a0)    // s0
  lw    s1,  REGSZ * 3 (a0)    // s1
  lw    s2,  REGSZ * 4 (a0)    // s2
  lw    s3,  REGSZ * 5 (a0)    // s3
  lw    s4,  REGSZ * 6 (a0)    // s4
  lw    s5,  REGSZ * 7 (a0)    // s5
  lw    s6,  REGSZ * 8 (a0)    // s6
  lw    s7,  REGSZ * 9 (a0)    // s7
  lw    fp, REGSZ * 10 (a0)    // fp/s8
  lw    gp, REGSZ * 11 (a0)    // gp

  move  v0, a1
  jr    ra
  nop

  .set reorder
  .end longjmp


