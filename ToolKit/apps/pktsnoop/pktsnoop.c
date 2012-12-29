/*++

Copyright (c) 1998  Intel Corporation

Module Name:

    test.c
    
Abstract:



Revision History

--*/

#include "efi.h"
#include "efilib.h"

UINT8 GlobalFrameBuffer[0x4000];

CHAR16 *StatisticName[] = {
	L"Total Packets Received        ",
	L"Good Packets Received         ",
	L"Undersized Packets Received   ",
	L"Oversized Packets Received    ",
	L"Dropped Packets Received      ",
	L"Unicase Packets Received      ",
	L"Broadcast Packets Received    ",
	L"Multicast Packets Received    ",
	L"CRC Error Packets Received    ",
	L"Total Bytes Received          ",

	L"Total Packets Transmitted     ",
	L"Good Packets Transmitted      ",
	L"Undersized Packets Transmitted",
	L"Oversized Packets Transmitted ",
	L"Dropped Packets Transmitted   ",
	L"Unicase Packets Transmitted   ",
	L"Broadcast Packets Transmitted ",
	L"Multicast Packets Transmitted ",
	L"CRC Error Packets Transmitted ",
	L"Total Bytes Transmitted       ",

	L"Csma Collisions               ",
	L"Unsupported Protocol          "
};

