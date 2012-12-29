/*++

Copyright (c) 2011, kontais                                                                   
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  LoongsonSecTlbInit.s

Abstract:

  Loongson2f tlb init.

--*/



#include "Mips.h"
#include "LoongsonSecTlbInit.h"
#include "Pte.h"


  .text
  .align 3
  .set noreorder


  .globl LoongsonSecTlbInit
  .ent   LoongsonSecTlbInit
  .type  LoongsonSecTlbInit, @function

LoongsonSecTlbInit:
/*---------------------------------------------------------------

Invoke: 
  
  BRANCH_CALL(LoongsonSecTlbInit)

Routine Description:

  None

Arguments:

  None

Returns:

  None

----------------------------------------------------------------*/

  BRANCH_CALL(LoongsonSecTlbClear)


  BRANCH_RETURN(LoongsonSecTlbInit)

  .end LoongsonSecTlbInit

  .globl LoongsonSecTlbClear
  .ent   LoongsonSecTlbClear
  .type  LoongsonSecTlbClear, @function

LoongsonSecTlbClear:
/*---------------------------------------------------------------

Invoke: 
  
  BRANCH_CALL(LoongsonSecTlbClear)

Routine Description:

  None

Arguments:

  None

Returns:

  None

----------------------------------------------------------------*/
  mtc0  zero, CP0_Wired    // Clear Wired Register
  li    v0, TLB_PAGE_MASK
  mtc0  v0, CP0_PageMask   // Whatever
  mtc0  zero, CP0_EntryLo0 // Clear entry low0.
  mtc0  zero, CP0_EntryLo1 // Clear entry low1.

  move  v0, zero           // First TLB index
  li    v1, KSEG0_BASE
  li    t0, TLB_ENTRY_SIZE
1:
  mtc0  v1, CP0_EntryHi    // Clear entry high
  addiu v1, (1 << PG_HVPN_SHIFT)

  mtc0  v0, CP0_Index      // Set the index.
  addiu v0, 1
  nop
  nop
  tlbwi                    // Write the TLB

  bne   v0, t0, 1b
  nop

  mtc0  zero, CP0_Index
  mtc0  zero, CP0_EntryHi

  BRANCH_RETURN(LoongsonSecTlbClear)

  .end LoongsonSecTlbClear

  .set reorder



