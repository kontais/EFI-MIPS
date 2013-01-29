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
#include "Pci22.h"
#include "Acpi.h"

//
// Driver Consumed Protocols
//
#include EFI_PROTOCOL_DEFINITION (DevicePath)
#include EFI_PROTOCOL_DEFINITION (PciIo)
#include EFI_PROTOCOL_DEFINITION (LinuxIo)

//
// Driver Produced Protocols
//
#include EFI_PROTOCOL_DEFINITION (DriverBinding)
#include EFI_PROTOCOL_DEFINITION (ComponentName)
#include EFI_PROTOCOL_DEFINITION (UgaDraw)
#include EFI_PROTOCOL_DEFINITION (UgaIo)
#include "LinkedList.h"

#define SM712_UGA_CLASS_NAME       L"SM712UgaWindow"

#define SM712_UGA_PRIVATE_DATA_SIGNATURE  EFI_SIGNATURE_32 ('S', 'm', 'G', 'o')
typedef struct {
  UINT64                      Signature;

  EFI_HANDLE                  Handle;
  EFI_UGA_DRAW_PROTOCOL       UgaDraw;
  EFI_PCI_IO_PROTOCOL          *PciIo;

  EFI_UNICODE_STRING_TABLE    *ControllerNameTable;

  //
  // UGA Private Data for GetMode ()
  //
  UINT32                      HorizontalResolution;
  UINT32                      VerticalResolution;
  UINT32                      ColorDepth;
  UINT32                      RefreshRate;

  UINT8                       *VedioMemBase;

  EFI_UGA_PIXEL        *FillLine;

} SM712_UGA_PRIVATE_DATA;

#define SM712_UGA_PRIVATE_DATA_FROM_THIS(a)  \
         CR(a, SM712_UGA_PRIVATE_DATA, UgaDraw, SM712_UGA_PRIVATE_DATA_SIGNATURE)


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
  IN  SM712_UGA_PRIVATE_DATA    *Private
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
EFIAPI
Sm712UgaDriverBindingSupported (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      Controller,
  IN  EFI_DEVICE_PATH_PROTOCOL        *RemainingDevicePath
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  This                - TODO: add argument description
  Controller          - TODO: add argument description
  RemainingDevicePath - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
EFIAPI
Sm712UgaDriverBindingStart (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      Controller,
  IN  EFI_DEVICE_PATH_PROTOCOL        *RemainingDevicePath
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  This                - TODO: add argument description
  Controller          - TODO: add argument description
  RemainingDevicePath - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
EFIAPI
Sm712UgaDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN  EFI_HANDLE                   Controller,
  IN  UINTN                        NumberOfChildren,
  IN  EFI_HANDLE                   *ChildHandleBuffer
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  This              - TODO: add argument description
  Controller        - TODO: add argument description
  NumberOfChildren  - TODO: add argument description
  ChildHandleBuffer - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

#endif
