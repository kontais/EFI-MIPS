/*++

Copyright (c) 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  ConsoleIn.c

Abstract:

  Console based on Win32 APIs. 

  This file attaches a SimpleTextIn protocol to a previously open window.
  
  The constructor for this protocol depends on an open window. Currently
  the SimpleTextOut protocol creates a window when it's constructor is called.
  Thus this code must run after the constructor for the SimpleTextOut 
  protocol
  
--*/

#include "Console.h"
#include <sys/poll.h>

//
// Private worker functions
//
STATIC
EFI_STATUS
LinuxSimpleTextInCheckKey (
  LINUX_SIMPLE_TEXT_PRIVATE_DATA  *Private
  );

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

  EFI_SUCCESS - TODO: Add description for return value

--*/
{
  LINUX_SIMPLE_TEXT_PRIVATE_DATA *Private;

  Private = LINUX_SIMPLE_TEXT_IN_PRIVATE_DATA_FROM_THIS (This);
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
LinuxConvertInputRecordToEfiKey (
  IN  char c,
  OUT EFI_INPUT_KEY   *Key
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  InputRecord - TODO: add argument description
  Key         - TODO: add argument description

Returns:

  EFI_NOT_READY - TODO: Add description for return value
  EFI_NOT_READY - TODO: Add description for return value
  EFI_NOT_READY - TODO: Add description for return value
  EFI_SUCCESS - TODO: Add description for return value

--*/
{
  Key->ScanCode     = 0;
  if (c == '\n')
    c = '\r';
  Key->UnicodeChar  = c;
  return EFI_SUCCESS;
}

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

  EFI_DEVICE_ERROR - TODO: Add description for return value
  EFI_NOT_READY - TODO: Add description for return value

--*/
{
  EFI_STATUS                      Status;
  LINUX_SIMPLE_TEXT_PRIVATE_DATA *Private;
  char c;

  Private = LINUX_SIMPLE_TEXT_IN_PRIVATE_DATA_FROM_THIS (This);

  Status  = LinuxSimpleTextInCheckKey (Private);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (Private->LinuxThunk->Read (0, &c, 1) != 1)
    return EFI_NOT_READY;
  Status = LinuxConvertInputRecordToEfiKey (c, Key);

  return Status;
}

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
{
  LINUX_SIMPLE_TEXT_PRIVATE_DATA *Private;
  EFI_STATUS                      Status;

  Private = (LINUX_SIMPLE_TEXT_PRIVATE_DATA *) Context;
  Status  = LinuxSimpleTextInCheckKey (Private);
  if (!EFI_ERROR (Status)) {
    gBS->SignalEvent (Event);
  }
}

STATIC
EFI_STATUS
LinuxSimpleTextInCheckKey (
  LINUX_SIMPLE_TEXT_PRIVATE_DATA   *Private
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  Private - TODO: add argument description

Returns:

  TODO: add return values

--*/
{
  struct pollfd pfd;

  pfd.fd = 0;
  pfd.events = POLLIN;
  if (Private->LinuxThunk->poll (&pfd, 1, 0) <= 0) {
    return EFI_NOT_READY;
  }
  return EFI_SUCCESS;
}

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
{
  EFI_STATUS  Status;

  Private->SimpleTextIn.Reset         = LinuxSimpleTextInReset;
  Private->SimpleTextIn.ReadKeyStroke = LinuxSimpleTextInReadKeyStroke;

  Status = gBS->CreateEvent (
                  EFI_EVENT_NOTIFY_WAIT,
                  EFI_TPL_NOTIFY,
                  LinuxSimpleTextInWaitForKey,
                  Private,
                  &Private->SimpleTextIn.WaitForKey
                  );
  ASSERT_EFI_ERROR (Status);

  return Status;
}
