/*++

Copyright (c) 1998  Intel Corporation

Module Name:

    test2.c
    
Abstract:

    Code to determine the handle of the block device of the loaded path, without
    materially using efilib.h


Revision History

--*/

#include "efi.h"
#include "efilib.h"


EFI_HANDLE
GetRootBlockDeviceHandle (
    IN EFI_HANDLE           DeviceHandle,
    IN EFI_GUID             *Protocol
    )
// Finds the originating device handle for protocol of DeviceHandle 
{
    EFI_DEVICE_PATH         *DevicePath, *TestPath;
    UINTN                   PathSize, Size;
    UINTN                   BufferSize;
    UINTN                   NoHandles, Index;
    UINTN                   BestMatchSize;
    EFI_HANDLE              *Handles;
    EFI_HANDLE              BestMatch;
    EFI_STATUS              Status;    
    
    //
    // Get the device path of the device
    //

    Status = BS->HandleProtocol(DeviceHandle, &DevicePathProtocol, (VOID*)&DevicePath);
    ASSERT (!EFI_ERROR(Status));
    PathSize = DevicePathSize(DevicePath);

    //
    // Get the array of all handles that support block io
    //

    Handles = NULL;
    BufferSize = 1;
    do {

        if (Handles) {
            FreePool (Handles);
        }

        Handles = AllocatePool (BufferSize);
        Status = BS->LocateHandle (ByProtocol, Protocol, NULL, &BufferSize, Handles);

    } while (Status == EFI_BUFFER_TOO_SMALL);
    ASSERT (!EFI_ERROR(Status));
    Print(L"got here\n");

    //
    // Check each handle's device path for the shortest match
    //

    BestMatch = NULL;
    BestMatchSize = PathSize;
    NoHandles  = BufferSize / sizeof(EFI_HANDLE);
    for (Index = 0; Index < NoHandles; Index++) {

        Print(L"Index: %d\n", Index);
        Status = BS->HandleProtocol (Handles[Index], &DevicePathProtocol, (VOID*)&TestPath);

        if (!EFI_ERROR(Status)) {
            Size = DevicePathSize(TestPath);
            if (Size <= PathSize &&
                Size <= BestMatchSize &&
                CompareMem (TestPath, DevicePath, Size - sizeof(EFI_DEVICE_PATH)) == 0) {

                //
                // Got a hit.
                //

                BestMatchSize = Size;
                BestMatch = Handles[Index];
            }
        }
    }

    FreePool (Handles);

    return BestMatch;
}



EFI_STATUS
InitializeTest2Application (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    )
{
    EFI_LOADED_IMAGE        *ImageInfo;
    EFI_HANDLE              Handle;
    EFI_STATUS              Status;    
    

    InitializeLib (ImageHandle, SystemTable);
    Print(L"Test2 application started\n");

    //
    // Get the device path for the device that loaded this image
    //

    Status = BS->HandleProtocol(ImageHandle, &LoadedImageProtocol, (VOID*)&ImageInfo);
    ASSERT (!EFI_ERROR(Status));

    //
    // Get the original block device from the image's devicehandle
    //

    Handle = GetRootBlockDeviceHandle(ImageInfo->DeviceHandle, &BlockIoProtocol);

    //
    // Print the result
    //

    Print(L"The blkio handle with the smallest matching device path is: %x\n", Handle);
    return EFI_SUCCESS;
}
