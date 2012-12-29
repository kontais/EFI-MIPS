/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  help.c 

Abstract:

  EFI shell command "help"

Revision History

--*/

#include "shelle.h"
#include "shellenvguid.h"

extern EFI_LIST_ENTRY SEnvCmds;
CHAR16 *InvalidCmd[] = {
  L"...",
  L"_load_defaults",
  L"_this_is_not_a_useful_command",
  L"endfor",
  L"endif",
  L"else",
  NULL
};

SHELL_VAR_CHECK_ITEM    HelpCheckList[] = {
  {
    L"-b",
    0x01,
    0,
    FlagTypeSingle
  },
  {
    L"-?",
    0x02,
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

BOOLEAN
IsWildMatch (
  IN const CHAR16             *String
  )
/*++

Routine Description:
	to judge if there is '*' in String
	
Arguments:

Returns:
      TRUE   - if String contains '*'
--*/
{
  const CHAR16  *Start;

  Start = NULL;
  while (*String) {
    if (L'*' == *String || L'?' == *String) {
      return TRUE;
    }

    if (L'[' == *String) {
      Start = String;
      while (*Start && L']' != *Start) {
        Start++;
      }

      if (*Start) {
        return TRUE;
      }
    }

    String++;
  }

  return FALSE;
}

EFI_STATUS
PrintVerboseHelp (
  IN     EFI_HANDLE           ImageHandle,
  IN     COMMAND              *Command
  )
{
  EFI_STATUS  Status;
  CHAR16      *Cmd;

  Cmd = AllocatePool (StrSize (Command->Cmd) + StrSize (L" -?") - sizeof (CHAR16));
  if (NULL == Cmd) {
    return EFI_OUT_OF_RESOURCES;
  }

  StrCpy (Cmd, Command->Cmd);
  StrCat (Cmd, L" -?");
  Status = ShellExecute (ImageHandle, Cmd, FALSE);
  FreePool (Cmd);

  return Status;
}

BOOLEAN
IsValidCommand (
  IN CHAR16                   *Cmd
  )
{
  UINTN         Index;

  for (Index = 0; InvalidCmd[Index]; Index++) {
    if (0 == StriCmp (Cmd, InvalidCmd[Index])) {
      return FALSE;
    }
  }

  return TRUE;
}

EFI_STATUS
EFIAPI
SEnvHelpGetLineHelp (
  IN CHAR16                   **Str
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
  return SEnvCmdGetStringByToken (STRING_TOKEN (STR_SHELLENV_HELP_LINE_HELP), Str);
}

EFI_STATUS
EFIAPI
SEnvHelp (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  )
/*++

Routine Description:

Arguments:

  ImageHandle - The image handle
  SystemTable - The system table

Returns:

--*/
{
  EFI_STATUS              Status;
  EFI_LIST_ENTRY          *Link;
  COMMAND                 *Command;
  UINTN                   SynLen;
  UINTN                   Len;
  CHAR16                  *Pattern;
  BOOLEAN                 WildMatched;
  SHELL_VAR_CHECK_CODE    RetCode;
  CHAR16                  *Useful;
  SHELL_VAR_CHECK_PACKAGE ChkPck;
  SHELL_ARG_LIST          *Item;

  //
  // Initialize application
  //
  Pattern     = NULL;
  WildMatched = FALSE;
  ZeroMem (&ChkPck, sizeof (SHELL_VAR_CHECK_PACKAGE));

  EFI_SHELL_APP_INIT (ImageHandle, SystemTable);

  Status = LibFilterNullArgs ();
  if (EFI_ERROR (Status)) {
    goto Quit;
  }

  RetCode = LibCheckVariables (SI, HelpCheckList, &ChkPck, &Useful);
  if (VarCheckOk != RetCode) {
    switch (RetCode) {
    case VarCheckDuplicate:

      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_DUP_FLAG), HiiEnvHandle, L"help", Useful);
      break;

    case VarCheckUnknown:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_UNKNOWN_FLAG), HiiEnvHandle, L"help", Useful);
      break;

    default:

      break;
    }

    Status = EFI_INVALID_PARAMETER;
    goto Quit;
  }

  if (ChkPck.ValueCount > 1) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiEnvHandle, L"help");
    Status = EFI_INVALID_PARAMETER;
    goto Quit;
  }

  Item = LibCheckVarGetFlag (&ChkPck, L"-b");
  if (Item) {
    EnablePageBreak (DEFAULT_INIT_ROW, DEFAULT_AUTO_LF);
  }

  if (LibCheckVarGetFlag (&ChkPck, L"-?")) {
    if (IS_OLD_SHELL) {
      PrintToken (STRING_TOKEN (STR_NO_HELP), HiiEnvHandle);
      goto Quit;
    }

    if (ChkPck.ValueCount > 0 ||
        ChkPck.FlagCount > 2 ||
        (2 == ChkPck.FlagCount && !LibCheckVarGetFlag (&ChkPck, L"-b"))
        ) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiEnvHandle, L"help");
      Status = EFI_INVALID_PARAMETER;
      goto Quit;
    }

    PrintToken (STRING_TOKEN (STR_SHELLENV_HELP_VERBOSE_HELP), HiiEnvHandle);
    goto Quit;
  }

  AcquireLock (&SEnvLock);

  if (1 == ChkPck.ValueCount && !IsWildMatch (ChkPck.VarList->VarStr)) {
    //
    // Process 'help cmd'
    //
    for (Link = SEnvCmds.Flink; Link != &SEnvCmds; Link = Link->Flink) {
      Command = CR (Link, COMMAND, Link, COMMAND_SIGNATURE);
      if (!IsValidCommand (Command->Cmd)) {
        continue;
      }

      if (0 == StriCmp (Command->Cmd, ChkPck.VarList->VarStr)) {
        ReleaseLock (&SEnvLock);
        Status = PrintVerboseHelp (ImageHandle, Command);
        AcquireLock (&SEnvLock);
        break;
      }
    }

    if (Link == &SEnvCmds) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_HELP_UNKNOWN_COMMAND), HiiEnvHandle, ChkPck.VarList->VarStr);
      Status = EFI_NOT_FOUND;
    }
  } else {
    //
    // process help [-b] / help pattern [-b]
    //
    if (1 == ChkPck.ValueCount) {
      Pattern = ChkPck.VarList->VarStr;
    }

    SynLen = 0;
    for (Link = SEnvCmds.Flink; Link != &SEnvCmds; Link = Link->Flink) {
      Command = CR (Link, COMMAND, Link, COMMAND_SIGNATURE);
      if (Command->Cmd && IsValidCommand (Command->Cmd)) {
        Len = StrLen (Command->Cmd);
        //
        // Make help line aligned
        //
        if (Len > SynLen) {
          SynLen = Len;
        }
      }
    }

    for (Link = SEnvCmds.Flink; Link != &SEnvCmds; Link = Link->Flink) {
      //
      // Break the execution?
      //
      if (GetExecutionBreak ()) {
        goto Done;
      }

      Command = CR (Link, COMMAND, Link, COMMAND_SIGNATURE);
      if (!IsValidCommand (Command->Cmd)) {
        continue;
      }

      if (Pattern) {
        if (!MetaiMatch (Command->Cmd, Pattern)) {
          continue;
        }

        WildMatched = TRUE;
      }

      if (Command->GetLineHelp) {
        Status = (*Command->GetLineHelp) (&Useful);
        PrintToken (STRING_TOKEN (STR_SHELLENV_HELP_HS), HiiEnvHandle, SynLen, Command->Cmd, Useful ? Useful : L"");
        if (Useful) {
          FreePool (Useful);
        }
      } else {
        PrintToken (STRING_TOKEN (STR_SHELLENV_HELP_HS), HiiEnvHandle, SynLen, Command->Cmd, L"");
      }
    }
    //
    // end of for (Link = SEnv...
    //
    if (Pattern && !WildMatched) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_HELP_NO_MATCH), HiiEnvHandle, Pattern);
    }
  }

Done:
  ReleaseLock (&SEnvLock);
Quit:
  LibCheckVarFreeVarList (&ChkPck);
  return Status;
}
