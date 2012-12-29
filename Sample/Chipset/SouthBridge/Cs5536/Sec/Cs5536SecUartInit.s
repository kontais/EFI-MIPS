/*++

Copyright (c) 2011, kontais                                                                   
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  Cs5536SecUartInit.s

Abstract:

  Cs5536 UART init.

--*/



#include "Mips.h"
#include "BonitoReg.h"
#include "BonitoPci.h"
#include "GlxReg.h"
#include "GlxPci.h"
#include "Cs5536SecLib.h"
#include "Cs5536SecInit.h"

#define UART1_TX  8
#define UART1_RX  9
#define UART2_TX  4
#define UART2_RX  3


  .text
  .align 3
  .set noreorder

  .globl Cs5536SecUartInit
  .ent   Cs5536SecUartInit
  .type  Cs5536SecUartInit, @function

Cs5536SecUartInit:
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Invoke: 
  
  BRANCH_CALL(Cs5536SecUartInit)

Routine Description:

  None

Arguments:

  None

Returns:

  None

----------------------------------------------------------------*/

  //=================UART1================

  // UART1 io port 0x3f8
  // UART2 io port 0x2f8
  MSR_WRITE(DIVIL_LEG_IO, 0x04570003, 0x0);

  // Enable UART1
  MSR_WRITE(UART1_CONF, 0x2, 0x0);

  // Config GPIO to UART1
  GPIO_HI_BIT(UART1_TX, GPIO_BASE_ADDR | GPIOL_OUT_EN);
  GPIO_HI_BIT(UART1_TX, GPIO_BASE_ADDR | GPIOL_OUT_AUX1_SEL);
  GPIO_HI_BIT(UART1_TX, GPIO_BASE_ADDR | GPIOL_PU_EN);
  GPIO_LO_BIT(UART1_TX, GPIO_BASE_ADDR | GPIOL_IN_EN);
  GPIO_LO_BIT(UART1_TX, GPIO_BASE_ADDR | GPIOL_OUT_AUX2_SEL);
  GPIO_LO_BIT(UART1_TX, GPIO_BASE_ADDR | GPIOL_IN_AUX1_SEL);

  GPIO_HI_BIT(UART1_RX, GPIO_BASE_ADDR | GPIOL_PU_EN);
  GPIO_HI_BIT(UART1_RX, GPIO_BASE_ADDR | GPIOL_IN_EN);
  GPIO_HI_BIT(UART1_RX, GPIO_BASE_ADDR | GPIOL_IN_AUX1_SEL);
  GPIO_LO_BIT(UART1_RX, GPIO_BASE_ADDR | GPIOL_OUT_EN);
  GPIO_LO_BIT(UART1_RX, GPIO_BASE_ADDR | GPIOL_OUT_AUX2_SEL);
  GPIO_LO_BIT(UART1_RX, GPIO_BASE_ADDR | GPIOL_OUT_AUX1_SEL);

  //=================UART2=================

  // Enable UART2
  MSR_WRITE(UART2_CONF, 0x2, 0x0);

  // Config GPIO to UART2
  GPIO_HI_BIT(UART2_TX, GPIO_BASE_ADDR | GPIOL_OUT_EN);
  GPIO_LO_BIT(UART2_TX, GPIO_BASE_ADDR | GPIOL_OUT_OD_EN);
  GPIO_HI_BIT(UART2_TX, GPIO_BASE_ADDR | GPIOL_OUT_AUX1_SEL);
  GPIO_LO_BIT(UART2_TX, GPIO_BASE_ADDR | GPIOL_OUT_AUX2_SEL);
  // GPIO_LO_BIT(UART2_TX, GPIO_BASE_ADDR | GPIOL_PU_EN);
  // GPIO_LO_BIT(UART2_TX, GPIO_BASE_ADDR | GPIOL_PD_EN);
  GPIO_LO_BIT(UART2_TX, GPIO_BASE_ADDR | GPIOL_IN_EN);
  GPIO_LO_BIT(UART2_TX, GPIO_BASE_ADDR | GPIOL_IN_AUX1_SEL);

  // GPIO_HI_BIT(UART2_RX, GPIO_BASE_ADDR | GPIOL_PU_EN);
  GPIO_HI_BIT(UART2_RX, GPIO_BASE_ADDR | GPIOL_IN_EN);
  GPIO_HI_BIT(UART2_RX, GPIO_BASE_ADDR | GPIOL_IN_AUX1_SEL);
  GPIO_LO_BIT(UART2_RX, GPIO_BASE_ADDR | GPIOL_OUT_EN);
  GPIO_LO_BIT(UART2_RX, GPIO_BASE_ADDR | GPIOL_OUT_AUX2_SEL);
  GPIO_LO_BIT(UART2_RX, GPIO_BASE_ADDR | GPIOL_OUT_AUX1_SEL);


  BRANCH_RETURN(Cs5536SecUartInit)

  .end Cs5536SecUartInit

  .set reorder

