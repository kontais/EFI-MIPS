/*++

Copyright (c) 2005 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

    event.h

Abstract:

  Information about the event function


Revision History

--*/
#ifndef _EVENT_H
#define _EVENT_H

EFI_EVENT
LibCreateProtocolNotifyEvent (
  IN EFI_GUID                       *ProtocolGuid,
  IN EFI_TPL                        NotifyTpl,
  IN EFI_EVENT_NOTIFY               NotifyFunction,
  IN VOID                           *NotifyContext,
  OUT VOID                          **Registration
  );

EFI_STATUS
WaitForSingleEvent (
  IN EFI_EVENT                                Event,
  IN UINT64                                   Timeout OPTIONAL
  );

VOID
WaitForEventWithTimeout (
  IN  EFI_EVENT                     Event,
  IN  UINTN                         Timeout,
  IN  UINTN                         Row,
  IN  UINTN                         Column,
  IN  CHAR16                        *String,
  IN  EFI_INPUT_KEY                 TimeoutKey,
  OUT EFI_INPUT_KEY                 *Key
  );

#endif