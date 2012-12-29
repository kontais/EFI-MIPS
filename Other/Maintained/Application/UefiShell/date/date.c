/*++

Copyright (c) 2005 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  date.c

Abstract: 

  shell command "date"

Revision History

--*/

#include "EfiShellLib.h"
#include "date.h"

extern UINT8  STRING_ARRAY_NAME[];

//
// This is the generated header file which includes whatever needs to be exported (strings + IFR)
//
#include STRING_DEFINES_FILE

//
// Global Variables
//
EFI_GUID  EfiDateGuid = EFI_DATE_GUID;

//
//
//
EFI_STATUS
EFIAPI
InitializeDate (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

//
//
//
STATIC
BOOLEAN
GetNumber (
  IN      CHAR16  *Str,
  IN OUT  INTN    *Position,
  IN OUT  INTN    *Number,
  IN BOOLEAN      EndNum
  );

STATIC
BOOLEAN
ValidDay (
  IN  EFI_TIME  time
  );

STATIC
BOOLEAN
IsLeapYear (
  IN EFI_TIME   time
  );

EFI_BOOTSHELL_CODE(
  EFI_APPLICATION_ENTRY_POINT(InitializeDate)
)

INTN  DayOfMonth[12] = { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
SHELL_VAR_CHECK_ITEM    DateCheckList[] = {
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

//
//
//
EFI_STATUS
EFIAPI
InitializeDate (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
/*++

Routine Description:
  Display or set date
  
Arguments:
  ImageHandle     The image handle.
  SystemTable     The system table.

Returns:
  EFI_SUCCESS             - Command completed successfully
  EFI_INVALID_PARAMETER   - Command usage error
  Other value             - Unknown error
  
Notes:  
  date [mm/dd/yyyy]

--*/
{
  EFI_STATUS              Status;
  EFI_TIME                Time;
  UINTN                   Offset;
  UINTN                   Data;
  EFI_HII_HANDLE          HiiHandle;
  SHELL_VAR_CHECK_PACKAGE ChkPck;
  CHAR16                  *Useful;
  SHELL_VAR_CHECK_CODE    RetCode;

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
  Status = LibInitializeStrings (&HiiHandle, STRING_ARRAY_NAME, &EfiDateGuid);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (!EFI_PROPER_VERSION (0, 99)) {
    PrintToken (
      STRING_TOKEN (STR_SHELLENV_GNC_COMMAND_NOT_SUPPORT),
      HiiHandle,
      L"date",
      EFI_VERSION_0_99 
      );
    Status = EFI_UNSUPPORTED;
    goto Done;
  }

  RetCode = LibCheckVariables (SI, DateCheckList, &ChkPck, &Useful);
  if (VarCheckOk != RetCode) {
    switch (RetCode) {
    case VarCheckUnknown:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_UNKNOWN_FLAG), HiiHandle, L"date", Useful);
      break;

    case VarCheckDuplicate:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_DUP_FLAG), HiiHandle, L"date", Useful);
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
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiHandle, L"date");
      Status = EFI_INVALID_PARAMETER;
    } else {
      PrintToken (STRING_TOKEN (STR_DATE_VERBOSE_HELP), HiiHandle);
      Status = EFI_SUCCESS;
    }

    goto Done;
  }

  if (ChkPck.ValueCount == 0) {
    Status = RT->GetTime (&Time, NULL);
    if (!EFI_ERROR (Status)) {
      PrintToken (STRING_TOKEN (STR_DATE_PRINTDATE), HiiHandle, Time.Month, Time.Day, Time.Year);
    } else {
      PrintToken (STRING_TOKEN (STR_DATE_CLOCK_NOT_FUNC), HiiHandle);
    }

    goto Done;
  }

  if (ChkPck.ValueCount > 1) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiHandle, L"date");
    Status = EFI_INVALID_PARAMETER;
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
  //
  // Init start number of argument and
  // offset position in argument string
  //
  Offset = 0;

  //
  // Get month
  //
  if (!GetNumber (ChkPck.VarList->VarStr, &Offset, &Data, FALSE)) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_ARG), HiiHandle, L"date", ChkPck.VarList->VarStr);
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  if (Data < 1 || Data > 12) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_ARG), HiiHandle, L"date", ChkPck.VarList->VarStr);
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  Time.Month = (UINT8) Data;

  //
  // Get day
  //
  if (!GetNumber (ChkPck.VarList->VarStr, &Offset, &Data, FALSE)) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_ARG), HiiHandle, L"date", ChkPck.VarList->VarStr);
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  if (Data < 1 || Data > 31) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_ARG), HiiHandle, L"date", ChkPck.VarList->VarStr);
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  Time.Day = (UINT8) Data;

  //
  // Get year.
  //
  if (!GetNumber (ChkPck.VarList->VarStr, &Offset, &Data, TRUE)) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_ARG), HiiHandle, L"date", ChkPck.VarList->VarStr);
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }
  //
  // Minimal year number supported is 1998
  //
  if (Data < 100) {
    Data = Data + 1900;
    if (Data < 1998) {
      Data = Data + 100;
    }
  }

  if (Data < 1998 || Data > 2099) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_ARG), HiiHandle, L"date", ChkPck.VarList->VarStr);
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  Time.Year = (UINT16) Data;

  if (!ValidDay (Time)) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_ARG), HiiHandle, L"date", ChkPck.VarList->VarStr);
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  Status = RT->SetTime (&Time);
  if (EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_DATE_CLOCK_NOT_FUNC), HiiHandle);
  }

