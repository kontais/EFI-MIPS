/*++

Copyright (c) 2011, kontais                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  Sis315eUgaIo.c

Abstract:

  This protocol allows an EFI driver in the Linux emulation envirnment
  to make Linux API calls.

--*/

#include "Tiano.h"
#include EFI_PROTOCOL_DEFINITION (Sis315eUgaIo)

EFI_GUID  gEfiSis315eUgaIoProtocolGuid = EFI_SIS315E_UGA_IO_PROTOCOL_GUID;

EFI_GUID_STRING(&gEfiSis315eUgaIoProtocolGuid, "EFI SIS315E UGA IO", "SIS315E UGA IO protocol");
