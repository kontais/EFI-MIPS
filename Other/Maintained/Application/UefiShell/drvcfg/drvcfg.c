/*++
 
Copyright (c) 2005 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  drvcfg.c
  
Abstract:

  Shell command "drvcfg"



Revision History

--*/

#include "EfiShellLib.h"
#include "drvcfg.h"

#include EFI_PROTOCOL_DEFINITION (ComponentName)
#include EFI_PROTOCOL_DEFINITION (DriverConfiguration)

extern UINT8  STRING_ARRAY_NAME[];

//
// This is the generated header file which includes whatever needs to be exported (strings + IFR)
//
#include STRING_DEFINES_FILE

//
// Global Variables
//
EFI_HII_HANDLE  HiiHandle;
EFI_GUID        EfiDrvcfgGuid = EFI_DRVCFG_GUID;
SHELL_VAR_CHECK_ITEM  DrvcfgCheckList[] = {
  {
    L"-l",
    0x01,
    0x40,
    FlagTypeNeedVar
  },
  {
    L"-c",
    0x02,
    0x40,
    FlagTypeSingle
  },
  {
    L"-f",
    0x04,
    0x58,
    FlagTypeNeedVar
  },
  {
    L"-v",
    0x08,
    0x54,
    FlagTypeSingle
  },
  {
    L"-s",
    0x10,
    0x4c,
    FlagTypeSingle
  },
  {
    L"-b",
    0x20,
    0,
    FlagTypeSingle
  },
  {
    L"-?",
    0x40,
    0x1f,
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
ShellCmdDriverConfigurationProcessActionRequired (
  EFI_HANDLE                                DriverImageHandle,
  EFI_HANDLE                                ControllerHandle,
  EFI_HANDLE                                ChildHandle,
  EFI_DRIVER_CONFIGURATION_ACTION_REQUIRED  ActionRequired
  );

EFI_BOOTSHELL_CODE(
  EFI_APPLICATION_ENTRY_POINT(DrvcfgMain)
)

EFI_STATUS
SEnvCmdDriverConfiguration (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  );

EFI_STATUS
EFIAPI
DrvcfgMain (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  )
/*++

Routine Description:

  Code for external shell "drvcfg" command.

Arguments:

  ImageHandle - The image handle
  SystemTable - The system table

Returns:

--*/
{
  EFI_STATUS                                Status;
  UINTN                                     HandleNumber;
  EFI_HANDLE                                DriverImageHandle;
  EFI_HANDLE                                DeviceHandle;
  EFI_HANDLE                                ChildHandle;
  UINTN                                     StringIndex;
  UINTN                                     Index;
  CHAR8                                     *Language;
  CHAR8                                     *SupportedLanguages;
  UINTN                                     DriverImageHandleCount;
  EFI_HANDLE                                *DriverImageHandleBuffer;
  UINTN                                     HandleCount;
  EFI_HANDLE                                *HandleBuffer;
  UINT32                                    *HandleType;
  UINTN                                     HandleIndex;
  UINTN                                     ChildIndex;
  UINTN                                     ChildHandleCount;
  EFI_HANDLE                                *ChildHandleBuffer;
  UINT32                                    *ChildHandleType;
  EFI_DRIVER_CONFIGURATION_ACTION_REQUIRED  ActionRequired;
  EFI_DRIVER_CONFIGURATION_PROTOCOL         *DriverConfiguration;
  BOOLEAN                                   ForceDefaults;
  UINT32                                    DefaultType;
  BOOLEAN                                   ValidateOptions;
  BOOLEAN                                   SetOptions;
  BOOLEAN                                   AllChildren;
  SHELL_VAR_CHECK_CODE                      RetCode;
  CHAR16                                    *Useful;
  SHELL_ARG_LIST                            *Item;
  SHELL_VAR_CHECK_PACKAGE                   ChkPck;

  Language                = NULL;
  DriverImageHandle       = NULL;
  DeviceHandle            = NULL;
  ChildHandle             = NULL;
  ForceDefaults           = FALSE;
  DefaultType             = 0;
  ValidateOptions         = FALSE;
  SetOptions              = FALSE;
  AllChildren             = FALSE;
  DriverImageHandleCount  = 0;
  DriverImageHandleBuffer = NULL;
  ZeroMem (&ChkPck, sizeof (SHELL_VAR_CHECK_PACKAGE));

  EFI_SHELL_APP_INIT (ImageHandle, SystemTable);

  //
  // Enable tab key which can pause the output
  //
  EnableOutputTabPause();
  
  Status = LibInitializeStrings (&HiiHandle, STRING_ARRAY_NAME, &EfiDrvcfgGuid);

  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (!EFI_PROPER_VERSION (1, 10)) {
    PrintToken (
      STRING_TOKEN (STR_SHELLENV_GNC_COMMAND_NOT_SUPPORT),
      HiiHandle,
      L"drvcfg",
      EFI_VERSION_1_10
      );
    Status = EFI_UNSUPPORTED;
    goto Done;
  }

  ShellInitHandleEnumerator ();
  ShellInitProtocolInfoEnumerator ();

  if (IS_OLD_SHELL) {
    Status = SEnvCmdDriverConfiguration (ImageHandle, SystemTable);
    goto Done;
  }

  RetCode = LibCheckVariables (SI, DrvcfgCheckList, &ChkPck, &Useful);
  if (VarCheckOk != RetCode) {
    switch (RetCode) {
    case VarCheckDuplicate:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_DUP_FLAG), HiiHandle, L"drvcfg", Useful);
      break;

    case VarCheckUnknown:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_UNKNOWN_FLAG), HiiHandle, L"drvcfg", Useful);
      break;

    case VarCheckLackValue:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_LACK_ARG), HiiHandle, L"drvcfg", Useful);
      break;

    case VarCheckConflict:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_FLAG_CONFLICT), HiiHandle, L"drvcfg", Useful);
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
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiHandle, L"drvcfg");
      Status = EFI_INVALID_PARAMETER;
    } else {
      PrintToken (STRING_TOKEN (STR_DRVCFG_VERBOSE_HELP), HiiHandle);
      Status = EFI_SUCCESS;
    }

    goto Done;
  }
  //
  // Setup Handle and Protocol Globals
  //
  Language = LibGetVariable (VarLanguage, &gEfiGlobalVariableGuid);
  if (Language == NULL) {
    Language    = AllocatePool (4);
    Language[0] = 'e';
    Language[1] = 'n';
    Language[2] = 'g';
    Language[3] = 0;
  }

  Item = LibCheckVarGetFlag (&ChkPck, L"-l");
  if (Item) {
    if (StrLen (Item->VarStr) != 3) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_DRVCFG_BAD_LANG), HiiHandle, L"drvcfg", Item->VarStr);
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }

    for (StringIndex = 0; StringIndex < 3; StringIndex++) {
      Language[StringIndex] = (CHAR8) Item->VarStr[StringIndex];
    }

    Language[StringIndex] = 0;
  }

  Item = LibCheckVarGetFlag (&ChkPck, L"-f");
  if (Item) {
    ForceDefaults = TRUE;
    DefaultType   = (UINT32) (StrToUIntegerBase (Item->VarStr, 16, &Status));
    if (EFI_ERROR (Status)) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_ARG), HiiHandle, L"drvcfg", Item->VarStr);
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }
  }

  if (LibCheckVarGetFlag (&ChkPck, L"-c")) {
    AllChildren = TRUE;
  }

  if (LibCheckVarGetFlag (&ChkPck, L"-v")) {
    ValidateOptions = TRUE;
  }

  if (LibCheckVarGetFlag (&ChkPck, L"-s")) {
    SetOptions = TRUE;
  }

  if (ChkPck.ValueCount > 3) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiHandle, L"drvcfg");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  Item = ChkPck.VarList;
  if (Item != NULL) {
    HandleNumber = (UINTN) (StrToUIntegerBase (Item->VarStr, 16, &Status));
    if (EFI_ERROR (Status)) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_ARG), HiiHandle, L"drvcfg", Item->VarStr);
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }

    DriverImageHandle = ShellHandleFromIndex (HandleNumber - 1);
    if (DriverImageHandle == NULL) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_HANDLE_NUM), HiiHandle, L"drvcfg", Item->VarStr);
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }

    Item = Item->Next;
  }

  if (Item != NULL) {
    HandleNumber = (UINTN) (StrToUIntegerBase (Item->VarStr, 16, &Status));
    if (EFI_ERROR (Status)) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_ARG), HiiHandle, L"drvcfg", Item->VarStr);
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }

    DeviceHandle = ShellHandleFromIndex (HandleNumber - 1);
    if (DeviceHandle == NULL) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_HANDLE_NUM), HiiHandle, L"drvcfg", Item->VarStr);
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }

    Item = Item->Next;
  }

  if (Item != NULL) {
    HandleNumber = (UINTN) (StrToUIntegerBase (Item->VarStr, 16, &Status));
    if (EFI_ERROR (Status)) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_ARG), HiiHandle, L"drvcfg", Item->VarStr);
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }

    ChildHandle = ShellHandleFromIndex (HandleNumber - 1);
    if (ChildHandle == NULL) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_HANDLE_NUM), HiiHandle, L"drvcfg", Item->VarStr);
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }
  }

  if (ChildHandle == NULL && AllChildren) {
    SetOptions = FALSE;
  }

  if (ForceDefaults) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_FORCE_DEFAULT), HiiHandle, DefaultType);
  } else if (ValidateOptions) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_VALIDATE_CONFIG_OPTIONS), HiiHandle);
  } else if (SetOptions) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_SET_CONFIG_OPTIONS), HiiHandle);
  } else {
    PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_CONFIGURABLE_COMPONENTS), HiiHandle);
  }
  //
  // Display all handles that support being configured
  //
  if (DriverImageHandle == NULL) {
    Status = LibLocateHandle (
              ByProtocol,
              &gEfiDriverConfigurationProtocolGuid,
              NULL,
              &DriverImageHandleCount,
              &DriverImageHandleBuffer
              );
    if (EFI_ERROR (Status)) {
      Status = EFI_NOT_FOUND;
      goto Done;
    }
  } else {
    DriverImageHandleCount = 1;
    //
    // Allocate buffer to hold the image handle so as to
    // keep consistent with the above clause
    //
    DriverImageHandleBuffer = AllocatePool (sizeof (EFI_HANDLE));
    ASSERT (DriverImageHandleBuffer);
    DriverImageHandleBuffer[0] = DriverImageHandle;
  }
  //
  // MARMAR:  We need to look at getting the component name exporters to support more than just "eng"
  //
  for (Index = 0; Index < DriverImageHandleCount; Index++) {
    Status = BS->OpenProtocol (
                  DriverImageHandleBuffer[Index],
                  &gEfiDriverConfigurationProtocolGuid,
                  (VOID **) &DriverConfiguration,
                  NULL,
                  NULL,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
    if (EFI_ERROR (Status)) {
      PrintToken (
        STRING_TOKEN (STR_SHELLENV_PROTID_DRVCFG_NOT_SUPPORT_PROT),
        HiiHandle,
        ShellHandleToIndex (DriverImageHandleBuffer[Index])
        );
      continue;
    }

    Status = EFI_NOT_FOUND;
    for (SupportedLanguages = DriverConfiguration->SupportedLanguages;
         SupportedLanguages[0] != 0;
         SupportedLanguages += 3
        ) {
      if (CompareMem (SupportedLanguages, Language, 3) == 0) {
        Status = EFI_SUCCESS;
      }
    }

    if (EFI_ERROR (Status)) {
      PrintToken (
        STRING_TOKEN (STR_SHELLENV_PROTID_DRVCFG_HANDLE_NOT_SUPPORT_LANG),
        HiiHandle,
        ShellHandleToIndex (DriverImageHandleBuffer[Index]),
        Language
        );
      continue;
    }

    Status = LibScanHandleDatabase (
              DriverImageHandleBuffer[Index],
              NULL,
              NULL,
              NULL,
              &HandleCount,
              &HandleBuffer,
              &HandleType
              );
    if (EFI_ERROR (Status)) {
      continue;
    }

    if (SetOptions && DeviceHandle == NULL) {

      ST->ConOut->ClearScreen (ST->ConOut);
      Status = DriverConfiguration->SetOptions (
                                      DriverConfiguration,
                                      NULL,
                                      NULL,
                                      Language,
                                      &ActionRequired
                                      );
      ST->ConOut->ClearScreen (ST->ConOut);

      PrintToken (
        STRING_TOKEN (STR_SHELLENV_PROTID_DRV_ALL_LANG),
        HiiHandle,
        ShellHandleToIndex (DriverImageHandleBuffer[Index]),
        DriverConfiguration->SupportedLanguages
        );
      if (!EFI_ERROR (Status)) {
        PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_OPTIONS_SET), HiiHandle);

        for (HandleIndex = 0; HandleIndex < HandleCount; HandleIndex++) {

          if ((HandleType[HandleIndex] & EFI_HANDLE_TYPE_CONTROLLER_HANDLE) == EFI_HANDLE_TYPE_CONTROLLER_HANDLE) {

            ShellCmdDriverConfigurationProcessActionRequired (
              DriverImageHandleBuffer[Index],
              HandleBuffer[HandleIndex],
              NULL,
              ActionRequired
              );

          }
        }

      } else {
        PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_OPTIONS_NOT_SET), HiiHandle, Status);
      }

      continue;
    }

    for (HandleIndex = 0; HandleIndex < HandleCount; HandleIndex++) {

      if ((HandleType[HandleIndex] & EFI_HANDLE_TYPE_CONTROLLER_HANDLE) != EFI_HANDLE_TYPE_CONTROLLER_HANDLE) {
        continue;
      }

      if (DeviceHandle != NULL && DeviceHandle != HandleBuffer[HandleIndex]) {
        continue;
      }

      if (ChildHandle == NULL) {
        PrintToken (
          STRING_TOKEN (STR_SHELLENV_PROTID_DRV_CTRL_LANG),
          HiiHandle,
          ShellHandleToIndex (DriverImageHandleBuffer[Index]),
          ShellHandleToIndex (HandleBuffer[HandleIndex]),
          DriverConfiguration->SupportedLanguages
          );

        if (ForceDefaults) {
          Status = DriverConfiguration->ForceDefaults (
                                          DriverConfiguration,
                                          HandleBuffer[HandleIndex],
                                          NULL,
                                          DefaultType,
                                          &ActionRequired
                                          );

          if (!EFI_ERROR (Status)) {
            PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_DEFAULTS_FORCED), HiiHandle);

            ShellCmdDriverConfigurationProcessActionRequired (
              DriverImageHandleBuffer[Index],
              HandleBuffer[HandleIndex],
              NULL,
              ActionRequired
              );
          } else {
            PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_DEFAULTS_FORCE_FAILED), HiiHandle, Status);
          }
        } else if (ValidateOptions) {
          Status = DriverConfiguration->OptionsValid (
                                          DriverConfiguration,
                                          HandleBuffer[HandleIndex],
                                          NULL
                                          );

          if (!EFI_ERROR (Status)) {
            PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_OPTIONS_VALID), HiiHandle);
          } else {
            PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_OPTIONS_NOT_VALID), HiiHandle, Status);
          }
        } else if (SetOptions) {
          ST->ConOut->ClearScreen (ST->ConOut);
          Status = DriverConfiguration->SetOptions (
                                          DriverConfiguration,
                                          HandleBuffer[HandleIndex],
                                          NULL,
                                          Language,
                                          &ActionRequired
                                          );
          ST->ConOut->ClearScreen (ST->ConOut);
          PrintToken (
            STRING_TOKEN (STR_SHELLENV_PROTID_DRV_CTRL_LANG),
            HiiHandle,
            ShellHandleToIndex (DriverImageHandleBuffer[Index]),
            ShellHandleToIndex (HandleBuffer[HandleIndex]),
            DriverConfiguration->SupportedLanguages
            );
          if (!EFI_ERROR (Status)) {
            PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_OPTIONS_SET), HiiHandle);

            ShellCmdDriverConfigurationProcessActionRequired (
              DriverImageHandleBuffer[Index],
              HandleBuffer[HandleIndex],
              NULL,
              ActionRequired
              );

          } else {
            PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_OPTIONS_NOT_SET), HiiHandle, Status);
          }
        } else {
          Print (L"\n");
        }
      }

      if (ChildHandle == NULL && !AllChildren) {
        continue;
      }

      Status = LibScanHandleDatabase (
                DriverImageHandleBuffer[Index],
                NULL,
                HandleBuffer[HandleIndex],
                NULL,
                &ChildHandleCount,
                &ChildHandleBuffer,
                &ChildHandleType
                );
      if (EFI_ERROR (Status)) {
        continue;
      }

      for (ChildIndex = 0; ChildIndex < ChildHandleCount; ChildIndex++) {

        if ((ChildHandleType[ChildIndex] & EFI_HANDLE_TYPE_CHILD_HANDLE) != EFI_HANDLE_TYPE_CHILD_HANDLE) {
          continue;
        }

        if (ChildHandle != NULL && ChildHandle != ChildHandleBuffer[ChildIndex]) {
          continue;
        }

        PrintToken (
          STRING_TOKEN (STR_SHELLENV_PROTID_DRV_CTRL_CHILD_LANG),
          HiiHandle,
          ShellHandleToIndex (DriverImageHandleBuffer[Index]),
          ShellHandleToIndex (HandleBuffer[HandleIndex]),
          ShellHandleToIndex (ChildHandleBuffer[ChildIndex]),
          DriverConfiguration->SupportedLanguages
          );

        if (ForceDefaults) {
          Status = DriverConfiguration->ForceDefaults (
                                          DriverConfiguration,
                                          HandleBuffer[HandleIndex],
                                          ChildHandleBuffer[ChildIndex],
                                          DefaultType,
                                          &ActionRequired
                                          );

          if (!EFI_ERROR (Status)) {
            PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_DEFAULTS_FORCED), HiiHandle);

            ShellCmdDriverConfigurationProcessActionRequired (
              DriverImageHandleBuffer[Index],
              HandleBuffer[HandleIndex],
              ChildHandleBuffer[ChildIndex],
              ActionRequired
              );

          } else {
            PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_FORCE_DEFAULTS_FAILED), HiiHandle, Status);
          }
        } else if (ValidateOptions) {
          Status = DriverConfiguration->OptionsValid (
                                          DriverConfiguration,
                                          HandleBuffer[HandleIndex],
                                          ChildHandleBuffer[ChildIndex]
                                          );

          if (!EFI_ERROR (Status)) {
            PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_OPTIONS_VALID), HiiHandle);
          } else {
            PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_OPTIONS_NOT_VALID), HiiHandle, Status);
          }
        } else if (SetOptions) {
          ST->ConOut->ClearScreen (ST->ConOut);
          Status = DriverConfiguration->SetOptions (
                                          DriverConfiguration,
                                          HandleBuffer[HandleIndex],
                                          ChildHandleBuffer[ChildIndex],
                                          Language,
                                          &ActionRequired
                                          );
          ST->ConOut->ClearScreen (ST->ConOut);
          PrintToken (
            STRING_TOKEN (STR_SHELLENV_PROTID_DRV_CTRL_CHILD_LANG),
            HiiHandle,
            ShellHandleToIndex (DriverImageHandleBuffer[Index]),
            ShellHandleToIndex (HandleBuffer[HandleIndex]),
            ShellHandleToIndex (ChildHandleBuffer[ChildIndex]),
            DriverConfiguration->SupportedLanguages
            );
          if (!EFI_ERROR (Status)) {
            PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_OPTIONS_SET), HiiHandle);

            ShellCmdDriverConfigurationProcessActionRequired (
              DriverImageHandleBuffer[Index],
              HandleBuffer[HandleIndex],
              ChildHandleBuffer[ChildIndex],
              ActionRequired
              );

          } else {
            PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_OPTIONS_NOT_SET), HiiHandle, Status);
          }
        } else {
          Print (L"\n");
        }
      }

      FreePool (ChildHandleBuffer);
      FreePool (ChildHandleType);
    }

    FreePool (HandleBuffer);
    FreePool (HandleType);
  }

  Status = EFI_SUCCESS;
