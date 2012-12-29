/*++
 
Copyright (c) 2005 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  drivers.c
  
Abstract:

  Shell command "drivers"



Revision History

--*/

#include "EfiShellLib.h"
#include "drivers.h"

#include EFI_PROTOCOL_DEFINITION (ComponentName)
#include EFI_PROTOCOL_DEFINITION (DriverConfiguration)
#include EFI_PROTOCOL_DEFINITION (DriverDiagnostics)

extern UINT8  STRING_ARRAY_NAME[];

//
// This is the generated header file which includes whatever needs to be exported (strings + IFR)
//
#include STRING_DEFINES_FILE

//
// Global Variables
//
EFI_HII_HANDLE  HiiHandle;
EFI_GUID        EfiDriversGuid = EFI_DRIVERS_GUID;
SHELL_VAR_CHECK_ITEM    DriversCheckList[] = {
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

EFI_BOOTSHELL_CODE(
  EFI_APPLICATION_ENTRY_POINT(DriversMain)
)
//
//
//
EFI_STATUS
ShellDeviceTree (
  IN EFI_HANDLE       ControllerHandle,
  IN UINTN            Level,
  IN BOOLEAN          RootOnly,
  IN BOOLEAN          BestName,
  IN CHAR8            *Language
  );

EFI_STATUS
DriversSyntaxOld (
  IN  EFI_HANDLE         ImageHandle,
  IN  EFI_SYSTEM_TABLE   *SystemTable,
  OUT CHAR8              **Language,
  OUT BOOLEAN            *IsHelp
  );

EFI_STATUS
EFIAPI
DriversMain (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  );

EFI_STATUS
EFIAPI
DriversMain (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  )
/*++

Routine Description:

  Code for external shell "drivers" command.

Arguments:

  ImageHandle - The image handle
  SystemTable - THe system table

Returns:

--*/
{
  EFI_STATUS                  Status;
  EFI_STATUS                  DiagnosticsStatus;
  EFI_STATUS                  ConfigurationStatus;
  UINTN                       StringIndex;
  UINTN                       Index;
  CHAR8                       *Language;
  UINTN                       DriverImageHandleCount;
  EFI_HANDLE                  *DriverImageHandleBuffer;
  UINTN                       HandleIndex;
  EFI_DRIVER_BINDING_PROTOCOL *DriverBinding;
  EFI_LOADED_IMAGE_PROTOCOL   *LoadedImage;
  EFI_COMPONENT_NAME_PROTOCOL *ComponentName;
  CHAR16                      *DriverName;
  CHAR16                      FormattedDriverName[40];
  UINTN                       NumberOfChildren;
  UINTN                       ControllerHandleCount;
  EFI_HANDLE                  *ControllerHandleBuffer;
  UINTN                       ChildControllerHandleCount;
  CHAR16                      *ImageName;
  BOOLEAN                     IsHelp;
  SHELL_VAR_CHECK_CODE        RetCode;
  CHAR16                      *Useful;
  SHELL_ARG_LIST              *Item;
  SHELL_VAR_CHECK_PACKAGE     ChkPck;

  Language                = NULL;
  DriverImageHandleCount  = 0;
  DriverImageHandleBuffer = NULL;
  IsHelp                  = FALSE;
  ZeroMem (&ChkPck, sizeof (SHELL_VAR_CHECK_PACKAGE));

  EFI_SHELL_APP_INIT (ImageHandle, SystemTable);

  //
  // Enable tab key which can pause the output
  //
  EnableOutputTabPause();
  
  Status = LibInitializeStrings (&HiiHandle, STRING_ARRAY_NAME, &EfiDriversGuid);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (!EFI_PROPER_VERSION (1, 10)) {
    PrintToken (
      STRING_TOKEN (STR_SHELLENV_GNC_COMMAND_NOT_SUPPORT),
      HiiHandle,
      L"drivers",
      EFI_VERSION_1_10
      );
    Status = EFI_UNSUPPORTED;
    goto Done;
  }

  if (IS_OLD_SHELL) {
    Status = DriversSyntaxOld (ImageHandle, SystemTable, &Language, &IsHelp);
    if (EFI_ERROR (Status)) {
      goto Done;
    }

    if (IsHelp) {
      PrintToken (STRING_TOKEN (STR_NO_HELP), HiiHandle);
      goto Done;
    }
  } else {
    RetCode = LibCheckVariables (SI, DriversCheckList, &ChkPck, &Useful);
    if (VarCheckOk != RetCode) {
      switch (RetCode) {
      case VarCheckDuplicate:
        PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_DUP_FLAG), HiiHandle, L"drivers", Useful);
        break;

      case VarCheckUnknown:
        PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_UNKNOWN_FLAG), HiiHandle, L"drivers", Useful);
        break;

      case VarCheckLackValue:
        PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_LACK_ARG), HiiHandle, L"drivers", Useful);
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
        PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiHandle, L"drivers");
        Status = EFI_INVALID_PARAMETER;
      } else {
        PrintToken (STRING_TOKEN (STR_DRIVERS_VERBOSE_HELP), HiiHandle);
        Status = EFI_SUCCESS;
      }

      goto Done;
    }

    if (ChkPck.ValueCount > 0) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiHandle, L"drivers");
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }

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
        PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_DRIVERS_BAD_LANG), HiiHandle, L"drivers", Item->VarStr);
        Status = EFI_INVALID_PARAMETER;
        goto Done;
      }

      for (StringIndex = 0; StringIndex < 3; StringIndex++) {
        Language[StringIndex] = (CHAR8) Item->VarStr[StringIndex];
      }

      Language[StringIndex] = 0;
    }
  }

  ShellInitProtocolInfoEnumerator ();
  //
  // Display all driver handles
  //
  Status = LibLocateHandle (
            ByProtocol,
            &gEfiDriverBindingProtocolGuid,
            NULL,
            &DriverImageHandleCount,
            &DriverImageHandleBuffer
            );
  if (EFI_ERROR (Status)) {
    Status = EFI_NOT_FOUND;
    goto Done;
  }

  PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_H_T_DN), HiiHandle);
  PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_HD_YC_IN), HiiHandle);
  PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_HR_PF_AN), HiiHandle);
  PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_HV_VERSION), HiiHandle);
  PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_H_EQUAL_SIGNS_N), HiiHandle);

  for (Index = 0; Index < DriverImageHandleCount; Index++) {
    if (GetExecutionBreak ()) {
      break;
    }

    Status = BS->OpenProtocol (
                  DriverImageHandleBuffer[Index],
                  &gEfiDriverBindingProtocolGuid,
                  (VOID **) &DriverBinding,
                  NULL,
                  NULL,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
    if (EFI_ERROR (Status)) {
      continue;
    }

    LoadedImage = NULL;
    Status = BS->OpenProtocol (
                  DriverBinding->ImageHandle,
                  &gEfiLoadedImageProtocolGuid,
                  (VOID **) &LoadedImage,
                  NULL,
                  NULL,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );

    ComponentName = NULL;
    Status = BS->OpenProtocol (
                  DriverImageHandleBuffer[Index],
                  &gEfiComponentNameProtocolGuid,
                  (VOID **) &ComponentName,
                  NULL,
                  NULL,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );

    DiagnosticsStatus = BS->OpenProtocol (
                              DriverImageHandleBuffer[Index],
                              &gEfiDriverDiagnosticsProtocolGuid,
                              NULL,
                              NULL,
                              NULL,
                              EFI_OPEN_PROTOCOL_TEST_PROTOCOL
                              );

    ConfigurationStatus = BS->OpenProtocol (
                                DriverImageHandleBuffer[Index],
                                &gEfiDriverConfigurationProtocolGuid,
                                NULL,
                                NULL,
                                NULL,
                                EFI_OPEN_PROTOCOL_TEST_PROTOCOL
                                );

    NumberOfChildren = 0;
    Status = LibGetManagedControllerHandles (
              DriverImageHandleBuffer[Index],
              &ControllerHandleCount,
              &ControllerHandleBuffer
              );
    if (!EFI_ERROR (Status)) {
      for (HandleIndex = 0; HandleIndex < ControllerHandleCount; HandleIndex++) {
        Status = LibGetManagedChildControllerHandles (
                  DriverImageHandleBuffer[Index],
                  ControllerHandleBuffer[HandleIndex],
                  &ChildControllerHandleCount,
                  NULL
                  );
        NumberOfChildren += ChildControllerHandleCount;
      }

      FreePool (ControllerHandleBuffer);
    }

    PrintToken (
      STRING_TOKEN (STR_SHELLENV_PROTID_TWO_VARS_H_X),
      HiiHandle,
      ShellHandleToIndex (DriverImageHandleBuffer[Index]),
      DriverBinding->Version
      );

    if (NumberOfChildren > 0) {
      Print (L" B");
    } else if (ControllerHandleCount > 0) {
      Print (L" D");
    } else {
      Print (L" ?");
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

    if (ControllerHandleCount == 0) {
      Print (L"  -");
    } else {
      PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_ONE_VAR_D), HiiHandle, ControllerHandleCount);
    }

    if (NumberOfChildren == 0) {
      Print (L"  -");
    } else {
      PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_ONE_VAR_D), HiiHandle, NumberOfChildren);
    }

    DriverName = L"<UNKNOWN>";
    if (ComponentName != NULL) {

      if (ComponentName->GetDriverName != NULL) {
        Status = ComponentName->GetDriverName (
                                  ComponentName,
                                  Language,
                                  &DriverName
                                  );
      }

      if (EFI_ERROR (Status)) {
        DriverName = L"<UNKNOWN>";
      }
    }

    for (StringIndex = 0; StringIndex < StrLen (DriverName) && StringIndex < 35; StringIndex++) {
      FormattedDriverName[StringIndex] = DriverName[StringIndex];
    }

    for (StringIndex = StrLen (DriverName); StringIndex < 35; StringIndex++) {
      FormattedDriverName[StringIndex] = L' ';
    }

    FormattedDriverName[35] = ' ';
    FormattedDriverName[36] = 0;
    PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_ONE_VAR_S_2), HiiHandle, FormattedDriverName);

    if (LoadedImage == NULL) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_UNKNOWN_2), HiiHandle);
    } else {
      ImageName = LibGetImageName (LoadedImage);
      if (ImageName != NULL) {
        Print (ImageName);
        Print (L"\n");
        FreePool (ImageName);
      } else {
        DriverName = LibDevicePathToStr (LoadedImage->FilePath);
        if (StrLen (DriverName) > 18) {
          DriverName[18] = 0;
        }

        PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_ONE_VAR_S), HiiHandle, DriverName);
        FreePool (DriverName);
      }
    }
  }

  Status = EFI_SUCCESS;
