/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:
 
  handle.c
  
Abstract:

  Shell environment handle information management

Revision History

--*/

#include "shelle.h"
#include "shellenvguid.h"

UINTN       SEnvNoHandles;
EFI_HANDLE  *SEnvHandles;
UINTN       SEnvHandleIndex;

VOID
SEnvInitHandleGlobals (
  VOID
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  SEnvNoHandles   = 0;
  SEnvHandles     = NULL;
  SEnvHandleIndex = 0;
}

VOID
SEnvLoadHandleTable (
  VOID
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  //
  // For ease of use the shell maps handle #'s to short numbers.
  //
  // This is only done on request for various internal commands and
  // the references are immediately freed when the internal command
  // completes.
  //
  //
  // Free any old info
  //
  SEnvFreeHandleTable ();

  //
  // Load new info
  //
  SEnvHandles     = NULL;
  SEnvHandleIndex = 0;

  LibLocateHandle (AllHandles, NULL, NULL, &SEnvNoHandles, &SEnvHandles);
}

VOID
SEnvFreeHandleTable (
  VOID
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  if (SEnvNoHandles) {
    SEnvFreeHandleProtocolInfo ();

    FreePool (SEnvHandles);
    SEnvHandles     = NULL;
    SEnvNoHandles   = 0;
    SEnvHandleIndex = 0;
  }
}

UINTN
SEnvHandleNoFromStr (
  IN CHAR16       *Str
  )
/*++

Routine Description:

Arguments:

  Str - The string

Returns:

--*/
{
  UINTN HandleNo;

  HandleNo  = Xtoi (Str);
  HandleNo  = HandleNo > SEnvNoHandles ? 0 : HandleNo;
  return HandleNo;
}

EFI_HANDLE
SEnvHandleFromStr (
  IN CHAR16       *Str
  )
/*++

Routine Description:

Arguments:

  Str - The string

Returns:

--*/
{
  UINTN       HandleNo;
  EFI_HANDLE  Handle;

  HandleNo  = Xtoi (Str) - 1;
  Handle    = HandleNo > SEnvNoHandles - 1 ? NULL : SEnvHandles[HandleNo];
  return Handle;
}

UINTN
SEnvHandleNoFromUINT (
  IN UINTN        Value
  )
/*++

Routine Description:

Arguments:

  Value - The value

Returns:

--*/
{
  UINTN HandleNo;

  HandleNo  = Value;
  HandleNo  = HandleNo > SEnvNoHandles ? 0 : HandleNo;
  return HandleNo;
}

EFI_HANDLE
SEnvHandleFromUINT (
  IN UINTN        Value
  )
/*++

Routine Description:

Arguments:

  Value - The value

Returns:

--*/
{
  UINTN       HandleNo;
  EFI_HANDLE  Handle;

  HandleNo  = Value - 1;
  Handle    = HandleNo > SEnvNoHandles - 1 ? NULL : SEnvHandles[HandleNo];
  return Handle;
}

UINTN
SEnvHandleToNumber (
  IN  EFI_HANDLE  Handle
  )
/*++

Routine Description:

Arguments:

  Handle - The handle

Returns:

--*/
{
  UINTN Index;

  for (Index = 0; Index < SEnvNoHandles; Index++) {
    if (SEnvHandles[Index] == Handle) {
      return Index + 1;
    }
  }

  return 0;
}
//
// functions to enumerate handle database
//
EFI_STATUS
SEnvNextHandle (
  IN OUT   EFI_HANDLE             **Handle
  )
{
  if (SEnvHandleIndex >= SEnvNoHandles) {
    *Handle = NULL;
    return EFI_NOT_FOUND;
  }

  *Handle = SEnvHandles + SEnvHandleIndex;
  SEnvHandleIndex++;
  return EFI_SUCCESS;
}

EFI_STATUS
SEnvSkipHandle (
  IN UINTN             NumSkip
  )
{
  if ((SEnvHandleIndex + NumSkip) >= SEnvNoHandles) {
    return EFI_ACCESS_DENIED;
  } else {
    SEnvHandleIndex += NumSkip;
    return EFI_SUCCESS;
  }
}

UINTN
SEnvResetHandleEnumerator (
  IN UINTN             EnumIndex
  )
{
  UINTN Index;
  Index           = SEnvHandleIndex;
  SEnvHandleIndex = EnumIndex;
  return Index;
}

VOID
SEnvInitHandleEnumerator (
  VOID
  )
{
  SEnvLoadHandleTable ();
}

VOID
SEnvCloseHandleEnumerator (
  VOID
  )
{
  SEnvHandleIndex = 0;
  SEnvFreeHandleTable ();
}

UINTN
SEnvGetHandleNum (
  VOID
  )
{
  return SEnvNoHandles;
}
