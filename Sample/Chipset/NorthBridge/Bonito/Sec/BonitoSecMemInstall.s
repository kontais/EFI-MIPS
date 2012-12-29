/*++

Copyright (c) 2009, kontais                                                                   
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  BonitoSecInit.s

Abstract:

  Bonito PowerOn init.

--*/



#include "Mips.h"
#include "BonitoReg.h"
#include "BonitoSecInit.h"


#define  REG_ADDRESS   0x0
#define  CONFIG_BASE   0xaffffe00

  .data
  .align 5
  
ddr2_reg_data:
DDR2_CTL_00_DATA_LO: .word 0x00000101
DDR2_CTL_00_DATA_HI: .word 0x01000100 #no_concurrentap
DDR2_CTL_01_DATA_LO: .word 0x00010000
DDR2_CTL_01_DATA_HI: .word 0x00000000
DDR2_CTL_02_DATA_LO: .word 0x00000000
DDR2_CTL_02_DATA_HI: .word 0x01000101
DDR2_CTL_03_DATA_LO: .word 0x01000000
DDR2_CTL_03_DATA_HI: .word 0x01010000
DDR2_CTL_04_DATA_LO: .word 0x01010101
DDR2_CTL_04_DATA_HI: .word 0x01000202
DDR2_CTL_05_DATA_LO: .word 0x04050202 #CL =4
DDR2_CTL_05_DATA_HI: .word 0x00000000
DDR2_CTL_06_DATA_LO: .word 0x03050203 #800
DDR2_CTL_06_DATA_HI: .word 0x0a040306 #800
DDR2_CTL_07_DATA_LO: .word 0x00030a0b #CL=5//cs_map to cs0-cs3 
DDR2_CTL_07_DATA_HI: .word 0x00000400 #800
DDR2_CTL_08_DATA_LO: .word 0x00000102
DDR2_CTL_08_DATA_HI: .word 0x00000102
DDR2_CTL_09_DATA_LO: .word 0x00000000
DDR2_CTL_09_DATA_HI: .word 0x0000060c #800
DDR2_CTL_10_DATA_LO: .word 0x3f1f0200 #800
DDR2_CTL_10_DATA_HI: .word 0x2323233f //yanhua
DDR2_CTL_11_DATA_LO: .word 0x23232323 //yanhua
DDR2_CTL_11_DATA_HI: .word 0x5f7f2323 //yanhua
DDR2_CTL_12_DATA_LO: .word 0x15000000 #800
DDR2_CTL_12_DATA_HI: .word 0x002a3c06 #800
DDR2_CTL_13_DATA_LO: .word 0x002a002a
DDR2_CTL_13_DATA_HI: .word 0x002a002a
DDR2_CTL_14_DATA_LO: .word 0x002a002a
DDR2_CTL_14_DATA_HI: .word 0x002a002a
DDR2_CTL_15_DATA_LO: .word 0x00000004
DDR2_CTL_15_DATA_HI: .word 0x00b40020
DDR2_CTL_16_DATA_LO: .word 0x00000087
DDR2_CTL_16_DATA_HI: .word 0x000007ff #no_interrupt
DDR2_CTL_17_DATA_LO: .word 0x0016101f
DDR2_CTL_17_DATA_HI: .word 0x00000000
DDR2_CTL_18_DATA_LO: .word 0x00000000
DDR2_CTL_18_DATA_HI: .word 0x001c0000
DDR2_CTL_19_DATA_LO: .word 0x00c8006b
DDR2_CTL_19_DATA_HI: .word 0x28e10002 #800
DDR2_CTL_20_DATA_LO: .word 0x00c8002f #800
DDR2_CTL_20_DATA_HI: .word 0x00000000
DDR2_CTL_21_DATA_LO: .word 0x00030d40 #real
DDR2_CTL_21_DATA_HI: .word 0x00000000
DDR2_CTL_22_DATA_LO: .word 0x00000000
DDR2_CTL_22_DATA_HI: .word 0x00000000
DDR2_CTL_23_DATA_LO: .word 0x00000000
DDR2_CTL_23_DATA_HI: .word 0x00000000
DDR2_CTL_24_DATA_LO: .word 0x00000000
DDR2_CTL_24_DATA_HI: .word 0x00000000
DDR2_CTL_25_DATA_LO: .word 0x00000000
DDR2_CTL_25_DATA_HI: .word 0x00000000
DDR2_CTL_26_DATA_LO: .word 0x00000000
DDR2_CTL_26_DATA_HI: .word 0x00000000
DDR2_CTL_27_DATA_LO: .word 0x00000000
DDR2_CTL_27_DATA_HI: .word 0x00000000
DDR2_CTL_28_DATA_LO: .word 0x00000000
DDR2_CTL_28_DATA_HI: .word 0x00000000
DDR2_CTL_start_DATA_LO: .word 0x01000000
DDR2_CTL_start_DATA_HI: .word 0x01010100



