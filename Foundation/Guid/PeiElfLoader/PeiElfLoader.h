/*++

Copyright (c) 2009, kontais                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:
  
    EfiElfLoader.h
    
Abstract:

  GUID for the ELF executable & relocatable object file Loader APIs shared 
  between PEI and DXE

--*/

#ifndef _EFI_ELF_LOADER_H
#define _EFI_ELF_LOADER_H

#include "EfiImage.h"


#define EFI_PEI_ELF_LOADER_GUID  \
  { 0xd8117cff, 0x94a6, 0x11d4, {0x9a, 0x3a, 0x0, 0x90, 0x27, 0x3f, 0xc1, 0x4d} }

EFI_FORWARD_DECLARATION (EFI_PEI_ELF_LOADER_PROTOCOL);

#define EFI_ELF_IMAGE_ERROR_SUCCESS                      0
#define EFI_ELF_IMAGE_ERROR_IMAGE_READ                   1  
#define EFI_ELF_IMAGE_ERROR_INVALID_ELF_MAGIC            2
#define EFI_ELF_IMAGE_ERROR_INVALID_MACHINE_TYPE         3
#define EFI_ELF_IMAGE_ERROR_INVALID_SUBSYSTEM            4
#define EFI_ELF_IMAGE_ERROR_INVALID_IMAGE_ADDRESS        5
#define EFI_ELF_IMAGE_ERROR_INVALID_IMAGE_SIZE           6
#define EFI_ELF_IMAGE_ERROR_INVALID_SECTION_ALIGNMENT    7
#define EFI_ELF_IMAGE_ERROR_SECTION_NOT_LOADED           8
#define EFI_ELF_IMAGE_ERROR_FAILED_RELOCATION            9
#define EFI_ELF_IMAGE_ERROR_FAILED_ICACHE_FLUSH          10
#define EFI_ELF_IMAGE_ERROR_UNSUPPORTED_ELF_FILE_TYPE    11
#define EFI_ELF_IMAGE_ERROR_PROGRAM_ENTRY_SIZE           12
#define EFI_ELF_IMAGE_ERROR_SECTION_ENTRY_SIZE           13
#define EFI_ELF_IMAGE_ERROR_SYMBOL_ENTRY_SIZE            14
#define EFI_ELF_IMAGE_ERROR_RELOCATION_ENTRY_SIZE        15
#define EFI_ELF_IMAGE_ERROR_MAX_SECTION_NUMBER           16
#define EFI_ELF_IMAGE_ERROR_PROGRAM_NOT_LOADED           17
#define EFI_ELF_IMAGE_ERROR_SYMBOL_NOT_FOUND             18
#define EFI_ELF_IMAGE_ERROR_UNSUPPORTED_RELA_TYPE        19
#define EFI_ELF_IMAGE_ERROR_UNSUPPORTED_REL_TYPE         20
#define EFI_ELF_IMAGE_ERROR_UNSUPPORTED_CLASS            21
#define EFI_ELF_IMAGE_ERROR_INCORRECT_JUMP_ADDRESS       22
#define EFI_ELF_IMAGE_ERROR_INCORRECT_R_MIPS16_PAIR      23

typedef
EFI_STATUS
(EFIAPI *EFI_PEI_ELF_LOADER_READ_FILE) (
  IN     VOID   *FileHandle,
  IN     UINTN  FileOffset,
  IN OUT UINTN  *ReadSize,
  OUT    VOID   *Buffer
  );

#define MAX_SUPPORT_SECTION_NUM    30

typedef struct {
  EFI_PHYSICAL_ADDRESS        ImageAddress;
  UINT64                      ImageSize;
  EFI_PHYSICAL_ADDRESS        DestinationAddress;
  EFI_PHYSICAL_ADDRESS        EntryPoint;
  EFI_PEI_ELF_LOADER_READ_FILE    ImageRead;
  VOID                        *Handle;
  VOID                        *FixupData;
  UINTN                       FixupDataSize;
  BOOLEAN                     IsExecutable;
  BOOLEAN                     IsRelocatable;
  Elf32_Half                  FileType;
  UINT16                      ImageType;
  Elf32_Half                  TextSectionIndex;
  Elf32_Half                  LoadSegmentIndex;
  Elf32_Half                  Machine;
  Elf32_Word                  FileVersion;
  Elf32_Word                  FileFlags;
  Elf32_Off                   ProgramHeaderOffset;
  Elf32_Half                  ProgramHeaderEntrySize;
  Elf32_Half                  NumberOfProgramHeaders;
  Elf32_Off                   SectionHeaderOffset;
  Elf32_Half                  SizeOfElfHeader;
  Elf32_Half                  SectionHeaderEntrySize;
  Elf32_Half                  NumberOfSections;
  Elf32_Half                  SectionHeaderStringIndex;
  Elf32_Addr                  SectionHeaderStringOffset;
  Elf32_Half                  StringTableIndex;
  Elf32_Half                  SymbolTableIndex;
  Elf32_Addr                  StringTableOffset;
  Elf32_Addr                  SymbolTableOffset;
  Elf32_Word                  NumberOfSymbols;
  Elf32_Addr                  SectionLoadAddress[MAX_SUPPORT_SECTION_NUM];
  UINT32                      ImageCodeMemoryType;
  UINT32                      ImageDataMemoryType;
  UINT32                      ImageError;
} EFI_PEI_ELF_LOADER_IMAGE_CONTEXT;

typedef 
EFI_STATUS
(EFIAPI *EFI_ELF_LOADER_GET_IMAGE_INFO) (
  IN     EFI_PEI_ELF_LOADER_PROTOCOL  *This,
  IN OUT EFI_PEI_ELF_LOADER_IMAGE_CONTEXT *ImageContext
  );

typedef 
EFI_STATUS
(EFIAPI *EFI_ELF_LOADER_LOAD_IMAGE) (
  IN     EFI_PEI_ELF_LOADER_PROTOCOL  *This,
  IN OUT EFI_PEI_ELF_LOADER_IMAGE_CONTEXT *ImageContext
  );

typedef 
EFI_STATUS
(EFIAPI *EFI_ELF_LOADER_RELOCATE_IMAGE) (
  IN     EFI_PEI_ELF_LOADER_PROTOCOL  *This,
  IN OUT EFI_PEI_ELF_LOADER_IMAGE_CONTEXT *ImageContext
  );

typedef 
EFI_STATUS
(EFIAPI *EFI_ELF_LOADER_UNLOAD_IMAGE) (
  IN EFI_PEI_ELF_LOADER_IMAGE_CONTEXT *ImageContext
  );

struct _EFI_PEI_ELF_LOADER_PROTOCOL {
  EFI_ELF_LOADER_GET_IMAGE_INFO  GetImageInfo;
  EFI_ELF_LOADER_LOAD_IMAGE      LoadImage;
  EFI_ELF_LOADER_RELOCATE_IMAGE  RelocateImage;
  EFI_ELF_LOADER_UNLOAD_IMAGE    UnloadImage;
};

extern EFI_GUID gEfiPeiElfLoaderGuid;

#endif /* _EFI_ELF_LOADER_H */
