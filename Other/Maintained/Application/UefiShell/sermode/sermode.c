/*++

Copyright (c) 2005 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  sermode.c
  
Abstract:

  Shell app "sermode"



Revision History

--*/

#include "EfiShellLib.h"
#include "sermode.h"

extern UINT8    STRING_ARRAY_NAME[];

//
// This is the generated header file which includes whatever needs to be exported (strings + IFR)
//
#include STRING_DEFINES_FILE

#include EFI_PROTOCOL_DEFINITION (SerialIo)

EFI_HII_HANDLE  HiiHandle;
EFI_GUID        EfiSermodeGuid = EFI_SERMODEB_GUID;
SHELL_VAR_CHECK_ITEM    SermodeCheckList[] = {
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
InitializeSerialMode (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  );

//
//
//
STATIC
EFI_STATUS
iDisplaySettings (
  IN UINTN                   HandleIdx,
  IN BOOLEAN                 HandleValid

  )
{
  EFI_SERIAL_IO_PROTOCOL  *SerialIo;
  UINTN                   NoHandles;
  EFI_HANDLE              *Handles;
  EFI_STATUS              Status;
  UINTN                   Index;
  CHAR16                  *StopBits;
  CHAR16                  Parity;

  Handles   = NULL;
  StopBits  = NULL;

  Status    = LibLocateHandle (ByProtocol, &gEfiSerialIoProtocolGuid, NULL, &NoHandles, &Handles);
  if (EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_SERMODE_NO_SERIAL_PORTS), HiiHandle);
    return EFI_INVALID_PARAMETER;
  }

  for (Index = 0; Index < NoHandles; Index++) {
    if (HandleValid) {
      if (ShellHandleFromIndex (HandleIdx) != Handles[Index]) {
        continue;

      }

    }

    Status = BS->HandleProtocol (Handles[Index], &gEfiSerialIoProtocolGuid, &SerialIo);
    if (!EFI_ERROR (Status)) {
      switch (SerialIo->Mode->Parity) {
      case DefaultParity:

        Parity = 'D';
        break;

      case NoParity:

        Parity = 'N';
        break;

      case EvenParity:

        Parity = 'E';
        break;

      case OddParity:

        Parity = 'O';
        break;

      case MarkParity:

        Parity = 'M';
        break;

      case SpaceParity:

        Parity = 'S';
        break;

      default:

        Parity = 'U';
      }

      switch (SerialIo->Mode->StopBits) {
      case DefaultStopBits:

        StopBits = L"Default";
        break;

      case OneStopBit:

        StopBits = L"1";
        break;

      case TwoStopBits:

        StopBits = L"2";
        break;

      case OneFiveStopBits:

        StopBits = L"1.5";
        break;

      default:

        StopBits = L"Unknown";
      }

      PrintToken (
        STRING_TOKEN (STR_SERMODE_DISPLAY),
        HiiHandle,
        ShellHandleToIndex (Handles[Index]),
        Handles[Index],
        SerialIo->Mode->BaudRate,
        Parity,
        SerialIo->Mode->DataBits,
        StopBits
        );
    } else {
      PrintToken (STRING_TOKEN (STR_SERMODE_NO_SERIAL_PORTS), HiiHandle);
      break;
    }

    if (HandleValid) {
      break;
    }
  }

  if (Index == NoHandles) {
    if ((NoHandles && HandleValid) || 0 == NoHandles) {
      PrintToken (STRING_TOKEN (STR_SERMODE_NOT_FOUND), HiiHandle);
    }
  }

  return Status;
}

EFI_BOOTSHELL_CODE(
  EFI_APPLICATION_ENTRY_POINT(InitializeSerialMode)
)

