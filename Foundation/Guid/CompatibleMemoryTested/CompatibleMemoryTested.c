/*++

Copyright (c) 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  CompatibleMemoryTested.c
    
Abstract:

  Tiano Guid used for all Compatible Memory Range Tested GUID.  

--*/

#include "Tiano.h"
#include EFI_GUID_DEFINITION (CompatibleMemoryTested)

EFI_GUID  gEfiCompatibleMemoryTestedGuid = EFI_COMPATIBLE_MEMORY_TESTED_PROTOCOL_GUID;

EFI_GUID_STRING
  (&gEfiCompatibleMemoryTestedGuid, "CompatibleMemoryTested Protocol", "Tiano Generic Memory Test Protocol");
