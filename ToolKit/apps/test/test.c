/*++

Copyright (c) 1998  Intel Corporation

Module Name:

    test.c
    
Abstract:



Revision History

--*/

#include "efi.h"
#include "efilib.h"

VOID
PrintLoadedImageInfo (
    IN  EFI_LOADED_IMAGE    *LoadedImage
    );

VOID
DumpEFIShellInformation (
    IN  EFI_HANDLE          ImageHandle,
    IN  EFI_LOADED_IMAGE    *LoadedImage
    );


EFI_STATUS
InitializeTestApplication (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    )
{
    EFI_LOADED_IMAGE    *LoadedImage;
    EFI_INPUT_KEY       Key;

    //
    // Initialize the Library. Set BS, RT, &ST globals
    //  BS = Boot Services RT = RunTime Services
    //  ST = System Table
    //
    InitializeLib (ImageHandle, SystemTable);

    Print(L"Test application started\n");

    BS->HandleProtocol (ImageHandle, &LoadedImageProtocol, (VOID*)&LoadedImage);
    PrintLoadedImageInfo (LoadedImage);


    DumpEFIShellInformation (ImageHandle, LoadedImage);

    Print(L"\n%EHit any key to exit this image%N");
    WaitForSingleEvent (ST->ConIn->WaitForKey, 0);

    //
    // flush the KB buffer once wait is satisfied
    //

    ST->ConIn->ReadKeyStroke(ST->ConIn,&Key);

    ST->ConOut->OutputString (ST->ConOut, L"\n\n");

    return EFI_SUCCESS;
}

UINT16 *MemoryType[] = {
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
            L"BUG:BUG: MaxMemoryType"
};


VOID
DumpEFIShellInformation (
    IN  EFI_HANDLE          ImageHandle,
    IN  EFI_LOADED_IMAGE    *LoadedImage
    )
{
    EFI_LOADED_IMAGE    *ParentImage;

    if (!LoadedImage->ParentHandle) {
        //
        // If you are loaded from the EFI boot manager the ParentHandle
        //  is Null. Thus a pure EFI application will not have a parrent.
        //
        Print (L"\n%HImage was loaded from EFI Boot Manager%N\n");
        return;
    }

    BS->HandleProtocol (LoadedImage->ParentHandle, &LoadedImageProtocol, (VOID*)&ParentImage);
    Print (L"\n%HImage Parent was %N%s %Hand it passed the following arguments%N\n", DevicePathToStr (ParentImage->FilePath));
}
    

VOID
PrintLoadedImageInfo (
    IN  EFI_LOADED_IMAGE    *LoadedImage
    )
{
    EFI_STATUS          Status;
    EFI_DEVICE_PATH     *DevicePath;

    Print (L"\n%HImage was loaded from file %N%s\n", DevicePathToStr (LoadedImage->FilePath));

    BS->HandleProtocol (LoadedImage->DeviceHandle, &DevicePathProtocol, (VOID*)&DevicePath);
    if (DevicePath) {
        Print (L"%HImage was loaded from this device %N%s\n", DevicePathToStr (DevicePath)); 

    }

    Print (L"\n Image Base is %X", LoadedImage->ImageBase);
    Print (L"\n Image Size is %X", LoadedImage->ImageSize);
    Print (L"\n  Image Code Type %s", MemoryType[LoadedImage->ImageCodeType]);
    Print (L"\n  Image Data Type %s", MemoryType[LoadedImage->ImageDataType]);
    Print (L"\n %d Bytes of Options passed to this Image\n", LoadedImage->LoadOptionsSize);

    if (LoadedImage->ParentHandle) {
        Status = BS->HandleProtocol (LoadedImage->ParentHandle, &DevicePathProtocol, (VOID*)&DevicePath);
        if (Status == EFI_SUCCESS && DevicePath) {
           Print (L"Images parent is %s\n\n",DevicePathToStr (DevicePath));
        }
    }
}

