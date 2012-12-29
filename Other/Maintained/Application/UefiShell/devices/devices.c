/*++
 
Copyright (c) 2005 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  devices.c
  
Abstract:

  Shell command "devices"



Revision History

--*/

#include "EfiShellLib.h"
#include "devices.h"

extern UINT8  STRING_ARRAY_NAME[];

//
// This is the generated header file which includes whatever needs to be exported (strings + IFR)
//
#include STRING_DEFINES_FILE

//
// Global Variables
//
EFI_HII_HANDLE  HiiHandle;
EFI_GUID        EfiDevicesGuid = EFI_DEVICES_GUID;
SHELL_VAR_CHECK_ITEM    DevicesCheckList[] = {
  {
    L"-l",
    0x01,
    0,
    FlagTypeNeedVar
  },
  {
    L"-b",
    0x02,
    0,
    FlagTypeSingle
  },
  {
    L"-?",
    0x04,
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
DevicesMain (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  );

EFI_BOOTSHELL_CODE(
  EFI_APPLICATION_ENTRY_POINT(DevicesMain)
)
//
//
//
EFI_STATUS
DevicesMainOld (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  );

//
//
//
EFI_STATUS
DevicesMain (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  )
/*++

Routine Description:

  Code for external shell "devices" command.

Arguments:

  ImageHandle - Image handle
  SystemTable - System table

Returns:

  EFI_INVALID_PARAMETER - Invalid parameter
  EFI_SUCCESS - Success

--*/
{
  EFI_STATUS              Status;
  EFI_STATUS              ConfigurationStatus;
  EFI_STATUS              DiagnosticsStatus;
  UINTN                   StringIndex;
  UINTN                   Index;
  CHAR8                   *Language;
  UINTN                   DeviceHandleCount;
  EFI_HANDLE              *DeviceHandleBuffer;
  UINTN                   DriverBindingHandleCount;
  EFI_HANDLE              *DriverBindingHandleBuffer;
  UINTN                   ParentControllerHandleCount;
  EFI_HANDLE              *ParentControllerHandleBuffer;
  UINTN                   ChildControllerHandleCount;
  EFI_HANDLE              *ChildControllerHandleBuffer;
  CHAR16                  *BestDeviceName;

  SHELL_VAR_CHECK_CODE    RetCode;
  CHAR16                  *Useful;
  SHELL_ARG_LIST          *Item;
  SHELL_VAR_CHECK_PACKAGE ChkPck;

  Language = NULL;
  ZeroMem (&ChkPck, sizeof (SHELL_VAR_CHECK_PACKAGE));
  EFI_SHELL_APP_INIT (ImageHandle, SystemTable);
  
  //
  // Enable tab key which can pause the output
  //
  EnableOutputTabPause();

  Status = LibInitializeStrings (&HiiHandle, STRING_ARRAY_NAME, &EfiDevicesGuid);

  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (!EFI_PROPER_VERSION (1, 10)) {
    PrintToken (
      STRING_TOKEN (STR_SHELLENV_GNC_COMMAND_NOT_SUPPORT),
      HiiHandle,
      L"devices",
      EFI_VERSION_1_10
      );
    Status = EFI_UNSUPPORTED;
    goto Done;
  }

  //
  // Setup Handle and Protocol Globals
  //
  ShellInitProtocolInfoEnumerator ();
  Language = LibGetVariable (VarLanguage, &gEfiGlobalVariableGuid);

  if (Language == NULL) {
    Language    = AllocatePool (4);
    Language[0] = 'e';
    Language[1] = 'n';
    Language[2] = 'g';
    Language[3] = 0;
  }

  if (IS_OLD_SHELL) {
    Status = DevicesMainOld (ImageHandle, SystemTable);
    goto Done;
  }

  RetCode = LibCheckVariables (SI, DevicesCheckList, &ChkPck, &Useful);

  if (VarCheckOk != RetCode) {
    switch (RetCode) {
    case VarCheckDuplicate:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_DUP_FLAG), HiiHandle, L"devices", Useful);
      break;

    case VarCheckUnknown:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_UNKNOWN_FLAG), HiiHandle, L"devices", Useful);
      break;

    case VarCheckLackValue:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_LACK_ARG), HiiHandle, L"devices", Useful);
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
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiHandle, L"devices");
      Status = EFI_INVALID_PARAMETER;
    } else {
      PrintToken (STRING_TOKEN (STR_HELPINFO_DEVICES_VERBOSEHELP), HiiHandle);
      Status = EFI_SUCCESS;
    }

    goto Done;
  }

  Item = LibCheckVarGetFlag (&ChkPck, L"-l");
  if (Item != NULL) {
    if (StrLen (Item->VarStr) != 3) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_DEVICES_BAD_LANG), HiiHandle, L"devices", Item->VarStr);
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }

    for (StringIndex = 0; StringIndex < 3; StringIndex++) {
      Language[StringIndex] = (CHAR8) Item->VarStr[StringIndex];
    }

    Language[StringIndex] = 0;
  }
  //
  // Display all device handles
  //
  Status = LibLocateHandle (
            AllHandles,
            NULL,
            NULL,
            &DeviceHandleCount,
            &DeviceHandleBuffer
            );
  if (EFI_ERROR (Status)) {
    Status = EFI_NOT_FOUND;
    goto Done;
  }

  PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_HC_T_DN), HiiHandle);
  PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_HT_Y_C_IN), HiiHandle);
  PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_HR_P_F_AN), HiiHandle);
  PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_HL_ECGPDC_N), HiiHandle);
  PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_H_EQUAL_SIGNS), HiiHandle);

  for (Index = 0; Index < DeviceHandleCount; Index++) {

    BREAK_LOOP_ON_ESC ();

    Status = ShellGetDeviceName (
              DeviceHandleBuffer[Index],
              TRUE,
              TRUE,
              Language,
              &BestDeviceName,
              &ConfigurationStatus,
              &DiagnosticsStatus,
              FALSE,
              0
              );

    //
    // Retrieve the best name for DeviceHandleBuffer[Index].  Also determine if
    // the device support being configured, or having diagnostics run on it.
    // Also determine the number of parents controllers, the number of managing
    // drivers, and the number of children.
    //
    BREAK_LOOP_ON_ESC ();

    Status = LibGetManagingDriverBindingHandles (
              DeviceHandleBuffer[Index],
              &DriverBindingHandleCount,
              &DriverBindingHandleBuffer
              );

    BREAK_LOOP_ON_ESC ();

    Status = LibGetParentControllerHandles (
              DeviceHandleBuffer[Index],
              &ParentControllerHandleCount,
              &ParentControllerHandleBuffer
              );

    BREAK_LOOP_ON_ESC ();

    Status = LibGetChildControllerHandles (
              DeviceHandleBuffer[Index],
              &ChildControllerHandleCount,
              &ChildControllerHandleBuffer
              );

    BREAK_LOOP_ON_ESC ();

    if (DriverBindingHandleCount > 0 || ParentControllerHandleCount > 0 || ChildControllerHandleCount > 0) {
      PrintToken (
        STRING_TOKEN (STR_SHELLENV_PROTID_ONE_VAR_H02),
        HiiHandle,
        ShellHandleToIndex (DeviceHandleBuffer[Index])
        );

      if (ParentControllerHandleCount == 0) {
        Print (L" R");
      } else if (ChildControllerHandleCount > 0) {
        Print (L" B");
      } else {
        Print (L" D");
      }

      if (!EFI_ERROR (ConfigurationStatus)) {
        Print (L" X");
      } else {
        Print (L" -");
      }

      if (!EFI_ERROR (DiagnosticsStatus)) {
        Print (L" X");
      } else {
        Print (L" -");
      }

      if (ParentControllerHandleCount == 0) {
        Print (L"  -");
      } else {
        PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_ONE_VAR_D), HiiHandle, ParentControllerHandleCount);
      }

      if (DriverBindingHandleCount == 0) {
        Print (L"  -");
      } else {
        PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_ONE_VAR_D), HiiHandle, DriverBindingHandleCount);
      }

      if (ChildControllerHandleCount == 0) {
        Print (L"  -");
      } else {
        PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_ONE_VAR_D), HiiHandle, ChildControllerHandleCount);
      }

      if (BestDeviceName == NULL) {
        PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_UNKNOWN_2), HiiHandle);
      } else {
        PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_ONE_VAR_S_SPACE), HiiHandle, BestDeviceName);
      }
    }

    if (DriverBindingHandleBuffer) {
      FreePool (DriverBindingHandleBuffer);
    }

    if (ParentControllerHandleBuffer) {
      FreePool (ParentControllerHandleBuffer);
    }

    if (ChildControllerHandleBuffer) {
      FreePool (ChildControllerHandleBuffer);
    }

    if (BestDeviceName) {
      FreePool (BestDeviceName);
    }
  }

  FreePool (DeviceHandleBuffer);

