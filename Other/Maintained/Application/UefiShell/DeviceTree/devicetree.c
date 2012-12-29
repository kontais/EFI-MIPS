/*++
 
Copyright (c) 2005 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  devicetree.c
  
Abstract:

  Shell command "devicetree"



Revision History

--*/

#include "EfiShellLib.h"
#include "devicetree.h"

extern UINT8  STRING_ARRAY_NAME[];

//
// This is the generated header file which includes whatever needs to be exported (strings + IFR)
//
#include STRING_DEFINES_FILE

//
// Global Variables
//
EFI_HII_HANDLE  HiiHandle;
EFI_GUID        EfiDevicetreeGuid = EFI_DEVICETREE_GUID;
SHELL_VAR_CHECK_ITEM    DevicetreeCheckList[] = {
  {
    L"-d",
    0x01,
    0,
    FlagTypeSingle
  },
  {
    L"-l",
    0x02,
    0,
    FlagTypeNeedVar
  },
  {
    L"-b",
    0x04,
    0,
    FlagTypeSingle
  },
  {
    L"-?",
    0x08,
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
ShellDeviceTree (
  IN EFI_HANDLE       ControllerHandle,
  IN UINTN            Level,
  IN BOOLEAN          RootOnly,
  IN BOOLEAN          BestName,
  IN CHAR8            *Language
  );

EFI_STATUS
DevicetreeMain (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  );

EFI_STATUS
DevicetreeMainOld (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  );

EFI_BOOTSHELL_CODE(
  EFI_APPLICATION_ENTRY_POINT(DevicetreeMain)
)
//
//
//
EFI_STATUS
DevicetreeMain (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  )
/*++

Routine Description:

  Code for external shell "devicetree" command.

Arguments:

  ImageHandle - The image handle
  SystemTable - The system table

Returns:

--*/
{
  CHAR16                  *Arg;
  CHAR8                   *Language;
  BOOLEAN                 BestName;
  EFI_STATUS              Status;
  UINTN                   Index;
  UINTN                   StringIndex;
  UINTN                   HandleNumber;
  EFI_HANDLE              Handle;
  SHELL_VAR_CHECK_CODE    RetCode;
  CHAR16                  *Useful;
  SHELL_ARG_LIST          *Item;
  SHELL_VAR_CHECK_PACKAGE ChkPck;

  BestName = TRUE;
  Arg = NULL;
  Language = NULL;
  Handle = NULL;
  ZeroMem (&ChkPck, sizeof (SHELL_VAR_CHECK_PACKAGE));
  EFI_SHELL_APP_INIT (ImageHandle, SystemTable);

  //
  // Enable tab key which can pause the output
  //
  EnableOutputTabPause();
  
  Status = LibInitializeStrings (&HiiHandle, STRING_ARRAY_NAME, &EfiDevicetreeGuid);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (!EFI_PROPER_VERSION (1, 10)) {
    PrintToken (
      STRING_TOKEN (STR_SHELLENV_GNC_COMMAND_NOT_SUPPORT),
      HiiHandle,
      L"devicetree",
      EFI_VERSION_1_10
      );
    Status = EFI_UNSUPPORTED;
    goto Done;
  }

  LibFilterNullArgs ();
  if (IS_OLD_SHELL) {
    Status = DevicetreeMainOld (ImageHandle, SystemTable);
    goto Done;
  }
  //
  // Crack args
  //
  RetCode = LibCheckVariables (SI, DevicetreeCheckList, &ChkPck, &Useful);
  if (VarCheckOk != RetCode) {
    switch (RetCode) {
    case VarCheckDuplicate:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_DUP_FLAG), HiiHandle, L"devtree", Useful);
      break;

    case VarCheckUnknown:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_UNKNOWN_FLAG), HiiHandle, L"devtree", Useful);
      break;

    case VarCheckLackValue:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_LACK_ARG), HiiHandle, L"devtree", Useful);
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
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiHandle, L"devtree");
      Status = EFI_INVALID_PARAMETER;
    } else {
      PrintToken (STRING_TOKEN (STR_DEVTREE_VERBOSEHELP), HiiHandle);
      Status = EFI_SUCCESS;
    }

    goto Done;
  }

  if (ChkPck.ValueCount > 1) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiHandle, L"devtree");
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
      PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_DEVTREE_BAD_LANG), HiiHandle, L"devtree", Item->VarStr);
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }

    for (StringIndex = 0; StringIndex < 3; StringIndex++) {
      Language[StringIndex] = (CHAR8) Item->VarStr[StringIndex];
    }

    Language[StringIndex] = 0;
  }

  if (LibCheckVarGetFlag (&ChkPck, L"-d") != NULL) {
    BestName = FALSE;
  }
  //
  // Load handle & protocol info tables
  //
  ShellInitProtocolInfoEnumerator ();
  ShellInitHandleEnumerator ();
  if (ChkPck.ValueCount == 1) {
    Arg           = ChkPck.VarList->VarStr;
    HandleNumber  = (UINTN) (StrToUIntegerBase (Arg, 16, &Status));
    if (EFI_ERROR (Status)) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_ARG), HiiHandle, L"devtree", ChkPck.VarList->VarStr);
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }

    if ((Handle = ShellHandleFromIndex (HandleNumber - 1)) == NULL) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_HANDLE_NUM), HiiHandle, L"devtree", ChkPck.VarList->VarStr);
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }
  }

  if (Arg) {
    //
    // Dump 1 handle
    //
    PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_DEVICE_TREE), HiiHandle);
    ShellDeviceTree (
      Handle,
      1,
      FALSE,
      BestName,
      Language
      );
  } else {
    //
    // Dump all handles
    //
    PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_DEVICE_TREE), HiiHandle);
    for (Index = 0; Index < ShellGetHandleNum (); Index++) {
      BREAK_LOOP_ON_ESC ();
      ShellDeviceTree (
        ShellHandleFromIndex (Index),
        1,
        TRUE,
        BestName,
        Language
        );
    }
  }

  Status = EFI_SUCCESS;

