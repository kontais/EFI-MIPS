/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  shift.c
  
Abstract:

  Internal Shell batch cmd "shift" 

Revision History

--*/

#include "shelle.h"
#include "shellenvguid.h"

//
// Internal prototypes
//
EFI_STATUS
EFIAPI
SEnvCmdShift (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
/*++

Routine Description:

  Built-in shell command "shift" for shift the positional arguments 
  functionality from scripts.

Arguments:

  ImageHandle - The image handle
  SystemTable - The system table

Returns:
  EFI_SUCCESS - Success

--*/
{
  UINTN       Argc;
  EFI_STATUS  Status;

  Argc    = 0;
  Status  = EFI_SUCCESS;
  EFI_SHELL_APP_INIT (ImageHandle, SystemTable);

  //
  //  Output help
  //
  if (SI->Argc == 2) {
    if (StriCmp (SI->Argv[1], L"-?") == 0) {
      if (IS_OLD_SHELL) {
        PrintToken (STRING_TOKEN (STR_NO_HELP), HiiEnvHandle);
      } else {
        PrintToken (STRING_TOKEN (STR_SHELLENV_SHIFT_VERBOSE_HELP), HiiEnvHandle);
      }

      return EFI_SUCCESS;
    }
  } else if (SI->Argc == 3) {
    if ((StriCmp (SI->Argv[1], L"-?") == 0 && StriCmp (SI->Argv[2], L"-b") == 0) ||
        (StriCmp (SI->Argv[2], L"-?") == 0 && StriCmp (SI->Argv[1], L"-b") == 0)
        ) {
      EnablePageBreak (DEFAULT_INIT_ROW, DEFAULT_AUTO_LF);
      if (IS_OLD_SHELL) {
        PrintToken (STRING_TOKEN (STR_NO_HELP), HiiEnvHandle);
      } else {
        PrintToken (STRING_TOKEN (STR_SHELLENV_SHIFT_VERBOSE_HELP), HiiEnvHandle);
      }

      return EFI_SUCCESS;
    }
  }

  Argc  = SI->Argc;

  //
  // Pause only takes affect in batch script
  //
  if (!SEnvBatchIsActive ()) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_ONLY_SCRIPT), HiiEnvHandle, L"shift");
    Status = EFI_UNSUPPORTED;
    goto Done;
  }

  if (Argc > 1) {
    PrintToken (
      STRING_TOKEN (STR_SHELLENV_SCRIPT_TOO_MANY_ARGUMENTS),
      HiiEnvHandle,
      L"shift",
      SEnvGetLineNumber ()
      );
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  SEnvBatchShiftArgument ();

Done:
  return Status;
}

EFI_STATUS
EFIAPI
SEnvCmdShiftGetLineHelp (
  OUT CHAR16              **Str
  )
/*++

Routine Description:

  Get this command's line help

Arguments:

  Str - The line help

Returns:

  EFI_SUCCESS   - Success

--*/
{
  return SEnvCmdGetStringByToken (STRING_TOKEN (STR_SHELLENV_SHIFT_LINE_HELP), Str);
}
