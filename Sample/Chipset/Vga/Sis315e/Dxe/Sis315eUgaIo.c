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

#include "Sis315eUga.h"
#include "Sis315e.h"

//#include EFI_PROTOCOL_DEFINITION (LinuxThunk)
//#include EFI_PROTOCOL_DEFINITION (UgaDraw)
#include EFI_PROTOCOL_DEFINITION (Sis315eUgaIo)
//#include EFI_PROTOCOL_DEFINITION (SimpleTextIn)


typedef struct _UGA_DRV_SHIFT_MASK
{
  UINT8  Shift;
  UINT8  Size;
  UINT8  Csize;
} UGA_DRV_SHIFT_MASK;

typedef struct _UGA_IO_PRIVATE
{
  EFI_SIS315E_UGA_IO_PROTOCOL UgaIo;

  UGA_DRV_SHIFT_MASK  r;
  UGA_DRV_SHIFT_MASK  g;
  UGA_DRV_SHIFT_MASK  b;


  UINT32 Depth;
  UINT32 Width;
  UINT32 Height;

  UINT8  *VedioMemBase;
  UINT32 LineBytes;
  UINT32 PixelShift;

} UGA_IO_PRIVATE;

VOID
FillShiftMask (UGA_DRV_SHIFT_MASK *ShiftMask, UINT32 Mask)
{
  ShiftMask->Shift = 0;
  ShiftMask->Size  = 0;
  
  while ((Mask & 1) == 0) {
    Mask >>= 1;
    ShiftMask->Shift++;
  }
  
  while (Mask & 1) {
    ShiftMask->Size++;
    Mask >>= 1;
  }
  
  ShiftMask->Csize = 8 - ShiftMask->Size;
}

EFI_STATUS
UgaClose (EFI_SIS315E_UGA_IO_PROTOCOL *UgaIo)
{
  UGA_IO_PRIVATE *drv = (UGA_IO_PRIVATE *)UgaIo;

  if (drv == NULL) {
    return EFI_SUCCESS;
  }

  gBS->FreePool (drv);

  return EFI_SUCCESS;
}

EFI_STATUS
UgaSize(EFI_SIS315E_UGA_IO_PROTOCOL *UgaIo, UINT32 Width, UINT32 Height)
{
  UGA_IO_PRIVATE *drv = (UGA_IO_PRIVATE *)UgaIo;

  if ((Width != SIS315E_WIDTH) || (Height != SIS315E_HEIGHT)) {
    return EFI_UNSUPPORTED;
  }

  drv->Width = Width;
  drv->Height = Height;

  return EFI_SUCCESS;
}

UGA_COLOR
UgaPixelToColor (UGA_IO_PRIVATE *drv, EFI_UGA_PIXEL pixel)
{
  return pixel.Reserved;
}

EFI_UGA_PIXEL
UgaColorToPixel (UGA_IO_PRIVATE *drv, UGA_COLOR val)
{
  EFI_UGA_PIXEL res;

  EfiCommonLibZeroMem (&res, sizeof(EFI_UGA_PIXEL));

  res.Reserved  = (UINT8)(val);

  return res;
}

VOID
UgaPutPixel (EFI_SIS315E_UGA_IO_PROTOCOL *UgaIo, 
             UINTN DstX, 
             UINTN DstY, 
             UGA_COLOR Color
             )
{
  UINT8 *Dst;
  
  UGA_IO_PRIVATE *Private = (UGA_IO_PRIVATE *)UgaIo;

  Dst = Private->VedioMemBase + (DstX << Private->PixelShift)
        + DstY * Private->LineBytes;
  /* BUG BUG, only 16bit */
  //*(UINT16*) Dst = (UINT16) Color;
  *(UINT8*) Dst = (UINT8) Color;

  return ;
}