EFI_STATUS
InitializePacketSnooper (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    )
{
    EFI_STATUS             Status;
    EFI_DEVICE_PATH        *DevicePath;
    UINTN                  NoHandles;
    EFI_HANDLE             *HandleBuffer;
    EFI_SIMPLE_NETWORK     *SimpleNetwork;
    UINTN                  HeaderSize;
    UINTN                  BufferSize;
    UINTN                  PacketNumber;
    UINTN                  ErrorPackets;
    UINTN                  ReadAttempts;
    EFI_INPUT_KEY          Key;
    BOOLEAN                StopReceive;
    UINTN                  StatisticsSize;
    EFI_NETWORK_STATISTICS *StatisticsTable;
    UINTN                  Index;

    InitializeLib (ImageHandle, SystemTable);

    ST->ConOut->ClearScreen(ST->ConOut);

    Print(L"EFI Packet Snooper 1.0\n");

    Print(L"Get all device handles the support the SIMPLE_NETWORK protocol\n");

    NoHandles = 0;
    HandleBuffer = NULL;
    Status = LibLocateHandle(ByProtocol,
                             &SimpleNetworkProtocol,
                             NULL,
                             &NoHandles,
                             &HandleBuffer
                             );
    if (EFI_ERROR(Status)) {
        Print(L"Error finding device handles that support the SIMPLE NETWORK Protocol\n");
        BS->Exit(ImageHandle,EFI_SUCCESS,0,NULL);
    }
    if (NoHandles == 0) {
        Print(L"Can not find any device handles that support the SIMPLE NETWORK Protocol\n");
        BS->Exit(ImageHandle,EFI_SUCCESS,0,NULL);
    }

    //
    // Get the DEVICE_PATH Protocol Interface to the device
    //

    Print(L"Get the device path to handle 0\n");

    Status = BS->HandleProtocol (HandleBuffer[0], 
                                 &DevicePathProtocol, 
                                 (VOID*)&DevicePath
                                 );
    if (EFI_ERROR(Status)) {
        Print(L"Can not get the device path for the network device handle\n");
        BS->Exit(ImageHandle,EFI_SUCCESS,0,NULL);
    }

    //
    // Get the EFI_SIMPLE_NETWORK Protocol Interface to the device
    //

    Print(L"Get the SIMPLE_NETWORK interface to handle 0\n");

    Status = BS->HandleProtocol (HandleBuffer[0], 
                                 &SimpleNetworkProtocol, 
                                 (VOID*)&SimpleNetwork
                                 );
    if (EFI_ERROR(Status)) {
        Print(L"Can not get the EFI_SIMPLE_NETWORK protocol interface for the network device handle\n");
        BS->Exit(ImageHandle,EFI_SUCCESS,0,NULL);
    }

    Print(L"  Start Network Interface\n");

    if (SimpleNetwork->Mode->State == EfiSimpleNetworkStopped) {
        Status = SimpleNetwork->Start(SimpleNetwork);
        if (EFI_ERROR(Status)) {
            Print(L"Can not Start() the network device\n");
            BS->Exit(ImageHandle,EFI_SUCCESS,0,NULL);
        }
    }

    Print(L"  Initialize Network Interface\n");

    if (SimpleNetwork->Mode->State == EfiSimpleNetworkStarted) {
        Status = SimpleNetwork->Initialize(SimpleNetwork, 0, 0);
        if (EFI_ERROR(Status)) {
            Print(L"Can not Initialize() the network device\n");
            BS->Exit(ImageHandle,EFI_SUCCESS,0,NULL);
        }
    }

    if (SimpleNetwork->Mode->State != EfiSimpleNetworkInitialized) {
        Print(L"Can not Initialize() the network device\n");
        BS->Exit(ImageHandle,EFI_SUCCESS,0,NULL);
    }

    Print(L"  Put Network Interface in Promiscuous Mode\n");

    Status = SimpleNetwork->ReceiveFilters(SimpleNetwork, 0x1d, 0, FALSE, 0, NULL);
    if (EFI_ERROR(Status)) {
        Print(L"Can not ReceiveFilters() the network device\n");
        BS->Exit(ImageHandle,EFI_SUCCESS,0,NULL);
    }

    Print(L"  Reset all statistics counters\n");

    Status = SimpleNetwork->Statistics(
                 SimpleNetwork,
                 TRUE,
                 NULL,
                 NULL
                 );
    if (EFI_ERROR(Status)) {
        Print(L"Can not reset Statistics() on the network device\n");
        BS->Exit(ImageHandle,EFI_SUCCESS,0,NULL);
    }

    //
    //
    //

    ReadAttempts    = 0;
    PacketNumber    = 0;
    ErrorPackets    = 0;
    StatisticsSize  = 0;
    StatisticsTable = NULL;
    do {
        BufferSize = 0x4000;
        HeaderSize = 0;
        for(Index = 0;Index < BufferSize; Index++) {
            GlobalFrameBuffer[Index] = 0xEF;
        }

        Print(L"\n");
        Print(L"Wait for receive packet...");

        StopReceive = FALSE;
        do {

            //
            // Collect statistics every 1000 read attempts
            //

            if ((ReadAttempts % 0x100) == 0) {
                if (StatisticsTable != NULL) {
                    FreePool(StatisticsTable);
                }

                StatisticsSize  = 0;
                StatisticsTable = NULL;
                Status = SimpleNetwork->Statistics(
                             SimpleNetwork,
                             FALSE,
                             &StatisticsSize,
                             StatisticsTable
                             );

                if (Status == EFI_BUFFER_TOO_SMALL) {

                    StatisticsTable = AllocatePool(StatisticsSize);
                    Status = SimpleNetwork->Statistics(
                                 SimpleNetwork,
                                 FALSE,
                                 &StatisticsSize,
                                 StatisticsTable
                                 );

                    if (EFI_ERROR(Status)) {

                        Print(L"Error : Can not collect statistics : Status = %r\n",Status);
                        Print(L"  StatisticsSize = %08x  StatisticsTable = %08x\n",StatisticsSize,StatisticsTable);
                        Print(L"  Press a key to continue...\n");
                        while(ST->ConIn->ReadKeyStroke(ST->ConIn,&Key) != EFI_SUCCESS);

                        FreePool(StatisticsTable);
                        StatisticsSize  = 0;
                        StatisticsTable = NULL;
                    }
                }
            }

            Status = ST->ConIn->ReadKeyStroke(ST->ConIn,&Key);
            if (Status == EFI_SUCCESS && (Key.UnicodeChar == 32 || Key.UnicodeChar == 0x0d)) {
                StopReceive = TRUE;
            }

            Status = SimpleNetwork->Receive(
                         SimpleNetwork,
                         &HeaderSize,
                         &BufferSize,
                         GlobalFrameBuffer,
                         NULL,
                         NULL,
                         NULL
                         );

            ReadAttempts++;

        } while (Status == EFI_NOT_READY && StopReceive == FALSE);

        if (StopReceive == FALSE) {

            //
            // Print Header
            //

            ST->ConOut->ClearScreen(ST->ConOut);
            Print(L"EFI Packet Snooper 1.0\n");
            Print(L"  Press [SPACE] to pause or [ENTER] to exit\n\n");

            //
            // Print Statistics Table if available
            //

            if (StatisticsTable != NULL) {
                for(Index = 0;Index < (StatisticsSize / sizeof(UINT64)); Index++) {
                    Print(L"  %s : %08x\n",
                          StatisticName[Index],
                          *((UINT64 *)StatisticsTable +Index)
                          );
                }
            }

            //
            // Print Packet information
            //

            if (EFI_ERROR(Status)) {
                Print(L"PacketRx: #%08X Size=0x%X RA=%08x Err=0x%X ERROR(%r)",PacketNumber,BufferSize,ReadAttempts,ErrorPackets,Status);
                ErrorPackets++;
            } else {
                Print(L"PacketRx: #%08X Size=0x%X RA=%08x Err=0x%X\n",PacketNumber,BufferSize,ReadAttempts,ErrorPackets);
                if (BufferSize <= 0x80) {
                    DumpHex(0,0,BufferSize,GlobalFrameBuffer);
                } else {
                    DumpHex(0,0,0x80,GlobalFrameBuffer);
                }
                PacketNumber++;
            }
        }

        //
        // Check for a keystroke
        //

        if (StopReceive == TRUE) {
            Status = EFI_SUCCESS;
        } else {
            Status = ST->ConIn->ReadKeyStroke(ST->ConIn,&Key);
        }
        if (Status == EFI_SUCCESS && Key.UnicodeChar == 32) {

            //
            // If the user pressed the space bar, then pause until another key is pressed.
            //

            Print(L"\n");
            Print(L"Receive Paused.  Press a key to continue\n");

            while(ST->ConIn->ReadKeyStroke(ST->ConIn,&Key) != EFI_SUCCESS);
        }

        //
        // Exit the application is [ENTER] is pressed.
        //
        
    } while (!(Status == EFI_SUCCESS && Key.UnicodeChar == 0xD));

    Print(L"\n\n");
    /*
    Print(L"  Shutdown Network Interface\n");

    Status = SimpleNetwork->Shutdown(SimpleNetwork);
    if (EFI_ERROR(Status)) {
        Print(L"Can not Shutdown() the network device\n");
        BS->Exit(ImageHandle,EFI_SUCCESS,0,NULL);
    }

    Print(L"  Stop Network Interface\n");

    Status = SimpleNetwork->Stop(SimpleNetwork);
    if (EFI_ERROR(Status)) {
        Print(L"Can not Stop() the network device\n");
        BS->Exit(ImageHandle,EFI_SUCCESS,0,NULL);
    }
    */
    Print(L"  Done\n");

    return EFI_SUCCESS;
}
