/*++

Copyright (c) 1998  Intel Corporation

Module Name:

    test.c
    
Abstract:


Revision History

--*/

#include "efi.h"

//
// Globals
//

EFI_SYSTEM_TABLE        *T3ST;
EFI_BOOT_SERVICES       *T3BS;
EFI_RUNTIME_SERVICES    *T3RT;

//
// Global constants
//

CHAR8 T3HexDigit[] = {'0','1','2','3','4','5','6','7',
                      '8','9','A','B','C','D','E','F'};

EFI_GUID T3DevicePathProtocol       = DEVICE_PATH_PROTOCOL;
EFI_GUID T3LoadedImageProtocol      = LOADED_IMAGE_PROTOCOL;
EFI_GUID T3BlockIoProtocol          = BLOCK_IO_PROTOCOL;
EFI_GUID T3DiskIoProtocol           = DISK_IO_PROTOCOL;
EFI_GUID T3FileSystemProtocol       = SIMPLE_FILE_SYSTEM_PROTOCOL;

//
// Internal prototypes
//

VOID
T3Output (
    IN CHAR16       *String
    );

VOID
T3OutputValue (
    IN CHAR16           *String,
    IN UINTN            Value
    );

VOID
T3HexDump (
    IN VOID             *Buffer,
    IN UINTN            Value
    );

VOID
T3DumpDevicePath (
    IN EFI_DEVICE_PATH  *DevicePath
    );

VOID
T3DumpLoadedImageInfo (
    IN  EFI_LOADED_IMAGE    *LoadedImage
    );


//
// Code
//


EFI_STATUS
InitializeTest3Application (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    )
{
    EFI_LOADED_IMAGE        *ImageInfo;
    UINTN                   Index;
    EFI_INPUT_KEY           Key;

    //
    // Set up global pointer to the system table, boot services table,
    // and runtime services table
    //

    T3ST = SystemTable;
    T3BS = SystemTable->BootServices;
    T3RT = SystemTable->RuntimeServices;

    //
    // Get the loaded image information for this application
    //

    T3BS->HandleProtocol (ImageHandle, &T3LoadedImageProtocol, (VOID*)&ImageInfo);
    T3DumpLoadedImageInfo (ImageInfo);


    //
    // Done . . . wait
    //

    T3Output (L"Press any key to exit Test3 application");
    T3BS->WaitForEvent (1, &T3ST->ConIn->WaitForKey, &Index);
    
    //
    // flush the KB buffer once wait is satisfied
    //

    T3ST->ConIn->ReadKeyStroke (T3ST->ConIn, &Key);

    T3Output (L"\n\n\r");

    return EFI_SUCCESS;
}

VOID
T3DumpLoadedImageInfo (
    IN  EFI_LOADED_IMAGE    *ImageInfo
    )
{
    EFI_STATUS          Status;
    EFI_DEVICE_PATH     *DevicePath;
    EFI_DISK_IO         *DiskIo;
    EFI_BLOCK_IO        *BlkIo;
    CHAR8               Buffer[112];
    UINT32              MediaId;

    //
    // Dump Image Info structure
    //

    T3OutputValue (L"Handle to parent image..................: ", (UINTN) ImageInfo->ParentHandle);
    T3OutputValue (L"Image base..............................: ", (UINTN) ImageInfo->ImageBase);
    T3OutputValue (L"Image size..............................: ", (UINTN) ImageInfo->ImageSize);
    T3OutputValue (L"Image code memory type..................: ", (UINTN) ImageInfo->ImageCodeType);
    T3OutputValue (L"Image data memory type..................: ", (UINTN) ImageInfo->ImageDataType);

    T3OutputValue (L"Handle to device image was loaded from..: ", (UINTN) ImageInfo->DeviceHandle);

    //
    // If the device has a device path, dump it
    //

    Status = T3BS->HandleProtocol (ImageInfo->DeviceHandle, &T3DevicePathProtocol, (VOID*)&DevicePath);
    if (!EFI_ERROR(Status)) {
        T3Output (L"File path of image on device\n\r");
        T3DumpDevicePath (DevicePath);
    }
    //
    // Dump the file path for the image on the device
    //

    T3Output (L"This image was loaded from the file:\n\r");
    T3DumpDevicePath (ImageInfo->FilePath);

    //
    // Dump the load options
    //

    T3OutputValue (L"Image load options size.................: ", ImageInfo->LoadOptionsSize);
    if (ImageInfo->LoadOptionsSize) {
        T3HexDump (ImageInfo->LoadOptions, ImageInfo->LoadOptionsSize);
    }

    //
    // Check to see the device we're loaded from supports the disk i/o protocol
    //

    Status = T3BS->HandleProtocol (ImageInfo->DeviceHandle, &T3DiskIoProtocol, (VOID*)&DiskIo);
    if (!EFI_ERROR(Status)) {
        Status = T3BS->HandleProtocol (ImageInfo->DeviceHandle, &T3BlockIoProtocol, (VOID*)&BlkIo);
    }
    if (!EFI_ERROR(Status)) {

        T3Output (L"Loading device supports DiskIo.  Here's the first\n\r");
        T3Output (L"bytes from the disk/partition the image was loaded from.\n\r");

        //
        // Read in the first 512 bytes from the device
        //

        MediaId = BlkIo->Media->MediaId;
        Status = DiskIo->ReadDisk (DiskIo, MediaId, 0, sizeof(Buffer), Buffer);
        if (!EFI_ERROR(Status)) {
            T3HexDump (Buffer, sizeof(Buffer));
        } else {
            T3OutputValue (L"  disk read error: ", Status);
        }
    }

    T3Output (L"\n\r");
}


