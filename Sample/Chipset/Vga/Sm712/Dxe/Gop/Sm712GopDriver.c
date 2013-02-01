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
// TODO:    Controller - add argument and description to function comment
// TODO:    RemainingDevicePath - add argument and description to function comment
{
  EFI_STATUS                Status;
  EFI_PCI_IO_PROTOCOL       *PciIo;
  PCI_TYPE00                PciData;

  //
  // Open the IO Abstraction(s) needed to perform the supported test
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiDevicePathProtocolGuid,
                  NULL,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_TEST_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Now test the EfiPciIoProtocol
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiPciIoProtocolGuid,
                  (VOID **) &PciIo,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Now further check the PCI header: Vendor Id , Device Id and Revision id
  //
  Status = PciIo->Pci.Read (
                        PciIo,
                        EfiPciIoWidthUint8,
                        0,
                        sizeof (PciData),
                        &PciData
                        );

  if (EFI_ERROR (Status)) {
    gBS->CloseProtocol (
          Controller,
          &gEfiPciIoProtocolGuid,
          This->DriverBindingHandle,
          Controller
          );
    return EFI_UNSUPPORTED;
  }
  //
  // Examine SM712 PCI Configuration table fields
  //
  if ((PciData.Hdr.VendorId != SM712_VENDORID) ||
      (PciData.Hdr.DeviceId != SM712_DEVICEID)) {
    Status = EFI_UNSUPPORTED;
  }

  gBS->CloseProtocol (
        Controller,
        &gEfiPciIoProtocolGuid,
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
  EFI_STATUS                Status;
  SM712_GOP_PRIVATE_DATA    *Private;

  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiDevicePathProtocolGuid,
                  NULL,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_TEST_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiPciIoProtocolGuid,
                  (VOID**)&mPciIo,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    return Status;
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
  EfiZeroMem (Private, sizeof (SM712_GOP_PRIVATE_DATA));
  Private->Signature  = SM712_GOP_PRIVATE_DATA_SIGNATURE;
  Private->Handle     = Controller;
  Private->PciIo      = mPciIo;
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
          &gEfiPciIoProtocolGuid,
          This->DriverBindingHandle,
          Controller
          );
    if (Private != NULL) {
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
// TODO:    Controller - add argument and description to function comment
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
          &gEfiPciIoProtocolGuid,
          This->DriverBindingHandle,
          Controller
          );
    gBS->FreePool (Private);
  }

  return Status;
}

