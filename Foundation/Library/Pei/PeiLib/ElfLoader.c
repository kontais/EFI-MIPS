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
#include "Pei.h"
#include "PeiLib.h"

#define  IMAGE_ADD_ELF_HEADER

#ifdef EDK_TOOLS_DEBUG
#include "stdio.h"
#define DEBUG_PRINTF(x) printf x
// DEBUG_PRINTF(());
#else
#define DEBUG_PRINTF(x)
#endif

#include EFI_GUID_DEFINITION (PeiElfLoader)

STATIC
EFI_STATUS
EfiElfLoaderGetElfHeader (
  IN OUT EFI_PEI_ELF_LOADER_IMAGE_CONTEXT       *ImageContext,
  OUT    Elf32_Ehdr                         *Hdr
  );

EFI_STATUS
EFIAPI
EfiElfLoaderGetImageInfo (
  IN     EFI_PEI_ELF_LOADER_PROTOCOL   *This,
  IN OUT EFI_PEI_ELF_LOADER_IMAGE_CONTEXT  *ImageContext
  );

EFI_STATUS
EFIAPI
EfiElfLoaderRelocateImage (
  IN     EFI_PEI_ELF_LOADER_PROTOCOL   *This,
  IN OUT EFI_PEI_ELF_LOADER_IMAGE_CONTEXT  *ImageContext
  );

EFI_STATUS
EFIAPI
EfiElfLoaderLoadImage (
  IN     EFI_PEI_ELF_LOADER_PROTOCOL   *This,
  IN OUT EFI_PEI_ELF_LOADER_IMAGE_CONTEXT  *ImageContext
  );

EFI_STATUS
EFIAPI
EfiElfLoaderUnloadImage (
  IN EFI_PEI_ELF_LOADER_IMAGE_CONTEXT      *ImageContext
  );

STATIC
EFI_STATUS
ElfAllocateSectionBase (
  IN OUT Elf32_Addr     *ImageBase,
  IN OUT Elf32_Addr     *ImageEnd,
  OUT    Elf32_Addr     *SectionBase,
  IN     Elf32_Word     SectionSize,
  IN     Elf32_Word     SectionAlignment
  );

EFI_PEI_ELF_LOADER_PROTOCOL mElfLoader = {
  EfiElfLoaderGetImageInfo,
  EfiElfLoaderLoadImage,
  EfiElfLoaderRelocateImage,
  EfiElfLoaderUnloadImage
};

EFI_STATUS
InstallEfiPeiElfLoader (
  IN EFI_PEI_SERVICES                     **PeiServices,
  IN EFI_PEI_ELF_LOADER_PROTOCOL          **This,
  IN EFI_PEI_PPI_DESCRIPTOR               *ThisPpi
  ) 
