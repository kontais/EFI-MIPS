/*++

Copyright (c) 2011, kontais                                                                   
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  ArchSecInit.s

Abstract:

  Arch Startup Init Code.

--*/



#include "Mips.h"

  .text
  .align 3
  .globl ArchSecInit
  .ent   ArchSecInit
  .type  ArchSecInit, @function

ArchSecInit:
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Invoke: 
  
  BRANCH_CALL(ArchSecInit)

Routine Description:

  None

Arguments:

  None

Returns:

  None

----------------------------------------------------------------*/

  .set noreorder
  
  mtc0  zero, CP0_Cause
  //li    t0, CP0_STATUS_BEV
  //mtc0  t0, CP0_Status

  
  BRANCH_RETURN(ArchSecInit)

  .set reorder
  .end ArchSecInit

