/*++

Copyright (c) 1998  Intel Corporation

Module Name:

    rtunload.c
    
Abstract:

    Call ExitBootServices

Revision History

--*/

#include "efi.h"
#include "efilib.h"


EFI_GUID RtTestDriverId = { 0xcc2ac9d1, 0x14a9, 0x11d3, 0x8e, 0x77, 0x0, 0xa0, 0xc9, 0x69, 0x72, 0x3b };

EFI_STATUS
InitializeTestUnloadApplication (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    )
{
    EFI_STATUS              Status;
    EFI_HANDLE              Handle;
    UINTN                   BufferSize;

    //
    // Initialize the Library.
    //

    InitializeLib (ImageHandle, SystemTable);


    //
    // Find the Rt test driver
    //

    BufferSize = sizeof(Handle);
    Status = BS->LocateHandle (
                ByProtocol,
                &RtTestDriverId,
                NULL,
                &BufferSize,
                &Handle
                );

    if (!EFI_ERROR(Status)) {
        //
        // Rt test driver assigns that protocol on it's image handle, ask it to unload
        //

        Status = BS->UnloadImage(Handle);
        if (EFI_ERROR(Status)) {
            Print (L"RtUnload: image did not unload: %r\n", Status);
        } else {
            Print (L"RtUnload: image unloaded\n");
        }

    } else {
        Print (L"RtUnload: did not locate runtime test driver: %r\n", Status);
    }

    return EFI_SUCCESS;
}
