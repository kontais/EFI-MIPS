/*++

Copyright (c) 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  DxeIpl.h

Abstract:

--*/

#ifndef _PEI_DXEIPL_H_
#define _PEI_DXEIPL_H_

#include "PeiHob.h"
#include "PeiLib.h"
#include "EfiFirmwareVolumeHeader.h"
#include "EfiFirmwareFileSystem.h"
#include "Pei.h"

//
// Imported Interfaces
//
#include EFI_GUID_DEFINITION (PeiTransferControl)
#include EFI_GUID_DEFINITION (FirmwareFileSystem)
#include EFI_PPI_DEFINITION (RecoveryModule)
#include EFI_PPI_DEFINITION (S3Resume)
#include EFI_PPI_DEFINITION (SectionExtraction)
#include EFI_PPI_DEFINITION (Security)
#include EFI_PPI_DEFINITION (PeiInMemory)
#include EFI_PPI_DEFINITION (LoadFile)

//
// Exported Interfaces
//
#include EFI_PPI_DEFINITION (DxeIpl)
#include EFI_PPI_DEFINITION (EndOfPeiSignal)
#include EFI_PROTOCOL_DEFINITION (Decompress)
#include EFI_PROTOCOL_DEFINITION (TianoDecompress)
#include EFI_PROTOCOL_DEFINITION (CustomizedDecompress)
#include EFI_GUID_DEFINITION (PeiFlushInstructionCache)
#include EFI_GUID_DEFINITION (PeiElfLoader)

#include "ImageRead.h"
#include "PeiHobLib.h"

#define PEI_DXEIPL_STACK_SIZE 0x10000

EFI_STATUS
InstallEfiDecompress (
  EFI_DECOMPRESS_PROTOCOL  **This
  )
;

EFI_STATUS
InstallTianoDecompress (
  EFI_TIANO_DECOMPRESS_PROTOCOL  **This
  )
;

EFI_STATUS
InstallCustomizedDecompress (
  EFI_CUSTOMIZED_DECOMPRESS_PROTOCOL  **This
  )
;

VOID
SwitchStacks (
  VOID  *EntryPoint,
  UINTN Parameter,
  VOID  *NewStack,
  VOID  *NewBsp
  )
;


EFI_STATUS
PeiFindFile (
  IN  EFI_PEI_SERVICES       **PeiServices,
  IN  UINT8                  Type,
  IN  UINT16                 SectionType,
  OUT EFI_GUID               *FileName,
  OUT VOID                   **Pe32Data
  )
;

EFI_STATUS
PeiLoadFile (
  IN  EFI_PEI_SERVICES                          **PeiServices,
  IN  EFI_PEI_ELF_LOADER_PROTOCOL           *PeiEfiPeiElfLoader,
  IN  EFI_PEI_FLUSH_INSTRUCTION_CACHE_PROTOCOL  *PeiEfiPeiFlushInstructionCache,
  IN  VOID                                      *Pe32Data,
  OUT EFI_PHYSICAL_ADDRESS                      *ImageAddress,
  OUT UINT64                                    *ImageSize,
  OUT EFI_PHYSICAL_ADDRESS                      *EntryPoint
  )
;

EFI_STATUS
ElfLoaderGetImageInfo (
  IN     EFI_PEI_ELF_LOADER_PROTOCOL       *This,
  IN OUT EFI_PEI_ELF_LOADER_IMAGE_CONTEXT  *ImageContext
  )
;

EFI_STATUS
ElfLoaderRelocateImage (
  IN     EFI_PEI_ELF_LOADER_PROTOCOL       *This,
  IN OUT EFI_PEI_ELF_LOADER_IMAGE_CONTEXT  *ImageContext
  )
;

EFI_STATUS
ElfLoaderLoadImage (
  IN     EFI_PEI_ELF_LOADER_PROTOCOL       *This,
  IN OUT EFI_PEI_ELF_LOADER_IMAGE_CONTEXT  *ImageContext
  )
;

EFI_STATUS
CreateArchSpecificHobs (
  IN  EFI_PEI_SERVICES          **PeiServices,
  OUT EFI_PHYSICAL_ADDRESS      *BspStore
  )
;
EFI_PHYSICAL_ADDRESS
CreateIdentityMappingPageTables (
  IN EFI_PEI_SERVICES      **PeiServices,
  IN UINT32 NumberOfProcessorPhysicalAddressBits
  )
 ;

 EFI_PHYSICAL_ADDRESS
AllocateZeroedHobPages (
  IN  EFI_PEI_SERVICES       **PeiServices,
  IN  UINTN   NumberOfPages
  );

VOID
ActivateLongMode (
  IN  EFI_PHYSICAL_ADDRESS  PageTables,  
  IN  EFI_PHYSICAL_ADDRESS  HobStart,
  IN  EFI_PHYSICAL_ADDRESS  Stack,
  IN  EFI_PHYSICAL_ADDRESS  CodeEntryPoint1,
  IN  EFI_PHYSICAL_ADDRESS  CodeEntryPoint2
  );

VOID
LoadGo64Gdt();
#endif