//
//
//

VOID
T3Output (
    IN CHAR16       *String
    )
{
    SIMPLE_TEXT_OUTPUT_INTERFACE    *ConOut;

    ConOut = T3ST->ConOut;
    ConOut->OutputString (ConOut, String);
}


VOID
T3OutputValue (
    IN CHAR16           *String,
    IN UINTN            Value
    )
{
    CHAR16              Str1[30], Str2[30];
    CHAR16              *p1, *p2;

    // Output the string
    T3Output (String);
 
    // Convert Value to a string
    p1 = Str1;
    p2 = Str2;

    while (Value) {
        *(p1++) = T3HexDigit[Value & 0xF];
        Value = Value >> 4;
    }

    while (p1 != Str1) {
        *(p2++) = *(--p1);
    }

    *p2 = 0;

    // Output Value
    T3Output (Str2);
    T3Output (L"\n\r");
}

VOID
T3HexDump (
    IN VOID             *UserData,
    IN UINTN            DataSize
    )
{
    CHAR8           *Data, c;
    CHAR16          Val[50], Str[20];
    UINTN           Size, Index, Offset;


    Offset = 0;
    Data = UserData;
    while (DataSize) {
        Size = 16;
        if (Size > DataSize) {
            Size = DataSize;
        }

        for (Index=0; Index < Size; Index += 1) {
            c = Data[Index];
            Val[Index*3+0] = T3HexDigit[c>>4];
            Val[Index*3+1] = T3HexDigit[c&0xF];
            Val[Index*3+2] = (Index == 7) ? '-' : ' ';
            Str[Index] = (c < ' ' || c > 'z') ? '.' : c;
        }

        Str[Index] = 0;
        for (; Index < 16; Index += 1) {
            Val[Index*3+0] = ' ';
            Val[Index*3+1] = ' ';
            Val[Index*3+2] = (Index == 7) ? '-' : ' ';
        }
        Val[Index*3] = 0;
        
        // Output the line
        T3Output (L"  ");
        T3Output (Val);
        T3Output (L" *");
        T3Output (Str);
        T3Output (L"*\n\r");

        // Next
        Data += Size;
        Offset += Size;
        DataSize -= Size;
    }
}


UINTN
T3DevicePathSize (
    IN EFI_DEVICE_PATH  *DevPath
    )
{
    EFI_DEVICE_PATH     *Start;

    //
    // Search for the end of the device path structure
    //    

    Start = DevPath;
    while (!IsDevicePathEnd(DevPath)) {
        DevPath = NextDevicePathNode(DevPath);
    }

    //
    // Compute the size
    //

    return ((UINTN) DevPath - (UINTN) Start) + sizeof(EFI_DEVICE_PATH);
}



VOID
T3DumpDevicePath (
    IN EFI_DEVICE_PATH  *DevicePath
    )
{
    UINTN           Size;

    Size = T3DevicePathSize (DevicePath);
    T3HexDump (DevicePath, Size);
}

