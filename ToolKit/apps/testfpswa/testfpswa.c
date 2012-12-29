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

#define EFI_INTEL_FPSWA     \
    { 0xc41b6531, 0x97b9, 0x11d3, 0x9a, 0x29, 0x0, 0x90, 0x27, 0x3f, 0xc1, 0x4d }

typedef 
EFI_STATUS
(EFIAPI *EFI_FPSWA) (
    IN struct _FPSWA_INTERFACE  *This,
    IN UINTN                    TrapType,
    IN OUT VOID                 *Bundle,
    IN OUT UINT64               *pipsr,
    IN OUT UINT64               *pfsr,
    IN OUT UINT64               *pisr,
    IN OUT UINT64               *ppreds,
    IN OUT UINT64               *pifs,
    IN OUT VOID                 *fp_state
    );

typedef struct _FPSWA_INTERFACE {
    UINT32      Revision;
    UINT32      Reserved;

    EFI_FPSWA   Fpswa;    
} FPSWA_INTERFACE;

EFI_GUID FpswaId = EFI_INTEL_FPSWA;

UINTN MemoryBase;

BOOLEAN 
IsRealMemory(
    EFI_PHYSICAL_ADDRESS Pa
    );

VOID
WriteVGA(
    UINTN Row,
    UINTN Column,
    CHAR8 Data
    );

