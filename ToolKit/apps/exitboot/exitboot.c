/*++

Copyright (c) 1998  Intel Corporation

Module Name:

    extboot.c
    
Abstract:

    Call ExitBootServices

Revision History

--*/

#include "efi.h"
#include "efilib.h"

#define PAGE_SHIFT      12


EFI_STATUS
InitializeExitBootTestApplication (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    )
{
    EFI_MEMORY_DESCRIPTOR       *MemMap, *MapEntry;
    UINTN                       MapEntries, MapKey, EntrySize;
    UINT32                      MapVersion;
    UINT64                      Va;
    EFI_STATUS                  Status;
    UINTN                       Index;

    //
    // Initialize the Library.
    //

    InitializeLib (ImageHandle, SystemTable);
    Print(L"ExitBootServices Test application started\n");

    //
    // Get the current memory map
    //

    MemMap = LibMemoryMap (&MapEntries, &MapKey, &EntrySize, &MapVersion);
    ASSERT (MemMap);
    ASSERT (MapVersion == EFI_MEMORY_DESCRIPTOR_VERSION);

    //
    // Build some fake mappings
    //

    Print(L"%HBogus virtual mapping table\n");

    Va = (0x80000000 >> PAGE_SHIFT);
    MapEntry = MemMap;
    for (Index=0; Index < MapEntries; Index++) {

        if (MapEntry->Attribute & EFI_MEMORY_RUNTIME) {
            MapEntry->VirtualStart = LShiftU64(Va, PAGE_SHIFT);
            Va = Va + MapEntry->NumberOfPages + 1;

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

    //
    // We can print here only on nt_emulate because the base FW address map
    // really didn't change via the above call
    //

    DEBUG((D_INIT, "ExitBootTestApp: Returned from SetVirtualAddressMap: %x\n", Status));
    ASSERT (!EFI_ERROR(Status));

    return EFI_SUCCESS;
}
