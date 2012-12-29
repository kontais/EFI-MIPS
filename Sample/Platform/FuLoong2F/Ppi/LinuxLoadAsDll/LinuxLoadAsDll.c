/*++

Copyright (c) 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:
  
    LinuxLoadAsDll.c

Abstract:

  DXE Initial Program Load PPI GUID as defined in Tiano

--*/

#include "Tiano.h"
#include "Pei.h"
#include EFI_PPI_DEFINITION (LinuxLoadAsDll)

EFI_GUID  gEfiLinuxLoadAsDllPpiGuid = EFI_LINUX_LOAD_AS_DLL_PPI_GUID;

EFI_GUID_STRING(&gEfiLinuxLoadAsDllPpiGuid, "LinuxLoadAsDll", "LINUX LOAD AS DLL PPI");