EFI_STATUS
InitializeTestVA_Application (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    )
{
    EFI_MEMORY_DESCRIPTOR       *MemMap, *MapEntry;
    UINTN                       MapEntries, MapKey, EntrySize;
    UINTN                       OriginalMapEntries;
    UINT32                      MapVersion;
    UINT64                      Va=0;
    EFI_STATUS                  Status;
    UINTN                       Index;
    UINTN                       j;
    EFI_TIME                    Time;
    EFI_PHYSICAL_ADDRESS        VirtualAddress[100];
    EFI_PHYSICAL_ADDRESS        PhysicalAddress[100];
    UINTN                       NoHandles;
    EFI_HANDLE                  *HandleBuffer;
    FPSWA_INTERFACE             *FpswaInterface;
    EFI_FPSWA                   LocalFpswa;    
    EFI_GET_TIME                LocalGetTime;
    CHAR16                      Buffer[256];

    //
    // Initialize the Library.
    //

    InitializeLib (ImageHandle, SystemTable);
    Print(L"Virtual Address Test application started\n");
    Print(L"  ST = %X  RT = %X  BS = %X\n",SystemTable,SystemTable->RuntimeServices,SystemTable->BootServices);

#ifdef EFI64 || EFIX64
    MemoryBase = 0x8000000000000000;
#else
    MemoryBase = 0;
#endif

    //
    //
    //

    NoHandles = 0;
    HandleBuffer = NULL;
    Status = LibLocateHandle(ByProtocol,
                             &FpswaId,
                             NULL,
                             &NoHandles,
                             &HandleBuffer
                             );

    if (NoHandles != 1) {
        Print(L"No FPSWA driver loaded\n");
        return EFI_NOT_FOUND;
    }

    BS->HandleProtocol(HandleBuffer[0], &FpswaId, (VOID*)&FpswaInterface);
    
    LocalFpswa = FpswaInterface->Fpswa;

    DEBUG((D_INIT, "TestVAApp: Call FPSWA (%X,%X)\n", FpswaInterface, FpswaInterface->Fpswa));
    Status = LocalFpswa(FpswaInterface,0,NULL,NULL,NULL,NULL,NULL,NULL,NULL);
    DEBUG((D_INIT, "  Return Value = %r\n",Status));

    //
    // Get the current memory map
    //

    MemMap = LibMemoryMap (&MapEntries, &MapKey, &EntrySize, &MapVersion);
    ASSERT (MemMap);
    ASSERT (MapVersion == EFI_MEMORY_DESCRIPTOR_VERSION);

    MapEntry = MemMap;
    for (Index=0; Index < MapEntries; Index++) {

        PhysicalAddress[Index] = MapEntry->PhysicalStart;

        if ((MapEntry->Attribute & EFI_MEMORY_RUNTIME)) {
            if ((MapEntry->Type == EfiRuntimeServicesData || MapEntry->Type == EfiRuntimeServicesCode) &&
                (IsRealMemory(MapEntry->PhysicalStart))) {

                Status = BS->AllocatePages(AllocateAnyPages, 
                                           EfiLoaderData, 
                                           (UINTN)MapEntry->NumberOfPages, 
                                           &(VirtualAddress[Index]));

            } else {
                VirtualAddress[Index] = MapEntry->PhysicalStart | 0x8000000000000000;
            }
        } else {
            VirtualAddress[Index] = 0;
        }

        Print(L"  PA : %016lx - %016lx  VA : %016lx\n",
              MapEntry->PhysicalStart,
              MapEntry->PhysicalStart + LShiftU64(MapEntry->NumberOfPages, PAGE_SHIFT) - 1,
              VirtualAddress[Index]);

        MapEntry = NextMemoryDescriptor(MapEntry, EntrySize);
    }    
    OriginalMapEntries = MapEntries;

    Print(L"%N\n");

    //
    // Get the current memory map
    //

    MemMap = LibMemoryMap (&MapEntries, &MapKey, &EntrySize, &MapVersion);
    ASSERT (MemMap);
    ASSERT (MapVersion == EFI_MEMORY_DESCRIPTOR_VERSION);

    //
    // Call ExitBootSevices();
    //

    Print(L"Call ExitBootServices()\n");

    Status = BS->ExitBootServices (ImageHandle, MapKey);
//    DEBUG((D_INIT, "  Return Value = %r\n",Status));
//    ASSERT (!EFI_ERROR(Status));

    //
    // Clean the top line of the display, so we can see the status of the test.
    //

    for(j=0;j<80;j++) {
        WriteVGA(0,j,' ');
        WriteVGA(1,j,' ');
        WriteVGA(2,j,' ');
    }

    WriteVGA(2,0,'A');

    //
    // Fill in all the virtual mappings
    //

//    DEBUG((D_INIT, "Fill in all virtual mappings\n"));

    MapEntry = MemMap;
    for (Index=0; Index < MapEntries; Index++) {

        MapEntry->VirtualStart = 0;
        for(j=0;j<OriginalMapEntries;j++) {
            if (PhysicalAddress[j] == MapEntry->PhysicalStart) {
                MapEntry->VirtualStart = VirtualAddress[j];

                if ((MapEntry->Attribute & EFI_MEMORY_RUNTIME) && 
                    (MapEntry->Type == EfiRuntimeServicesData || MapEntry->Type == EfiRuntimeServicesCode) &&
                    (IsRealMemory(MapEntry->PhysicalStart))) {

//                    DEBUG((D_INIT, "  PA : %X - %X  VA : %X\n",
//                                    MapEntry->PhysicalStart,
//                                    MapEntry->PhysicalStart + LShiftU64(MapEntry->NumberOfPages, PAGE_SHIFT) - 1,
//                                    MapEntry->VirtualStart));
                }
            }
        }

        MapEntry = NextMemoryDescriptor(MapEntry, EntrySize);
    }    

//    DEBUG((D_INIT, "Done\n"));

    WriteVGA(2,1,'B');

    //
    // Zero all BS_DATA and BS_CODE entries
    //

//    DEBUG((D_INIT, "Zero all BS_DATA and BS_CODE sections\n"));

    MapEntry = MemMap;
    for (Index=0; Index < MapEntries; Index++) {

       if ((MapEntry->Type == EfiBootServicesData || MapEntry->Type == EfiBootServicesCode) &&
           (IsRealMemory(MapEntry->PhysicalStart))) {

            ZeroMem((VOID *)(UINTN)MapEntry->PhysicalStart,
                    (UINTN)LShiftU64(MapEntry->NumberOfPages, PAGE_SHIFT));
        }

        MapEntry = NextMemoryDescriptor(MapEntry, EntrySize);
    }    

    WriteVGA(0,0,'A');

    //
    // Call SetVirtualAddressMap()
    //

    Status = RT->SetVirtualAddressMap (
                    MapEntries * EntrySize,
                    EntrySize,
                    MapVersion,
                    MemMap
                    );

    WriteVGA(0,1,'B');

    //
    // If SetVirtualAddressMap got an error, then Halt the sytem.
    //

    if (EFI_ERROR(Status)) {
        for(;;);
    }

    WriteVGA(0,2,'C');

    LocalFpswa   = FpswaInterface->Fpswa;
    LocalGetTime = RT->GetTime;

    WriteVGA(0,3,'D');

    //
    // Copy all the sections to their new virtual address and zero the physical address
    //

    MapEntry = MemMap;
    for (Index=0; Index < MapEntries; Index++) {

        if ((MapEntry->Attribute & EFI_MEMORY_RUNTIME) && 
            (MapEntry->Type == EfiRuntimeServicesData || MapEntry->Type == EfiRuntimeServicesCode) &&
            (IsRealMemory(MapEntry->PhysicalStart))) {
 
            CopyMem((VOID *)(UINTN)MapEntry->VirtualStart, 
                    (VOID *)(UINTN)MapEntry->PhysicalStart,  
                    (UINTN)LShiftU64(MapEntry->NumberOfPages, PAGE_SHIFT));

            ZeroMem((VOID *)(UINTN)MapEntry->PhysicalStart,
                    (UINTN)LShiftU64(MapEntry->NumberOfPages, PAGE_SHIFT));
        }

        MapEntry = NextMemoryDescriptor(MapEntry, EntrySize);
    }    

    WriteVGA(0,4,'E');

    //
    // After successfully switching to virtual mode, cannot call library functions
    // Hence use DEBUG function (with fake input flag),
    // which might output if stderr is available
    //

    if(!EFI_ERROR(Status)) {

        WriteVGA(0,5,'F');

        Status = LocalGetTime(&Time, NULL);

        TimeToString(Buffer,&Time);

        for(j=0;j<StrLen(Buffer);j++) {
            WriteVGA(1,j,(UINT8)Buffer[j]);
        }

        WriteVGA(0,6,'G');

        //
        // if GetTime() returned an error, then Halt the system.
        //

        if (EFI_ERROR(Status)) {
            for(;;);
        }

        WriteVGA(0,7,'H');
    }

    WriteVGA(0,8,'I');

    //
    // Make a call into the Runtime Driver
    //

    Status = LocalFpswa(FpswaInterface,0,NULL,NULL,NULL,NULL,NULL,NULL,NULL);

    WriteVGA(0,9,'J');

    //
    // Test complete.  Halt the system.
    //

    for(;;);

    return EFI_SUCCESS;
}

BOOLEAN 
IsRealMemory(
    EFI_PHYSICAL_ADDRESS Pa
    )

{
    if (Pa < 0xa0000) {
        return TRUE;
    }
    if (Pa >= 0x100000 && Pa < 0xff000000) {
        return TRUE;
    }
    if (Pa >= 0x100000000 && Pa < 0x400000000) {
        return TRUE;
    }
    return FALSE;
}

VOID
WriteVGA(
    UINTN Row,
    UINTN Column,
    CHAR8 Data
    )

{
#if EFI_NT_EMULATOR
   DEBUG((D_INIT, "%d %d %c\n",Row, Column, Data));
#else
   *(UINT8 *)(MemoryBase + 0xb8000 + Row * 160 + Column * 2) = Data;
#endif
}
