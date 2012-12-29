/*++

Copyright (c) 2009, kontais                                            
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  ElfLoader.c

Abstract:

  ELF executable & relocatable object file loader 

Revision History

--*/

#include "Tiano.h"

#include EFI_GUID_DEFINITION (PeiElfLoader)

int Func1()
{
  int a;
  a = 0;
  a = 12345678;
  return 0;
}

STATIC
EFI_STATUS
EfiElfLoaderGetElfHeader (
  IN OUT EFI_PEI_ELF_LOADER_IMAGE_CONTEXT       *ImageContext,
  OUT    Elf32_Ehdr                         *Hdr
  )
/*++

Routine Description:

  Retrieves the ELF Header from a ELF executable & relocatable object file

Arguments:

  ImageContext  - The context of the image being loaded

  Hdr         - The buffer in which to return the ELF header

Returns:

  EFI_SUCCESS if the ELF Header is read, 
  Otherwise, the error status from reading the ELF executable & relocatable 
  object file image using the ImageRead function.

--*/
{

    // NOTE: now we just support Relocatable & Executable ELF file.
    //
    if (Hdr->e_type == ET_REL || Hdr->e_type == ET_EXEC) {
      ImageContext->FileType  = Hdr->e_type;
    } else {
      ImageContext->ImageError = EFI_ELF_IMAGE_ERROR_UNSUPPORTED_ELF_FILE_TYPE;
      return EFI_UNSUPPORTED;
    }
  return EFI_SUCCESS;
}

