/*++

Copyright (c) 2012, kontais                                                                          
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module name:
    Rtl81x9Dev.h

Abstract:

Revision history:

--*/
#ifndef _RTL81X9_DEV_H_
#define _RTL81X9_DEV_H_

#define  CSR_READ_1(Drv, Reg)              Rtl81x9Read1((Drv), (Reg))
#define  CSR_READ_2(Drv, Reg)              Rtl81x9Read2((Drv), (Reg))
#define  CSR_READ_4(Drv, Reg)              Rtl81x9Read4((Drv), (Reg))
#define  CSR_WRITE_1(Drv, Reg, Val)        Rtl81x9Write1((Drv), (Reg), (Val))
#define  CSR_WRITE_2(Drv, Reg, Val)        Rtl81x9Write2((Drv), (Reg), (Val))
#define  CSR_WRITE_4(Drv, Reg, Val)        Rtl81x9Write4((Drv), (Reg), (Val))

#define  DELAY(x)             gBS->Stall (x)

UINT8
Rtl81x9Read1 (
  IN RTL81X9_SNP_DRIVER *Drv,
  IN UINT32              Reg
  );

UINT16
Rtl81x9Read2 (
  IN RTL81X9_SNP_DRIVER *Drv,
  IN UINT32              Reg
  )
;

UINT32
Rtl81x9Read4 (
  IN RTL81X9_SNP_DRIVER *Drv,
  IN UINT32              Reg
  )
;

VOID
Rtl81x9Write1 (
  IN RTL81X9_SNP_DRIVER *Drv,
  IN UINT32              Reg,
  IN UINT8               Data
  )
;

VOID
Rtl81x9Write2 (
  IN RTL81X9_SNP_DRIVER *Drv,
  IN UINT32              Reg,
  IN UINT16              Data
  )
;

VOID
Rtl81x9Write4 (
  IN RTL81X9_SNP_DRIVER *Drv,
  IN UINT32              Reg,
  IN UINT32              Data
  )
;

EFI_STATUS
Rtl81x9DevInit (
  IN RTL81X9_SNP_DRIVER *Drv
  );

EFI_STATUS
Rtl81x9DevStop (
  RTL81X9_SNP_DRIVER *Drv
  );

EFI_STATUS
Rtl81x9DevReset (
  RTL81X9_SNP_DRIVER *Drv
  );

EFI_STATUS
Rtl81x9EepromRead (
  IN RTL81X9_SNP_DRIVER *Drv,
  IN UINTN              RegOffset,
  IN UINTN              NumBytes,
  IN OUT VOID           *BufferPtr
  );

EFI_STATUS
Rtl81x9EepromWrite (
  IN RTL81X9_SNP_DRIVER *Drv,
  IN UINTN              RegOffset,
  IN UINTN              NumBytes,
  IN OUT VOID           *BufferPtr
  );

EFI_STATUS
Rtl81x9Shutdown (
  IN RTL81X9_SNP_DRIVER *Drv
  );

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
;

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
;

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
;

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
;

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
;

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
;

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
;

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
;

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
;

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
;

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
;

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
;

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
;

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
;

#endif /*  _RTL81X9_DEV_H_  */
