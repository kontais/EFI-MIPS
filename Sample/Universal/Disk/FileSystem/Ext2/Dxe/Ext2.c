/*++

Copyright (c) 2012, kontais                                                                   
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  Ext2.c

Abstract:

  Ext2 File System driver routines that support EFI driver model

--*/

#include "Ext2.h"

EFI_STATUS
EFIAPI
Ext2DriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   ControllerHandle,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  );

EFI_STATUS
EFIAPI
Ext2DriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   ControllerHandle,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  );

EFI_STATUS
EFIAPI
Ext2DriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN  EFI_HANDLE                   ControllerHandle,
  IN  UINTN                        NumberOfChildren,
  IN  EFI_HANDLE                   *ChildHandleBuffer
  );

EFI_STATUS
EFIAPI
Ext2ComponentNameGetDriverName (
  IN  EFI_COMPONENT_NAME_PROTOCOL  *This,
  IN  CHAR8                        *Language,
  OUT CHAR16                       **DriverName
  );

EFI_STATUS
EFIAPI
Ext2ComponentNameGetControllerName (
  IN  EFI_COMPONENT_NAME_PROTOCOL    *This,
  IN  EFI_HANDLE                     ControllerHandle,
  IN  EFI_HANDLE                     ChildHandle  OPTIONAL,
  IN  CHAR8                          *Language,
  OUT CHAR16                         **ControllerName
  );

//
// DriverBinding protocol instance
//
EFI_DRIVER_BINDING_PROTOCOL gExt2DriverBinding = {
    Ext2DriverBindingSupported,
    Ext2DriverBindingStart,
    Ext2DriverBindingStop,
    0x10,
    NULL,
    NULL
};



EFI_DRIVER_ENTRY_POINT (Ext2EntryPoint)

EFI_STATUS
EFIAPI
Ext2EntryPoint (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
/*++

Routine Description:

  Register Driver Binding protocol for this driver.

Arguments:

  ImageHandle           - Handle for the image of this driver.
  SystemTable           - Pointer to the EFI System Table.

Returns:

  EFI_SUCCESS           - Driver loaded.
  other                 - Driver not loaded.

--*/
{
  return EfiLibInstallAllDriverProtocols(
          ImageHandle,
          SystemTable,
          &gExt2DriverBinding,
          ImageHandle,
          &gExt2ComponentName,
          NULL,
          NULL
          );
}

EFI_STATUS
EFIAPI
Ext2DriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   ControllerHandle,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  )
/*++

Routine Description:

  Test to see if this driver can add a file system to ControllerHandle.
  ControllerHandle must support both Disk IO and Block IO protocols.

Arguments:

  This                  - Protocol instance pointer.
  ControllerHandle      - Handle of device to test.
  RemainingDevicePath   - Not used.

Returns:

  EFI_SUCCESS           - This driver supports this device.
  EFI_ALREADY_STARTED   - This driver is already running on this device.
  other                 - This driver does not support this device.

--*/
{
  EFI_STATUS            Status;
  EFI_DISK_IO_PROTOCOL  *DiskIo;

  //
  // Open the IO Abstraction(s) needed to perform the supported test
  //
  Status = gBS->OpenProtocol (
                  ControllerHandle,
                  &gEfiDiskIoProtocolGuid,
                  (VOID **) &DiskIo,
                  This->DriverBindingHandle,
                  ControllerHandle,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );

  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Close the I/O Abstraction(s) used to perform the supported test
  //
  gBS->CloseProtocol (
         ControllerHandle,
         &gEfiDiskIoProtocolGuid,
         This->DriverBindingHandle,
         ControllerHandle
         );

  //
  // Open the IO Abstraction(s) needed to perform the supported test
  //
  Status = gBS->OpenProtocol (
                  ControllerHandle,
                  &gEfiBlockIoProtocolGuid,
                  NULL,
                  This->DriverBindingHandle,
                  ControllerHandle,
                  EFI_OPEN_PROTOCOL_TEST_PROTOCOL
                  );

  return Status;
}

