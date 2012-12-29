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
EFI_MAC_ADDRESS NewMacAddress = {0,1,2,3,4,5,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};


VOID
DisplayModeInfo(
    EFI_SIMPLE_NETWORK *SimpleNetwork
    )

{
    UINTN Index;
    UINTN j;

    //
    // Print Mode information
    //
    
    DEBUG((D_ERROR,"Mode->State                = %d\n",SimpleNetwork->Mode->State));
    DEBUG((D_ERROR,"Mode->HwAddressSize        = %d\n",SimpleNetwork->Mode->HwAddressSize));
    DEBUG((D_ERROR,"Mode->MacAddressChangeable = %d\n",SimpleNetwork->Mode->MacAddressChangeable));
    DEBUG((D_ERROR,"Mode->MultiplTxSupported   = %d\n",SimpleNetwork->Mode->MultipleTxSupported));
    DEBUG((D_ERROR,"Mode->NvRamSize            = %d\n",SimpleNetwork->Mode->NvRamSize));
    DEBUG((D_ERROR,"Mode->NvRamAccessSize      = %d\n",SimpleNetwork->Mode->NvRamAccessSize));
    DEBUG((D_ERROR,"Mode->ReceiveFilterSetting = %d\n",SimpleNetwork->Mode->ReceiveFilterSetting));
    DEBUG((D_ERROR,"Mode->IfType               = %d\n",SimpleNetwork->Mode->IfType));
    DEBUG((D_ERROR,"Mode->MCastFilterCount     = %d\n",SimpleNetwork->Mode->MCastFilterCount));
    for (Index = 0; Index < SimpleNetwork->Mode->MCastFilterCount; Index++) {
        DEBUG((D_ERROR,"  Filter[%02d] = ",Index));
        for(j=0;j<16;j++) {
            DEBUG((D_ERROR,"%02x ",SimpleNetwork->Mode->MCastFilter[Index].Addr[j]));
        }
        DEBUG((D_ERROR,"\n"));
    }

    DEBUG((D_ERROR,"CurrentAddress = "));
    for(j=0;j<16;j++) {
        DEBUG((D_ERROR,"%02x ",SimpleNetwork->Mode->CurrentAddress.Addr[j]));
    }
    DEBUG((D_ERROR,"\n"));

    DEBUG((D_ERROR,"BroadcastAddress = "));
    for(j=0;j<16;j++) {
        DEBUG((D_ERROR,"%02x ",SimpleNetwork->Mode->BroadcastAddress.Addr[j]));
    }
    DEBUG((D_ERROR,"\n"));

    DEBUG((D_ERROR,"PermanentAddress = "));
    for(j=0;j<16;j++) {
        DEBUG((D_ERROR,"%02x ",SimpleNetwork->Mode->PermanentAddress.Addr[j]));
    }
    DEBUG((D_ERROR,"\n"));
}

EFI_STATUS
InitializePacketTransmitter (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    )
{
    EFI_STATUS         Status;
    EFI_DEVICE_PATH    *DevicePath;
    UINTN              NoHandles;
    EFI_HANDLE         *HandleBuffer;
    EFI_SIMPLE_NETWORK *SimpleNetwork;
    UINTN              HeaderSize;
    UINTN              BufferSize;
    UINTN              PacketNumber;
    UINTN              ErrorPackets;
    EFI_INPUT_KEY      Key;
    UINTN              Index;
    BOOLEAN            StopTransmit;
    VOID               *TxBuf;

    InitializeLib (ImageHandle, SystemTable);

    ST->ConOut->ClearScreen(ST->ConOut);

    Print(L"EFI Packet Transmitter 1.0\n");

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

    Print(L"  Put Network Interface in Receive no packets\n");

  //  Status = SimpleNetwork->ReceiveFilters(SimpleNetwork, 0x00, 0, FALSE, 0, NULL);
  //  if (EFI_ERROR(Status)) {
  //      Print(L"Can not ReceiveFilters() the network device\n");
  //      BS->Exit(ImageHandle,EFI_SUCCESS,0,NULL);
  //  }

    PacketNumber = 0;
    ErrorPackets = 0;
    do {
        HeaderSize = 0;
        BufferSize = 14 + (PacketNumber % 1500);
        HeaderSize = 0;
        for (Index = 0; Index < BufferSize ; Index++ ) {
            GlobalFrameBuffer[Index] = (UINT8)(PacketNumber + Index);
        }

        ST->ConOut->ClearScreen(ST->ConOut);
        Print(L"EFI Packet Transmitter 1.0\n");
        Print(L"  Press [SPACE] to pause or [ENTER] to exit\n\n");

        Print(L"PacketTx: #%05d Size 0x%X Error 0x%X\n",PacketNumber,BufferSize,ErrorPackets);
        if (BufferSize <= 0x80) {
            DumpHex(0,0,BufferSize,GlobalFrameBuffer);
        } else {
            DumpHex(0,0,0x80,GlobalFrameBuffer);
        }

        Print(L"\n");
        Print(L"Wait to transmit packet...");

        StopTransmit = FALSE;
        do {

            Status = ST->ConIn->ReadKeyStroke(ST->ConIn,&Key);
            if (Status == EFI_SUCCESS && (Key.UnicodeChar == 32 || Key.UnicodeChar == 0x0d)) {
                StopTransmit = TRUE;
            }

            Status = SimpleNetwork->Transmit(
                         SimpleNetwork,
                         HeaderSize,
                         BufferSize,
                         GlobalFrameBuffer,
                         NULL,
                         NULL,
                         NULL
                         );

            //
            // If the Transmit engine is busy, it might be due to the recycled transmit buffer
            // queue being full.  Go get all the elements from that queue.
            //

            if (Status == EFI_NOT_READY) {
                do {
                    SimpleNetwork->GetStatus(SimpleNetwork,NULL,&TxBuf);
                } while (TxBuf != NULL);
            }

        } while (Status == EFI_NOT_READY && StopTransmit == FALSE);

        if (EFI_ERROR(Status)) {
            Print(L"\n\n");
            Print(L"PacketTx: #%08X Size 0x%X Error 0x%X ERROR(%r)",PacketNumber,BufferSize,ErrorPackets,Status);
            Print(L"\n\n");
            Print(L"Press a key to continue\n");
            while(ST->ConIn->ReadKeyStroke(ST->ConIn,&Key) != EFI_SUCCESS);
        } else {
            PacketNumber++;
        }

        //
        // Check for a keystroke
        //

        if (StopTransmit == TRUE) {
            Status = EFI_SUCCESS;
        } else {
            Status = ST->ConIn->ReadKeyStroke(ST->ConIn,&Key);
        }
        if (Status == EFI_SUCCESS && Key.UnicodeChar == 32) {

            //
            // If the user pressed the space bar, then pause until another key is pressed.
            //

            Print(L"\n");
            Print(L"Transmit Paused.  Press a key to continue\n");

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
