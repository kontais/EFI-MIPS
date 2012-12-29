/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  Mem.c

Abstract:
    
  Memory functions

Revision History

--*/

#include "EfiShellLib.h"

VOID
SetMem (
  IN VOID     *Buffer,
  IN UINTN    Size,
  IN UINT8    Value
  )
/*++

Routine Description:
  Fills a buffer with a value.

Arguments:
  Buffer                - A pointer to the buffer to free.

  Size                  - The number of bytes in the buffer to fill.

  Value                 - The value to fill Buffer with.

Returns:

  None

--*/
{
  volatile INT8  *pt;

  ASSERT (Buffer != NULL);

  pt = Buffer;
  while (Size--) {
    *(pt++) = Value;
  }
}

VOID
CopyMem (
  IN VOID     *Dest,
  IN VOID     *Src,
  IN UINTN    len
  )
/*++

Routine Description:
  Copies the contents of one buffer to another.

Arguments:
  Dest                - A pointer to the buffer to copy to

  Src                 - A pointer to the buffer to copy from.

  len                 - The number of bytes to copy.

Returns:

  None

--*/
{
  CHAR8 *Destination8;
  CHAR8 *Source8;

  ASSERT (Dest != NULL);
  ASSERT (Src != NULL);

  Destination8  = Dest;
  Source8       = Src;

  if (Destination8 > Source8 && (Source8 + len - 1) > Destination8) {
    Source8 += (len - 1);
    Destination8 += (len - 1);
    while (len--) {
      *(Destination8--) = *(Source8--);
    }

    return ;
  }

  while (len--) {
    *(Destination8++) = *(Source8++);
  }
}

VOID *
AllocatePool (
  IN UINTN                Size
  )
/*++

Routine Description:
  Allocates pool memory.

Arguments:
  Size                 - Size in bytes of the pool being requested.

Returns:

  EFI_SUCEESS          - The requested number of bytes were allocated.

  EFI_OUT_OF_RESOURCES - The pool requested could not be allocated.

--*/
{
  EFI_STATUS  Status;
  VOID        *p;

  Status = BS->AllocatePool (PoolAllocationType, Size, &p);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "AllocatePool: out of pool  %x\n", Status));
    p = NULL;
  }

  return p;
}

VOID *
ReallocatePool (
  IN VOID                 *OldPool,
  IN UINTN                OldSize,
  IN UINTN                NewSize
  )
/*++

Routine Description:
  Adjusts the size of a previously allocated buffer.

Arguments:
  OldPool               - A pointer to the buffer whose size is being adjusted.
  OldSize               - The size of the current buffer.
  NewSize               - The size of the new buffer.

Returns:

  EFI_SUCEESS           - The requested number of bytes were allocated.

  EFI_OUT_OF_RESOURCES  - The pool requested could not be allocated.

  EFI_INVALID_PARAMETER - The buffer was invalid.

--*/
{
  VOID  *NewPool;

  NewPool = NULL;
  if (NewSize) {
    NewPool = AllocatePool (NewSize);
  }

  if (OldPool) {
    if (NewPool) {
      CopyMem (NewPool, OldPool, OldSize < NewSize ? OldSize : NewSize);
    }

    FreePool (OldPool);
  }

  return NewPool;
}

VOID *
AllocateZeroPool (
  IN UINTN                Size
  )
/*++

Routine Description:
  Allocates pool memory and initializes the memory to zeros.

Arguments:
  Size                 - Size in bytes of the pool being requested.

Returns:

  EFI_SUCEESS          - The requested number of bytes were allocated.

  EFI_OUT_OF_RESOURCES - The pool requested could not be allocated.

--*/
{
  VOID  *p;

  p = AllocatePool (Size);
  if (p) {
    SetMem (p, Size, 0);
  }

  return p;
}

INTN
CompareMem (
  IN VOID     *Dest,
  IN VOID     *Src,
  IN UINTN    len
  )
/*++

Routine Description:
  Compares the contents of one buffer to another.

Arguments:
  Dest                - A pointer to the buffer to compare

  Src                 - A pointer to the buffer to compare

  len                 - The number of bytes to compare

Returns:

  0                   - Dest is identical to Src for len bytes.
  !=0                 - Dest is not identical to Src for len bytes.

--*/
{
  CHAR8 *d;

  CHAR8 *s;

  ASSERT (Dest != NULL);
  ASSERT (Src != NULL);

  d = Dest;
  s = Src;
  while (len--) {
    if (*d != *s) {
      return *d -*s;
    }

    d += 1;
    s += 1;
  }

  return 0;
}

VOID
ZeroMem (
  IN VOID     *Buffer,
  IN UINTN    Size
  )
/*++

Routine Description:
  Fills a buffer with zeros.

Arguments:
  Buffer                - A pointer to the buffer to fill with zeros.

  Size                  - Number of bytes to zero in the buffer.

Returns:

  None

--*/
{
  SetMem (Buffer, Size, 0);
}

VOID
FreePool (
  IN VOID                 *Buffer
  )
/*++

Routine Description:
  Releases a previously allocated buffer.

Arguments:
  Buffer                - A pointer to the buffer to free.

Returns:

  EFI_SUCEESS           - The requested number of bytes were allocated.

  EFI_INVALID_PARAMETER - The buffer was invalid.

--*/
{
  ASSERT (Buffer);
  BS->FreePool (Buffer);
}
