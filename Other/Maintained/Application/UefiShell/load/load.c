/*++

Copyright (c) 2005 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  load.c

Abstract:

  EFI Shell command "load"


Revision History

--*/

#include "EfiShellLib.h"
#include "load.h"

extern UINT8  STRING_ARRAY_NAME[];

//
// This is the generated header file which includes whatever needs to be exported (strings + IFR)
//
#include STRING_DEFINES_FILE

//
// Global Variables
//
EFI_HII_HANDLE  HiiLoadHandle;
EFI_GUID        EfiLoadGuid = EFI_LOAD_GUID;
SHELL_VAR_CHECK_ITEM    LoadCheckList[] = {
  {
    L"-nc",
    0x01,
    0,
    FlagTypeSingle
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

//
//
//
EFI_STATUS
EFIAPI
InitializeLoad (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  );

STATIC
EFI_STATUS
LoadDriver (
  IN EFI_HANDLE       ImageHandle,
  IN SHELL_FILE_ARG   *Arg,
  BOOLEAN             Connect
  );

//
//
//
EFI_BOOTSHELL_CODE(
  EFI_APPLICATION_ENTRY_POINT(InitializeLoad)
)

EFI_STATUS
EFIAPI
InitializeLoad (
  IN EFI_HANDLE             ImageHandle,
  IN EFI_SYSTEM_TABLE       *SystemTable
  )
/*++

Routine Description:

  Loads EFI drivers.

Arguments:
  ImageHandle     The image handle. 
  SystemTable     The system table.

Returns:
  EFI_SUCCESS             - Command completed successfully
  EFI_INVALID_PARAMETER   - Command usage error

--*/
{
  EFI_STATUS              Status;
  EFI_LIST_ENTRY          FileList;
  EFI_LIST_ENTRY          *Link;
  SHELL_FILE_ARG          *Arg;
  BOOLEAN                 Connect;
  SHELL_VAR_CHECK_CODE    RetCode;
  CHAR16                  *Useful;
  SHELL_ARG_LIST          *Item;
  SHELL_VAR_CHECK_PACKAGE ChkPck;

  //
  // We are no being installed as an internal command driver, initialize
  // as an nshell app and run
  //
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
  Status = LibInitializeStrings (&HiiLoadHandle, STRING_ARRAY_NAME, &EfiLoadGuid);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (!EFI_PROPER_VERSION (1, 10)) {
    PrintToken (
      STRING_TOKEN (STR_SHELLENV_GNC_COMMAND_NOT_SUPPORT),
      HiiLoadHandle,
      L"load",
      EFI_VERSION_1_10
      );
    Status = EFI_UNSUPPORTED;
    goto Done;
  }
  //
  // Expand each arg
  //
  LibFilterNullArgs ();
  RetCode = LibCheckVariables (SI, LoadCheckList, &ChkPck, &Useful);
  if (VarCheckOk != RetCode) {
    switch (RetCode) {
    case VarCheckUnknown:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_UNKNOWN_FLAG), HiiLoadHandle, L"load", Useful);
      break;

    case VarCheckDuplicate:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_DUP_FLAG), HiiLoadHandle, L"load", Useful);
      break;

    default:
      break;
    }

    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  if (LibCheckVarGetFlag (&ChkPck, L"-b") != NULL) {
    EnablePageBreak (DEFAULT_INIT_ROW, DEFAULT_AUTO_LF);
  }

  if (LibCheckVarGetFlag (&ChkPck, L"-?") != NULL) {
    if (IS_OLD_SHELL) {
      PrintToken (STRING_TOKEN (STR_NO_HELP), HiiLoadHandle);
    } else if (ChkPck.ValueCount > 0 ||
             ChkPck.FlagCount > 2 ||
             (2 == ChkPck.FlagCount && !LibCheckVarGetFlag (&ChkPck, L"-b"))
            ) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiLoadHandle, L"load");
      Status = EFI_INVALID_PARAMETER;
    } else {
      PrintToken (STRING_TOKEN (STR_LOAD_VERBOSE_HELP), HiiLoadHandle);
    }

    goto Done;
  }

  Connect = TRUE;
  if (LibCheckVarGetFlag (&ChkPck, L"-nc") != NULL) {
    Connect = FALSE;
  }

  if (ChkPck.ValueCount < 1) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_FEW), HiiLoadHandle, L"load");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  InitializeListHead (&FileList);
  Item = GetFirstArg (&ChkPck);
  while (NULL != Item) {
    Status = ShellFileMetaArg (Item->VarStr, &FileList);
    if (EFI_ERROR (Status) && EFI_NOT_FOUND != Status) {
      goto FreeDone;
    } else if (Status == EFI_NOT_FOUND) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_FILE_NOT_FOUND), HiiLoadHandle, L"load", Item->VarStr);
    }

    Item = GetNextArg (Item);
  }

  for (Link = FileList.Flink; Link != &FileList; Link = Link->Flink) {
    if (GetExecutionBreak ()) {
      Status = EFI_ABORTED;
      break;
    }

    Arg = CR (Link, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);
    if (Arg->Handle == NULL) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_FILE_NOT_FOUND), HiiLoadHandle, L"load", Arg->FullName);
    } else {
      Status = LoadDriver (ImageHandle, Arg, Connect);
    }
  }

