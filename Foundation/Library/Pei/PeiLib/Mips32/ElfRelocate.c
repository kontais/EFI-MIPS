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

  Relocate MIPS32 Relocation Section.

Revision History

--*/

#include "Tiano.h"

#include EFI_GUID_DEFINITION (PeiElfLoader)

#ifdef EDK_TOOLS_DEBUG
#include "stdio.h"
#define DEBUG_PRINTF(x) printf x
// DEBUG_PRINTF(());
#else
#define DEBUG_PRINTF(x)
#endif

typedef
struct _MIPS_HI16_REL {
  UINT32       Relocated;
  UINT32       *Location;
  UINTN        Value;
} MIPS_HI16_REL;

EFI_STATUS
EFIAPI
ElfRelocateRela (
  IN Elf32_Half                        Index,
  IN Elf32_Shdr                        *pSectionHeader,
  IN OUT EFI_PEI_ELF_LOADER_IMAGE_CONTEXT  *ImageContext
  );

EFI_STATUS
EFIAPI
ElfRelocateRel (
  IN Elf32_Half                        Index,
  IN Elf32_Shdr                        *pSectionHeader,
  IN OUT EFI_PEI_ELF_LOADER_IMAGE_CONTEXT  *ImageContext
  );

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
  EFI_UNSUPPORTED  not support

--*/
{
  EFI_STATUS             Status;
  Elf32_Shdr             SectionHeader;
  Elf32_Off              SectionHeaderOffset;
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
    //
    //
    switch (SectionHeader.sh_type) {
    case SHT_RELA:
      Status = ElfRelocateRela(Index, &SectionHeader, ImageContext);
      if ( EFI_ERROR(Status)) {
       return Status;
      }
      break;
    case SHT_REL:
      Status = ElfRelocateRel(Index, &SectionHeader, ImageContext);
      if ( EFI_ERROR(Status)) {
        return Status;
      }
      break;
    default:
      continue;
    }
  }
  
  return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
ElfRelocateRela (
  IN Elf32_Half                        Index,
  IN Elf32_Shdr                        *pSectionHeader,
  IN OUT EFI_PEI_ELF_LOADER_IMAGE_CONTEXT  *ImageContext
  )
/*++

Routine Description:

  Relocates a EFL file image in memory

Arguments:

  Index          - Rela section index

  pSectionHeader - Pointer to Rela section header

  ImageContext  - Contains information on the loaded image to relocate

Returns:

  EFI_SUCCESS      if the EFL file image was relocated
  EFI_NOT_FOUND    Symbol not found
  EFI_UNSUPPORTED  not support

--*/
{
  Elf32_Shdr             *RelaHdr;
  Elf32_Rela             *RelaEntry;
  Elf32_Addr             RelaBase;
  Elf32_Rela             *RelaEnd;
  Elf32_Half             SymTabIndex;
  Elf32_Half             StrTabIndex;
  Elf32_Sym              *SymbolEntry;
  Elf32_Addr             SymbolTableBase;
  Elf32_Addr             StringTableBase;
  Elf32_Word             NumberOfSymbols;
  Elf32_Addr             FromBase;
  Elf32_Addr             LocationBase;
  UINT32                 *Location;
  UINTN                  Value;
  Elf32_Half             RelaIndex;

  if (NULL == ImageContext) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Assume success
  //
  ImageContext->ImageError = EFI_ELF_IMAGE_ERROR_SUCCESS;

  SymTabIndex     = ImageContext->SymbolTableIndex;
  StrTabIndex     = ImageContext->StringTableIndex;
  NumberOfSymbols = ImageContext->NumberOfSymbols;
  SymbolTableBase = ImageContext->SectionLoadAddress[SymTabIndex];
  StringTableBase = ImageContext->SectionLoadAddress[StrTabIndex];

  RelaHdr   = pSectionHeader;
  RelaIndex = Index;
  if (RelaHdr->sh_entsize != sizeof(Elf32_Rela)) {
    ImageContext->ImageError = EFI_ELF_IMAGE_ERROR_RELOCATION_ENTRY_SIZE;
    return EFI_UNSUPPORTED;
  }
  
  RelaBase          = ImageContext->SectionLoadAddress[RelaIndex];
  RelaEnd           = (Elf32_Rela *)RelaBase + 
                         RelaHdr->sh_size / sizeof(Elf32_Rela);
  LocationBase      = ImageContext->SectionLoadAddress[RelaHdr->sh_info];

  //
  // Rel section for not SHF_ALLOC section, e.g .pdr.rel for .pdr in MIPS
  //
  if ( LocationBase == 0) {
    return EFI_SUCCESS;
  }

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
    case R_MIPS_NONE:
      ;
      break;
      
    case R_MIPS_32:
      *Location = Value;
      break;
      
    case R_MIPS_26:
      //
      //  26 = 28 - 2 , 4 byte aligned
      //
      if (Value % 4) {
        ImageContext->ImageError = EFI_ELF_IMAGE_ERROR_INCORRECT_JUMP_ADDRESS;
        return EFI_UNSUPPORTED;
      }
      //
      //  Jump > 256MB
      //
      if ((Value & 0xf0000000) != (((UINT32)Location + 4) & 0xf0000000)) {
        ImageContext->ImageError = EFI_ELF_IMAGE_ERROR_INCORRECT_JUMP_ADDRESS;
        return EFI_UNSUPPORTED;
      }
      *Location = (*Location & ~0x03ffffff) | ((Value >> 2) & 0x03ffffff);
      break;
      
    case R_MIPS_HI16:
      *Location = (*Location & 0xffff0000) | 
                  ((((long long) Value + 0x8000LL) >> 16) & 0xffff);
      break;
      
    case R_MIPS_LO16:
      *Location = (*Location & 0xffff0000) | (Value & 0xffff);
      break;
      
    case R_MIPS_64:
      *(UINT64 *)Location = Value;
      break;
      
    case R_MIPS_HIGHER:
      *Location = (*Location & 0xffff0000) |
                  ((((long long) Value + 0x80008000LL) >> 32) & 0xffff);
      break;
      
    case R_MIPS_HIGHEST:
      *Location = (*Location & 0xffff0000) |
                  ((((long long) Value + 0x800080008000LL) >> 48) & 0xffff);
      break;
      
    default:
      ImageContext->ImageError = EFI_ELF_IMAGE_ERROR_UNSUPPORTED_RELA_TYPE;
      return EFI_UNSUPPORTED;
      break;
    }
  }
  
  return EFI_SUCCESS;
}

