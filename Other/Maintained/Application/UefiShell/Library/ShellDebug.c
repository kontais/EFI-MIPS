/*++

Copyright (c) 2005 - 2007, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  ShellDebug.c

Abstract:

  Support for Debug primatives. 

--*/

#include "EfiShellLib.h"
#include EFI_PROTOCOL_DEFINITION (DebugAssert)

EFI_DEBUG_ASSERT_PROTOCOL *mDebugAssert = NULL;

EFI_STATUS
EfiDebugAssertInit (
  VOID
  )
{
  EFI_STATUS  Status;
  Status = LibLocateProtocol (&gEfiDebugAssertProtocolGuid, (VOID **) &mDebugAssert);
  return Status;
}

VOID
EfiDebugAssert (
  IN CHAR8    *FileName,
  IN INTN     LineNumber,
  IN CHAR8    *Description
  )
/*++

Routine Description:

  Worker function for ASSERT(). If Error Logging hub is loaded log ASSERT
  information. If Error Logging hub is not loaded BREAKPOINT().
  
Arguments:

  FileName    - File name of failing routine.

  LineNumber  - Line number of failing ASSERT().

  Description - Descritption, usally the assertion,
  
Returns:
  
  None

--*/
{
  if (mDebugAssert != NULL) {
    mDebugAssert->Assert (mDebugAssert, FileName, LineNumber, Description);
  }
}

VOID
EfiDebugPrint (
  IN  UINTN     ErrorLevel,
  IN  CHAR8     *Format,
  ...
  )
/*++

Routine Description:

  Worker function for DEBUG(). If Error Logging hub is loaded log ASSERT
  information. If Error Logging hub is not loaded do nothing.
  
Arguments:

  ErrorLevel - If error level is set do the debug print.

  Format     - String to use for the print, followed by Print arguments.
  
Returns:
  
  None

--*/
{
  VA_LIST Marker;

  VA_START (Marker, Format);
  if (mDebugAssert != NULL) {
    mDebugAssert->Print (mDebugAssert, ErrorLevel, Format, Marker);
  }

  VA_END (Marker);
}

VOID
EFIDebugVariable (
  VOID
  )
/*++

Routine Description:
  Initializes the EFIDebug variable
    
Arguments:

  None
    
Returns:

  None

--*/
{
  EFI_STATUS  Status;
  UINT32      Attributes;
  UINTN       DataSize;
  UINTN       NewEFIDebug;

  DataSize    = sizeof (EFIDebug);
  Status      = RT->GetVariable (L"EFIDebug", &gEfiGenericVariableGuid, &Attributes, &DataSize, &NewEFIDebug);
  if (!EFI_ERROR (Status)) {
    EFIDebug = NewEFIDebug;
  }
}