FreeDone:
  ShellFreeFileList (&FileList);
Done:
  LibCheckVarFreeVarList (&ChkPck);
  LibUnInitializeStrings ();
  return Status;
}

EFI_STATUS
LoadConnectAllDriversToAllControllers (
  VOID
  )

{
  EFI_STATUS  Status;
  UINTN       AllHandleCount;
  EFI_HANDLE  *AllHandleBuffer;
  UINTN       Index;
  UINTN       HandleCount;
  EFI_HANDLE  *HandleBuffer;
  UINT32      *HandleType;
  UINTN       HandleIndex;
  BOOLEAN     Parent;
  BOOLEAN     Device;

  Status = LibLocateHandle (
            AllHandles,
            NULL,
            NULL,
            &AllHandleCount,
            &AllHandleBuffer
            );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  for (Index = 0; Index < AllHandleCount; Index++) {
    if (GetExecutionBreak ()) {
      Status = EFI_ABORTED;
      goto Done;
    }
    //
    // Scan the handle database
    //
    Status = LibScanHandleDatabase (
              NULL,
              NULL,
              AllHandleBuffer[Index],
              NULL,
              &HandleCount,
              &HandleBuffer,
              &HandleType
              );
    if (EFI_ERROR (Status)) {
      goto Done;
    }

    Device = TRUE;
    if (HandleType[Index] & EFI_HANDLE_TYPE_DRIVER_BINDING_HANDLE) {
      Device = FALSE;
    }

    if (HandleType[Index] & EFI_HANDLE_TYPE_IMAGE_HANDLE) {
      Device = FALSE;
    }

    if (Device) {
      Parent = FALSE;
      for (HandleIndex = 0; HandleIndex < HandleCount; HandleIndex++) {
        if (HandleType[HandleIndex] & EFI_HANDLE_TYPE_PARENT_HANDLE) {
          Parent = TRUE;
        }
      }

      if (!Parent) {
        if (HandleType[Index] & EFI_HANDLE_TYPE_DEVICE_HANDLE) {
          Status = BS->ConnectController (
                        AllHandleBuffer[Index],
                        NULL,
                        NULL,
                        TRUE
                        );
        }
      }
    }

    FreePool (HandleBuffer);
    FreePool (HandleType);
  }

Done:
  FreePool (AllHandleBuffer);
  return Status;
}

