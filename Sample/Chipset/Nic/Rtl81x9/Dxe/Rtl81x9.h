/*++

Copyright (c) 2004 - 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module name:
    Rtl81x9.h

Abstract:

Revision history:

--*/
#ifndef _RTL81X9_H_
#define _RTL81X9_H_


#include "Tiano.h"
#include "EfiDriverLib.h"
#include "Pci22.h"
#include "EfiPxe.h"
#include "EfiPrintLib.h"

#include "Rtl81x9Reg.h"

#define RTL81X9_DEBUG       1

#if RTL81X9_DEBUG
#define RTL81X9_PRINT(DebugInfo, ...)  EfiDebugPrint (EFI_D_ERROR, (CHAR8*)DebugInfo, __VA_ARGS__)
#else
#define RTL81X9_PRINT(DebugInfo, ...)
#endif

//
// Driver Consumed Protocol Prototypes
//
#include EFI_PROTOCOL_DEFINITION (DevicePath)
#include EFI_PROTOCOL_DEFINITION (PciIo)
#include EFI_PROTOCOL_DEFINITION (EfiNetworkInterfaceIdentifier)

//
// Driver Produced Protocol Prototypes
//
#include EFI_PROTOCOL_DEFINITION (DriverBinding)
#include EFI_PROTOCOL_DEFINITION (ComponentName)
#include EFI_PROTOCOL_DEFINITION (SimpleNetwork)

#define RTL81X9_SNP_DRIVER_SIGNATURE  EFI_SIGNATURE_32 ('R', 'L', 'T', 'K')

typedef struct _RTL81X9_SNP_DRIVER {
  UINT32                      Signature;
  EFI_LOCK                    lock;

  EFI_SIMPLE_NETWORK_PROTOCOL Snp;
  EFI_SIMPLE_NETWORK_MODE     Mode;

  EFI_HANDLE                  DeviceHandle;
  EFI_DEVICE_PATH_PROTOCOL    *DevicePath;

  EFI_PCI_IO_PROTOCOL         *PciIo;

  EFI_NETWORK_INTERFACE_IDENTIFIER_PROTOCOL Nii;

  UINT8                       IoBarIndex;
  UINT8                       MemoryBarIndex;

  UINT32                      NvAddrShift;

  RTL81X9_CHAIN_DATA          ChainData;  
} RTL81X9_SNP_DRIVER;

#define RTL81X9_PRIVATE_FROM_EFI_SIMPLE_NETWORK(a) CR (a, RTL81X9_SNP_DRIVER, Snp, RTL81X9_SNP_DRIVER_SIGNATURE)

//
// Global Variables
//
extern EFI_COMPONENT_NAME_PROTOCOL  gSimpleNetworkComponentName;
extern EFI_PCI_IO_PROTOCOL          *mPciIo;
extern EFI_SIMPLE_NETWORK_PROTOCOL Rtl81x9SimpleNetworkProtocol;


EFI_STATUS
EFIAPI
Rtl81x9Snp_Shutdown (
  IN EFI_SIMPLE_NETWORK_PROTOCOL *This
  )
/*++

Routine Description:
 This is the SNP interface routine for shutting down the interface
 This routine basically retrieves Snp structure, checks the SNP state and
 calls the Rtl81x9Snp_Shutdown routine to actually do the undi shutdown

Arguments:
  this  - context pointer

Returns:

--*/
;

EFI_STATUS
EFIAPI
Rtl81x9Snp_NvData (
  IN EFI_SIMPLE_NETWORK_PROTOCOL *This,
  IN BOOLEAN                     Read,
  IN UINTN                       RegOffset,
  IN UINTN                       NumBytes,
  IN OUT VOID                    *BufferPtr
  )
/*++

Routine Description:
 This is an interface call provided by SNP.
 It does the basic checking on the input parameters and retrieves snp structure
 and then calls the read_nvdata() call which does the actual reading

Arguments:
  This      - context pointer
  Read      - true for reading and false for writing
  RegOffset - eeprom register relative to the base
  NumBytes  - how many bytes to read
  BufferPtr - address of memory to read into

Returns:

--*/
;

EFI_STATUS
EFIAPI
Rtl81x9Snp_Stop (
  IN EFI_SIMPLE_NETWORK_PROTOCOL *This
  )
/*++

Routine Description:
 This is the SNP interface routine for stopping the interface.
 This routine basically retrieves snp structure, checks the SNP state and
 calls the Rtl81x9Stop routine to actually stop the undi interface

Arguments:
  This  - context pointer

Returns:

--*/
;