Done:
  LibCheckVarFreeVarList (&ChkPck);
  LibUnInitializeStrings ();
  return Status;
}
//
// Get number from arguments
//
STATIC
BOOLEAN
GetNumber (
  IN      CHAR16    *Str,
  IN OUT  INTN      *Position,
  IN OUT  INTN      *Number,
  IN BOOLEAN        EndNum
  )
{
  CHAR16  Char;
  INTN    Data;
  BOOLEAN FindNum;

  Data    = 0;
  FindNum = FALSE;

  for (; Str[*Position] != '/' && Str[*Position] != 0; (*Position)++) {
    Char = Str[*Position];
    if (Char >= '0' && Char <= '9') {
      FindNum = TRUE;
      Data    = Data * 10 + (Char - '0');
      //
      // to aVOID data over-flow
      //
      if (Data > 10000) {
        return FALSE;
      }
    } else {
      //
      // Any other characters regards as invalid
      //
      return FALSE;
    }
  }

  if (!FindNum) {
    return FALSE;
  }

  if ((Str[*Position] == '/' && EndNum) || (Str[*Position] == 0 && !EndNum)) {
    return FALSE;
  }

  if (Str[*Position] == '/') {
    (*Position)++;
  }

  *Number = Data;
  return TRUE;
}

STATIC
BOOLEAN
ValidDay (
  IN  EFI_TIME  time
  )
{
  if (time.Day > DayOfMonth[time.Month - 1]) {
    return FALSE;
  }
  //
  // Pay attention to month==2
  //
  if (time.Month == 2 && ((IsLeapYear (time) && time.Day > 29) || (!IsLeapYear (time) && time.Day > 28))) {
    return FALSE;
  }

  return TRUE;
}

STATIC
BOOLEAN
IsLeapYear (
  IN EFI_TIME   time
  )
{
  if (time.Year % 4 == 0) {
    if (time.Year % 100 == 0) {
      if (time.Year % 400 == 0) {
        return TRUE;
      } else {
        return FALSE;
      }
    } else {
      return TRUE;
    }
  } else {
    return FALSE;
  }
}

EFI_STATUS
EFIAPI
InitializeDateGetLineHelp (
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
  return LibCmdGetStringByToken (STRING_ARRAY_NAME, &EfiDateGuid, STRING_TOKEN (STR_DATE_LINE_HELP), Str);
}