Done:
  if (Language != NULL) {
    FreePool (Language);
  }

  ShellCloseHandleEnumerator ();
  ShellCloseProtocolInfoEnumerator ();
  LibCheckVarFreeVarList (&ChkPck);
  LibUnInitializeStrings ();
  return Status;
}

EFI_STATUS
ShellDeviceTree (
  IN EFI_HANDLE       ControllerHandle,
  IN UINTN            Level,
  IN BOOLEAN          RootOnly,
  IN BOOLEAN          BestName,
  IN CHAR8            *Language
  )
/*++

Routine Description:

Arguments:

  ControllerHandle - The Controller handle
  Level            - The device level
  RootOnly         - Is root only
  BestName         - The best name
  Language         - The name language

Returns:

  EFI_SUCCESS - Success

--*/
{
  EFI_STATUS                  Status;
  EFI_LOADED_IMAGE_PROTOCOL   *Image;
  EFI_DRIVER_BINDING_PROTOCOL *DriverBinding;
  UINT32                      ControllerHandleIndex;
  UINTN                       HandleCount;
  EFI_HANDLE                  *HandleBuffer;
  UINT32                      *HandleType;
  UINTN                       HandleIndex;
  UINTN                       ChildIndex;
  EFI_DEVICE_PATH_PROTOCOL    *DevicePath;
  UINTN                       ChildHandleCount;
  EFI_HANDLE                  *ChildHandleBuffer;
  UINT32                      *ChildHandleType;
  UINTN                       Index;
  BOOLEAN                     Root;
  EFI_STATUS                  ConfigurationStatus;
  EFI_STATUS                  DiagnosticsStatus;
  CHAR16                      *DeviceName;

  Status = BS->OpenProtocol (
                ControllerHandle,
                &gEfiDriverBindingProtocolGuid,
                (VOID **) &DriverBinding,
                NULL,
                NULL,
                EFI_OPEN_PROTOCOL_GET_PROTOCOL
                );
  if (!EFI_ERROR (Status)) {
    return EFI_SUCCESS;
  }

  Status = BS->OpenProtocol (
                ControllerHandle,
                &gEfiLoadedImageProtocolGuid,
                (VOID **) &Image,
                NULL,
                NULL,
                EFI_OPEN_PROTOCOL_GET_PROTOCOL
                );
  if (!EFI_ERROR (Status)) {
    return EFI_SUCCESS;
  }

  Status = LibScanHandleDatabase (
            NULL,
            NULL,
            ControllerHandle,
            &ControllerHandleIndex,
            &HandleCount,
            &HandleBuffer,
            &HandleType
            );
  if (EFI_ERROR (Status)) {
    return EFI_SUCCESS;
  }

  if (RootOnly) {
    Root = TRUE;
    for (HandleIndex = 0; HandleIndex < HandleCount; HandleIndex++) {
      BREAK_LOOP_ON_ESC ();

      if (HandleType[HandleIndex] & EFI_HANDLE_TYPE_PARENT_HANDLE) {
        Root = FALSE;
      }
    }

    Status = BS->OpenProtocol (
                  ControllerHandle,
                  &gEfiDevicePathProtocolGuid,
                  (VOID **) &DevicePath,
                  NULL,
                  NULL,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
    if (EFI_ERROR (Status)) {
      Root = FALSE;
    }

    if (!Root) {
      return EFI_SUCCESS;
    }
  }
  //
  // Display the handle specified by ControllerHandle
  //
  for (Index = 0; Index < Level; Index++) {
    Print (L"  ");
  }

  PrintToken (
    STRING_TOKEN (STR_SHELLENV_PROTID_CTRL),
    HiiHandle,
    ShellHandleToIndex (ControllerHandle)
    );

  Status = ShellGetDeviceName (
            ControllerHandle,
            BestName,
            TRUE,
            Language,
            &DeviceName,
            &ConfigurationStatus,
            &DiagnosticsStatus,
            FALSE,
            0
            );
  if (DeviceName != NULL) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_ONE_VAR_S), HiiHandle, DeviceName);
  } else {
    PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_UNKNOWN_2), HiiHandle);
  }
  //
  // Print the list of drivers that are managing this controller
  //
  for (HandleIndex = 0; HandleIndex < HandleCount; HandleIndex++) {
    BREAK_LOOP_ON_ESC ();
    if (HandleType[HandleIndex] & EFI_HANDLE_TYPE_DRIVER_BINDING_HANDLE &&
        HandleType[HandleIndex] & EFI_HANDLE_TYPE_DEVICE_DRIVER
        ) {

      DriverBinding = NULL;
      Status = BS->OpenProtocol (
                    HandleBuffer[HandleIndex],
                    &gEfiDriverBindingProtocolGuid,
                    (VOID **) &DriverBinding,
                    NULL,
                    NULL,
                    EFI_OPEN_PROTOCOL_GET_PROTOCOL
                    );

      Status = LibScanHandleDatabase (
                HandleBuffer[HandleIndex],
                NULL,
                ControllerHandle,
                &ControllerHandleIndex,
                &ChildHandleCount,
                &ChildHandleBuffer,
                &ChildHandleType
                );

      if (!EFI_ERROR (Status)) {
        for (ChildIndex = 0; ChildIndex < ChildHandleCount; ChildIndex++) {
          BREAK_LOOP_ON_ESC ();
          if (ChildHandleType[ChildIndex] & EFI_HANDLE_TYPE_CHILD_HANDLE &&
              ChildHandleType[ChildIndex] & EFI_HANDLE_TYPE_DEVICE_HANDLE
              ) {

            Status = ShellDeviceTree (
                      ChildHandleBuffer[ChildIndex],
                      Level + 1,
                      FALSE,
                      BestName,
                      Language
                      );
            if (EFI_ERROR (Status)) {
              return Status;
            }
          }
        }

        FreePool (ChildHandleBuffer);
        FreePool (ChildHandleType);
      }
    }
  }

  FreePool (HandleBuffer);
  FreePool (HandleType);

  return EFI_SUCCESS;
}

