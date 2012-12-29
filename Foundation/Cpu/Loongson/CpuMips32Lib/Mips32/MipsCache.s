/*++

Copyright (c) 1998-2004 Opsycon AB (www.opsycon.se)
Copyright (c) 2009 Miodrag Vallat.
Copyright (c) 2010, kontais               
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  MipsCache.s

Abstract:

  MIPS Cache routines

--*/

#include "Mips.h"
#include "MipsCache.h"


  .text
  .align 3
  .set   noreorder


  .globl MipsCacheSync
  .ent   MipsCacheSync
  .type  MipsCacheSync, @function

MipsCacheSync:
/*++
VOID
MipsCacheSync (
  VOID
  )

Routine Description:

  Sync ALL caches.

Arguments:

    None
  
Returns:

  None
--*/

	sync

	li	t1, L1_ICACHE_SIZE
	srl	t1, t1, 2			# / 4ways
	li	t2, L1_DCACHE_SIZE
	srl	t2, t2, 2			# / 4ways

	/* L1 I$ */

	li   t0, KSEG0_BASE
	addu t1, t0, t1			# Compute end address
	subu t1, 32
1:
	cache	IndexInvalidate_I, 0(t0)
	bne	t0, t1, 1b
	addu t0, 32

	/* L1 D$ */

	li   t0, KSEG0_BASE
	addu t1, t0, t2			# End address
	subu t1, 32
1:
	cache	IndexWBInvalidate_D, 0(t0)
	cache	IndexWBInvalidate_D, 1(t0)
	cache	IndexWBInvalidate_D, 2(t0)
	cache	IndexWBInvalidate_D, 3(t0)
	bne	t0, t1, 1b
	addu t0, 32

	/* L2 */

	li  t0, KSEG0_BASE
	li	t2, L2_CACHE_SIZE
	srl	t2, 2				# because cache is 4 way
	addu t1, t0, t2
	subu t1, 32
1:
	cache	IndexWBInvalidate_S, 0(t0)
	cache	IndexWBInvalidate_S, 1(t0)
	cache	IndexWBInvalidate_S, 2(t0)
	cache	IndexWBInvalidate_S, 3(t0)
	bne	t0, t1, 1b
	addu t0, 32

	j	ra
	nop
  
  .end MipsCacheSync

LEAF(GetL1ICacheSize)
  mfc0  v1, CP0_Config
  srl   v1, 9
  and   v1, 7
  li    v0, 4096  // 2^12
  sllv  v0, v0, v1
  jr    ra
  nop
END(GetL1ICacheSize)

LEAF(GetL1ICacheLineSize)
  mfc0  v1, CP0_Config
  and   v1, 0x20
  srl   v1, 1
  addiu v0, v1, 16
  jr    ra
  nop
END(GetL1ICacheLineSize)

LEAF(GetL1DCacheSize)
  mfc0  v1, CP0_Config
  srl   v1, 6
  and   v1, 7
  li    v0, 4096  // 2^12
  sllv  v0, v0, v1
  jr    ra
  nop
END(GetL1DCacheSize)

LEAF(GetL1DCacheLineSize)
  mfc0  v1, CP0_Config
  and   v1, 0x10
  addiu v0, v1, 16
  jr    ra
  nop
END(GetL1DCacheLineSize)