EFI_STATUS
EFIAPI
Rtl81x9Snp_StationAddress (
  IN EFI_SIMPLE_NETWORK_PROTOCOL * This,
  IN BOOLEAN                     ResetFlag,
  IN EFI_MAC_ADDRESS             * NewMacAddr OPTIONAL
  )
/*++

Routine Description:
 This is the SNP interface routine for changing the NIC's mac address.
 this routine basically retrieves Drv structure, checks the Drv state and
 calls the above routines to actually do the work

Arguments:
 This  - context pointer
 NewMacAddr - pointer to a mac address to be set for the nic, if This is NULL
              then this routine resets the mac address to the NIC's original
              address.
 ResetFlag - If true, the mac address will change to NIC's original address

Returns:

--*/
;

EFI_STATUS
EFIAPI
Rtl81x9Snp_Start (
  IN EFI_SIMPLE_NETWORK_PROTOCOL *This
  )
/*++

Routine Description:
 This is the SNP interface routine for starting the interface
 this routine basically retrieves Drv structure, checks the Drv state and
 calls the pxe_start routine to actually do start undi interface

Arguments:
  This  - context pointer

Returns:
  EFI_INVALID_PARAMETER - "This" is Null
                        - No Drv driver can be extracted from "This"
  EFI_ALREADY_STARTED   - The state of Drv is EfiSimpleNetworkStarted
                          or EfiSimpleNetworkInitialized
  EFI_DEVICE_ERROR      - The state of Drv is other than EfiSimpleNetworkStarted,
                          EfiSimpleNetworkInitialized, and EfiSimpleNetworkStopped
  EFI_SUCCESS           - UNDI interface is succesfully started
  Other                 - Error occurs while calling pxe_start function.
  
--*/
;

EFI_STATUS
EFIAPI
Rtl81x9Snp_Initialize (
  IN EFI_SIMPLE_NETWORK_PROTOCOL *This,
  IN UINTN                       extra_rx_buffer_size OPTIONAL,
  IN UINTN                       extra_tx_buffer_size OPTIONAL
  )
/*++

Routine Description:
 This is the SNP interface routine for initializing the interface
 this routine basically retrieves Drv structure, checks the Drv state and
 calls the pxe_initialize routine to actually do the undi initialization

Arguments:
 This  - context pointer
 extra_rx_buffer_size - optional parameter, indicates extra space for rx_buffers
 extra_tx_buffer_size - optional parameter, indicates extra space for tx_buffers

Returns:

--*/
;

EFI_STATUS
EFIAPI
Rtl81x9Snp_Reset (
  IN EFI_SIMPLE_NETWORK_PROTOCOL *This,
  IN BOOLEAN                     ExtendedVerification
  )
/*++

Routine Description:
 This is the SNP interface routine for resetting the NIC
 this routine basically retrieves Drv structure, checks the Drv state and
 calls the pxe_reset routine to actually do the reset!

Arguments:
 This - context pointer
 ExtendedVerification - not implemented

Returns:

--*/
;

