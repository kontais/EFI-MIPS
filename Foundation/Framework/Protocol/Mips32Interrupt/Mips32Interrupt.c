/*++

Copyright (c) 2010, kontais
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.


Module Name:
  
    Mips32Interrupt.c
    
Abstract:

  EFI MIPS32 cp0 interanl interrupt control.

Revision History

--*/

#include "Tiano.h"

#include EFI_PROTOCOL_DEFINITION (Mips32Interrupt)

EFI_GUID  gEfiMips32InterruptProtocolGuid = EFI_MIPS32_INTRRUPT_PROTOCOL_GUID;

EFI_GUID_STRING(&gEfiMips32InterruptProtocolGuid, "MIPS32 Interrupt Protocol", "MIPS32 Interrupt Protocol GUID");
