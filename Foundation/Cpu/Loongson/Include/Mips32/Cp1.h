/*++

Copyright (c) 2009, kontais                                                                   
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  Cp1.h 

Abstract:

  Loongson CP1寄存器定义

--*/

#ifndef _MIPS_CP1_REGS_H_
#define _MIPS_CP1_REGS_H_

#if defined(__ASSEMBLER__)

//
// 浮点实现和修订寄存器
//
#define  CP1_FIR  $0

//
// 浮点控制/状态寄存器
//
#define  CP1_FCSR  $31

#endif

#endif // _MIPS_CP1_REGS_H_
