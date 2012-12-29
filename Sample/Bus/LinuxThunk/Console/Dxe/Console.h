/*++

Copyright (c) 2004 - 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  Console.h

Abstract:

  Console based on Win32 APIs.

  This file attaches a SimpleTextIn protocol to a previously open window.
  
  The constructor for this protocol depends on an open window. Currently
  the SimpleTextOut protocol creates a window when it's constructor is called.
  Thus this code must run after the constructor for the SimpleTextOut 
  protocol
  
--*/

#ifndef _CONSOLE_H_
#define _CONSOLE_H_

#include "EfiLinux.h"
#include "EfiDriverLib.h"

//
// Driver Consumed Protocols
//
#include EFI_PROTOCOL_DEFINITION (DevicePath)
#include EFI_PROTOCOL_DEFINITION (LinuxIo)

//
// Driver Produced Protocols
//
#include EFI_PROTOCOL_DEFINITION (DriverBinding)
#include EFI_PROTOCOL_DEFINITION (ComponentName)
#include EFI_PROTOCOL_DEFINITION (SimpleTextOut)

#define LINUX_SIMPLE_TEXT_PRIVATE_DATA_SIGNATURE \
          EFI_SIGNATURE_32('L','X','s','c')

typedef struct {
  UINT64                        Signature;

  EFI_HANDLE                    Handle;

  EFI_SIMPLE_TEXT_OUT_PROTOCOL  SimpleTextOut;
  EFI_SIMPLE_TEXT_OUTPUT_MODE   SimpleTextOutMode;

  EFI_LINUX_IO_PROTOCOL        *LinuxIo;
  EFI_LINUX_THUNK_PROTOCOL     *LinuxThunk;

  //
  // SimpleTextOut Private Data including Win32 types.
  //
  //  HANDLE                        NtOutHandle;
  //  HANDLE                        NtInHandle;

  //COORD                         MaxScreenSize;
  //COORD                         Position;
  //WORD                          Attribute;
  BOOLEAN                       CursorEnable;

  EFI_SIMPLE_TEXT_IN_PROTOCOL   SimpleTextIn;

  EFI_UNICODE_STRING_TABLE      *ControllerNameTable;

} LINUX_SIMPLE_TEXT_PRIVATE_DATA;

#define LINUX_SIMPLE_TEXT_OUT_PRIVATE_DATA_FROM_THIS(a) \
         CR(a, LINUX_SIMPLE_TEXT_PRIVATE_DATA, SimpleTextOut, LINUX_SIMPLE_TEXT_PRIVATE_DATA_SIGNATURE)

#define LINUX_SIMPLE_TEXT_IN_PRIVATE_DATA_FROM_THIS(a) \
         CR(a, LINUX_SIMPLE_TEXT_PRIVATE_DATA, SimpleTextIn, LINUX_SIMPLE_TEXT_PRIVATE_DATA_SIGNATURE)

//
// Console Globale Variables
//
extern EFI_DRIVER_BINDING_PROTOCOL  gLinuxConsoleDriverBinding;
extern EFI_COMPONENT_NAME_PROTOCOL  gLinuxConsoleComponentName;

typedef struct {
  UINTN ColumnsX;
  UINTN RowsY;
} LINUX_SIMPLE_TEXT_OUT_MODE;

#if 0
//
// Simple Text Out protocol member functions
//

