/*++

Copyright (c) 2005 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  stall.c 
  
Abstract: 

  Shell app "stall" - stall for some microseconds

Revision History

--*/

#include "EfiShellLib.h"
#include "stall.h"

extern UINT8    STRING_ARRAY_NAME[];

//
// This is the generated header file which includes whatever needs to be exported (strings + IFR)
//
#include STRING_DEFINES_FILE

EFI_HII_HANDLE  HiiStallHandle;
EFI_GUID        EfiStallGuid = EFI_STALL_GUID;
SHELL_VAR_CHECK_ITEM    StallCheckList[] = {
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

EFI_STATUS
InitializeStall (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_BOOTSHELL_CODE(
  EFI_APPLICATION_ENTRY_POINT(InitializeStall)
)

EFI_STATUS
InitializeStall (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
/*++
Routine Description:
  Command entry point. Stall for [microseconds].

Arguments:
  ImageHandle     The image handle. 
  SystemTable     The system table.

Returns:
  EFI_SUCCESS             - The command completed successfully
  EFI_INVALID_PARAMETER   - Command usage error
  EFI_UNSUPPORTED         - Stall service is not supported
--*/
{
  UINTN                   Microseconds;
  UINTN                   Remaining;
  EFI_TIME                TmpTime;
  UINTN                   MilliSeconds1;
  UINTN                   MilliSeconds2;
  EFI_STATUS              Status;
  SHELL_VAR_CHECK_CODE    RetCode;
  CHAR16                  *Useful;
  SHELL_VAR_CHECK_PACKAGE ChkPck;

  ZeroMem (&ChkPck, sizeof (SHELL_VAR_CHECK_PACKAGE));
  EFI_SHELL_APP_INIT (ImageHandle, SystemTable);
  
  //
  // Enable tab key which can pause the output
  //
  EnableOutputTabPause();

  Status = LibInitializeStrings (&HiiStallHandle, STRING_ARRAY_NAME, &EfiStallGuid);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (!EFI_PROPER_VERSION (0, 99)) {
    PrintToken (
      STRING_TOKEN (STR_SHELLENV_GNC_COMMAND_NOT_SUPPORT),
      HiiStallHandle,
      L"stall",
      EFI_VERSION_0_99 
      );
    Status = EFI_UNSUPPORTED;
    goto Done;
  }

  LibFilterNullArgs ();
  RetCode = LibCheckVariables (SI, StallCheckList, &ChkPck, &Useful);
  if (VarCheckOk != RetCode) {
    switch (RetCode) {
    case VarCheckUnknown:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_UNKNOWN_FLAG), HiiStallHandle, L"stall", Useful);
      break;

    default:
      break;
    }

    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  if (LibCheckVarGetFlag (&ChkPck, L"-b")) {
    EnablePageBreak (DEFAULT_INIT_ROW, DEFAULT_AUTO_LF);
  }

  if (LibCheckVarGetFlag (&ChkPck, L"-?")) {
    if (ChkPck.ValueCount > 0 ||
        ChkPck.FlagCount > 2 ||
        (2 == ChkPck.FlagCount && !LibCheckVarGetFlag (&ChkPck, L"-b"))
        ) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiStallHandle, L"stall");
      Status = EFI_INVALID_PARAMETER;
    } else {
      PrintToken (STRING_TOKEN (STR_STALL_VERBOSEHELP), HiiStallHandle);
      Status = EFI_SUCCESS;
    }

    goto Done;
  }

  if (ChkPck.ValueCount < 1) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_FEW), HiiStallHandle, L"stall");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  if (ChkPck.ValueCount > 1) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiStallHandle, L"stall");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  if (BS->Stall == NULL) {
    PrintToken (STRING_TOKEN (STR_STALL_SERVICES), HiiStallHandle, L"stall");
    Status = EFI_UNSUPPORTED;
    goto Done;
  }

  Useful = ChkPck.VarList->VarStr;
  while (*Useful && *Useful >= L'0' && *Useful <= L'9') {
    Useful++;
  }

  if (*Useful) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_ARG), HiiStallHandle, L"stall", ChkPck.VarList->VarStr);
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  } else {
    Useful = ChkPck.VarList->VarStr;
    while (*Useful && (L' ' == *Useful || L'0' == *Useful)) {
      Useful++;
    }

    if (StrLen (Useful) > 8) {
      PrintToken (STRING_TOKEN (STR_STALL_TOO_BIG), HiiStallHandle, L"stall", Useful);
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }
  }

  Microseconds = Atoi (ChkPck.VarList->VarStr);
  PrintToken (STRING_TOKEN (STR_STALL_FOR), HiiStallHandle, Microseconds);
  Remaining = Microseconds % 100000;
  Status    = RT->GetTime (&TmpTime, NULL);
  if (EFI_ERROR (Status)) {
    Status = EFI_ABORTED;
    goto Done;
  }

  MilliSeconds1 = (TmpTime.Hour * 3600 + TmpTime.Minute * 60 + TmpTime.Second) * 1000;
  while (Microseconds - Remaining) {
    //
    // Break the execution?
    //
    if (GetExecutionBreak ()) {
      goto Done;
    }

    BS->Stall (100000);
    Status = RT->GetTime (&TmpTime, NULL);
    if (EFI_ERROR (Status)) {
      Status = EFI_ABORTED;
      goto Done;
    }

    MilliSeconds2 = (TmpTime.Hour * 3600 + TmpTime.Minute * 60 + TmpTime.Second) * 1000;
    if (MilliSeconds2 < MilliSeconds1) {
      MilliSeconds2 += (24 * 3600 * 1000);
    }

    if ((Microseconds - Remaining) / 1000 <= (MilliSeconds2 - MilliSeconds1)) {
      break;
    }
  }

  BS->Stall (Remaining);

Done:
  LibCheckVarFreeVarList (&ChkPck);
  LibUnInitializeStrings ();
  return Status;
}

EFI_STATUS
InitializeStallGetLineHelp (
  OUT CHAR16            **Str
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
  return LibCmdGetStringByToken (STRING_ARRAY_NAME, &EfiStallGuid, STRING_TOKEN (STR_STALL_LINE_HELP), Str);
}