Done:
  if (DriverImageHandle != NULL && DriverImageHandleCount != 0) {
    FreePool (DriverImageHandleBuffer);
  }

  if (Language != NULL) {
    FreePool (Language);
  }

  ShellCloseProtocolInfoEnumerator ();
  ShellCloseHandleEnumerator ();
  LibCheckVarFreeVarList (&ChkPck);
  LibUnInitializeStrings ();
  return Status;
}

EFI_STATUS
ShellCmdDriverConfigurationProcessActionRequired (
  EFI_HANDLE                                DriverImageHandle,
  EFI_HANDLE                                ControllerHandle,
  EFI_HANDLE                                ChildHandle,
  EFI_DRIVER_CONFIGURATION_ACTION_REQUIRED  ActionRequired
  )

{
  CHAR16      ReturnStr[2];
  EFI_HANDLE  ContextOverride[2];

  switch (ActionRequired) {
  case EfiDriverConfigurationActionNone:
    PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_NONE), HiiHandle);
    break;

  case EfiDriverConfigurationActionStopController:
    PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_STOP_CONTROLLER), HiiHandle);
    PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_ENTER_STOP_CONTROL), HiiHandle);
    Input (L"", ReturnStr, sizeof (ReturnStr) / sizeof (CHAR16));
    BS->DisconnectController (ControllerHandle, DriverImageHandle, ChildHandle);
    PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_CONTROLLER_STOPPED), HiiHandle);
    break;

  case EfiDriverConfigurationActionRestartController:
    PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_RESTART_CONTROLLER), HiiHandle);
    PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_ENTER_RESTART_CONTROLLER), HiiHandle);
    Input (L"", ReturnStr, sizeof (ReturnStr) / sizeof (CHAR16));
    BS->DisconnectController (ControllerHandle, DriverImageHandle, ChildHandle);
    ContextOverride[0]  = DriverImageHandle;
    ContextOverride[1]  = NULL;
    BS->ConnectController (ControllerHandle, ContextOverride, NULL, TRUE);
    PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_CONTROLLER_RESTARTED), HiiHandle);
    break;

  case EfiDriverConfigurationActionRestartPlatform:
    PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_RESTART_PLATFORM), HiiHandle);
    PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_ENTER_RESTART_PLATFORM), HiiHandle);
    Input (L"", ReturnStr, sizeof (ReturnStr) / sizeof (CHAR16));
    RT->ResetSystem (EfiResetCold, EFI_SUCCESS, 0, NULL);
    break;

  default:
    PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_UNKNOWN_3), HiiHandle);
    break;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
