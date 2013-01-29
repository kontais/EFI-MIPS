/*++
Copyright (c) 2012, kontais
Copyright (c) 2004 - 2005, Intel Corporation
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

Module name:
    Rtl81x9.c

Abstract:

--*/

#include "Rtl81x9.h"
#include "Rtl81x9Dev.h"

EFI_DRIVER_ENTRY_POINT (InitializeRtl81x9Driver)

EFI_STATUS
EFIAPI
InitializeRtl81x9Driver (
  IN EFI_HANDLE       ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable
  );

EFI_STATUS
EFIAPI
Rtl81x9SimpleNetworkDriverSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  );

EFI_STATUS
EFIAPI
Rtl81x9SimpleNetworkDriverStart (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  );

EFI_STATUS
EFIAPI
Rtl81x9SimpleNetworkDriverStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN  EFI_HANDLE                     Controller,
  IN  UINTN                          NumberOfChildren,
  IN  EFI_HANDLE                     *ChildHandleBuffer
  );

//
// Simple Network Protocol Driver Global Variables
//
EFI_DRIVER_BINDING_PROTOCOL mRtl81x9SimpleNetworkDriverBinding = {
  Rtl81x9SimpleNetworkDriverSupported,
  Rtl81x9SimpleNetworkDriverStart,
  Rtl81x9SimpleNetworkDriverStop,
  0xa,
  NULL,
  NULL
};

//
//  Module Global Variables
//
EFI_PCI_IO_PROTOCOL         *mPciIo;

