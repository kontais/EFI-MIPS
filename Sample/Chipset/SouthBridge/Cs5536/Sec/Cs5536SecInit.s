/*++

Copyright (c) 2009, kontais                                                                   
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  Cs5536SecInit.s

Abstract:

  Cs5536 PowerOn init.

--*/



#include "Mips.h"
#include "BonitoReg.h"
#include "BonitoPci.h"
#include "GlxReg.h"
#include "GlxPci.h"
#include "Cs5536SecLib.h"
#include "Cs5536SecInit.h"


  .text
  .align 3
  .set noreorder

  .globl Cs5536SecInit
  .ent   Cs5536SecInit
  .type  Cs5536SecInit, @function

Cs5536SecInit:
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Invoke: 
  
  BRANCH_CALL(Cs5536SecInit)

Routine Description:

  None

Arguments:

  None

Returns:

  None

----------------------------------------------------------------*/

  // Set the MSR enable
  PCI_WRITE32(CS5536_PCI_IDSEL, CS5536_PCI_FUNC_ISA, PCI_MSR_CTRL, 0x01);

  // Active all the ports
  MSR_WRITE(GLIU_PAE, 0x0000ffff, 0x0);
  

  // Global Control Setting
  MSR_WRITE(GLPCI_CTRL, 0x00000003, 0x44000030);

  // Local Base Address Register(LBAR) Setting
  MSR_WRITE(DIVIL_LBAR_SMB,     SMB_BASE_ADDR,   0xf001);
  MSR_WRITE(DIVIL_LBAR_GPIO,    GPIO_BASE_ADDR,  0xf001);
  //MSR_WRITE(DIVIL_LBAR_MFGPT, MFGPT_BASE_ADDR, 0xf001);
  //MSR_WRITE(DIVIL_LBAR_ACPI,  ACPI_BASE_ADDR,  0xf001);
  MSR_WRITE(DIVIL_LBAR_PMS,     PMS_BASE_ADDR,   0xf001);


  BRANCH_CALL(Cs5536SecUartInit)
  BRANCH_CALL(Cs5536SecGpioInit)
  BRANCH_CALL(Cs5536SecPwrMngSupport)
  BRANCH_CALL(Cs5536SecSMBusInit)


  BRANCH_RETURN(Cs5536SecInit)

  .end Cs5536SecInit

  .set reorder

