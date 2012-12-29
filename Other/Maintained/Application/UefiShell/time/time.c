/*++
 
Copyright (c) 2005 - 2007, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name: 

  time.c
  
Abstract: 

  EFI shell command "time"

Revision History

--*/

#include "EfiShellLib.h"
#include "time.h"

extern UINT8  STRING_ARRAY_NAME[];

//
// This is the generated header file which includes whatever needs to be exported (strings + IFR)
//
#include STRING_DEFINES_FILE

EFI_GUID      EfiTimeGuid = EFI_TIME_GUID;
SHELL_VAR_CHECK_ITEM    TimeCheckList[] = {
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
InitializeTime (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

STATIC
BOOLEAN
GetNumber (
  IN      CHAR16  *Str,
  IN OUT  INTN    *Offset,
  IN OUT  INTN    *number,
  IN      BOOLEAN GetSecond
  );

EFI_BOOTSHELL_CODE(
  EFI_APPLICATION_ENTRY_POINT(InitializeTime)
)

EFI_STATUS
EFIAPI
InitializeTime (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
/*++
Routine Description:
  time [hh:mm:ss] 

Arguments:
  ImageHandle   - The image handle
  SystemTable   - The system table
  
Returns:

--*/
{
  EFI_STATUS              Status;
  EFI_TIME                Time;
  UINTN                   Offset;
  UINTN                   Data;
  EFI_HII_HANDLE          HiiHandle;
  INTN                    nValue;
  UINTN                   uValueSize;
  CHAR16                  wchSign;

  SHELL_VAR_CHECK_CODE    RetCode;
  CHAR16                  *Useful;
  SHELL_VAR_CHECK_PACKAGE ChkPck;

  ZeroMem (&ChkPck, sizeof (SHELL_VAR_CHECK_PACKAGE));
  EFI_SHELL_APP_INIT (ImageHandle, SystemTable);
  
  //
  // Enable tab key which can pause the output
  //
  EnableOutputTabPause();

  //
  // Register our string package with HII and return the handle to it.
  // If previously registered we will simply receive the handle
  //
  wchSign = L'+';
  Status  = LibInitializeStrings (&HiiHandle, STRING_ARRAY_NAME, &EfiTimeGuid);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (!EFI_PROPER_VERSION (0, 99)) {
    PrintToken (
      STRING_TOKEN (STR_SHELLENV_GNC_COMMAND_NOT_SUPPORT),
      HiiHandle,
      L"time",
      EFI_VERSION_0_99 
      );
    Status = EFI_UNSUPPORTED;
    goto Done;
  }

  RetCode = LibCheckVariables (SI, TimeCheckList, &ChkPck, &Useful);
  if (VarCheckOk != RetCode) {
    switch (RetCode) {
    case VarCheckDuplicate:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_DUP_FLAG), HiiHandle, L"time", Useful);
      break;

    case VarCheckUnknown:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_UNKNOWN_FLAG), HiiHandle, L"time", Useful);
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
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiHandle, L"time");
      Status = EFI_INVALID_PARAMETER;
    } else {
      if (IS_OLD_SHELL) {
        PrintToken (STRING_TOKEN (STR_NO_HELP), HiiHandle);
      } else {
        PrintToken (STRING_TOKEN (STR_TIME_VERBOSE_HELP), HiiHandle);
      }

      Status = EFI_SUCCESS;
    }

    goto Done;
  }

  if (ChkPck.ValueCount > 1) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiHandle, L"time");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  if (ChkPck.ValueCount == 0) {
    Status = RT->GetTime (&Time, NULL);

    if (!EFI_ERROR (Status)) {
      Status = RT->GetVariable (
                    L"TimeZone",
                    &gEfiGenericVariableGuid,
                    NULL,
                    &uValueSize,
                    &nValue
                    );
      if (!EFI_ERROR (Status) || EFI_NOT_FOUND == Status) {
        if (EFI_NOT_FOUND == Status) {
          nValue = 0;
        }

        if (nValue < 0) {
          nValue  = -nValue;
          wchSign = L'-';
        }

        PrintToken (
          STRING_TOKEN (STR_TIME_THREE_VARS),
          HiiHandle,
          Time.Hour,
          Time.Minute,
          Time.Second,
          wchSign,
          nValue / 100,
          nValue % 100
          );
        Status = EFI_SUCCESS;
        goto Done;
      }
    }

    PrintToken (STRING_TOKEN (STR_TIME_CLOCK_NOT_FUNC), HiiHandle, L"time");
    goto Done;
  }
  //
  // Get current time
  //
  Status = RT->GetTime (&Time, NULL);
  if (EFI_ERROR (Status)) {
    //
    // Error in GetTime, so set all fields of Timer to default value.
    //
    Time.Second = 0;
    Time.Minute = 0;
    Time.Hour   = 0;
    Time.Day    = 1;
    Time.Month  = 1;
    Time.Year   = 2001;
  }

  Offset = 0;
  //
  // Offset in argument string
  //
  // Get hour
  //
  if (!GetNumber (ChkPck.VarList->VarStr, &Offset, &Data, FALSE)) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_ARG), HiiHandle, L"time", ChkPck.VarList->VarStr);
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  if (Data < 0 || Data > 23) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_ARG), HiiHandle, L"time", ChkPck.VarList->VarStr);
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  Time.Hour = (UINT8) Data;

  //
  // Get minute
  //
  if (!GetNumber (ChkPck.VarList->VarStr, &Offset, &Data, FALSE)) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_ARG), HiiHandle, L"time", ChkPck.VarList->VarStr);
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  if (Data < 0 || Data > 59) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_ARG), HiiHandle, L"time", ChkPck.VarList->VarStr);
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  Time.Minute = (UINT8) Data;

  //
  // Get second
  //
  if (!GetNumber (ChkPck.VarList->VarStr, &Offset, &Data, TRUE)) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_ARG), HiiHandle, L"time", ChkPck.VarList->VarStr);
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  if (Data < 0 || Data > 59) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_ARG), HiiHandle, L"time", ChkPck.VarList->VarStr);
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  Time.Second = (UINT8) Data;

  Status      = RT->SetTime (&Time);
  if (EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_TIME_CLOCK_NOT_FUNC), HiiHandle, L"time");
  }

