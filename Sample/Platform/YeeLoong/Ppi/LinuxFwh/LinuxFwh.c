/*++

Copyright (c) 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

 LinuxFwh.c

Abstract:

  NT FWH Information PPI GUID as defined in Tiano

--*/

#include "Tiano.h"
#include "PeiBind.h"
#include "PeiApi.h"
#include EFI_PPI_DEFINITION (LinuxFwh)

EFI_GUID  gLinuxFwhPpiGuid = LINUX_FWH_PPI_GUID;

EFI_GUID_STRING(&gLinuxFwhPpiGuid, "LinuxFwh", "LINUX PEI FWH INFO PPI");
