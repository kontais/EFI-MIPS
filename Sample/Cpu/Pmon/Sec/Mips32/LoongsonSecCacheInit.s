/*++

Copyright (c) 2009, kontais                                                                   
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  LoongsonSecCacheInit.s

Abstract:

  Via686b PowerOn init.

--*/



#include "Mips.h"
#include "LoongsonSecCacheInit.h"
#include "MipsCache.h"


  .text
  .align 3
  .set noreorder
  
  .globl LoongsonSecCacheInit
  .ent   LoongsonSecCacheInit
  .type  LoongsonSecCacheInit, @function

LoongsonSecCacheInit:
/*---------------------------------------------------------------

Invoke: 
  
  BRANCH_CALL(LoongsonSecCacheInit)

Routine Description:

  None

Arguments:

  None

Returns:

  None

----------------------------------------------------------------*/
  li    a0, KSEG0_BASE
  li    a1, L1_ICACHE_SIZE
  li    a2, L1_DCACHE_SIZE
  
  //
  // Cache Init D 2way
  //
  // a3,  v0 and v1 used as local registers
  mtc0  zero, CP0_TagHi
  addu  v0, zero, a0
  addu  v1, a0, a2
1:  
  slt   a3, v0, v1
  beq   a3, zero, 1f
  nop
  mtc0  zero, CP0_TagLo
  cache IndexStoreTag_D, 0x0(v0)
  mtc0  zero, CP0_TagLo
  cache IndexStoreTag_D, 0x1(v0)
  mtc0  zero, CP0_TagLo
  cache IndexStoreTag_D, 0x2(v0)
  mtc0  zero, CP0_TagLo
  cache IndexStoreTag_D, 0x3(v0)
  beq   zero, zero, 1b
  addiu v0, v0, L1_DCACHE_LINE_SIZE

1:

  //
  // Cache Init L2 4way
  //
  mtc0  zero, CP0_TagHi
  addu  v0, zero, a0
  addu  v1, a0, L2_CACHE_SIZE
1:      
  slt   a3, v0, v1
  beq   a3, zero, 1f
  nop
  mtc0  zero, CP0_TagLo
  cache IndexStoreTag_S, 0x0(v0)
  mtc0  zero, CP0_TagLo
  cache IndexStoreTag_S, 0x1(v0)
  mtc0  zero, CP0_TagLo
  cache IndexStoreTag_S, 0x2(v0)
  mtc0  zero, CP0_TagLo
  cache IndexStoreTag_S, 0x3(v0)
  beq   zero, zero, 1b
  addiu v0, v0, L2_CACHE_LINE_SIZE

1:
  //
  // Cache Flush L2 4way
  //
  addu  v0, zero, a0
  addu  v1, a0, L2_CACHE_SIZE
1:  
  slt   a3, v0, v1
  beq   a3, zero, 1f
  nop
  cache IndexWBInvalidate_S, 0x0(v0)
  cache IndexWBInvalidate_S, 0x1(v0)
  cache IndexWBInvalidate_S, 0x2(v0)
  cache IndexWBInvalidate_S, 0x3(v0)
  beq   zero, zero, 1b
  addiu v0, v0, L2_CACHE_LINE_SIZE

1:

  //
  // Cache Flush I 2way
  //
  addu  v0, zero, a0
  addu  v1, a0, a1
1:  
  slt   a3, v0, v1
  beq   a3, zero, 1f
  nop
  cache IndexInvalidate_I, 0x0(v0)
  beq   zero, zero, 1b
  addiu v0, v0, L1_ICACHE_LINE_SIZE
1:
  //
  // Cache Flush D 2way
  //
  addu  v0, zero, a0
  addu  v1, a0, a2
1:  
  slt   a3, v0, v1
  beq   a3, zero, 1f
  nop
  cache IndexWBInvalidate_D, 0x0(v0)
  cache IndexWBInvalidate_D, 0x1(v0)
  cache IndexWBInvalidate_D, 0x2(v0)
  cache IndexWBInvalidate_D, 0x3(v0)
  beq   zero, zero, 1b
  addiu v0, v0, L1_DCACHE_LINE_SIZE
1:



  BRANCH_RETURN(LoongsonSecCacheInit)

  .end LoongsonSecCacheInit

  .set reorder