MemorySizeTable:
  .word 0
  .word 0x40000000
  .word 0x80000000
  .word 0x0
  .word 0x0 // 4G fixme


  .text
  .align 3
  .set noreorder


  .globl ddr2_config
  .ent   ddr2_config
  .type  ddr2_config, @function
ddr2_config:
  // Save return address to s1
  move  s1, ra
  
  la    t0, ddr2_reg_data

1: 
  li    t1, 0x1d
  li    t2, CONFIG_BASE

ddr2_reg_write:
  ld    a1, 0x0(t0)
  sd    a1, REG_ADDRESS(t2)
  subu  t1, t1, 0x1
  addiu t0, t0, 0x8
  addiu t2, t2, 0x10
  bne   t1, $0, ddr2_reg_write
  nop

  //
  // Read SPD to initialize memory controller Start ===>
  //
  li    a0, 0xa0
  li    a1, 3 //Row Address
  bal   SMBusRead8
  nop
  li    v1, 15
  sub   v1, v0
  and   v1, 0x7
  sll   v1, 0x8
  li    v0, (CONFIG_BASE + 0x50)
  lw    a1, (v0)
  and   a1, 0xfffff8ff
  or    a1, v1
  sw    a1, (v0)

  li    a0, 0xa0
  li    a1, 0x04 // Column Address
  bal   SMBusRead8
  nop
  li    v1, 14
  sub   v1, v0
  sll   v1, 24
  li    v0, (CONFIG_BASE + 0x50)
  lw    a1, (v0)
  and   a1, 0xffffff
  or    a1, v1
  sw    a1, (v0)

  /* number of CS */
  li    a0, 0xa0
  li    a1, 5
  bal   SMBusRead8
  nop
  li    v1, 0x7
  and   v1, v0
  add   v1, 1
  li    a0, 1
  sll   a0, v1
  sub   a0, 1
  li    v0, (CONFIG_BASE + 0x70)
  lw    v1, 0(v0)
  nop 
  sll   a0, a0, 16
  li    t0, 0xfff0ffff
  and   v1, v1, t0
  or    v1, v1, a0
  sw    v1, 0(v0)
  nop

  //  Number of banks
  li    a0, 0xa0 
  li    a1, 0x11 //Bank 
  bal   SMBusRead8
  nop 
  li    v1, 0x4
  beq   v1, v0, 1f
  nop
  li    v1, 0x1
  li    v0, (CONFIG_BASE + 0x10)
  sw    v1, 4(v0) 
1:
  //
  // Read SPD to initialize memory controller End <===
  //

  
  li    t2, CONFIG_BASE
  la    t0, DDR2_CTL_start_DATA_LO
  ld    a1, 0x0(t0)
  sd    a1, 0x30(t2)

  // Restore return address for s1
  move  ra, s1
  jr    ra
  nop
  
  .end ddr2_config


  .globl BonitoSecMemInstall
  .ent   BonitoSecMemInstall
  .type  BonitoSecMemInstall, @function

BonitoSecMemInstall:
/*++

Invoke: 
  
  BRANCH_CALL(BonitoSecMemInstall)

Routine Description:

  None

Arguments:

  None

Returns:

  None

--*/

//
// BUGBUG 暂时在内存中运行，不配置这些寄存器
//
#if 0

  //
  // Enable register space of MEMORY
  // Disable ddr_buffer_cpu
  //
  li    t2, LOONGSON_CHIP_CONFIG0
  ld    a1, 0x0(t2)
  and   a1, a1, 0x4ff
  sd    a1, 0x0(t2)
  
  bal   ddr2_config
  nop

  // Check DDR2 DLL lock status
  li    t0, CONFIG_BASE
not_locked:
  ld    t1, 0x10(t0)
  andi  t1, 0x01
  beqz  t1, not_locked
  nop


  //
  // Disable register space of MEMORY
  //
  li    t2, LOONGSON_CHIP_CONFIG0
  ld    a1, 0x0(t2)
  or    a1, a1,0x100
  sd    a1, 0x0(t2)