VOID
EFIAPI
Rtl81x9WaitForPacketNotify (
  EFI_EVENT Event,
  VOID      *SnpPtr
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
;

EFI_STATUS
EFIAPI
Rtl81x9Snp_ReceiveFilters (
  IN EFI_SIMPLE_NETWORK_PROTOCOL * This,
  IN UINT32                      EnableFlags,
  IN UINT32                      DisableFlags,
  IN BOOLEAN                     ResetMCastList,
  IN UINTN                       MCastAddressCount OPTIONAL,
  IN EFI_MAC_ADDRESS             * MCastAddressList OPTIONAL
  )
/*++

Routine Description:
 This is the SNP interface routine for reading/enabling/disabling the
 receive filters.
 this routine basically retrieves Drv structure, checks the Drv state and
 checks the parameter validity, calls one of the above routines to actually
 do the work

Arguments:
  This  - context pointer
  EnableFlags - bit mask for enabling the receive filters
  DisableFlags - bit mask for disabling the receive filters
  ResetMCastList - boolean flag to reset/delete the multicast filter list
  MCastAddressCount - multicast address count for a new multicast address list
  MCastAddressList  - list of new multicast addresses

Returns:

--*/
;

EFI_STATUS
EFIAPI
Rtl81x9Snp_Statistics (
  IN EFI_SIMPLE_NETWORK_PROTOCOL * This,
  IN BOOLEAN                     ResetFlag,
  IN OUT UINTN                   *StatTableSizePtr OPTIONAL,
  IN OUT EFI_NETWORK_STATISTICS  * StatTablePtr OPTIONAL
  )
/*++

Routine Description:
 This is the SNP interface routine for getting the NIC's statistics.
 this routine basically retrieves Drv structure, checks the Drv state and
 calls the pxe_ routine to actually do the 

Arguments:
  This  - context pointer
  ResetFlag - true to reset the NIC's statistics counters to zero.
  StatTableSizePtr - pointer to the statistics table size
  StatTablePtr - pointer to the statistics table 
  
Returns:

--*/
;

EFI_STATUS
EFIAPI
Rtl81x9Snp_MCastIpToMac (
  IN EFI_SIMPLE_NETWORK_PROTOCOL *This,
  IN BOOLEAN                     IPv6,
  IN EFI_IP_ADDRESS              *IP,
  OUT EFI_MAC_ADDRESS            *MAC
  )
/*++

Routine Description:
 This is the SNP interface routine for converting a multicast IP address to
 a MAC address.
 this routine basically retrieves Drv structure, checks the Drv state and
 calls the pxe_ip2mac routine to actually do the conversion

Arguments:
  This  - context pointer
  IPv6  - flag to indicate if This is an ipv6 address
  IP    - multicast IP address
  MAC   - pointer to hold the return MAC address

Returns:

--*/
;

EFI_STATUS
EFIAPI
Rtl81x9Snp_GetStatus (
  IN EFI_SIMPLE_NETWORK_PROTOCOL * This,
  OUT UINT32                     *InterruptStatusPtr OPTIONAL,
  OUT VOID                       **TransmitBufferListPtr OPTIONAL
  )
/*++

Routine Description:
 This is the SNP interface routine for getting the status
 this routine basically retrieves Drv structure, checks the Drv state and
 calls the pxe_getstatus routine to actually get the undi status

Arguments:
 This  - context pointer
 InterruptStatusPtr - a non null pointer gets the interrupt status
 TransmitBufferListPtrs - a non null ointer gets the list of pointers of previously 
              transmitted buffers whose transmission was completed 
              asynchrnously.
              
Returns:

--*/
;

EFI_STATUS
EFIAPI
Rtl81x9Snp_Transmit (
  IN EFI_SIMPLE_NETWORK_PROTOCOL * This,
  IN UINTN                       MacHeaderSize,
  IN UINTN                       BufferLength,
  IN VOID                        *BufferPtr,
  IN EFI_MAC_ADDRESS             * SourceAddrPtr OPTIONAL,
  IN EFI_MAC_ADDRESS             * DestinationAddrPtr OPTIONAL,
  IN UINT16                      *ProtocolPtr OPTIONAL
  )
/*++

Routine Description:
 This is the SNP interface routine for transmitting a packet. This routine 
 basically retrieves the Drv structure, checks the Drv state and calls
 pxe_fill_header and pxe_transmit calls to complete the transmission.
 
Arguments:
 This - pointer to Drv driver context
 MacHeaderSize - size of the memory at MacHeaderPtr
 BufferLength - Size of data in the BufferPtr
 BufferPtr - data buffer pointer
 SourceAddrPtr - address of the source mac address buffer
 DestinationAddrPtr - address of the destination mac address buffer
 ProtocolPtr - address of the protocol type
 
Returns:
 EFI_SUCCESS - if successfully completed the undi call
 Other - error return from undi call.
 
--*/
;

EFI_STATUS
EFIAPI
Rtl81x9Snp_Receive (
  IN EFI_SIMPLE_NETWORK_PROTOCOL * This,
  OUT UINTN                      *HeaderSizePtr OPTIONAL,
  IN OUT UINTN                   *BuffSizePtr,
  OUT VOID                       *BufferPtr,
  OUT EFI_MAC_ADDRESS            * SourceAddrPtr OPTIONAL,
  OUT EFI_MAC_ADDRESS            * DestinationAddrPtr OPTIONAL,
  OUT UINT16                     *ProtocolPtr OPTIONAL
  )
/*++

Routine Description:
 This is the SNP interface routine for receiving network data.
 this routine basically retrieves Drv structure, checks the Drv state and
 calls the pxe_receive routine to actually do the receive!

Arguments:
  This  - context pointer
  HeaderSizePtr - optional parameter and is a pointer to the header portion of
                the data received.
  BuffSizePtr - is a pointer to the length of the buffer on entry and contains
                the length of the received data on return
  BufferPtr   - pointer to the memory for the received data
  SourceAddrPtr    - optional parameter, is a pointer to contain the source
                ethernet address on return
  DestinationAddrPtr   - optional parameter, is a pointer to contain the destination
                ethernet address on return
  ProtocolPtr    - optional parameter, is a pointer to contain the protocol type
                from the ethernet header on return

Returns:

--*/
;

#endif /*  _RTL81X9_H_  */
