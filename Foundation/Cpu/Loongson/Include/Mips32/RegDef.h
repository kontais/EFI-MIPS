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

#include "Cp0.h"
#include "Cp1.h"
#include "RegNum.h"

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

#endif

#endif // _REG_DEF_H_
