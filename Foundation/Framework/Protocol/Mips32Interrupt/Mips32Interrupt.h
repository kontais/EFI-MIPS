/*++

Copyright (c) 2010, kontais
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.


Module Name:

  Mips32Intrrupt.h
    
Abstract:

  This protocol abstracts the MIPS32 CP0 internal interrupt controller.

Revision History

  None.

--*/

#ifndef _EFI_MIPS32_INTRRUPT_H
#define _EFI_MIPS32_INTRRUPT_H

#define EFI_MIPS32_INTRRUPT_PROTOCOL_GUID \
  { \
    0xee0171f6, 0x8a58, 0x4244, {0xb4, 0xbc, 0x5c, 0x7f, 0xf5, 0x05, 0xb7, 0x08} \
  }

EFI_FORWARD_DECLARATION (EFI_MIPS32_INTRRUPT_PROTOCOL);

typedef enum {
  EfiMips32Intr0,
  EfiMips32Intr1,
  EfiMips32Intr2,
  EfiMips32Intr3,
  EfiMips32Intr4,
  EfiMips32Intr5,
  EfiMips32Intr6,
  EfiMips32Intr7,
  EfiMips32IntrMax
} EFI_MIPS32_INTR;

typedef
EFI_STATUS
(EFIAPI *EFI_MIPS32_INTRRUPT_GET_MASK) (
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

typedef
EFI_STATUS
(EFIAPI *EFI_MIPS32_INTRRUPT_SET_MASK) (
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

typedef
EFI_STATUS
(EFIAPI *EFI_MIPS32_INTRRUPT_GET_VECTOR) (
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

typedef
EFI_STATUS
(EFIAPI *EFI_MIPS32_INTRRUPT_ENABLE_INTR) (
  IN EFI_MIPS32_INTRRUPT_PROTOCOL           * This,
  IN  EFI_MIPS32_INTR                      Intr
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

typedef
EFI_STATUS
(EFIAPI *EFI_MIPS32_INTRRUPT_DISABLE_INTR) (
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

typedef
EFI_STATUS
(EFIAPI *EFI_MIPS32_INTRRUPT_CLEAR_PENDING_INTR) (
  IN EFI_MIPS32_INTRRUPT_PROTOCOL                  * This,
  IN  EFI_MIPS32_INTR                              Intr
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

struct _EFI_MIPS32_INTRRUPT_PROTOCOL {
  EFI_MIPS32_INTRRUPT_GET_VECTOR          GetVector;
  EFI_MIPS32_INTRRUPT_ENABLE_INTR         EnableIntr;
  EFI_MIPS32_INTRRUPT_DISABLE_INTR        DisableIntr;
  EFI_MIPS32_INTRRUPT_GET_MASK            GetMask;
  EFI_MIPS32_INTRRUPT_SET_MASK            SetMask;
  EFI_MIPS32_INTRRUPT_CLEAR_PENDING_INTR  ClearPendingIntr;
};

extern EFI_GUID gEfiMips32InterruptProtocolGuid;

#endif
