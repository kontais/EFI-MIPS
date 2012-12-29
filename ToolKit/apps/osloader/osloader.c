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
#include "pe.h"

static CHAR16  *OsLoaderMemoryTypeDesc[EfiMaxMemoryType]  = {
            L"reserved  ",
            L"LoaderCode",
            L"LoaderData",
            L"BS_code   ",
            L"BS_data   ",
            L"RT_code   ",
            L"RT_data   ",
            L"available ",
            L"Unusable  ",
            L"ACPI_recl ",
            L"ACPI_NVS  ",
            L"MemMapIO  ",
			L"MemPortIO ",
			L"PAL_code  "
    };

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

    UINTN                   NoHandles;
    EFI_HANDLE              *HandleBuffer;

    EFI_BLOCK_IO            *BlkIo;
    EFI_BLOCK_IO_MEDIA      *Media;
    UINT8                   *Block;
    UINT32                  MediaId;

    VOID                    *AcpiTable              = NULL;
    VOID                    *SMBIOSTable            = NULL;
    VOID                    *SalSystemTable         = NULL;
    VOID                    *MpsTable               = NULL;

    EFI_MEMORY_DESCRIPTOR   *MemoryMap;
    EFI_MEMORY_DESCRIPTOR   *MemoryMapEntry;
    UINTN                   NoEntries;
    UINTN                   MapKey;
    UINTN                   DescriptorSize;
    UINT32                  DescriptorVersion;

    UINTN                   VariableNameSize;
    CHAR16                  VariableName[256];
    EFI_GUID                VendorGuid;
    UINT8                   *VariableValue;

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
    // Open the file OSKERNEL.BIN in the same path as the EFI OS Loader.
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
    StrCat(FileName,L"\\OSKERNEL.BIN");

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
    // Allocate a 1MB buffer for OSKERNEL.BIN
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
    // Load OSKERNEL.BIN into the allocated memory.
    //

    Status = FileHandle->Read(FileHandle,
                              &Size,
                              OsKernelBuffer
                              );

    if (EFI_ERROR(Status)) {
        Print(L"Can not read the file %s\n",FileName);
        BS->Exit(ImageHandle,EFI_SUCCESS,0,NULL);
    }

    Print(L"%X bytes of OSKERNEL.BIN read into memory at %X\n",Size,OsKernelBuffer);

    //
    // Close OSKERNEL.BIN 
    //

    Status = FileHandle->Close(FileHandle);
    if (EFI_ERROR(Status)) {
        Print(L"Can not close the file %s\n",FileName);
        BS->Exit(ImageHandle,EFI_SUCCESS,0,NULL);
    }

    //
    // Free the resources allocated from pool.
    //

#ifndef REAL_OS_LOADER
    FreePool(OsKernelBuffer);
