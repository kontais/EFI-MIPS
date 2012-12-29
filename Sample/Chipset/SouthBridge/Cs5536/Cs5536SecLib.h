/*++

Copyright (c) 2011, kontais                                                                   
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  Cs5536SecLib.h

Abstract:


--*/
#ifndef _CS5536_SEC_LIB_H_
#define _CS5536_SEC_LIB_H_

#define  MSR_WRITE(Reg, Low, Hi) \
  PCI_WRITE32(CS5536_PCI_IDSEL, CS5536_PCI_FUNC_ISA, PCI_MSR_ADDR, Reg); \
  PCI_WRITE32(CS5536_PCI_IDSEL, CS5536_PCI_FUNC_ISA, PCI_MSR_LO32, Low); \
  PCI_WRITE32(CS5536_PCI_IDSEL, CS5536_PCI_FUNC_ISA, PCI_MSR_HI32, Hi);
  
#define  MSR_READ(Reg) \
  PCI_WRITE32(CS5536_PCI_IDSEL, CS5536_PCI_FUNC_ISA, PCI_MSR_ADDR, Reg); \
  PCI_READ32(CS5536_PCI_IDSEL, CS5536_PCI_FUNC_ISA, PCI_MSR_LO32);  \
  PCI_READ32(CS5536_PCI_IDSEL, CS5536_PCI_FUNC_ISA, PCI_MSR_HI32); 

#define GPIO_HI_BIT(Bit, Reg) \
  li    a0, Reg;    \
  jal   IoRead32;   \
  nop;              \
  li    t0, 0x0001; \
  sll   t0, Bit;    \
  or    v0, t0;     \
  sll   t0, 16;     \
  not   t0;         \
  and   v0, t0;     \
  li    a0, Reg;    \
  move  a1, v0;     \
  jal   IoWrite32;  \
  nop;
  
#define GPIO_LO_BIT(Bit, Reg) \
  li    a0, Reg;      \
  jal   IoRead32;     \
  nop;                \
  li  t0, 0x0001;     \
  sll t0, (Bit + 16); \
  or  v0, t0;         \
  srl t0, 16;         \
  not t0;             \
  and v0, t0;         \
  li    a0, Reg;      \
  move  a1, v0;       \
  jal   IoWrite32;    \
  nop;

#define SMBUS_WAIT \
  li    a2, 1000;  \
1: \
  li    a3, 0x100; \
2: \
  addiu a3, -1;    \
  bnez  a3, 2b;    \
  nop;             \
  IO_READ8(SMB_BASE_ADDR | SMB_STS); \
  andi  v0, SMB_STS_SDAST; \
  li    a3, 0x0;   \
  bnez  v0, 3f;    \
  nop; \
  IO_READ8(SMB_BASE_ADDR | SMB_STS); \
  andi  v0, (SMB_STS_BER | SMB_STS_NEGACK); \
  li    a3, 0x01;  \
  bnez  v0, 3f;    \
  nop; \
  addiu a2, -1;    \
  bnez  a2, 1b;    \
  nop; \
  li    a3, 0x01;  \
3: \
  nop;

#endif /* _CS5536_SEC_LIB_H_ */

