/*++

Copyright (c) 2009, kontais                                            
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  ElfArch.h

Abstract:

  MIPS ELF Relocation Type

Revision History

--*/

#ifndef _ELF_ARCH_H_
#define _ELF_ARCH_H_

/* MIPSABI relocation (rel type) */

#define R_MIPS_NONE          0
#define R_MIPS_16            1
#define R_MIPS_32            2
#define R_MIPS_REL32         3
#define R_MIPS_26            4
#define R_MIPS_HI16          5
#define R_MIPS_LO16          6
#define R_MIPS_GPREL16       7
#define R_MIPS_LITERAL       8
#define R_MIPS_GOT16         9
#define R_MIPS_PC16          10
#define R_MIPS_CALL16        11
#define R_MIPS_GPREL32       12

#define R_MIPS_UNUSED1       13
#define R_MIPS_UNUSED2       14
#define R_MIPS_UNUSED3       15
#define R_MIPS_SHIFT5        16
#define R_MIPS_SHIFT6        17
#define R_MIPS_64            18
#define R_MIPS_GOT_DISP      19
#define R_MIPS_GOT_PAGE      20
#define R_MIPS_GOT_OFST      21

#define R_MIPS_GOTHI16       22
#define R_MIPS_GOTLO16       23
#define R_MIPS_SUB           24
#define R_MIPS_INSERT_A      25
#define R_MIPS_INSERT_B      26
#define R_MIPS_DELETE        27
#define R_MIPS_HIGHER        28
#define R_MIPS_HIGHEST       29

#endif // _ELF_ARCH_H_
