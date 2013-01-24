/*++

Copyright (c) 2009, kontais                                            
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  ElfRelocate.c

Abstract:

  Relocate PowerPC Relocation Section.

Revision History

--*/

#include "Tiano.h"

#include EFI_GUID_DEFINITION (PeiElfLoader)

EFI_STATUS
EFIAPI
EfiElfLoaderRelocateImage (
  IN     EFI_PEI_ELF_LOADER_PROTOCOL   *This,
  IN OUT EFI_PEI_ELF_LOADER_IMAGE_CONTEXT  *ImageContext
  )
/*++

Routine Description:

  Relocate a ELF file image in memory

Arguments:

  This         - Calling context

  ImageContext - Contains information on the loaded image to relocate

Returns:

  EFI_SUCCESS      if the EFL file image was relocated
  EFI_LOAD_ERROR   if the image is not a valid EFL file image
  EFI_UNSUPPORTED  not support

--*/
{
  EFI_STATUS             Status;
  Elf32_Shdr             SectionHeader;
  UINTN                  SectionHeaderOffset;
  Elf32_Shdr             *RelaHdr;
  Elf32_Rela             *RelaEntry;
  Elf32_Addr             RelaBase;
  Elf32_Rela             *RelaEnd;
  Elf32_Half             SymTabIndex;
  Elf32_Sym              *SymbolEntry;
  Elf32_Addr             SymbolTableBase;
  Elf32_Word             NumberOfSymbols;
  Elf32_Addr             FromBase;
  Elf32_Addr             LocationBase;
  UINT32                 *Location;
  UINT32                 Value;
  Elf32_Half             Index;
  UINTN                  Size;

  if (NULL == ImageContext) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Assume success
  //
  ImageContext->ImageError = EFI_ELF_IMAGE_ERROR_SUCCESS;

  if (!ImageContext->IsRelocatable) {
    return EFI_SUCCESS;
  }

  SymTabIndex     = ImageContext->SymbolTableIndex;
  NumberOfSymbols = ImageContext->NumberOfSymbols;
  SymbolTableBase = ImageContext->SectionLoadAddress[SymTabIndex];

  for (Index = 0; Index < ImageContext->NumberOfSections; Index++) {
    SectionHeaderOffset = ImageContext->SectionHeaderOffset +
                          Index * sizeof(Elf32_Shdr);
    Size   = sizeof(Elf32_Shdr);
    Status = ImageContext->ImageRead (
                               ImageContext->Handle,
                               SectionHeaderOffset,
                               &Size,
                               &SectionHeader
                               );
    if (EFI_ERROR(Status)) {
        ImageContext->ImageError = EFI_ELF_IMAGE_ERROR_IMAGE_READ;
      return Status;
    }

    //
    //PowerPC do not use Rel section
    //
    if ( SectionHeader.sh_type != SHT_RELA) {
      continue;
    }

    RelaHdr = &SectionHeader;
    if (RelaHdr->sh_entsize != sizeof(Elf32_Rela)) {
      ImageContext->ImageError = EFI_ELF_IMAGE_ERROR_RELOCATION_ENTRY_SIZE;
      return EFI_UNSUPPORTED;
    }
    
    RelaBase          = ImageContext->SectionLoadAddress[Index];
    RelaEnd           = (Elf32_Rela *)RelaBase + 
                           RelaHdr->sh_size / sizeof(Elf32_Rela);
    LocationBase      = ImageContext->SectionLoadAddress[RelaHdr->sh_info];

    //
    // Apply Relas
    //
    for (RelaEntry = (Elf32_Rela *)RelaBase;
         RelaEntry < RelaEnd;
         RelaEntry ++) {
      
      //
      // May be no need
      //
      if (ELF32_R_SYM(RelaEntry->r_info) > NumberOfSymbols) {
        ImageContext->ImageError = EFI_ELF_IMAGE_ERROR_SYMBOL_NOT_FOUND;
        return EFI_NOT_FOUND;
      }
      
      SymbolEntry = (Elf32_Sym *)SymbolTableBase + ELF32_R_SYM(RelaEntry->r_info);
      
      //
      // Make sure this module do not inclue extern variable and use ld's -d 
      // option to collect common symbol into .bss section
      //
      if ((SymbolEntry->st_shndx == SHN_UNDEF)||
          (SymbolEntry->st_shndx == SHN_COMMON)) {
        ImageContext->ImageError = EFI_ELF_IMAGE_ERROR_SYMBOL_NOT_FOUND;
        return EFI_NOT_FOUND;
      }

      FromBase = ImageContext->SectionLoadAddress[SymbolEntry->st_shndx];
      Location = (VOID *)(LocationBase + RelaEntry->r_offset);
      Value    = FromBase + SymbolEntry->st_value + RelaEntry->r_addend;

      switch (ELF32_R_TYPE(RelaEntry->r_info)) {
      case R_PPC_ADDR32:                        /* S + A */
        *(UINT32 *)Location = Value;
        break;
        
      case R_PPC_ADDR16_LO:                     /* #lo (S + A) */
        *(UINT16 *)Location = (UINT16)Value;
        break;
        
      case R_PPC_ADDR16_HI:                     /* #hi (S + A) */
        *(UINT16 *)Location = (UINT16)(Value >> 16);
        break;
        
      case R_PPC_ADDR16_HA:
        *(UINT16 *)Location = (UINT16)((Value + 0x8000) >> 16); /* #ha (S + A) */
        break;
        
      case R_PPC_REL24:                               /* (S + A -P) >> 2 */
        //
        // PowerPC Relative jump < +/-32M
        //
        if ((INT32)(Value - (UINT32)Location) < -0x02000000 ||
            (INT32)(Value - (UINT32)Location) >= 0x02000000) {
          ImageContext->ImageError = EFI_ELF_IMAGE_ERROR_UNSUPPORTED_RELA_TYPE;
          return EFI_UNSUPPORTED;
        }
        *(UINT32 *)Location
          = (*(UINT32 *)Location & ~0x03fffffc) |
            ((Value - (UINT32)Location) & 0x03fffffc);
        break;
        
      case R_PPC_REL32:                               /* S + A -P */
        *(UINT32 *)Location = Value - (UINT32)Location;
        break;
        
      default:
        ImageContext->ImageError = EFI_ELF_IMAGE_ERROR_UNSUPPORTED_RELA_TYPE;
        return EFI_UNSUPPORTED;
        break;
      }
    }
  }
  
  return EFI_SUCCESS;
}

