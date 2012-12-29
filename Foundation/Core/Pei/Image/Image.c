/*++

Copyright (c) 2004 - 2007, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  Image.c

Abstract:

  Pei Core Load Image Support

--*/

#include "Tiano.h"
#include "PeiCore.h"
#include "PeiLib.h"

#include EFI_PPI_DEFINITION (LoadFile)

#include EFI_GUID_DEFINITION(PeiFlushInstructionCache)

STATIC
EFI_STATUS
PeiCoreLoadFile (
  IN  EFI_PEI_SERVICES                          **PeiServices,
  IN  EFI_PEI_ELF_LOADER_PROTOCOL               *PeiEfiPeiElfLoader,
  IN  EFI_PEI_FLUSH_INSTRUCTION_CACHE_PROTOCOL  *PeiEfiPeiFlushInstructionCache,
  IN  VOID                                      *Pe32Data,
  OUT EFI_PHYSICAL_ADDRESS                      *ImageAddress,
  OUT UINT64                                    *ImageSize,
  OUT EFI_PHYSICAL_ADDRESS                      *EntryPoint
  )
;

STATIC
EFI_STATUS
PeiCoreImageRead (
  IN     VOID    *FileHandle,
  IN     UINTN   FileOffset,
  IN OUT UINTN   *ReadSize,
  OUT    VOID    *Buffer
  )
;

EFI_STATUS
PeiLoadImage (
  IN EFI_PEI_SERVICES         **PeiServices,
  IN EFI_FFS_FILE_HEADER      *PeimFileHeader,
  OUT VOID                    **EntryPoint
  )
