/*++

Copyright (c) 2011, kontais                                                                   
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  Cs5536SecSMBusInit.s

Abstract:

  Cs5536 IDE IRQ, CS5536 INTR, PWN_BUT GPIO.

--*/



#include "Mips.h"
#include "BonitoReg.h"
#include "BonitoPci.h"
#include "BonitoIo.h"
#include "GlxReg.h"
#include "GlxPci.h"
#include "Cs5536SecLib.h"
#include "Cs5536SecInit.h"

#define  SMB_CLK   14
#define  SMB_DATA  15


  .text
  .align 3
  .set noreorder

  .globl Cs5536SecSMBusInit
  .ent   Cs5536SecSMBusInit
  .type  Cs5536SecSMBusInit, @function

Cs5536SecSMBusInit:
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Invoke: 
  
  BRANCH_CALL(Cs5536SecSMBusInit)

Routine Description:

  None

Arguments:

  None

Returns:

  None

----------------------------------------------------------------*/

  // SMBus Clock Pin
  GPIO_HI_BIT(SMB_CLK, GPIO_BASE_ADDR | GPIOL_IN_EN);
  GPIO_HI_BIT(SMB_CLK, GPIO_BASE_ADDR | GPIOL_OUT_EN);
  GPIO_HI_BIT(SMB_CLK, GPIO_BASE_ADDR | GPIOL_IN_AUX1_SEL);
  GPIO_HI_BIT(SMB_CLK, GPIO_BASE_ADDR | GPIOL_OUT_AUX1_SEL);
  
  // SMBus Data Pin
  GPIO_HI_BIT(SMB_DATA, GPIO_BASE_ADDR | GPIOL_IN_EN);
  GPIO_HI_BIT(SMB_DATA, GPIO_BASE_ADDR | GPIOL_OUT_EN);
  GPIO_HI_BIT(SMB_DATA, GPIO_BASE_ADDR | GPIOL_IN_AUX1_SEL);
  GPIO_HI_BIT(SMB_DATA, GPIO_BASE_ADDR | GPIOL_OUT_AUX1_SEL);

  // Disable SMB, Config SMB_CLK Frequency
  IO_WRITE8(SMB_BASE_ADDR | SMB_CTRL2, ((0x3c << 1) & 0xfe));

  // Disable functional block
  IO_WRITE8(SMB_BASE_ADDR | SMB_CTRL1, 0x00);

  // Disable Device address & Slave mode
  IO_WRITE8(SMB_BASE_ADDR | SMB_ADDR, 0x00);

  // SMB Enable
  IO_MASK_SET8(SMB_BASE_ADDR | SMB_CTRL2, (1 << 0));

  // Stall After Start Enable, New Match Interrupt Enable
  IO_MASK_CLR8(SMB_BASE_ADDR | SMB_CTRL1, ((1 << 6)|(1 << 7)));

  // Stop
  IO_MASK_SET8(SMB_BASE_ADDR | SMB_CTRL1, (1 << 1));

  // Clear Bus Error, Neg Acknowledge, Stall After Start
  IO_WRITE8(SMB_BASE_ADDR | SMB_STS, (0x07 << 3));

  // Clear Bus Busy
  IO_MASK_SET8(SMB_BASE_ADDR | SMB_CTRL_STS, (1 << 1));


  BRANCH_RETURN(Cs5536SecSMBusInit)

  .end Cs5536SecSMBusInit

  .globl SMBusRead8
  .ent   SMBusRead8
  .type  SMBusRead8, @function

SMBusRead8:
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Invoke: 
  
  UINT8 SMBusRead8(UINT8 SlaveAddr, UINT8 Reg)

Routine Description:

  None

Arguments:

  None

Returns:

  None

----------------------------------------------------------------*/
  move  s2, ra
  andi  a0, 0xfe;
  move  t2, a0;
  move  t3, a1;
  
  // Start Condition
  IO_MASK_SET8(SMB_BASE_ADDR | SMB_CTRL1, SMB_CTRL1_START)
  
  IO_READ8(SMB_BASE_ADDR | SMB_STS)
  andi  v0, SMB_STS_BER
  bnez  v0, SmbusError
  nop
  SMBUS_WAIT
  bnez  a3, SmbusError
  nop
  
  // Send Slave Address
  move  a1, t2
  IO_WRITE82(SMB_BASE_ADDR | SMB_SDA)
  IO_READ8(SMB_BASE_ADDR | SMB_STS)
  andi  v0, (SMB_STS_BER | SMB_STS_NEGACK)
  bnez  v0, SmbusError
  nop
  SMBUS_WAIT
  bnez  a3, SmbusError
  nop
  
  // Acknowledge Smbus
  IO_READ8(SMB_BASE_ADDR | SMB_CTRL1)
  ori   a1, v0, (SMB_CTRL1_ACK)
  IO_WRITE82(SMB_BASE_ADDR | SMB_CTRL1)
  
  // Send command
  move  a1, t3
  IO_WRITE82(SMB_BASE_ADDR | SMB_SDA)
  IO_READ8(SMB_BASE_ADDR | SMB_STS)
  andi  v0, (SMB_STS_BER | SMB_STS_NEGACK)
  bnez  v0, SmbusError
  nop
  SMBUS_WAIT
  bnez  a3, SmbusError
  nop
  
  // Start Condition Again
  IO_READ8(SMB_BASE_ADDR | SMB_CTRL1)
  ori   a1, v0, SMB_CTRL1_START
  IO_WRITE82(SMB_BASE_ADDR | SMB_CTRL1)
  IO_READ8(SMB_BASE_ADDR | SMB_STS)
  andi  v0, SMB_STS_BER
  bnez  v0, SmbusError
  nop
  SMBUS_WAIT
  bnez  a3, SmbusError
  nop
  
  // Send Slave Address Agaisn
  ori   a1, t2, 0x01
  IO_WRITE82(SMB_BASE_ADDR | SMB_SDA)
  IO_READ8(SMB_BASE_ADDR | SMB_STS)
  andi  v0, (SMB_STS_BER | SMB_STS_NEGACK)
  bnez  v0, SmbusError
  nop
  SMBUS_WAIT
  bnez  a3, SmbusError
  nop
  
  // Stop Condition
  IO_WRITE8(SMB_BASE_ADDR | SMB_CTRL1, SMB_CTRL1_STOP)
  SMBUS_WAIT
  bnez  a3, SmbusError
  nop
  
  // Read Data
  IO_READ8(SMB_BASE_ADDR | SMB_SDA)

SmbusError:
  nop
  move  ra, s2
  jr    ra
  nop

  .end SMBusRead8

  .set reorder

