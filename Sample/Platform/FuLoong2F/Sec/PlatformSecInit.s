/*++

Copyright (c) 2011, kontais                                                                   
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  PlatformSecInit.s

Abstract:

  platform init.

--*/



#include "Mips.h"
#include "PlatformSecInit.h"

  .text
  .align 3
  .globl PlatformSecInit
  .ent   PlatformSecInit
  .type  PlatformSecInit, @function

PlatformSecInit:
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Invoke: 
  
  BRANCH_CALL(PlatformSecInit)

Routine Description:

  None

Arguments:

  None

Returns:

  None

----------------------------------------------------------------*/

  .set noreorder
  
  BRANCH_CALL(Ns16550SecInit)   // Bonito uart init
  BRANCH_CALL(BonitoSecInit)    // Bonito power on init
  BRANCH_CALL(Cs5536SecInit)    // Cs5536 power on init
  BRANCH_CALL(BonitoSecMemInstall)
  BRANCH_CALL(LoongsonSecTlbInit)
  BRANCH_CALL(LoongsonSecCacheInit)

  BRANCH_RETURN(PlatformSecInit)

  .set reorder
  .end PlatformSecInit

