/*++

Copyright (c) 1998  Intel Corporation

Module Name:

    testva.c
    
Abstract:

    Tests virtual address call by making 1:1 transition

Revision History

--*/

#include "efi.h"
#include "efilib.h"

#define PAGE_SHIFT      12


EFI_STATUS
InitializeTestVA_Application (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    )
{
    EFI_MEMORY_DESCRIPTOR       *MemMap, *MapEntry;
    UINTN                       MapEntries, MapKey, EntrySize;
    UINT32                      MapVersion;
    UINT64                      Va=0;
    EFI_STATUS                  Status;
    UINTN                       Index;
    EFI_TIME                    Time;

    //
    // Initialize the Library.
    //

    InitializeLib (ImageHandle, SystemTable);
    Print(L"Virtual Address Test application started\n");

    //
    // Get the current memory map
    //

    MemMap = LibMemoryMap (&MapEntries, &MapKey, &EntrySize, &MapVersion);
    ASSERT (MemMap);
    ASSERT (MapVersion == EFI_MEMORY_DESCRIPTOR_VERSION);

    //
    // Build some fake mappings
    //

    Print(L"%HCreating 1:1 virtual address mapping\n");

    MapEntry = MemMap;
    for (Index=0; Index < MapEntries; Index++) {

        if (MapEntry->Attribute & EFI_MEMORY_RUNTIME) {
            MapEntry->VirtualStart = MapEntry->PhysicalStart;

            Print (L"%2x. %08lx-%08lx  va:%08lx\n", 
                Index,
                MapEntry->PhysicalStart,
                MapEntry->PhysicalStart + LShiftU64(MapEntry->NumberOfPages, PAGE_SHIFT) - 1,
                MapEntry->VirtualStart
                );
        }

        MapEntry = NextMemoryDescriptor(MapEntry, EntrySize);
    }    

    Print(L"%N\n");

    // Call with valid key
    Status = BS->ExitBootServices (ImageHandle, MapKey);
    ASSERT (!EFI_ERROR(Status));

    //
    Status = RT->SetVirtualAddressMap (
                    MapEntries * EntrySize,
                    EntrySize,
                    MapVersion,
                    MemMap
                    );

    
    DEBUG((D_INIT, "TestVAApp: Returned from SetVirtualAddressMap: %x\n", Status));

    ASSERT (!EFI_ERROR(Status));

    //
    // After successfully switching to virtual mode, cannot call library functions
    // Hence use DEBUG function (with fake input flag),
    // which might output if stderr is available
    //

    if(!EFI_ERROR(Status)) {
        DEBUG((D_ERROR, "Making a test virtual call after switching to virtual mode\n"));

        Status = RT->GetTime(&Time, NULL);

        if(!EFI_ERROR(Status)) {
            DEBUG((D_ERROR, "Current Time is %02d:%02d:%02d\n", Time.Hour,Time.Minute,Time.Second));
        }
    }

    DEBUG((D_ERROR, "Virtual Address Test application exiting...\n"));

    return EFI_SUCCESS;
}