UGA_COLOR
UgaGetPixel (EFI_SIS315E_UGA_IO_PROTOCOL *UgaIo, 
             UINTN SrcX, 
             UINTN SrcY)
{
  UINT8 *Src;
  
  UGA_IO_PRIVATE *Private = (UGA_IO_PRIVATE *)UgaIo;

  Src = Private->VedioMemBase + (SrcX << Private->PixelShift)
        + SrcY * Private->LineBytes;
  /* BUG BUG, only 16bit */
  //return (UGA_COLOR) (*(UINT16*) Src);
  return (UGA_COLOR) (*(UINT8*) Src);
}

EFI_STATUS
UgaBlt(EFI_SIS315E_UGA_IO_PROTOCOL *UgaIo,
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
  UGA_IO_PRIVATE *Private = (UGA_IO_PRIVATE *)UgaIo;
  UINTN             DstY;
  UINTN             SrcY;
  UINTN             DstX;
  UINTN             SrcX;
  UINTN             Index;
  EFI_UGA_PIXEL     *Blt;
  UINT8             *Dst;
  UINT8             *Src;
  UINTN             Nbr;
  UGA_COLOR         Color;

  DEBUG((EFI_D_ERROR,"UgaBlt %d %d %d %d %d %d\n", SourceX,SourceY,DestinationX,DestinationY,Width,Height));
  DEBUG((EFI_D_ERROR,"UgaBlt BltOperation %d Delta %d\n", BltOperation, Delta));
  //
  //  Check bounds
  //
  if (BltOperation == EfiUgaVideoToBltBuffer
      || BltOperation == EfiUgaVideoToVideo) {
    //
    // Source is Video.
    //
    if (SourceY + Height > Private->Height) {
      return EFI_INVALID_PARAMETER;
    }

    if (SourceX + Width > Private->Width) {
      return EFI_INVALID_PARAMETER;
    }
  }

  if (BltOperation == EfiUgaBltBufferToVideo
      || BltOperation == EfiUgaVideoToVideo
      || BltOperation == EfiUgaVideoFill) {
    //
    // Destination is Video
    //
    if (DestinationY + Height > Private->Height) {
      return EFI_INVALID_PARAMETER;
    }

    if (DestinationX + Width > Private->Width) {
      return EFI_INVALID_PARAMETER;
    }
  }

  switch (BltOperation) {
  case EfiUgaVideoToBltBuffer:
  DEBUG((EFI_D_ERROR,"UgaBlt Trace 0\n"));
    Blt = (EFI_UGA_PIXEL *)((UINT8 *)BltBuffer + (DestinationY * Delta) + DestinationX * sizeof (EFI_UGA_PIXEL));
    Delta -= Width * sizeof (EFI_UGA_PIXEL);
    for (SrcY = SourceY; SrcY < (Height + SourceY); SrcY++) {
      for (SrcX = SourceX; SrcX < (Width + SourceX); SrcX++) {
        *Blt++ = UgaColorToPixel(Private,
                                  UgaGetPixel(UgaIo, SrcX, SrcY));
      }
      Blt = (EFI_UGA_PIXEL *) ((UINT8 *) Blt + Delta);
    }
    break;
  case EfiUgaBltBufferToVideo:
  DEBUG((EFI_D_ERROR,"UgaBlt Trace 1\n"));
    Blt = (EFI_UGA_PIXEL *)((UINT8 *)BltBuffer + (SourceY * Delta) + SourceX * sizeof (EFI_UGA_PIXEL));
    Delta -= Width * sizeof (EFI_UGA_PIXEL);
    for (DstY = DestinationY; DstY < (Height + DestinationY); DstY++) {
      for (DstX = DestinationX; DstX < (Width + DestinationX); DstX++) {
        UgaPutPixel(UgaIo, DstX, DstY, UgaPixelToColor(Private, *Blt));
        Blt++;
      }
      Blt = (EFI_UGA_PIXEL *) ((UINT8 *) Blt + Delta);
    }
    break;
  case EfiUgaVideoToVideo:
  DEBUG((EFI_D_ERROR,"UgaBlt Trace 2\n"));
    Dst = Private->VedioMemBase + (DestinationX << Private->PixelShift)
      + DestinationY * Private->LineBytes;
    Src = Private->VedioMemBase + (SourceX << Private->PixelShift)
      + SourceY * Private->LineBytes;
    Nbr = Width << Private->PixelShift;
    if (DestinationY < SourceY) {
      for (Index = 0; Index < Height; Index++) {
        EfiCommonLibCopyMem (Dst, Src, Nbr);
        Dst += Private->LineBytes;
        Src += Private->LineBytes;
      }
    }
    else {
      Dst += (Height - 1) * Private->LineBytes;
      Src += (Height - 1) * Private->LineBytes;
      for (Index = 0; Index < Height; Index++) {
      //
      // Source and Destination Y may be equal, therefore Dst and Src may
      // overlap.
      //
      EfiCommonLibCopyMem (Dst, Src, Nbr);
      Dst -= Private->LineBytes;
      Src -= Private->LineBytes;
      }
    }
    break;
  case EfiUgaVideoFill:
  DEBUG((EFI_D_ERROR,"UgaBlt Trace 3\n"));
    Color = UgaPixelToColor(Private, *BltBuffer);
    for (DstY = DestinationY; DstY < (Height + DestinationY); DstY++) {
      for (DstX = DestinationX; DstX < (Width + DestinationX); DstX++) {
        UgaPutPixel(UgaIo, DstX, DstY, Color);
      }
    }
    break;
  default:
      return EFI_INVALID_PARAMETER;
  }

  //
  //  Refresh screen.
  //
#if 0
  switch (BltOperation) {
  case EfiUgaVideoToVideo:
    XCopyArea(Private->display, Private->win, Private->win, Private->gc,
               SourceX, SourceY, Width, Height, DestinationX, DestinationY);
    while (1) {
      XEvent ev;

      XNextEvent (Private->display, &ev);
      HandleEvent(Private, &ev);
      if (ev.type == NoExpose || ev.type == GraphicsExpose)
        break;
    }
    break;
  case EfiUgaVideoFill:
    Color = UgaPixelToColor(Private, *BltBuffer);
    XSetForeground(Private->display, Private->gc, Color);
    XFillRectangle(Private->display, Private->win, Private->gc,
                    DestinationX, DestinationY, Width, Height);
    XFlush(Private->display);
    break;
  case EfiUgaBltBufferToVideo:
    Redraw(Private, DestinationX, DestinationY, Width, Height);
    break;
  default:
    break;
  }
#endif
  return EFI_SUCCESS;
}

