/*++

Copyright (c) 2011, kontais                                                                   
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  BonitoIo.s

Abstract:

  Bonito ISA IO implement

--*/

#include "Mips.h"
#include "BonitoReg.h"

  .text
  .align 3
  .set noreorder

 /*------------------------------------------------------------------------------
 *  UINT8
 *  IoRead8 (
 *    IN  UINT16  Port
 *    )
 *-----------------------------------------------------------------------------*/
LEAF(IoRead8)
  li    v1, PHYS_TO_UNCACHED(BONITO_PCIIO_BASE)
  add   a0, v1
  lbu   v0, 0(a0)
  nop
  jr    ra
  nop
END(IoRead8)

 /*------------------------------------------------------------------------------
 *  UINT16
 *  IoRead16 (
 *    IN  UINT16  Port
 *    )
 *-----------------------------------------------------------------------------*/
LEAF(IoRead16)
  li    v1, PHYS_TO_UNCACHED(BONITO_PCIIO_BASE)
  add   a0, v1
  lhu   v0, 0(a0)
  nop
  jr    ra
  nop
END(IoRead16)

 /*------------------------------------------------------------------------------
 *  UINT32
 *  IoRead32 (
 *    IN  UINT16  Port
 *    )
 *-----------------------------------------------------------------------------*/
LEAF(IoRead32)
  li    v1, PHYS_TO_UNCACHED(BONITO_PCIIO_BASE)
  add   a0, v1
  lw    v0, 0(a0)
  nop
  jr    ra
  nop
END(IoRead32)



 /*------------------------------------------------------------------------------
 *  VOID
 *  IoWrite8 (
 *    IN  UINT16  Port,
 *    IN  UINT8   Data
 *    )
 *-----------------------------------------------------------------------------*/
LEAF(IoWrite8)
  li    v1, PHYS_TO_UNCACHED(BONITO_PCIIO_BASE)
  add   a0, v1
  sb    a1, 0(a0)
  nop
  jr    ra
  nop
END(IoWrite8)


 /*------------------------------------------------------------------------------
 *  VOID
 *  IoWrite16 (
 *    IN  UINT16  Port,
 *    IN  UINT16  Data
 *    )
 *-----------------------------------------------------------------------------*/
LEAF(IoWrite16)
  li    v1, PHYS_TO_UNCACHED(BONITO_PCIIO_BASE)
  add   a0, v1
  sh    a1, 0(a0)
  nop
  jr    ra
  nop
END(IoWrite16)


 /*------------------------------------------------------------------------------
 *  VOID
 *  IoWrite32 (
 *    IN  UINT16  Port,
 *    IN  UINT32  Data
 *    )
 *-----------------------------------------------------------------------------*/
LEAF(IoWrite32)
  li    v1, PHYS_TO_UNCACHED(BONITO_PCIIO_BASE)
  add   a0, v1
  sw    a1, 0(a0)
  nop
  jr    ra
  nop
END(IoWrite32)

 /*------------------------------------------------------------------------------
 *  VOID
 *  IoMaskSet8 (
 *    IN  UINT16  Port,
 *    IN  UINT8   Data
 *    )
 *-----------------------------------------------------------------------------*/
LEAF(IoMaskSet8)
  li    v1, PHYS_TO_UNCACHED(BONITO_PCIIO_BASE)
  add   a0, v1
  lbu   v0,  0(a0)
  nop
  or    v0, a1
  sb    v0, 0(a0)
  jr    ra
  nop
END(IoMaskSet8)

 /*------------------------------------------------------------------------------
 *  VOID
 *  IoMaskSet16 (
 *    IN  UINT16  Port,
 *    IN  UINT16  Data
 *    )
 *-----------------------------------------------------------------------------*/
LEAF(IoMaskSet16)
  li    v0, PHYS_TO_UNCACHED(BONITO_PCIIO_BASE)
  add   a0, v0
  lhu   v1,  0(a0)
  nop
  or    v1, a1
  sh    v1, 0(a0)
  jr    ra
  nop
END(IoMaskSet16)

 /*------------------------------------------------------------------------------
 *  VOID
 *  IoMaskSet32 (
 *    IN  UINT16  Port,
 *    IN  UINT32  Data
 *    )
 *-----------------------------------------------------------------------------*/
LEAF(IoMaskSet32)
  li    v0, PHYS_TO_UNCACHED(BONITO_PCIIO_BASE)
  add   a0, v0
  lw    v1,  0(a0)
  nop
  or    v1, a1
  sw    v1, 0(a0)
  jr    ra
  nop
END(IoMaskSet32)

 /*------------------------------------------------------------------------------
 *  VOID
 *  IoMaskClr8 (
 *    IN  UINT16  Port,
 *    IN  UINT8   Data
 *    )
 *-----------------------------------------------------------------------------*/
LEAF(IoMaskClr8)
  li    v0, PHYS_TO_UNCACHED(BONITO_PCIIO_BASE)
  add   a0, v0
  lbu   v1, 0(a0)
  nop
  not   a1
  and   v1, a1
  sb    v1, 0(a0)
  jr    ra
  nop
END(IoMaskClr8)

 /*------------------------------------------------------------------------------
 *  VOID
 *  IoMaskClr16 (
 *    IN  UINT16  Port,
 *    IN  UINT16  Data
 *    )
 *-----------------------------------------------------------------------------*/
LEAF(IoMaskClr16)
  li    v0, PHYS_TO_UNCACHED(BONITO_PCIIO_BASE)
  add   a0, v0
  lhu   v1, 0(a0)
  nop
  not   a1
  and   v1, a1
  sh    v1, 0(a0)
  jr    ra
  nop
END(IoMaskClr16)

 /*------------------------------------------------------------------------------
 *  VOID
 *  IoMaskClr32 (
 *    IN  UINT16  Port,
 *    IN  UINT32  Data
 *    )
 *-----------------------------------------------------------------------------*/
LEAF(IoMaskClr32)
  li    v0, PHYS_TO_UNCACHED(BONITO_PCIIO_BASE)
  add   a0, v0
  lw    v1, 0(a0)
  nop
  not   a1
  and   v1, a1
  sw    v1, 0(a0)
  jr    ra
  nop
END(IoMaskClr32)

  .set reorder