/*++

Routine Description:

  Install EFI ELF Loader Protocol.
  
Arguments:

  PeiServices - The PEI core services table.
  
  This        - Pointer to get EFI ELF Loader Protocol as output
  
  ThisPpi     - Passed in as PPI

Returns:

  EFI_SUCCESS

--*/
{
  EFI_STATUS  Status;

  Status = EFI_SUCCESS;

  if (NULL != This) {
    *This = &mElfLoader;
  }

  return Status;
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
  EFI_STATUS            Status;
  UINTN                 Size;

  //
  // Read the ELF executable & relocatable object file Header.
  //
  Size = sizeof (Elf32_Ehdr);
  Status = ImageContext->ImageRead (
                           ImageContext->Handle,
                           0,
                           &Size,
                           Hdr
                           );
  if (EFI_ERROR (Status)) {
    ImageContext->ImageError = EFI_ELF_IMAGE_ERROR_IMAGE_READ;
    return Status;
  }

  //
  // Use Signature to figure out if we understand the image format
  //
  if (Hdr->e_ident[EI_MAG0] != ELFMAG0 ||
      Hdr->e_ident[EI_MAG1] != ELFMAG1 ||
      Hdr->e_ident[EI_MAG2] != ELFMAG2 ||
      Hdr->e_ident[EI_MAG3] != ELFMAG3 ) {
    ImageContext->ImageError = EFI_ELF_IMAGE_ERROR_INVALID_ELF_MAGIC;
    return EFI_UNSUPPORTED;
  }    

  //
  // This module is for ELF32
  //
  if (Hdr->e_ident[EI_CLASS] != ELFCLASS32) {
    ImageContext->ImageError = EFI_ELF_IMAGE_ERROR_UNSUPPORTED_CLASS;
    return EFI_UNSUPPORTED;
  }

  //
  // NOTE: now we just support Relocatable & Executable ELF file.
  //
  if (Hdr->e_type == ET_REL || Hdr->e_type == ET_EXEC) {
    ImageContext->FileType  = Hdr->e_type;
  } else {
    ImageContext->ImageError = EFI_ELF_IMAGE_ERROR_UNSUPPORTED_ELF_FILE_TYPE;
    return EFI_UNSUPPORTED;
  }
  //
  // Get elf header infomation
  //
  if (Hdr->e_machine == EFI_IMAGE_MACHINE_TYPE) {
    ImageContext->Machine                  = Hdr->e_machine;
    ImageContext->ImageType                = *(UINT16*)(&Hdr->e_ident[EI_SUBSYSTEM]);
    ImageContext->FileVersion              = Hdr->e_version;
    ImageContext->FileFlags                = Hdr->e_flags;
    ImageContext->SizeOfElfHeader          = Hdr->e_ehsize;
    ImageContext->ProgramHeaderOffset      = Hdr->e_phoff;
    ImageContext->ProgramHeaderEntrySize   = Hdr->e_phentsize;
    ImageContext->NumberOfProgramHeaders   = Hdr->e_phnum;
    ImageContext->SectionHeaderOffset      = Hdr->e_shoff;
    ImageContext->SectionHeaderEntrySize   = Hdr->e_shentsize;
    ImageContext->NumberOfSections         = Hdr->e_shnum;
    ImageContext->SectionHeaderStringIndex = Hdr->e_shstrndx;
    ImageContext->EntryPoint               = (EFI_PHYSICAL_ADDRESS) Hdr->e_entry;
  } else {
    ImageContext->ImageError = EFI_ELF_IMAGE_ERROR_INVALID_MACHINE_TYPE;
    return EFI_UNSUPPORTED;    
  }

  return EFI_SUCCESS;
}


STATIC
EFI_STATUS
EfiElfLoaderCheckImageType (
  IN OUT EFI_PEI_ELF_LOADER_IMAGE_CONTEXT  *ImageContext
  )
