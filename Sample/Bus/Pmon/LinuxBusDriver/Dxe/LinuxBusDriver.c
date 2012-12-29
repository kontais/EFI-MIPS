/*+++

Copyright (c) 2004 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  LinuxBusDriver.c

Abstract:

This following section documents the envirnoment variables for the Win NT 
build.  These variables are used to define the (virtual) hardware 
configuration of the NT environment

A ! can be used to seperate multiple instances in a variable. Each 
instance represents a seperate hardware device. 

EFI_LINUX_PHYSICAL_DISKS - maps to drives on your system
EFI_LINUX_VIRTUAL_DISKS  - maps to a device emulated by a file
EFI_LINUX_FILE_SYSTEM    - mouts a directory as a file system
EFI_LINUX_CONSOLE        - make a logical comand line window (only one!)
EFI_LINUX_UGA            - Builds UGA Windows of Width and Height
EFI_LINUX_GOP            - Builds GOP Windows of Width and Height
EFI_LINUX_SERIAL_PORT    - maps physical serial ports

 <F>ixed       - Fixed disk like a hard drive.
 <R>emovable   - Removable media like a floppy or CD-ROM.
 Read <O>nly   - Write protected device.
 Read <W>rite  - Read write device.
 <block count> - Decimal number of blocks a device supports.
 <block size>  - Decimal number of bytes per block.

 NT envirnonment variable contents. '<' and '>' are not part of the variable, 
 they are just used to make this help more readable. There should be no 
 spaces between the ';'. Extra spaces will break the variable. A '!' is  
 used to seperate multiple devices in a variable.

 EFI_LINUX_VIRTUAL_DISKS = 
   <F | R><O | W>;<block count>;<block size>[!...]

 EFI_LINUX_PHYSICAL_DISKS =
   <drive letter>:<F | R><O | W>;<block count>;<block size>[!...]

 Virtual Disks: These devices use a file to emulate a hard disk or removable
                media device. 
                
   Thus a 20 MB emulated hard drive would look like:
   EFI_LINUX_VIRTUAL_DISKS=FW;40960;512

   A 1.44MB emulated floppy with a block size of 1024 would look like:
   EFI_LINUX_VIRTUAL_DISKS=RW;1440;1024

 Physical Disks: These devices use NT to open a real device in your system

   Thus a 120 MB floppy would look like:
   EFI_LINUX_PHYSICAL_DISKS=B:RW;245760;512

   Thus a standard CD-ROM floppy would look like:
   EFI_LINUX_PHYSICAL_DISKS=Z:RO;307200;2048

 EFI_LINUX_FILE_SYSTEM = 
   <directory path>[!...]

   Mounting the two directories C:\FOO and C:\BAR would look like:
   EFI_LINUX_FILE_SYSTEM=c:\foo!c:\bar

 EFI_LINUX_CONSOLE = 
   <window title>

   Declaring a text console window with the title "My EFI Console" woild look like:
   EFI_LINUX_CONSOLE=My EFI Console

 EFI_LINUX_UGA = 
   <width> <height>[!...]

   Declaring a two UGA windows with resolutions of 800x600 and 1024x768 would look like:
   Example : EFI_LINUX_UGA=800 600!1024 768

 EFI_LINUX_GOP = 
   <width> <height>[!...]

   Declaring a two GOP windows with resolutions of 800x600 and 1024x768 would look like:
   Example : EFI_LINUX_GOP=800 600!1024 768

 EFI_LINUX_SERIAL_PORT = 
   <port name>[!...]

   Declaring two serial ports on COM1 and COM2 would look like:
   Example : EFI_LINUX_SERIAL_PORT=COM1!COM2

 EFI_LINUX_PASS_THROUGH =
   <BaseAddress>;<Bus#>;<Device#>;<Function#>

   Declaring a base address of 0xE0000000 (used for PCI Express devices)
   and having NT32 talk to a device located at bus 0, device 1, function 0:
   Example : EFI_LINUX_PASS_THROUGH=E000000;0;1;0

---*/

#include "LinuxBusDriver.h"
#include "PciHostBridge.h"

//
// Define GUID for the Linux Bus Driver
//
static EFI_GUID gLinuxBusDriverGuid = {
  0x419f582, 0x625, 0x4531, {0x8a, 0x33, 0x85, 0xa9, 0x96, 0x5c, 0x95, 0xbc}
};