Done:
  LibCheckVarFreeVarList (&ChkPck);
  LibUnInitializeStrings ();
  return Status;
}

STATIC
BOOLEAN
GetNumber (
  IN      CHAR16  *Str,
  IN OUT  INTN    *Offset,
  IN OUT  INTN    *Number,
  IN    BOOLEAN   GetSecond
  )
{
  CHAR16  ch;
  INTN    Data;
  BOOLEAN FindNumber;

  Data        = 0;
  FindNumber  = FALSE;
  for (; Str[*Offset] != ':' && Str[*Offset] != 0; (*Offset)++) {
    ch = Str[*Offset];
    if (ch >= '0' && ch <= '9') {
      Data        = Data * 10 + (ch - '0');
      FindNumber  = TRUE;
    } else {
      return FALSE;
    }
  }

  if (!FindNumber) {
    if (!GetSecond) {
      //
      // The case "hh::"
      //
      return FALSE;
    }

    if (Str[*Offset - 1] == ':') {
      //
      // The case "hh:mm:"
      //
      return FALSE;
    }
  }

  if (Str[*Offset] == ':' && GetSecond) {
    //
    //  The case "hh:mm:ss:"
    //
    return FALSE;
  }

  if (Str[*Offset] == ':') {
    (*Offset)++;
  }

  *Number = Data;
  return TRUE;
}

EFI_STATUS
EFIAPI
InitializeTimeGetLineHelp (
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
  return LibCmdGetStringByToken (STRING_ARRAY_NAME, &EfiTimeGuid, STRING_TOKEN (STR_TIME_LINE_HELP), Str);
}
