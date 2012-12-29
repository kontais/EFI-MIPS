/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  Mem.h

Abstract:
    
  Information about the memory functions

Revision History

--*/

#ifndef _SHELL_MEM_H
#define _SHELL_MEM_H

VOID
SetMem (
  IN VOID     *Buffer,
  IN UINTN    Size,
  IN UINT8    Value
  );

VOID
CopyMem (
  IN VOID     *Dest,
  IN VOID     *Src,
  IN UINTN    Len
  );

VOID                            *
AllocatePool (
  IN UINTN     Size
  );

VOID
FreePool (
  IN VOID     *Pointer
  );

VOID                            *
AllocateZeroPool (
  IN UINTN     Size
  );

VOID                                  *
ReallocatePool (
  IN VOID                 *OldPool,
  IN UINTN                OldSize,
  IN UINTN                NewSize
  );

INTN
CompareMem (
  IN VOID                           *Dest,
  IN VOID                           *Src,
  IN UINTN                          len
  );

VOID
ZeroMem (
  IN VOID     *Buffer,
  IN UINTN    Size
  );

#endif