/*++

Routine Description:

  Routine for loading file image.

Arguments:

  PeiServices     - The PEI core services table.
  PeimFileHeader  - Pointer to the FFS file header of the image.
  EntryPoint      - Pointer to entry point of specified image file for output.

Returns:

  Status - EFI_SUCCESS    - Image is successfully loaded.
           EFI_NOT_FOUND  - Fail to locate necessary PPI
           Others         - Fail to load file.

--*/
{
  EFI_STATUS                  Status;
  VOID                        *Pe32Data;
  EFI_IMAGE_ELF_HEADERS       *PeHdr;
  EFI_IMAGE_ELF_HEADERS       *TeHdr;
  EFI_PEI_FV_FILE_LOADER_PPI  *FvLoadFilePpi;
#ifdef EFI_LINUX_EMULATOR
  EFI_PEI_PPI_DESCRIPTOR      *PpiDescriptor;
  LINUX_PEI_LOAD_FILE_PPI     *PeiLinuxService;
  EFI_PHYSICAL_ADDRESS        LinuxEntryPoint;
#endif
  EFI_PHYSICAL_ADDRESS        ImageAddress;
  UINT64                      ImageSize;
  EFI_PHYSICAL_ADDRESS        ImageEntryPoint;
  UINT16                      Machine;
  EFI_PEI_ELF_LOADER_PROTOCOL               *PeiEfiPeiElfLoader;
  EFI_PEI_FLUSH_INSTRUCTION_CACHE_PROTOCOL  *PeiEfiPeiFlushInstructionCache;

  PEI_DEBUG_CODE (ImageAddress  = 0;)
  
  *EntryPoint   = NULL;
  PeHdr         = NULL;
  TeHdr         = NULL;
  Machine       = 0;

  //
  // Find a TE section first.
  //
  Status = PeiFfsFindSectionData (
            PeiServices,
            EFI_SECTION_TE,
            PeimFileHeader,
            &Pe32Data
            );
  if (Status == EFI_SUCCESS) {
    if (*(UINT32*)(&((EFI_IMAGE_ELF_HEADERS *) Pe32Data)->e_ident[0])
        != EFI_IMAGE_ELF_SIGNATURE) {
        return EFI_UNSUPPORTED; 
    }
    
    TeHdr = (EFI_IMAGE_ELF_HEADERS *) Pe32Data;
    if (TeHdr->e_machine != EFI_IMAGE_MACHINE_TYPE) {
      return EFI_UNSUPPORTED;
    }
    Machine = TeHdr->e_machine;
    ImageAddress = (EFI_PHYSICAL_ADDRESS) (UINT32)TeHdr;
    *EntryPoint  = (VOID*) TeHdr->e_entry;
    return EFI_SUCCESS;    
  }

  //
  // Try to find a PE32 section.
  //
  Status = PeiFfsFindSectionData (
            PeiServices,
            EFI_SECTION_PE32,
            PeimFileHeader,
            &Pe32Data
            );

  if (Status == EFI_SUCCESS) {
    if (*(UINT32*)(&((EFI_IMAGE_ELF_HEADERS *) Pe32Data)->e_ident[0])
        != EFI_IMAGE_ELF_SIGNATURE) {
      return EFI_UNSUPPORTED;
      
    }

    PeHdr = (EFI_IMAGE_ELF_HEADERS *) Pe32Data;

    //
    // Check PEIM image machine. A PEIM image for different processor architecture
    // from the host can't be started.
    //
    if (PeHdr->e_machine != EFI_IMAGE_MACHINE_TYPE) {
      return EFI_UNSUPPORTED;
    }
    Machine = PeHdr->e_machine;


#ifdef EFI_LINUX_EMULATOR
    Status = (**PeiServices).LocatePpi (
                              PeiServices,
                              &gLinuxPeiLoadFileGuid,
                              0,
                              &PpiDescriptor,
                              &PeiLinuxService
                              );
    if (EFI_ERROR (Status)) {
      return Status;
    }

    Status = PeiLinuxService->PeiLoadFileService (
                            Pe32Data,
                            &ImageAddress,
                            &ImageSize,
                            &LinuxEntryPoint
                            );

    if (EFI_ERROR (Status)) {
      return Status;
    } else {
      *EntryPoint = (VOID *) (UINTN) LinuxEntryPoint;
    }

#else
    //
    // ELF relocate for PEI
    //
    InstallEfiPeiFlushInstructionCache (&PeiEfiPeiFlushInstructionCache);
    InstallEfiPeiElfLoader (PeiServices, &PeiEfiPeiElfLoader, NULL);
    Status = PeiCoreLoadFile (
              PeiServices,
              PeiEfiPeiElfLoader,
              PeiEfiPeiFlushInstructionCache,
              Pe32Data,
              &ImageAddress,
              &ImageSize,
              &ImageEntryPoint
              );
    if (EFI_ERROR(Status)) {
      return Status;
    }
    
    *EntryPoint = ImageEntryPoint;
#endif

  } else {
    //
    // There was not a PE32 section, so assume that it's a Compressed section
    // and use the LoadFile
    //
    Status = PeiLocatePpi (
              PeiServices,
              &gPeiFvFileLoaderPpiGuid,
              0,
              NULL,
              &FvLoadFilePpi
              );

    if (!EFI_ERROR (Status)) {
      Status = FvLoadFilePpi->FvLoadFile (
                                FvLoadFilePpi,
                                PeimFileHeader,
                                &ImageAddress,
                                &ImageSize,
                                &ImageEntryPoint
                                );

      if (!EFI_ERROR (Status)) {
        *EntryPoint = (VOID *) ((UINTN) ImageEntryPoint);
        if (*(UINT32*)(&((EFI_IMAGE_ELF_HEADERS *) (UINTN) ImageAddress)->e_ident[0])
            == EFI_IMAGE_ELF_SIGNATURE) {
          PeHdr = (EFI_IMAGE_ELF_HEADERS *) (UINTN) ImageAddress;
        }

        if (PeHdr != NULL) {
          Machine = PeHdr->e_machine;
        }

        //
        // Check PEIM image machine. A PEIM image for different processor architecture
        // from the host can't be started.
        //
        if (Machine != EFI_IMAGE_MACHINE_TYPE) {
          return EFI_UNSUPPORTED;
        }
      }
    }
  }

  PEI_DEBUG_CODE (
  {
    if (Status == EFI_SUCCESS) {
      //
      // Print debug message: Loading PEIM at 0x12345678 EntryPoint=0x12345688 Driver.efi
      //
      PEI_DEBUG ((PeiServices, EFI_D_INFO | EFI_D_LOAD, "Loading PEIM at 0x%08x EntryPoint=0x%08x ", (UINTN) ImageAddress, *EntryPoint));
  
      PEI_DEBUG ((PeiServices, EFI_D_INFO | EFI_D_LOAD, "\n"));
    }
  }
  )

  return Status;
}

STATIC
EFI_STATUS
PeiCoreLoadFile (
  IN  EFI_PEI_SERVICES                          **PeiServices,
  IN  EFI_PEI_ELF_LOADER_PROTOCOL               *PeiEfiPeiElfLoader,
  IN  EFI_PEI_FLUSH_INSTRUCTION_CACHE_PROTOCOL  *PeiEfiPeiFlushInstructionCache,
  IN  VOID                                      *Pe32Data,
  OUT EFI_PHYSICAL_ADDRESS                      *ImageAddress,
  OUT UINT64                                    *ImageSize,
  OUT EFI_PHYSICAL_ADDRESS                      *EntryPoint
  )
