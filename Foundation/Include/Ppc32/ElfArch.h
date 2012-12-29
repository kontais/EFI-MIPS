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

  PowerPC ELF Relocation Type

Revision History

--*/

#ifndef _ELF_ARCH_H_
#define _ELF_ARCH_H_

//
// Relocation type enum
//

#define R_PPC_NONE  0
#define R_PPC_ADDR32  1
#define R_PPC_ADDR24  2
#define R_PPC_ADDR16  3
#define R_PPC_ADDR16_LO  4
#define R_PPC_ADDR16_HI  5
#define R_PPC_ADDR16_HA  6
#define R_PPC_ADDR14  7
#define R_PPC_ADDR14_BRTAKEN 8
#define R_PPC_ADDR14_BRNTAKEN 9
#define R_PPC_REL24  10
#define R_PPC_REL14  11
#define R_PPC_REL14_BRTAKEN 12
#define R_PPC_REL14_BRNTAKEN 13
#define R_PPC_GOT16  14
#define R_PPC_GOT16_LO  15
#define R_PPC_GOT16_HI  16
#define R_PPC_GOT16_HA  17
#define R_PPC_PLT24  18
#define R_PPC_COPY  19
#define R_PPC_JMP_SLOT  21
#define R_PPC_RELATIVE  22
#define R_PPC_LOCAL24PC  23
#define R_PPC_UADDR32  24
#define R_PPC_UADDR16  25
#define R_PPC_REL32  26
#define R_PPC_PLT32  27
#define R_PPC_PLTREL32  28
#define R_PPC_PLT16_LO  29
#define R_PPC_PLT16_HI  30
#define R_PPC_PLT16_HA  31
#define R_PPC_SDAREL  32

/* EABI relocation types */

#define R_PPC_EMB_NADDR32 101
#define R_PPC_EMB_NADDR16 102
#define R_PPC_EMB_NADDR16_LO 103
#define R_PPC_EMB_NADDR16_HI 104
#define R_PPC_EMB_NADDR16_HA 105
#define R_PPC_EMB_SDAI16 106
#define R_PPC_EMB_SDA2I16 107
#define R_PPC_EMB_SDA2REL 108
#define R_PPC_EMB_SDA21  109
#define R_PPC_EMB_MRKREF 110
#define R_PPC_EMB_RELSEC16 111
#define R_PPC_EMB_RELST_LO 112
#define R_PPC_EMB_RELST_HI 113
#define R_PPC_EMB_RELST_HA 114
#define R_PPC_EMB_BIT_FLD 115
#define R_PPC_EMB_RELSDA 116

#define R_PPC_EMB_ROSDAI16_OLD 107
#define R_PPC_EMB_SDA16_OLD 108
#define R_PPC_EMB_ROSDA16_OLD 109
#define R_PPC_EMB_SDA21_OLD 110
#define R_PPC_EMB_MRKREF_OLD 111
#define R_PPC_EMB_RELSEC16_OLD 114
#define R_PPC_EMB_RELST_LO_OLD 115
#define R_PPC_EMB_RELST_HA_OLD 116

#endif /* _ELF_ARCH_H_ */