Done:
  if (Language != NULL) {
    FreePool (Language);
  }
  LibCheckVarFreeVarList (&ChkPck);
  ShellCloseProtocolInfoEnumerator ();
  LibUnInitializeStrings ();
  return Status;
}

EFI_STATUS
InitializeDevicesGetLineHelp (
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
  return LibCmdGetStringByToken (
          STRING_ARRAY_NAME,
          &EfiDevicesGuid,
          STRING_TOKEN (STR_HELPINFO_DEVICES_LINEHELP),
          Str
          );
}

EFI_STATUS
DevicesMainOld (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  )
/*++

Routine Description:

Arguments:

  ImageHandle - Image handle
  SystemTable - System table

Returns:

--*/
{
  EFI_STATUS  Status;
  EFI_STATUS  ConfigurationStatus;
  EFI_STATUS  DiagnosticsStatus;
  CHAR16      *Ptr;
  UINTN       StringIndex;
  UINTN       Index;
  CHAR8       *Language;
  UINTN       DeviceHandleCount;
  EFI_HANDLE  *DeviceHandleBuffer;
  UINTN       DriverBindingHandleCount;
  EFI_HANDLE  *DriverBindingHandleBuffer;
  UINTN       ParentControllerHandleCount;
  EFI_HANDLE  *ParentControllerHandleBuffer;
  UINTN       ChildControllerHandleCount;
  EFI_HANDLE  *ChildControllerHandleBuffer;
  CHAR16      *BestDeviceName;
  BOOLEAN     PrtHelp;

  PrtHelp   = FALSE;
  Language  = LibGetVariable (VarLanguage, &gEfiGlobalVariableGuid);
  if (Language == NULL) {
    Language    = AllocatePool (4);
    Language[0] = 'e';
    Language[1] = 'n';
    Language[2] = 'g';
    Language[3] = 0;
  }

  for (Index = 1; Index < SI->Argc; Index += 1) {
    Ptr = SI->Argv[Index];
    if (*Ptr == '-') {
      switch (Ptr[1]) {
      case 'l':
      case 'L':
        if (*(Ptr + 2) != 0) {
          for (StringIndex = 0; StringIndex < 3 && Ptr[StringIndex + 2] != 0; StringIndex++) {
            Language[StringIndex] = (CHAR8) Ptr[StringIndex + 2];
          }

          Language[StringIndex] = 0;
        }
        break;

      case 'b':
      case 'B':
        EnablePageBreak (DEFAULT_INIT_ROW, DEFAULT_AUTO_LF);
        break;

      case '?':
        PrtHelp = TRUE;
        break;

      default:
        PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_UNKNOWN_FLAG), HiiHandle, L"device", Ptr);
        Status = EFI_INVALID_PARAMETER;
        goto Done;
      }
    } else {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_ARG), HiiHandle, L"devices", Ptr);
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }
  }

  if (PrtHelp) {
    PrintToken (STRING_TOKEN (STR_HELPINFO_DEVICES_VERBOSEHELP), HiiHandle);
    Status = EFI_SUCCESS;
    goto Done;
  }
  //
  // Display all device handles
  //
  Status = LibLocateHandle (
            AllHandles,
            NULL,
            NULL,
            &DeviceHandleCount,
            &DeviceHandleBuffer
            );
  if (EFI_ERROR (Status)) {
    Status = EFI_NOT_FOUND;
    goto Done;
  }

  PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_HC_T_DN), HiiHandle);
  PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_HT_Y_C_IN), HiiHandle);
  PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_HR_P_F_AN), HiiHandle);
  PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_HL_ECGPDC_N), HiiHandle);
  PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_H_EQUAL_SIGNS), HiiHandle);

  for (Index = 0; Index < DeviceHandleCount; Index++) {
    if (GetExecutionBreak ()) {
      Status = EFI_ABORTED;
      break;
    }

    Status = ShellGetDeviceName (
              DeviceHandleBuffer[Index],
              TRUE,
              TRUE,
              Language,
              &BestDeviceName,
              &ConfigurationStatus,
              &DiagnosticsStatus,
              FALSE,
              0
              );

    //
    // Retrieve the best name for DeviceHandleBuffer[Index].  Also determine if
    // the device support being configured, or having diagnostics run on it.
    // Also determine the number of parents controllers, the number of managing
    // drivers, and the number of children.
    //
    Status = LibGetManagingDriverBindingHandles (
              DeviceHandleBuffer[Index],
              &DriverBindingHandleCount,
              &DriverBindingHandleBuffer
              );

    Status = LibGetParentControllerHandles (
              DeviceHandleBuffer[Index],
              &ParentControllerHandleCount,
              &ParentControllerHandleBuffer
              );

    Status = LibGetChildControllerHandles (
              DeviceHandleBuffer[Index],
              &ChildControllerHandleCount,
              &ChildControllerHandleBuffer
              );

    if (DriverBindingHandleCount > 0 || ParentControllerHandleCount > 0 || ChildControllerHandleCount > 0) {
      PrintToken (
        STRING_TOKEN (STR_SHELLENV_PROTID_ONE_VAR_H02),
        HiiHandle,
        ShellHandleToIndex (DeviceHandleBuffer[Index])
        );

      if (ParentControllerHandleCount == 0) {
        Print (L" R");
      } else if (ChildControllerHandleCount > 0) {
        Print (L" B");
      } else {
        Print (L" D");
      }

      if (!EFI_ERROR (ConfigurationStatus)) {
        Print (L" X");
      } else {
        Print (L" -");
      }

      if (!EFI_ERROR (DiagnosticsStatus)) {
        Print (L" X");
      } else {
        Print (L" -");
      }

      if (ParentControllerHandleCount == 0) {
        Print (L"  -");
      } else {
        PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_ONE_VAR_D), HiiHandle, ParentControllerHandleCount);
      }

      if (DriverBindingHandleCount == 0) {
        Print (L"  -");
      } else {
        PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_ONE_VAR_D), HiiHandle, DriverBindingHandleCount);
      }

      if (ChildControllerHandleCount == 0) {
        Print (L"  -");
      } else {
        PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_ONE_VAR_D), HiiHandle, ChildControllerHandleCount);
      }

      if (BestDeviceName == NULL) {
        PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_UNKNOWN_2), HiiHandle);
      } else {
        PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_ONE_VAR_S_SPACE), HiiHandle, BestDeviceName);
      }
    }

    if (DriverBindingHandleBuffer) {
      FreePool (DriverBindingHandleBuffer);
    }

    if (ParentControllerHandleBuffer) {
      FreePool (ParentControllerHandleBuffer);
    }

    if (ChildControllerHandleBuffer) {
      FreePool (ChildControllerHandleBuffer);
    }
  }

  FreePool (DeviceHandleBuffer);

Done:
  FreePool (Language);
  return Status;
}
