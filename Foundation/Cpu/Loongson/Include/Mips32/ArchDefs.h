/*++

Copyright (c) 2009, kontais                                                                   
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  ArchDefs.h

Abstract:


--*/

#ifndef _ARCH_DEFS_H_
#define _ARCH_DEFS_H_

#define  KUSEG_BASE             (0x00000000)  // MMU Mapped
#define  KUSEG_SIZE             (0x80000000)  // 2GB

#define  KSEG0_BASE             (0x80000000)  // Unmapped, Cached
#define  KSEG0_SIZE             (0x20000000)  //  512MB

#define  KSEG1_BASE             (0xa0000000)  // Unmapped, Uncached
#define  KSEG1_SIZE             (0x20000000)  // 512MB

#define  KSEG2_BASE             (0xc0000000)  // MMU Mapped
#define  KSEG2_SIZE             (0x40000000)  // 1GB

#define  PHYS_TO_UNCACHED(x)    ((x) | KSEG1_BASE)
#define  PHYS_TO_CACHED(x)      ((x) | KSEG0_BASE)
#define  CACHED_TO_PHYS(x)      ((x) & (KSEG0_SIZE - 1))
#define  UNCACHED_TO_PHYS(x)    ((x) & (KSEG1_SIZE - 1))
#define  CACHED_TO_UNCACHED(x)  ((x) | KSEG1_BASE)
#define  UNCACHED_TO_CACHED(x)  (((x) & ~KSEG1_BASE) | KSEG0_BASE)

/*
 * Mips32 Exception Vectors
 */
#define	RESET_EXC_VEC       (KSEG1_BASE + 0x1fc00000)
#define	TLB_MISS_EXC_VEC    (KSEG0_BASE + 0x00000000)
#define	XTLB_MISS_EXC_VEC   (KSEG0_BASE + 0x00000000)
#define	CACHE_ERR_EXC_VEC   (KSEG0_BASE + 0x00000100)
#define	GEN_EXC_VEC         (KSEG0_BASE + 0x00000180)

#endif /* _ARCH_DEFS_H_ */
