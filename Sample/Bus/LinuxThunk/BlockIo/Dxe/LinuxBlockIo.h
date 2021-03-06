/*++

Copyright (c) 2004 - 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  LinuxBlockIo.h

Abstract:

  Produce block IO abstractions for real devices on your PC using Win32 APIs.
  The configuration of what devices to mount or emulate comes from NT 
  environment variables. The variables must be visible to the Microsoft* 
  Developer Studio for them to work.

  * Other names and brands may be claimed as the property of others.

--*/

#ifndef _LINUX_BLOCK_IO_H_
#define _LINUX_BLOCK_IO_H_

#include "EfiLinux.h"
#include "EfiDriverLib.h"

//
// Driver Consumed Protocols
//
#include EFI_PROTOCOL_DEFINITION (DevicePath)
#include EFI_PROTOCOL_DEFINITION (LinuxIo)

//
// Driver Produced Protocols
//
#include EFI_PROTOCOL_DEFINITION (DriverBinding)
#include EFI_PROTOCOL_DEFINITION (ComponentName)
#include EFI_PROTOCOL_DEFINITION (DriverConfiguration)
#include EFI_PROTOCOL_DEFINITION (DriverDiagnostics)
#include EFI_PROTOCOL_DEFINITION (BlockIo)

#define FILENAME_BUFFER_SIZE  80

//
// Language supported for driverconfiguration protocol
//
#define LANGUAGESUPPORTED "eng"

#define LINUX_BLOCK_IO_PRIVATE_SIGNATURE EFI_SIGNATURE_32 ('L', 'X', 'b', 'k')
typedef struct {
  UINTN                       Signature;

  EFI_LOCK                    Lock;

  char                        Filename[FILENAME_BUFFER_SIZE];
  UINTN                       ReadMode;
  UINTN                       Mode;

  int                         fd;

  UINT64                      LastBlock;
  UINTN                       BlockSize;
  UINT64                      NumberOfBlocks;

  EFI_HANDLE                  EfiHandle;
  EFI_BLOCK_IO_PROTOCOL       BlockIo;
  EFI_BLOCK_IO_MEDIA          Media;

  EFI_UNICODE_STRING_TABLE    *ControllerNameTable;

  EFI_LINUX_THUNK_PROTOCOL   *LinuxThunk;

} LINUX_BLOCK_IO_PRIVATE;

#define LINUX_BLOCK_IO_PRIVATE_DATA_FROM_THIS(a) \
         CR(a, LINUX_BLOCK_IO_PRIVATE, BlockIo, LINUX_BLOCK_IO_PRIVATE_SIGNATURE)

#define LIST_BUFFER_SIZE  512

//
// Block I/O Global Variables
//
extern EFI_DRIVER_BINDING_PROTOCOL        gLinuxBlockIoDriverBinding;
extern EFI_COMPONENT_NAME_PROTOCOL        gLinuxBlockIoComponentName;
extern EFI_DRIVER_CONFIGURATION_PROTOCOL  gLinuxBlockIoDriverConfiguration;
extern EFI_DRIVER_DIAGNOSTICS_PROTOCOL    gLinuxBlockIoDriverDiagnostics;

