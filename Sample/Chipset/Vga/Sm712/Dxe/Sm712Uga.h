/*++

Copyright (c) 2011, kontais
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  Sm712Uga.h

Abstract:

  Private data for the Uga driver for SM712

--*/

#ifndef _SM712_UGA_H_
#define _SM712_UGA_H_

#include "Tiano.h"
#include "EfiDriverLib.h"

typedef unsigned long UGA_COLOR;

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
#include EFI_PROTOCOL_DEFINITION (UgaDraw)
#include EFI_PROTOCOL_DEFINITION (UgaIo)
#include EFI_PROTOCOL_DEFINITION (Sm712UgaIo)
#include "LinkedList.h"

#define SM712_UGA_CLASS_NAME       L"SM712UgaWindow"

#define UGA_PRIVATE_DATA_SIGNATURE  EFI_SIGNATURE_32 ('S', 'g', 'o', 'N')
typedef struct {
  UINT64                      Signature;

  EFI_HANDLE                  Handle;
  EFI_UGA_DRAW_PROTOCOL       UgaDraw;
//  EFI_SIMPLE_TEXT_IN_PROTOCOL SimpleTextIn;

//  EFI_LINUX_THUNK_PROTOCOL   *LinuxThunk;

  EFI_UNICODE_STRING_TABLE    *ControllerNameTable;

  //
  // UGA Private Data for GetMode ()
  //
  UINT32                      HorizontalResolution;
  UINT32                      VerticalResolution;
  UINT32                      ColorDepth;
  UINT32                      RefreshRate;

  //
  // UGA Private Data knowing when to start hardware
  //
  BOOLEAN                     HardwareNeedsStarting;

  CHAR16                      *WindowName;

  EFI_SM712_UGA_IO_PROTOCOL    *UgaIo;

} UGA_PRIVATE_DATA;


#define UGA_DRAW_PRIVATE_DATA_FROM_THIS(a)  \
         CR(a, UGA_PRIVATE_DATA, UgaDraw, UGA_PRIVATE_DATA_SIGNATURE)

//
// Global Protocol Variables
//
extern EFI_DRIVER_BINDING_PROTOCOL  gSm712UgaDriverBinding;
extern EFI_COMPONENT_NAME_PROTOCOL  gSm712UgaComponentName;

//
// Uga Hardware abstraction internal worker functions
//
EFI_STATUS
Sm712UgaSupported (
  IN  EFI_LINUX_IO_PROTOCOL  *LinuxIo
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  Sm712Io - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
Sm712UgaConstructor (
  IN  UGA_PRIVATE_DATA    *Private
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

EFI_STATUS
Sm712UgaDestructor (
  IN  UGA_PRIVATE_DATA    *Private
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

//
// EFI 1.1 driver model prototypes for Win LINUX UGA
//

EFI_STATUS
EFIAPI
Sm712UgaInitialize (
  IN EFI_HANDLE            ImageHandle,
  IN EFI_SYSTEM_TABLE      *SystemTable
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
UgaCreate (EFI_SM712_UGA_IO_PROTOCOL **Uga);


EFI_STATUS
EFIAPI
Sm712UgaDriverBindingSupported (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      Handle,
  IN  EFI_DEVICE_PATH_PROTOCOL        *RemainingDevicePath
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
Sm712UgaDriverBindingStart (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      Handle,
  IN  EFI_DEVICE_PATH_PROTOCOL        *RemainingDevicePath
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
Sm712UgaDriverBindingStop (
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

#endif
