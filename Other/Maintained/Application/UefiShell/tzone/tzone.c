/*++

Copyright (c) 2005 - 2007, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  tzone.c

Abstract: 

  EFI shell command "timezone"

Revision History

--*/

#include "tzone.h"

extern UINT8    STRING_ARRAY_NAME[];

#include STRING_DEFINES_FILE

EFI_HII_HANDLE  hHiiHandle;
EFI_GUID        EfiTzoneGuid = EFI_TZONE_GUID;

SHELL_VAR_CHECK_ITEM    TzoneCheckList[] = {
  {
    L"-s",
    0x01,
    0x1a,
    FlagTypeNeedVar
  },
  {
    L"-l",
    0x02,
    0x11,
    FlagTypeSingle
  },
  {
    L"-b",
    0x04,
    0,
    FlagTypeSingle
  },
  {
    L"-f",
    0x08,
    0x11,
    FlagTypeSingle
  },
  {
    L"-?",
    0x10,
    0x0f,
    FlagTypeSingle
  },
  {
    NULL,
    0,
    0,
    0
  }
};

struct tagMap {
  INTN  m_nValue;
  UINTN m_uToken;
};

EFI_STATUS
InitializeTZone (
  IN EFI_HANDLE        hImageHandle,
  IN EFI_SYSTEM_TABLE  *pSystemTable
  );

BOOLEAN
TZoneArgtoValue (
  IN CHAR16              *wszValue,
  OUT INTN               *pnValue
  );

EFI_STATUS
TZoneSetValue (
  IN INTN                  nValue
  );

EFI_STATUS
TZoneList (
  IN EFI_HII_HANDLE                   *phHiiHandle
  );

EFI_BOOTSHELL_CODE(
  EFI_APPLICATION_ENTRY_POINT(InitializeTZone)
)

struct tagMap gTZMap[] = {
  {
    -1200,
    STRING_TOKEN(STR_TZONE_GMT_N1200)
  },
  {
    -1100,
    STRING_TOKEN(STR_TZONE_GMT_N1100)
  },
  {
    -1000,
    STRING_TOKEN(STR_TZONE_GMT_N1000)
  },
  {
    -900,
    STRING_TOKEN(STR_TZONE_GMT_N0900)
  },
  {
    -800,
    STRING_TOKEN(STR_TZONE_GMT_N0800)
  },
  {
    -700,
    STRING_TOKEN(STR_TZONE_GMT_N0700)
  },
  {
    -600,
    STRING_TOKEN(STR_TZONE_GMT_N0600)
  },
  {
    -500,
    STRING_TOKEN(STR_TZONE_GMT_N0500)
  },
  {
    -400,
    STRING_TOKEN(STR_TZONE_GMT_N0400)
  },
  {
    -330,
    STRING_TOKEN(STR_TZONE_GMT_N0330)
  },
  {
    -300,
    STRING_TOKEN(STR_TZONE_GMT_N0300)
  },
  {
    -200,
    STRING_TOKEN(STR_TZONE_GMT_N0200)
  },
  {
    -100,
    STRING_TOKEN(STR_TZONE_GMT_N0100)
  },
  {
    0,
    STRING_TOKEN(STR_TZONE_GMT_0000)
  },
  {
    100,
    STRING_TOKEN(STR_TZONE_GMT_P0100)
  },
  {
    200,
    STRING_TOKEN(STR_TZONE_GMT_P0200)
  },
  {
    300,
    STRING_TOKEN(STR_TZONE_GMT_P0300)
  },
  {
    330,
    STRING_TOKEN(STR_TZONE_GMT_P0330)
  },
  {
    400,
    STRING_TOKEN(STR_TZONE_GMT_P0400)
  },
  {
    430,
    STRING_TOKEN(STR_TZONE_GMT_P0430)
  },
  {
    500,
    STRING_TOKEN(STR_TZONE_GMT_P0500)
  },
  {
    530,
    STRING_TOKEN(STR_TZONE_GMT_P0530)
  },
  {
    545,
    STRING_TOKEN(STR_TZONE_GMT_P0545)
  },
  {
    600,
    STRING_TOKEN(STR_TZONE_GMT_P0600)
  },
  {
    630,
    STRING_TOKEN(STR_TZONE_GMT_P0630)
  },
  {
    700,
    STRING_TOKEN(STR_TZONE_GMT_P0700)
  },
  {
    800,
    STRING_TOKEN(STR_TZONE_GMT_P0800)
  },
  {
    900,
    STRING_TOKEN(STR_TZONE_GMT_P0900)
  },
  {
    930,
    STRING_TOKEN(STR_TZONE_GMT_P0930)
  },
  {
    1000,
    STRING_TOKEN(STR_TZONE_GMT_P1000)
  },
  {
    1100,
    STRING_TOKEN(STR_TZONE_GMT_P1100)
  },
  {
    1200,
    STRING_TOKEN(STR_TZONE_GMT_P1200)
  },
  {
    1300,
    STRING_TOKEN(STR_TZONE_GMT_P1300)
  }
};

