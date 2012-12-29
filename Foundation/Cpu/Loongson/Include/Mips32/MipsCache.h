/*++

Copyright (c) 2009, kontais                                                                   
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  MipsCache.h

Abstract:


--*/

#ifndef _MIPS_CACHE_H_
#define _MIPS_CACHE_H_

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

#endif /* _MIPS_CACHE_H_ */