EFI_STATUS
EFIAPI
Rtl81x9SimpleNetworkDriverSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  )
/*++

  Routine Description:
    Test to see if this driver supports Controller. Any Controller
    that contains a Nii protocol can be supported.

  Arguments:
    This                - Protocol instance pointer.
    Controller          - Handle of device to test.
    RemainingDevicePath - Not used.

  Returns:
    EFI_SUCCESS         - This driver supports this device.
    EFI_ALREADY_STARTED - This driver is already running on this device.
    other               - This driver does not support this device.

--*/
{
  EFI_STATUS                Status;
  EFI_DEVICE_PATH_PROTOCOL  *ParentDevicePath;
  EFI_DEV_PATH              *Node;
  EFI_PCI_IO_PROTOCOL       *PciIo;
  PCI_TYPE00                PciData;


  if (RemainingDevicePath != NULL) {
    Node = (EFI_DEV_PATH *) RemainingDevicePath;
    if (Node->DevPath.Type != MESSAGING_DEVICE_PATH ||
        Node->DevPath.SubType != MSG_MAC_ADDR_DP ||
        DevicePathNodeLength(&Node->DevPath) != sizeof(MAC_ADDR_DEVICE_PATH)) {
      return EFI_UNSUPPORTED;
    }
  }

  //
  // Open the IO Abstraction(s) needed to perform the supported test
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiDevicePathProtocolGuid,
                  (VOID **) &ParentDevicePath,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (Status == EFI_ALREADY_STARTED) {
    return EFI_SUCCESS;
  }

  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Clsoe protocol, don't use device path protocol in the .Support() function
  //
  gBS->CloseProtocol (
        Controller,
        &gEfiDevicePathProtocolGuid,
        This->DriverBindingHandle,
        Controller
        );

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
  // Examine NIC PCI Configuration table fields
  //
  if ((PciData.Hdr.VendorId != RT_VENDORID) ||
      (PciData.Hdr.DeviceId != RT_DEVICEID_8139) ||
      (PciData.Hdr.RevisionID != 0x10)
      ) {

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
Rtl81x9SimpleNetworkDriverStart (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  )
/*++

Routine Description:
 called for any handle that we said "supported" in the above call!

Arguments:
    This                - Protocol instance pointer.
    Controller          - Handle of device to start
    RemainingDevicePath - Not used.

  Returns:
    EFI_SUCCESS         - This driver supports this device.
    other               - This driver failed to start this device.

--*/
{
  EFI_STATUS                Status;
  EFI_DEVICE_PATH_PROTOCOL  *ParentDevicePath;
  RTL81X9_SNP_DRIVER        *Rtl81x9Private;
  EFI_DEV_PATH              NewNode;
  UINTN                     Size;
  RTL81X9_CHAIN_DATA        *TxRxChain;

  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiDevicePathProtocolGuid,
                  (VOID **) &ParentDevicePath,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
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
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Allocate Ide private data structure
  //
  Status = gBS->AllocatePool (
                  EfiBootServicesData,
                  sizeof (RTL81X9_SNP_DRIVER),
                  (VOID **) &Rtl81x9Private
                  );
  if (EFI_ERROR (Status)) {
    goto StartErrorExit;
  }

  EfiZeroMem (Rtl81x9Private, sizeof (RTL81X9_SNP_DRIVER));
  EfiCopyMem (&Rtl81x9Private->Snp, &Rtl81x9SimpleNetworkProtocol, sizeof(EFI_SIMPLE_NETWORK_PROTOCOL));

  Rtl81x9Private->Signature       = RTL81X9_SNP_DRIVER_SIGNATURE;
  Rtl81x9Private->Snp.Mode        = &Rtl81x9Private->Mode;
  Rtl81x9Private->Nii.Revision    = EFI_NETWORK_INTERFACE_IDENTIFIER_PROTOCOL_REVISION_31;
  Rtl81x9Private->Nii.Type        = EfiNetworkInterfaceSnp;
  Rtl81x9Private->Nii.StringId[0] = 'S';
  Rtl81x9Private->Nii.StringId[1] = 'N';
  Rtl81x9Private->Nii.StringId[2] = 'P';
  Rtl81x9Private->Nii.StringId[3] = 'N';
  Rtl81x9Private->PciIo           = mPciIo;

  TxRxChain = &(Rtl81x9Private->ChainData);
  //
  // Allocate Tx/Rx Buffer
  //
  Size = EFI_SIZE_TO_PAGES(RL_TXRX_BUFLEN);
  Status = mPciIo->AllocateBuffer (
                        mPciIo,
                        AllocateAnyPages,
                        EfiBootServicesData,
                        Size,
                        (VOID **)&(TxRxChain->BufferPtr),
                        0
                        );

  if (Status != EFI_SUCCESS) {
    goto AllocateError;
  }

  TxRxChain->Buffer = TxRxChain->BufferPtr;

  Status = mPciIo->Map (
                        mPciIo,
                        EfiPciIoOperationBusMasterCommonBuffer,
                        TxRxChain->Buffer,
                        &Size,
                        (EFI_PHYSICAL_ADDRESS *)&(TxRxChain->BufferDma),
                        &(TxRxChain->BufferDmaUnmap)
                        );
  if (Status != EFI_SUCCESS) {
    goto AllocateError;
  }

  Rtl81x9DevInit (Rtl81x9Private);

  Rtl81x9Init (Rtl81x9Private, PXE_OPFLAGS_INITIALIZE_DO_NOT_DETECT_CABLE);

  Rtl81x9Private->Mode.MediaPresent = FALSE;

  Rtl81x9Shutdown (Rtl81x9Private);
  Rtl81x9Stop (Rtl81x9Private);

  //
  // Device Path
  //
  EfiZeroMem (&NewNode, sizeof (NewNode));
  NewNode.DevPath.Type    = MESSAGING_DEVICE_PATH;
  NewNode.DevPath.SubType = MSG_MAC_ADDR_DP;
  SetDevicePathNodeLength (&NewNode.DevPath, sizeof (MAC_ADDR_DEVICE_PATH));

  EfiCopyMem (&(NewNode.MacAddr.MacAddress),
              &(Rtl81x9Private->Mode.CurrentAddress),
              Rtl81x9Private->Mode.HwAddressSize);
  NewNode.MacAddr.IfType = Rtl81x9Private->Mode.IfType;

  Rtl81x9Private->DevicePath = EfiAppendDevicePathNode (
                                 ParentDevicePath,
                                 &NewNode.DevPath
                                 );
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Rtl81x9Private->DeviceHandle,
                  &gEfiSimpleNetworkProtocolGuid,
                  &(Rtl81x9Private->Snp),
                  &gEfiDevicePathProtocolGuid,
                  Rtl81x9Private->DevicePath,
/* Install Nii will active PXE
                  &gEfiNetworkInterfaceIdentifierProtocolGuid_31,
                  &Rtl81x9Private->Nii,
*/
                  NULL
                  );

  if (!EFI_ERROR (Status)) {
    Status = gBS->OpenProtocol (
                    Controller,           
                    &gEfiPciIoProtocolGuid, 
                    (VOID **)&Rtl81x9Private->PciIo,
                    mRtl81x9SimpleNetworkDriverBinding.DriverBindingHandle,
                    Rtl81x9Private->DeviceHandle,   
                    EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER
                    );
    if (!EFI_ERROR (Status)) {
      return Status;
    }
  }

AllocateError:
  if (TxRxChain->BufferPtr != NULL) {
    mPciIo->FreeBuffer (
                  mPciIo,
                  EFI_SIZE_TO_PAGES(RL_TXRX_BUFLEN),
                  TxRxChain->BufferPtr
                  );
  }

  if (Rtl81x9Private->DevicePath != NULL) {
    gBS->FreePool (Rtl81x9Private->DevicePath);
  }

  gBS->FreePool (Rtl81x9Private);

StartErrorExit:

  gBS->CloseProtocol (
        Controller,
        &gEfiDevicePathProtocolGuid,
        This->DriverBindingHandle,
        Controller
        );

  return Status;
}

EFI_STATUS
EFIAPI
Rtl81x9SimpleNetworkDriverStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN  EFI_HANDLE                     Controller,
  IN  UINTN                          NumberOfChildren,
  IN  EFI_HANDLE                     *ChildHandleBuffer
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  EFI_STATUS                  Status;
  EFI_SIMPLE_NETWORK_PROTOCOL *SnpProtocol;
  RTL81X9_SNP_DRIVER          *Rtl81x9Private;

  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiSimpleNetworkProtocolGuid,
                  (VOID**)&SnpProtocol,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );

  if (EFI_ERROR (Status)) {
    return EFI_UNSUPPORTED;
  }

  Rtl81x9Private = RTL81X9_PRIVATE_FROM_EFI_SIMPLE_NETWORK (SnpProtocol);

  Status = gBS->UninstallProtocolInterface (
                  Controller,
                  &gEfiSimpleNetworkProtocolGuid,
                  &Rtl81x9Private->Snp
                  );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = gBS->CloseProtocol (
                  Controller,
                  &gEfiDevicePathProtocolGuid,
                  This->DriverBindingHandle,
                  Controller
                  );

  Rtl81x9Shutdown (Rtl81x9Private);
  Rtl81x9Stop (Rtl81x9Private);

  if (Rtl81x9Private->DevicePath != NULL) {
    gBS->FreePool (Rtl81x9Private->DevicePath);
  }

  gBS->FreePool (Rtl81x9Private);

  return Status;
}

EFI_STATUS
EFIAPI
InitializeRtl81x9Driver (
  IN EFI_HANDLE       ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable
  )
/*++

Routine Description:
  Install all the driver protocol

Arguments:
  (Standard EFI Image entry - EFI_IMAGE_ENTRY_POINT)

Returns:

  EFI_SUCEESS - Initialization routine has found UNDI hardware, loaded it's ROM, and installed
                a notify event for the Network Indentifier Interface Protocol successfully.

  Other       - Return value from HandleProtocol for DeviceIoProtocol or LoadedImageProtocol

--*/
{
  return EfiLibInstallAllDriverProtocols (
          ImageHandle,
          SystemTable,
          &mRtl81x9SimpleNetworkDriverBinding,
          NULL,
          &gRtl81x9SimpleNetworkComponentName,
          NULL,
          NULL
          );
}

