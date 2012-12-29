/*++
 
Copyright (c) 2005 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  drvdiag.c
  
Abstract:

  Shell command "drvdiag"



Revision History

--*/

#include "EfiShellLib.h"
#include "drvdiag.h"

#include EFI_PROTOCOL_DEFINITION (ComponentName)
#include EFI_PROTOCOL_DEFINITION (DriverConfiguration)
#include EFI_PROTOCOL_DEFINITION (DriverDiagnostics)
#include EFI_PROTOCOL_DEFINITION (DriverBinding)

extern UINT8  STRING_ARRAY_NAME[];

//
// This is the generated header file which includes whatever needs to be exported (strings + IFR)
//
#include STRING_DEFINES_FILE

//
// Global Variables
//
EFI_HII_HANDLE  HiiHandle;
EFI_GUID        EfiDrvdiagGuid = EFI_DRVDIAG_GUID;
SHELL_VAR_CHECK_ITEM  DrvdiagCheckList[] = {
  {
    L"-l",
    0x01,
    0,
    FlagTypeNeedVar
  },
  {
    L"-c",
    0x02,
    0,
    FlagTypeSingle
  },
  {
    L"-s",
    0x04,
    0x18,
    FlagTypeSingle
  },
  {
    L"-e",
    0x08,
    0x14,
    FlagTypeSingle
  },
  {
    L"-m",
    0x10,
    0x0c,
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
  EFI_APPLICATION_ENTRY_POINT(DrvdiagMain)
)

EFI_STATUS
SEnvCmdDriverDiagnostics (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  );

//
//
//
EFI_STATUS
ShellCmdDriverRunDiagnostics (
  EFI_DRIVER_DIAGNOSTICS_PROTOCOL   *DriverDiagnostics,
  EFI_HANDLE                        ControllerHandle,
  EFI_HANDLE                        ChildHandle,
  EFI_DRIVER_DIAGNOSTIC_TYPE        DiagnosticType,
  CHAR8                             *Language
  );

EFI_STATUS
EFIAPI
DrvdiagMain (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  );

//
//
//
EFI_STATUS
EFIAPI
DrvdiagMain (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  )
/*++

Routine Description:

  Code for external shell "drvdiag" command.

Arguments:

 ImageHandle - The image handle
 SystemTable - The system table

Returns:

--*/
{
  EFI_STATUS                      Status;
  UINTN                           HandleNumber;
  EFI_HANDLE                      DriverImageHandle;
  EFI_HANDLE                      DeviceHandle;
  EFI_HANDLE                      ChildHandle;
  UINTN                           StringIndex;
  UINTN                           Index;
  CHAR8                           *Language;
  CHAR8                           *SupportedLanguages;
  EFI_DRIVER_DIAGNOSTIC_TYPE      DiagnosticType;
  EFI_DRIVER_DIAGNOSTICS_PROTOCOL *DriverDiagnostics;
  UINTN                           DriverImageHandleCount;
  EFI_HANDLE                      *DriverImageHandleBuffer;
  BOOLEAN                         RunDiagnostics;
  BOOLEAN                         DiagnoseAllChildren;
  BOOLEAN                         Found;
  UINTN                           HandleCount;
  EFI_HANDLE                      *HandleBuffer;
  UINT32                          *HandleType;
  UINTN                           HandleIndex;
  UINTN                           ChildIndex;
  UINTN                           ChildHandleCount;
  EFI_HANDLE                      *ChildHandleBuffer;
  UINT32                          *ChildHandleType;
  SHELL_VAR_CHECK_CODE            RetCode;
  CHAR16                          *Useful;
  SHELL_ARG_LIST                  *Item;
  SHELL_VAR_CHECK_PACKAGE         ChkPck;

  Language                = NULL;
  DriverImageHandle       = NULL;
  DeviceHandle            = NULL;
  ChildHandle             = NULL;
  RunDiagnostics          = FALSE;
  DiagnosticType          = EfiDriverDiagnosticTypeStandard;
  DiagnoseAllChildren     = FALSE;
  Found                   = FALSE;
  DriverImageHandleCount  = 0;
  DriverImageHandleBuffer = NULL;
  ZeroMem (&ChkPck, sizeof (SHELL_VAR_CHECK_PACKAGE));

  EFI_SHELL_APP_INIT (ImageHandle, SystemTable);

  //
  // Enable tab key which can pause the output
  //
  EnableOutputTabPause();
  
  Status = LibInitializeStrings (&HiiHandle, STRING_ARRAY_NAME, &EfiDrvdiagGuid);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (!EFI_PROPER_VERSION (1, 10)) {
    PrintToken (
      STRING_TOKEN (STR_SHELLENV_GNC_COMMAND_NOT_SUPPORT),
      HiiHandle,
      L"drvdiag",
      EFI_VERSION_1_10
      );
    Status = EFI_UNSUPPORTED;
    goto Done;
  }

  ShellInitHandleEnumerator ();
  ShellInitProtocolInfoEnumerator ();
  if (IS_OLD_SHELL) {
    Status = SEnvCmdDriverDiagnostics (ImageHandle, SystemTable);
    goto Done;
  }

  RetCode = LibCheckVariables (SI, DrvdiagCheckList, &ChkPck, &Useful);
  if (VarCheckOk != RetCode) {
    switch (RetCode) {
    case VarCheckDuplicate:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_DUP_FLAG), HiiHandle, L"drvdiag", Useful);
      break;

    case VarCheckUnknown:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_UNKNOWN_FLAG), HiiHandle, L"drvdiag", Useful);
      break;

    case VarCheckLackValue:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_LACK_ARG), HiiHandle, L"drvdiag", Useful);
      break;

    case VarCheckConflict:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_FLAG_CONFLICT), HiiHandle, L"drvdiag", Useful);
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
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiHandle, L"drvdiag");
      Status = EFI_INVALID_PARAMETER;
    } else {
      PrintToken (STRING_TOKEN (STR_DRVDIAG_VERBOSE_HELP), HiiHandle);
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
      PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_DRVDIAG_BAD_LANG), HiiHandle, L"drvdiag", Item->VarStr);
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }

    for (StringIndex = 0; StringIndex < 3; StringIndex++) {
      Language[StringIndex] = (CHAR8) Item->VarStr[StringIndex];
    }

    Language[StringIndex] = 0;
  }

  if (LibCheckVarGetFlag (&ChkPck, L"-c")) {
    DiagnoseAllChildren = TRUE;
  }

  if (LibCheckVarGetFlag (&ChkPck, L"-s")) {
    DiagnosticType  = EfiDriverDiagnosticTypeStandard;
    RunDiagnostics  = TRUE;
  }

  if (LibCheckVarGetFlag (&ChkPck, L"-e")) {
    DiagnosticType  = EfiDriverDiagnosticTypeExtended;
    RunDiagnostics  = TRUE;
  }

  if (LibCheckVarGetFlag (&ChkPck, L"-m")) {
    DiagnosticType  = EfiDriverDiagnosticTypeManufacturing;
    RunDiagnostics  = TRUE;
  }

  if (ChkPck.ValueCount > 3) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiHandle, L"drvdiag");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  Item = ChkPck.VarList;
  if (Item != NULL) {
    HandleNumber = (UINTN) (StrToUIntegerBase (Item->VarStr, 16, &Status));
    if (EFI_ERROR (Status)) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_ARG), HiiHandle, L"drvdiag", Item->VarStr);
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }

    DriverImageHandle = ShellHandleFromIndex (HandleNumber - 1);
    if (DriverImageHandle == NULL) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_HANDLE_NUM), HiiHandle, L"drvdiag", Item->VarStr);
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }

    Item = Item->Next;
  }

  if (Item != NULL) {
    HandleNumber = (UINTN) (StrToUIntegerBase (Item->VarStr, 16, &Status));
    if (EFI_ERROR (Status)) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_ARG), HiiHandle, L"drvdiag", Item->VarStr);
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }

    DeviceHandle = ShellHandleFromIndex (HandleNumber - 1);
    if (DeviceHandle == NULL) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_HANDLE_NUM), HiiHandle, L"drvdiag", Item->VarStr);
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }

    Item = Item->Next;
  }

  if (Item != NULL) {
    HandleNumber = (UINTN) (StrToUIntegerBase (Item->VarStr, 16, &Status));
    if (EFI_ERROR (Status)) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_ARG), HiiHandle, L"drvdiag", Item->VarStr);
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }

    ChildHandle = ShellHandleFromIndex (HandleNumber - 1);
    if (ChildHandle == NULL) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_HANDLE_NUM), HiiHandle, L"drvdiag", Item->VarStr);
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }
  }

  if (RunDiagnostics) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_RUN_DIAGNOSTICS), HiiHandle);
  } else {
    PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_AVAIL_DIAGNOSTICS), HiiHandle);
  }
  //
  // Display all handles that support being diagnosed
  //
  if (DriverImageHandle == NULL) {
    Status = LibLocateHandle (
              ByProtocol,
              &gEfiDriverDiagnosticsProtocolGuid,
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

  Found = TRUE;
  for (Index = 0; Index < DriverImageHandleCount; Index++) {
    Status = BS->OpenProtocol (
                  DriverImageHandleBuffer[Index],
                  &gEfiDriverDiagnosticsProtocolGuid,
                  (VOID **) &DriverDiagnostics,
                  NULL,
                  NULL,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
    if (EFI_ERROR (Status)) {
      PrintToken (
        STRING_TOKEN (STR_SHELLENV_PROTID_DRVDIAG_HANDLE_NOT_SUPPORT_PROT),
        HiiHandle,
        ShellHandleToIndex (DriverImageHandleBuffer[Index])
        );
      continue;
    }

    if (!Found) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_DRVDIAG_NOT_FOUND), HiiHandle);
      Found = TRUE;
    };
    PrintToken (
      STRING_TOKEN (STR_SHELLENV_PROTID_DRVDIAG_DRIVER_HANDLE),
      HiiHandle,
      ShellHandleToIndex (DriverImageHandleBuffer[Index])
      );

    Status = EFI_NOT_FOUND;
    for (SupportedLanguages = DriverDiagnostics->SupportedLanguages;
         SupportedLanguages[0] != 0;
         SupportedLanguages += 3
        ) {
      if (CompareMem (SupportedLanguages, Language, 3) == 0) {
        Status = EFI_SUCCESS;
      }
    }

    if (EFI_ERROR (Status)) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_DRVDIAG_SUPPORT_LANG), HiiHandle, Language);
      continue;
    }

    Found = FALSE;
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

    for (HandleIndex = 0; HandleIndex < HandleCount; HandleIndex++) {

      if ((HandleType[HandleIndex] & EFI_HANDLE_TYPE_CONTROLLER_HANDLE) != EFI_HANDLE_TYPE_CONTROLLER_HANDLE) {
        continue;
      }

      if (DeviceHandle != NULL && DeviceHandle != HandleBuffer[HandleIndex]) {
        continue;
      }

      if (ChildHandle == NULL) {
        if (!Found) {
          Found = TRUE;
          PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_DRVDIAG_NEW_LINE), HiiHandle);
        }

        PrintToken (
          STRING_TOKEN (STR_SHELLENV_PROTID_DRV_CTRL_LANG),
          HiiHandle,
          ShellHandleToIndex (DriverImageHandleBuffer[Index]),
          ShellHandleToIndex (HandleBuffer[HandleIndex]),
          DriverDiagnostics->SupportedLanguages
          );

        if (RunDiagnostics) {

          ShellCmdDriverRunDiagnostics (
            DriverDiagnostics,
            HandleBuffer[HandleIndex],
            NULL,
            DiagnosticType,
            Language
            );

        } else {
          Print (L"\n");
        }
      }

      if (ChildHandle == NULL && !DiagnoseAllChildren) {
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

        if (!Found) {
          Found = TRUE;
          PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_DRVDIAG_NEW_LINE), HiiHandle);
        }

        PrintToken (
          STRING_TOKEN (STR_SHELLENV_PROTID_DRV_CTRL_CHILD_LANG),
          HiiHandle,
          ShellHandleToIndex (DriverImageHandleBuffer[Index]),
          ShellHandleToIndex (HandleBuffer[HandleIndex]),
          ShellHandleToIndex (ChildHandleBuffer[ChildIndex]),
          DriverDiagnostics->SupportedLanguages
          );

        if (RunDiagnostics) {

          ShellCmdDriverRunDiagnostics (
            DriverDiagnostics,
            HandleBuffer[HandleIndex],
            ChildHandleBuffer[ChildIndex],
            DiagnosticType,
            Language
            );

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
  if (!Found) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_DRVDIAG_NOT_FOUND), HiiHandle);
  }

  if (DriverImageHandle != NULL && DriverImageHandleCount != 0) {
    FreePool (DriverImageHandleBuffer);
  }

  if (Language) {
    FreePool (Language);
  }

  ShellCloseProtocolInfoEnumerator ();
  ShellCloseHandleEnumerator ();
  LibCheckVarFreeVarList (&ChkPck);
  LibUnInitializeStrings ();
  return Status;
}

