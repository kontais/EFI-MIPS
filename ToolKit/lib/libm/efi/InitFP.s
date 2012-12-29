/*++

Copyright (c) 2012, kontais                                                                   
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  Math.s

Abstract:


--*/

#include "RegDef.h"

  .text
  .align 2
  .globl InitFP
  .ent   InitFP
  .type  InitFP, @function

InitFP:

  .set noreorder

  mfc0 v0, CP0_Status
  lui  v1, 0x3400
  or   v0, v1
  mtc0 v0, CP0_Status
  move v0, zero
  ctc1 v0, CP1_FCSR

  jr  ra
  nop

  .set reorder
  .end InitFP