#endif


  BRANCH_CALL(BonitoRomSpeedUp)
  BRANCH_CALL(BonitoPCIXBridgeCfg)


  BRANCH_RETURN(BonitoSecMemInstall)

  .end BonitoSecMemInstall


#define  tmpsize  s3

  .globl GetMemorySize
  .ent   GetMemorySize
  .type  GetMemorySize, @function

GetMemorySize:
/*++

Invoke: 
  
  UINT32 GetMemorySize();

Routine Description:

  None

Arguments:

  None

Returns:

  None

--*/

#if  0

  li    v0, 256*1024*1024
  jr    ra
  nop

#else

  // Save return address to s1
  move  s1, ra

  // Read DIMM memory size per side
  li    a0, 0xa1
  li    a1, 31
  bal   SMBusRead8
  nop
  beqz  v0, MemorySizeExit
  nop;
  sll   a0, v0, 2
  la    a1, MemorySizeTable
  addu  a1, a1, a0
  lw    tmpsize, 0(a1)
  li    a0, 0x10
  blt   v0, a0, 2f
  nop
  // multiply by 4M
  sll   tmpsize, v0, 22

2:
  // Read DIMM number of sides (banks)
  li    a0, 0xa1
  li    a1, 5
  bal   SMBusRead8
  nop
  bltu  v0, 0x60, MemorySizeExit
  nop
  subu  v0, 0x60
  sll   tmpsize, v0
  nop

  // Memory Size OK
  move  v0, tmpsize
  b     1f
  nop

MemorySizeExit:
  // Set the default memory size
  li    v0, 256*1024*1024
1:
  // Restore return address for s1
  move  ra, s1
  jr    ra
  nop

#endif

  .end GetMemorySize

  .globl BonitoPCIXBridgeCfg
  .ent   BonitoPCIXBridgeCfg
  .type  BonitoPCIXBridgeCfg, @function

BonitoPCIXBridgeCfg:
/*++

Invoke: 
  
  BRANCH_CALL(BonitoPCIXBridgeCfg)

Routine Description:

  None

Arguments:

  None

Returns:

  None

--*/
  //
  // Set BAR0 mask and translation to point to SDRAM
  //
  bal   GetMemorySize
  nop
  li    t1, 0x0
  sub   t0, v0,1
  not   t0
  srl   t0, LOONGSON_PCIX_BRIDGE_CFG_ASHIFT-LOONGSON_PCIX_BRIDGE_BASE0_MASK_SHIFT
  and   t0, LOONGSON_PCIX_BRIDGE_BASE0_MASK
  or    t1, t0
  
  li    t0, 0x00000000
  srl   t0, LOONGSON_PCIX_BRIDGE_CFG_ASHIFT-LOONGSON_PCIX_BRIDGE_BASE0_TRANS_SHIFT
  and   t0, LOONGSON_PCIX_BRIDGE_BASE0_TRANS
  or    t1, t0
  or    t1, LOONGSON_PCIX_BRIDGE_BASE0_CACHED

  //
  // Set BAR1 to minimum size to conserve PCI space
  //
  li    t0, ~0x0
  srl   t0, LOONGSON_PCIX_BRIDGE_CFG_ASHIFT-LOONGSON_PCIX_BRIDGE_BASE1_MASK_SHIFT
  and   t0, LOONGSON_PCIX_BRIDGE_BASE1_MASK
  or    t1, t0
  
  li    t0, 0x0
  srl   t0, LOONGSON_PCIX_BRIDGE_CFG_ASHIFT-LOONGSON_PCIX_BRIDGE_BASE1_TRANS_SHIFT
  and   t0, LOONGSON_PCIX_BRIDGE_BASE1_TRANS
  or    t1, t0
  or    t1, LOONGSON_PCIX_BRIDGE_BASE1_CACHED

  li    t0, PHYS_TO_UNCACHED(LOONGSON_PCIX_BRIDGE_CFG)
  sw    t1, 0(t0)

  //
  // Enable configuration cycles
  //
  li    t1, PHYS_TO_UNCACHED(LOONGSON_PONCFG)
  lw    t0, 0(t1)
  and   t0, ~LOONGSON_PONCFG_CONFIG_DIS
  sw    t0, 0(t1)

  BRANCH_RETURN(BonitoPCIXBridgeCfg)
  
  .end BonitoPCIXBridgeCfg

  .set reorder