EFI_STATUS
EFIAPI
Ext2DriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   ControllerHandle,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  )
/*++

Routine Description:

  Start this driver on ControllerHandle by opening a Block IO and Disk IO
  protocol, reading Device Path. Add a Simple File System protocol to
  ControllerHandle if the media contains a valid file system.

Arguments:

  This                  - Protocol instance pointer.
  ControllerHandle      - Handle of device to bind driver to.
  RemainingDevicePath   - Not used.

Returns:

  EFI_SUCCESS           - This driver is added to DeviceHandle.
  EFI_ALREADY_STARTED   - This driver is already running on DeviceHandle.
  EFI_OUT_OF_RESOURCES  - Can not allocate the memory.
  other                 - This driver does not support this device.

--*/
{
  EFI_STATUS            Status;
  EFI_BLOCK_IO_PROTOCOL *BlockIo;
  EFI_DISK_IO_PROTOCOL  *DiskIo;
  BOOLEAN               LockedByMe;

  LockedByMe = FALSE;
  //
  // Acquire the lock.
  // If caller has already acquired the lock, cannot lock it again.
  //
  Status = Ext2AcquireLockOrFail ();
  if (!EFI_ERROR (Status)) {
    LockedByMe = TRUE;
  }
  
  //
  // Open our required BlockIo and DiskIo
  //
  Status = gBS->OpenProtocol (
                  ControllerHandle,
                  &gEfiBlockIoProtocolGuid,
                  (VOID **) &BlockIo,
                  This->DriverBindingHandle,
                  ControllerHandle,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  Status = gBS->OpenProtocol (
                  ControllerHandle,
                  &gEfiDiskIoProtocolGuid,
                  (VOID **) &DiskIo,
                  This->DriverBindingHandle,
                  ControllerHandle,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    goto Exit;
  }
  //
  // Allocate Volume structure. In FatAllocateVolume(), Resources
  // are allocated with protocol installed and cached initialized
  //
  Status = Ext2AllocateVolume (ControllerHandle, DiskIo, BlockIo);

  //
  // When the media changes on a device it will Reinstall the BlockIo interaface.
  // This will cause a call to our Stop(), and a subsequent reentrant call to our
  // Start() successfully. We should leave the device open when this happen.
  //
  if (EFI_ERROR (Status)) {
    Status = gBS->OpenProtocol (
                    ControllerHandle,
                    &gEfiSimpleFileSystemProtocolGuid,
                    NULL,
                    This->DriverBindingHandle,
                    ControllerHandle,
                    EFI_OPEN_PROTOCOL_TEST_PROTOCOL
                    );
    if (EFI_ERROR (Status)) {
      gBS->CloseProtocol (
             ControllerHandle,
             &gEfiDiskIoProtocolGuid,
             This->DriverBindingHandle,
             ControllerHandle
             );
    }
  }

Exit:
  //
  // Unlock if locked by myself.
  //
  if (LockedByMe) {
    Ext2ReleaseLock ();
  }
  return Status;
}

EFI_STATUS
EFIAPI
Ext2DriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL   *This,
  IN  EFI_HANDLE                    ControllerHandle,
  IN  UINTN                         NumberOfChildren,
  IN  EFI_HANDLE                    *ChildHandleBuffer
  )
/*++

Routine Description:
  Stop this driver on ControllerHandle.

Arguments:
  This                  - Protocol instance pointer.
  ControllerHandle      - Handle of device to stop driver on.
  NumberOfChildren      - Not used.
  ChildHandleBuffer     - Not used.

Returns:
  EFI_SUCCESS           - This driver is removed DeviceHandle.
  other                 - This driver was not removed from this device.

--*/
{
  EFI_STATUS                      Status;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FileSystem;
  EXT2_VOLUME                     *Volume;

  //
  // Get our context back
  //
  Status = gBS->OpenProtocol (
                  ControllerHandle,
                  &gEfiSimpleFileSystemProtocolGuid,
                  (VOID **) &FileSystem,
                  This->DriverBindingHandle,
                  ControllerHandle,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );

  if (!EFI_ERROR (Status)) {
    Volume = EXT2_VOLUME_FROM_FILE_SYSTEM (FileSystem);
    Status = Ext2AbandonVolume (Volume);
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  Status = gBS->CloseProtocol (
                  ControllerHandle,
                  &gEfiDiskIoProtocolGuid,
                  This->DriverBindingHandle,
                  ControllerHandle
                  );

  return Status;
}

