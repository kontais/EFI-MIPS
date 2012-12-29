/*++

Copyright (c) 2009, kontais                                                                   
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  Cp0.h 

Abstract:

  Loongson CP0 寄存器定义

--*/

#ifndef _MIPS_CP0_REGS_H_
#define _MIPS_CP0_REGS_H_

#if defined(__ASSEMBLER__)
#define  CP0_Index      $0
#define  CP0_Random     $1
#define  CP0_EntryLo0   $2
#define  CP0_EntryLo1   $3
#define  CP0_Context    $4
#define  CP0_PageMask   $5
#define  CP0_Wired      $6
#define  CP0_BadVAddr   $8
#define  CP0_Count      $9
#define  CP0_EntryHi    $10
#define  CP0_Compare    $11
#define  CP0_Status     $12
#define  CP0_Cause      $13
#define  CP0_EPC        $14
#define  CP0_PRId       $15
#define  CP0_Config     $16
#define  CP0_LLAddr     $17
#define  CP0_WatchLo    $18     
#define  CP0_WatchHi    $19
#define  CP0_XContext   $20
#define  CP0_Diagnostic $22  // 64Bit 
#define  CP0_PerfCntLo  $24
#define  CP0_PerfCntHi  $25
#define  CP0_ECC        $26
#define  CP0_CacheErr   $27
#define  CP0_TagLo      $28
#define  CP0_TagHi      $29
#define  CP0_ErrorEPC   $30

#endif

//
// CP0_PRId
//
#define  CP0_PRID_IMP_SHIFT   8
#define  CP0_PRID_IMP_MASK    0x000000ff
#define  CP0_PRID_REV_SHIFT   0
#define  CP0_PRID_REV_MASK    0x000000ff


//
// CP0_Status
//
#define  CP0_STATUS_CU        0xf0000000
#define  CP0_STATUS_CU_CP1    0x20000000
#define  CP0_STATUS_BEV       0x00400000
#define  CP0_STATUS_IM        0x0000ff00
#define  CP0_STATUS_IM_SHIFT  8
#define  CP0_STATUS_KSU       0x00000018
#define  CP0_STATUS_EXL       0x00000002
#define  CP0_STATUS_IE        0x00000001


//
// CP0_Cause
//
#define  CP0_CAUSE_IP0        0x00000100
#define  CP0_CAUSE_IP1        0x00000200
#define  CP0_CAUSE_IP2        0x00000400
#define  CP0_CAUSE_IP3        0x00000800
#define  CP0_CAUSE_IP4        0x00001000
#define  CP0_CAUSE_IP5        0x00002000
#define  CP0_CAUSE_IP6        0x00004000
#define  CP0_CAUSE_IP7        0x00008000
#define  CP0_CAUSE_IP         0x0000ff00
#define  CP0_CAUSE_IP_SHIFT   8
#define  CP0_CAUSE_EXC_CODE   0x0000007c
#define  CP0_CAUSE_EXC_SHIFT  2

//
// CP0_Diagnostic
//
#define	CP0_DIAG_ITLB_CLEAR   0x04
#define	CP0_DIAG_BTB_CLEAR    0x02
#define	CP0_DIAG_RAS_DISABLE  0x01


#endif // _MIPS_CP0_REGS_H_
