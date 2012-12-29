/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  TelnetServer.h
  
Abstract:

  Private Data definition for Telnet Server driver

--*/

#ifndef _EFI_TELNET_SERVER_PROTOCOL_H_
#define _EFI_TELNET_SERVER_PROTOCOL_H_

#define EFI_TELNET_SERVER_PROTOCOL_GUID \
  { \
    0x6d3569d4, 0x85e5, 0x4943, 0xae, 0x46, 0xee, 0x67, 0xa6, 0xe1, 0xab, 0x5a \
  }

EFI_FORWARD_DECLARATION (EFI_TELNET_SERVER_PROTOCOL);

//
// Telnet Options (used in conjunction with Telnet Commands)
//
#define TRANSMIT_BINARY (UINT8) 0 // Transmit in 8-bit binary form
#define ECHO            (UINT8) 1 // Allow one side to echo data it receives
#define SUPPRESS_GA     (UINT8) 3 // Suppress Go-ahead signal after data
#define STATUS          (UINT8) 5 // Request status of TELNET option from
// remote
//
#define TIMING_MARK (UINT8) 6 // Request timing mark be inserted in return
//  stream to synch two ends of connection
//
#define TERMINAL_TYPE (UINT8) 24  // Exchange information about the make and
// model of a terminal being used
//
#define END_OF_RECORD (UINT8) 25  // Terminate data sent with EOR code
#define WINDOW_SIZE   (UINT8) 31  // Send sub-option with windowing size back
#define LINE_MODE     (UINT8) 34  // Send complete lines instead of
// individual characters
//
// Supported terminal type
//
#define PcAnsiType    0
#define VT100Type     1
#define VT100PlusType 2
#define VTUTF8Type    3

typedef
EFI_STATUS
(EFIAPI *EFI_TELNET_SERVER_SET_OPTION) (
  IN EFI_TELNET_SERVER_PROTOCOL           * This,
  IN  UINT8                               OptionCode,
  IN  UINTN                               OptionValue
  );

//
// Efi Telnet Server Protocol structure
//
#define EFI_TELNET_SERVER_REVISION  0x00010000

typedef struct _EFI_TELNET_SERVER_PROTOCOL {
  UINT64                        Revision;
  UINT8                         TerminalType;
  EFI_TELNET_SERVER_SET_OPTION  SetOption;
} EFI_TELNET_SERVER_PROTOCOL;

//
// Global Variables
//
extern EFI_GUID gEfiTelnetServerGuid;

#endif
