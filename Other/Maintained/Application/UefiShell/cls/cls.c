/*++

Copyright (c) 2005 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  cls.c
  
Abstract:


Revision History

--*/

#include "EfiShellLib.h"
#include "cls.h"

extern UINT8    STRING_ARRAY_NAME[];

//
// This is the generated header file which includes whatever needs to be exported (strings + IFR)
//
#include STRING_DEFINES_FILE

EFI_HII_HANDLE  HiiHandle;
EFI_GUID        EfiClsGuid = EFI_CLS_GUID;
SHELL_VAR_CHECK_ITEM    ClsCheckList[] = {
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
EFIAPI
InitializeCls (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  );

EFI_BOOTSHELL_CODE(
  EFI_DRIVER_ENTRY_POINT (InitializeCls)
)

EFI_STATUS
EFIAPI
InitializeCls (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
/*++

Routine Description:

  Clear the sreen

Arguments:

  ImageHandle - The image handle
  SystemTable - The system table

Returns:

  EFI_SUCCESS - Success

--*/
{
  EFI_STATUS              Status;
  UINTN                   Background;
  UINTN                   ForeColor;
  CHAR16                  *Useful;
  SHELL_VAR_CHECK_CODE    RetCode;
  SHELL_VAR_CHECK_PACKAGE ChkPck;

  ZeroMem (&ChkPck, sizeof (SHELL_VAR_CHECK_PACKAGE));

  //
  // We are no being installed as an internal command driver, initialize
  // as an nshell app and run
  //
  EFI_SHELL_APP_INIT (ImageHandle, SystemTable);

  //
  // Enable tab key which can pause the output
  //
  EnableOutputTabPause();
  
  Status = LibInitializeStrings (&HiiHandle, STRING_ARRAY_NAME, &EfiClsGuid);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  RetCode = LibCheckVariables (SI, ClsCheckList, &ChkPck, &Useful);
  if (VarCheckOk != RetCode) {
    switch (RetCode) {
    case VarCheckUnknown:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_UNKNOWN_FLAG), HiiHandle, L"cls", Useful);
      break;

    case VarCheckDuplicate:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_DUP_FLAG), HiiHandle, L"cls", Useful);
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
    if (IS_OLD_SHELL) {
      PrintToken (STRING_TOKEN (STR_NO_HELP), HiiHandle);
    } else if (ChkPck.ValueCount > 0 ||
             ChkPck.FlagCount > 2 ||
             (2 == ChkPck.FlagCount && !LibCheckVarGetFlag (&ChkPck, L"-b"))
            ) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiHandle, L"cls");
      Status = EFI_INVALID_PARAMETER;
    } else {
      PrintToken (STRING_TOKEN (STR_CLS_VERBOSE_HELP), HiiHandle);
      Status = EFI_SUCCESS;
    }

    goto Done;
  }

  if (ChkPck.ValueCount > 1) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiHandle, L"cls");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  Background = 0;
  if (ChkPck.ValueCount == 1) {
    Background = (UINTN) StrToUIntegerBase (ChkPck.VarList->VarStr, 10, &Status);
    if (EFI_ERROR (Status) || Background > 7) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_ARG), HiiHandle, L"cls", ChkPck.VarList->VarStr);
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }

    ForeColor = (~Background) & 0x07;
    ST->ConOut->SetAttribute (ST->ConOut, (ForeColor & 0x0f) | (Background << 4));
  } else {
    Background  = (ST->ConOut->Mode->Attribute >> 4) & 0x0f;
    ForeColor   = (~Background) & 0x07;
    ST->ConOut->SetAttribute (ST->ConOut, (ForeColor & 0x0f) | (Background << 4));
  }

  ST->ConOut->ClearScreen (ST->ConOut);
  Status = EFI_SUCCESS;

Done:
  LibCheckVarFreeVarList (&ChkPck);
  LibUnInitializeStrings ();
  return Status;
}

EFI_STATUS
EFIAPI
InitializeClsGetLineHelp (
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
  return LibCmdGetStringByToken (STRING_ARRAY_NAME, &EfiClsGuid, STRING_TOKEN (STR_CLS_LINE_HELP), Str);
}
