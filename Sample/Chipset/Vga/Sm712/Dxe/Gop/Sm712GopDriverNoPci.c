/*++
Copyright (c) 2012, kontais
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  Sm712GopDriver.c

Abstract:

  This file implements the UEFI Device Driver model requirements for GOP

  GOP is short hand for Graphics Output Protocol.

--*/

#include "Sm712Gop.h"
#include "Sm712Dev.h"

EFI_DRIVER_BINDING_PROTOCOL gSm712GopDriverBinding = {
  Sm712GopDriverBindingSupported,
  Sm712GopDriverBindingStart,
  Sm712GopDriverBindingStop,
  0x10,
  NULL,
  NULL
};

//
//  Module Global Variables
//
EFI_PCI_IO_PROTOCOL         *mPciIo;

EFI_DRIVER_ENTRY_POINT  (InitializeSm712Gop);

EFI_STATUS
EFIAPI
InitializeSm712Gop(
  IN EFI_HANDLE            ImageHandle,
  IN EFI_SYSTEM_TABLE      *SystemTable
  )
/*++

Routine Description:

  Intialize Win32 windows to act as EFI SimpleTextOut and SimpleTextIn windows. . 

Arguments:

  (Standard EFI Image entry - EFI_IMAGE_ENTRY_POINT)

Returns: 

  EFI_STATUS

--*/
{
  return EfiLibInstallAllDriverProtocols (
          ImageHandle,
          SystemTable,
          &gSm712GopDriverBinding,
          ImageHandle,
          &gSm712GopComponentName,
          NULL,
          NULL
          );
}

EFI_STATUS
EFIAPI
Sm712GopDriverBindingSupported (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      Controller,
  IN  EFI_DEVICE_PATH_PROTOCOL        *RemainingDevicePath
  )
/*++

Routine Description:

Arguments:

Returns:

  None

--*/
// TODO:    This - add argument and description to function comment
// TODO:    Handle - add argument and description to function comment
// TODO:    RemainingDevicePath - add argument and description to function comment
{
  EFI_STATUS              Status;
  EFI_LINUX_IO_PROTOCOL  *LinuxIo;

  //
  // Open the IO Abstraction(s) needed to perform the supported test
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiLinuxIoProtocolGuid,
                  &LinuxIo,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = Sm712GopSupported (LinuxIo);
  //
  // Close the I/O Abstraction(s) used to perform the supported test
  //
  gBS->CloseProtocol (
        Controller,
        &gEfiLinuxIoProtocolGuid,
        This->DriverBindingHandle,
        Controller
        );

  return Status;
}

EFI_STATUS
EFIAPI
Sm712GopDriverBindingStart (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      Controller,
  IN  EFI_DEVICE_PATH_PROTOCOL        *RemainingDevicePath
  )
/*++

Routine Description:

Arguments:

Returns:

  None

--*/
// TODO:    This - add argument and description to function comment
// TODO:    Controller - add argument and description to function comment
// TODO:    RemainingDevicePath - add argument and description to function comment
// TODO:    EFI_UNSUPPORTED - add return value to function comment
{
  EFI_LINUX_IO_PROTOCOL  *LinuxIo;
  EFI_STATUS              Status;
  SM712_GOP_PRIVATE_DATA        *Private;

  //
  // Grab the protocols we need
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiLinuxIoProtocolGuid,
                  &LinuxIo,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    return EFI_UNSUPPORTED;
  }

  //
  // Allocate Private context data for SGO inteface.
  //
  Private = NULL;
  Status = gBS->AllocatePool (
                  EfiBootServicesData,
                  sizeof (SM712_GOP_PRIVATE_DATA),
                  (VOID**)&Private
                  );
  if (EFI_ERROR (Status)) {
    goto Done;
  }
  //
  // Set up context record
  //
  Private->Signature            = SM712_GOP_PRIVATE_DATA_SIGNATURE;
  Private->Handle               = Controller;

  Private->ControllerNameTable  = NULL;

  EfiLibAddUnicodeString (
    "eng",
    gSm712GopComponentName.SupportedLanguages,
    &Private->ControllerNameTable,
    LinuxIo->EnvString
    );


  Status              = Sm712GopConstructor (Private);
  if (EFI_ERROR (Status)) {
    goto Done;
  }
  //
  // Publish the Sm712 Gop interface to the world
  //
  Status = gBS->InstallProtocolInterface (
                  &Private->Handle,
                  &gEfiGraphicsOutputProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &Private->GraphicsOutput
                  );

Done:
  if (EFI_ERROR (Status)) {
    gBS->CloseProtocol (
          Controller,
          &gEfiLinuxIoProtocolGuid,
          This->DriverBindingHandle,
          Controller
          );

    if (Private != NULL) {
      //
      // On Error Free back private data
      //
      if (Private->ControllerNameTable != NULL) {
        EfiLibFreeUnicodeStringTable (Private->ControllerNameTable);
      }

      gBS->FreePool (Private);
    }
  }

  return Status;
}

EFI_STATUS
EFIAPI
Sm712GopDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN  EFI_HANDLE                   Controller,
  IN  UINTN                        NumberOfChildren,
  IN  EFI_HANDLE                   *ChildHandleBuffer
  )
/*++

Routine Description:

Arguments:

Returns:

  None

--*/
// TODO:    This - add argument and description to function comment
// TODO:    Handle - add argument and description to function comment
// TODO:    NumberOfChildren - add argument and description to function comment
// TODO:    ChildHandleBuffer - add argument and description to function comment
// TODO:    EFI_NOT_STARTED - add return value to function comment
// TODO:    EFI_DEVICE_ERROR - add return value to function comment
{
  EFI_STATUS                   Status;
  EFI_GRAPHICS_OUTPUT_PROTOCOL *Gop;
  SM712_GOP_PRIVATE_DATA       *Private;

  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiGraphicsOutputProtocolGuid,
                  (VOID**)&Gop,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    //
    // If the GOP interface does not exist the driver is not started
    //
    return EFI_NOT_STARTED;
  }

  //
  // Get our private context information
  //
  Private = SM712_GOP_PRIVATE_DATA_FROM_THIS (Gop);

  //
  // Remove the GOP interface from the system
  //
  Status = gBS->UninstallProtocolInterface (
                  Private->Handle,
                  &gEfiGraphicsOutputProtocolGuid,
                  &Private->GraphicsOutput
                  );
  if (!EFI_ERROR (Status)) {
    gBS->CloseProtocol (
          Controller,
          &gEfiLinuxIoProtocolGuid,
          This->DriverBindingHandle,
          Controller
          );
    gBS->FreePool (Private);
  }

  return Status;
}

