/*++

Copyright (c) 2005 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

    Event.c

Abstract:

  The event function

Revision History

--*/

#include "EfiShellLib.h"

EFI_EVENT
LibCreateProtocolNotifyEvent (
  IN EFI_GUID             *ProtocolGuid,
  IN EFI_TPL              NotifyTpl,
  IN EFI_EVENT_NOTIFY     NotifyFunction,
  IN VOID                 *NotifyContext,
  OUT VOID                **Registration
  )
/*++

Routine Description:
  Function creates a notification event and registers that event with all 
  the protocol instances specified by ProtocolGuid.   

Arguments:
  ProtocolGuid     - Supplies GUID of the protocol upon whose installation the event is fired.

  NotifyTpl        - Supplies the task priority level of the event notifications.

  NotifyFunction   - Supplies the function to notify when the event is signaled.

  NotifyContext    - The context parameter to pass to NotifyFunction. 

  Registration     - A pointer to a memory location to receive the registration value.  
                     This value is passed to LibLocateHandle() to obtain new handles that 
                     have been added that support the ProtocolGuid-specified protocol.

Returns:

  This function returns the notification event that was created.

--*/
{
  EFI_STATUS  Status;
  EFI_EVENT   Event;

  //
  // Create the event
  //
  Status = BS->CreateEvent (
                EFI_EVENT_NOTIFY_SIGNAL,
                NotifyTpl,
                NotifyFunction,
                NotifyContext,
                &Event
                );
  if (EFI_ERROR (Status)) {
    return NULL;
  }
  //
  // Register for protocol notifactions on this event
  //
  Status = BS->RegisterProtocolNotify (
                ProtocolGuid,
                Event,
                Registration
                );

  if (EFI_ERROR (Status)) {
    return NULL;
  }
  //
  // Kick the event so we will perform an initial pass of
  // current installed drivers
  //
  BS->SignalEvent (Event);
  return Event;
}

VOID
WaitForEventWithTimeout (
  IN  EFI_EVENT       Event,
  IN  UINTN           Timeout,
  IN  UINTN           Row,
  IN  UINTN           Column,
  IN  CHAR16          *String,
  IN  EFI_INPUT_KEY   TimeoutKey,
  OUT EFI_INPUT_KEY   *Key
  )
/*++

Routine Description:
  function prints a string for the given number of seconds until either the 
  timeout expires, or the user presses a key.

Arguments:
  Event            - The event to wait for

  Timeout          - A timeout value in 1 second units.

  Row              - A row to print String

  Column           - A column to print String

  String           - The string to display on the standard output device

  TimeoutKey       - The key to return in Key if a timeout occurs

  Key              - Either the key the user pressed or TimeoutKey if the Timeout expired.

Returns:

  none

--*/
{
  EFI_STATUS  Status;

  ASSERT (Key);
  
  while (Timeout > 0) {
    PrintAt (Column, Row, String, Timeout);
    Status = WaitForSingleEvent (Event, 10000000);
    if (Status == EFI_SUCCESS) {
      if (!EFI_ERROR (ST->ConIn->ReadKeyStroke (ST->ConIn, Key))) {
        return ;
      }
    }

    Timeout--;
  }

  *Key = TimeoutKey;
}

EFI_STATUS
WaitForSingleEvent (
  IN EFI_EVENT                  Event,
  IN UINT64                     Timeout OPTIONAL
  )
/*++

Routine Description:
  Function waits for a given event to fire, or for an optional timeout to expire.

Arguments:
  Event            - The event to wait for

  Timeout          - An optional timeout value in 100 ns units.

Returns:

  EFI_SUCCESS       - Event fired before Timeout expired.
  EFI_TIME_OUT     - Timout expired before Event fired..

--*/
{
  EFI_STATUS  Status;
  UINTN       Index;
  EFI_EVENT   TimerEvent;
  EFI_EVENT   WaitList[2];

  if (Timeout) {
    //
    // Create a timer event
    //
    Status = BS->CreateEvent (EFI_EVENT_TIMER, 0, NULL, NULL, &TimerEvent);
    if (!EFI_ERROR (Status)) {
      //
      // Set the timer event
      //
      BS->SetTimer (
            TimerEvent,
            TimerRelative,
            Timeout
            );

      //
      // Wait for the original event or the timer
      //
      WaitList[0] = Event;
      WaitList[1] = TimerEvent;
      Status      = BS->WaitForEvent (2, WaitList, &Index);
      BS->CloseEvent (TimerEvent);

      //
      // If the timer expired, change the return to timed out
      //
      if (!EFI_ERROR (Status) && Index == 1) {
        Status = EFI_TIMEOUT;
      }
    }
  } else {
    //
    // No timeout... just wait on the event
    //
    Status = BS->WaitForEvent (1, &Event, &Index);
    ASSERT (!EFI_ERROR (Status));
    ASSERT (Index == 0);
  }

  return Status;
}

