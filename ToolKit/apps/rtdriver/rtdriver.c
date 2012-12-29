/*++

Copyright (c) 1998  Intel Corporation

Module Name:

    rtdriver.c
    
Abstract:

    Test runtime driver

Revision History

--*/

#include "efi.h"
#include "efilib.h"


//
//
//

EFI_STATUS
TestRtUnload (
    IN EFI_HANDLE       ImageHandle
    );

//
//
//

CHAR16  *RtTestString1 = L"This is string #1";
CHAR16  *RtTestString2 = L"This is string #2";
CHAR16  *RtTestString3 = L"This is string #3";
EFI_GUID RtTestDriverId = { 0xcc2ac9d1, 0x14a9, 0x11d3, 0x8e, 0x77, 0x0, 0xa0, 0xc9, 0x69, 0x72, 0x3b };


EFI_STATUS
InitializeTestRtDriver (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    )
{
    EFI_LOADED_IMAGE        *Image;
    EFI_STATUS              Status;

    //
    // Initialize the Library.
    //

    InitializeLib (ImageHandle, SystemTable);
    Print(L"Test RtDriver loaded\n");

    //
    // Add it an unload handler
    //

    Status = BS->HandleProtocol (ImageHandle, &LoadedImageProtocol, (VOID*)&Image);
    ASSERT (!EFI_ERROR(Status));
    Image->Unload = TestRtUnload;

    // 
    // Add a protocol so someone can locate us
    //

    Status = LibInstallProtocolInterfaces (&ImageHandle, &RtTestDriverId, NULL, NULL);
    ASSERT (!EFI_ERROR(Status));

    //
    // Modify one pointer to verify fixups don't reset it
    //

    Print(L"Address of RtTestString3 is %x\n", RtTestString3);
    Print(L"Address of RtTestString3 pointer is %x\n", &RtTestString3);
    RtTestString3 = RtTestString2;
    return EFI_SUCCESS;
}

EFI_STATUS
TestRtUnload (
    IN EFI_HANDLE       ImageHandle
    )
{
    DEBUG ((D_INIT, "Test RtDriver unload being requested\n"));
    LibUninstallProtocolInterfaces (ImageHandle, &RtTestDriverId, NULL, NULL);
    return EFI_SUCCESS;
}