/*++

Routine Description:

  Loads and relocates a PE/COFF image into memory.

Arguments:

  PeiService         - General purpose services available to every PEIM.
  PeiEfiPeiElfLoader - Pointer to a PE COFF loader protocol
  PeiEfiPeiFlushInstructionCache  - Pointer to a flush-instruction-cache protocol so
                                    we can flush the cache after loading
  Pe32Data         - The base address of the PE/COFF file that is to be loaded and relocated
  ImageAddress     - The base address of the relocated PE/COFF image
  ImageSize        - The size of the relocated PE/COFF image
  EntryPoint       - The entry point of the relocated PE/COFF image

Returns:

  EFI_SUCCESS   - The file was loaded and relocated
  EFI_OUT_OF_RESOURCES - There was not enough memory to load and relocate the PE/COFF file

--*/
{
  EFI_STATUS                        Status;
  EFI_PEI_ELF_LOADER_IMAGE_CONTEXT  ImageContext;
  EFI_PHYSICAL_ADDRESS              MemoryBuffer;

  (*PeiServices)->SetMem (
                    &ImageContext,
                    sizeof (ImageContext),
                    0
                    );
  ImageContext.Handle    = Pe32Data;
  ImageContext.ImageRead = (EFI_PEI_ELF_LOADER_READ_FILE) PeiCoreImageRead;

  //
  // Flush the Code Copy to Memroy, 0x400 is the Code Size, 
  // make sure no little than
  //
  Status = PeiEfiPeiFlushInstructionCache->Flush (
  	                                  PeiEfiPeiFlushInstructionCache,
  	                                  ImageContext.ImageRead,
  	                                  0x400
  	                                  );

  Status = PeiEfiPeiElfLoader->GetImageInfo (PeiEfiPeiElfLoader, &ImageContext);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Allocate Memory for the image
  //
  if (ImageContext.IsRelocatable) {
    Status = (*PeiServices)->AllocatePages (
                              PeiServices,
                              EfiBootServicesData,
                              EFI_SIZE_TO_PAGES ((UINT32) ImageContext.ImageSize),
                              &MemoryBuffer
                              );

    ASSERT_PEI_ERROR (PeiServices, Status);

  ImageContext.ImageAddress = MemoryBuffer;
  }
  //
  // Load the image to our new buffer
  //
  Status = PeiEfiPeiElfLoader->LoadImage (PeiEfiPeiElfLoader, &ImageContext);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Relocate the image in our new buffer
  //
  Status = PeiEfiPeiElfLoader->RelocateImage (PeiEfiPeiElfLoader, &ImageContext);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Flush the instruction cache so the image data is written before we execute it
  //
  Status = PeiEfiPeiFlushInstructionCache->Flush (
                                            PeiEfiPeiFlushInstructionCache,
                                            ImageContext.ImageAddress,
                                            ImageContext.ImageSize
                                            );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  *ImageAddress = ImageContext.ImageAddress;
  *ImageSize    = ImageContext.ImageSize;
  *EntryPoint   = ImageContext.EntryPoint;

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
PeiCoreImageRead (
  IN     VOID    *FileHandle,
  IN     UINTN   FileOffset,
  IN OUT UINTN   *ReadSize,
  OUT    VOID    *Buffer
  )
/*++

Routine Description:

  Support routine for the PE/COFF Loader that reads a buffer from a PE/COFF file

Arguments:

  FileHandle - The handle to the PE/COFF file

  FileOffset - The offset, in bytes, into the file to read

  ReadSize   - The number of bytes to read from the file starting at FileOffset

  Buffer     - A pointer to the buffer to read the data into.

Returns:

  EFI_SUCCESS - ReadSize bytes of data were read into Buffer from the PE/COFF file starting at FileOffset

--*/
{
  CHAR8 *Destination8;
  CHAR8 *Source8;
  UINTN Length;

  Destination8  = Buffer;
  Source8       = (CHAR8 *) ((UINTN) FileHandle + FileOffset);
  Length        = *ReadSize;
  while (Length--) {
    *(Destination8++) = *(Source8++);
  }

  return EFI_SUCCESS;
}

