/*++

Copyright (c) 2005 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  unload.c

Abstract:

  EFI Shell command "load"


Revision History

--*/

#include "EfiShellLib.h"
#include "unload.h"

extern UINT8  STRING_ARRAY_NAME[];

//
// This is the generated header file which includes whatever needs to be exported (strings + IFR)
//
#include STRING_DEFINES_FILE

//
// Global Variables
//
EFI_HII_HANDLE  HiiUnloadHandle;
EFI_GUID        EfiUnloadGuid = EFI_UNLOAD_GUID;
SHELL_VAR_CHECK_ITEM      UnloadCheckList[] = {
  {
    L"-n",
    0x01,
    0,
    FlagTypeSingle
  },
  {
    L"-v",
    0x02,
    0,
    FlagTypeSingle
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
_UnloadGetDriverName (
  EFI_HANDLE                        DriverBindingHandle,
  UINT8                             *Language,
  BOOLEAN                           ImageName,
  CHAR16                            **DriverName
  )

{
  EFI_STATUS                  Status;
  EFI_DRIVER_BINDING_PROTOCOL *DriverBinding;
  EFI_LOADED_IMAGE_PROTOCOL   *Image;
  EFI_COMPONENT_NAME_PROTOCOL *ComponentName;

  *DriverName = NULL;

  Status = BS->OpenProtocol (
                DriverBindingHandle,
                &gEfiDriverBindingProtocolGuid,
                (VOID **) &DriverBinding,
                NULL,
                NULL,
                EFI_OPEN_PROTOCOL_GET_PROTOCOL
                );
  if (EFI_ERROR (Status)) {
    return EFI_SUCCESS;
  }

  if (ImageName) {
    Status = BS->OpenProtocol (
                  DriverBinding->ImageHandle,
                  &gEfiLoadedImageProtocolGuid,
                  (VOID **) &Image,
                  NULL,
                  NULL,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
    if (!EFI_ERROR (Status)) {
      *DriverName = LibDevicePathToStr (Image->FilePath);
    }
  } else {
    Status = BS->OpenProtocol (
                  DriverBindingHandle,
                  &gEfiComponentNameProtocolGuid,
                  (VOID **) &ComponentName,
                  NULL,
                  NULL,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
    if (!EFI_ERROR (Status)) {
      //
      // Make sure the interface has been implemented
      //
      if (ComponentName->GetDriverName != NULL) {
        Status = ComponentName->GetDriverName (
                                  ComponentName,
                                  Language,
                                  DriverName
                                  );
      }
    }
  }

  return EFI_SUCCESS;
}

EFI_STATUS
_UnloadDisplayDriverModelHandle (
  IN EFI_HANDLE                     Handle,
  IN BOOLEAN                        BestName,
  IN CHAR8                          *Language
  )
/*++

Routine Description:

Arguments:
  Handle   - The handle
  BestName - The best name
  Language - The language

Returns:

--*/
{
  EFI_STATUS                  Status;
  EFI_STATUS                  ConfigurationStatus;
  EFI_STATUS                  DiagnosticsStatus;
  UINTN                       DriverBindingHandleCount;
  EFI_HANDLE                  *DriverBindingHandleBuffer;
  UINTN                       ParentControllerHandleCount;
  EFI_HANDLE                  *ParentControllerHandleBuffer;
  UINTN                       ChildControllerHandleCount;
  EFI_HANDLE                  *ChildControllerHandleBuffer;
  CHAR16                      *BestDeviceName;
  EFI_DEVICE_PATH_PROTOCOL    *DevicePath;
  UINTN                       Index;
  CHAR16                      *DriverName;
  EFI_DRIVER_BINDING_PROTOCOL *DriverBinding;
  EFI_COMPONENT_NAME_PROTOCOL *ComponentName;
  UINTN                       NumberOfChildren;
  UINTN                       HandleIndex;
  UINTN                       ControllerHandleCount;
  EFI_HANDLE                  *ControllerHandleBuffer;
  UINTN                       ChildIndex;
  BOOLEAN                     Image;

  //
  // See if Handle is a device handle and display its details.
  //
  DriverBindingHandleBuffer = NULL;
  Status = LibGetManagingDriverBindingHandles (
            Handle,
            &DriverBindingHandleCount,
            &DriverBindingHandleBuffer
            );

  ParentControllerHandleBuffer = NULL;
  Status = LibGetParentControllerHandles (
            Handle,
            &ParentControllerHandleCount,
            &ParentControllerHandleBuffer
            );

  ChildControllerHandleBuffer = NULL;
  Status = LibGetChildControllerHandles (
            Handle,
            &ChildControllerHandleCount,
            &ChildControllerHandleBuffer
            );

  if (DriverBindingHandleCount > 0 || ParentControllerHandleCount > 0 || ChildControllerHandleCount > 0) {

    DevicePath      = NULL;
    BestDeviceName  = NULL;
    Status          = BS->HandleProtocol (Handle, &gEfiDevicePathProtocolGuid, &DevicePath);

    Print (L"\n");
    PrintToken (STRING_TOKEN (STR_UNLOAD_CONTROLLER_NAME), HiiUnloadHandle);
    Status = ShellGetDeviceName (
              Handle,
              TRUE,
              FALSE,
              Language,
              &BestDeviceName,
              &ConfigurationStatus,
              &DiagnosticsStatus,
              TRUE,
              26
              );

    if (BestDeviceName == NULL) {
      PrintToken (STRING_TOKEN (STR_UNLOAD_UNKNOWN_2), HiiUnloadHandle);
    } else {
      FreePool (BestDeviceName);
      BestDeviceName = NULL;
    }

    Print (L"\n");

    if (DevicePath != NULL) {
      PrintToken (STRING_TOKEN (STR_UNLOAD_DEV_PAATH), HiiUnloadHandle, LibDevicePathToStr (DevicePath));
    } else {
      PrintToken (STRING_TOKEN (STR_UNLOAD_DEV_PATH_2), HiiUnloadHandle);
    }

    PrintToken (STRING_TOKEN (STR_UNLOAD_CONTROLLER_TYPE), HiiUnloadHandle);
    if (ParentControllerHandleCount == 0) {
      PrintToken (STRING_TOKEN (STR_UNLOAD_ROOT), HiiUnloadHandle);
    } else if (ChildControllerHandleCount > 0) {
      PrintToken (STRING_TOKEN (STR_UNLOAD_HBUS), HiiUnloadHandle);
    } else {
      PrintToken (STRING_TOKEN (STR_UNLOAD_HDEVICE), HiiUnloadHandle);
    }

    if (!EFI_ERROR (ConfigurationStatus)) {
      PrintToken (STRING_TOKEN (STR_UNLOAD_CONFIG_HYES), HiiUnloadHandle);
    } else {
      PrintToken (STRING_TOKEN (STR_UNLOAD_CONFIG_HNO), HiiUnloadHandle);
    }

    if (!EFI_ERROR (DiagnosticsStatus)) {
      PrintToken (STRING_TOKEN (STR_UNLOAD_DIAG_HYES), HiiUnloadHandle);
    } else {
      PrintToken (STRING_TOKEN (STR_UNLOAD_DIAG_HNO), HiiUnloadHandle);
    }

    if (DriverBindingHandleCount != 0) {
      PrintToken (STRING_TOKEN (STR_UNLOAD_MANAGED_BY), HiiUnloadHandle);
      for (Index = 0; Index < DriverBindingHandleCount; Index++) {
        Image = FALSE;
        Status = _UnloadGetDriverName (
                  DriverBindingHandleBuffer[Index],
                  Language,
                  FALSE,
                  &DriverName
                  );
        if (DriverName == NULL) {
          Status = _UnloadGetDriverName (
                    DriverBindingHandleBuffer[Index],
                    Language,
                    TRUE,
                    &DriverName
                    );
          if (!EFI_ERROR (Status)) {
            Image = TRUE;
          }
        }

        if (DriverName == NULL) {
          DriverName = L"<UNKNOWN>";
        }

        if (Image) {
          PrintToken (
            STRING_TOKEN (STR_UNLOAD_DRV_IMAGE),
            HiiUnloadHandle,
            ShellHandleToIndex (DriverBindingHandleBuffer[Index]),
            DriverName
            );
        } else {
          PrintToken (
            STRING_TOKEN (STR_UNLOAD_DRV_HS),
            HiiUnloadHandle,
            ShellHandleToIndex (DriverBindingHandleBuffer[Index]),
            DriverName
            );
        }
      }
    } else {
      PrintToken (STRING_TOKEN (STR_UNLOAD_MANAGED_BY_NONE), HiiUnloadHandle);
    }

    if (ParentControllerHandleCount != 0) {
      PrintToken (STRING_TOKEN (STR_UNLOAD_PARENT_CONTROLLERS), HiiUnloadHandle);
      for (Index = 0; Index < ParentControllerHandleCount; Index++) {
        Status = ShellGetDeviceName (
                  ParentControllerHandleBuffer[Index],
                  BestName,
                  TRUE,
                  Language,
                  &BestDeviceName,
                  &ConfigurationStatus,
                  &DiagnosticsStatus,
                  FALSE,
                  0
                  );
        if (BestDeviceName == NULL) {
          BestDeviceName = StrDuplicate (L"<UNKNOWN>");
        }

        PrintToken (
          STRING_TOKEN (STR_UNLOAD_PARENT),
          HiiUnloadHandle,
          ShellHandleToIndex (ParentControllerHandleBuffer[Index]),
          BestDeviceName
          );
        FreePool (BestDeviceName);
        BestDeviceName = NULL;
      }
    } else {
      PrintToken (STRING_TOKEN (STR_UNLOAD_PARENT_CONT_NONE), HiiUnloadHandle);
    }

    if (ChildControllerHandleCount != 0) {
      PrintToken (STRING_TOKEN (STR_UNLOAD_CHILD_CONTROL), HiiUnloadHandle);
      for (Index = 0; Index < ChildControllerHandleCount; Index++) {
        Status = ShellGetDeviceName (
                  ChildControllerHandleBuffer[Index],
                  BestName,
                  TRUE,
                  Language,
                  &BestDeviceName,
                  &ConfigurationStatus,
                  &DiagnosticsStatus,
                  FALSE,
                  0
                  );
        if (BestDeviceName == NULL) {
          BestDeviceName = StrDuplicate (L"<UNKNOWN>");
        }

        PrintToken (
          STRING_TOKEN (STR_UNLOAD_CHILD),
          HiiUnloadHandle,
          ShellHandleToIndex (ChildControllerHandleBuffer[Index]),
          BestDeviceName
          );
        FreePool (BestDeviceName);
        BestDeviceName = NULL;
      }
    } else {
      PrintToken (STRING_TOKEN (STR_UNLOAD_CHILD_CONTROL_NONE), HiiUnloadHandle);
    }
  }

  Status = EFI_SUCCESS;

  if (DriverBindingHandleBuffer) {
    FreePool (DriverBindingHandleBuffer);
  }

  if (ParentControllerHandleBuffer) {
    FreePool (ParentControllerHandleBuffer);
  }

  if (ChildControllerHandleBuffer) {
    FreePool (ChildControllerHandleBuffer);
  }

  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // See if Handle is a driver binding handle and display its details.
  //
  Status = BS->OpenProtocol (
                Handle,
                &gEfiDriverBindingProtocolGuid,
                (VOID **) &DriverBinding,
                NULL,
                NULL,
                EFI_OPEN_PROTOCOL_GET_PROTOCOL
                );
  if (EFI_ERROR (Status)) {
    return EFI_SUCCESS;
  }

  ComponentName = NULL;
  Status = BS->OpenProtocol (
                Handle,
                &gEfiComponentNameProtocolGuid,
                (VOID **) &ComponentName,
                NULL,
                NULL,
                EFI_OPEN_PROTOCOL_GET_PROTOCOL
                );

  DiagnosticsStatus = BS->OpenProtocol (
                            Handle,
                            &gEfiDriverDiagnosticsProtocolGuid,
                            NULL,
                            NULL,
                            NULL,
                            EFI_OPEN_PROTOCOL_TEST_PROTOCOL
                            );

  ConfigurationStatus = BS->OpenProtocol (
                              Handle,
                              &gEfiDriverConfigurationProtocolGuid,
                              NULL,
                              NULL,
                              NULL,
                              EFI_OPEN_PROTOCOL_TEST_PROTOCOL
                              );

  NumberOfChildren        = 0;
  ControllerHandleBuffer  = NULL;
  Status = LibGetManagedControllerHandles (
            Handle,
            &ControllerHandleCount,
            &ControllerHandleBuffer
            );
  if (ControllerHandleCount > 0) {
    for (HandleIndex = 0; HandleIndex < ControllerHandleCount; HandleIndex++) {
      Status = LibGetManagedChildControllerHandles (
                Handle,
                ControllerHandleBuffer[HandleIndex],
                &ChildControllerHandleCount,
                NULL
                );
      NumberOfChildren += ChildControllerHandleCount;
    }
  }

  Status = _UnloadGetDriverName (
            Handle,
            Language,
            FALSE,
            &DriverName
            );
  Print (L"\n");

  if (DriverName != NULL) {
    PrintToken (STRING_TOKEN (STR_UNLOAD_DRIVER_NAME), HiiUnloadHandle, DriverName);
  } else {
    PrintToken (STRING_TOKEN (STR_UNLOAD_DRIVER_NAME_NONE), HiiUnloadHandle);
  }

  Status = _UnloadGetDriverName (
            Handle,
            Language,
            TRUE,
            &DriverName
            );
  if (DriverName != NULL) {
    PrintToken (STRING_TOKEN (STR_UNLOAD_IAMGE_NAME), HiiUnloadHandle, DriverName);
  } else {
    PrintToken (STRING_TOKEN (STR_UNLOAD_IMAGE_NAME_NONE), HiiUnloadHandle);
  }

  PrintToken (STRING_TOKEN (STR_UNLOAD_DRIVER_VERSION), HiiUnloadHandle, DriverBinding->Version);

  if (NumberOfChildren > 0) {
    PrintToken (STRING_TOKEN (STR_UNLOAD_DRIVER_TYPE_HBUS), HiiUnloadHandle);
  } else if (ControllerHandleCount > 0) {
    PrintToken (STRING_TOKEN (STR_UNLOAD_DRIVER_TYPE_HDEVICE), HiiUnloadHandle);
  } else {
    PrintToken (STRING_TOKEN (STR_UNLOAD_DRIVER_TYPE_UNKNOWN), HiiUnloadHandle);
  }

  if (!EFI_ERROR (ConfigurationStatus)) {
    PrintToken (STRING_TOKEN (STR_UNLOAD_CONFIG_HYES_2), HiiUnloadHandle);
  } else {
    PrintToken (STRING_TOKEN (STR_UNLOAD_CONFIG_HNO_2), HiiUnloadHandle);
  }

  if (!EFI_ERROR (DiagnosticsStatus)) {
    PrintToken (STRING_TOKEN (STR_UNLOAD_DIAGNOSTICS_HYES), HiiUnloadHandle);
  } else {
    PrintToken (STRING_TOKEN (STR_UNLOAD_DIAGNOSTICS_HNO), HiiUnloadHandle);
  }

  if (ControllerHandleCount == 0) {
    PrintToken (STRING_TOKEN (STR_UNLOAD_MANAGING_NONE), HiiUnloadHandle);
  } else {
    PrintToken (STRING_TOKEN (STR_UNLOAD_MANAGING), HiiUnloadHandle);
    for (HandleIndex = 0; HandleIndex < ControllerHandleCount; HandleIndex++) {
      Status = ShellGetDeviceName (
                ControllerHandleBuffer[HandleIndex],
                BestName,
                TRUE,
                Language,
                &BestDeviceName,
                &ConfigurationStatus,
                &DiagnosticsStatus,
                FALSE,
                0
                );
      if (BestDeviceName == NULL) {
        BestDeviceName = StrDuplicate (L"<UNKNOWN>");
      }

      PrintToken (
        STRING_TOKEN (STR_UNLOAD_CTRL_HS),
        HiiUnloadHandle,
        ShellHandleToIndex (ControllerHandleBuffer[HandleIndex]),
        BestDeviceName
        );
      FreePool (BestDeviceName);
      BestDeviceName = NULL;

      Status = LibGetManagedChildControllerHandles (
                Handle,
                ControllerHandleBuffer[HandleIndex],
                &ChildControllerHandleCount,
                &ChildControllerHandleBuffer
                );
      if (!EFI_ERROR (Status)) {
        for (ChildIndex = 0; ChildIndex < ChildControllerHandleCount; ChildIndex++) {
          Status = ShellGetDeviceName (
                    ChildControllerHandleBuffer[ChildIndex],
                    BestName,
                    TRUE,
                    Language,
                    &BestDeviceName,
                    &ConfigurationStatus,
                    &DiagnosticsStatus,
                    FALSE,
                    0
                    );
          if (BestDeviceName == NULL) {
            BestDeviceName = StrDuplicate (L"<UNKNOWN>");
          }

          PrintToken (
            STRING_TOKEN (STR_UNLOAD_CHILD_HS),
            HiiUnloadHandle,
            ShellHandleToIndex (ChildControllerHandleBuffer[ChildIndex]),
            BestDeviceName
            );
          FreePool (BestDeviceName);
          BestDeviceName = NULL;
        }

        FreePool (ChildControllerHandleBuffer);
      }
    }

    FreePool (ControllerHandleBuffer);
  }

  return EFI_SUCCESS;
}

EFI_STATUS
_DHProt (
  IN BOOLEAN                  Verbose,
  IN BOOLEAN                  DriverModel,
  IN UINTN                    HandleNo,
  IN EFI_HANDLE               Handle,
  IN CHAR8                    *Language
  )
/*++

Routine Description:

Arguments:

  Verbose - The verbose
  DriverModel - The driver model
  HandleNo - The handle number
  Handle - The handle
  Language - The language

Returns:
  EFI_SUCCESS  Success

--*/
{
  PROTOCOL_INFO               *Prot;
  VOID                        *Interface;
  UINTN                       Index;
  UINTN                       Index1;
  EFI_STATUS                  Status;
  SHELLENV_DUMP_PROTOCOL_INFO Dump;
  EFI_GUID                    **ProtocolBuffer;
  UINTN                       ProtocolBufferCount;

  if (!HandleNo) {
    HandleNo = ShellHandleToIndex (Handle);
  }
  //
  // Get protocol info by handle
  //
  ProtocolBuffer = NULL;
  Status = BS->ProtocolsPerHandle (
                Handle,
                &ProtocolBuffer,
                &ProtocolBufferCount
                );
  if (EFI_ERROR (Status)) {
    return EFI_SUCCESS;
  }

  if (Verbose) {
    PrintToken (STRING_TOKEN (STR_UNLOAD_NHANDLE), HiiUnloadHandle, HandleNo, Handle);
  } else {
    PrintToken (STRING_TOKEN (STR_UNLOAD_TWO_VARS_N), HiiUnloadHandle, HandleNo, Handle);
  }

  ShellInitProtocolInfoEnumerator ();
  for (;;) {
    Status = ShellNextProtocolInfo (&Prot);
    if (NULL == Prot || EFI_ERROR (Status)) {
      break;
    }

    for (Index = 0; Index < Prot->NoHandles; Index++) {
      //
      // If this handle supports this protocol, dump it
      //
      if (GetExecutionBreak ()) {
        Status = EFI_ABORTED;
        goto Done;
      }

      if (Prot->Handles[Index] == Handle) {
        Dump    = Verbose ? Prot->DumpInfo : Prot->DumpToken;
        Status  = BS->HandleProtocol (Handle, &Prot->ProtocolId, &Interface);
        if (!EFI_ERROR (Status)) {
          if (Verbose) {
            for (Index1 = 0; Index1 < ProtocolBufferCount; Index1++) {
              if (ProtocolBuffer[Index1] != NULL) {
                if (CompareGuid (ProtocolBuffer[Index1], &Prot->ProtocolId) == 0) {
                  ProtocolBuffer[Index1] = NULL;
                }
              }
            }
            //
            // Dump verbose info
            //
            PrintToken (STRING_TOKEN (STR_UNLOAD_TWO_VARS_HS), HiiUnloadHandle, Prot->IdString, Interface);
            if (Dump != NULL) {
              Dump (Handle, Interface);
            }

            Print (L"\n");
          } else {
            if (Dump != NULL) {
              Dump (Handle, Interface);
            } else {
              PrintToken (STRING_TOKEN (STR_UNLOAD_ONE_VAR_HS), HiiUnloadHandle, Prot->IdString);
            }
          }
        }
      }
    }
  }

  if (DriverModel) {
    Status = _UnloadDisplayDriverModelHandle (Handle, TRUE, Language);
    if (EFI_ERROR (Status)) {
      goto Done;
    }
  }
  //
  // Dump verbose info
  //
  if (Verbose) {
    for (Index1 = 0; Index1 < ProtocolBufferCount; Index1++) {
      if (ProtocolBuffer[Index1] != NULL) {
        if (GetExecutionBreak ()) {
          Status = EFI_ABORTED;
          goto Done;
        }

        Status = BS->HandleProtocol (Handle, ProtocolBuffer[Index1], &Interface);
        if (!EFI_ERROR (Status)) {
          PrintToken (STRING_TOKEN (STR_UNLOAD_TWO_VARS_HG_NEW), HiiUnloadHandle, ProtocolBuffer[Index1], Interface);
        }
      }
    }

    Print (L"%N");
  } else {
    Print (L"%N\n");
  }

  Status = EFI_SUCCESS;

Done:
  if (ProtocolBuffer != NULL) {
    FreePool (ProtocolBuffer);
  }
  
  ShellCloseProtocolInfoEnumerator ();
  return Status;
}

EFI_STATUS
EFIAPI
InitializeUnload (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  );

EFI_BOOTSHELL_CODE(
  EFI_APPLICATION_ENTRY_POINT(InitializeUnload)
)

EFI_STATUS
EFIAPI
InitializeUnload (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  )
/*++

Routine Description:

  Code for internal "Unload" command

Arguments:
  ImageHandle  - The image handle
  SystemTable  - The system table

Returns:
  EFI_ABORTED  - Aborted

--*/
{
  BOOLEAN                   Prompt;
  BOOLEAN                   Verbose;
  CHAR16                    *Arg;
  EFI_STATUS                Status;
  UINTN                     Index;
  CHAR16                    ReturnStr[2];
  EFI_LOADED_IMAGE_PROTOCOL *ImageInfo;
  EFI_HANDLE                TargetHandle;
  SHELL_VAR_CHECK_CODE      RetCode;
  UINTN                     ItemValue;
  CHAR16                    *Useful;
  SHELL_VAR_CHECK_PACKAGE   ChkPck;
  //
  // Initializing variable to aVOID level 4 warning
  //
  ZeroMem (&ChkPck, sizeof (SHELL_VAR_CHECK_PACKAGE));
  //
  // Initialize
  //
  EFI_SHELL_APP_INIT (ImageHandle, SystemTable);
  
  //
  // Enable tab key which can pause the output
  //
  EnableOutputTabPause();

  Status = LibInitializeStrings (&HiiUnloadHandle, STRING_ARRAY_NAME, &EfiUnloadGuid);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (!EFI_PROPER_VERSION (1, 10)) {
    PrintToken (
      STRING_TOKEN (STR_SHELLENV_GNC_COMMAND_NOT_SUPPORT),
      HiiUnloadHandle,
      L"unload",
      EFI_VERSION_1_10
      );
    Status = EFI_UNSUPPORTED;
    goto Done;
  }

  Prompt  = TRUE;
  Verbose = FALSE;

  LibFilterNullArgs ();
  Useful  = NULL;
  RetCode = LibCheckVariables (SI, UnloadCheckList, &ChkPck, &Useful);
  if (VarCheckOk != RetCode) {
    switch (RetCode) {
    case VarCheckDuplicate:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_DUP_FLAG), HiiUnloadHandle, L"unload", Useful);
      break;

    case VarCheckUnknown:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_UNKNOWN_FLAG), HiiUnloadHandle, L"unload", Useful);
      break;

    default:
      break;
    }

    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }
  //
  // Crack args
  //
  if (LibCheckVarGetFlag (&ChkPck, L"-b") != NULL) {
    EnablePageBreak (DEFAULT_INIT_ROW, DEFAULT_AUTO_LF);
  }

  if (LibCheckVarGetFlag (&ChkPck, L"-?") != NULL) {
    if (ChkPck.ValueCount > 0 ||
        ChkPck.FlagCount > 2 ||
        (2 == ChkPck.FlagCount && !LibCheckVarGetFlag (&ChkPck, L"-b"))
        ) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiUnloadHandle, L"unload");
      Status = EFI_INVALID_PARAMETER;
    } else {
      if (IS_OLD_SHELL) {
        PrintToken (STRING_TOKEN (STR_NO_HELP), HiiUnloadHandle);
      } else {
        PrintToken (STRING_TOKEN (STR_UNLOAD_VERBOSE_HELP), HiiUnloadHandle);
      }

      Status = EFI_SUCCESS;
    }

    goto Done;
  }

  if (ChkPck.ValueCount < 1) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_FEW), HiiUnloadHandle, L"unload");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  if (ChkPck.ValueCount > 1) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiUnloadHandle, L"unload");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  if (LibCheckVarGetFlag (&ChkPck, L"-n") != NULL) {
    Prompt = FALSE;
  }

  if (LibCheckVarGetFlag (&ChkPck, L"-v") != NULL) {
    Verbose = TRUE;
  }

  Arg       = ChkPck.VarList->VarStr;

  ItemValue = (UINTN) StrToUIntegerBase (Arg, 16, &Status);
  if (EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_ARG), HiiUnloadHandle, L"unload", Arg);
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }
  //
  // Load handle & protocol info tables
  //
  ShellInitProtocolInfoEnumerator ();

  Index = ShellHandleNoFromIndex (ItemValue) - 1;

  if (Index > ShellGetHandleNum ()) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_HANDLE_OUT_OF_RANGE), HiiUnloadHandle, L"unload", Arg);
    Status = EFI_INVALID_PARAMETER;
  } else {
    TargetHandle = ShellHandleFromIndex (Index);
    _DHProt (Verbose, FALSE, Index + 1, TargetHandle, "eng");
    //
    // Make sure it is an image handle to a protocol
    //
    Status = BS->HandleProtocol (
                  TargetHandle,
                  &gEfiLoadedImageProtocolGuid,
                  (VOID **) &ImageInfo
                  );
    if (EFI_ERROR (Status)) {
      PrintToken (STRING_TOKEN (STR_UNLOAD_LOCATE_LOADIMAGEPROT), HiiUnloadHandle, Status);
    } else if ((ImageInfo->ImageCodeType != EfiBootServicesCode) && (ImageInfo->ImageCodeType != EfiRuntimeServicesCode)) {
      PrintToken (STRING_TOKEN (STR_UNLOAD_HANDLE_INDEX), HiiUnloadHandle);
    } else {
      //
      // Ask permission if needed
      //
      if (Prompt) {
        Input (
          L"Unload driver image (y/n)? ",
          ReturnStr,
          sizeof (ReturnStr) / sizeof (CHAR16)
          );
        Print (L"\n");
        if (ReturnStr[0] != L'y' && ReturnStr[0] != L'Y') {
          Status = EFI_ABORTED;
          goto Done;
        }
      }

      Status = BS->UnloadImage (TargetHandle);
      if (EFI_ERROR (Status)) {
        PrintToken (STRING_TOKEN (STR_UNLOAD_PROTOCOL_IMAGE_ERROR), HiiUnloadHandle, Status);
      } else {
        PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_SUCCESS), HiiUnloadHandle, L"unload");
      }
    }
  }

Done:
  ShellCloseProtocolInfoEnumerator ();
  LibCheckVarFreeVarList (&ChkPck);
  LibUnInitializeStrings ();
  return Status;
}

EFI_STATUS
EFIAPI
InitializeUnloadGetLineHelp (
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
  return LibCmdGetStringByToken (STRING_ARRAY_NAME, &EfiUnloadGuid, STRING_TOKEN (STR_UNLOAD_LINE_HELP), Str);
}