EFI_STATUS
UgaCreate (EFI_SIS315E_UGA_IO_PROTOCOL **Uga)
{
  EFI_STATUS     Status;
  UGA_IO_PRIVATE *drv;

  drv = NULL;
  Status = gBS->AllocatePool (
                      EfiBootServicesData,
                      sizeof(UGA_IO_PRIVATE),
                      (VOID**)&drv
                      );
  if (EFI_ERROR (Status)) {
    return EFI_OUT_OF_RESOURCES;
  }

  drv->UgaIo.UgaClose    = UgaClose;
  drv->UgaIo.UgaSize     = UgaSize;
  drv->UgaIo.UgaBlt      = UgaBlt;
  drv->UgaIo.UgaPutPixel = UgaPutPixel;
  drv->UgaIo.UgaGetPixel = UgaGetPixel;

  /* 1024*600 16bit color */
  drv->Depth  = SIS315E_DEPTH;
  drv->Width  = SIS315E_WIDTH;
  drv->Height = SIS315E_HEIGHT;

  drv->VedioMemBase = (UINT8*)0xB0000000; /* BUG BUG */
  drv->LineBytes  = drv->Width * (drv->Depth / 8);
  drv->PixelShift = (drv->Depth / 8) - 1;

  /* R:G:B = 5:6:5 */
  FillShiftMask (&drv->r, 0x07);
  FillShiftMask (&drv->g, 0x38);
  FillShiftMask (&drv->b, 0xc0);

  *Uga = (EFI_SIS315E_UGA_IO_PROTOCOL *)drv;
  return EFI_SUCCESS;
}

