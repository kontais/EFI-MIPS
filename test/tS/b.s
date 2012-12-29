/*++

Copyright (c) 2009, kontais                                                                   
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  InitMem.s

Abstract:


--*/

#include "Mips.h"
#include "AsmMacro.h"

  .text
  .align 2
  .globl InitMem
  .ent   InitMem
  .type  InitMem, @function

InitMem:

  .set noreorder
  .set nomacro

  li v0, 2

  JMP_t9_BACK(InitMem)

  .set macro
  .set reorder

  .end InitMem

