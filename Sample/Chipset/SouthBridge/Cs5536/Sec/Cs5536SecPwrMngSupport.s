/*++

Copyright (c) 2011, kontais                                                                   
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  Cs5536SecPwrMngSupport.s

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

// 0x18000/32KHz = 3s
#define HW_POWER_OFF_TIME 0x18000 


  .text
  .align 3
  .set noreorder

  .globl Cs5536SecPwrMngSupport
  .ent   Cs5536SecPwrMngSupport
  .type  Cs5536SecPwrMngSupport, @function

Cs5536SecPwrMngSupport:
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Invoke: 
  
  BRANCH_CALL(Cs5536SecPwrMngSupport)

Routine Description:

  None

Arguments:

  None

Returns:

  None

----------------------------------------------------------------*/

  // Power Button Enable
  IO_MASK_SET32(PMS_BASE_ADDR | 0x40, (1 << 30));

  // Power Button Delay
  IO_MASK_SET32(PMS_BASE_ADDR | 0x40, (HW_POWER_OFF_TIME << 0));

  // Power Button Lock
  IO_MASK_SET32(PMS_BASE_ADDR | 0x40, (1 << 31));


  BRANCH_RETURN(Cs5536SecPwrMngSupport)

  .end Cs5536SecPwrMngSupport

  .set reorder