//
// EFI Driver Binding Functions
//
EFI_STATUS
EFIAPI
LinuxBlockIoDriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN  EFI_HANDLE                    Handle,
  IN  EFI_DEVICE_PATH_PROTOCOL      *RemainingDevicePath
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
LinuxBlockIoDriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN  EFI_HANDLE                    Handle,
  IN  EFI_DEVICE_PATH_PROTOCOL      *RemainingDevicePath
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
LinuxBlockIoDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL   *This,
  IN  EFI_HANDLE                    Handle,
  IN  UINTN                         NumberOfChildren,
  IN  EFI_HANDLE                    *ChildHandleBuffer
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
// Block IO protocol member functions
//
STATIC
EFI_STATUS
EFIAPI
LinuxBlockIoReadBlocks (
  IN EFI_BLOCK_IO_PROTOCOL  *This,
  IN UINT32                 MediaId,
  IN EFI_LBA                Lba,
  IN UINTN                  BufferSize,
  OUT VOID                  *Buffer
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  This        - TODO: add argument description
  MediaId     - TODO: add argument description
  Lba         - TODO: add argument description
  BufferSize  - TODO: add argument description
  Buffer      - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

STATIC
EFI_STATUS
EFIAPI
LinuxBlockIoWriteBlocks (
  IN EFI_BLOCK_IO_PROTOCOL  *This,
  IN UINT32                 MediaId,
  IN EFI_LBA                Lba,
  IN UINTN                  BufferSize,
  IN VOID                   *Buffer
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  This        - TODO: add argument description
  MediaId     - TODO: add argument description
  Lba         - TODO: add argument description
  BufferSize  - TODO: add argument description
  Buffer      - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

STATIC
EFI_STATUS
EFIAPI
LinuxBlockIoFlushBlocks (
  IN EFI_BLOCK_IO_PROTOCOL  *This
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  This  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

STATIC
EFI_STATUS
EFIAPI
LinuxBlockIoResetBlock (
  IN EFI_BLOCK_IO_PROTOCOL  *This,
  IN BOOLEAN                ExtendedVerification
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  This                  - TODO: add argument description
  ExtendedVerification  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

//
// Private Worker functions
//
STATIC
EFI_STATUS
LinuxBlockIoCreateMapping (
  IN EFI_LINUX_IO_PROTOCOL             *LinuxIo,
  IN EFI_HANDLE                         EfiDeviceHandle,
  IN CHAR16                             *Filename,
  IN BOOLEAN                            ReadOnly,
  IN BOOLEAN                            RemovableMedia,
  IN UINTN                              NumberOfBlocks,
  IN UINTN                              BlockSize
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  LinuxIo         - TODO: add argument description
  EfiDeviceHandle - TODO: add argument description
  Filename        - TODO: add argument description
  ReadOnly        - TODO: add argument description
  RemovableMedia  - TODO: add argument description
  NumberOfBlocks  - TODO: add argument description
  BlockSize       - TODO: add argument description
  DeviceType      - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

STATIC
EFI_STATUS
LinuxBlockIoReadWriteCommon (
  IN  LINUX_BLOCK_IO_PRIVATE *Private,
  IN UINT32                   MediaId,
  IN EFI_LBA                  Lba,
  IN UINTN                    BufferSize,
  IN VOID                     *Buffer,
  IN CHAR8                    *CallerName
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  Private     - TODO: add argument description
  MediaId     - TODO: add argument description
  Lba         - TODO: add argument description
  BufferSize  - TODO: add argument description
  Buffer      - TODO: add argument description
  CallerName  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

STATIC
EFI_STATUS
LinuxBlockIoError (
  IN LINUX_BLOCK_IO_PRIVATE      *Private
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  Private - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

STATIC
EFI_STATUS
LinuxBlockIoOpenDevice (
  LINUX_BLOCK_IO_PRIVATE         *Private
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  Private - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

STATIC
char                                    *
GetNextElementPastTerminator (
  IN  char  *EnvironmentVariable,
  IN  char  Terminator
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  EnvironmentVariable - TODO: add argument description
  Terminator          - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
EFIAPI
InitializeLinuxBlockIo (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
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

EFI_STATUS
SetFilePointer64 (
  IN  LINUX_BLOCK_IO_PRIVATE    *Private,
  IN  INT64                      DistanceToMove,
  OUT UINT64                     *NewFilePointer,
  IN  INT32                      MoveMethod
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  Private         - TODO: add argument description
  DistanceToMove  - TODO: add argument description
  NewFilePointer  - TODO: add argument description
  MoveMethod      - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

UINTN
Atoi (
  char  *String
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  String  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

#endif
