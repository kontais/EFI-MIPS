/*

Copyright (c) 2004 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  LinuxBusDriver.h

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
EFI_LINUX_PASS_THRU      - associates a device with our PCI support

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

// TODO: fix comment to start with /*++
#ifndef _LINUX_BUS_DRIVER_H_
#define _LINUX_BUS_DRIVER_H_

#include "Tiano.h"
#include "EfiDriverLib.h"
#include "EfiDriverLib.h"
#include "Pci22.h"

//
// Driver Consumed Protocols
//
#include EFI_PROTOCOL_DEFINITION (DevicePath)
#include EFI_PROTOCOL_DEFINITION (LinuxThunk)
#include EFI_PROTOCOL_DEFINITION (CpuIo)

//
// Driver Produced Protocols
//
#include EFI_PROTOCOL_DEFINITION (DriverBinding)
#include EFI_PROTOCOL_DEFINITION (ComponentName)
#include EFI_PROTOCOL_DEFINITION (LinuxIo)

//
// Linux Bus Driver Global Variables
//
extern EFI_DRIVER_BINDING_PROTOCOL  gLinuxBusDriverBinding;
extern EFI_COMPONENT_NAME_PROTOCOL  gLinuxBusDriverComponentName;

//
// Linux Bus Controller Structure
//
#define LINUX_BUS_DEVICE_SIGNATURE EFI_SIGNATURE_32 ('L', 'X', 'B', 'D')

typedef struct {
  UINT64                    Signature;
  EFI_UNICODE_STRING_TABLE  *ControllerNameTable;
} LINUX_BUS_DEVICE;

//
// Linux Child Device Controller Structure
//
#define LINUX_IO_DEVICE_SIGNATURE  EFI_SIGNATURE_32 ('L', 'X', 'V', 'D')

typedef struct {
  UINT64                    Signature;
  EFI_HANDLE                Handle;
  EFI_LINUX_IO_PROTOCOL    LinuxIo;
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath;

  //
  // Private data about the parent
  //
  EFI_HANDLE                ControllerHandle;
  EFI_DEVICE_PATH_PROTOCOL  *ParentDevicePath;

  EFI_UNICODE_STRING_TABLE  *ControllerNameTable;

} LINUX_IO_DEVICE;

#define LINUX_IO_DEVICE_FROM_THIS(a) \
  CR(a, LINUX_IO_DEVICE, LinuxIo, LINUX_IO_DEVICE_SIGNATURE)

//
// This is the largest env variable we can parse
//
#define MAX_LINUX_ENVIRNMENT_VARIABLE_LENGTH 512

typedef struct {
  CHAR16    *Variable;
  EFI_GUID  *DevicePathGuid;
} LINUX_ENVIRONMENT_VARIABLE_ENTRY;

typedef struct {
  VENDOR_DEVICE_PATH  VendorDevicePath;
  UINT32              Instance;
} LINUX_VENDOR_DEVICE_PATH_NODE;

EFI_STATUS
EFIAPI
CpuIoInitialize (
  IN EFI_HANDLE                            ImageHandle,
  IN EFI_SYSTEM_TABLE                      *SystemTable
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  ImageHandle - TODO: add argument description
  SystemTable - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

//
// Driver Binding Protocol function prototypes
//
EFI_STATUS
EFIAPI
LinuxBusDriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Handle,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  This                - TODO: add argument description
  Handle              - TODO: add argument description
  RemainingDevicePath - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
EFIAPI
LinuxBusDriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     ParentHandle,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  This                - TODO: add argument description
  ParentHandle        - TODO: add argument description
  RemainingDevicePath - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
EFIAPI
LinuxBusDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN  EFI_HANDLE                   Handle,
  IN  UINTN                        NumberOfChildren,
  IN  EFI_HANDLE                   *ChildHandleBuffer
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  This              - TODO: add argument description
  Handle            - TODO: add argument description
  NumberOfChildren  - TODO: add argument description
  ChildHandleBuffer - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

//
// Linux Bus Driver private worker functions
//
EFI_DEVICE_PATH_PROTOCOL  *
LinuxBusCreateDevicePath (
  IN  EFI_DEVICE_PATH_PROTOCOL  *RootDevicePath,
  IN  EFI_GUID                  *Guid,
  IN  UINT16                    InstanceNumber
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  RootDevicePath  - TODO: add argument description
  Guid            - TODO: add argument description
  InstanceNumber  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;


EFI_LINUX_THUNK_PROTOCOL *gLinuxThunk;
#endif
