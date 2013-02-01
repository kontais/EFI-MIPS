/*++

Copyright (c) 2012, kontais
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

Module Name:

    Sm712GopScreen.c

Abstract:

--*/

#include "Sm712Gop.h"
#include "Sm712Dev.h"

SM712_GOP_MODE_DATA mGopModeData[] = {
    {1024, 600, 16, 0}
    };

//
// GOP Protocol Member Functions
//

EFI_STATUS
EFIAPI
Sm712GopQuerytMode (
  IN  EFI_GRAPHICS_OUTPUT_PROTOCOL          *This,
  IN  UINT32                                ModeNumber,
  OUT UINTN                                 *SizeOfInfo,
  OUT EFI_GRAPHICS_OUTPUT_MODE_INFORMATION  **Info
  )
/*++

Routine Description:

  Graphics Output protocol interface to get video mode

  Arguments:
    This                  - Protocol instance pointer.
    ModeNumber            - The mode number to return information on.
    Info                  - Caller allocated buffer that returns information about ModeNumber.
    SizeOfInfo            - A pointer to the size, in bytes, of the Info buffer.

  Returns:
    EFI_SUCCESS           - Mode information returned.
    EFI_BUFFER_TOO_SMALL  - The Info buffer was too small.
    EFI_DEVICE_ERROR      - A hardware error occurred trying to retrieve the video mode.
    EFI_NOT_STARTED       - Video display is not initialized. Call SetMode ()
    EFI_INVALID_PARAMETER - One of the input args was NULL.

--*/
{
  EFI_STATUS        Status;
  SM712_GOP_PRIVATE_DATA  *Private;

  Private = SM712_GOP_PRIVATE_DATA_FROM_THIS (This);

  if (Info == NULL || SizeOfInfo == NULL || (UINTN) ModeNumber >= This->Mode->MaxMode) {
    return EFI_INVALID_PARAMETER;
  }

  Status = gBS->AllocatePool (
                  EfiBootServicesData,
                  sizeof (EFI_GRAPHICS_OUTPUT_MODE_INFORMATION),
                  Info
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  *SizeOfInfo = sizeof (EFI_GRAPHICS_OUTPUT_MODE_INFORMATION);

  (*Info)->Version = 0;
  (*Info)->HorizontalResolution = Private->ModeData[ModeNumber].HorizontalResolution;
  (*Info)->VerticalResolution   = Private->ModeData[ModeNumber].VerticalResolution;
  (*Info)->PixelFormat          = PixelBitMask;
  (*Info)->PixelsPerScanLine    = (*Info)->HorizontalResolution;

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
Sm712GopSetMode (
  IN  EFI_GRAPHICS_OUTPUT_PROTOCOL * This,
  IN  UINT32                       ModeNumber
  )
/*++

Routine Description:

  Graphics Output protocol interface to set video mode

  Arguments:
    This             - Protocol instance pointer.
    ModeNumber       - The mode number to be set.

  Returns:
    EFI_SUCCESS      - Graphics mode was changed.
    EFI_DEVICE_ERROR - The device had an error and could not complete the request.
    EFI_UNSUPPORTED  - ModeNumber is not supported by this device.

--*/
{
  EFI_STATUS                    Status;
  SM712_GOP_PRIVATE_DATA        *Private;
  SM712_GOP_MODE_DATA           *ModeData;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL Fill;
  EFI_GRAPHICS_OUTPUT_BLT_PIXEL *NewFillLine;

  Private = SM712_GOP_PRIVATE_DATA_FROM_THIS (This);

  if (ModeNumber >= This->Mode->MaxMode) {
    return EFI_UNSUPPORTED;
  }

  if (ModeNumber == This->Mode->Mode) {
    return EFI_SUCCESS;
  }

  ModeData = &Private->ModeData[ModeNumber];
  This->Mode->Mode = ModeNumber;
  Private->GraphicsOutput.Mode->Info->HorizontalResolution = ModeData->HorizontalResolution;
  Private->GraphicsOutput.Mode->Info->VerticalResolution   = ModeData->VerticalResolution;
  Private->GraphicsOutput.Mode->Info->PixelsPerScanLine    = ModeData->HorizontalResolution;

  Status = gBS->AllocatePool (
                  EfiBootServicesData,
                  sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL) * ModeData->HorizontalResolution,
                  &NewFillLine
                  );
  if (EFI_ERROR (Status)) {
    return EFI_DEVICE_ERROR;
  }

  if (Private->FillLine != NULL) {
    gBS->FreePool (Private->FillLine);
  }

  Private->FillLine             = NewFillLine;

  Fill.Red                      = 0x7f;
  Fill.Green                    = 0x7f;
  Fill.Blue                     = 0x7f;
  This->Blt (
          This,
          &Fill,
          EfiBltVideoFill,
          0,
          0,
          0,
          0,
          ModeData->HorizontalResolution,
          ModeData->VerticalResolution,
          ModeData->HorizontalResolution * sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL)
          );
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
Sm712GopBlt (
  IN  EFI_GRAPHICS_OUTPUT_PROTOCOL            *This,
  IN  EFI_GRAPHICS_OUTPUT_BLT_PIXEL           *BltBuffer, OPTIONAL
  IN  EFI_GRAPHICS_OUTPUT_BLT_OPERATION       BltOperation,
  IN  UINTN                                   SourceX,
  IN  UINTN                                   SourceY,
  IN  UINTN                                   DestinationX,
  IN  UINTN                                   DestinationY,
  IN  UINTN                                   Width,
  IN  UINTN                                   Height,
  IN  UINTN                                   Delta         OPTIONAL
  )
/*++

  Routine Description:
    Blt pixels from the rectangle (Width X Height) formed by the BltBuffer
    onto the graphics screen starting a location (X, Y). (0, 0) is defined as
    the upper left hand side of the screen. (X, Y) can be outside of the
    current screen geometry and the BltBuffer will be cliped when it is
    displayed. X and Y can be negative or positive. If Width or Height is
    bigger than the current video screen the image will be clipped.

  Arguments:
    This          - Protocol instance pointer.
    X             - X location on graphics screen.
    Y             - Y location on the graphics screen.
    Width         - Width of BltBuffer.
    Height        - Hight of BltBuffer
    BltOperation  - Operation to perform on BltBuffer and video memory
    BltBuffer     - Buffer containing data to blt into video buffer. This
                    buffer has a size of Width*Height*sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL)
    SourceX       - If the BltOperation is a EfiCopyBlt this is the source
                    of the copy. For other BLT operations this argument is not
                    used.
    SourceX       - If the BltOperation is a EfiCopyBlt this is the source
                    of the copy. For other BLT operations this argument is not
                    used.

  Returns:
    EFI_SUCCESS           - The palette is updated with PaletteArray.
    EFI_INVALID_PARAMETER - BltOperation is not valid.
    EFI_DEVICE_ERROR      - A hardware error occured writting to the video
                             buffer.

--*/
// TODO:    SourceY - add argument and description to function comment
// TODO:    DestinationX - add argument and description to function comment
// TODO:    DestinationY - add argument and description to function comment
// TODO:    Delta - add argument and description to function comment
{
  EFI_STATUS                    Status;
  SM712_GOP_PRIVATE_DATA        *Private;
  EFI_TPL                       OriginalTPL;
  UINT32                        VerticalResolution;
  UINT32                        HorizontalResolution;

  Private = SM712_GOP_PRIVATE_DATA_FROM_THIS (This);

  if ((BltOperation < 0) || (BltOperation >= EfiGraphicsOutputBltOperationMax)) {
    return EFI_INVALID_PARAMETER;
  }

  if (Width == 0 || Height == 0) {
    return EFI_INVALID_PARAMETER;
  }
  //
  // If Delta is zero, then the entire BltBuffer is being used, so Delta
  // is the number of bytes in each row of BltBuffer.  Since BltBuffer is Width pixels size,
  // the number of bytes in each row can be computed.
  //
  if (Delta == 0) {
    Delta = Width * sizeof (EFI_GRAPHICS_OUTPUT_BLT_PIXEL);
  }

  VerticalResolution   = This->Mode->Info->VerticalResolution;
  HorizontalResolution = This->Mode->Info->HorizontalResolution;

  //
  //  Check bounds
  //
  if (BltOperation == EfiBltVideoToBltBuffer
      || BltOperation == EfiBltVideoToVideo) {
    //
    // Video to BltBuffer: Source is Video, destination is BltBuffer
    //
    if (SourceY + Height > VerticalResolution) {
      return EFI_INVALID_PARAMETER;
    }

    if (SourceX + Width > HorizontalResolution) {
      return EFI_INVALID_PARAMETER;
    }
  }

  if (BltOperation == EfiBltBufferToVideo
      || BltOperation == EfiBltVideoToVideo
      || BltOperation == EfiBltVideoFill) {
    //
    // BltBuffer to Video: Source is BltBuffer, destination is Video
    //
    if (DestinationY + Height > VerticalResolution) {
      return EFI_INVALID_PARAMETER;
    }

    if (DestinationX + Width > HorizontalResolution) {
      return EFI_INVALID_PARAMETER;
    }
  }

  //
  // We have to raise to TPL Notify, so we make an atomic write the frame buffer.
  // We would not want a timer based event (Cursor, ...) to come in while we are
  // doing this operation.
  //
  OriginalTPL = gBS->RaiseTPL (EFI_TPL_NOTIFY);

  Status = Sm712DevBlt (
             Private,
             BltBuffer,
             BltOperation,
             SourceX, 
             SourceY,
             DestinationX, 
             DestinationY,
             Width, Height,
             Delta);

  gBS->RestoreTPL (OriginalTPL);

  return Status;
}


//
// Construction and Destruction functions
//
EFI_STATUS
Sm712GopConstructor (
  SM712_GOP_PRIVATE_DATA    *Private
  )
/*++

Routine Description:

Arguments:

Returns:

  None

--*/
// TODO:    Private - add argument and description to function comment
// TODO:    EFI_SUCCESS - add return value to function comment
{
  EFI_STATUS    Status;
  UINT64        Attributes;
  VOID          *Unmap;
  EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR *Res;

  Private->ModeData = mGopModeData;

  Private->GraphicsOutput.QueryMode = Sm712GopQuerytMode;
  Private->GraphicsOutput.SetMode   = Sm712GopSetMode;
  Private->GraphicsOutput.Blt       = Sm712GopBlt;
  Private->GraphicsOutput.Mode      = &Private->Mode;

  Private->GraphicsOutput.Mode->MaxMode = sizeof(mGopModeData) / sizeof(SM712_GOP_MODE_DATA);
  Private->GraphicsOutput.Mode->Mode    = GOP_INVALIDE_MODE_NUMBER;
  Private->GraphicsOutput.Mode->Info    = &Private->Info;

  Private->GraphicsOutput.Mode->Info->Version = 0;
  Private->GraphicsOutput.Mode->Info->HorizontalResolution = 0;
  Private->GraphicsOutput.Mode->Info->VerticalResolution   = 0;
  Private->GraphicsOutput.Mode->Info->PixelFormat = PixelBitMask;

  /* R:G:B = 5:6:5 */
  Private->GraphicsOutput.Mode->Info->PixelInformation.RedMask   = 0xF800;
  Private->GraphicsOutput.Mode->Info->PixelInformation.GreenMask = 0x07E0;
  Private->GraphicsOutput.Mode->Info->PixelInformation.BlueMask  = 0x001F;
  
  Private->GraphicsOutput.Mode->SizeOfInfo = sizeof (EFI_GRAPHICS_OUTPUT_MODE_INFORMATION);
#if 0
  Status = mPciIo->GetBarAttributes (mPciIo, 0, NULL, (VOID**)&Res);
  if ((Res->Desc != ACPI_ADDRESS_SPACE_DESCRIPTOR) || (Res->ResType != ACPI_ADDRESS_SPACE_TYPE_MEM)) {
    gBS->FreePool (Res);
    return EFI_UNSUPPORTED;
  }

  Status = mPciIo->Map (
                    mPciIo,
                    EfiPciIoOperationBusMasterRead,
                    (VOID*)(UINTN)Res->AddrRangeMin,
                    (UINTN*)&Res->AddrLen,
                    &Private->GraphicsOutput.Mode->FrameBufferBase,
                    &Unmap
                    );
  if (Status != EFI_SUCCESS) {
    gBS->FreePool (Res);
    return EFI_UNSUPPORTED;    
  }


  Private->GraphicsOutput.Mode->FrameBufferSize = (UINTN)Res->AddrLen;

  gBS->FreePool (Res);
#else
  Private->GraphicsOutput.Mode->FrameBufferBase = 0xB4000000;
  Private->GraphicsOutput.Mode->FrameBufferSize = 0x01000000;
#endif
  
  Private->FillLine               = NULL;

  return EFI_SUCCESS;
}

