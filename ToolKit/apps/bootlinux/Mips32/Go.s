/*++

Copyright (c) 2009, kontais                                                                   
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

/* Loongson L1 cache operations */
#define IndexInvalidate_I 0x00
#define IndexWBInvalidate_D 0x01
#define IndexLoadTag_D    0x05
#define IndexStoreTag_D   0x09
#define HitInvalidate_D   0x11
#define HitWBInvalidate_D 0x15
#define IndexLoadData_D   0x19
#define IndexStoreData_D  0x1d

/* Loongson L2 cache operations */
#define IndexWBInvalidate_S 0x03
#define IndexLoadTag_S    0x07
#define IndexStoreTag_S   0x0b
#define HitInvalidate_S   0x13
#define HitWBInvalidate_S 0x17
#define IndexLoadData_S   0x1b
#define IndexStoreData_S  0x1f

/* Loongson L1/L2 Cache size */
#define L1_ICACHE_SIZE   (64*1024)
#define L1_DCACHE_SIZE   (64*1024)
#define L2_CACHE_SIZE    (512*1024)

/* Loongson L1/L2 Cache line size */
#define L1_ICACHE_LINE_SIZE   (0x20)
#define L1_DCACHE_LINE_SIZE   (0x20)
#define L2_CACHE_LINE_SIZE    (0x20)

#define  KSEG0_BASE           (0x80000000)  // Unmapped, Cached

#define SR_COP_1_BIT  0x20000000
#define SR_FR_32      0x04000000
#define SR_EXL        0x00000002



  .text
  .align 3
  .set   noreorder


  .globl FlushCache
  .ent   FlushCache
  .type  FlushCache, @function

FlushCache:
/*++
VOID
FlushCache (
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
  
  .end FlushCache

  .globl go
  .ent   go
  .type  go, @function

go:
/*++
VOID
go (
  int argc,
  char **argv,
  char **envp,
  int pc
  );
--*/
	li		k0, SR_COP_1_BIT|SR_FR_32|SR_EXL
	mtc0	k0, CP0_Status

	/* stack */
	li		sp, 0xa8000000
	
	j	a3
	nop
  
  .end go