//
//  最多连续16个R_MIPS_HI16
//
#define CONTINUOUS_MIPS_HI16_REL_MAX    16

EFI_STATUS
EFIAPI
ElfRelocateRel (
  IN Elf32_Half                        Index,
  IN Elf32_Shdr                        *pSectionHeader,
  IN OUT EFI_PEI_ELF_LOADER_IMAGE_CONTEXT  *ImageContext
  )
/*++

Routine Description:

  Relocates a EFL file image in memory

Arguments:

  This         - Calling context

  ImageContext - Contains information on the loaded image to relocate

Returns:

  EFI_SUCCESS      if the EFL file image was relocated
  EFI_NOT_FOUND    Symbol not found
  EFI_UNSUPPORTED  not support

--*/
{
  Elf32_Shdr             *RelHdr;
  Elf32_Rel              *RelEntry;
  Elf32_Addr             RelBase;
  Elf32_Rel              *RelEnd;
  Elf32_Half             SymTabIndex;
  Elf32_Half             StrTabIndex;
  Elf32_Sym              *SymbolEntry;
  Elf32_Addr             SymbolTableBase;
  Elf32_Addr             StringTableBase;
  Elf32_Word             NumberOfSymbols;
  Elf32_Addr             FromBase;
  Elf32_Addr             LocationBase;
  UINT32                 *Location;
  UINTN                  Value;
  UINTN                  Adjust;
  Elf32_Half             RelIndex;
  MIPS_HI16_REL          HI16_REL_ENTRY[CONTINUOUS_MIPS_HI16_REL_MAX];
  UINT32                 Hi16RelLevel;
  MIPS_HI16_REL          *pLastHi16;

  if (NULL == ImageContext) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Assume success
  //
  ImageContext->ImageError = EFI_ELF_IMAGE_ERROR_SUCCESS;

  SymTabIndex     = ImageContext->SymbolTableIndex;
  StrTabIndex     = ImageContext->StringTableIndex;
  NumberOfSymbols = ImageContext->NumberOfSymbols;
  SymbolTableBase = ImageContext->SectionLoadAddress[SymTabIndex];
  StringTableBase = ImageContext->SectionLoadAddress[StrTabIndex];

  RelHdr   = pSectionHeader;
  RelIndex = Index;
  if (RelHdr->sh_entsize != sizeof(Elf32_Rel)) {
    ImageContext->ImageError = EFI_ELF_IMAGE_ERROR_RELOCATION_ENTRY_SIZE;
    return EFI_UNSUPPORTED;
  }
  
  RelBase          = ImageContext->SectionLoadAddress[RelIndex];
  RelEnd           = (Elf32_Rel *)RelBase + 
                         RelHdr->sh_size / sizeof(Elf32_Rel);
  LocationBase      = ImageContext->SectionLoadAddress[RelHdr->sh_info];

  //
  // Rel section for not SHF_ALLOC section, e.g .pdr.rel for .pdr in MIPS
  //
  if ( LocationBase == 0) {
    return EFI_SUCCESS;
  }

  //
  // If the destination address is not 0, use that rather than the
  // image address as the relocation target.
  //
  if (ImageContext->DestinationAddress != 0) {
    Adjust  = (UINTN) (ImageContext->DestinationAddress - ImageContext->ImageAddress);
  } else {
    Adjust  = 0;
  }
  
  //
  // Init R_MIPS_HI16 Count
  //
  Hi16RelLevel = 0;

  //
  // Apply Rels
  //
  for (RelEntry = (Elf32_Rel *)RelBase;
       RelEntry < RelEnd;
       RelEntry ++) {
    
    //
    // May be no need
    //
    if (ELF32_R_SYM(RelEntry->r_info) > NumberOfSymbols) {
      ImageContext->ImageError = EFI_ELF_IMAGE_ERROR_SYMBOL_NOT_FOUND;
      return EFI_NOT_FOUND;
    }
    
    SymbolEntry = (Elf32_Sym *)SymbolTableBase + ELF32_R_SYM(RelEntry->r_info);
    
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
    Location = (VOID *)(LocationBase + RelEntry->r_offset);
    Value    = FromBase + SymbolEntry->st_value;

    switch (ELF32_R_TYPE(RelEntry->r_info)) {
    case R_MIPS_NONE:
      ;
      break;

    case R_MIPS_32:
      *(UINT32 *)Location += (Value + Adjust);
      break;
      
    case R_MIPS_26:
      //
      //  26 = 28 - 2 , 4 byte aligned
      //
      if ((Value + Adjust) % 4) {
        ImageContext->ImageError = EFI_ELF_IMAGE_ERROR_INCORRECT_JUMP_ADDRESS;
        return EFI_UNSUPPORTED;
      }
      //
      //  Jump > 256MB
      //
      if (((Value + Adjust) & 0xf0000000) != (((UINT32)Location +Adjust + 4) & 0xf0000000)) {
        ImageContext->ImageError = EFI_ELF_IMAGE_ERROR_INCORRECT_JUMP_ADDRESS;
        return EFI_UNSUPPORTED;
      }
      *Location = (*Location & ~0x03ffffff) | 
                  ((*Location + ((Value + Adjust) >> 2)) & 0x03ffffff);
      break;

    case R_MIPS_HI16:
      //
      // Record the R_MIPS_HI16 entry, do actual relcate in R_MIPS_LO16
      //
      HI16_REL_ENTRY[Hi16RelLevel].Relocated = 0;
      HI16_REL_ENTRY[Hi16RelLevel].Location  = Location;
      HI16_REL_ENTRY[Hi16RelLevel].Value     = Value;
      Hi16RelLevel ++ ;
      if (Hi16RelLevel > CONTINUOUS_MIPS_HI16_REL_MAX) {
        ImageContext->ImageError = EFI_ELF_IMAGE_ERROR_INCORRECT_R_MIPS16_PAIR;
        return EFI_UNSUPPORTED;
      }
      break;
      
    case R_MIPS_LO16:
      {
        UINT32 insnlo;
        UINT32 val;
        UINT32 vallo;
        UINT32 insn;

        insnlo = *Location;
        vallo = ((insnlo & 0xffff) ^ 0x8000) - 0x8000;

        if (Hi16RelLevel > 0) {
          Hi16RelLevel --;
        }

        pLastHi16 = &HI16_REL_ENTRY[Hi16RelLevel];

        if (pLastHi16->Relocated == 0)
        {
          if (Value != pLastHi16->Value) {
            ImageContext->ImageError = EFI_ELF_IMAGE_ERROR_INCORRECT_R_MIPS16_PAIR;
            return EFI_UNSUPPORTED;
          }
          //
          // Do HI16 relocate
          //
          insn =  *(pLastHi16->Location);
          val  =  ((insn & 0xffff) << 16) + vallo;
          val  += (Value + Adjust);

          val  =  ((val >> 16) + ((val & 0x8000) != 0)) & 0xffff;

          insn =  (insn & ~0xffff) | val;
          *(pLastHi16->Location) = insn;

          pLastHi16->Relocated = 1;
        }

        //
        // deal with LO16
        //
        val  = (Value + Adjust) + vallo;
        insnlo = (insnlo & ~0xffff) | (val & 0xffff);
        *Location = insnlo;

        break;
      }
    case R_MIPS_PC16:
      {
        INT32  rel;

        rel = (UINT32)Value - (UINT32)Location;
        rel >>= 2;
        rel -= 1;

        if( (rel > 32768) || (rel < -32768) ) {
          return EFI_UNSUPPORTED;
        }

        *Location = (*Location & 0xffff0000) | (rel & 0xffff);
        
        break;
      }
      
    default:
      ImageContext->ImageError = EFI_ELF_IMAGE_ERROR_UNSUPPORTED_REL_TYPE;
      return EFI_UNSUPPORTED;
      break;
    }
  }

  return EFI_SUCCESS;
}

