/*++

Copyright (c) 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  EfiLinuxLib.h

Abstract:

  Set up gLinux pointer so we can call Linux APIs.

--*/

#ifndef _EFI_LINUX_LIB_H_
#define _EFI_LINUX_LIB_H_

extern EFI_LINUX_THUNK_PROTOCOL  *gLinux;

EFI_STATUS
EfiInitializeLinuxDriverLib (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
/*++

Routine Description:

  Intialize gLinux and initialize debug console. 

Arguments:

  ImageHandle     - The firmware allocated handle for the EFI image.
  
  SystemTable     - A pointer to the EFI System Table.

Returns: 

  Status code

--*/
;

//
// LinuxDebugConsole Prototypes
//
VOID
LinuxDebugConsoleInit (
  VOID
  )
/*++

Routine Description:

  Linux debug console initialize.

Arguments:

  None

Returns:

  None

--*/
;

#endif