Done:
  if (DriverImageHandleCount != 0) {
    FreePool (DriverImageHandleBuffer);
  }

  if (Language != NULL) {
    FreePool (Language);
  }

  ShellCloseHandleEnumerator ();
  LibCheckVarFreeVarList (&ChkPck);
  LibUnInitializeStrings ();
  return Status;
}

EFI_STATUS
EFIAPI
DriversGetLineHelp (
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
  return LibCmdGetStringByToken (STRING_ARRAY_NAME, &EfiDriversGuid, STRING_TOKEN (STR_DRIVERS_LINE_HELP), Str);
}
//
// Compatible support
//
EFI_STATUS
DriversSyntaxOld (
  IN  EFI_HANDLE         ImageHandle,
  IN  EFI_SYSTEM_TABLE   *SystemTable,
  OUT CHAR8              **Language,
  OUT BOOLEAN            *IsHelp
  )
{
  EFI_STATUS  Status;
  UINTN       Index;
  UINTN       StringIndex;
  CHAR16      *Ptr;

  *Language = LibGetVariable (VarLanguage, &gEfiGlobalVariableGuid);
  if (*Language == NULL) {
    *Language       = AllocatePool (4);
    (*Language)[0]  = 'e';
    (*Language)[1]  = 'n';
    (*Language)[2]  = 'g';
    (*Language)[3]  = 0;
  }

  for (Index = 1; Index < SI->Argc; Index += 1) {
    Ptr = SI->Argv[Index];
    if (*Ptr == '-') {
      switch (Ptr[1]) {
      case 'l':
      case 'L':
        if (*(Ptr + 2) != 0) {
          for (StringIndex = 0; StringIndex < 3 && Ptr[StringIndex + 2] != 0; StringIndex++) {
            (*Language)[StringIndex] = (CHAR8) Ptr[StringIndex + 2];
          }
          (*Language)[StringIndex] = 0;
        }
        break;

      case 'b':
      case 'B':
        EnablePageBreak (DEFAULT_INIT_ROW, DEFAULT_AUTO_LF);
        break;

      case '?':
        *IsHelp = TRUE;
        break;

      default:
        PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_UNKNOWN_FLAG), HiiHandle, L"drivers", Ptr);
        Status = EFI_INVALID_PARAMETER;
        goto Done;
      }
    } else {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiHandle, L"drivers");
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }
  }

  Status = EFI_SUCCESS;
Done:
  return Status;
}
