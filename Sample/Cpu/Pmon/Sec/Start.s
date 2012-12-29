/*++

Copyright (c) 2009, kontais                                                                   
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  Start.s

Abstract:


--*/

#include "Mips.h"

  .text
  .set    noreorder

  .globl  start
start:
  
  BRANCH_CALL(ArchSecInit)
  BRANCH_CALL(PlatformSecInit)

  li    sp, 0x80FFC000
  nop

  la    v0, SecMain
  jalr  v0
  nop


