/*++

Copyright (c) 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  LinuxUgaIo.c

Abstract:

  This protocol allows an EFI driver in the Linux emulation envirnment
  to make Linux API calls.

--*/

#include "Tiano.h"
#include EFI_PROTOCOL_DEFINITION (LinuxUgaIo)

EFI_GUID  gEfiLinuxUgaIoProtocolGuid = EFI_LINUX_UGA_IO_PROTOCOL_GUID;

EFI_GUID_STRING(&gEfiLinuxUgaIoProtocolGuid, "EFI Linux UGA IO", "Linux API UGA IO protocol");
