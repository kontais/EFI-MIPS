/*++

Copyright (c) 2010, kontais                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  EfiCpuVersion.c
  
Abstract:
  
  Provide processor implement and revision.
--*/

#include "CpuMips32.h"
#include "Mips.h"

VOID
EfiCpuVersion (
  IN  OUT UINT8   *Imp,  OPTIONAL
  IN  OUT UINT8   *Rev   OPTIONAL
  )
/*++

Routine Description:
  Extract CPU detail version infomation

Arguments:
  Imp        - MIPS ISA implement 
  Rev        - Revision 

--*/
{
  UINT32  ProcessorId;
  
  ProcessorId = EfiCpuid ();
  
  if (Imp != NULL) {
    *Imp = (UINT8) ((ProcessorId >> CP0_PRID_IMP_SHIFT) & CP0_PRID_IMP_MASK);
  }

  if (Rev != NULL) {
    *Rev = (UINT8) ((ProcessorId >> CP0_PRID_REV_SHIFT) & CP0_PRID_REV_MASK);
  }
}

