/*++

Copyright (c) 2009, kontais                                                                   
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  Via686bSecInit.s

Abstract:

  Via686b PowerOn init.

--*/



#include "Mips.h"
#include "BonitoReg.h"
#include "Via686bSecInit.h"
#include "Via686b.h"

  .text
  .align 3
  .set noreorder
  
  .globl Via686bSecInit
  .ent   Via686bSecInit
  .type  Via686bSecInit, @function

Via686bSecInit:
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Invoke: 
  
  BRANCH_CALL(Via686bSecInit)

Routine Description:

  None

Arguments:

  None

Returns:

  None

----------------------------------------------------------------*/

  
  
  BRANCH_CALL(SIO_DecodeInit)
  
  
  BRANCH_RETURN(Via686bSecInit)


  .end Via686bSecInit



  .globl SIO_DecodeInit
  .ent   SIO_DecodeInit
  .type  SIO_DecodeInit, @function

SIO_DecodeInit:
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Invoke: 
  
  BRANCH_CALL(SIO_DecodeInit)

Routine Description:

  SouthBridge SuperIO IO space decode enable

Arguments:

  None

Returns:

  None

----------------------------------------------------------------*/


  BRANCH_RETURN(SIO_DecodeInit)  
  
  .end SIO_DecodeInit


  .set reorder