#endif

    Print(L"\nPress [ENTER] to continue...\n");
    Input(NULL,InputString,20);

    //
    // Get a list of all the BLOCK_IO devices
    //

    NoHandles = 0;
    HandleBuffer = NULL;
    Status = LibLocateHandle(ByProtocol,
                             &BlockIoProtocol,
                             NULL,
                             &NoHandles,
                             &HandleBuffer
                             );
    if (EFI_ERROR(Status)) {
        Print(L"Can not get the array of BLOCK_IO handles\n");
        BS->Exit(ImageHandle,EFI_SUCCESS,0,NULL);
    }

    //
    // Read the first block from each BLOCK_IO device and display it
    //

    for(i=0;i<NoHandles;i++) {

        //
        // Get the DEVICE_PATH Protocol Interface to the device
        //

        Status = BS->HandleProtocol (HandleBuffer[i], 
                                     &DevicePathProtocol, 
                                     (VOID*)&DevicePath
                                     );

        if (EFI_ERROR(Status) || DevicePath==NULL) {
            Print(L"Can not get a DevicePath handle for HandleBuffer[%d]\n",i);
            BS->Exit(ImageHandle,EFI_SUCCESS,0,NULL);
        }
        
        //
        // Get the BLOCK_IO Protocol Interface to the device
        //

        Status = BS->HandleProtocol (HandleBuffer[i],
                                     &BlockIoProtocol,
                                     (VOID*)&BlkIo
                                     );
    
        if (EFI_ERROR(Status) || BlkIo==NULL) {
            Print(L"Handle does not support the BLOCK_IO protocol\n");
            BS->Exit(ImageHandle,EFI_SUCCESS,0,NULL);
        }

        //
        // Allocate a buffer for the first block on the device.
        //

        Media = BlkIo->Media;

        Block = AllocatePool (Media->BlockSize);
        if (Block == NULL) {
            Print(L"Can not allocate buffer for a block\n");
            BS->Exit(ImageHandle,EFI_SUCCESS,0,NULL);
        }

        //
        // Read block #0 from the device.
        //

        MediaId = Media->MediaId;
 
        Status = BlkIo->ReadBlocks(BlkIo, 
                                   MediaId, 
                                   (EFI_LBA)0, 
                                   Media->BlockSize, 
                                   Block);

        //
        // Display block#0 from the device.
        //

        DevicePathAsString = DevicePathToStr(DevicePath);
        if (DevicePathAsString != NULL) {
            Print(L"\nBlock #0 of device %s\n",DevicePathAsString);
            FreePool(DevicePathAsString);
        }
        DumpHex(0,0,Media->BlockSize,Block);

        //
        // Free the resources allocated from pool.
        //

        FreePool(Block);

        Print(L"\nPress [ENTER] to continue...\n");
        Input(NULL,InputString,20);
    }

    //
    // Free the resources allocated from pool.
    //

    FreePool(HandleBuffer);

    //
    // Get System Configuration
    //

    Print(L"System Configuration Tables:\n\n");

    Status = LibGetSystemConfigurationTable(&AcpiTableGuid,&AcpiTable);
    if (!EFI_ERROR(Status)) {
        Print(L"  ACPI Table is at address                   : %X\n",AcpiTable);
    }
    Status = LibGetSystemConfigurationTable(&SMBIOSTableGuid,&SMBIOSTable);
    if (!EFI_ERROR(Status)) {
        Print(L"  SMBIOS Table is at address                 : %X\n",SMBIOSTable);
    }
    Status = LibGetSystemConfigurationTable(&SalSystemTableGuid,&SalSystemTable);
    if (!EFI_ERROR(Status)) {
        Print(L"  Sal System Table is at address             : %X\n",SalSystemTable);
    }
    Status = LibGetSystemConfigurationTable(&MpsTableGuid,&MpsTable);
    if (!EFI_ERROR(Status)) {
        Print(L"  MPS Table is at address                    : %X\n",MpsTable);
    }

    Print(L"\nPress [ENTER] to continue...\n");
    Input(NULL,InputString,20);

    //
    // Display the current Memory Map
    //

    MemoryMap = LibMemoryMap(&NoEntries,&MapKey,&DescriptorSize,&DescriptorVersion);
    if (MemoryMap == NULL) {
        Print(L"Can not retrieve the current memory map\n");
        BS->Exit(ImageHandle,EFI_SUCCESS,0,NULL);
    }

    Print(L"Memory Descriptor List:\n\n");
    Print(L"  Type        Start Address     End Address       Attributes      \n");
    Print(L"  ==========  ================  ================  ================\n");
    MemoryMapEntry = MemoryMap;
    for(i=0;i<NoEntries;i++) {
        Print(L"  %s  %lX  %lX  %lX\n",
              OsLoaderMemoryTypeDesc[MemoryMapEntry->Type],
              MemoryMapEntry->PhysicalStart,
              MemoryMapEntry->PhysicalStart+LShiftU64(MemoryMapEntry->NumberOfPages,EFI_PAGE_SHIFT)-1,
              MemoryMapEntry->Attribute);
        MemoryMapEntry = NextMemoryDescriptor(MemoryMapEntry, DescriptorSize);
    }

    //
    // Free the resources allocated from pool.
    //

    FreePool(MemoryMap);

    Print(L"\nPress [ENTER] to continue...\n");
    Input(NULL,InputString,20);

    //
    // Display all the Environment Variables
    //

    Print(L"Environment Variable List:\n\n");

    VariableName[0] = 0x0000;
    VendorGuid = NullGuid;
    Print(L"GUID                                Variable Name        Value Pointer\n");
    Print(L"=================================== ==================== =============\n");
    do {
        VariableNameSize = 256;
        Status = RT->GetNextVariableName(&VariableNameSize,
                                         VariableName,
                                         &VendorGuid
                                         );

        if (Status == EFI_SUCCESS) {
            VariableValue = LibGetVariable(VariableName,&VendorGuid);
            if (VariableValue != NULL) {
                Print(L"%.-35g %.-20s %X\n",&VendorGuid,VariableName,VariableValue);
                FreePool(VariableValue);
            }
        }
    } while (Status == EFI_SUCCESS);

    Print(L"\nPress [ENTER] to continue...\n");
    Input(NULL,InputString,20);

    //
    // Get the most current memory map.
    //

    MemoryMap = LibMemoryMap(&NoEntries,&MapKey,&DescriptorSize,&DescriptorVersion);

    //
    // Transition from Boot Services to Run Time Services.  
    //

    Print(L"Call ExitBootServices()\n");

