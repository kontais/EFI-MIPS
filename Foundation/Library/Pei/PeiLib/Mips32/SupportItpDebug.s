/*++

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

#define REGSZ   4      // 4 Bytes Register Size, 32 bit for MIPS32

  .text
  .align 3
  .globl AsmEfiSetBreakSupport
  .ent   AsmEfiSetBreakSupport
  .type  AsmEfiSetBreakSupport, @function

AsmEfiSetBreakSupport:
/*++
VOID
AsmEfiSetBreakSupport (
  IN UINTN  LoadAddr
  )

Routine Description:

  This is the code for debuging MIPS32, to add a break hook at loading every module

Arguments:

  a0   Load Address
  
Returns:

  EFI_SUCCESS
  
--*/
  .set noreorder

  and   a0, ~3
  or    a0, 6
  mtc0  a0, CP0_WatchLo
  nop
  nop
  nop
  nop
  nop
  nop
  nop
  nop
  nop
  nop
  nop
  nop
  nop
  nop
  nop
  nop
  nop
  nop
  nop
  nop
  nop
  nop
  nop
  nop
  
  jr    ra
  nop

  .set reorder
  .end AsmEfiSetBreakSupport

