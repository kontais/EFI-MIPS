/*++

Copyright (c) 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

 LinuxThunkPpi.c

Abstract:

  Abstraction for the LINUX Thunk PPI

--*/

#include "Tiano.h"
#include "PeiBind.h"
#include "PeiApi.h"
#include EFI_PPI_DEFINITION (LinuxThunkPpi)

EFI_GUID  gPeiLinuxThunkPpiGuid = PEI_LINUX_THUNK_GUID;

EFI_GUID_STRING(&gPeiLinuxThunkPpiGuid, "LinuxThunk", "LINUX PEI WINNT THUNK PPI");