/*++

Routine Description:

  Checks the EFI ELF Image Subsystem Type

Arguments:

  ImageContext  - The context of the image being loaded

Returns:

  EFI_SUCCESS if the EFI ELF image is supported
  EFI_UNSUPPORTED of EFI ELF image is not supported.

--*/
{
  switch (ImageContext->ImageType) {

  case EFI_IMAGE_SUBSYSTEM_EFI_APPLICATION:
    ImageContext->ImageCodeMemoryType = EfiLoaderCode;
    ImageContext->ImageDataMemoryType = EfiLoaderData;
    break;

  case EFI_IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER:
    ImageContext->ImageCodeMemoryType = EfiBootServicesCode;
    ImageContext->ImageDataMemoryType = EfiBootServicesData;
    break;

  case EFI_IMAGE_SUBSYSTEM_EFI_RUNTIME_DRIVER:
  case EFI_IMAGE_SUBSYSTEM_SAL_RUNTIME_DRIVER:
    ImageContext->ImageCodeMemoryType = EfiRuntimeServicesCode;
    ImageContext->ImageDataMemoryType = EfiRuntimeServicesData;
    break;

  default:
    ImageContext->ImageError = EFI_ELF_IMAGE_ERROR_INVALID_SUBSYSTEM;
    return EFI_UNSUPPORTED;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
EfiElfLoaderGetImageInfo (
  IN     EFI_PEI_ELF_LOADER_PROTOCOL   *This,
  IN OUT EFI_PEI_ELF_LOADER_IMAGE_CONTEXT  *ImageContext
  )
/*++

Routine Description:

  Retrieves information on a EFL file image

Arguments:

  This          - Calling context

  ImageContext  - The context of the image being loaded

Returns:

  EFI_SUCCESS if the information on the EFL file image was collected.
  EFI_UNSUPPORTED of the EFL file image is not supported.
  Otherwise, the error status from reading the EFL file image using the 
    ImageContext->ImageRead() function

  EFI_INVALID_PARAMETER   - ImageContext is NULL.

--*/
{
  EFI_STATUS                            Status;
  Elf32_Ehdr                            HdrData;
  Elf32_Ehdr                            *Hdr;
  UINTN                                 Size;
  Elf32_Half                            Index;
  Elf32_Phdr                            ProgramHeader;
  Elf32_Off                             ProgramHeaderOffset;
  Elf32_Shdr                            SectionHeader;
  Elf32_Off                             TempSection;

  if (NULL == ImageContext) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Assume success
  //
  ImageContext->ImageError  = EFI_ELF_IMAGE_ERROR_SUCCESS;

  //
  // Image size & FixupDataSize init to zero
  //
  ImageContext->ImageSize     = 0;
  ImageContext->FixupDataSize = 0;

  Hdr    = &HdrData;
  Status = EfiElfLoaderGetElfHeader (ImageContext, Hdr);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Verify Efi Image type
  //
  Status = EfiElfLoaderCheckImageType (ImageContext);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (ImageContext->FileType == ET_EXEC) {
    ImageContext->IsExecutable = TRUE;
  }
  
  if (ImageContext->FileType == ET_REL) {
    ImageContext->IsRelocatable = TRUE;
  }

  //
  // Retrieve the Program Load Segment Index of the executable image
  //
  if (ImageContext->IsExecutable) {

    if (Hdr->e_phentsize != sizeof(Elf32_Phdr)) {
      ImageContext->ImageError = EFI_ELF_IMAGE_ERROR_PROGRAM_ENTRY_SIZE;
      return EFI_UNSUPPORTED;
    }
    ProgramHeaderOffset = ImageContext->ProgramHeaderOffset;

    for (Index = 0; 
         Index < ImageContext->NumberOfProgramHeaders; 
         Index++) {
      Size   = sizeof(Elf32_Phdr);
      Status = ImageContext->ImageRead (
                               ImageContext->Handle,
                               ProgramHeaderOffset,
                               &Size,
                               &ProgramHeader
                               );
      if (EFI_ERROR (Status)) {
        ImageContext->ImageError = EFI_ELF_IMAGE_ERROR_IMAGE_READ;
        return Status;
      }

      //
      // The Elf excutable file can have many PT_LOADs
      //
      if ((ProgramHeader.p_type == PT_LOAD) && (ProgramHeader.p_flags & PF_X)) {
          ImageContext->LoadSegmentIndex = Index;
        }
      
      ProgramHeaderOffset += sizeof(Elf32_Phdr);
    }
  } 

  //
  // Retrieve infomation of the relocatable image
  //
  if (ImageContext->IsRelocatable) {

    if (ImageContext->NumberOfSections > MAX_SUPPORT_SECTION_NUM) {
      ImageContext->ImageError = EFI_ELF_IMAGE_ERROR_MAX_SECTION_NUMBER;
      return EFI_UNSUPPORTED;
    }

    if (Hdr->e_shentsize != sizeof(Elf32_Shdr)) {
      ImageContext->ImageError = EFI_ELF_IMAGE_ERROR_SECTION_ENTRY_SIZE;
      return EFI_UNSUPPORTED;
    }

    //
    // Get Section Header String Offset
    //
    TempSection = ImageContext->SectionHeaderOffset +
      ImageContext->SectionHeaderStringIndex * sizeof(Elf32_Shdr);
    Size   = sizeof(Elf32_Shdr);
    Status = ImageContext->ImageRead (
                             ImageContext->Handle,
                             TempSection,
                             &Size,
                             &SectionHeader
                             );
    if (EFI_ERROR(Status)) {
      ImageContext->ImageError = EFI_ELF_IMAGE_ERROR_IMAGE_READ;
      return Status;
    }
    
    ImageContext->SectionHeaderStringOffset =
        SectionHeader.sh_offset;

    //
    // Add Each SHF_ALLOC Section Size
    //      
    TempSection = ImageContext->SectionHeaderOffset;

    for (Index = 0; Index < ImageContext->NumberOfSections; Index++) {
      Size   = sizeof(Elf32_Shdr);
      Status = ImageContext->ImageRead (
                                 ImageContext->Handle,
                                 TempSection,
                                 &Size,
                                 &SectionHeader
                                 );
      if (EFI_ERROR(Status)) {
          ImageContext->ImageError = EFI_ELF_IMAGE_ERROR_IMAGE_READ;
        return Status;
      }

      switch ( SectionHeader.sh_type ) {
      case SHT_SYMTAB:
        ImageContext->SymbolTableIndex  = Index;
        ImageContext->SymbolTableOffset = SectionHeader.sh_offset;
        if (SectionHeader.sh_entsize != sizeof(Elf32_Sym)) {
          ImageContext->ImageError = EFI_ELF_IMAGE_ERROR_SYMBOL_ENTRY_SIZE;
          return EFI_UNSUPPORTED;
        }
        ImageContext->NumberOfSymbols   = SectionHeader.sh_size / 
                                            sizeof(Elf32_Sym);
        ImageContext->FixupDataSize     += 
          SectionHeader.sh_size + SectionHeader.sh_addralign * 2;
        break;
        
      case SHT_STRTAB:
        //
        //.dynstr, .shstrtab, .strtab all have SHT_STRTAB type
        //
        if (Index != ImageContext->SectionHeaderStringIndex) {
          ImageContext->StringTableIndex  = Index;
          ImageContext->StringTableOffset = SectionHeader.sh_offset;
        }
        ImageContext->FixupDataSize     += 
          SectionHeader.sh_size + SectionHeader.sh_addralign * 2;
        break;

      case SHT_RELA:
      case SHT_REL:
        ImageContext->FixupDataSize     += 
          SectionHeader.sh_size + SectionHeader.sh_addralign * 2;
        break;
        
      case SHT_PROGBITS:
        //
        // Add each allocate section size
        //
        if ( SectionHeader.sh_flags & SHF_ALLOC ) {
          ImageContext->ImageSize +=
            (UINT64) ( SectionHeader.sh_size + SectionHeader.sh_addralign * 2 );
          //
          //BUGBUG: Assume SHF_ALLOC|SHF_EXECINSTR is .text section
          //Relocatable file have no .fini, .init, .plt section which have 
          //SHF_EXECINSTR Attribute
          //
          if ( SectionHeader.sh_flags & SHF_EXECINSTR ) {
            ImageContext->TextSectionIndex = Index;
          }
        }
        break;

      case SHT_NOBITS:
        //
        // Add .bss section size. Make sure use ld's -r with -d option 
        // to allocate common symbols to .bss section
        //
        if ((SectionHeader.sh_flags & SHF_ALLOC) &&
            (SectionHeader.sh_flags & SHF_WRITE)) {
          ImageContext->ImageSize +=
            (UINT64) ( SectionHeader.sh_size + SectionHeader.sh_addralign * 2 );
        }
        break;

      default:
        //
        // Don' case non SHF_ALLOC section size
        //
        break;
      }

      TempSection += sizeof(Elf32_Shdr);
    }
  }

  //
  // Initialize the alternate destination address to 0 indicating that it
  // should not be used.
  //
  ImageContext->DestinationAddress = 0;

  //
  // Add Ehdr & Phdr for Loaded Image.
  //
  ImageContext->ImageSize += sizeof(Elf32_Ehdr) + sizeof(Elf32_Phdr);

  //
  // runtime drivers not use Handle as pointer to ELF header, so need to read 
  // rela/rel/symtab/strtab section as FixupData in memory
  //
  ImageContext->FixupData = NULL;
  
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
EfiElfLoaderLoadImage (
  IN     EFI_PEI_ELF_LOADER_PROTOCOL   *This,
  IN OUT EFI_PEI_ELF_LOADER_IMAGE_CONTEXT  *ImageContext
  )
/*++

Routine Description:

  Loads a EFL file image into memory

Arguments:

  This         - Calling context

  ImageContext - Contains information on image to load into memory

Returns:

  EFI_SUCCESS            if the EFL file image was loaded
  EFI_BUFFER_TOO_SMALL   if the caller did not provide a large enough buffer
  EFI_LOAD_ERROR         if the image is a runtime driver with no relocations
  EFI_INVALID_PARAMETER  if the image address is invalid

--*/
{
  EFI_STATUS                            Status;
  EFI_PEI_ELF_LOADER_IMAGE_CONTEXT      CheckContext;
  UINTN                                 Size;
  Elf32_Word                            MemorySize;
  Elf32_Half                            Index;
  Elf32_Addr                            Base;
  Elf32_Addr                            End;
  Elf32_Addr                            MaxEnd;
  Elf32_Word                            Adjust;
  Elf32_Addr                            TempBase;
  Elf32_Addr                            FixupBase;
  Elf32_Addr                            FixupEnd;
  Elf32_Addr                            FixupMaxEnd;
  Elf32_Off                             FileOffset;
  Elf32_Phdr                            ProgramHeader;
  Elf32_Off                             LoadProgramHeaderOffset;
  Elf32_Shdr                            SectionHeader;
  Elf32_Off                             TempSection;
#ifdef IMAGE_ADD_ELF_HEADER
  Elf32_Ehdr                            *pEhdr;
#endif

  if (NULL == ImageContext) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Assume success
  //
  ImageContext->ImageError = EFI_ELF_IMAGE_ERROR_SUCCESS;

  //
  // Copy the provided context info into our local version, get what we
  // can from the original image, and then use that to make sure everything
  // is legit.
  //
  CopyMem(&CheckContext,ImageContext,sizeof(EFI_PEI_ELF_LOADER_IMAGE_CONTEXT));

  Status = EfiElfLoaderGetImageInfo (This, &CheckContext);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Make sure there is enough allocated space for the image being loaded
  //
  if (ImageContext->ImageSize < CheckContext.ImageSize) {
    ImageContext->ImageError = EFI_ELF_IMAGE_ERROR_INVALID_IMAGE_SIZE;
    return EFI_BUFFER_TOO_SMALL;
  }

  //
  // Loading executable image file
  //
  if (ImageContext->IsExecutable) {
    LoadProgramHeaderOffset = ImageContext->ProgramHeaderOffset
      + ImageContext->LoadSegmentIndex * sizeof(Elf32_Phdr);

    Size   = sizeof(Elf32_Phdr);
    Status = ImageContext->ImageRead (
                             ImageContext->Handle,
                             LoadProgramHeaderOffset,
                             &Size,
                             &ProgramHeader
                             );
    if (EFI_ERROR (Status)) {
      ImageContext->ImageError = EFI_ELF_IMAGE_ERROR_IMAGE_READ;
      return Status;
    }

    Size       = ProgramHeader.p_filesz;
    MemorySize = ProgramHeader.p_memsz;
    Base       = ProgramHeader.p_vaddr;
    FileOffset = ProgramHeader.p_offset;

    End        = Base + Size;
    MaxEnd     = Base + MemorySize;
    
    if (End > MaxEnd) {
      MaxEnd = End;
    }

    if (Size > MemorySize) {
      MemorySize = Size;
    }
    //
    // If the base start or end address resolved to 0, then fail.
    //
    if ((Base == 0) || (End == 0)) {
      ImageContext->ImageError = EFI_ELF_IMAGE_ERROR_PROGRAM_NOT_LOADED;
      return EFI_LOAD_ERROR;
    }

    Status = ImageContext->ImageRead (
                             ImageContext->Handle,
                             FileOffset,
                             &Size,
                             (VOID *)(UINTN)Base
                             );
    if (EFI_ERROR (Status)) {
      ImageContext->ImageError = EFI_ELF_IMAGE_ERROR_IMAGE_READ;
      return Status;
    }

    //
    // Fill .bss memory space with zero
    //
    if (MemorySize > Size) {
      ZeroMem ((VOID *)((UINTN)Base + Size), MemorySize - Size);
    }

    ImageContext->ImageAddress = (EFI_PHYSICAL_ADDRESS) Base;
    ImageContext->ImageSize    = (UINT64) MemorySize;

  }

  //
  // Loading relocatable sections
  //
  if (ImageContext->IsRelocatable) {

    //
    // If the destination address is not 0, use that rather than the
    // image address as the relocation target.
    //
    if (ImageContext->DestinationAddress != 0) {
      Base    = (UINTN) ImageContext->DestinationAddress;
      End     = (UINTN) ImageContext->DestinationAddress;
      MaxEnd  = (UINTN) (ImageContext->DestinationAddress + ImageContext->ImageSize);
      Adjust  = (UINTN) ImageContext->ImageAddress - Base;
    } else {
      Base    = (UINTN) ImageContext->ImageAddress;
      End     = (UINTN) ImageContext->ImageAddress;
      MaxEnd  = (UINTN) (ImageContext->ImageAddress + ImageContext->ImageSize);
      Adjust  = 0;
    }
	
#ifdef IMAGE_ADD_ELF_HEADER
    //
    // Add Elf32_Ehdr
    //
    Status = ElfAllocateSectionBase(
               &Base,
               &End,
               &TempBase,
               sizeof(Elf32_Ehdr),
               1
               );
    if (EFI_ERROR(Status)) {
      ImageContext->ImageError = EFI_ELF_IMAGE_ERROR_SECTION_NOT_LOADED;
      return Status;
    }

    //
    // Read the ELF executable & relocatable object file Header.
    //
    Size = sizeof (Elf32_Ehdr);
    Status = ImageContext->ImageRead (
                             ImageContext->Handle,
                             0,
                             &Size,
                             (VOID*)(UINTN)(TempBase + Adjust)
                             );
    if (EFI_ERROR (Status)) {
      ImageContext->ImageError = EFI_ELF_IMAGE_ERROR_IMAGE_READ;
      return Status;
    }

    pEhdr  =  (Elf32_Ehdr*)(UINTN)(TempBase + Adjust);
	
#endif // IMAGE_ADD_ELF_HEADER

    //
    //  Fixup for DXE Image load
    //
    FixupBase    = (UINTN) ImageContext->FixupData;
    FixupEnd     = (UINTN) ImageContext->FixupData;
    FixupMaxEnd  = (UINTN) ImageContext->FixupData + 
                           ImageContext->FixupDataSize;
    
    TempSection = ImageContext->SectionHeaderOffset;

    for (Index = 0; Index < ImageContext->NumberOfSections; Index++) {
      Size   = sizeof(Elf32_Shdr);
      Status = ImageContext->ImageRead (
                                 ImageContext->Handle,
                                 TempSection,
                                 &Size,
                                 &SectionHeader
                                 );
      if (EFI_ERROR(Status)) {
          ImageContext->ImageError = EFI_ELF_IMAGE_ERROR_IMAGE_READ;
        return Status;
      }

      switch ( SectionHeader.sh_type ) {
      case SHT_SYMTAB:
      case SHT_STRTAB:
      case SHT_RELA:
      case SHT_REL:
        if (ImageContext->FixupData == NULL) {
          ImageContext->SectionLoadAddress[Index] = 
            SectionHeader.sh_offset + (Elf32_Addr)ImageContext->Handle;
        } else {
          Status = ElfAllocateSectionBase(
                     &FixupBase,
                     &FixupEnd,
                     &ImageContext->SectionLoadAddress[Index],
                     SectionHeader.sh_size,
                     SectionHeader.sh_addralign
                     );
          if (EFI_ERROR(Status)) {
            ImageContext->ImageError = EFI_ELF_IMAGE_ERROR_SECTION_NOT_LOADED;
            return Status;
          }
          
          //
          //Load section data
          //
          Size   = SectionHeader.sh_size;
          Status = ImageContext->ImageRead (
                     ImageContext->Handle,
                     SectionHeader.sh_offset,
                     &Size,
                     (VOID *)(UINTN)ImageContext->SectionLoadAddress[Index]
                     );
          if (EFI_ERROR (Status)) {
            ImageContext->ImageError = EFI_ELF_IMAGE_ERROR_IMAGE_READ;
            return Status;
          }
        }
        break;
        
      case SHT_PROGBITS:
        if ( SectionHeader.sh_flags & SHF_ALLOC ) {
          Status = ElfAllocateSectionBase(
                     &Base,
                     &End,
                     &TempBase,
                     SectionHeader.sh_size,
                     SectionHeader.sh_addralign
                     );
          if (EFI_ERROR(Status)) {
            ImageContext->ImageError = EFI_ELF_IMAGE_ERROR_SECTION_NOT_LOADED;
            return Status;
          }

          //
          // Adjust to ImageAddress
          //
          ImageContext->SectionLoadAddress[Index] 
                                        = TempBase + Adjust;
          //
          //Load section data
          //
          Size   = SectionHeader.sh_size;
          Status = ImageContext->ImageRead (
                     ImageContext->Handle,
                     SectionHeader.sh_offset,
                     &Size,
                     (VOID *)(UINTN)ImageContext->SectionLoadAddress[Index]
                     );
          if (EFI_ERROR (Status)) {
            ImageContext->ImageError = EFI_ELF_IMAGE_ERROR_IMAGE_READ;
            return Status;
          }
        }
        break;

      case SHT_NOBITS:
        if ((SectionHeader.sh_flags & SHF_ALLOC) &&
            (SectionHeader.sh_flags & SHF_WRITE)) {
          Status = ElfAllocateSectionBase(
                     &Base,
                     &End,
                     &TempBase,
                     SectionHeader.sh_size,
                     SectionHeader.sh_addralign
                     );
          if (EFI_ERROR(Status)) {
            ImageContext->ImageError = EFI_ELF_IMAGE_ERROR_SECTION_NOT_LOADED;
            return Status;
          }

          //
          // Adjust to ImageAddress
          //
          ImageContext->SectionLoadAddress[Index] 
                                     = TempBase + Adjust;
          
          //
          // Fill .bss section to zero
          //
          ZeroMem ((VOID *)(UINTN)(ImageContext->SectionLoadAddress[Index]), 
            SectionHeader.sh_size);
        }
        break;

      default:
        break;
      }

      TempSection += sizeof(Elf32_Shdr);
    }

    //
    // After all section loaded, check the size
    //
    if (End > MaxEnd) {
      ImageContext->ImageError = EFI_ELF_IMAGE_ERROR_SECTION_NOT_LOADED;
      return EFI_BUFFER_TOO_SMALL;
    }
    //
    // Check FixupData size
    //
    if (ImageContext->FixupData != NULL && FixupEnd > FixupMaxEnd) {
      ImageContext->ImageError = EFI_ELF_IMAGE_ERROR_SECTION_NOT_LOADED;
      return EFI_BUFFER_TOO_SMALL;
    }
    ImageContext->ImageAddress = (EFI_PHYSICAL_ADDRESS) (Base + Adjust);
    ImageContext->ImageSize    = (UINT64) (End - Base);
  }

  if (ImageContext->ImageAddress == 0) {
    //
    // Image cannot be loaded into 0 address.
    //
    ImageContext->ImageError = EFI_ELF_IMAGE_ERROR_INVALID_IMAGE_ADDRESS;
    return EFI_INVALID_PARAMETER;
  }

  //
  // Get image's entry point
  //
  if (ImageContext->IsRelocatable) {
    Index = ImageContext->TextSectionIndex;
    ImageContext->EntryPoint += 
		(EFI_PHYSICAL_ADDRESS) (ImageContext->SectionLoadAddress[Index] - Adjust);
#ifdef IMAGE_ADD_ELF_HEADER
    pEhdr->e_entry = (UINT32) ImageContext->EntryPoint;
    pEhdr->e_type  = ET_NONE; 
#endif //IMAGE_ADD_ELF_HEADER
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
EfiElfLoaderUnloadImage (
  IN EFI_PEI_ELF_LOADER_IMAGE_CONTEXT   *ImageContext
  )
/*++

Routine Description:

  Unload a EFL file image from memory

Arguments:

  ImageBase   -  只是在第一次根据第一个Section的对齐变化一次

Returns:

  EFI_SUCCESS            

--*/
{
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
ElfAllocateSectionBase (
  IN OUT Elf32_Addr     *ImageBase,
  IN OUT Elf32_Addr     *ImageEnd,
  OUT    Elf32_Addr     *SectionBase,
  IN     Elf32_Word     SectionSize,
  IN     Elf32_Word     SectionAlignment
  )
{
  Elf32_Addr NewSectionBase;
  Elf32_Addr NewSectionEnd;
  
  if (SectionSize == 0) {
    return EFI_SUCCESS;
  }
  
  NewSectionBase = *ImageEnd;
  
  if ((NewSectionBase & (SectionAlignment -1)) != 0) {
    NewSectionBase += SectionAlignment;
    NewSectionBase &= ~(SectionAlignment - 1);
  }

  NewSectionEnd  = NewSectionBase + SectionSize;

  if (*ImageBase != *ImageEnd) {
    *SectionBase = NewSectionBase;
    *ImageEnd    = NewSectionEnd;
  } else {
    *ImageBase   = NewSectionBase;   // First Allocate section
    *SectionBase = NewSectionBase;
    *ImageEnd    = NewSectionEnd;
  }

  return EFI_SUCCESS;
}

