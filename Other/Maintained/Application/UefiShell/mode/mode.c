/*++

Copyright (c) 2005 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  mode.c
   
Abstract:

  EFI Shell command "mode"


Revision History

--*/

#include "EfiShellLib.h"
#include "mode.h"

extern UINT8  STRING_ARRAY_NAME[];

//
// This is the generated header file which includes whatever needs to be exported (strings + IFR)
//
#include STRING_DEFINES_FILE

//
//
//
EFI_GUID  EfiModeGuid = EFI_MODE_GUID;
SHELL_VAR_CHECK_ITEM    ModeCheckList[] = {
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
InitializeMode (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  );

//
//
//
EFI_BOOTSHELL_CODE(
  EFI_APPLICATION_ENTRY_POINT(InitializeMode)
)

EFI_STATUS
InitializeMode (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
/*++

Routine Description:
  Command entry point. 

Arguments:
  ImageHandle     The image handle.
  SystemTable     The system table.

Returns:
  EFI_SUCCESS             - The command completed successfully
  EFI_INVALID_PARAMETER   - Command usage error
  Other value             - Unknown error

--*/
{
  UINTN                         NewCol;
  UINTN                         NewRow;
  UINTN                         Col;
  UINTN                         Row;
  INTN                          Mode;
  EFI_STATUS                    Status;
  EFI_HII_HANDLE                HiiHandle;

  EFI_SIMPLE_TEXT_OUT_PROTOCOL  *ConOut;

  SHELL_VAR_CHECK_CODE          RetCode;
  CHAR16                        *Useful;
  SHELL_ARG_LIST                *Item;
  SHELL_VAR_CHECK_PACKAGE       ChkPck;

  //
  // Initialize app
  //
  ZeroMem (&ChkPck, sizeof (SHELL_VAR_CHECK_PACKAGE));

  EFI_SHELL_APP_INIT (ImageHandle, SystemTable);
  
  //
  // Enable tab key which can pause the output
  //
  EnableOutputTabPause();

  Status  = LibInitializeStrings (&HiiHandle, STRING_ARRAY_NAME, &EfiModeGuid);

  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (!EFI_PROPER_VERSION (0, 99)) {
    PrintToken (
      STRING_TOKEN (STR_SHELLENV_GNC_COMMAND_NOT_SUPPORT),
      HiiHandle,
      L"mode",
      EFI_VERSION_0_99 
      );
    Status = EFI_UNSUPPORTED;
    goto Done;
  }

  ConOut  = ST->ConOut;

  RetCode = LibCheckVariables (SI, ModeCheckList, &ChkPck, &Useful);
  if (VarCheckOk != RetCode) {
    switch (RetCode) {
    case VarCheckDuplicate:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_DUP_FLAG), HiiHandle, L"mode", Useful);
      break;

    case VarCheckUnknown:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_UNKNOWN_FLAG), HiiHandle, L"mode", Useful);
      break;

    default:
      break;
    }

    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }
  //
  // Out put help.
  //
  if (LibCheckVarGetFlag (&ChkPck, L"-b") != NULL) {
    EnablePageBreak (DEFAULT_INIT_ROW, DEFAULT_AUTO_LF);
  }

  if (LibCheckVarGetFlag (&ChkPck, L"-?") != NULL) {
    if (ChkPck.ValueCount > 0 ||
        ChkPck.FlagCount > 2 ||
        (2 == ChkPck.FlagCount && !LibCheckVarGetFlag (&ChkPck, L"-b"))
        ) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiHandle, L"mode");
      Status = EFI_INVALID_PARAMETER;
    } else {
      PrintToken (STRING_TOKEN (STR_MODE_VERBOSE_HELP), HiiHandle);
      Status = EFI_SUCCESS;
    }

    goto Done;
  }

  if (ChkPck.ValueCount == 0) {
    PrintToken (STRING_TOKEN (STR_MODE_STAND_OUTPUT), HiiHandle);
    //
    // dump the available modes
    //
    for (Mode = 0; Mode < ConOut->Mode->MaxMode; Mode++) {
      Status = ConOut->QueryMode (ConOut, Mode, &Col, &Row);
      if (EFI_ERROR (Status)) {
        if (Mode == ConOut->Mode->MaxMode - 1) {
          PrintToken (STRING_TOKEN (STR_MODE_CANNOT_QUERY), HiiHandle, Status);
          Status = EFI_INVALID_PARAMETER;
          goto Done;
        } else {
          continue;
        }
      }
      //
      // Current mode
      //
      PrintToken (STRING_TOKEN (STR_MODE_COL_ROW), HiiHandle, Col, Row, Mode == ConOut->Mode->Mode ? '*' : ' ');
    }
  } else if (ChkPck.ValueCount == 2) {
    Item    = ChkPck.VarList;
    NewCol  = (UINTN) StrToUInteger (Item->VarStr, &Status);
    if (EFI_ERROR (Status)) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_ARG), HiiHandle, L"mode", Item->VarStr);
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }

    Item    = Item->Next;
    NewRow  = (UINTN) StrToUInteger (Item->VarStr, &Status);
    if (EFI_ERROR (Status)) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_ARG), HiiHandle, L"mode", Item->VarStr);
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }

    for (Mode = 0; Mode < ConOut->Mode->MaxMode; Mode++) {
      Status = ConOut->QueryMode (ConOut, Mode, &Col, &Row);
      if (EFI_ERROR (Status)) {
        if (Mode == ConOut->Mode->MaxMode - 1) {
          PrintToken (STRING_TOKEN (STR_MODE_CANNOT_QUERY), HiiHandle, Status);
          Status = EFI_INVALID_PARAMETER;
          goto Done;
        } else {
          continue;
        }
      }

      if (Row == NewRow && Col == NewCol) {
        ConOut->SetMode (ConOut, Mode);
        ConOut->ClearScreen (ConOut);
        Status = EFI_SUCCESS;
        goto Done;
      }
    }

    PrintToken (STRING_TOKEN (STR_MODE_MODE_NOT_FOUND), HiiHandle, NewCol, NewRow);
  } else if (ChkPck.ValueCount == 1) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_FEW), HiiHandle, L"mode");
    Status = EFI_INVALID_PARAMETER;
  } else {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiHandle, L"mode");
    Status = EFI_INVALID_PARAMETER;
  }

Done:
  LibCheckVarFreeVarList (&ChkPck);
  LibUnInitializeStrings ();
  return Status;
}

EFI_STATUS
InitializeModeGetLineHelp (
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
  return LibCmdGetStringByToken (STRING_ARRAY_NAME, &EfiModeGuid, STRING_TOKEN (STR_MODE_LINE_HELP), Str);
}
