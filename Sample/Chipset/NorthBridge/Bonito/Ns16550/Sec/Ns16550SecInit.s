/*++

Copyright (c) 2009, kontais                                                                   
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  Ns16550SecInit.s

Abstract:

  Ns16550 PowerOn init.

--*/



#include "Mips.h"
#include "BonitoReg.h"
#include "Ns16550.h"
#include "Ns16550SecInit.h"



  .text
  .align 3
  .globl Ns16550SecInit
  .ent   Ns16550SecInit
  .type  Ns16550SecInit, @function

  .set noreorder

Ns16550SecInit:
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Invoke: 
  
  BRANCH_CALL(Ns16550SecInit)

Routine Description:

  None

Arguments:

  None

Returns:

  None

----------------------------------------------------------------*/
  
  la    v0, SEC_DEBUG_SERIAL_BASE_ADDRESS
  li    v1, FIFO_ENABLE|FIFO_RCV_RST|FIFO_XMT_RST|FIFO_TRIGGER_1
  sb    v1, NSREG(NS16550_FIFO)(v0)
  li    v1, CFCR_DLAB                // DLAB
  sb    v1, NSREG(NS16550_CFCR)(v0)
  la    v1, NS16550HZ/(16 * SEC_DEBUG_SERIAL_BAUD_RATE)  // set BRDL
  sb    v1, NSREG(NS16550_DATA)(v0)
  srl   v1, 8
  sb    v1, NSREG(NS16550_IER)(v0)  // set BRDH
  li    v1, CFCR_8BITS              // 8bit
  sb    v1, NSREG(NS16550_CFCR)(v0)
  li    v1, MCR_DTR|MCR_RTS
  sb    v1, NSREG(NS16550_MCR)(v0)
  li    v1, 0x0
  sb    v1, NSREG(NS16550_IER)(v0)
  
  BRANCH_RETURN(Ns16550SecInit)

  .end Ns16550SecInit

/*-------------------------------------------------------
 *  函数名:  SecOutputByte
 *  调用  :  void SecOutputByte(char c);
 *  寄存使用:
 *        使用a0,v0,v1. 确保不使用 a1,a2,a3
 *------------------------------------------------------*/
LEAF(SecOutputByte)

  la    v0, SEC_DEBUG_SERIAL_BASE_ADDRESS
1:
  lbu   v1, NSREG(NS16550_LSR)(v0)
  and   v1, LSR_TXRDY
  beqz  v1, 1b
  nop

  sb    a0, NSREG(NS16550_DATA)(v0)
  nop

  jr     ra
  nop  
  
END(SecOutputByte)

  .set reorder


