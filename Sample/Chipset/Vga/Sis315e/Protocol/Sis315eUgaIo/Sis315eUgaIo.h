/*++

Copyright (c) 2011, kontais
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the
BSD License which accompanies this distribution.  The full text of the
license may be found at http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

Module Name:

  Sis315eUgaIo.h

Abstract:

--*/

#ifndef _SIS315E_UGA_IO_H_
#define _SIS315E_UGA_IO_H_

#include EFI_PROTOCOL_DEFINITION (UgaDraw)

#define EFI_SIS315E_UGA_IO_PROTOCOL_GUID \
  { \
    0xe29ae9c2, 0x33a9, 0xac98, {0xb1, 0x97, 0x0a, 0x30, 0xd1, 0xcb, 0x23, 0xee } \
  }

struct _EFI_SIS315E_UGA_IO_PROTOCOL;
typedef struct _EFI_SIS315E_UGA_IO_PROTOCOL EFI_SIS315E_UGA_IO_PROTOCOL;

typedef
EFI_STATUS
(*UGAClose)(EFI_SIS315E_UGA_IO_PROTOCOL *Uga);

typedef
EFI_STATUS
(*UGASize)(EFI_SIS315E_UGA_IO_PROTOCOL *Uga, UINT32 Width, UINT32 Height);

typedef
VOID
(*UGAPutPixel)(EFI_SIS315E_UGA_IO_PROTOCOL * Uga, UINTN DstX, UINTN DstY, UGA_COLOR Color);

typedef
UGA_COLOR
(*UGAGetPixel)(EFI_SIS315E_UGA_IO_PROTOCOL * Uga, UINTN SrcX, UINTN SrcY);

typedef
EFI_STATUS
(*UGABlt)(EFI_SIS315E_UGA_IO_PROTOCOL *Uga,
	  IN  EFI_UGA_PIXEL                           *BltBuffer OPTIONAL,
	  IN  EFI_UGA_BLT_OPERATION                   BltOperation,
	  IN  UINTN                                   SourceX,
	  IN  UINTN                                   SourceY,
	  IN  UINTN                                   DestinationX,
	  IN  UINTN                                   DestinationY,
	  IN  UINTN                                   Width,
	  IN  UINTN                                   Height,
	  IN  UINTN                                   Delta OPTIONAL);

struct _EFI_SIS315E_UGA_IO_PROTOCOL {
  VOID                                *Private;
  UGAClose                            UgaClose;
  UGASize                             UgaSize;
  UGAPutPixel                         UgaPutPixel;
  UGAGetPixel                         UgaGetPixel;
  UGABlt                              UgaBlt;
};


extern EFI_GUID gEfiSis315eUgaIoProtocolGuid;

#endif
