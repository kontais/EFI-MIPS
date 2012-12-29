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

#define EFI_TEST     \
    { 0x91ca4f3a, 0x903, 0x11d4, 0x9a, 0x36, 0x0, 0x90, 0x27, 0x3f, 0xc1, 0x4d }

EFI_GUID TestId = EFI_TEST;

UINTN MemoryBase;

CHAR16  *MemoryTypeDesc[EfiMaxMemoryType]  = {
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
IsRealMemory(
    EFI_PHYSICAL_ADDRESS Pa
    );

VOID
WriteVGAString(
    CHAR16 *String
    );

EFI_STATUS
InitializeTestVirtualRuntime (
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
    volatile UINTN                       Index;
    UINTN                       j;
    BOOLEAN                     Enabled;
    BOOLEAN                     Pending;
    EFI_TIME                    Time;
    EFI_PHYSICAL_ADDRESS        VirtualAddress[100];
    EFI_PHYSICAL_ADDRESS        PhysicalAddress[100];
    UINTN                       NoHandles;
    EFI_HANDLE                  *HandleBuffer;
    CHAR16                      PrintString[256];
    EFI_RUNTIME_SERVICES        LocalRuntimeServices;
    //EFI_DEVICE_IO_INTERFACE     *DeviceIo;
    UINTN                       Seconds;
#ifdef EFI64
    EFI_PHYSICAL_ADDRESS        SalDataArea;
#endif

    CHAR16                      VariableName[256];
    UINT8                       VariableValue[1024];
    EFI_GUID                    VendorGuid;
    UINTN                       VariableNameSize;
    UINTN                       VariableValueSize;
    UINT32                      VariableAttributes;

    //
    // Initialize the Library.
    //

    InitializeLib (ImageHandle, SystemTable);

    Print(L"Virtual Runtime Test Application Started\n");
    Print(L"  ST = %X  RT = %X  BS = %X  Stack=%X\n",SystemTable,SystemTable->RuntimeServices,SystemTable->BootServices,&ImageHandle);

#if defined(EFI64) || defined(EFIX64)
    MemoryBase = 0x8000000000000000;
#else
    MemoryBase = 0;
#endif

    //
    // Get the current memory map
    //

    MemMap = LibMemoryMap (&MapEntries, &MapKey, &EntrySize, &MapVersion);
    ASSERT (MemMap);
    ASSERT (MapVersion == EFI_MEMORY_DESCRIPTOR_VERSION);

    DEBUG((D_INIT,"  MemMap = %X\n",MemMap));

#ifdef EFI64
    SalDataArea = 0;
#endif
    MapEntry = MemMap;
    for (Index=0; Index < MapEntries; Index++) {

        PhysicalAddress[Index] = MapEntry->PhysicalStart;

        if ((MapEntry->Attribute & EFI_MEMORY_RUNTIME)) {
            if ((MapEntry->Type == EfiRuntimeServicesData || MapEntry->Type == EfiRuntimeServicesCode) &&
                (IsRealMemory(MapEntry->PhysicalStart))) {

#ifdef EFI64
                //
                // Find the SAL Data Area
                //

                if (MapEntry->PhysicalStart > SalDataArea) {
                    SalDataArea = MapEntry->PhysicalStart + LShiftU64(MapEntry->NumberOfPages, PAGE_SHIFT) - 0x10000;
                }
#endif

                //
                // Allocate memory for new virtual location of runtime sections.
                //

                Status = BS->AllocatePages(AllocateAnyPages, 
                                           EfiLoaderData, 
                                           (UINTN)MapEntry->NumberOfPages, 
                                           &(VirtualAddress[Index]));

            } else {

                //
                // Set bit 63 of the Virtual Address
                //

                VirtualAddress[Index] = MapEntry->PhysicalStart | 0x8000000000000000;
            }
        } else {

            //
            // No virtual mapping required
            //

            VirtualAddress[Index] = 0;
        }

        DEBUG((D_INIT,"  %s PA:%016lx - %016lx  VA:%016lx\n",
              MemoryTypeDesc[MapEntry->Type],
              MapEntry->PhysicalStart,
              MapEntry->PhysicalStart + LShiftU64(MapEntry->NumberOfPages, PAGE_SHIFT) - 1,
              VirtualAddress[Index]));

        MapEntry = NextMemoryDescriptor(MapEntry, EntrySize);
    }    
    OriginalMapEntries = MapEntries;

    DEBUG((D_INIT,"%N\n"));

    //
    // Get the current memory map
    //

    MemMap = LibMemoryMap (&MapEntries, &MapKey, &EntrySize, &MapVersion);
    ASSERT (MemMap);
    ASSERT (MapVersion == EFI_MEMORY_DESCRIPTOR_VERSION);

    //
    // Display the current memory map.
    //

    DEBUG((D_INIT,"  MemMap = %X\n",MemMap));
    MapEntry = MemMap;
    for (Index=0; Index < MapEntries; Index++) {
        DEBUG((D_INIT,"  %s PA:%016lx - %016lx  VA:%016lx\n",
              MemoryTypeDesc[MapEntry->Type],
              MapEntry->PhysicalStart,
              MapEntry->PhysicalStart + LShiftU64(MapEntry->NumberOfPages, PAGE_SHIFT) - 1,
              VirtualAddress[Index]));

        MapEntry = NextMemoryDescriptor(MapEntry, EntrySize);
    }    

    //
    // Get the global Device I/O Protocol instance so we can turn off PIC interrupts.
    //

    NoHandles = 0;
    HandleBuffer = NULL;
    Status = LibLocateHandle(ByProtocol,
                             &DeviceIoProtocol,
                             NULL,
                             &NoHandles,
                             &HandleBuffer
                             );
    if (EFI_ERROR(Status)) {
        Print(L"Can not get the array of DEVICE_IO handles\n");
//        BS->Exit(ImageHandle,EFI_SUCCESS,0,NULL);
    }

//    Status = BS->HandleProtocol (HandleBuffer[0], 
//                                 &DeviceIoProtocol, 
//                                 (VOID*)&DeviceIo
//                                 );

    if (EFI_ERROR(Status)) {
        Print(L"Can not get the DEVICE_IO interface\n");
//        BS->Exit(ImageHandle,EFI_SUCCESS,0,NULL);
    }

    //
    // Disable PIC interrupts
    //

//    WritePort(DeviceIo, IO_UINT8, 0x21, 0xff);
//    WritePort(DeviceIo, IO_UINT8, 0xA1, 0xff);

    //
    // Call ExitBootSevices();
    //
    WriteVGAString(L"Call GetMemoryMap\n");

    MemMap = LibMemoryMap (&MapEntries, &MapKey, &EntrySize, &MapVersion);
    ASSERT (MemMap);
    ASSERT (MapVersion == EFI_MEMORY_DESCRIPTOR_VERSION);

    WriteVGAString(L"Call ExitBootServices\n");

    Status = BS->ExitBootServices (ImageHandle, MapKey);

    WriteVGAString(L"  Done\n");
    SPrint(PrintString,sizeof(PrintString),L"  Return Value = %r\n",Status);
    WriteVGAString(PrintString);

    if (EFI_ERROR(Status)) {
        for(;;);
    }

    //
    // Fill in all the virtual mappings
    //

    WriteVGAString(L"Fill in all virtual mappings\n");

    MapEntry = MemMap;
    for (Index=0; Index < MapEntries; Index++) {
        MapEntry->VirtualStart = 0;
        for(j=0;j<OriginalMapEntries;j++) {
            if (PhysicalAddress[j] == MapEntry->PhysicalStart) {
                MapEntry->VirtualStart = VirtualAddress[j];
            }
        }
        MapEntry = NextMemoryDescriptor(MapEntry, EntrySize);
    }    

    WriteVGAString(L"  Done\n");

    //
    // Zero all BS_DATA and BS_CODE entries
    //

    WriteVGAString(L"Zero all BS_DATA and BS_CODE sections\n");

    MapEntry = MemMap;
    for (Index=0; Index < MapEntries; Index++) {

       if ((MapEntry->Type == EfiBootServicesData || MapEntry->Type == EfiBootServicesCode) &&
           (IsRealMemory(MapEntry->PhysicalStart))) {

           //
           // Do not zero the memory descriptor for the stack.
           // The variable "Index" is on this application's stack.
           //

           if (( (UINTN)(&Index) < (UINTN)MapEntry->PhysicalStart ) ||
               ( (UINTN)(&Index) > (UINTN)(MapEntry->PhysicalStart + LShiftU64(MapEntry->NumberOfPages, PAGE_SHIFT)) ) ) { 

               SPrint(PrintString,
                      sizeof(PrintString),
                      L"  ZeroMem(%016lx,%016lx)\n",
                      (UINTN)MapEntry->PhysicalStart,
                      (UINTN)LShiftU64(MapEntry->NumberOfPages, PAGE_SHIFT)
                      );
               WriteVGAString(PrintString);

               ZeroMem((VOID *)(UINTN)MapEntry->PhysicalStart,
                       (UINTN)LShiftU64(MapEntry->NumberOfPages, PAGE_SHIFT));
           }
        }

        MapEntry = NextMemoryDescriptor(MapEntry, EntrySize);
    }    

    WriteVGAString(L"  Done\n");
for (Index = 0; Index < 1000000000; Index++);
    //
    // Call SetVirtualAddressMap()
    //

    WriteVGAString(L"Call SetVirtualAddressMap\n");

    Status = RT->SetVirtualAddressMap (
                    MapEntries * EntrySize,
                    EntrySize,
                    MapVersion,
                    MemMap
                    );

    WriteVGAString(L"  Done\n");
    SPrint(PrintString,sizeof(PrintString),L"  Return Value = %r\n",Status);
    WriteVGAString(PrintString);

    //
    // If SetVirtualAddressMap got an error, then Halt the sytem.
    //

    if (EFI_ERROR(Status)) {
        for(;;);
    }

    //
    // Cache the Runtime entry points before runtime code and data are moved.
    //

    WriteVGAString(L"Cache Runtime Services Table\n");

    CopyMem(&LocalRuntimeServices,RT,sizeof(EFI_RUNTIME_SERVICES));

    WriteVGAString(L"  Done\n");

    //
    // Copy all the sections to their new virtual address and zero the physical address
    //

    WriteVGAString(L"Copy all sections to virtual address and zero physical address\n");

    MapEntry = MemMap;
    for (Index=0; Index < MapEntries; Index++) {

        if ((MapEntry->Attribute & EFI_MEMORY_RUNTIME) && 
            (MapEntry->Type == EfiRuntimeServicesData || MapEntry->Type == EfiRuntimeServicesCode) &&
            (IsRealMemory(MapEntry->PhysicalStart))) {

            //
            // Copy code and data from their physical address to the new virtual address
            //

            CopyMem((VOID *)(UINTN)MapEntry->VirtualStart, 
                    (VOID *)(UINTN)MapEntry->PhysicalStart,  
                    (UINTN)LShiftU64(MapEntry->NumberOfPages, PAGE_SHIFT));

            //
            // Zero the code and data at the physical address
            //

#ifdef EFI64
            //
            // Do not Zero the SAL Data Area.  It is at Top of Memory - 64K.
            //

            if (( (UINTN)(SalDataArea) < (UINTN)MapEntry->PhysicalStart ) ||
                ( (UINTN)(SalDataArea) > (UINTN)(MapEntry->PhysicalStart + LShiftU64(MapEntry->NumberOfPages, PAGE_SHIFT)) ) ) { 
#endif

                SPrint(PrintString,
                       sizeof(PrintString),
                       L"  ZeroMem(%016lx,%016lx)\n",
                       (UINTN)MapEntry->PhysicalStart,
                       (UINTN)LShiftU64(MapEntry->NumberOfPages, PAGE_SHIFT)
                       );
                WriteVGAString(PrintString);

                ZeroMem((VOID *)(UINTN)MapEntry->PhysicalStart,
                        (UINTN)LShiftU64(MapEntry->NumberOfPages, PAGE_SHIFT));
#ifdef EFI64
           }
#endif
        }

        MapEntry = NextMemoryDescriptor(MapEntry, EntrySize);
    }    

    WriteVGAString(L"  Done\n");
for (Index = 0; Index < 1000000000; Index++);


    //
    // Test GetTime()
    //

    WriteVGAString(L"Call GetTime()...");

    Status = LocalRuntimeServices.GetTime(&Time, NULL);

    SPrint(PrintString,sizeof(PrintString),L"%r\n",Status);
    WriteVGAString(PrintString);

    if (!EFI_ERROR(Status)) {
        SPrint(PrintString,sizeof(PrintString),L"Time and Date = %t\n",&Time);
        WriteVGAString(PrintString);
    }

    //
    // Test GetWakeupTime()
    //

    WriteVGAString(L"Call GetWakeupTime()...");

    Status = LocalRuntimeServices.GetWakeupTime(&Enabled, &Pending, &Time);

    SPrint(PrintString,sizeof(PrintString),L"%r\n",Status);
    WriteVGAString(PrintString);

    if (!EFI_ERROR(Status)) {
        SPrint(PrintString,sizeof(PrintString),L"Wakeup Time and Date = %t %d %d\n",&Time,Enabled,Pending);
        WriteVGAString(PrintString);
    }

    //
    // Set (TestVariable1,TestId) to "foo"
    //

    WriteVGAString(L"Call SetVariable(TestVariable1,TestId,foo)...");

    StrCpy(VariableName,L"TestVariable1");
    VendorGuid = TestId;
    StrCpy((CHAR16 *)VariableValue,L"foo");
    Status = LocalRuntimeServices.SetVariable(VariableName,
                                              &VendorGuid,
                                              EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
                                              StrSize((CHAR16 *)VariableValue),
                                              VariableValue
                                              );

    SPrint(PrintString,sizeof(PrintString),L"%r\n",Status);
    WriteVGAString(PrintString);

    //
    // Set (TestVariable2,TestId) to nothing
    //

    WriteVGAString(L"Call SetVariable(TestVariable2,TestId,DELETE)...");

    StrCpy(VariableName,L"TestVariable2");
    VendorGuid = TestId;
    StrCpy((CHAR16 *)VariableValue,L"foo");
    Status = LocalRuntimeServices.SetVariable(VariableName,
                                              &VendorGuid,
                                              0,
                                              StrSize((CHAR16 *)VariableValue),
                                              VariableValue
                                              );

    SPrint(PrintString,sizeof(PrintString),L"%r\n",Status);
    WriteVGAString(PrintString);

    //
    // Set (TestVariable3,TestId) to nothing
    //

    WriteVGAString(L"Call SetVariable(TestVariable3,TestId,DELETE)...");

    StrCpy(VariableName,L"TestVariable3");
    VendorGuid = TestId;
    StrCpy((CHAR16 *)VariableValue,L"foo");
    Status = LocalRuntimeServices.SetVariable(VariableName,
                                              &VendorGuid,
                                              EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
                                              0,
                                              VariableValue
                                              );

    SPrint(PrintString,sizeof(PrintString),L"%r\n",Status);
    WriteVGAString(PrintString);

    //
    // Set (TestVariable2,TestId) to "foo"
    //

    WriteVGAString(L"Call SetVariable(TestVariable2,TestId,foo)...");

    StrCpy(VariableName,L"TestVariable2");
    VendorGuid = TestId;
    StrCpy((CHAR16 *)VariableValue,L"foo");
    Status = LocalRuntimeServices.SetVariable(VariableName,
                                              &VendorGuid,
                                              EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
                                              StrSize((CHAR16 *)VariableValue),
                                              VariableValue
                                              );

    SPrint(PrintString,sizeof(PrintString),L"%r\n",Status);
    WriteVGAString(PrintString);

    //
    // Set (TestVariable3,TestId) to "foo"
    //

    WriteVGAString(L"Call SetVariable(TestVariable3,TestId,foo)...");

    StrCpy(VariableName,L"TestVariable3");
    VendorGuid = TestId;
    StrCpy((CHAR16 *)VariableValue,L"foo");
    Status = LocalRuntimeServices.SetVariable(VariableName,
                                              &VendorGuid,
                                              EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
                                              StrSize((CHAR16 *)VariableValue),
                                              VariableValue
                                              );

    SPrint(PrintString,sizeof(PrintString),L"%r\n",Status);
    WriteVGAString(PrintString);

    //
    // Set (TestVariable2,TestId) to nothing
    //

    WriteVGAString(L"Call SetVariable(TestVariable2,TestId,DELETE)...");

    StrCpy(VariableName,L"TestVariable2");
    VendorGuid = TestId;
    StrCpy((CHAR16 *)VariableValue,L"foo");
    Status = LocalRuntimeServices.SetVariable(VariableName,
                                              &VendorGuid,
                                              0,
                                              StrSize((CHAR16 *)VariableValue),
                                              VariableValue
                                              );

    SPrint(PrintString,sizeof(PrintString),L"%r\n",Status);
    WriteVGAString(PrintString);

    //
    // Set (TestVariable3,TestId) to nothing
    //

    WriteVGAString(L"Call SetVariable(TestVariable3,TestId,DELETE)...");

    StrCpy(VariableName,L"TestVariable3");
    VendorGuid = TestId;
    StrCpy((CHAR16 *)VariableValue,L"foo");
    Status = LocalRuntimeServices.SetVariable(VariableName,
                                              &VendorGuid,
                                              EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
                                              0,
                                              VariableValue
                                              );

    SPrint(PrintString,sizeof(PrintString),L"%r\n",Status);
    WriteVGAString(PrintString);

    //
    // Display all environment variables:
    //

    WriteVGAString(L"Environment Variable List:\n");

    VariableName[0] = 0x0000;
    VendorGuid = NullGuid;
    WriteVGAString(L"GUID                                Variable Name        Value\n");
    WriteVGAString(L"=================================== ==================== ========\n");
    do {

        WriteVGAString(L"Call GetNextVariableName()...");

        VariableNameSize = 256;
        Status = LocalRuntimeServices.GetNextVariableName(&VariableNameSize,
                                                          VariableName,
                                                          &VendorGuid
                                                          );

        SPrint(PrintString,sizeof(PrintString),L"%r\n",Status);
        WriteVGAString(PrintString);

        if (Status == EFI_SUCCESS) {

            SPrint(PrintString,sizeof(PrintString),L"Variable Name = %s\n",VariableName);
            WriteVGAString(PrintString);

            WriteVGAString(L"Call GetVariable()...");

            VariableValueSize = sizeof(VariableValue);
            Status = LocalRuntimeServices.GetVariable(VariableName,
                                                      &VendorGuid,
                                                      &VariableAttributes,
                                                      &VariableValueSize,
                                                      VariableValue
                                                      );

            SPrint(PrintString,sizeof(PrintString),L"%r\n",Status);
            WriteVGAString(PrintString);

            if (Status == EFI_SUCCESS) {
                SPrint(PrintString,sizeof(PrintString),L"Attribute = %04x\n",VariableAttributes);
                WriteVGAString(PrintString);
                SPrint(PrintString,sizeof(PrintString),L"DataSize  = %04x\n",VariableValueSize);
                WriteVGAString(PrintString);
                WriteVGAString(L"Data : \n");
                for(Index = 0; Index < VariableValueSize; Index++) {
                    SPrint(PrintString,sizeof(PrintString),L"%02x ",VariableValue[Index]);
                    WriteVGAString(PrintString);
                    if ((Index & 0x0f) == 0x0f) {
                        WriteVGAString(L"\n");
                    }
                }
                WriteVGAString(L"\n");
            }
        }
    } while (Status == EFI_SUCCESS);

    //
    // Test complete.  Reset the system.in 10 seconds
    //

    WriteVGAString(L"Reset system in ");

    Status = LocalRuntimeServices.GetTime(&Time, NULL);
    Seconds = Time.Second;

    for(Index = 10;Index != 0;Index --) {
        SPrint(PrintString,sizeof(PrintString),L"%d ",Index);
        WriteVGAString(PrintString);
        do {
            Status = LocalRuntimeServices.GetTime(&Time, NULL);
        } while (Time.Second == Seconds);
        Seconds = Time.Second;
    }

    //
    // Reset the system
    //

    WriteVGAString(L"ResetSystem()...");

#if defined(EFI_APP_102)
     Status = LocalRuntimeServices.ResetSystem(EfiResetWarm, EFI_SUCCESS, 0, NULL);

    SPrint(PrintString,sizeof(PrintString),L"%r\n",Status);
    WriteVGAString(PrintString);
#else	
    LocalRuntimeServices.ResetSystem(EfiResetWarm, EFI_SUCCESS, 0, NULL);
#endif
    //
    // Reset fails, so Halt system.
    //

    for(;;);

    return EFI_SUCCESS;
}

BOOLEAN 
IsRealMemory(
    EFI_PHYSICAL_ADDRESS Pa
    )

{
#ifdef EFI32
    if (Pa >= 0x20000 && Pa <= 0x22000) {
        return FALSE;
    }
#endif
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
//   DEBUG((D_ERROR, "%d %d %c\n",Row, Column, Data));
#else
//   *(UINT8 *)(MemoryBase + 0xb8000 + Row * 160 + Column * 2) = Data;
   CHAR16 String[2];

   String[0] = Data;
   String[1] = 0;
   WriteVGAString(String);
#endif
}

UINTN CursorRow = 0;
UINTN CursorColumn = 0;

VOID
WriteVGAString(
    CHAR16 *String
    )

{
#if EFI_NT_EMULATOR
//   DEBUG((D_ERROR, "%s\n", String));
#else
    volatile UINTN Index;
    UINTN j;

    for(Index = 0; String[Index] != 0; Index++) {
        switch (String[Index]) {
        case '\n' :
            for(; CursorColumn < 80; CursorColumn++) {
                *(UINT8 *)(MemoryBase + 0xb8000 + CursorRow * 160 + CursorColumn * 2)     = ' ';
                *(UINT8 *)(MemoryBase + 0xb8000 + CursorRow * 160 + CursorColumn * 2 + 1) = 0x0f;
            }
            break;
        case '\r':
            break;
        default:
            *(UINT8 *)(MemoryBase + 0xb8000 + CursorRow * 160 + CursorColumn * 2)     = (UINT8)(String[Index]);
            *(UINT8 *)(MemoryBase + 0xb8000 + CursorRow * 160 + CursorColumn * 2 + 1) = 0x0f;
            CursorColumn++;
            break;
        }
        if (CursorColumn >= 80) {
            CursorColumn = 0;
            CursorRow++;
        }
        if (CursorRow >= 25) {
            for (j = 0; j < (80*24*2); j++) {
                *(INT8 *)(MemoryBase + 0xb8000 + j) = *(UINT8 *)(MemoryBase + 0xb8000 + j + 160);
            }
            CursorRow    = 24;
            for(; CursorColumn < 80; CursorColumn++) {
                *(UINT8 *)(MemoryBase + 0xb8000 + CursorRow * 160 + CursorColumn * 2)     = ' ';
                *(UINT8 *)(MemoryBase + 0xb8000 + CursorRow * 160 + CursorColumn * 2 + 1) = 0x0f;
            }
            CursorColumn = 0;
        }
    }
#endif
}
