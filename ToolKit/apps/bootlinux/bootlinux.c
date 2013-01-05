/*++

Copyright (c) 1998  Intel Corporation

Module Name:

    osloader.c
    
Abstract:


Author:

Revision History

--*/

#include "efi.h"
#include "efilib.h"
#include "elf.h"

BOOLEAN
CheckELFHeader (
    IN  UINT8   *Base
    )
;

BOOLEAN
JumpToImage (
    IN UINT8                 *ImageStart,
    IN UINTN                 NoEntries,
    IN EFI_MEMORY_DESCRIPTOR *MemoryMap,
    IN UINTN                 MapKey,
    IN UINTN                 DescriptorSize,
    IN UINT32                DescriptorVersion
    );

EFI_STATUS
LoadElf64 (
  IN UINT8           *Header,
  IN EFI_FILE_HANDLE File
  );


EFI_STATUS
InitializeOSLoader (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    )
{
    EFI_STATUS              Status;
    UINT16                  InputString[20];

    EFI_TIME                Time;

    CHAR16                  *DevicePathAsString;

    EFI_LOADED_IMAGE        *LoadedImage;
    EFI_DEVICE_PATH         *DevicePath;

    EFI_FILE_IO_INTERFACE   *Vol;
    EFI_FILE_HANDLE         RootFs;
    EFI_FILE_HANDLE         CurDir;
    EFI_FILE_HANDLE         FileHandle;
    CHAR16                  FileName[100];
    UINTN                   i;

    UINTN                   Size;
    VOID                    *OsKernelBuffer;

    EFI_MEMORY_DESCRIPTOR   *MemoryMap;
    UINTN                   NoEntries;
    UINTN                   MapKey;
    UINTN                   DescriptorSize;
    UINT32                  DescriptorVersion;



    //
    // Initialize the Library. Set BS, RT, &ST globals
    //  BS = Boot Services RT = RunTime Services
    //  ST = System Table
    //

    InitializeLib (ImageHandle, SystemTable);

    //
    // Print a message to the console output device.
    //

    Print(L"OS Loader application started\n");

    //
    // Print Date and Time 
    //

    Status = RT->GetTime(&Time,NULL);

    if (!EFI_ERROR(Status)) {
        Print(L"Date : %02d/%02d/%04d  Time : %02d:%02d:%02d\n",Time.Month,Time.Day,Time.Year,Time.Hour,Time.Minute,Time.Second);
    }

    //
    // Get the device handle and file path to the EFI OS Loader itself.
    //

    Status = BS->HandleProtocol (ImageHandle, 
                                 &LoadedImageProtocol, 
                                 (VOID*)&LoadedImage
                                 );

    if (EFI_ERROR(Status)) {
        Print(L"Can not retrieve a LoadedImageProtocol handle for ImageHandle\n");
        BS->Exit(ImageHandle,EFI_SUCCESS,0,NULL);
    }

    Status = BS->HandleProtocol (LoadedImage->DeviceHandle, 
                                 &DevicePathProtocol, 
                                 (VOID*)&DevicePath
                                 );

    if (EFI_ERROR(Status) || DevicePath==NULL) {
        Print(L"Can not find a DevicePath handle for LoadedImage->DeviceHandle\n");
        BS->Exit(ImageHandle,EFI_SUCCESS,0,NULL);
    }

    DevicePathAsString = DevicePathToStr(DevicePath);
    if (DevicePathAsString != NULL) {
        Print (L"Image device : %s\n", DevicePathAsString);
        FreePool(DevicePathAsString);
    }

    DevicePathAsString = DevicePathToStr(LoadedImage->FilePath);
    if (DevicePathAsString != NULL) {
        Print (L"Image file   : %s\n", DevicePathToStr (LoadedImage->FilePath));
        FreePool(DevicePathAsString);
    }

    Print (L"Image Base   : %X\n", LoadedImage->ImageBase);
    Print (L"Image Size   : %X\n", LoadedImage->ImageSize);

    //
    // Open the volume for the device where the EFI OS Loader was loaded from.
    //

    Status = BS->HandleProtocol (LoadedImage->DeviceHandle,
                                 &FileSystemProtocol,
                                 (VOID*)&Vol
                                 );

    if (EFI_ERROR(Status)) {
        Print(L"Can not get a FileSystem handle for LoadedImage->DeviceHandle\n");
        BS->Exit(ImageHandle,EFI_SUCCESS,0,NULL);
    }

    Status = Vol->OpenVolume (Vol, &RootFs);

    if (EFI_ERROR(Status)) {
        Print(L"Can not open the volume for the file system\n");
        BS->Exit(ImageHandle,EFI_SUCCESS,0,NULL);
    }

    CurDir = RootFs;

    //
    // Open the file vmlinux in the same path as the EFI OS Loader.
    //

    DevicePathAsString = DevicePathToStr(LoadedImage->FilePath);
    if (DevicePathAsString!=NULL) {
        StrCpy(FileName,DevicePathAsString);
        FreePool(DevicePathAsString);
    }
    for(i=StrLen(FileName);i>0 && FileName[i]!='/';i--);
	if( FileName[i-1] == '\\' )
		i-- ;
    FileName[i] = 0;
    StrCat(FileName,L"\\vmlinux");

    Status = CurDir->Open (CurDir,
                           &FileHandle,
                           FileName,
                           EFI_FILE_MODE_READ,
                           0
                           );

    if (EFI_ERROR(Status)) {
        Print(L"Can not open the file %s\n",FileName);
        BS->Exit(ImageHandle,EFI_SUCCESS,0,NULL);
    }

    Print(L"Opened %s\n",FileName);

    //
    // Allocate a 1MB buffer for vmlinux
    //

    Size = 0x00100000;
    BS->AllocatePool(EfiLoaderData,
                     Size,
                     &OsKernelBuffer
                     );

    if (OsKernelBuffer == NULL) {
        Print(L"Can not allocate a buffer for the file %s\n",FileName);
        BS->Exit(ImageHandle,EFI_SUCCESS,0,NULL);
    }

    //
    // Load vmlinux into the allocated memory.
    //

    Status = FileHandle->Read(FileHandle,
                              &Size,
                              OsKernelBuffer
                              );

    if (EFI_ERROR(Status)) {
        Print(L"Can not read the file %s\n",FileName);
        BS->Exit(ImageHandle,EFI_SUCCESS,0,NULL);
    }

    Print(L"%X bytes of vmlinux read into memory at %X\n",Size,OsKernelBuffer);

    //
    // Get the most current memory map.
    //

    MemoryMap = LibMemoryMap(&NoEntries,&MapKey,&DescriptorSize,&DescriptorVersion);

    //
    // Transition from Boot Services to Run Time Services.  
    //

    Print(L"Call ExitBootServices()\n");

    //BS->ExitBootServices(ImageHandle,MapKey);

  //
  // Assume the OsKerenlBuffer contains a ELF image and jump to it.
  //  The image could have been loaded as a file to a fixed address or
  //  loaded with BS->LoadImage(). The OS could also have it's own format
  //  that the loader understands.
  //
  if (CheckELFHeader (OsKernelBuffer)) {
    LoadElf64(OsKernelBuffer, FileHandle);

    Status = FileHandle->Close(FileHandle);
    if (EFI_ERROR(Status)) {
        Print(L"Can not close the file %s\n",FileName);
        BS->Exit(ImageHandle,EFI_SUCCESS,0,NULL);
    }

    JumpToImage (OsKernelBuffer,  NoEntries, MemoryMap, MapKey, DescriptorSize, DescriptorVersion);
    //
    // Should never get here.
    //  ResetSystem with Error level if we get here. This will only work if the 
    //  kernel does not dealloct the code image of the loader.
    //
    Print(L"Illegal Kerenl Exit\n");
    Print(L"\nPress [ENTER] to continue...\n");
    Input(NULL,InputString,20);

    RT->ResetSystem (EfiResetWarm, EFI_LOAD_ERROR, 40, L"Illegal Kerenl Exit");
  }

  Print(L"Illegal Kerenl image\n");
  Print(L"\nPress [ENTER] to continue...\n");
  Input(NULL,InputString,20);
    
  //
  // Image does not contain a valid entry point
  //
  RT->ResetSystem (EfiResetWarm, EFI_LOAD_ERROR, 42, L"Illegal Kerenl image");    

  return EFI_SUCCESS;
}

//
// This code assumes vmlinux is a elf64 image. If the image is in another 
//  format that method for doing an indirect procedure call to that envirnment
//  must be used. 
//


BOOLEAN
CheckELFHeader (
    IN  UINT8   *Base
    )
{
  Elf64_Ehdr *e64;

  e64 = (Elf64_Ehdr *)Base;

  if ((e64->e_ident[0] != 0x7f) ||
      (e64->e_ident[1] != 'E') ||
      (e64->e_ident[2] != 'L') ||
      (e64->e_ident[3] != 'F')) {
    return FALSE;
  }

  if (e64->e_ident[EI_CLASS] != ELFCLASS64) {
    return FALSE;
  }

  if (e64->e_type != ET_EXEC) {
    return FALSE;
  }

  if (e64->e_machine != EM_MIPS) {
    return FALSE;
  }

  if (e64->e_phentsize != sizeof (Elf64_Phdr)) {
    return FALSE;
  }

  return TRUE;
}

