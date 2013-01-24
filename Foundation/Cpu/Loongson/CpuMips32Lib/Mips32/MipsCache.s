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


LEAF(GetL1ICacheSize)
  mfc0  v1, CP0_Config
  srl   v1, 9
  and   v1, 7
  li    v0, 4096     # 2^12
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
  li    v0, 4096     # 2^12
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

  li    t1, L1_ICACHE_SIZE
  srl   t1, t1, 2          # / 4ways
  li    t2, L1_DCACHE_SIZE
  srl   t2, t2, 2          # / 4ways

  /* L1 I$ */

  li    t0, KSEG0_BASE
  addu  t1, t0, t1         # Compute end address
  subu  t1, 32
1:
  cache IndexInvalidate_I, 0(t0)
  bne   t0, t1, 1b
  addu  t0, 32

  /* L1 D$ */

  li    t0, KSEG0_BASE
  addu  t1, t0, t2         # End address
  subu  t1, 32
1:
  cache IndexWBInvalidate_D, 0(t0)
  cache IndexWBInvalidate_D, 1(t0)
  cache IndexWBInvalidate_D, 2(t0)
  cache IndexWBInvalidate_D, 3(t0)
  bne   t0, t1, 1b
  addu  t0, 32

  /* L2 */

  li    t0, KSEG0_BASE
  li    t2, L2_CACHE_SIZE
  srl   t2, 2        # because cache is 4 way
  addu  t1, t0, t2
  subu  t1, 32
1:
  cache IndexWBInvalidate_S, 0(t0)
  cache IndexWBInvalidate_S, 1(t0)
  cache IndexWBInvalidate_S, 2(t0)
  cache IndexWBInvalidate_S, 3(t0)
  bne   t0, t1, 1b
  addu  t0, 32

  j     ra
  nop

.ent   MipsCacheSync

/*----------------------------------------------------------------------------
 *
 * MipsFlushICache --
 *
 *  void MipsFlushICache(paddr_t addr, len)
 *
 *  Invalidate the L1 instruction cache for at least range
 *  of addr to addr + len - 1.
 *  The address is reduced to a KSEG0 index to avoid TLB faults.
 *
 * Results:
 *  None.
 *
 * Side effects:
 *  The contents of the L1 Instruction cache is flushed.
 *  Must not touch v0.
 *
 * Assumtions:
 *      If a cache is not direct mapped, line size is 32.
 *
 *----------------------------------------------------------------------------
 */
LEAF(MipsFlushICache)
  andi  a0, ((1 << 14) - 1)    # only keep index bits
  addu  a1, 31          # Round up size
  li    a2, KSEG0_BASE
  addu  a1, a0          # Add extra from address
  dsrl  a0, a0, 5
  dsll  a0, a0, 5       # align address
  subu  a1, a1, a0
  addu  a0, a2          # a0 now new XKPHYS address
  dsrl  a1, a1, 5       # Number of unrolled loops
1:
  addu  a1, -1
  cache IndexInvalidate_I, 0(a0)
  bne   a1, zero, 1b
  addu  a0, 32

  j     ra
  nop
END(MipsFlushICache)



/*----------------------------------------------------------------------------
 *
 * MipsHitSyncDCache --
 *
 *  void MipsHitSyncDCache(paddr_t pa, size_t len)
 *
 *  Sync L1 and L2 data caches for range of pa to pa + len - 1.
 *  Since L2 is writeback, we need to operate on L1 first, to make sure
 *  L1 is clean. The usual mips strategy of doing L2 first, and then
 *  the L1 orphans, will not work as the orphans would only be pushed
 *  to L2, and not to physical memory.
 *
 *----------------------------------------------------------------------------
 */
LEAF(MipsHitSyncDCache)
  sync

  beq   a1, zero, 3f    # size is zero!
  addu  a1, 31          # Round up
  addu  a1, a1, a0      # Add extra from address
  dsrl  a0, a0, 5
  dsll  a0, a0, 5       # align to cacheline boundary
  subu  a1, a1, a0
  dsrl  a1, a1, 5       # Compute number of cache lines
  li    a3, KSEG0_BASE
  or    a0, a3          # build suitable va

  move  t0, a1          # save for L2
  move  a3, a0

  /* L1 */
1:
  addu  a1, -1
  cache HitWBInvalidate_D, 0(a0)
  bne   a1, zero, 1b
  addu  a0, 32

  /* L2 */
2:
  addu  t0, -1
  cache HitWBInvalidate_S, 0(a3)
  bne   t0, zero, 2b
  addu  a3, 32

3:
  j     ra
  nop
END(MipsHitSyncDCache)

/*----------------------------------------------------------------------------
 *
 * MipsHitInvalidateDCache --
 *
 *  void MipsHitInvalidateDCache(paddr_t pa, size_t len)
 *
 *  Invalidate L1 and L2 data caches for range of pa to pa + len - 1.
 *
 *----------------------------------------------------------------------------
 */
LEAF(MipsHitInvalidateDCache)
  sync

  beq   a1, zero, 3f    # size is zero!
  addu  a1, 31          # Round up
  addu  a1, a1, a0      # Add extra from address
  dsrl  a0, a0, 5
  dsll  a0, a0, 5       # align to cacheline boundary
  subu  a1, a1, a0
  dsrl  a1, a1, 5       # Compute number of cache lines
  li    a3, KSEG0_BASE
  or    a0, a3          # build suitable va

  move  t0, a1          # save for L2
  move  a3, a0

  /* L1 */
1:
  addu  a1, -1
  cache HitInvalidate_D, 0(a0)
  bne   a1, zero, 1b
  addu  a0, 32

  /* L2 */
2:
  addu  t0, -1
  cache HitInvalidate_S, 0(a3)
  bne   t0, zero, 2b
  addu  a3, 32

3:
  j     ra
  nop
END(MipsHitInvalidateDCache)

/*----------------------------------------------------------------------------
 *
 * MipsIOSyncDCache --
 *
 *  void MipsIOSyncDCache(paddr_t pa, size_t len, int how)
 *
 *  Invalidate or flush L1 and L2 data caches for range of pa to
 *  pa + len - 1.
 *
 *  If how == 0 (invalidate):
 *    L1 and L2 caches are invalidated or flushed if the area
 *    does not match the alignment requirements.
 *  If how == 1 (writeback):
 *    L1 and L2 are written back.
 *  If how == 2 (writeback and invalidate):
 *    L1 and L2 are written back to memory and invalidated (flushed).
 *
 *----------------------------------------------------------------------------
 */
NON_LEAF(MipsIOSyncDCache, FRAMESZ(CF_SZ+REGSZ), ra)
  sync

  subu  sp, FRAMESZ(CF_SZ+REGSZ)
  sw    ra, CF_RA_OFFS+REGSZ(sp)
  beqz  a2, SyncInv      # Sync PREREAD
  nop

SyncWBInv:
  jal   MipsHitSyncDCache
  nop
  b     SyncDone
  lw    ra, CF_RA_OFFS+REGSZ(sp)

SyncInv:
  or    t0, a0, a1        # check if invalidate possible
  and   t0, t0, 31        # both address and size must
  bnez  t0, SyncWBInv     # be aligned to the cache size
  nop

  jal   MipsHitInvalidateDCache
  nop
  lw    ra, CF_RA_OFFS+REGSZ(sp)

SyncDone:
  j     ra
  addu  sp, FRAMESZ(CF_SZ+REGSZ)
END(MipsIOSyncDCache)