STATIC
EFI_STATUS
EFIAPI
LinuxSimpleTextOutReset (
  IN EFI_SIMPLE_TEXT_OUT_PROTOCOL     *This,
  IN BOOLEAN                          ExtendedVerification
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  This                  - TODO: add argument description
  ExtendedVerification  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

STATIC
EFI_STATUS
EFIAPI
LinuxSimpleTextOutOutputString (
  IN EFI_SIMPLE_TEXT_OUT_PROTOCOL   *This,
  IN CHAR16                         *String
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  This    - TODO: add argument description
  String  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

STATIC
EFI_STATUS
EFIAPI
LinuxSimpleTextOutTestString (
  IN EFI_SIMPLE_TEXT_OUT_PROTOCOL   *This,
  IN CHAR16                         *String
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  This    - TODO: add argument description
  String  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

STATIC
EFI_STATUS
EFIAPI
LinuxSimpleTextOutQueryMode (
  IN EFI_SIMPLE_TEXT_OUT_PROTOCOL   *This,
  IN UINTN                          ModeNumber,
  OUT UINTN                         *Columns,
  OUT UINTN                         *Rows
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  This        - TODO: add argument description
  ModeNumber  - TODO: add argument description
  Columns     - TODO: add argument description
  Rows        - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
EFIAPI
LinuxSimpleTextOutSetMode (
  IN EFI_SIMPLE_TEXT_OUT_PROTOCOL   *This,
  IN UINTN                          ModeNumber
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  This        - TODO: add argument description
  ModeNumber  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

STATIC
EFI_STATUS
EFIAPI
LinuxSimpleTextOutSetAttribute (
  IN EFI_SIMPLE_TEXT_OUT_PROTOCOL   *This,
  IN UINTN                          Attribute
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  This      - TODO: add argument description
  Attribute - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

STATIC
EFI_STATUS
EFIAPI
LinuxSimpleTextOutClearScreen (
  IN EFI_SIMPLE_TEXT_OUT_PROTOCOL   *This
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  This  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

STATIC
EFI_STATUS
EFIAPI
LinuxSimpleTextOutSetCursorPosition (
  IN EFI_SIMPLE_TEXT_OUT_PROTOCOL   *This,
  IN UINTN                          Column,
  IN UINTN                          Row
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  This    - TODO: add argument description
  Column  - TODO: add argument description
  Row     - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

STATIC
EFI_STATUS
EFIAPI
LinuxSimpleTextOutEnableCursor (
  IN EFI_SIMPLE_TEXT_OUT_PROTOCOL   *This,
  IN BOOLEAN                        Enable
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  This    - TODO: add argument description
  Enable  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

#endif
//
// Simple Text Out constructor and destructor.
//
EFI_STATUS
LinuxSimpleTextOutOpenWindow (
  IN OUT  LINUX_SIMPLE_TEXT_PRIVATE_DATA *Private
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  Private - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
LinuxSimpleTextOutCloseWindow (
  IN OUT  LINUX_SIMPLE_TEXT_PRIVATE_DATA *Console
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  Console - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

#if 0
//
// Simple Text In protocol member functions.
//
STATIC
EFI_STATUS
EFIAPI
LinuxSimpleTextInReset (
  IN EFI_SIMPLE_TEXT_IN_PROTOCOL          *This,
  IN BOOLEAN                              ExtendedVerification
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  This                  - TODO: add argument description
  ExtendedVerification  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

STATIC
EFI_STATUS
EFIAPI
LinuxSimpleTextInReadKeyStroke (
  IN EFI_SIMPLE_TEXT_IN_PROTOCOL          *This,
  OUT EFI_INPUT_KEY                       *Key
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  This  - TODO: add argument description
  Key   - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

STATIC
VOID
EFIAPI
LinuxSimpleTextInWaitForKey (
  IN EFI_EVENT          Event,
  IN VOID               *Context
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  Event   - TODO: add argument description
  Context - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

#endif
//
// Simple Text In constructor
//
EFI_STATUS
LinuxSimpleTextInAttachToWindow (
  IN  LINUX_SIMPLE_TEXT_PRIVATE_DATA *Private
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  Private - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

//
// Main Entry Point
//
EFI_STATUS
EFIAPI
InitializeLinuxConsole (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  ImageHandle - TODO: add argument description
  SystemTable - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
AppendDevicePathInstanceToVar (
  IN  CHAR16                    *VariableName,
  IN  EFI_DEVICE_PATH_PROTOCOL  *DevicePathInstance
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  VariableName        - TODO: add argument description
  DevicePathInstance  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

#endif