STATIC
EFI_STATUS
LoadDriver (
  IN EFI_HANDLE               ParentImage,
  IN SHELL_FILE_ARG           *Arg,
  IN BOOLEAN                  Connect
  )
{
  EFI_HANDLE                ImageHandle;
  EFI_STATUS                Status;
  EFI_DEVICE_PATH_PROTOCOL  *NodePath;
  EFI_DEVICE_PATH_PROTOCOL  *FilePath;
  EFI_LOADED_IMAGE_PROTOCOL *ImageInfo;
  CHAR16                    *LoadOptions;
  UINTN                     LoadOptionsSize;
  CHAR16                    *Cwd;

  NodePath  = FileDevicePath (NULL, Arg->FileName);
  FilePath  = AppendDevicePath (Arg->ParentDevicePath, NodePath);
  if (NodePath) {
    FreePool (NodePath);
    NodePath = NULL;
  }

  if (!FilePath) {
    return EFI_OUT_OF_RESOURCES;
  }

  Status = BS->LoadImage (
                FALSE,
                ParentImage,
                FilePath,
                NULL,
                0,
                &ImageHandle
                );
  FreePool (FilePath);

  if (EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_LOAD_NOT_IMAGE), HiiLoadHandle, Arg->FullName);
    return EFI_INVALID_PARAMETER;
  }
  //
  // Verify the image is a driver ?
  //
  BS->HandleProtocol (ImageHandle, &gEfiLoadedImageProtocolGuid, (VOID *) &ImageInfo);
  if (ImageInfo->ImageCodeType != EfiBootServicesCode && ImageInfo->ImageCodeType != EfiRuntimeServicesCode) {

    PrintToken (STRING_TOKEN (STR_LOAD_IMAGE_NOT_DRIVER), HiiLoadHandle, Arg->FullName);
    BS->Exit (ImageHandle, EFI_INVALID_PARAMETER, 0, NULL);
    return EFI_INVALID_PARAMETER;
  }
  //
  // Construct a load options buffer containing the command line and
  // current working directory.
  //
  // NOTE: To prevent memory leaks, the protocol is responsible for
  // freeing the memory associated with the load options.
  //
  // One day we'll pass arguments to the protocol....
  //
  Cwd = ShellCurDir (NULL);
  if (NULL == Cwd) {
    Cwd = StrDuplicate (L"");
  }

  LoadOptionsSize = (StrLen (Arg->FullName) + 2 + StrLen (Cwd) + 2) * sizeof (CHAR16);
  LoadOptions     = AllocatePool (LoadOptionsSize);
  ASSERT (LoadOptions);

  StrCpy (LoadOptions, Arg->FullName);
  StrCpy (&LoadOptions[StrLen (LoadOptions) + 1], Cwd);
  FreePool (Cwd);

  if (ImageInfo->LoadOptions) {
    FreePool (ImageInfo->LoadOptions);
  }

  ImageInfo->LoadOptionsSize  = (UINT32) LoadOptionsSize;
  ImageInfo->LoadOptions      = LoadOptions;

  //
  // Start the image
  //
  Status = BS->StartImage (ImageHandle, NULL, NULL);
  if (!EFI_ERROR (Status)) {
    PrintToken (
      STRING_TOKEN (STR_LOAD_IMAGE_LOADED),
      HiiLoadHandle,
      Arg->FullName,
      ImageInfo->ImageBase,
      Status
      );
  } else {
    PrintToken (
      STRING_TOKEN (STR_LOAD_IMAGE_ERROR),
      HiiLoadHandle,
      Arg->FullName,
      Status
      );
  }

  if (Connect) {
    Status = LoadConnectAllDriversToAllControllers ();
  }
  //
  // When any driver starts, turn off the watchdog timer
  //
  BS->SetWatchdogTimer (0x0000, 0x0000, 0x0000, NULL);
  return Status;
}

EFI_STATUS
EFIAPI
InitializeLoadGetLineHelp (
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
  return LibCmdGetStringByToken (STRING_ARRAY_NAME, &EfiLoadGuid, STRING_TOKEN (STR_LOAD_LINE_HELP), Str);
}