EFI_STATUS
InitializeTZone (
  IN EFI_HANDLE               hImageHandle,
  IN EFI_SYSTEM_TABLE         *pSystemTable
  )
/*++
   Routine Description:
      sets or displays time zone information

   Arguments:
      hImageHandle    -     The image handle
      pSystemTable    -     The system table pointer

   Returns:
   
 --*/
{
  EFI_STATUS              Status;
  UINTN                   uIndex;
  BOOLEAN                 bFullInfo;
  BOOLEAN                 bList;
  BOOLEAN                 bSet;
  INTN                    nValue;
  UINTN                   uValueSize;
  SHELL_VAR_CHECK_CODE    RetCode;
  CHAR16                  *Useful;
  SHELL_VAR_CHECK_PACKAGE ChkPck;
  SHELL_ARG_LIST          *Item;

  bFullInfo     = FALSE;
  bList         = FALSE;
  bSet          = FALSE;
  nValue        = 0;
  uValueSize    = sizeof (INTN);
  ZeroMem (&ChkPck, sizeof (SHELL_VAR_CHECK_PACKAGE));
  //
  // We are now being installed as an internal command driver, initialize
  // as an nshell app and run
  //
  EFI_SHELL_APP_INIT (hImageHandle, pSystemTable);
  
  //
  // Enable tab key which can pause the output
  //
  EnableOutputTabPause();

  Status = LibInitializeStrings (&hHiiHandle, STRING_ARRAY_NAME, &EfiTzoneGuid);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (!EFI_PROPER_VERSION (0, 99)) {
    PrintToken (
      STRING_TOKEN (STR_SHELLENV_GNC_COMMAND_NOT_SUPPORT),
      hHiiHandle,
      L"tzone",
      EFI_VERSION_0_99 
      );
    Status = EFI_UNSUPPORTED;
    goto Done;
  }

  RetCode = LibCheckVariables (SI, TzoneCheckList, &ChkPck, &Useful);
  if (VarCheckOk != RetCode) {
    switch (RetCode) {
    case VarCheckUnknown:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_UNKNOWN_FLAG), hHiiHandle, L"timezone", Useful);
      break;

    case VarCheckDuplicate:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_DUP_FLAG), hHiiHandle, L"timezone", Useful);
      break;

    case VarCheckConflict:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_FLAG_CONFLICT), hHiiHandle, L"timezone", Useful);
      break;

    case VarCheckLackValue:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_LACK_ARG), hHiiHandle, L"timezone", Useful);
      break;

    default:
      break;
    }

    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  if (ChkPck.ValueCount > 0) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), hHiiHandle, L"timezone");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  if (LibCheckVarGetFlag (&ChkPck, L"-b") != NULL) {
    EnablePageBreak (DEFAULT_INIT_ROW, DEFAULT_AUTO_LF);
  }

  if (LibCheckVarGetFlag (&ChkPck, L"-?") != NULL) {
    if (ChkPck.ValueCount > 0 ||
        ChkPck.FlagCount > 2 ||
        (2 == ChkPck.FlagCount && !LibCheckVarGetFlag (&ChkPck, L"-b"))
        ) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), hHiiHandle, L"timezone");
      Status = EFI_INVALID_PARAMETER;
    } else {
      PrintToken (STRING_TOKEN (STR_HELPINFO_TZONE_VERBOSEHELP), hHiiHandle);
      Status = EFI_SUCCESS;
    }

    goto Done;
  }

  if (LibCheckVarGetFlag (&ChkPck, L"-f") != NULL) {
    bFullInfo = TRUE;
  }

  if (LibCheckVarGetFlag (&ChkPck, L"-l") != NULL) {
    bList = TRUE;
  } else {
    Item = LibCheckVarGetFlag (&ChkPck, L"-s");
    if (NULL != Item) {
      if (FALSE == TZoneArgtoValue (Item->VarStr, &nValue)) {
        PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_BAD_VALUE_4_FLAG), hHiiHandle, L"timezone", Item->VarStr, L"-s");
        Status = EFI_INVALID_PARAMETER;
        goto Done;
      }

      bSet = TRUE;
    }
  }


  if (bSet) {
    for (uIndex = 0; uIndex < sizeof (gTZMap) / sizeof (struct tagMap); uIndex++) {
      if (gTZMap[uIndex].m_nValue == nValue) {
        Status = TZoneSetValue (nValue);
        goto Done;
      }
    }

    if (nValue >= 0) {
      PrintToken (STRING_TOKEN (STR_TZONE_INVALID_VALUE), hHiiHandle, L"timezone", L"+", nValue / 100, nValue % 100);
    } else {
      nValue = -nValue;
      PrintToken (STRING_TOKEN (STR_TZONE_INVALID_VALUE), hHiiHandle, L"timezone", L"-", nValue / 100, nValue % 100);
    }

    Status = EFI_INVALID_PARAMETER;
    goto Done;
  } else if (bList) {
    Status = TZoneList (&hHiiHandle);
    goto Done;
  } else {
    Status = RT->GetVariable (
                  L"TimeZone",
                  &gEfiGenericVariableGuid,
                  NULL,
                  &uValueSize,
                  &nValue
                  );
    if (EFI_ERROR (Status)) {
      if (EFI_NOT_FOUND == Status) {
        if (bFullInfo) {
          PrintToken (STRING_TOKEN (STR_TZONE_GMT_0000), hHiiHandle);
        } else {
          PrintToken (STRING_TOKEN (STR_TZONE_GMT_FMT), hHiiHandle, L"+", 0, 0);
        }

        Status = EFI_SUCCESS;
      }

      goto Done;
    }
    //
    // end of if (EFI_ERROR(Status))
    //
    if (bFullInfo) {
      for (uIndex = 0; uIndex < sizeof (gTZMap) / sizeof (struct tagMap); uIndex++) {
        if (gTZMap[uIndex].m_nValue == nValue) {
          PrintToken ((UINT16) gTZMap[uIndex].m_uToken, hHiiHandle);
          break;
        }
      }
    } else {
      if (nValue >= 0) {
        PrintToken (STRING_TOKEN (STR_TZONE_GMT_FMT), hHiiHandle, L"+", nValue / 100, nValue % 100);
      } else {
        nValue = -nValue;
        PrintToken (STRING_TOKEN (STR_TZONE_GMT_FMT), hHiiHandle, L"-", nValue / 100, nValue % 100);
      }
    }
  }

