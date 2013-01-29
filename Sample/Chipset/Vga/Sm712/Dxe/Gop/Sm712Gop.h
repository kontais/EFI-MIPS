/*++

Copyright (c) 2011, kontais
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  Sm712Gop.h

Abstract:

  Private data for the Gop driver for SM712

--*/

#ifndef _SM712_GOP_H_
#define _SM712_GOP_H_

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
#include EFI_PROTOCOL_DEFINITION (GraphicsOutput)
#include "LinkedList.h"

#define SM712_GOP_CLASS_NAME       L"SM712Gop"

#define SM712_GOP_PRIVATE_DATA_SIGNATURE  EFI_SIGNATURE_32 ('S', 'm', 'G', 'o')

#define GOP_INVALIDE_MODE_NUMBER 0xffff

typedef struct {
  UINT32                     HorizontalResolution;
  UINT32                     VerticalResolution;
  UINT32                     ColorDepth;
  UINT32                     RefreshRate;
} SM712_GOP_MODE_DATA;

typedef struct {
  UINT64                       Signature;
  EFI_HANDLE                   Handle;
  EFI_GRAPHICS_OUTPUT_PROTOCOL GraphicsOutput;
  EFI_PCI_IO_PROTOCOL          *PciIo;

  EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE    Mode;
  EFI_GRAPHICS_OUTPUT_MODE_INFORMATION Info;
  
  //
  // GOP Private Data for QueryMode ()
  //
  SM712_GOP_MODE_DATA                  *ModeData;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL        *FillLine;
  EFI_UNICODE_STRING_TABLE    *ControllerNameTable;

} SM712_GOP_PRIVATE_DATA;

#define SM712_GOP_PRIVATE_DATA_FROM_THIS(a)  \
         CR(a, SM712_GOP_PRIVATE_DATA, GraphicsOutput, SM712_GOP_PRIVATE_DATA_SIGNATURE)


//
// Global Protocol Variables
//
extern EFI_DRIVER_BINDING_PROTOCOL  gSm712GopDriverBinding;
extern EFI_COMPONENT_NAME_PROTOCOL  gSm712GopComponentName;
extern EFI_PCI_IO_PROTOCOL          *mPciIo;;

EFI_STATUS
Sm712GopSupported (
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
Sm712GopConstructor (
  IN  SM712_GOP_PRIVATE_DATA    *Private
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
// EFI 1.1 driver model prototypes for SM712 GOP
//

EFI_STATUS
EFIAPI
Sm712GopInitialize (
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
EFIAPI
Sm712GopDriverBindingSupported (
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
Sm712GopDriverBindingStart (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      Controller,
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
;

#endif