//
// DriverBinding protocol global
//
EFI_DRIVER_BINDING_PROTOCOL           gLinuxBusDriverBinding = {
  LinuxBusDriverBindingSupported,
  LinuxBusDriverBindingStart,
  LinuxBusDriverBindingStop,
  0xa,
  NULL,
  NULL
};

//
// Table to map NT Environment variable to the GUID that should be in
// device path.
//
static LINUX_ENVIRONMENT_VARIABLE_ENTRY  mEnvironment[] = {
  {L"EFI_LINUX_CONSOLE",          &gEfiLinuxConsoleGuid},
  {L"EFI_LINUX_UGA",              &gEfiLinuxUgaGuid},
#if (EFI_SPECIFICATION_VERSION >= 0x00020000)
  {L"EFI_LINUX_GOP",              &gEfiLinuxGopGuid},
#endif
  {L"EFI_LINUX_SERIAL_PORT",      &gEfiLinuxSerialPortGuid},
  {L"EFI_LINUX_FILE_SYSTEM",      &gEfiLinuxFileSystemGuid},
  {L"EFI_LINUX_VIRTUAL_DISKS",    &gEfiLinuxVirtualDisksGuid},
  {L"EFI_LINUX_PHYSICAL_DISKS",   &gEfiLinuxPhysicalDisksGuid},
  {L"EFI_LINUX_CPU_MODEL",        &gEfiLinuxCPUModelGuid},
  {L"EFI_LINUX_CPU_SPEED",        &gEfiLinuxCPUSpeedGuid},
  {L"EFI_MEMORY_SIZE",             &gEfiLinuxMemoryGuid},
  {L"EFI_LINUX_PASS_THROUGH",     &gEfiLinuxPassThroughGuid},
  {NULL, NULL}
};

EFI_DRIVER_ENTRY_POINT (InitializeLinuxBusDriver)

