/*++
Copyright (c) 2012, kontais                                               
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module name:
    Rtl81x9Dev.c

Abstract:

--*/

#include "Rtl81x9.h"
#include "Rtl81x9Dev.h"

UINT8
Rtl81x9Read1 (
  IN RTL81X9_SNP_DRIVER *Drv,
  IN UINT32              Reg
  )
{
  UINT8  Data;
  
  mPciIo->Io.Read (mPciIo, EfiPciIoWidthUint8, Drv->IoBarIndex, Reg, 1, &Data);
  
  return Data;
}

UINT16
Rtl81x9Read2 (
  IN RTL81X9_SNP_DRIVER *Drv,
  IN UINT32              Reg
  )
{
  UINT16  Data;
  
  mPciIo->Io.Read (mPciIo, EfiPciIoWidthUint16, Drv->IoBarIndex, Reg, 1, &Data);

  return Data;
}

UINT32
Rtl81x9Read4 (
  IN RTL81X9_SNP_DRIVER *Drv,
  IN UINT32              Reg
  )
{
  UINT32  Data;
  
  mPciIo->Io.Read (mPciIo, EfiPciIoWidthUint32, Drv->IoBarIndex, Reg, 1, &Data);

  return Data;
}

VOID
Rtl81x9Write1 (
  IN RTL81X9_SNP_DRIVER *Drv,
  IN UINT32              Reg,
  IN UINT8               Data
  )
{
  UINT8  DataW = Data;
  
  mPciIo->Io.Write (mPciIo, EfiPciIoWidthUint8, Drv->IoBarIndex, Reg, 1, &DataW);
  
}

VOID
Rtl81x9Write2 (
  IN RTL81X9_SNP_DRIVER *Drv,
  IN UINT32              Reg,
  IN UINT16              Data
  )
{
  UINT16  DataW = Data;
  
  mPciIo->Io.Write (mPciIo, EfiPciIoWidthUint16, Drv->IoBarIndex, Reg, 1, &DataW);

}

VOID
Rtl81x9Write4 (
  IN RTL81X9_SNP_DRIVER *Drv,
  IN UINT32              Reg,
  IN UINT32              Data
  )
{
  UINT32  DataW = Data;
  
  mPciIo->Io.Write (mPciIo, EfiPciIoWidthUint32, Drv->IoBarIndex, Reg, 1, &DataW);

}

EFI_STATUS
Rtl81x9DevInit (
  IN RTL81X9_SNP_DRIVER *Drv
  )
{
  RTL81X9_CHAIN_DATA   *Chain;
  UINT32               Index;
  
  Drv->IoBarIndex      = 0;

  //
  // Check EEPROM type 9346 or 9356
  //
  if (CSR_READ_4 (Drv, RL_RXCFG) & RL_RXCFG_9356_SEL) {
    Drv->NvAddrShift = RL_EEADDR_LEN1;
  } else {
    Drv->NvAddrShift = RL_EEADDR_LEN0;
  }

  Drv->Mode.State               = EfiSimpleNetworkStopped;
  Drv->Mode.HwAddressSize       = ETHER_ADDR_LEN;
  Drv->Mode.MediaHeaderSize     = ETHER_HDR_LEN;      // DA + SA + EtherType
  Drv->Mode.MaxPacketSize       = ETHER_MAX_PLD_LEN;
  Drv->Mode.NvRamAccessSize     = 2;
  Drv->Mode.NvRamSize           = (1 << Drv->NvAddrShift) * Drv->Mode.NvRamAccessSize;
  Drv->Mode.IfType              = PXE_IFTYPE_ETHERNET; // rfc1700.txt "Number Hardware Type"
  Drv->Mode.MaxMCastFilterCount = MAX_MCAST_ADDRESS_CNT;
  Drv->Mode.MCastFilterCount    = 0;

  Drv->Mode.MacAddressChangeable  = FALSE;
  Drv->Mode.MediaPresentSupported = FALSE;
  Drv->Mode.MultipleTxSupported   = FALSE;

  Drv->Mode.ReceiveFilterMask     =  EFI_SIMPLE_NETWORK_RECEIVE_UNICAST;
  Drv->Mode.ReceiveFilterMask     |= EFI_SIMPLE_NETWORK_RECEIVE_MULTICAST;
  Drv->Mode.ReceiveFilterMask     |= EFI_SIMPLE_NETWORK_RECEIVE_BROADCAST;
  Drv->Mode.ReceiveFilterMask     |= EFI_SIMPLE_NETWORK_RECEIVE_PROMISCUOUS;
  Drv->Mode.ReceiveFilterSetting  = 0;
  Drv->Mode.MediaPresent          = FALSE;

  Chain         = &Drv->ChainData;

  Chain->RxBuffer    = (UINT8*)(UINT32)Chain->Buffer;
  Chain->RxBufferDma = Chain->BufferDma;
  for (Index = 0; Index < RL_TX_LIST_CNT; Index++) {
    Chain->TxBuffer[Index]    = Chain->Buffer + RL_RXBUFLEN + Index * RL_TXBUFLEN;
    Chain->TxBufferDma[Index] = Chain->BufferDma + RL_RXBUFLEN + Index * RL_TXBUFLEN;
  }

  // Reset MAC
  Rtl81x9SetStationAddress (Drv, NULL);

  return EFI_SUCCESS;
}