EFI_STATUS
ShellCmdDriverRunDiagnostics (
  EFI_DRIVER_DIAGNOSTICS_PROTOCOL   *DriverDiagnostics,
  EFI_HANDLE                        ControllerHandle,
  EFI_HANDLE                        ChildHandle,
  EFI_DRIVER_DIAGNOSTIC_TYPE        DiagnosticType,
  CHAR8                             *Language
  )

{
  EFI_STATUS  Status;
  EFI_GUID    *ErrorType;
  UINTN       BufferSize;
  CHAR16      *Buffer;

  ErrorType   = NULL;
  BufferSize  = 0;
  Buffer      = NULL;
  Status = DriverDiagnostics->RunDiagnostics (
                                DriverDiagnostics,
                                ControllerHandle,
                                ChildHandle,
                                DiagnosticType,
                                Language,
                                &ErrorType,
                                &BufferSize,
                                &Buffer
                                );

  if (!EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_HPASSED), HiiHandle);
  } else {
    PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_HFAILED), HiiHandle, Status);
  }

  if (ErrorType != NULL) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_ERRORTYPE), HiiHandle, ErrorType);
  }

  if (BufferSize > 0) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_BUFFERSIZE), HiiHandle, BufferSize);
    PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_BUFFER_ONE_VAR), HiiHandle, Buffer);
    DumpHex (2, 0, BufferSize, Buffer);
    FreePool (Buffer);
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
DrvdiagGetLineHelp (
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
  return LibCmdGetStringByToken (STRING_ARRAY_NAME, &EfiDrvdiagGuid, STRING_TOKEN (STR_DRVDIAG_LINE_HELP), Str);
}
//
// Compatible Support
//
EFI_STATUS
SEnvCmdDriverDiagnostics (
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
  EFI_STATUS                      Status;
  CHAR16                          *Ptr;
  UINTN                           HandleNumber;
  EFI_HANDLE                      DriverImageHandle;
  EFI_HANDLE                      DeviceHandle;
  EFI_HANDLE                      ChildHandle;
  UINTN                           StringIndex;
  UINTN                           Index;
  CHAR8                           *Language;
  CHAR8                           *SupportedLanguages;
  EFI_DRIVER_DIAGNOSTIC_TYPE      DiagnosticType;
  EFI_DRIVER_DIAGNOSTICS_PROTOCOL *DriverDiagnostics;
  UINTN                           DriverImageHandleCount;
  EFI_HANDLE                      *DriverImageHandleBuffer;
  BOOLEAN                         RunDiagnostics;
  BOOLEAN                         DiagnoseAllChildren;
  UINTN                           HandleCount;
  EFI_HANDLE                      *HandleBuffer;
  UINT32                          *HandleType;
  UINTN                           HandleIndex;
  UINTN                           ChildIndex;
  UINTN                           ChildHandleCount;
  EFI_HANDLE                      *ChildHandleBuffer;
  UINT32                          *ChildHandleType;
  BOOLEAN                         GetHelp;

  //
  // Setup Handle and Protocol Globals
  //
  DriverImageHandle       = NULL;
  DeviceHandle            = NULL;
  ChildHandle             = NULL;
  RunDiagnostics          = FALSE;
  DiagnosticType          = EfiDriverDiagnosticTypeStandard;
  DiagnoseAllChildren     = FALSE;
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
        DiagnoseAllChildren = TRUE;
        break;

      case 's':
      case 'S':
        DiagnosticType  = EfiDriverDiagnosticTypeStandard;
        RunDiagnostics  = TRUE;
        break;

      case 'e':
      case 'E':
        DiagnosticType  = EfiDriverDiagnosticTypeExtended;
        RunDiagnostics  = TRUE;
        break;

      case 'm':
      case 'M':
        DiagnosticType  = EfiDriverDiagnosticTypeManufacturing;
        RunDiagnostics  = TRUE;
        break;

      case 'b':
      case 'B':
        EnablePageBreak (DEFAULT_INIT_ROW, DEFAULT_AUTO_LF);
        break;

      case '?':
        GetHelp = TRUE;
        break;

      default:
        PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_UNKNOWN_FLAG), HiiHandle, L"drvdiag", Ptr);
        Status = EFI_INVALID_PARAMETER;
        goto Done;
      }
    } else {
      HandleNumber = ShellHandleNoFromStr (Ptr);
      if (HandleNumber == 0) {
        PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_HANDLE_NUM), HiiHandle, L"drvdiag", Ptr);
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
        PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiHandle, L"drvdiag");
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

  if (RunDiagnostics) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_RUN_DIAGNOSTICS), HiiHandle);
  } else {
    PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_AVAIL_DIAGNOSTICS), HiiHandle);
  }
  //
  // Display all handles that support being diagnosed
  //
  if (DriverImageHandle == NULL) {
    Status = LibLocateHandle (
              ByProtocol,
              &gEfiDriverDiagnosticsProtocolGuid,
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

  for (Index = 0; Index < DriverImageHandleCount; Index++) {
    Status = BS->OpenProtocol (
                  DriverImageHandleBuffer[Index],
                  &gEfiDriverDiagnosticsProtocolGuid,
                  (VOID **) &DriverDiagnostics,
                  NULL,
                  NULL,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
    if (EFI_ERROR (Status)) {
      PrintToken (
        STRING_TOKEN (STR_SHELLENV_PROTID_DRVDIAG_HANDLE_NOT_SUPPORT_PROT),
        HiiHandle,
        ShellHandleToIndex (DriverImageHandleBuffer[Index])
        );
      continue;
    }

    Status = EFI_NOT_FOUND;
    for (SupportedLanguages = DriverDiagnostics->SupportedLanguages;
         SupportedLanguages[0] != 0;
         SupportedLanguages += 3
        ) {
      if (CompareMem (SupportedLanguages, Language, 3) == 0) {
        Status = EFI_SUCCESS;
      }
    }

    if (EFI_ERROR (Status)) {
      PrintToken (
        STRING_TOKEN (STR_SHELLENV_PROTID_DRVDIAG_HANDLE_NOT_SUPPORT_LANG),
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
          DriverDiagnostics->SupportedLanguages
          );

        if (RunDiagnostics) {

          ShellCmdDriverRunDiagnostics (
            DriverDiagnostics,
            HandleBuffer[HandleIndex],
            NULL,
            DiagnosticType,
            Language
            );

        } else {
          Print (L"\n");
        }
      }

      if (ChildHandle == NULL && !DiagnoseAllChildren) {
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
          DriverDiagnostics->SupportedLanguages
          );

        if (RunDiagnostics) {

          ShellCmdDriverRunDiagnostics (
            DriverDiagnostics,
            HandleBuffer[HandleIndex],
            ChildHandleBuffer[ChildIndex],
            DiagnosticType,
            Language
            );

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
