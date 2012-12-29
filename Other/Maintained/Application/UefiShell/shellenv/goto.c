/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name: 

  goto.c
  
Abstract:

  Shell Environment batch goto command

Revision History

--*/

#include "shelle.h"
#include "shellenvguid.h"

//
//  Statics
//
STATIC CHAR16 *TargetLabel;

EFI_STATUS
EFIAPI
SEnvCmdGoto (
  IN  EFI_HANDLE               ImageHandle,
  IN  EFI_SYSTEM_TABLE         *SystemTable
  )
/*++

Routine Description:

  Transfers execution of batch file to location following a label (:labelname).

Arguments:
  ImageHandle      The image handle
  SystemTable      The system table

Returns:
  EFI_SUCCESS      The command finished sucessfully
  EFI_UNSUPPORTED  Unsupported
  EFI_INVALID_PARAMETER Invalid parameter
  EFI_OUT_OF_RESOURCES  Out of resources
  
--*/
{
  EFI_STATUS  Status;

  EFI_SHELL_APP_INIT (ImageHandle, SystemTable);

  //
  //  Output help
  //
  if (SI->Argc == 2) {
    if (StriCmp (SI->Argv[1], L"-?") == 0) {
      if (IS_OLD_SHELL) {
        PrintToken (STRING_TOKEN (STR_NO_HELP), HiiEnvHandle);
      } else {
        PrintToken (STRING_TOKEN (STR_SHELLENV_GOTO_VERBOSE_HELP), HiiEnvHandle);
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
        PrintToken (STRING_TOKEN (STR_SHELLENV_GOTO_VERBOSE_HELP), HiiEnvHandle);
      }

      return EFI_SUCCESS;
    }
  }

  if (!SEnvBatchIsActive ()) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GOTO_ONLY_SUPPORTED_SCRIPT), HiiEnvHandle);
    return EFI_UNSUPPORTED;
  }

  if (SI->Argc > 2) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GOTO_TOO_MANY_ARGS), HiiEnvHandle, SEnvGetLineNumber ());
    SEnvSetBatchAbort ();
    return EFI_INVALID_PARAMETER;
  }

  if (SI->Argc < 2) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GOTO_TOO_FEW_ARGS), HiiEnvHandle, SEnvGetLineNumber ());
    SEnvSetBatchAbort ();
    return EFI_INVALID_PARAMETER;
  }

  TargetLabel = StrDuplicate (SI->Argv[1]);
  if (TargetLabel == NULL) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GOTO_OUT_OF_RESOURCES), HiiEnvHandle);
    SEnvSetBatchAbort ();
    return EFI_OUT_OF_RESOURCES;
  }

  SEnvBatchSetGotoActive ();
  Status = SEnvBatchResetJumpStmt ();
  if (EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GOTO_CANNOT_EXECUTE_SCRIPT), HiiEnvHandle, Status);
    SEnvSetBatchAbort ();
    return Status;
  }

  return Status;
}

EFI_STATUS
SEnvCheckForGotoTarget (
  IN  CHAR16                   *Candidate,
  IN  UINT64                   GotoFilePos,
  IN  UINT64                   FilePosition,
  OUT UINTN                    *GotoTargetStatus
  )
/*++

Routine Description:

  Check to see if we have found the target label of a GOTO command.

Arguments:
  Candidate        String to be checked for goto target
  GotoFilePos      File position of the goto statement
  FilePosition     Current file position
  GotoTargetStatus The status of searching goto target
  
Returns:
  EFI_SUCCESS      The command finished sucessfully
  EFI_INVALID_PARAMETER Invalid parameter
  
--*/
{
  if (!Candidate) {
    return EFI_INVALID_PARAMETER;
  }
  //
  //  See if we found the label (strip the leading ':' off the candidate)
  //  or if we have searched the whole file without finding it.
  //
  if (GotoFilePos == FilePosition) {
    *GotoTargetStatus = GOTO_TARGET_DOESNT_EXIST;
    return EFI_SUCCESS;

  } else if (Candidate[0] != ':') {
    *GotoTargetStatus = GOTO_TARGET_NOT_FOUND;
    return EFI_SUCCESS;

  } else if (StriCmp (&Candidate[1], TargetLabel) == 0) {
    *GotoTargetStatus = GOTO_TARGET_FOUND;
    return EFI_SUCCESS;

  } else {
    *GotoTargetStatus = GOTO_TARGET_NOT_FOUND;
    return EFI_SUCCESS;
  }
}

VOID
SEnvPrintLabelNotFound (
  VOID
  )
/*++

Routine Description:

  Print an error message when a label referenced by a GOTO is not
  found in the script file..

Arguments:

Returns:

--*/
{
  PrintToken (
    STRING_TOKEN (STR_SHELLENV_GOTO_TARGET_LABEL),
    HiiEnvHandle,
    TargetLabel,
    SEnvGetLineNumber ()
    );
  return ;
}

VOID
SEnvInitTargetLabel (
  VOID
  )
/*++

Routine Description:

  Initialize the target label for the GOTO command.

Arguments:

Returns:

--*/
{
  TargetLabel = NULL;
  return ;
}

VOID
SEnvFreeTargetLabel (
  VOID
  )
/*++

Routine Description:

  Free the target label saved from the GOTO command.

Arguments:

Returns:

--*/
{
  if (TargetLabel) {
    FreePool (TargetLabel);
    TargetLabel = NULL;
  }

  return ;
}

EFI_STATUS
EFIAPI
SEnvCmdGotoGetLineHelp (
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
  return SEnvCmdGetStringByToken (STRING_TOKEN (STR_SHELLENV_GOTO_LINE_HELP), Str);
}
