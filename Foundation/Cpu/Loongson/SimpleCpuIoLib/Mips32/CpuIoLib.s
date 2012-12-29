/*++

Copyright (c) 2009, kontais                                                                   
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  CpuIoLib.s

Abstract:

  Supports MIPS32 CPU IO operation

--*/

#include "Mips.h"
#include "BonitoReg.h"

  .text
  .align 3
  .set noreorder

 /*------------------------------------------------------------------------------
 *  UINT8
 *  CpuIoRead8 (
 *    IN  UINT16  Port
 *    )
 *-----------------------------------------------------------------------------*/
LEAF(CpuIoRead8)
  li    v1, PHYS_TO_UNCACHED(BONITO_PCIIO_BASE)
  add   a0, v1
  lbu   v0, 0(a0)
  nop
  jr    ra
  nop
END(CpuIoRead8)

 /*------------------------------------------------------------------------------
 *  UINT16
 *  CpuIoRead16 (
 *    IN  UINT16  Port
 *    )
 *-----------------------------------------------------------------------------*/
LEAF(CpuIoRead16)
  li    v1, PHYS_TO_UNCACHED(BONITO_PCIIO_BASE)
  add   a0, v1
  lhu   v0, 0(a0)
  nop
  jr    ra
  nop
END(CpuIoRead16)

 /*------------------------------------------------------------------------------
 *  UINT32
 *  CpuIoRead32 (
 *    IN  UINT16  Port
 *    )
 *-----------------------------------------------------------------------------*/
LEAF(CpuIoRead32)
  li    v1, PHYS_TO_UNCACHED(BONITO_PCIIO_BASE)
  add   a0, v1
  lw    v0, 0(a0)
  nop
  jr    ra
  nop
END(CpuIoRead32)



 /*------------------------------------------------------------------------------
 *  VOID
 *  CpuIoWrite8 (
 *    IN  UINT16  Port,
 *    IN  UINT8   Data
 *    )
 *-----------------------------------------------------------------------------*/
LEAF(CpuIoWrite8)
  li    v1, PHYS_TO_UNCACHED(BONITO_PCIIO_BASE)
  add   a0, v1
  sb    a1, 0(a0)
  nop
  jr    ra
  nop
END(CpuIoWrite8)


 /*------------------------------------------------------------------------------
 *  VOID
 *  CpuIoWrite16 (
 *    IN  UINT16  Port,
 *    IN  UINT16  Data
 *    )
 *-----------------------------------------------------------------------------*/
LEAF(CpuIoWrite16)
  li    v1, PHYS_TO_UNCACHED(BONITO_PCIIO_BASE)
  add   a0, v1
  sh    a1, 0(a0)
  nop
  jr    ra
  nop
END(CpuIoWrite16)


 /*------------------------------------------------------------------------------
 *  VOID
 *  CpuIoWrite32 (
 *    IN  UINT16  Port,
 *    IN  UINT32  Data
 *    )
 *-----------------------------------------------------------------------------*/
LEAF(CpuIoWrite32)
  li    v1, PHYS_TO_UNCACHED(BONITO_PCIIO_BASE)
  add   a0, v1
  sw    a1, 0(a0)
  nop
  jr    ra
  nop
END(CpuIoWrite32)

  .set reorder
