/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:
  
  RtData.c

Abstract:

  EFI library global data

Revision History

--*/

#include "EfiShellLib.h"

//
// RT - pointer to the runtime table
//
EFI_RUNTIME_SERVICES          *RT;

//
// EFIDebug - Debug mask
//
UINTN                         EFIDebug = EFI_DBUG_MASK;

//
// LibRuntimeDebugOut - Runtime Debug Output device
//
EFI_SIMPLE_TEXT_OUT_PROTOCOL  *LibRuntimeDebugOut;

