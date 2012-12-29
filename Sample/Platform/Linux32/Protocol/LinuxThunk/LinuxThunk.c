/*++

Copyright (c) 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  LinuxThunk.c

Abstract:

  This protocol allows an EFI driver in the Linux emulation envirnment
  to make Linux API calls.

--*/

#include "Tiano.h"
#include EFI_PROTOCOL_DEFINITION (LinuxThunk)

EFI_GUID  gEfiLinuxThunkProtocolGuid = EFI_LINUX_THUNK_PROTOCOL_GUID;

EFI_GUID_STRING(&gEfiLinuxThunkProtocolGuid, "EFI Linux Thunk", "Linux API thunk protocol");
