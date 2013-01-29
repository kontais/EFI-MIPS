/*++

Copyright (c) 2011, kontais
Portions copyright (c) 2008-2009 Apple Inc. All rights reserved.
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

--*/

#include "Sm712Uga.h"
#include "Sm712.h"

EFI_UGA_PIXEL
RGB565ToUgaPixel(UINT16 RGB565)
{ 
  EFI_UGA_PIXEL Pixel;    

  Pixel.Red   = (RGB565 >> 11) << 3;
  Pixel.Green = (RGB565 >> 5) << 2;
  Pixel.Blue  = (RGB565 ) << 3;
  Pixel.Reserved = 0;                         
 return Pixel;                                      
}

UINT16
UgaPixelToRGB565 (EFI_UGA_PIXEL Pixel)
{
  return (UINT16) ((Pixel.Red >> 3) << 11 | (Pixel.Green >> 2) << 5 | Pixel.Blue >> 3);
}

VOID
SM712DevGetLine (
  IN EFI_UGA_PIXEL *Blt,
  IN UINTN                         FrameBuffer,
  IN UINTN                         Width
  )
{
  UINT16 *BufPtr;

  BufPtr = (UINT16*) FrameBuffer;
  while (Width--) {
    *Blt++ = RGB565ToUgaPixel(*BufPtr);
    BufPtr++;
  }
}

VOID
SM712DevPutLine (
  IN UINTN                         FrameBuffer,
  IN EFI_UGA_PIXEL *Blt,
  IN UINTN                         Width
  )
{
  UINT16 *BufPtr;

  BufPtr = (UINT16*) FrameBuffer;
  while (Width--) {
    *BufPtr++ = UgaPixelToRGB565(*Blt);
    Blt++;
  }
}

EFI_STATUS
UgaBlt(
       IN  SM712_UGA_PRIVATE_DATA *Private,
       IN  EFI_UGA_PIXEL                           *BltBuffer OPTIONAL,
       IN  EFI_UGA_BLT_OPERATION                   BltOperation,
       IN  UINTN                                   SourceX,
       IN  UINTN                                   SourceY,
       IN  UINTN                                   DestinationX,
       IN  UINTN                                   DestinationY,
       IN  UINTN                                   Width,
       IN  UINTN                                   Height,
       IN  UINTN                                   Delta OPTIONAL
  )
{
  EFI_UGA_PIXEL *Blt;
  EFI_UGA_PIXEL *FillPixel;
  UINT32                        HorizontalResolution;
  UINTN                         DstY;
  UINTN                         SrcY;
  UINTN                         Index;
  UINTN                         FrameBuffer;
  UINTN                         FrameBufferSrc;

  HorizontalResolution = Private->HorizontalResolution;
  
  if (BltOperation == EfiUgaVideoToBltBuffer) {
    for (SrcY = SourceY, DstY = DestinationY; DstY < (Height + DestinationY); SrcY++, DstY++) {
      Blt = (EFI_UGA_PIXEL *) ((UINT8 *) BltBuffer + (DstY * Delta) + DestinationX * sizeof (EFI_UGA_PIXEL));
      FrameBuffer = (UINTN)Private->VedioMemBase + (SrcY * HorizontalResolution + SourceX) * sizeof(UINT16);
      SM712DevGetLine (Blt, FrameBuffer, Width);
    }
  } else {
    if (BltOperation == EfiUgaVideoFill) {
      FillPixel = BltBuffer;
      for (Index = 0; Index < Width; Index++) {
        Private->FillLine[Index] = *FillPixel;
      }
    }

    for (Index = 0; Index < Height; Index++) {
      if (DestinationY <= SourceY) {
        SrcY  = SourceY + Index;
        DstY  = DestinationY + Index;
      } else {
        SrcY  = SourceY + Height - Index - 1;
        DstY  = DestinationY + Height - Index - 1;
      }

      FrameBuffer = (UINTN)Private->VedioMemBase + (DstY * HorizontalResolution + DestinationX) * sizeof(UINT16);
      switch (BltOperation) {
      case EfiUgaBltBufferToVideo:
        Blt = (EFI_UGA_PIXEL *) ((UINT8 *) BltBuffer + (SrcY * Delta) + SourceX * sizeof (EFI_UGA_PIXEL));
        SM712DevPutLine (FrameBuffer, Blt, Width);
        break;

      case EfiUgaVideoToVideo:
        FrameBufferSrc = (UINTN)Private->VedioMemBase + (SrcY * HorizontalResolution + SourceX) * sizeof(UINT16);
        EfiCopyMem ((VOID*)FrameBuffer, (VOID*)FrameBufferSrc, Width * sizeof(UINT16));
        break;

      case EfiUgaVideoFill:
        SM712DevPutLine (FrameBuffer, Private->FillLine, Width);
        break;

      default:
        break;
      }
    }
  }

  return EFI_SUCCESS;
}