EFI_STATUS
DevicetreeMainGetLineHelp (
  OUT CHAR16                **Str
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
  return LibCmdGetStringByToken (STRING_ARRAY_NAME, &EfiDevicetreeGuid, STRING_TOKEN (STR_DEVTREE_LINEHELP), Str);
}

EFI_STATUS
DevicetreeMainOld (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  )
/*++

Routine Description:

  Code for external shell "devicetree" command.

Arguments:

  ImageHandle - The image handle
  SystemTable - The system table
Returns:

--*/
{
  CHAR16      *Arg;
  CHAR16      *Ptr;
  CHAR8       *Language;
  BOOLEAN     BestName;
  EFI_STATUS  Status;
  UINTN       Index;
  UINTN       StringIndex;
  BOOLEAN     PrtHelp;

  Status    = EFI_SUCCESS;
  PrtHelp   = FALSE;
  Language  = LibGetVariable (VarLanguage, &gEfiGlobalVariableGuid);
  if (Language == NULL) {
    Language    = AllocatePool (4);
    Language[0] = 'e';
    Language[1] = 'n';
    Language[2] = 'g';
    Language[3] = 0;
  }

  Arg = NULL;
  //
  // Crack args
  //
  BestName = TRUE;
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

      case 'd':
      case 'D':
        BestName = FALSE;
        break;

      case 'b':
      case 'B':
        EnablePageBreak (DEFAULT_INIT_ROW, DEFAULT_AUTO_LF);
        break;

      case '?':
        PrtHelp = TRUE;
        break;

      default:
        PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_UNKNOWN_FLAG), HiiHandle, L"devtree", Ptr);
        Status = EFI_INVALID_PARAMETER;
        goto Done;
      }

      continue;
    }

    if (!Arg) {
      Arg = Ptr;
      continue;
    }

    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiHandle, L"devtree");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  if (PrtHelp) {
    PrintToken (STRING_TOKEN (STR_DEVTREE_VERBOSEHELP), HiiHandle);
    Status = EFI_SUCCESS;
    goto Done;
  }
  //
  // Load handle & protocol info tables
  //
  ShellInitProtocolInfoEnumerator ();

  if (Arg) {
    //
    // Dump 1 handle
    //
    Index = ShellHandleNoFromStr (Arg) - 1;
    if (Index < 0) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_DEVTREE_INVALID_HANDLE), HiiHandle, Arg);
    } else {
      PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_DEVICE_TREE), HiiHandle);
      ShellDeviceTree (
        ShellHandleFromIndex (Index),
        1,
        FALSE,
        BestName,
        Language
        );
    }
  } else {
    //
    // Dump all handles
    //
    PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_DEVICE_TREE), HiiHandle);
    for (Index = 0; Index < ShellGetHandleNum (); Index++) {
      if (GetExecutionBreak ()) {
        Status = EFI_ABORTED;
        goto Done;
      }

      ShellDeviceTree (
        ShellHandleFromIndex (Index),
        1,
        TRUE,
        BestName,
        Language
        );
    }
  }

  Status = EFI_SUCCESS;

Done:
  return Status;
}
