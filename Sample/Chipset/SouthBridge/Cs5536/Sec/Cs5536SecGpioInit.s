/*++

Copyright (c) 2011, kontais                                                                   
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  Cs5536SecGpioInit.s

Abstract:

  Cs5536 IDE IRQ, CS5536 INTR, PWN_BUT GPIO.

--*/



#include "Mips.h"
#include "BonitoReg.h"
#include "BonitoPci.h"
#include "GlxReg.h"
#include "GlxPci.h"
#include "Cs5536SecLib.h"
#include "Cs5536SecInit.h"

#define  MIDE_IRQ   2
#define  I8259_IRQ  12
#define  PWR_BUT    13


  .text
  .align 3
  .set noreorder

  .globl Cs5536SecGpioInit
  .ent   Cs5536SecGpioInit
  .type  Cs5536SecGpioInit, @function

Cs5536SecGpioInit:
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Invoke: 
  
  BRANCH_CALL(Cs5536SecGpioInit)

Routine Description:

  None

Arguments:

  None

Returns:

  None

----------------------------------------------------------------*/

  // GPIO2 use as IDE_IRQ0
  GPIO_HI_BIT(MIDE_IRQ, GPIO_BASE_ADDR | GPIOL_IN_EN);
  GPIO_HI_BIT(MIDE_IRQ, GPIO_BASE_ADDR | GPIOL_IN_AUX1_SEL);
  GPIO_LO_BIT(MIDE_IRQ, GPIO_BASE_ADDR | GPIOL_OUT_EN);
  GPIO_LO_BIT(MIDE_IRQ, GPIO_BASE_ADDR | GPIOL_OUT_AUX1_SEL);
  GPIO_LO_BIT(MIDE_IRQ, GPIO_BASE_ADDR | GPIOL_OUT_AUX2_SEL);

  // GPIO12 5536_INTR -> CPU INTn0
  GPIO_HI_BIT(I8259_IRQ, GPIO_BASE_ADDR | GPIOL_OUT_EN);
  GPIO_HI_BIT(I8259_IRQ, GPIO_BASE_ADDR | GPIOL_OUT_AUX1_SEL);
  GPIO_LO_BIT(I8259_IRQ, GPIO_BASE_ADDR | GPIOL_OUT_OD_EN);
  GPIO_LO_BIT(I8259_IRQ, GPIO_BASE_ADDR | GPIOL_OUT_AUX2_SEL);
  GPIO_LO_BIT(I8259_IRQ, GPIO_BASE_ADDR | GPIOL_IN_EN);
  GPIO_LO_BIT(I8259_IRQ, GPIO_BASE_ADDR | GPIOL_IN_AUX1_SEL);

  // GPIO13 use as PWN_BUT
  GPIO_HI_BIT(PWR_BUT, GPIO_BASE_ADDR | GPIOH_IN_EN);
  GPIO_HI_BIT(PWR_BUT, GPIO_BASE_ADDR | GPIOH_IN_AUX1_SEL);
  GPIO_LO_BIT(PWR_BUT, GPIO_BASE_ADDR | GPIOH_OUT_EN);
  GPIO_LO_BIT(PWR_BUT, GPIO_BASE_ADDR | GPIOH_OUT_AUX1_SEL);
  GPIO_LO_BIT(PWR_BUT, GPIO_BASE_ADDR | GPIOH_OUT_AUX2_SEL);


  BRANCH_RETURN(Cs5536SecGpioInit)

  .end Cs5536SecGpioInit

  .set reorder

