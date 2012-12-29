/*++

Copyright (c) 2009, kontais                                                                   
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  RegDef.h 

Abstract:

  MIPS32通用寄存器定义

--*/

#ifndef _REG_DEF_H_
#define _REG_DEF_H_

#if defined(__ASSEMBLER__)
#define zero $0
#define AT   $1  // 大写是为了不影响.set at的at
#define v0   $2
#define v1   $3
#define a0   $4
#define a1   $5
#define a2   $6
#define a3   $7
#define t0   $8
#define t1   $9
#define t2   $10
#define t3   $11
#define t4   $12
#define t5   $13
#define t6   $14
#define t7   $15
#define s0   $16
#define s1   $17
#define s2   $18
#define s3   $19
#define s4   $20
#define s5   $21
#define s6   $22
#define s7   $23
#define t8   $24
#define t9   $25
#define k0   $26
#define k1   $27
#define gp   $28
#define sp   $29
#define s8   $30  // 同fp
#define fp   $30
#define ra   $31


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

#define  CP1_FIR   $0
#define  CP1_FCSR  $31

#endif


#endif // _REG_DEF_H_

