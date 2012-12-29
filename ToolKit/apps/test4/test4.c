/*++

Copyright (c) 1998  Intel Corporation

Module Name:

    test4.c
    
Abstract:

    Simple test to watch a timer

Revision History

--*/

#include "efi.h"
#include "efilib.h"

struct {
    UINT64      Period;
    EFI_EVENT   Event;
} T4Timers[] = {
   10000000,    NULL,           // first one must be 1 second
    4999999,    NULL,
    2222222,    NULL,
    1111111,    NULL,
     123456,    NULL,
   10000001,    NULL,
   20000000,    NULL,
   33333333,    NULL,
    7777777,    NULL,
    0,          NULL
};

EFI_STATUS
InitializeTest4Application (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    );

EFI_DRIVER_ENTRY_POINT (InitializeTest4Application)

EFI_STATUS
InitializeTest4Application (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    )
{
    EFI_EVENT               WaitList[10];
    UINTN                   NoWait, WaitIndex;
    EFI_STATUS              Status;
    UINTN                   Seconds;
    EFI_INPUT_KEY           Key;
    UINTN                   Index;

    //
    // Initialize the Library.
    //

    InitializeLib (ImageHandle, SystemTable);

    Print(L"Test4 application started\n");


    //
    // Allocate a bunch of timers
    //

    for (Index=0; T4Timers[Index].Period; Index++) {
        Status = BS->CreateEvent(EVT_TIMER, 0, NULL, NULL, &T4Timers[Index].Event);
        ASSERT (!EFI_ERROR(Status));

        //
        // Set timer to automatically fire every second
        //

        Status = BS->SetTimer(T4Timers[Index].Event, TimerPeriodic, T4Timers[Index].Period);
        ASSERT (!EFI_ERROR(Status));
    }

    //
    // Set up a wait list for a key and the timer
    //

    NoWait = 0;
    WaitList[NoWait++] = T4Timers[0].Event;
    WaitList[NoWait++] = ST->ConIn->WaitForKey;

    //
    // Wait for either
    //

    Seconds = 0;
    while (NoWait) {
        Print (L"\rElasped: %,d.  Waiting for a key (or 1 second)  ", Seconds);

        Status = BS->WaitForEvent(NoWait, WaitList, &WaitIndex);
        ASSERT (!EFI_ERROR(Status));

        switch (WaitIndex) {
        case 0:
            Seconds += 1;
            break;

        case 1:
            Print (L"\n");

            // Read the key
            Status = ST->ConIn->ReadKeyStroke(ST->ConIn, &Key);
            if (Key.UnicodeChar != 0xD) {
                Print (L"key was pressed %x (scan code %x) - but not enter\n", Key.UnicodeChar, Key.ScanCode);
            } else {
                Print (L"Enter was pressed - terminating app\n");
                NoWait = 0;     // stop loop
            }
            break;

        default:
            // bad WaitIndex response
            ASSERT (FALSE);
        }
    }

    //
    // Done
    //

    for (Index=0; T4Timers[Index].Period; Index++) {
        BS->CloseEvent(T4Timers[Index].Event);
    }

    return EFI_SUCCESS;
}
