/*++

Copyright (c) 2011, kontais
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  Sm712UgaDriver.c

Abstract:

  This file implements the EFI 1.1 Device Driver model requirements for UGA

  UGA is short hand for Universal Graphics Abstraction protocol.

  This file is a verision of UgaIo the uses LinuxThunk system calls as an IO 
  abstraction. For a PCI device LinuxIo would be replaced with
  a PCI IO abstraction that abstracted a specific PCI device. 

--*/

#include "Sm712Uga.h"

EFI_DRIVER_BINDING_PROTOCOL gSm712UgaDriverBinding = {
  Sm712UgaDriverBindingSupported,
  Sm712UgaDriverBindingStart,
  Sm712UgaDriverBindingStop,
  0x10,
  NULL,
  NULL
};

EFI_DRIVER_ENTRY_POINT  (InitializeSm712Uga);

EFI_STATUS
EFIAPI
InitializeSm712Uga(
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
// TODO:    ImageHandle - add argument and description to function comment
// TODO:    SystemTable - add argument and description to function comment
{
  return EfiLibInstallAllDriverProtocols (
          ImageHandle,
          SystemTable,
          &gSm712UgaDriverBinding,
          ImageHandle,
          &gSm712UgaComponentName,
          NULL,
          NULL
          );
}

EFI_STATUS
EFIAPI
Sm712UgaDriverBindingSupported (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      Handle,
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
                  Handle,
                  &gEfiLinuxIoProtocolGuid,
                  &LinuxIo,
                  This->DriverBindingHandle,
                  Handle,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = Sm712UgaSupported (LinuxIo);
  //
  // Close the I/O Abstraction(s) used to perform the supported test
  //
  gBS->CloseProtocol (
        Handle,
        &gEfiLinuxIoProtocolGuid,
        This->DriverBindingHandle,
        Handle
        );

  return Status;
}

EFI_STATUS
EFIAPI
Sm712UgaDriverBindingStart (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      Handle,
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
// TODO:    EFI_UNSUPPORTED - add return value to function comment
{
  EFI_LINUX_IO_PROTOCOL  *LinuxIo;
  EFI_STATUS              Status;
  UGA_PRIVATE_DATA        *Private;

  //
  // Grab the protocols we need
  //
  Status = gBS->OpenProtocol (
                  Handle,
                  &gEfiLinuxIoProtocolGuid,
                  &LinuxIo,
                  This->DriverBindingHandle,
                  Handle,
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
                  sizeof (UGA_PRIVATE_DATA),
                  (VOID**)&Private
                  );
  if (EFI_ERROR (Status)) {
    goto Done;
  }
  //
  // Set up context record
  //
  Private->Signature            = UGA_PRIVATE_DATA_SIGNATURE;
  Private->Handle               = Handle;
//  Private->LinuxThunk           = LinuxIo->LinuxThunk;

  Private->ControllerNameTable  = NULL;

  EfiLibAddUnicodeString (
    "eng",
    gSm712UgaComponentName.SupportedLanguages,
    &Private->ControllerNameTable,
    LinuxIo->EnvString
    );

//  Private->WindowName = LinuxIo->EnvString;

  Status              = Sm712UgaConstructor (Private);
  if (EFI_ERROR (Status)) {
    goto Done;
  }
  //
  // Publish the Uga interface to the world
  //
  Status = gBS->InstallProtocolInterface (
                  &Private->Handle,
                  &gEfiUgaDrawProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &Private->UgaDraw
                  );

Done:
  if (EFI_ERROR (Status)) {
    gBS->CloseProtocol (
          Handle,
          &gEfiLinuxIoProtocolGuid,
          This->DriverBindingHandle,
          Handle
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
Sm712UgaDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN  EFI_HANDLE                   Handle,
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
  EFI_UGA_DRAW_PROTOCOL *UgaDraw;
  EFI_STATUS            Status;
  UGA_PRIVATE_DATA      *Private;

  Status = gBS->OpenProtocol (
                  Handle,
                  &gEfiUgaDrawProtocolGuid,
                  (VOID**)&UgaDraw,
                  This->DriverBindingHandle,
                  Handle,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    //
    // If the UGA interface does not exist the driver is not started
    //
    return EFI_NOT_STARTED;
  }

  //
  // Get our private context information
  //
  Private = UGA_DRAW_PRIVATE_DATA_FROM_THIS (UgaDraw);

  //
  // Remove the SGO interface from the system
  //
  Status = gBS->UninstallProtocolInterface (
                  Private->Handle,
                  &gEfiUgaDrawProtocolGuid,
                  &Private->UgaDraw
                  );
  if (!EFI_ERROR (Status)) {
    //
    // Shutdown the hardware
    //
    Status = Sm712UgaDestructor (Private);
    if (EFI_ERROR (Status)) {
      return EFI_DEVICE_ERROR;
    }
    gBS->CloseProtocol (
          Handle,
          &gEfiLinuxIoProtocolGuid,
          This->DriverBindingHandle,
          Handle
          );

    //
    // Free our instance data
    //
    EfiLibFreeUnicodeStringTable (Private->ControllerNameTable);

    gBS->FreePool (Private);

  }

  return Status;
}

