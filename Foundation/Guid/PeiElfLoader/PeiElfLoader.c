/*++

Copyright (c) 2009, kontais                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:
  
    EfiElfLoader.c
    
Abstract:

  GUID for the ELF executable & relocatable object file Loader APIs shared 
  between PEI and DXE

--*/

#include "Tiano.h"
#include EFI_GUID_DEFINITION(PeiElfLoader)

EFI_GUID gEfiPeiElfLoaderGuid  = EFI_PEI_ELF_LOADER_GUID;

EFI_GUID_STRING(&gEfiElfLoaderGuid, "EFI ELF Loader", "EFI ELF Loader APIs");