#ifdef REAL_OS_LOADER
    BS->ExitBootServices(ImageHandle,MapKey);

    //
    // Assume the OsKerenlBuffer contains a PE32+ image and jump to it.
    //  The image could have been loaded as a file to a fixed address or
    //  loaded with BS->LoadImage(). The OS could also have it's own format
    //  that the loader understands.
    //
    if (CheckPEHeader (OsKernelBuffer)) {
        JumpToImage (OsKernelBuffer,  NoEntries, MemoryMap, MapKey, DescriptorSize, DescriptorVersion);
        //
        // Should never get here.
        //  ResetSystem with Error level if we get here. This will only work if the 
        //  kernel does not dealloct the code image of the loader.
        //
        RT->ResetSystem (EfiResetWarm, EFI_LOAD_ERROR, 40, L"Illegal Kerenl Exit");
    } else {
        //
        // Image does not contain a valid entry point
        //
        RT->ResetSystem (EfiResetWarm, EFI_LOAD_ERROR, 42, L"Illegal Kerenl image");        
    }
#else

    //
    // We are an application simulating a loader so exit back to EFI
    //

    Print(L"\nPress [ENTER] to continue...\n");
    Input(NULL,InputString,20);

    return EFI_SUCCESS;
#endif
}

//
// This code assumes OSKERNEL.BIN is a PE32+ image. If the image is in another 
//  format that method for doing an indirect procedure call to that envirnment
//  must be used. 
//


BOOLEAN
CheckPEHeader (
    IN  UINT8   *Base
    )
{
    IMAGE_DOS_HEADER			*DosHdr;
    IMAGE_NT_HEADERS			*PeHdr;

    DosHdr = (IMAGE_DOS_HEADER *)Base;
    if (DosHdr->e_magic != IMAGE_DOS_SIGNATURE) {
        return FALSE;
    }
    PeHdr = (IMAGE_NT_HEADERS *)(Base + DosHdr->e_lfanew);
    if (PeHdr->Signature != IMAGE_NT_SIGNATURE) {
        return FALSE;
    }

#ifdef EFI32
  #define       APP_OSLDR_PEHDR_MACHINE     EFI_IMAGE_MACHINE_IA32
#else
  #ifdef EFI64
    #define     APP_OSLDR_PEHDR_MACHINE     EFI_IMAGE_MACHINE_IA64
  #else
    #ifdef EFIX64
      #define   APP_OSLDR_PEHDR_MACHINE     EFI_IMAGE_MACHINE_X64
    #else
      #ifdef EFIEBC // though currently tookit doesn't have an EBC implementation
        #define APP_OSLDR_PEHDR_MACHINE     EFI_IMAGE_MACHINE_EBC
      #else
        #error "Machine Type for PE Header Illegal"
      #endif
    #endif
  #endif
#endif

    if (PeHdr->FileHeader.Machine != APP_OSLDR_PEHDR_MACHINE) {
        return FALSE;
    }

    return TRUE;
}