EFI_STATUS
EFIAPI
InitializeLinuxBusDriver (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
/*++

Routine Description:
  Install DriverBinding Protocol for the Win NT Bus driver on the drivers
  image handle.

Arguments:
  (Standard EFI Image entry - EFI_IMAGE_ENTRY_POINT)

Returns:
  EFI_SUCEESS - DriverBinding protocol is added, or error status from 
                InstallProtocolInterface() is returned.

--*/
// TODO:    ImageHandle - add argument and description to function comment
// TODO:    SystemTable - add argument and description to function comment
{
  //gHostBridgeInit     = FALSE;
  //gReadPending        = FALSE;
  //gImageHandle        = ImageHandle;
  //gConfigData.Enable  = 1;
  //gBaseAddress        = 0;

  //CpuIoInitialize (ImageHandle, SystemTable);

  return EfiLibInstallAllDriverProtocols (
          ImageHandle,
          SystemTable,
          &gLinuxBusDriverBinding,
          ImageHandle,
          &gLinuxBusDriverComponentName,
          NULL,
          NULL
          );
}

EFI_STATUS
EFIAPI
LinuxBusDriverBindingSupported (
  IN  EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN  EFI_HANDLE                   ControllerHandle,
  IN  EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  )
/*++

Routine Description:

Arguments:

Returns:

  None

--*/
// TODO:    This - add argument and description to function comment
// TODO:    ControllerHandle - add argument and description to function comment
// TODO:    RemainingDevicePath - add argument and description to function comment
// TODO:    EFI_UNSUPPORTED - add return value to function comment
// TODO:    EFI_UNSUPPORTED - add return value to function comment
// TODO:    EFI_SUCCESS - add return value to function comment
// TODO:    EFI_SUCCESS - add return value to function comment
{
  EFI_STATUS                Status;
  EFI_DEVICE_PATH_PROTOCOL  *ParentDevicePath;
  EFI_LINUX_THUNK_PROTOCOL *LinuxThunk;
  UINTN                     Index;

  //
  // Check the contents of the first Device Path Node of RemainingDevicePath to make sure
  // it is a legal Device Path Node for this bus driver's children.
  //
  if (RemainingDevicePath != NULL) {
    if (RemainingDevicePath->Type != HARDWARE_DEVICE_PATH ||
        RemainingDevicePath->SubType != HW_VENDOR_DP ||
        DevicePathNodeLength(RemainingDevicePath) != sizeof(LINUX_VENDOR_DEVICE_PATH_NODE)) {
      return EFI_UNSUPPORTED;
    }

    for (Index = 0; mEnvironment[Index].Variable != NULL; Index++) {
      if (EfiCompareGuid (&((VENDOR_DEVICE_PATH *) RemainingDevicePath)->Guid, mEnvironment[Index].DevicePathGuid)) {
        break;
      }
    }

    if (mEnvironment[Index].Variable == NULL) {
      return EFI_UNSUPPORTED;
    }
  }
  
  //
  // Open the IO Abstraction(s) needed to perform the supported test
  //
  Status = gBS->OpenProtocol (
                  ControllerHandle,
                  &gEfiDevicePathProtocolGuid,
                  (VOID**)&ParentDevicePath,
                  This->DriverBindingHandle,
                  ControllerHandle,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (Status == EFI_ALREADY_STARTED) {
    return EFI_SUCCESS;
  }

  if (EFI_ERROR (Status)) {
    return Status;
  }

  gBS->CloseProtocol (
        ControllerHandle,
        &gEfiDevicePathProtocolGuid,
        This->DriverBindingHandle,
        ControllerHandle
        );

  Status = gBS->OpenProtocol (
                  ControllerHandle,
                  &gEfiLinuxThunkProtocolGuid,
                  (VOID**)&LinuxThunk,
                  This->DriverBindingHandle,
                  ControllerHandle,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (Status == EFI_ALREADY_STARTED) {
    return EFI_SUCCESS;
  }

  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Since we call through LinuxThunk we need to make sure it's valid
  //
  Status = EFI_SUCCESS;
  if (LinuxThunk->Signature != EFI_LINUX_THUNK_PROTOCOL_SIGNATURE) {
    Status = EFI_UNSUPPORTED;
  }

  //
  // Close the I/O Abstraction(s) used to perform the supported test
  //
  gBS->CloseProtocol (
        ControllerHandle,
        &gEfiLinuxThunkProtocolGuid,
        This->DriverBindingHandle,
        ControllerHandle
        );

  return Status;
}

EFI_STATUS
EFIAPI
LinuxBusDriverBindingStart (
  IN  EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN  EFI_HANDLE                   ControllerHandle,
  IN  EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  )
/*++

Routine Description:

Arguments:

Returns:

  None

--*/
// TODO:    This - add argument and description to function comment
// TODO:    ControllerHandle - add argument and description to function comment
// TODO:    RemainingDevicePath - add argument and description to function comment
// TODO:    EFI_OUT_OF_RESOURCES - add return value to function comment
// TODO:    EFI_OUT_OF_RESOURCES - add return value to function comment
// TODO:    EFI_SUCCESS - add return value to function comment
{
  EFI_STATUS                      Status;
  EFI_STATUS                      InstallStatus;
  EFI_LINUX_THUNK_PROTOCOL       *LinuxThunk;
  EFI_DEVICE_PATH_PROTOCOL        *ParentDevicePath;
  UINT32                          Result;
  LINUX_BUS_DEVICE               *LinuxBusDevice;
  LINUX_IO_DEVICE                *LinuxDevice;
  UINTN                           Index;
  CHAR16                          *StartString;
  CHAR16                          *SubString;
  UINT16                          Count;
  UINTN                           Value;
  UINTN                           StringSize;
  UINT8                           TempBuffer[MAX_LINUX_ENVIRNMENT_VARIABLE_LENGTH];
  UINT8                           NtEnvironmentVariableBuffer[MAX_LINUX_ENVIRNMENT_VARIABLE_LENGTH];
  UINT16                          ComponentName[MAX_LINUX_ENVIRNMENT_VARIABLE_LENGTH];
  LINUX_VENDOR_DEVICE_PATH_NODE  *Node;
  BOOLEAN                         CreateDevice;
  BOOLEAN                         NoLinuxConsole;

  Status = EFI_UNSUPPORTED;

  //
  // Grab the protocols we need
  //
  Status = gBS->OpenProtocol (
                  ControllerHandle,
                  &gEfiDevicePathProtocolGuid,
                  (VOID**)&ParentDevicePath,
                  This->DriverBindingHandle,
                  ControllerHandle,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status) && Status != EFI_ALREADY_STARTED) {
    return Status;
  }

  Status = gBS->OpenProtocol (
                  ControllerHandle,
                  &gEfiLinuxThunkProtocolGuid,
                  (VOID**)&LinuxThunk,
                  This->DriverBindingHandle,
                  ControllerHandle,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status) && Status != EFI_ALREADY_STARTED) {
    return Status;
  }

  //
  // Define the gLinuxThunk interface that can be used by child processes like CPUIo
  // This will be used to communicate to the kernel driver for PCI I/O redirections.
  //
  gLinuxThunk = LinuxThunk;

  if (Status != EFI_ALREADY_STARTED) {
    Status = gBS->AllocatePool (
                    EfiBootServicesData,
                    sizeof (LINUX_BUS_DEVICE),
                    (VOID *) &LinuxBusDevice
                    );
    if (EFI_ERROR (Status)) {
      return Status;
    }

    LinuxBusDevice->Signature           = LINUX_BUS_DEVICE_SIGNATURE;
    LinuxBusDevice->ControllerNameTable = NULL;

    EfiLibAddUnicodeString (
      "eng",
      gLinuxBusDriverComponentName.SupportedLanguages,
      &LinuxBusDevice->ControllerNameTable,
      L"Linux Bus Controller"
      );

    Status = gBS->InstallMultipleProtocolInterfaces (
                    &ControllerHandle,
                    &gLinuxBusDriverGuid,
                    LinuxBusDevice,
                    NULL
                    );
    if (EFI_ERROR (Status)) {
      EfiLibFreeUnicodeStringTable (LinuxBusDevice->ControllerNameTable);
      gBS->FreePool (LinuxBusDevice);
      return Status;
    }
  }

  //
  // Loop on the Variable list. Parse each variable to produce a set of handles that
  // represent virtual hardware devices.
  //
  NoLinuxConsole  = FALSE;
  InstallStatus   = EFI_NOT_FOUND;
  for (Index = 0; mEnvironment[Index].Variable != NULL; Index++) {
    EfiUnicodeStrToAsciiStr(mEnvironment[Index].Variable, NtEnvironmentVariableBuffer);
    Result = 0; 
    EfiAsciiStrToUnicodeStr(TempBuffer, NtEnvironmentVariableBuffer);

    if (Result == 0) {
      if ((EfiCompareGuid (mEnvironment[Index].DevicePathGuid, &gEfiLinuxUgaGuid) && NoLinuxConsole) ||
          (EfiCompareGuid (mEnvironment[Index].DevicePathGuid, &gEfiLinuxGopGuid) && NoLinuxConsole)) {
        //
        // If there is no GOP or UGA force a default 1024 x 600 console
        //
        StartString = L"1024 600";
      } else {
        if (EfiCompareGuid (mEnvironment[Index].DevicePathGuid, &gEfiLinuxConsoleGuid)) {
          NoLinuxConsole = TRUE;
        }

        continue;
      }
    } else {
      StartString = (CHAR16 *) NtEnvironmentVariableBuffer;
    }

    //
    // Parse the envirnment variable into sub strings using '!' as a delimator.
    // Each substring needs it's own handle to be added to the system. This code
    // does not understand the sub string. Thats the device drivers job.
    //
    Count = 0;
    while (*StartString != '\0') {

      //
      // Find the end of the sub string
      //
      SubString = StartString;
      while (*SubString != '\0' && *SubString != '!') {
        SubString++;
      }

      if (*SubString == '!') {
        //
        // Replace token with '\0' to make sub strings. If this is the end
        //  of the string SubString will already point to NULL.
        //
        *SubString = '\0';
        SubString++;
      }

      CreateDevice = TRUE;
      if (RemainingDevicePath != NULL) {
        CreateDevice  = FALSE;
        Node          = (LINUX_VENDOR_DEVICE_PATH_NODE *) RemainingDevicePath;
        if (Node->VendorDevicePath.Header.Type == HARDWARE_DEVICE_PATH &&
            Node->VendorDevicePath.Header.SubType == HW_VENDOR_DP &&
            DevicePathNodeLength (&Node->VendorDevicePath.Header) == sizeof (LINUX_VENDOR_DEVICE_PATH_NODE)
            ) {
          if (EfiCompareGuid (&Node->VendorDevicePath.Guid, mEnvironment[Index].DevicePathGuid) &&
              Node->Instance == Count
              ) {
            CreateDevice = TRUE;
          }
        }
      }

      if (CreateDevice) {

        //
        // Allocate instance structure, and fill in parent information.
        //
        Status = gBS->AllocatePool (
                        EfiBootServicesData,
                        sizeof (LINUX_IO_DEVICE),
                        (VOID *) &LinuxDevice
                        );
        if (EFI_ERROR (Status)) {
          return EFI_OUT_OF_RESOURCES;
        }

        LinuxDevice->Handle             = NULL;
        LinuxDevice->ControllerHandle   = ControllerHandle;
        LinuxDevice->ParentDevicePath   = ParentDevicePath;

        LinuxDevice->LinuxIo.LinuxThunk = LinuxThunk;

        //
        // Plus 2 to account for the NULL at the end of the Unicode string
        //
        StringSize = (UINTN) ((UINT8 *) SubString - (UINT8 *) StartString) + 2;
        Status = gBS->AllocatePool (
                        EfiBootServicesData,
                        StringSize,
                        (VOID *) &LinuxDevice->LinuxIo.EnvString
                        );
        if (EFI_ERROR (Status)) {
          LinuxDevice->LinuxIo.EnvString = NULL;
        } else {
          EfiCopyMem (LinuxDevice->LinuxIo.EnvString, StartString, StringSize);
        }

        LinuxDevice->ControllerNameTable = NULL;

        EfiStrCpy(ComponentName, mEnvironment[Index].Variable);
        EfiStrCat(ComponentName, L"=");
        EfiStrCat(ComponentName, LinuxDevice->LinuxIo.EnvString);

        LinuxDevice->DevicePath = LinuxBusCreateDevicePath (
                                    ParentDevicePath,
                                    mEnvironment[Index].DevicePathGuid,
                                    Count
                                    );
        if (LinuxDevice->DevicePath == NULL) {
          gBS->FreePool (LinuxDevice);
          return EFI_OUT_OF_RESOURCES;
        }

        EfiLibAddUnicodeString (
          "eng",
          gLinuxBusDriverComponentName.SupportedLanguages,
          &LinuxDevice->ControllerNameTable,
          ComponentName
          );

        LinuxDevice->LinuxIo.TypeGuid       = mEnvironment[Index].DevicePathGuid;
        LinuxDevice->LinuxIo.InstanceNumber = Count;

        LinuxDevice->Signature              = LINUX_IO_DEVICE_SIGNATURE;

        Status = gBS->InstallMultipleProtocolInterfaces (
                        &LinuxDevice->Handle,
                        &gEfiDevicePathProtocolGuid,
                        LinuxDevice->DevicePath,
                        &gEfiLinuxIoProtocolGuid,
                        &LinuxDevice->LinuxIo,
                        NULL
                        );
        if (EFI_ERROR (Status)) {
          EfiLibFreeUnicodeStringTable (LinuxDevice->ControllerNameTable);
          gBS->FreePool (LinuxDevice);
        } else {
          //
          // Open For Child Device
          //
          Status = gBS->OpenProtocol (
                          ControllerHandle,
                          &gEfiLinuxThunkProtocolGuid,
                          (VOID**)&LinuxThunk,
                          This->DriverBindingHandle,
                          LinuxDevice->Handle,
                          EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER
                          );
          if (!EFI_ERROR (Status)) {
            InstallStatus = EFI_SUCCESS;
          }
        }
      }

      //
      // Parse Next sub string. This will point to '\0' if we are at the end.
      //
      Count++;
      StartString = SubString;
    }
  }


  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
LinuxBusDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN  EFI_HANDLE                   ControllerHandle,
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
// TODO:    ControllerHandle - add argument and description to function comment
// TODO:    NumberOfChildren - add argument and description to function comment
// TODO:    ChildHandleBuffer - add argument and description to function comment
// TODO:    EFI_SUCCESS - add return value to function comment
// TODO:    EFI_DEVICE_ERROR - add return value to function comment
// TODO:    EFI_SUCCESS - add return value to function comment
{
  EFI_STATUS                Status;
  UINTN                     Index;
  BOOLEAN                   AllChildrenStopped;
  EFI_LINUX_IO_PROTOCOL    *LinuxIo;
  LINUX_BUS_DEVICE         *LinuxBusDevice;
  LINUX_IO_DEVICE          *LinuxDevice;
  EFI_LINUX_THUNK_PROTOCOL *LinuxThunk;

  //
  // Complete all outstanding transactions to Controller.
  // Don't allow any new transaction to Controller to be started.
  //

  if (NumberOfChildren == 0) {
    //
    // Close the bus driver
    //
    Status = gBS->OpenProtocol (
                    ControllerHandle,
                    &gLinuxBusDriverGuid,
                    (VOID**)&LinuxBusDevice,
                    This->DriverBindingHandle,
                    ControllerHandle,
                    EFI_OPEN_PROTOCOL_GET_PROTOCOL
                    );
    if (EFI_ERROR (Status)) {
      return Status;
    }

    gBS->UninstallMultipleProtocolInterfaces (
          ControllerHandle,
          &gLinuxBusDriverGuid,
          LinuxBusDevice,
          NULL
          );

    EfiLibFreeUnicodeStringTable (LinuxBusDevice->ControllerNameTable);

    gBS->FreePool (LinuxBusDevice);

    gBS->CloseProtocol (
          ControllerHandle,
          &gEfiLinuxThunkProtocolGuid,
          This->DriverBindingHandle,
          ControllerHandle
          );

    gBS->CloseProtocol (
          ControllerHandle,
          &gEfiDevicePathProtocolGuid,
          This->DriverBindingHandle,
          ControllerHandle
          );
    return EFI_SUCCESS;
  }

  AllChildrenStopped = TRUE;

  for (Index = 0; Index < NumberOfChildren; Index++) {

    Status = gBS->OpenProtocol (
                    ChildHandleBuffer[Index],
                    &gEfiLinuxIoProtocolGuid,
                    (VOID**)&LinuxIo,
                    This->DriverBindingHandle,
                    ControllerHandle,
                    EFI_OPEN_PROTOCOL_GET_PROTOCOL
                    );
    if (!EFI_ERROR (Status)) {

      LinuxDevice = LINUX_IO_DEVICE_FROM_THIS (LinuxIo);

      Status = gBS->CloseProtocol (
                      ControllerHandle,
                      &gEfiLinuxThunkProtocolGuid,
                      This->DriverBindingHandle,
                      LinuxDevice->Handle
                      );

      Status = gBS->UninstallMultipleProtocolInterfaces (
                      LinuxDevice->Handle,
                      &gEfiDevicePathProtocolGuid,
                      LinuxDevice->DevicePath,
                      &gEfiLinuxIoProtocolGuid,
                      &LinuxDevice->LinuxIo,
                      NULL
                      );

      if (EFI_ERROR (Status)) {
        gBS->OpenProtocol (
              ControllerHandle,
              &gEfiLinuxThunkProtocolGuid,
              (VOID **) &LinuxThunk,
              This->DriverBindingHandle,
              LinuxDevice->Handle,
              EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER
              );
      } else {
        //
        // Close the child handle
        //
        EfiLibFreeUnicodeStringTable (LinuxDevice->ControllerNameTable);
        gBS->FreePool (LinuxDevice);
      }
    }

    if (EFI_ERROR (Status)) {
      AllChildrenStopped = FALSE;
    }
  }

  if (!AllChildrenStopped) {
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}

EFI_DEVICE_PATH_PROTOCOL *
LinuxBusCreateDevicePath (
  IN  EFI_DEVICE_PATH_PROTOCOL  *RootDevicePath,
  IN  EFI_GUID                  *Guid,
  IN  UINT16                    InstanceNumber
  )
/*++

Routine Description:
  Create a device path node using Guid and InstanceNumber and append it to
  the passed in RootDevicePath

Arguments:
  RootDevicePath - Root of the device path to return.

  Guid           - GUID to use in vendor device path node.

  InstanceNumber - Instance number to use in the vendor device path. This
                    argument is needed to make sure each device path is unique.

Returns:

  EFI_DEVICE_PATH_PROTOCOL 

--*/
{
  LINUX_VENDOR_DEVICE_PATH_NODE  DevicePath;

  DevicePath.VendorDevicePath.Header.Type     = HARDWARE_DEVICE_PATH;
  DevicePath.VendorDevicePath.Header.SubType  = HW_VENDOR_DP;
  SetDevicePathNodeLength (&DevicePath.VendorDevicePath.Header, sizeof (LINUX_VENDOR_DEVICE_PATH_NODE));

  //
  // The GUID defines the Class
  //
  EfiCopyMem (&DevicePath.VendorDevicePath.Guid, Guid, sizeof (EFI_GUID));

  //
  // Add an instance number so we can make sure there are no Device Path
  // duplication.
  //
  DevicePath.Instance = InstanceNumber;

  return EfiAppendDevicePathNode (
          RootDevicePath,
          (EFI_DEVICE_PATH_PROTOCOL *) &DevicePath
          );
}