EFI_STATUS
InitializeSerialMode (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
/*++

Routine Description:

  Command entry point

Arguments:

  ImageHandle - The image handle
  SystemTable - The system table

Returns:

  EFI_SUCCESS - Success

--*/
{
  EFI_STATUS              Status;
  UINTN                   Index;
  UINTN                   NoHandles;
  EFI_HANDLE              *Handles;
  EFI_PARITY_TYPE         Parity;
  EFI_STOP_BITS_TYPE      StopBits;
  UINTN                   HandleIdx;
  UINTN                   BaudRate;
  UINTN                   DataBits;
  UINTN                   Value;
  EFI_SERIAL_IO_PROTOCOL  *SerialIo;
  CHAR16                  *Useful;
  SHELL_ARG_LIST          *Item;
  SHELL_VAR_CHECK_CODE    RetCode;
  SHELL_VAR_CHECK_PACKAGE ChkPck;

  //
  // Initialize app
  //
  EFI_SHELL_APP_INIT (ImageHandle, SystemTable);
  
  //
  // Enable tab key which can pause the output
  //
  EnableOutputTabPause();

  EFI_SHELL_STR_INIT (HiiHandle, STRING_ARRAY_NAME, EfiSermodeGuid);

  Handles = NULL;
  ZeroMem (&ChkPck, sizeof (SHELL_VAR_CHECK_PACKAGE));

  RetCode = LibCheckVariables (SI, SermodeCheckList, &ChkPck, &Useful);
  if (VarCheckOk != RetCode) {
    switch (RetCode) {
    case VarCheckDuplicate:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_DUP_FLAG), HiiHandle, L"sermode", Useful);
      break;

    case VarCheckUnknown:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_UNKNOWN_FLAG), HiiHandle, L"sermode", Useful);
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
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiHandle, L"sermode");
      Status = EFI_INVALID_PARAMETER;
    } else {
      PrintToken (STRING_TOKEN (STR_SERMODE_VERBOSE_HELP), HiiHandle);
      Status = EFI_SUCCESS;
    }

    goto Done;
  }

  if (ChkPck.ValueCount < 5 && ChkPck.ValueCount > 1) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_FEW), HiiHandle, L"sermode");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  if (ChkPck.ValueCount > 5) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiHandle, L"sermode");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  ShellInitHandleEnumerator ();
  Item = ChkPck.VarList;
  if (Item) {
    HandleIdx = (UINTN) StrToUIntegerBase (Item->VarStr, 16, &Status) - 1;
    if (EFI_ERROR (Status) || HandleIdx >= ShellGetHandleNum ()) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_ARG), HiiHandle, L"sermode", Item->VarStr);
      goto Done;
    }

    Item = GetNextArg (Item);
    if (NULL == Item) {
      Status = iDisplaySettings (HandleIdx, TRUE);
      goto Done;
    }
  } else {
    Status = iDisplaySettings (0, FALSE);
    goto Done;
  }

  BaudRate = (UINTN) StrToUInteger (Item->VarStr, &Status);
  if (EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_ARG), HiiHandle, L"sermode", Item->VarStr);
    goto Done;
  }

  Item = GetNextArg (Item);

  ASSERT (Item);
  if (StrLen (Item->VarStr) > 1) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_ARG), HiiHandle, L"sermode", Item->VarStr);
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  } else {
    switch ((CHAR8) Item->VarStr[0]) {
    case 'd':
    case 'D':

      Parity = DefaultParity;
      break;

    case 'n':
    case 'N':

      Parity = NoParity;
      break;

    case 'e':
    case 'E':

      Parity = EvenParity;
      break;

    case 'o':
    case 'O':

      Parity = OddParity;
      break;

    case 'm':
    case 'M':

      Parity = MarkParity;
      break;

    case 's':
    case 'S':

      Parity = SpaceParity;
      break;

    default:

      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_ARG), HiiHandle, L"sermode", Item->VarStr);
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }
  }

  Item = GetNextArg (Item);

  ASSERT (Item);
  DataBits = (UINTN) StrToUInteger (Item->VarStr, &Status);
  if (EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_ARG), HiiHandle, L"sermode", Item->VarStr);
    goto Done;
  } else {
    switch (DataBits) {
    case 4:
    case 7:
    case 8:

      break;

    default:

      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_ARG), HiiHandle, L"sermode", Item->VarStr);
      goto Done;
    }
  }

  Item = GetNextArg (Item);

  ASSERT (Item);
  Value = (UINTN) StrToUInteger (Item->VarStr, &Status);
  if (EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_ARG), HiiHandle, L"sermode", Item->VarStr);
    goto Done;
  } else {
    switch (Value) {
    case 0:
      StopBits = DefaultStopBits;
      break;

    case 1:
      StopBits = OneStopBit;
      break;

    case 2:
      StopBits = TwoStopBits;
      break;

    case 15:
      StopBits = OneFiveStopBits;
      break;

    default:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_ARG), HiiHandle, Item->VarStr);
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }
  }
  //
  //
  //
  Status = LibLocateHandle (ByProtocol, &gEfiSerialIoProtocolGuid, NULL, &NoHandles, &Handles);
  if (EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_SERMODE_NO_SERIAL_PORTS), HiiHandle);
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }
  //
  //
  //
  for (Index = 0; Index < NoHandles; Index++) {
    if (ShellHandleFromIndex (HandleIdx) != Handles[Index]) {
      continue;
    }

    Status = BS->HandleProtocol (Handles[Index], &gEfiSerialIoProtocolGuid, &SerialIo);
    if (!EFI_ERROR (Status)) {
      Status = SerialIo->SetAttributes (
                          SerialIo,
                          (UINT64) BaudRate,
                          SerialIo->Mode->ReceiveFifoDepth,
                          SerialIo->Mode->Timeout,
                          Parity,
                          (UINT8) DataBits,
                          StopBits
                          );
      if (EFI_ERROR (Status)) {
        PrintToken (STRING_TOKEN (STR_SERMODE_CANNOT_SET_ATTR), HiiHandle, Status, Handles[Index]);
      } else {
        PrintToken (STRING_TOKEN (STR_SERMODE_MODE_SET_ON_HANDLE), HiiHandle, Handles[Index]);
      }
      break;
    }
  }

  if (Index == NoHandles) {
    PrintToken (STRING_TOKEN (STR_SERMODE__NOT_SER_HANDLE), HiiHandle, HandleIdx + 1);
    Status = EFI_INVALID_PARAMETER;
  }

Done:
  if (Handles) {
    FreePool (Handles);
  }

  LibCheckVarFreeVarList (&ChkPck);
  ShellCloseHandleEnumerator ();
  LibUnInitializeStrings ();
  return Status;
}

EFI_STATUS
InitializeSerialModeGetLineHelp (
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
  return LibCmdGetStringByToken (STRING_ARRAY_NAME, &EfiSermodeGuid, STRING_TOKEN (STR_SERMODE_LINE_HELP), Str);
}
