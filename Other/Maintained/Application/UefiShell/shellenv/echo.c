/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  echo.c
  
Abstract:

  Shell app "echo"

Revision History

--*/

#include "shelle.h"
#include "shellenvguid.h"
SHELL_VAR_CHECK_ITEM    EchoCheckList[] = {
  {
    L"-on",
    0x01,
    0x0e,
    FlagTypeSingle
  },
  {
    L"-off",
    0x02,
    0x0d,
    FlagTypeSingle
  },
  {
    L"-b",
    0x04,
    0x3,
    FlagTypeSingle
  },
  {
    L"-?",
    0x08,
    0x3,
    FlagTypeSingle
  },
  {
    NULL,
    0,
    0,
    0
  }
};

EFI_STATUS
SEnvCmdEcho (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
/*++

Routine Description:

  Shell command "echo".

Arguments:

  ImageHandle - The image handle
  SystemTable - The system table
  
Returns:

--*/
{
  EFI_STATUS              Status;
  SHELL_VAR_CHECK_CODE    RetCode;
  CHAR16                  *Useful;
  SHELL_VAR_CHECK_PACKAGE ChkPck;
  SHELL_ARG_LIST          *Item;
  BOOLEAN                 PageBreak;

  EFI_SHELL_APP_INIT (ImageHandle, SystemTable);

  if (!EFI_PROPER_VERSION (0, 99)) {
    PrintToken (
      STRING_TOKEN (STR_SHELLENV_GNC_COMMAND_NOT_SUPPORT),
      HiiEnvHandle,
      L"echo",
      EFI_VERSION_0_99 
      );
    Status = EFI_UNSUPPORTED;
    goto Done;
  }

  Status    = EFI_SUCCESS;
  PageBreak = FALSE;
  ZeroMem (&ChkPck, sizeof (SHELL_VAR_CHECK_PACKAGE));

  //
  //  No args: print status
  //  One arg, either -on or -off: set console echo flag
  //  Otherwise: echo all the args.  Shell parser will expand any args or vars.
  //
  RetCode = LibCheckVariables (SI, EchoCheckList, &ChkPck, &Useful);
  if (VarCheckOk != RetCode) {
    switch (RetCode) {
    case VarCheckDuplicate:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_DUP_FLAG), HiiEnvHandle, L"echo", Useful);
      break;

    case VarCheckUnknown:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_UNKNOWN_FLAG), HiiEnvHandle, L"echo", Useful);
      break;

    case VarCheckConflict:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_FLAG_CONFLICT), HiiEnvHandle, L"echo", Useful);
      break;

    default:
      break;
    }

    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  if (LibCheckVarGetFlag (&ChkPck, L"-b") != NULL) {
    EnablePageBreak (DEFAULT_INIT_ROW, DEFAULT_AUTO_LF);
    PageBreak = TRUE;
  }

  //
  // Out put help.
  //
  if (LibCheckVarGetFlag (&ChkPck, L"-?") != NULL) {
    if (IS_OLD_SHELL) {
      PrintToken (STRING_TOKEN (STR_NO_HELP), HiiEnvHandle);
    } else if (ChkPck.ValueCount > 0 ||
             ChkPck.FlagCount > 2 ||
             (2 == ChkPck.FlagCount && !LibCheckVarGetFlag (&ChkPck, L"-b"))
            ) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiEnvHandle, L"echo");
      Status = EFI_INVALID_PARAMETER;
    } else {
      PrintToken (STRING_TOKEN (STR_SHELLENV_ECHO_VERBOSE_HELP), HiiEnvHandle);
    }

    goto Done;
  }

  if (LibCheckVarGetFlag (&ChkPck, L"-on") != NULL) {
    if (ChkPck.ValueCount != 0) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiEnvHandle, L"echo");
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    } else {
      SEnvBatchSetEcho (TRUE);
    }
  } else if (LibCheckVarGetFlag (&ChkPck, L"-off") != NULL) {
    if (ChkPck.ValueCount != 0) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiEnvHandle, L"echo");
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    } else {
      SEnvBatchSetEcho (FALSE);
    }
  }

  if ((ChkPck.FlagCount == 0 || (1 == ChkPck.FlagCount && PageBreak)) && ChkPck.ValueCount == 0) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_ECHO_ECHO_IS), HiiEnvHandle, (SEnvBatchGetEcho () ? L"on" : L"off"));
  } else if (ChkPck.ValueCount > 0) {
    Item = GetFirstArg (&ChkPck);
    for (;;) {
      Print (L"%s", Item->VarStr);
      Item = GetNextArg (Item);
      if (!Item) {
        break;
      }

      Print (L" ");
    }

    Print (L"\n");
  }

Done:
  LibCheckVarFreeVarList (&ChkPck);
  return Status;
}

EFI_STATUS
EFIAPI
SEnvCmdEchoGetLineHelp (
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
  return SEnvCmdGetStringByToken (STRING_TOKEN (STR_SHELLENV_ECHO_LINE_HELP), Str);
}
