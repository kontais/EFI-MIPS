/*++

Copyright (c) 2010, konais
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:
  8259.h

Abstract:

  Driver implementing the Tiano Legacy 8259 Protocol

--*/

#ifndef _INTERRUPT_DXE_H__
#define _INTERRUPT_DXE_H__

#include "Tiano.h"
#include "EfiDriverLib.h"

//
// Produced protocols
//
#include EFI_PROTOCOL_PRODUCER (Mips32Interrupt)

//
// Interrupt definitions
//


//
// Protocol Function Prototypes
//
EFI_STATUS
EFIAPI
Mips32InterruptGetMask (
  IN EFI_MIPS32_INTRRUPT_PROTOCOL        * This,
  OUT UINT32                             *IntrMask OPTIONAL
  )
/*++

  Routine Description:
    Get the MIPS32 CP0 internal interrupt mask.

  Arguments:
    This               - Protocol instance pointer.
    IntrMask         - Bit 0 is Intr0 - Bit 7 is Intr7

  Returns:
    EFI_SUCCESS

--*/
;

EFI_STATUS
EFIAPI
Mips32InterruptSetMask (
  IN EFI_MIPS32_INTRRUPT_PROTOCOL        * This,
  OUT UINT32                             *IntrMask OPTIONAL
  )
/*++

  Routine Description:
    Set the MIPS32 CP0 internal interrupt mask.

  Arguments:
    This               - Protocol instance pointer.
    IntrMask         - Bit 0 is Intr0 - Bit 7 is Intr7

  Returns:
    EFI_SUCCESS
    EFI_INVALID_PARAMETER - Mask not valid

--*/
;

EFI_STATUS
EFIAPI
Mips32InterruptGetVector (
  IN EFI_MIPS32_INTRRUPT_PROTOCOL          * This,
  IN  EFI_MIPS32_INTR                      Intr,
  OUT UINT32                               *Vector
  )
/*++

  Routine Description:
    Convert from INTR number to processor interrupt vector number.

  Arguments:
    This    - Protocol instance pointer.
    Intr    - MIPS32 CP0 status interrupt 0 -7
    Vector  - Processor vector number

  Returns:
    EFI_SUCCESS
    EFI_INVALID_PARAMETER - Intr not valid

--*/
;

EFI_STATUS
EFIAPI
Mips32InterruptEnableIntr (
  IN EFI_MIPS32_INTRRUPT_PROTOCOL           * This,
  IN  EFI_MIPS32_INTR                       Intr
  )
/*++

  Routine Description:
    Enable Intr by unmasking interrupt in cp0 status

  Arguments:
    This           - Protocol instance pointer.
    Intr           - MIPS32 CP0 status interrupt 0 -7

  Returns:
    EFI_SUCCESS           - Intr enabled in status
    EFI_INVALID_PARAMETER - Intr not valid

--*/
;

EFI_STATUS
EFIAPI
Mips32InterruptDisableIntr (
  IN EFI_MIPS32_INTRRUPT_PROTOCOL           * This,
  IN  EFI_MIPS32_INTR                      Intr
  )
/*++

  Routine Description:
    Disable Intr by masking interrupt in cp0 status
    if Intr is EfiMips32CauseIntrMax then disable all Intr.

  Arguments:
    This           - Protocol instance pointer.
    Intr           - MIPS32 CP0 status interrupt 0 -7

  Returns:
    EFI_SUCCESS           - Intr enabled in status
    EFI_INVALID_PARAMETER - Intr not valid

--*/
;

EFI_STATUS
EFIAPI
Mips32InterruptClearPendingIntr (
  IN EFI_MIPS32_INTRRUPT_PROTOCOL          * This,
  IN  EFI_MIPS32_INTR                      Intr
  )
/*++

  Routine Description:
    Clear Intr pending in cp0 cause.
    if Intr is EfiMips32CauseIntrMax then clear all pending status.

  Arguments:
    This           - Protocol instance pointer.
    Intr           - MIPS32 CP0 status interrupt 0 -7

  Returns:
    EFI_SUCCESS           - Clear success
    EFI_INVALID_PARAMETER - Intr not valid

--*/
;

#endif
