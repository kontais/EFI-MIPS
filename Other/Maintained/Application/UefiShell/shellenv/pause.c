/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  pause.c
  
Abstract:

  Internal Shell batch cmd "pause" 

Revision History

--*/

#include "shelle.h"
#include "shellenvguid.h"

SHELL_VAR_CHECK_ITEM    PauseCheckList[] = {
  {
    L"-q",
    0x02,
    0x01,
    FlagTypeSingle
  },
  {
    L"-b",
    0x04,
    0,
    FlagTypeSingle
  },
  {
    L"-?",
    0x08,
    0,
    FlagTypeSingle
  },
  {
    NULL,
    0,
    0,
    0
  }
};

//
// Internal prototypes
//
EFI_STATUS
EFIAPI
SEnvCmdPause (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
/*++

Routine Description:

  Built-in shell command "pause" for interactive continue/abort 
  functionality from scripts.

Arguments:

  ImageHandle - The image handle
  SystemTable - The system table

Returns:

--*/
{
  EFI_STATUS                    Status;
  EFI_SIMPLE_TEXT_IN_PROTOCOL   *TextIn;
  EFI_SIMPLE_TEXT_OUT_PROTOCOL  *TextOut;
  BOOLEAN                       Quiet;
  EFI_INPUT_KEY                 Key;
  CHAR16                        QStr[2];
  CHAR16                        *Useful;
  SHELL_VAR_CHECK_CODE          RetCode;
  SHELL_VAR_CHECK_PACKAGE       ChkPck;

  ZeroMem (&ChkPck, sizeof (SHELL_VAR_CHECK_PACKAGE));
  Status  = EFI_SUCCESS;
  Quiet   = FALSE;

  EFI_SHELL_APP_INIT (ImageHandle, SystemTable);

  //
  //  No args: print status
  //  One arg, either -on or -off: set console echo flag
  //  Otherwise: echo all the args.  Shell parser will expand any args or vars.
  //
  RetCode = LibCheckVariables (SI, PauseCheckList, &ChkPck, &Useful);
  if (VarCheckOk != RetCode) {
    switch (RetCode) {
    case VarCheckDuplicate:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_DUP_FLAG), HiiEnvHandle, L"pause", Useful);
      break;

    case VarCheckUnknown:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_UNKNOWN_FLAG), HiiEnvHandle, L"pause", Useful);
      break;

    case VarCheckConflict:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_FLAG_CONFLICT), HiiEnvHandle, L"pause", Useful);
      break;

    default:
      break;
    }

    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  if (LibCheckVarGetFlag (&ChkPck, L"-b") != NULL) {
    EnablePageBreak (DEFAULT_INIT_ROW, DEFAULT_AUTO_LF);
  }
  //
  // Out put help.
  //
  if (LibCheckVarGetFlag (&ChkPck, L"-?") != NULL) {
    if (ChkPck.ValueCount > 0 ||
        ChkPck.FlagCount > 2 ||
        (2 == ChkPck.FlagCount && !LibCheckVarGetFlag (&ChkPck, L"-b"))
        ) {
      PrintToken (
        STRING_TOKEN (STR_SHELLENV_PAUSE_TOO_MANY_ARGUMENTS),
        HiiEnvHandle,
        L"pause",
        SEnvGetLineNumber ()
        );
      Status = EFI_INVALID_PARAMETER;
    } else {
      if (IS_OLD_SHELL) {
        PrintToken (STRING_TOKEN (STR_NO_HELP), HiiEnvHandle);
      } else {
        PrintToken (STRING_TOKEN (STR_SHELLENV_PAUSE_VERBOSE_HELP), HiiEnvHandle);
      }

      Status = EFI_SUCCESS;
    }

    goto Done;
  }

  //
  // Pause only takes affect in batch script
  //
  if (!SEnvBatchIsActive ()) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_PAUSE_ONLY_SUPPORTED_SCRIPT), HiiEnvHandle);
    Status = EFI_UNSUPPORTED;
    goto Done;
  }

  if (ChkPck.ValueCount != 0) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiEnvHandle, L"pause");
    SEnvSetBatchAbort ();
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  if (LibCheckVarGetFlag (&ChkPck, L"-q") != NULL) {
    Quiet = TRUE;
  }

  SEnvBatchGetConsole (&TextIn, &TextOut);

  if (FALSE == Quiet) {
    Status = TextOut->OutputString (
                        TextOut,
                        L"Enter 'q' to quit, any other key to continue: "
                        );
    if (EFI_ERROR (Status)) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_PAUSE_WRITING_PROMPT_ERROR), HiiEnvHandle);
      goto Done;
    }
  }
  //
  // Wait for user input
  //
  WaitForSingleEvent (TextIn->WaitForKey, 0);
  Status = TextIn->ReadKeyStroke (TextIn, &Key);
  if (EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_PAUSE_READING_KEYSTROKE), HiiEnvHandle);
    goto Done;
  }
  //
  //  Check if input character is q or Q, if so set abort flag
  //
  if (Key.UnicodeChar == L'q' || Key.UnicodeChar == L'Q') {
    SEnvSetBatchAbort ();
  }

  if (Key.UnicodeChar != (CHAR16) 0x0000) {
    QStr[0] = Key.UnicodeChar;
    QStr[1] = (CHAR16) 0x0000;
    TextOut->OutputString (TextOut, QStr);
  }

  TextOut->OutputString (TextOut, L"\n\r");

Done:
  return Status;
}

EFI_STATUS
EFIAPI
SEnvCmdPauseGetLineHelp (
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
  return SEnvCmdGetStringByToken (STRING_TOKEN (STR_SHELLENV_PAUSE_LINE_HELP), Str);
}
