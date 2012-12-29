/*++

Copyright (c) 2004 - 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  LinuxThunk.c

Abstract:

  Produce LinuxThunk protocol and it's associated device path and controller 
  state protocols. LinuxThunk is to the NT emulation environment as 
  PCI_ROOT_BRIGE is to real hardware. The LinuxBusDriver is the child of this
  driver.

  Since we are a root hardware abstraction we do not install a Driver Binding
  protocol on this handle. This driver can only support one one LinuxThunk protocol
  in the system, since the device path is hard coded.

--*/

#include "LinuxThunk.h"

//
// LinuxThunk Device Path Protocol Instance
//
static LINUX_THUNK_DEVICE_PATH mLinuxThunkDevicePath = {
  {
    HARDWARE_DEVICE_PATH,
    HW_VENDOR_DP,
    (UINT8) (sizeof (VENDOR_DEVICE_PATH)),
    (UINT8) ((sizeof (VENDOR_DEVICE_PATH)) >> 8),
    EFI_LINUX_THUNK_PROTOCOL_GUID,
  },
  {
    END_DEVICE_PATH_TYPE,
    END_ENTIRE_DEVICE_PATH_SUBTYPE,
    END_DEVICE_PATH_LENGTH,
    0
  }
};

EFI_DRIVER_ENTRY_POINT (InitializeLinuxThunk)

EFI_STATUS
EFIAPI
InitializeLinuxThunk (
  IN EFI_HANDLE                            ImageHandle,
  IN EFI_SYSTEM_TABLE                      *SystemTable
  )
/*++

Routine Description:
  Install LinuxThunk Protocol and it's associated Device Path protocol

Arguments:
  (Standard EFI Image entry - EFI_IMAGE_ENTRY_POINT)

Returns:
  EFI_SUCEESS - LinuxThunk protocol is added or error status from 
                gBS->InstallMultiProtocolInterfaces().

--*/
// TODO:    ImageHandle - add argument and description to function comment
// TODO:    SystemTable - add argument and description to function comment
{
  EFI_STATUS  Status;
  EFI_HANDLE  ControllerHandle;

  EfiInitializeDriverLib (ImageHandle, SystemTable);
  EfiInitializeLinuxDriverLib (ImageHandle, SystemTable);

  ControllerHandle = NULL;
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &ControllerHandle,
                  &gEfiLinuxThunkProtocolGuid,
                  gLinux,
                  &gEfiDevicePathProtocolGuid,
                  &mLinuxThunkDevicePath,
                  NULL
                  );

  return Status;
}