EFI_STATUS
Rtl81x9DevStop (
  RTL81X9_SNP_DRIVER *Drv
  )
/*++

Routine Description:
 this routine calls undi to stop the interface and changes the snp state

Arguments:
  Drv  - pointer to snp driver structure

Returns:

--*/
{
  CSR_WRITE_1(Drv, RL_COMMAND, 0x00);
  
  return EFI_SUCCESS;
}

EFI_STATUS
Rtl81x9DevReset (
  RTL81X9_SNP_DRIVER *Drv
  )
/*++

Routine Description:
 this routine calls undi to reset the nic.

Arguments:
 Drv - pointer to the Drv driver structure

Returns:
 EFI_SUCCESSFUL for a successful completion
 other for failed calls

--*/
{
  UINT32  TimeOut;

  CSR_WRITE_1(Drv, RL_COMMAND, RL_CMD_RESET);

  for (TimeOut = 0; TimeOut < RL_TIMEOUT; TimeOut++) {
    DELAY(10);
    if (!(CSR_READ_1(Drv, RL_COMMAND) & RL_CMD_RESET)) {
      break;
    }
  }

  if (TimeOut == RL_TIMEOUT) {
    return EFI_TIMEOUT;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
Rtl81x9DevIntrDisable (
  RTL81X9_SNP_DRIVER *Drv
  )
{
  //
  // Disable Interrupt
  //
  CSR_WRITE_2(Drv, RL_IMR, 0x0000);
  CSR_READ_2(Drv, RL_ISR); // read clear
  
  return EFI_SUCCESS;
}

EFI_STATUS
Rtl81x9Shutdown (
  IN RTL81X9_SNP_DRIVER *Drv
  )
/*++

Routine Description:
 this routine calls undi to shut down the interface.

Arguments:
  Snp  - pointer to Snp driver structure

Returns:

--*/
{
  UINT32 Index;

  Rtl81x9DevStop (Drv);

  Rtl81x9DevIntrDisable (Drv);

	for (Index = 0; Index < RL_TX_LIST_CNT; Index++) {
    CSR_WRITE_4(Drv, RL_TXADDR0 + (Index * sizeof(UINT32)), 0x00000000);
	}
  
  return EFI_SUCCESS;
}



EFI_STATUS
Rtl81x9Stop (
  RTL81X9_SNP_DRIVER *Drv
  )
/*++

Routine Description:
 this routine calls undi to stop the interface and changes the snp state

Arguments:
  snp  - pointer to snp driver structure

Returns:

--*/
{
  //
  // Set simple network state to Started and return success.
  //
  Drv->Mode.State = EfiSimpleNetworkStopped;

  return EFI_SUCCESS;
}


EFI_STATUS
Rtl81x9GetStationAddress (
  RTL81X9_SNP_DRIVER *Drv
  )
/*++

Routine Description:
 this routine calls undi to read the MAC address of the NIC and updates the
 Mode structure with the address.

Arguments:
  Drv  - pointer to Drv driver structure

Returns:
 
--*/
{
  EFI_MAC_ADDRESS MacAddrReg;
  EFI_MAC_ADDRESS MacAddrEeprom;
  EFI_MAC_ADDRESS MacAddrBroadcast;
  UINT32          Index;

	CSR_WRITE_1(Drv, RL_EECMD, RL_EEMODE_WRITECFG);
  *(UINT32*)(&MacAddrReg.Addr[0]) = CSR_READ_4(Drv, RL_IDR0);
  *(UINT32*)(&MacAddrReg.Addr[4]) = CSR_READ_4(Drv, RL_IDR4);
	CSR_WRITE_1(Drv, RL_EECMD, RL_EEMODE_OFF);

  Rtl81x9EepromRead (
                 Drv, 
                 RL_EE_EADDR * Drv->Mode.NvRamAccessSize, 
                 Drv->Mode.HwAddressSize,
                 &MacAddrEeprom);

  for (Index = 0; Index < Drv->Mode.HwAddressSize; Index++) {
    MacAddrBroadcast.Addr[Index] = 0xff;
  }

  //
  // Set new station address in Drv->Mode structure and return success.
  //
  EfiCopyMem (
    &(Drv->Mode.CurrentAddress),
    &MacAddrReg,
    Drv->Mode.HwAddressSize
    );

  EfiCopyMem (
    &Drv->Mode.BroadcastAddress,
    &MacAddrBroadcast,
    Drv->Mode.HwAddressSize
    );

  EfiCopyMem (
    &Drv->Mode.PermanentAddress,
    &MacAddrEeprom,
    Drv->Mode.HwAddressSize
    );

  return EFI_SUCCESS;
}

EFI_STATUS
Rtl81x9SetStationAddress (
  RTL81X9_SNP_DRIVER      *Drv,
  EFI_MAC_ADDRESS         *NewMacAddr
  )
/*++

Routine Description:
 this routine calls undi to set a new MAC address for the NIC,

Arguments:
  Drv  - pointer to Drv driver structure
  NewMacAddr - pointer to a mac address to be set for the nic, if This is NULL
               then this routine resets the mac address to the NIC's original
               address.

Returns:

--*/
{
  EFI_MAC_ADDRESS MacAddrEeprom;
  EFI_MAC_ADDRESS *MacAddrPtr;
  
  if (NewMacAddr == NULL) {
    Rtl81x9EepromRead (
                   Drv, 
                   RL_EE_EADDR * Drv->Mode.NvRamAccessSize, 
                   Drv->Mode.HwAddressSize,
                   &MacAddrEeprom);
    MacAddrPtr = &MacAddrEeprom;
  } else {
    MacAddrPtr = NewMacAddr;
  }

	CSR_WRITE_1(Drv, RL_EECMD, RL_EEMODE_WRITECFG);
	CSR_WRITE_4(Drv, RL_IDR0, *(UINT32*)(&MacAddrPtr->Addr[0]));
	CSR_WRITE_4(Drv, RL_IDR4, *(UINT32*)(&MacAddrPtr->Addr[4]));
	CSR_WRITE_1(Drv, RL_EECMD, RL_EEMODE_OFF);

  //
  // read the changed address and save it in Drv->Mode structure
  //
  Rtl81x9GetStationAddress (Drv);

  return EFI_SUCCESS;
}


EFI_STATUS
Rtl81x9Start (
  RTL81X9_SNP_DRIVER *Drv
  )
/*++

Routine Description:
 this routine calls undi to start the interface and changes the Drv state!

Arguments:
  Drv  - pointer to Drv driver structure

Returns:

--*/
{
  //
  // Set simple network state to Started and return success.
  //
  Drv->Mode.State = EfiSimpleNetworkStarted;

  return EFI_SUCCESS;
}


EFI_STATUS
Rtl81x9Init (
  RTL81X9_SNP_DRIVER *Drv,
  UINT16             CableDetectFlag
  )
/*++

Routine Description:
 this routine calls undi to initialize the interface.

Arguments:
 Drv  - pointer to Drv driver structure
 CableDetectFlag - Do/don't detect the cable (depending on what undi supports)

Returns:

--*/
{
  RTL81X9_CHAIN_DATA *Chain;
  UINT32             Index;
  UINT32             Data;

  // Cancel pending I/O and free all RX/TX buffers.
  Rtl81x9DevStop (Drv);
 
  // Init Tx/Rx DMA Buffer
  Chain = &Drv->ChainData;
  CSR_WRITE_4(Drv, RL_RXADDR,  (UINT32)Chain->RxBufferDma);
  for (Index = 0; Index < RL_TX_LIST_CNT; Index++) {
    CSR_WRITE_4(Drv, RL_TXADDR0 + Index * sizeof(UINT32), (UINT32)Chain->TxBufferDma[Index]);
  }
  //Set DB to no used
  Chain->TxUsedMask = 0;
  
  // Enable transmit and receive.
	CSR_WRITE_1(Drv, RL_COMMAND, RL_CMD_TX_ENB|RL_CMD_RX_ENB);

  // Set the initial TX and RX configuration.
	CSR_WRITE_4(Drv, RL_TXCFG, RL_TXCFG_CONFIG);
  Data = CSR_READ_4(Drv, RL_RXCFG);
	CSR_WRITE_4(Drv, RL_RXCFG, Data | RL_RXCFG_CONFIG | RL_RXCFG_RX_INDIV | RL_RXCFG_RX_BROAD);  
  
  // Start RX/TX process.
  CSR_WRITE_4(Drv, RL_MISSEDPKT, 0);
  
  // Enable receiver and transmitter.
  CSR_WRITE_1(Drv, RL_COMMAND, RL_CMD_TX_ENB | RL_CMD_RX_ENB);
  
  CSR_WRITE_1(Drv, RL_EECMD, RL_EEMODE_WRITECFG );
  Data = CSR_READ_1(Drv, RL_CFG1);
  CSR_WRITE_1(Drv, RL_CFG1,  Data | RL_CFG1_PME);
  CSR_WRITE_1(Drv, RL_EECMD, RL_EEMODE_OFF);
  
  Rtl81x9DevIntrDisable(Drv);

  Drv->Mode.State = EfiSimpleNetworkInitialized;

  return EFI_SUCCESS;
}


EFI_STATUS
Rtl81x9Reset (
  RTL81X9_SNP_DRIVER *Drv
  )
/*++

Routine Description:
 this routine calls undi to reset the nic.

Arguments:
 Drv - pointer to the Drv driver structure

Returns:
 EFI_SUCCESSFUL for a successful completion
 other for failed calls

--*/
{
  Rtl81x9DevReset (Drv);

  Rtl81x9DevIntrDisable (Drv);

  Rtl81x9Init (Drv, PXE_OPFLAGS_INITIALIZE_DO_NOT_DETECT_CABLE);
  
  return EFI_SUCCESS;
}

EFI_STATUS
Rtl81x9RxStatus (
  RTL81X9_SNP_DRIVER *Drv,
  UINT32             *RxStatus
  )
/*++

Routine Description:
 this routine calls undi to reset the nic.

Arguments:
 Drv      - pointer to the Drv driver structure
 RxStatus - 1 - Packet Arrived
            0 - No Packet Arrived

Returns:
 EFI_SUCCESSFUL for a successful completion
 other for failed calls

--*/
{
  //
  // Check Rx Buffer Empty Flag
  //
  if (CSR_READ_1(Drv, RL_COMMAND) & RL_CMD_EMPTY_RXBUF) {
    *RxStatus = 0;
  } else {
    *RxStatus = 1;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
Rtl81x9ReceiveFilterEnable (
  RTL81X9_SNP_DRIVER      *Drv,
  UINT32          EnableFlags,
  UINTN           MCastAddressCount,
  EFI_MAC_ADDRESS *MCastAddressList
  )
/*++

Routine Description:
 this routine calls undi to enable the receive filters.

Arguments:
  Drv  - pointer to Drv driver structure
  EnableFlags - bit mask for enabling the receive filters
  MCastAddressCount - multicast address count for a new multicast address list
  MCastAddressList  - list of new multicast addresses

Returns:

--*/
{
  UINT32 RxConfig;
  UINT32 Data;

  RxConfig = 0;
  
  if (EnableFlags & EFI_SIMPLE_NETWORK_RECEIVE_UNICAST) {
    RxConfig |= RL_RXCFG_RX_INDIV;
  }

  if (EnableFlags & EFI_SIMPLE_NETWORK_RECEIVE_MULTICAST) {
    RxConfig |= RL_RXCFG_RX_MULTI;
  }

  if (EnableFlags & EFI_SIMPLE_NETWORK_RECEIVE_BROADCAST) {
    RxConfig |= RL_RXCFG_RX_BROAD;
  }
  
  if (EnableFlags & EFI_SIMPLE_NETWORK_RECEIVE_PROMISCUOUS) {
    RxConfig |= RL_RXCFG_RX_ALLPHYS;
  }

  Data = CSR_READ_4(Drv, RL_RXCFG);
	CSR_WRITE_4(Drv, RL_RXCFG, Data | RxConfig); 

  return EFI_SUCCESS;
}

EFI_STATUS
Rtl81x9ReceiveFilterDisable (
  RTL81X9_SNP_DRIVER *Drv,
  UINT32     DisableFlags,
  BOOLEAN    ResetMCastList
  )
/*++

Routine Description:
 this routine calls undi to disable the receive filters.

Arguments:
  Drv  - pointer to Drv driver structure
  DisableFlags - bit mask for disabling the receive filters
  ResetMCastList - boolean flag to reset/delete the multicast filter list

Returns:

--*/
{
  UINT32 RxConfig;
  UINT32 Data;

  RxConfig = 0;
  
  if (DisableFlags & EFI_SIMPLE_NETWORK_RECEIVE_UNICAST) {
    RxConfig |= RL_RXCFG_RX_INDIV;
  }

  if (DisableFlags & EFI_SIMPLE_NETWORK_RECEIVE_MULTICAST) {
    RxConfig |= RL_RXCFG_RX_MULTI;
  }

  if (DisableFlags & EFI_SIMPLE_NETWORK_RECEIVE_BROADCAST) {
    RxConfig |= RL_RXCFG_RX_BROAD;
  }

  if (DisableFlags & EFI_SIMPLE_NETWORK_RECEIVE_PROMISCUOUS) {
    RxConfig |= RL_RXCFG_RX_ALLPHYS;
  }

  Data = CSR_READ_4(Drv, RL_RXCFG);
	CSR_WRITE_4(Drv, RL_RXCFG, Data & ~RxConfig); 

  return EFI_SUCCESS;
}

EFI_STATUS
Rtl81x9ReceiveFilterRead (
  RTL81X9_SNP_DRIVER *Drv
  )
/*++

Routine Description:
 this routine calls undi to read the receive filters.

Arguments:
  Drv  - pointer to Drv driver structure

Returns:

--*/
{
  UINT32 RxConfig;

  RxConfig = CSR_READ_4(Drv, RL_RXCFG);
  
  Drv->Mode.ReceiveFilterSetting = 0;

  if (RxConfig & RL_RXCFG_RX_INDIV) {
    Drv->Mode.ReceiveFilterSetting |= EFI_SIMPLE_NETWORK_RECEIVE_UNICAST;
  }

  if (RxConfig & RL_RXCFG_RX_BROAD) {
    Drv->Mode.ReceiveFilterSetting |= EFI_SIMPLE_NETWORK_RECEIVE_BROADCAST;
  }

  if (RxConfig & RL_RXCFG_RX_ALLPHYS) {
    Drv->Mode.ReceiveFilterSetting |= EFI_SIMPLE_NETWORK_RECEIVE_PROMISCUOUS;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
Rtl81x9MCastIpToMac (
  IN RTL81X9_SNP_DRIVER  *Drv,
  IN BOOLEAN             IPv6,
  IN EFI_IP_ADDRESS      *IP,
  IN OUT EFI_MAC_ADDRESS *MAC
  )
/*++

Routine Description:
 this routine calls undi to convert an multicast IP address to a MAC address

Arguments:
  Drv  - pointer to Drv driver structure
  IPv6  - flag to indicate if This is an ipv6 address
  IP    - multicast IP address
  MAC   - pointer to hold the return MAC address

Returns:

--*/
{
  return EFI_UNSUPPORTED;
}


EFI_STATUS
Rtl81x9GetStatus (
  RTL81X9_SNP_DRIVER *Drv,
  UINT32             *InterruptStatusPtr,
  VOID               **TransmitBufferListPtr
  )
/*++

Routine Description:
 this routine calls undi to get the status of the interrupts, get the list of
 transmit buffers that completed transmitting! 

Arguments:
 Drv  - pointer to Drv driver structure
 InterruptStatusPtr - a non null pointer gets the interrupt status
 TransmitBufferListPtrs - a non null ointer gets the list of pointers of previously 
              transmitted buffers whose transmission was completed 
              asynchrnously.
              
Returns:

--*/
{
  UINT16             InterruptFlags;
  UINT32             TxStatus;
  RTL81X9_CHAIN_DATA *TxRxChain;
  UINT32             TxIndex;
  UINT32             CurrMask;

  //
  // Interrupt Status
  //
  if (InterruptStatusPtr != NULL) {
    InterruptFlags      = CSR_READ_2 (Drv, RL_ISR);

    *InterruptStatusPtr = 0;

    if (InterruptFlags & RL_ISR_RX_OK) {
      *InterruptStatusPtr |= EFI_SIMPLE_NETWORK_RECEIVE_INTERRUPT;
    }

    if (InterruptFlags & RL_ISR_TX_OK) {
      *InterruptStatusPtr |= EFI_SIMPLE_NETWORK_TRANSMIT_INTERRUPT;
    }
  
    // If the card has gone away, the read returns 0xffff
    if (InterruptFlags == 0xFFFF) {
      *InterruptStatusPtr =  EFI_SIMPLE_NETWORK_NO_MEDIA;
    }    
  }

  if (TransmitBufferListPtr != NULL) {
    
    TxRxChain = &Drv->ChainData;
    TxIndex   = TxRxChain->TxIndex;
    CurrMask  = 1 << TxIndex;

    *TransmitBufferListPtr = TxRxChain->TxBuffer[TxIndex];
    
    if (TxRxChain->TxUsedMask & CurrMask) {
      TxStatus  = CSR_READ_4 (Drv, RL_TXSTAT0 + (TxIndex * 4));
      // Tx Busy
      if (!(TxStatus & (RL_TXSTAT_TX_OK | RL_TXSTAT_TX_UNDERRUN | RL_TXSTAT_TXABRT))) {
        *TransmitBufferListPtr = 0;
      }

      // Tx Error
      if ((TxStatus & RL_TXSTAT_TXABRT) || (TxStatus & RL_TXSTAT_OUTOFWIN)) {
        CSR_WRITE_4(Drv, RL_TXCFG, RL_TXCFG_CONFIG);
        Rtl81x9Reset (Drv);
        MEMORY_FENCE();
      }
    }
  }

  return EFI_SUCCESS;
}

EFI_STATUS
Rtl81x9FillHeader (
  RTL81X9_SNP_DRIVER  *Drv,
  VOID                *MacHeaderPtr,
  UINTN               MacHeaderSize,
  VOID                *BufferPtr,
  UINTN               BufferLength,
  EFI_MAC_ADDRESS     *DestinationAddrPtr,
  EFI_MAC_ADDRESS     *SourceAddrPtr,
  UINT16              *ProtocolPtr
  )
/*++

Routine Description:
 this routine calls undi to create the meadia header for the given data buffer.
 
Arguments:
 Drv - pointer to Drv driver structure
 MacHeaderPtr - address where the media header will be filled in.
 MacHeaderSize - size of the memory at MacHeaderPtr
 BufferPtr - data buffer pointer
 BufferLength - Size of data in the BufferPtr
 DestinationAddrPtr - address of the destination mac address buffer
 SourceAddrPtr - address of the source mac address buffer
 ProtocolPtr - address of the protocol type
 
Returns:
 EFI_SUCCESS - if successfully completed the undi call
 Other - error return from undi call.
 
--*/
{
  ETHER_HEAD   *Header;
  
  Header = (ETHER_HEAD *)MacHeaderPtr;

  if (SourceAddrPtr) {
    EfiCopyMem (
      (VOID *) Header->SrcMac,
      (VOID *) SourceAddrPtr,
      Drv->Mode.HwAddressSize
      );
  } else {
    EfiCopyMem (
      (VOID *) Header->SrcMac,
      (VOID *) &(Drv->Mode.CurrentAddress),
      Drv->Mode.HwAddressSize
      );
  }

  EfiCopyMem (
    (VOID *) Header->DstMac,
    (VOID *) DestinationAddrPtr,
    Drv->Mode.HwAddressSize
    );

  Header->EtherType = (UINT16) PXE_SWAP_UINT16 (*ProtocolPtr);

  return EFI_SUCCESS;
}

EFI_STATUS
Rtl81x9Transmit (
  RTL81X9_SNP_DRIVER *Drv,
  VOID               *BufferPtr,
  UINTN              BufferLength
  )
/*++

Routine Description:
 this routine calls undi to transmit the given data buffer
 
Arguments:
 Drv - pointer to Drv driver structure
 BufferPtr - data buffer pointer
 BufferLength - Size of data in the BufferPtr
 
Returns:
 EFI_SUCCESS - if successfully completed the undi call
 Other - error return from undi call.
 
--*/
{
  RTL81X9_CHAIN_DATA *TxRxChain;
  UINT32             TxStatus;
  UINT32             TxIndex;
  UINT32             CopyLen;
  UINT32             PadLen;
  UINT32             FrameLen;
  UINT32             CurrMask;

  TxRxChain = &Drv->ChainData;
  TxIndex   = TxRxChain->TxIndex;
  CurrMask  = 1 << TxIndex;

  if (TxRxChain->TxUsedMask & CurrMask) {
    // Tx Busy Check
    TxStatus  = CSR_READ_4 (Drv, RL_TXSTAT0 + (TxIndex * 4));
    if (!(TxStatus & (RL_TXSTAT_TX_OK | RL_TXSTAT_TX_UNDERRUN | RL_TXSTAT_TXABRT))) {
      return EFI_NOT_READY;
    }
    //
    // Error Recovery
    //
    if (TxStatus & (RL_TXSTAT_TXABRT | RL_TXSTAT_OUTOFWIN)) {
      RTL81X9_PRINT("Tx Error Status = 0x%08x\n", TxStatus);
      Rtl81x9Reset (Drv);
      return EFI_NOT_READY;
    }
  }

  //
  // Copy Tx Data to Tx Buffer, can not exceed MaxPacketSize
  //
  CopyLen  = (BufferLength <= Drv->Mode.MaxPacketSize) ? BufferLength : Drv->Mode.MaxPacketSize;
  EfiCopyMem (TxRxChain->TxBuffer[TxIndex], BufferPtr, CopyLen);

  //
  // Add zero pad if length < 60
  //
  PadLen = (CopyLen < RL_MIN_FRAMELEN) ? (RL_MIN_FRAMELEN - CopyLen) : 0;
  if(PadLen) {
    EfiSetMem(TxRxChain->TxBuffer[TxIndex] + CopyLen, 0, PadLen);
  }

  FrameLen = CopyLen + PadLen;

  Drv->PciIo->Flush (Drv->PciIo);

  //
  // Start Transmit
  //
  CSR_WRITE_4(Drv, RL_TXSTAT0 +TxIndex * 4, (RL_TXTHRESH(RL_TX_THRESH_INIT)) | FrameLen);

  //
  // Set to used
  //
  TxRxChain->TxUsedMask |= CurrMask;
  
  TxIndex ++;
  TxRxChain->TxIndex = TxIndex % RL_TX_LIST_CNT;
  
  MEMORY_FENCE();

  return EFI_SUCCESS;
}

EFI_STATUS
Rtl81x9Receive (
  RTL81X9_SNP_DRIVER *Drv,
  VOID               *BufferPtr,
  UINTN              *BuffSizePtr,
  UINTN              *HeaderSizePtr,
  EFI_MAC_ADDRESS    *SourceAddrPtr,
  EFI_MAC_ADDRESS    *DestinationAddrPtr,
  UINT16             *ProtocolPtr
  )
/*++

Routine Description:
 this routine calls undi to receive a packet and fills in the data in the
 input pointers!
 
Arguments:
  Drv  - pointer to Drv driver structure
  BufferPtr   - pointer to the memory for the received data
  BuffSizePtr - is a pointer to the length of the buffer on entry and contains
                the length of the received data on return
  HeaderSizePtr - pointer to the header portion of the data received.
  SourceAddrPtr    - optional parameter, is a pointer to contain the source
                ethernet address on return
  DestinationAddrPtr   - optional parameter, is a pointer to contain the destination
                ethernet address on return
  ProtocolPtr    - optional parameter, is a pointer to contain the protocol type
                from the ethernet header on return


Returns:

--*/
{
  RTL81X9_CHAIN_DATA *TxRxChain;
  UINT32             RxSize;
  UINT32             RxStatus;
  UINT16             cur_rx;
  UINT32             SplitCnt;
  UINT32             PacketLen;
  UINT32             PacketOff;
  ETHER_HEAD         *PacketHdr;

  //
  // Check Rx Buffer Empty Flag
  //
  if (CSR_READ_1(Drv, RL_COMMAND) & RL_CMD_EMPTY_RXBUF) {
    return EFI_NOT_READY;
  }
  
  //
  // RL_CURRXADDR reset value is 0xfff0
  //
  cur_rx = (CSR_READ_2(Drv, RL_CURRXADDR) + 16) % RL_RXBUFLEN;

  //
  // Flush
  //
  Drv->PciIo->Flush (Drv->PciIo);

  //
  // Rx Stauts and Size
  //
  TxRxChain = &Drv->ChainData;
  RxStatus  = *(UINT32 *)(TxRxChain->RxBuffer + cur_rx);
  RxSize    = RxStatus >> 16;
  RxStatus  &= 0xffff;

  //
  // DMA FIFO to RAM in progress state, if EarlyRx is enabled.
  //
  if (RxSize == RL_RXSTAT_UNFINISHED) {
    return EFI_NOT_READY;
  }

  // Rx Error
  if (RxSize < ETHER_MIN_LEN || 
      RxSize > (ETHER_MAX_LEN + ETHER_VLAN_ENCAP_LEN) || 
      !(RxStatus & RL_RXSTAT_RXOK)) {
    RTL81X9_PRINT("Rx Error Status = 0x%08x Size = 0x%08x\n", RxStatus, RxSize);
    Rtl81x9Reset (Drv);
    return EFI_DEVICE_ERROR;
  }

  //
  // Rx OK
  //
  PacketLen = RxSize - 4; // Remove 4 Byte CRC
  PacketOff = cur_rx + 4; // Skip RxStatus and RxSize

  Drv->PciIo->Flush (Drv->PciIo);
  
  //
  // Copy Rx Data
  //
  if (PacketOff + PacketLen <= RL_RXBUFLEN) {
    EfiCopyMem(BufferPtr, TxRxChain->RxBuffer + PacketOff, PacketLen);
  } else {
    SplitCnt = RL_RXBUFLEN - PacketOff;
    EfiCopyMem(BufferPtr, TxRxChain->RxBuffer + PacketOff, SplitCnt);
    EfiCopyMem(BufferPtr + SplitCnt, TxRxChain->RxBuffer, PacketLen - SplitCnt);   
  }

  if (*BuffSizePtr < PacketLen) {
    return EFI_BUFFER_TOO_SMALL;
  }

  *BuffSizePtr = PacketLen;

  if (HeaderSizePtr != NULL) {
    *HeaderSizePtr = Drv->Mode.MediaHeaderSize;
  }

  PacketHdr = BufferPtr;
  if (SourceAddrPtr != NULL) {
    EfiCopyMem (SourceAddrPtr, &PacketHdr->SrcMac, Drv->Mode.HwAddressSize);
  }

  if (DestinationAddrPtr != NULL) {
    EfiCopyMem (DestinationAddrPtr, &PacketHdr->DstMac, Drv->Mode.HwAddressSize);
  }

  if (ProtocolPtr != NULL) {
    *ProtocolPtr = (UINT16) PXE_SWAP_UINT16 (PacketHdr->EtherType);
  }

  //
  // Set Driver Next Read Offset
  //
  cur_rx = ((cur_rx + 4 + RxSize + 3) & (~3)) % RL_RXBUFLEN;
  CSR_WRITE_2(Drv, RL_CURRXADDR, cur_rx - 16);

  //
  // Clear Interrupt Status
  //
  if(CSR_READ_2(Drv, RL_ISR) & RL_INTRS_ACT) {
    CSR_WRITE_2(Drv, RL_ISR, RL_INTRS_ACT);
  }

  return EFI_SUCCESS;
}


