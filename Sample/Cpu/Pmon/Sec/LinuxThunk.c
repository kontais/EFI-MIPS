/*++

Copyright (c) 2004 - 2009, Intel Corporation                                                         
Portions copyright (c) 2008-2009 Apple Inc. All rights reserved.
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  LinuxThunk.c

Abstract:

  Since the SEC is the only program in our emulation we 
  must use a Tiano mechanism to export APIs to other modules.
  This is the role of the EFI_LINUX_THUNK_PROTOCOL.

  The mLinuxThunkTable exists so that a change to EFI_LINUX_THUNK_PROTOCOL
  will cause an error in initializing the array if all the member functions
  are not added. It looks like adding a element to end and not initializing
  it may cause the table to be initaliized with the members at the end being
  set to zero. This is bad as jumping to zero will crash.
  

  gLinux is a a public exported global that contains the initialized
  data.

--*/

#include "Tiano.h"
#include "SecMain.h"

EFI_LINUX_THUNK_PROTOCOL mLinuxThunkTable = {
  EFI_LINUX_THUNK_PROTOCOL_SIGNATURE,
};

EFI_LINUX_THUNK_PROTOCOL *gLinux = &mLinuxThunkTable;