Done:
  LibCheckVarFreeVarList (&ChkPck);
  LibUnInitializeStrings ();
  return Status;
}

BOOLEAN
TZoneArgtoValue (
  IN CHAR16              *wszValue,
  OUT INTN               *pnValue
  )
{
  UINTN   uPos;
  CHAR16  *pwchBreak;
  BOOLEAN bDigitFound;
  BOOLEAN bSignGot;
  INTN    nSign;
  UINTN   uDigitStart;

  uPos        = 0;
  pwchBreak   = NULL;
  bDigitFound = FALSE;
  bSignGot    = FALSE;
  nSign       = 1;
  uDigitStart = 0;

  while (wszValue[uPos]) {
    if (':' != wszValue[uPos] && '-' != wszValue[uPos] && '+' != wszValue[uPos]) {
      if (wszValue[uPos] < '0' || wszValue[uPos] > '9') {
        return FALSE;
      }

      bDigitFound = TRUE;
      //
      // if encounter digit before '+' or '-', take the number as postive
      //
      if (FALSE == bSignGot) {
        bSignGot    = TRUE;
        uDigitStart = uPos;
      }
    } else if (':' == wszValue[uPos]) {
      //
      // if no digit encountered before ':', error value string
      //
      if (FALSE == bDigitFound) {
        return FALSE;
      }
      //
      // at the most, only two digits allowed before ':'
      //
      if (uPos - uDigitStart > 2) {
        return FALSE;
      }

      wszValue[uPos]  = 0;
      *pnValue        = Atoi (wszValue + uDigitStart);
      wszValue[uPos]  = L':';

      if (NULL != pwchBreak) {
        return FALSE;
        //
        // too many ':'
        //
      }

      pwchBreak   = &wszValue[uPos + 1];
      bDigitFound = FALSE;
    } else if ('+' == wszValue[uPos] || '-' == wszValue[uPos]) {
      //
      // sign char appears second time, error
      //
      if (bSignGot) {
        return FALSE;
      }

      bSignGot = TRUE;
      //
      // we found the sign char
      //
      nSign       = ('+' == (wszValue[uPos]) ? 1 : -1);
      uDigitStart = uPos + 1;
      //
      // use uDigitStart to point starting digit
      //
    }

    uPos++;
  }
  //
  // end of while
  //
  if (FALSE == bDigitFound || NULL == pwchBreak) {
    return FALSE;
  }
  //
  // at the most, only 2 digits after ':'
  //
  if ((UINTN) (wszValue + uPos - pwchBreak) > 2) {
    return FALSE;
  }

  *pnValue *= 100;
  *pnValue  = *pnValue + Atoi (pwchBreak);
  *pnValue  = *pnValue * nSign;

  return TRUE;
}

EFI_STATUS
TZoneSetValue (
  IN INTN   nValue
  )
{
  return RT->SetVariable (
              L"TimeZone",
              &gEfiGenericVariableGuid,
              EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
              sizeof (INTN),
              &nValue
              );
}

EFI_STATUS
TZoneList (
  IN EFI_HII_HANDLE   *phHiiHandle
  )
{
  UINTN uIndex;

  for (uIndex = 0; uIndex < sizeof (gTZMap) / sizeof (struct tagMap); uIndex++) {
    PrintToken ((UINT16) gTZMap[uIndex].m_uToken, *phHiiHandle);
  }

  return EFI_SUCCESS;
}

EFI_STATUS
InitializeZoneGetLineHelp (
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
  return LibCmdGetStringByToken (STRING_ARRAY_NAME, &EfiTzoneGuid, STRING_TOKEN (STR_HELPINFO_TZONE_LINEHELP), Str);
}