DrvcfgGetLineHelp (
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
  return LibCmdGetStringByToken (STRING_ARRAY_NAME, &EfiDrvcfgGuid, STRING_TOKEN (STR_DRVCFG_LINE_HELP), Str);
}
//
// Compatible Support
//
EFI_STATUS
SEnvCmdDriverConfiguration (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  )
/*++

Routine Description:

Arguments:

  ImageHandle - The image handle
  SystemTable - The system table

Returns:

  EFI_SUCCESS - Success

--*/
{
  EFI_STATUS                                Status;
  CHAR16                                    *Ptr;
  UINTN                                     HandleNumber;
  EFI_HANDLE                                DriverImageHandle;
  EFI_HANDLE                                DeviceHandle;
  EFI_HANDLE                                ChildHandle;
  UINTN                                     StringIndex;
  UINTN                                     Index;
  CHAR8                                     *Language;
  CHAR8                                     *SupportedLanguages;
  UINTN                                     DriverImageHandleCount;
  EFI_HANDLE                                *DriverImageHandleBuffer;
  UINTN                                     HandleCount;
  EFI_HANDLE                                *HandleBuffer;
  UINT32                                    *HandleType;
  UINTN                                     HandleIndex;
  UINTN                                     ChildIndex;
  UINTN                                     ChildHandleCount;
  EFI_HANDLE                                *ChildHandleBuffer;
  UINT32                                    *ChildHandleType;
  EFI_DRIVER_CONFIGURATION_ACTION_REQUIRED  ActionRequired;
  EFI_DRIVER_CONFIGURATION_PROTOCOL         *DriverConfiguration;
  BOOLEAN                                   ForceDefaults;
  UINT32                                    DefaultType;
  BOOLEAN                                   ValidateOptions;
  BOOLEAN                                   SetOptions;
  BOOLEAN                                   AllChildren;
  BOOLEAN                                   GetHelp;

  //
  // Setup Handle and Protocol Globals
  //
  DriverImageHandle       = NULL;
  DeviceHandle            = NULL;
  ChildHandle             = NULL;
  ForceDefaults           = FALSE;
  DefaultType             = 0;
  ValidateOptions         = FALSE;
  SetOptions              = FALSE;
  AllChildren             = FALSE;
  DriverImageHandleCount  = 0;
  DriverImageHandleBuffer = NULL;
  GetHelp                 = FALSE;

  Language                = LibGetVariable (VarLanguage, &gEfiGlobalVariableGuid);
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

      case 'c':
      case 'C':
        AllChildren = TRUE;
        break;

      case 'f':
      case 'F':
        if (ValidateOptions || SetOptions) {
          PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_FLAG_CONFLICT), HiiHandle, L"drvcfg", L"-f");
          Status = EFI_INVALID_PARAMETER;
          goto Done;
        }

        ForceDefaults = TRUE;
        DefaultType   = (UINT32) (Xtoi (Ptr + 2));
        break;

      case 'v':
      case 'V':
        if (ForceDefaults || SetOptions) {
          PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_FLAG_CONFLICT), HiiHandle, L"drvcfg", L"-v");
          Status = EFI_INVALID_PARAMETER;
          goto Done;
        }

        ValidateOptions = TRUE;
        break;

      case 's':
      case 'S':
        if (ForceDefaults || ValidateOptions) {
          PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_FLAG_CONFLICT), HiiHandle, L"drvcfg", L"-s");
          Status = EFI_INVALID_PARAMETER;
          goto Done;
        }

        SetOptions = TRUE;
        break;

      case 'b':
      case 'B':
        EnablePageBreak (DEFAULT_INIT_ROW, DEFAULT_AUTO_LF);
        break;

      case '?':
        GetHelp = TRUE;
        break;

      default:
        PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_UNKNOWN_FLAG), HiiHandle, L"drvcfg", Ptr);
        Status = EFI_INVALID_PARAMETER;
        goto Done;
      }
    } else {
      HandleNumber = ShellHandleNoFromStr (Ptr);
      if (HandleNumber == 0) {
        PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_HANDLE_NUM), HiiHandle, L"drvcfg", Ptr);
        Status = EFI_INVALID_PARAMETER;
        goto Done;
      }

      if (DriverImageHandle == NULL) {
        DriverImageHandle = ShellHandleFromIndex (HandleNumber - 1);
      } else if (DeviceHandle == NULL) {
        DeviceHandle = ShellHandleFromIndex (HandleNumber - 1);
      } else if (ChildHandle == NULL) {
        ChildHandle = ShellHandleFromIndex (HandleNumber - 1);
      } else {
        PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiHandle, L"drvcfg");
        Status = EFI_INVALID_PARAMETER;
        goto Done;
      }
    }
  }

  if (GetHelp) {
    PrintToken (STRING_TOKEN (STR_NO_HELP), HiiHandle);
    Status = EFI_SUCCESS;
    goto Done;
  }

  if (ChildHandle == NULL && AllChildren) {
    SetOptions = FALSE;
  }

  if (ForceDefaults) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_FORCE_DEFAULT), HiiHandle, DefaultType);
  } else if (ValidateOptions) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_VALIDATE_CONFIG_OPTIONS), HiiHandle);
  } else if (SetOptions) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_SET_CONFIG_OPTIONS), HiiHandle);
  } else {
    PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_CONFIGURABLE_COMPONENTS), HiiHandle);
  }
  //
  // Display all handles that support being configured
  //
  if (DriverImageHandle == NULL) {
    Status = LibLocateHandle (
              ByProtocol,
              &gEfiDriverConfigurationProtocolGuid,
              NULL,
              &DriverImageHandleCount,
              &DriverImageHandleBuffer
              );
    if (EFI_ERROR (Status)) {
      Status = EFI_NOT_FOUND;
      goto Done;
    }
  } else {
    DriverImageHandleCount = 1;
    //
    // Allocate buffer to hold the image handle so as to
    // keep consistent with the above clause
    //
    DriverImageHandleBuffer = AllocatePool (sizeof (EFI_HANDLE));
    ASSERT (DriverImageHandleBuffer);
    DriverImageHandleBuffer[0] = DriverImageHandle;
  }
  //
  // MARMAR:  We need to look at getting the component name exporters to support more than just "eng"
  //
  for (Index = 0; Index < DriverImageHandleCount; Index++) {
    Status = BS->OpenProtocol (
                  DriverImageHandleBuffer[Index],
                  &gEfiDriverConfigurationProtocolGuid,
                  (VOID **) &DriverConfiguration,
                  NULL,
                  NULL,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
    if (EFI_ERROR (Status)) {
      PrintToken (
        STRING_TOKEN (STR_SHELLENV_PROTID_DRVCFG_NOT_SUPPORT_PROT),
        HiiHandle,
        ShellHandleToIndex (DriverImageHandleBuffer[Index])
        );
      continue;
    }

    Status = EFI_NOT_FOUND;
    for (SupportedLanguages = DriverConfiguration->SupportedLanguages;
         SupportedLanguages[0] != 0;
         SupportedLanguages += 3
        ) {
      if (CompareMem (SupportedLanguages, Language, 3) == 0) {
        Status = EFI_SUCCESS;
      }
    }

    if (EFI_ERROR (Status)) {
      PrintToken (
        STRING_TOKEN (STR_SHELLENV_PROTID_DRVCFG_HANDLE_NOT_SUPPORT_LANG),
        HiiHandle,
        ShellHandleToIndex (DriverImageHandleBuffer[Index]),
        Language
        );
      continue;
    }

    Status = LibScanHandleDatabase (
              DriverImageHandleBuffer[Index],
              NULL,
              NULL,
              NULL,
              &HandleCount,
              &HandleBuffer,
              &HandleType
              );
    if (EFI_ERROR (Status)) {
      continue;
    }

    if (SetOptions && DeviceHandle == NULL) {

      ST->ConOut->ClearScreen (ST->ConOut);
      Status = DriverConfiguration->SetOptions (
                                      DriverConfiguration,
                                      NULL,
                                      NULL,
                                      Language,
                                      &ActionRequired
                                      );
      ST->ConOut->ClearScreen (ST->ConOut);

      PrintToken (
        STRING_TOKEN (STR_SHELLENV_PROTID_DRV_ALL_LANG),
        HiiHandle,
        ShellHandleToIndex (DriverImageHandleBuffer[Index]),
        DriverConfiguration->SupportedLanguages
        );
      if (!EFI_ERROR (Status)) {
        PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_OPTIONS_SET), HiiHandle);

        for (HandleIndex = 0; HandleIndex < HandleCount; HandleIndex++) {

          if ((HandleType[HandleIndex] & EFI_HANDLE_TYPE_CONTROLLER_HANDLE) == EFI_HANDLE_TYPE_CONTROLLER_HANDLE) {

            ShellCmdDriverConfigurationProcessActionRequired (
              DriverImageHandleBuffer[Index],
              HandleBuffer[HandleIndex],
              NULL,
              ActionRequired
              );

          }
        }

      } else {
        PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_OPTIONS_NOT_SET), HiiHandle, Status);
      }

      continue;
    }

    for (HandleIndex = 0; HandleIndex < HandleCount; HandleIndex++) {

      if ((HandleType[HandleIndex] & EFI_HANDLE_TYPE_CONTROLLER_HANDLE) != EFI_HANDLE_TYPE_CONTROLLER_HANDLE) {
        continue;
      }

      if (DeviceHandle != NULL && DeviceHandle != HandleBuffer[HandleIndex]) {
        continue;
      }

      if (ChildHandle == NULL) {
        PrintToken (
          STRING_TOKEN (STR_SHELLENV_PROTID_DRV_CTRL_LANG),
          HiiHandle,
          ShellHandleToIndex (DriverImageHandleBuffer[Index]),
          ShellHandleToIndex (HandleBuffer[HandleIndex]),
          DriverConfiguration->SupportedLanguages
          );

        if (ForceDefaults) {
          Status = DriverConfiguration->ForceDefaults (
                                          DriverConfiguration,
                                          HandleBuffer[HandleIndex],
                                          NULL,
                                          DefaultType,
                                          &ActionRequired
                                          );

          if (!EFI_ERROR (Status)) {
            PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_DEFAULTS_FORCED), HiiHandle);

            ShellCmdDriverConfigurationProcessActionRequired (
              DriverImageHandleBuffer[Index],
              HandleBuffer[HandleIndex],
              NULL,
              ActionRequired
              );
          } else {
            PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_DEFAULTS_FORCE_FAILED), HiiHandle, Status);
          }
        } else if (ValidateOptions) {
          Status = DriverConfiguration->OptionsValid (
                                          DriverConfiguration,
                                          HandleBuffer[HandleIndex],
                                          NULL
                                          );

          if (!EFI_ERROR (Status)) {
            PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_OPTIONS_VALID), HiiHandle);
          } else {
            PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_OPTIONS_NOT_VALID), HiiHandle, Status);
          }
        } else if (SetOptions) {
          ST->ConOut->ClearScreen (ST->ConOut);
          Status = DriverConfiguration->SetOptions (
                                          DriverConfiguration,
                                          HandleBuffer[HandleIndex],
                                          NULL,
                                          Language,
                                          &ActionRequired
                                          );
          ST->ConOut->ClearScreen (ST->ConOut);
          PrintToken (
            STRING_TOKEN (STR_SHELLENV_PROTID_DRV_CTRL_LANG),
            HiiHandle,
            ShellHandleToIndex (DriverImageHandleBuffer[Index]),
            ShellHandleToIndex (HandleBuffer[HandleIndex]),
            DriverConfiguration->SupportedLanguages
            );
          if (!EFI_ERROR (Status)) {
            PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_OPTIONS_SET), HiiHandle);

            ShellCmdDriverConfigurationProcessActionRequired (
              DriverImageHandleBuffer[Index],
              HandleBuffer[HandleIndex],
              NULL,
              ActionRequired
              );

          } else {
            PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_OPTIONS_NOT_SET), HiiHandle, Status);
          }
        } else {
          Print (L"\n");
        }
      }

      if (ChildHandle == NULL && !AllChildren) {
        continue;
      }

      Status = LibScanHandleDatabase (
                DriverImageHandleBuffer[Index],
                NULL,
                HandleBuffer[HandleIndex],
                NULL,
                &ChildHandleCount,
                &ChildHandleBuffer,
                &ChildHandleType
                );
      if (EFI_ERROR (Status)) {
        continue;
      }

      for (ChildIndex = 0; ChildIndex < ChildHandleCount; ChildIndex++) {

        if ((ChildHandleType[ChildIndex] & EFI_HANDLE_TYPE_CHILD_HANDLE) != EFI_HANDLE_TYPE_CHILD_HANDLE) {
          continue;
        }

        if (ChildHandle != NULL && ChildHandle != ChildHandleBuffer[ChildIndex]) {
          continue;
        }

        PrintToken (
          STRING_TOKEN (STR_SHELLENV_PROTID_DRV_CTRL_CHILD_LANG),
          HiiHandle,
          ShellHandleToIndex (DriverImageHandleBuffer[Index]),
          ShellHandleToIndex (HandleBuffer[HandleIndex]),
          ShellHandleToIndex (ChildHandleBuffer[ChildIndex]),
          DriverConfiguration->SupportedLanguages
          );

        if (ForceDefaults) {
          Status = DriverConfiguration->ForceDefaults (
                                          DriverConfiguration,
                                          HandleBuffer[HandleIndex],
                                          ChildHandleBuffer[ChildIndex],
                                          DefaultType,
                                          &ActionRequired
                                          );

          if (!EFI_ERROR (Status)) {
            PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_DEFAULTS_FORCED), HiiHandle);

            ShellCmdDriverConfigurationProcessActionRequired (
              DriverImageHandleBuffer[Index],
              HandleBuffer[HandleIndex],
              ChildHandleBuffer[ChildIndex],
              ActionRequired
              );

          } else {
            PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_FORCE_DEFAULTS_FAILED), HiiHandle, Status);
          }
        } else if (ValidateOptions) {
          Status = DriverConfiguration->OptionsValid (
                                          DriverConfiguration,
                                          HandleBuffer[HandleIndex],
                                          ChildHandleBuffer[ChildIndex]
                                          );

          if (!EFI_ERROR (Status)) {
            PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_OPTIONS_VALID), HiiHandle);
          } else {
            PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_OPTIONS_NOT_VALID), HiiHandle, Status);
          }
        } else if (SetOptions) {
          ST->ConOut->ClearScreen (ST->ConOut);
          Status = DriverConfiguration->SetOptions (
                                          DriverConfiguration,
                                          HandleBuffer[HandleIndex],
                                          ChildHandleBuffer[ChildIndex],
                                          Language,
                                          &ActionRequired
                                          );
          ST->ConOut->ClearScreen (ST->ConOut);
          PrintToken (
            STRING_TOKEN (STR_SHELLENV_PROTID_DRV_CTRL_CHILD_LANG),
            HiiHandle,
            ShellHandleToIndex (DriverImageHandleBuffer[Index]),
            ShellHandleToIndex (HandleBuffer[HandleIndex]),
            ShellHandleToIndex (ChildHandleBuffer[ChildIndex]),
            DriverConfiguration->SupportedLanguages
            );
          if (!EFI_ERROR (Status)) {
            PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_OPTIONS_SET), HiiHandle);

            ShellCmdDriverConfigurationProcessActionRequired (
              DriverImageHandleBuffer[Index],
              HandleBuffer[HandleIndex],
              ChildHandleBuffer[ChildIndex],
              ActionRequired
              );

          } else {
            PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_OPTIONS_NOT_SET), HiiHandle, Status);
          }
        } else {
          Print (L"\n");
        }
      }

      FreePool (ChildHandleBuffer);
      FreePool (ChildHandleType);
    }

    FreePool (HandleBuffer);
    FreePool (HandleType);
  }

Done:
  if (DriverImageHandle != NULL && DriverImageHandleCount != 0) {
    FreePool (DriverImageHandleBuffer);
  }

  FreePool (Language);
  return EFI_SUCCESS;
}
