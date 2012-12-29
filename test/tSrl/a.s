/*++

Copyright (c) 2009, kontais                                                                   
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  SecMain.s

Abstract:


--*/

#include "Mips.h"
#include "AsmMacro.h"

  .text
  .align 2
  .globl SecMain
  .ent   SecMain
  .type  SecMain, @function

  .set noreorder
  .set nomacro

SecMain:


  li   a0, 0x12345678
  srl  t0,  a0, 16

  move   v0, t0
  move   v0, a0



  jr  ra
  nop


  .end SecMain

  .set macro
  .set reorder

