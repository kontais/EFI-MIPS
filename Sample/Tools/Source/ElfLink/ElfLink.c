/*++

Copyright (c) 2010 Kontais                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  ElfLink.c

Abstract:

  This file contains functions required to generate a Firmware File System
  file.

--*/

#include "TianoCommon.h"
#include "EfiFirmwareFileSystem.h"
#include "EfiFirmwareVolumeHeader.h"
#include "EfiImageFormat.h"
#include "ParseInf.h"
#include "Compress.h"
#include "EfiCustomizedCompress.h"
#include "Crc32.h"
#include "ElfLink.h"
#include <stdio.h>
#include <ctype.h>  // for isalpha()
#include "SysLib.h"
#include <stdlib.h>
#include <string.h>
#include "CommonLib.h"
#include "EfiUtilityMsgs.h"
#include "SimpleFileParsing.h"
#include EFI_GUID_DEFINITION (PeiElfLoader)

extern EFI_PEI_ELF_LOADER_PROTOCOL mElfLoader;


#define UTILITY_NAME  "ElfLink"

static
void
PrintUsage (
  void
  );

EFI_STATUS
ElfLinkImageRead (
  IN     VOID    *FileHandle,
  IN     UINTN   FileOffset,
  IN OUT UINT32  *ReadSize,
  OUT    VOID    *Buffer
  )
;


int
main (
  INT32 argc,
  CHAR8 *argv[]
  )
/*++

  Routine Description:

  Main function.

Arguments:

  argc - Number of command line parameters.
  argv - Array of pointers to parameter strings.

Returns:
  STATUS_SUCCESS - Utility exits successfully.
  STATUS_ERROR   - Some error occurred during execution.

--*/
{
  EFI_STATUS                            Status;
  UINT64                                DestAddress;
  FILE                                  *In;
  FILE                                  *Out;
  FILE                                  *Out1;
  FILE                                  *Out2;
  VOID                                  *MemoryFileIn;
  VOID                                  *MemoryFileOut;
  UINTN                                 InFileSize;
  UINTN                                 OutFileSize;
  EFI_PEI_ELF_LOADER_IMAGE_CONTEXT      ImageContext;
  UINTN                                 MemoryImagePointer;
  UINTN                                 MemoryImagePointerAligned;

  EFI_PHYSICAL_ADDRESS                  ImageAddress;
  UINT64                                ImageSize;
  EFI_PHYSICAL_ADDRESS                  EntryPoint;

  UINT32                                Pe32FileSize;
  UINT32                                NewPe32BaseAddress;

  UINTN                                 Index;
  EFI_FILE_SECTION_POINTER              CurrentPe32Section;
  UINT8                                 FileGuidString[80];



  if (argc < 4) {
    PrintUsage();
  }

  DestAddress = strtoll(argv[1],NULL,16);

  In          = fopen(argv[2], "rb");
  if (In == NULL) {
    printf("Unable to open file \"%s\"\n",argv[2]);
    return -1;
  }

  Out         = fopen(argv[3], "wb");
  if (Out == NULL) {
    printf("Unable to open file \"%s\"\n",argv[2]);
    goto QuitRelease1;
  }

  InFileSize   = GetFileSize(In);
  MemoryFileIn = malloc(InFileSize);
  if (MemoryFileIn == NULL) {
    printf("malloc error.\n");
    goto QuitRelease2;
  }

  if (fread(MemoryFileIn, InFileSize, 1, In) != 1) {
    printf("ElfLink fread errror.\n");
    goto QuitRelease3;
  }

  //
  // Initialize context
  //
  memset(&ImageContext, 0, sizeof(ImageContext));
  ImageContext.Handle    = MemoryFileIn;
  ImageContext.ImageRead = (EFI_PEI_ELF_LOADER_READ_FILE) ElfLinkImageRead;

  Status                 = mElfLoader.GetImageInfo (&mElfLoader, &ImageContext);
  if (EFI_ERROR(Status)) {
    printf("GetImageInfo error. Status =0x%x\n",Status);
    goto QuitRelease3;
  }

  Pe32FileSize = ImageContext.ImageSize;
  printf("ImageContext.ImageSize = 0x%x\n",(UINTN)ImageContext.ImageSize);

  MemoryImagePointer        = (UINTN) (malloc (ImageContext.ImageSize + 0x1000));

  MemoryImagePointerAligned = (MemoryImagePointer + 0x0FFF) & (-1 << 12);
  if (MemoryImagePointerAligned == 0) {
    printf("malloc error.\n");
    goto QuitRelease3;
  }


  ImageContext.ImageAddress = MemoryImagePointerAligned;
  ImageContext.DestinationAddress = DestAddress;

  printf("ImageContext.DestinationAddress = 0x%x\n",(UINTN)ImageContext.DestinationAddress);
  printf("ImageContext.ImageAddress = 0x%x\n",(UINTN)ImageContext.ImageAddress);

  Status = mElfLoader.LoadImage (&mElfLoader, &ImageContext);
  if (EFI_ERROR (Status)) {
    printf("LoadImage error. Status =0x%x\n",Status);
    goto QuitRelease4;
  }


  Status = mElfLoader.RelocateImage (&mElfLoader, &ImageContext);
  if (EFI_ERROR (Status)) {
    printf("RelocateImage error. Status =0x%x\n",Status);
    printf("Error = %d\n",ImageContext.ImageError);
    goto QuitRelease4;
  }


  if (fwrite((VOID*) ImageContext.ImageAddress, (size_t) ImageContext.ImageSize, 1, Out) != 1) {
    goto QuitRelease4;
  }

  printf("ImageContext.DestinationAddress = 0x%x\n",(UINTN)ImageContext.DestinationAddress);
  printf("ImageContext.ImageAddress = 0x%x\n",(UINTN)ImageContext.ImageAddress);
  printf("ImageContext.ImageSize = 0x%x\n",(UINTN)ImageContext.ImageSize);

  ImageAddress  = ImageContext.ImageAddress;
  ImageSize     = ImageContext.ImageSize;
  EntryPoint    = ImageContext.EntryPoint;

  if (ImageSize > Pe32FileSize) {
    printf("ImageSize > Original file after LoadImage\n");
  }

  goto QuitSuccess;

QuitRelease4:
  free(MemoryImagePointer);
QuitRelease3:
  free(MemoryFileIn);
QuitRelease2:
  fclose(Out);
QuitRelease1:
  fclose(In);
  return -1;
QuitSuccess:

  return EFI_SUCCESS;
}

static
void
PrintUsage (
  void
  )
/*++

Routine Description:

  Print Error / Help message.

Arguments:

  void

Return:

  None

--*/
{
  printf("Usage:\n");
  printf(UTILITY_NAME " 0xXXXXXXXX InputFile OutputFile\n");
  printf("\n");
  return;
}


EFI_STATUS
ElfLinkImageRead (
  IN     VOID    *FileHandle,
  IN     UINTN   FileOffset,
  IN OUT UINT32  *ReadSize,
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
  CHAR8   *Destination8;
  CHAR8   *Source8;
  UINT32  Length;

  Destination8  = Buffer;
  Source8       = (CHAR8 *) ((UINTN) FileHandle + FileOffset);
  Length        = *ReadSize;
  while (Length--) {
    *(Destination8++) = *(Source8++);
  }

  return EFI_SUCCESS;
}


