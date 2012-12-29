/*++

Copyright (c) 2010, kontais
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:
  8259.c

Abstract:

  This contains the installation function for the driver.

--*/

#include "InterruptDxe.h"
#include "CpuMips32.h"
#include "Mips.h"

#include EFI_PROTOCOL_DEFINITION (DebugSupport)

//
// Global for the Legacy 8259 Protocol that is prodiced by this driver
//
EFI_MIPS32_INTRRUPT_PROTOCOL  mMips32Interrupt = {
  Mips32InterruptGetVector,
  Mips32InterruptEnableIntr,
  Mips32InterruptDisableIntr,
  Mips32InterruptGetMask,
  Mips32InterruptSetMask,
  Mips32InterruptClearPendingIntr
};

//
// Global for the handle that the Legacy 8259 Protocol is installed
//
EFI_HANDLE                mMips32InterruptHandle             = NULL;

//
// Worker Functions
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
{
  UINT32 Mask;
  
  Mask = CP0_GetStatus();
  Mask = Mask & CP0_STATUS_IM;
  Mask = Mask >> CP0_STATUS_IM_SHIFT;

  *IntrMask = Mask;
  
  return EFI_SUCCESS;
}

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
{
  UINT32 Mask;
  UINT32 Status;
  
  Mask = *IntrMask;
  if (Mask & ~(CP0_STATUS_IM >> CP0_STATUS_IM_SHIFT)) {
    return EFI_INVALID_PARAMETER;
  }

  Mask   = Mask << CP0_STATUS_IM_SHIFT;
  Mask   = Mask & CP0_STATUS_IM;
  Status = CP0_GetStatus();
  Status = Status & ~CP0_STATUS_IM;
  Status = Status | Mask;
  CP0_SetStatus(Status);

  return EFI_SUCCESS;
}

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
{
  if (Intr >= EfiMips32IntrMax) {
    return EFI_INVALID_PARAMETER;
  }

  *Vector = Intr + EXCEPT_VECTOR_INTR_OFFSET;
  
  return EFI_SUCCESS;
}

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
{
  UINT32 Mask;
  UINT32 Status;

  if (Intr >= EfiMips32IntrMax) {
    return EFI_INVALID_PARAMETER;
  }
  
  Mask = 1 << (Intr + CP0_STATUS_IM_SHIFT);
  Status = CP0_GetStatus();  
  Status = Status | Mask;
  CP0_SetStatus(Status);
  
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
Mips32InterruptDisableIntr (
  IN EFI_MIPS32_INTRRUPT_PROTOCOL           * This,
  IN  EFI_MIPS32_INTR                       Intr
  )
/*++

  Routine Description:
    Disable Intr by masking interrupt in cp0 status

  Arguments:
    This           - Protocol instance pointer.
    Intr           - MIPS32 CP0 status interrupt 0 -7

  Returns:
    EFI_SUCCESS           - Intr enabled in status
    EFI_INVALID_PARAMETER - Intr not valid

--*/
{
  UINT32 Mask;
  UINT32 Status;

  if (Intr >= EfiMips32IntrMax) {
    return EFI_INVALID_PARAMETER;
  }
  
  Mask = 1 << (Intr + CP0_STATUS_IM_SHIFT);
  Status = CP0_GetStatus();  
  Status = Status & ~Mask;
  CP0_SetStatus(Status);
  
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
Mips32InterruptClearPendingIntr (
  IN EFI_MIPS32_INTRRUPT_PROTOCOL  * This,
  IN  EFI_MIPS32_INTR              Intr
  )
/*++

  Routine Description:
    Clear Intr pending in cp0 cause.
    if Intr is EfiMips32IntrMax then clear all pending status.

  Arguments:
    This           - Protocol instance pointer.
    Intr           - MIPS32 CP0 status interrupt 0 -7

  Returns:
    EFI_SUCCESS           - Clear success
    EFI_INVALID_PARAMETER - Intr not valid

--*/
{
  UINT32 Mask;
  UINT32 Cause;

  if (Intr > EfiMips32IntrMax) {
    return EFI_INVALID_PARAMETER;
  }

  Cause = CP0_GetCause();
  
  if (Intr == EfiMips32IntrMax) {
    CP0_SetCause(Cause & ~CP0_CAUSE_IP);
  } else {  
    Mask = 1 << (Intr + CP0_CAUSE_IP_SHIFT);
    Cause = Cause & ~Mask;
    CP0_SetCause(Cause);
  }
  
  return EFI_SUCCESS;
}

//
// Delclare Mips32 interrupt Driver Entry Point
//
EFI_DRIVER_ENTRY_POINT (InitializeInterrupt)

//
// Mips32 Interrupt Driver Entry Point
//
EFI_STATUS
EFIAPI
InitializeInterrupt (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
/*++

Routine Description:
  

Arguments:

  (Standard EFI Image entry - EFI_IMAGE_ENTRY_POINT)

Returns:

  EFI_SUCCESS - MIPS32 intrrupt  Protocol Installed

--*/
{
  EFI_STATUS   Status;
  UINT32       Mask;
  
  //
  // Initializwe the EFI Driver Library
  //
  EfiInitializeDriverLib (ImageHandle, SystemTable);

  //
  // Disable all interupt by status mask.
  //
  Mask = 0;
  Status = Mips32InterruptSetMask(&mMips32Interrupt, &Mask);
  if (Status != EFI_SUCCESS) {
    return Status;
  }

  //
  // Clear all pending status by status mask.
  //
  Status = Mips32InterruptClearPendingIntr(&mMips32Interrupt, EfiMips32IntrMax);
  if (Status != EFI_SUCCESS) {
    return Status;
  }

  //
  // Install Mips32 interupt Protocol onto a new handle
  //
  Status = gBS->InstallProtocolInterface (
                  &mMips32InterruptHandle,
                  &gEfiMips32InterruptProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &mMips32Interrupt
                  );

  return Status;
}

