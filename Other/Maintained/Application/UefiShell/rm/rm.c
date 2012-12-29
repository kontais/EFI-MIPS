/*++

Copyright (c) 2005 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  rm.c

Abstract:

  EFI Shell command "rm"

Revision History

--*/

#include "rm.h"

extern UINT8    STRING_ARRAY_NAME[];

//
// Global variables
//
BOOLEAN         mRmUserCanceled;
EFI_HII_HANDLE  HiiRmHandle;
EFI_GUID        EfiRmGuid = EFI_RM_GUID;
SHELL_VAR_CHECK_ITEM    RmCheckList[] = {
  {
    L"-q",
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
    FALSE
  }
};

//
// Function declarations
//
EFI_STATUS
EFIAPI
InitializeRM (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

//
// Entry point
//
EFI_BOOTSHELL_CODE(
  EFI_APPLICATION_ENTRY_POINT(InitializeRM)
)

EFI_STATUS
EFIAPI
InitializeRM (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
/*++

Routine Description:
  Command entry point. Parses command line arguments and calls internal
  function to perform actual action.

Arguments:
  ImageHandle     The image handle.
  SystemTable     The system table.

Returns:
  EFI_SUCCESS             - Command completed successfully
  EFI_INVALID_PARAMETER   - Command usage error
  EFI_ACCESS_DENIED       - Files/directories can't be removed
  EFI_OUT_OF_RESOURCES    - Out of memory/file handles
  Other value             - Unknown error

--*/
{
  EFI_STATUS  Status;

  //
  // We are no being installed as an internal command driver, initialize
  // as an nshell app and run
  //
  EFI_SHELL_APP_INIT (ImageHandle, SystemTable);
  
  //
  // Enable tab key which can pause the output
  //
  EnableOutputTabPause();

  //
  // Register our string package with HII and return the handle to it.
  // If previously registered we will simply receive the handle
  //
  Status = LibInitializeStrings (&HiiRmHandle, STRING_ARRAY_NAME, &EfiRmGuid);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = RmMainProc (ImageHandle, SystemTable, STRING_TOKEN (STR_RM_VERBOSE_HELP));
  LibUnInitializeStrings ();
  return Status;
}

EFI_STATUS
RmMainProc (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable,
  IN UINT16               VHlpToken
  )
/*++

Routine Description:
  Command entry point. Parses command line arguments and calls internal
  function to perform actual action.

Arguments:
  ImageHandle     The image handle.
  SystemTable     The system table.
  VHlpToken       The help token.

Returns:
  EFI_SUCCESS             - Command completed successfully
  EFI_INVALID_PARAMETER   - Command usage error
  EFI_ACCESS_DENIED       - Files/directories can't be removed
  EFI_OUT_OF_RESOURCES    - Out of memory/file handles
  Other value             - Unknown error

--*/
{
  EFI_STATUS              Status;
  UINTN                   Index;
  EFI_LIST_ENTRY          FileList;
  EFI_LIST_ENTRY          *Link;
  SHELL_FILE_ARG          *Arg;
  CHAR16                  *Ptr;
  CHAR16                  *TargetDev;
  CHAR16                  *CurDir;
  CHAR16                  *CurPath;
  CHAR16                  *TargetPath;
  CHAR16                  *CurFullName;
  CHAR16                  TempChar;
  BOOLEAN                 Quiet;
  BOOLEAN                 IsSameFile;
  SHELL_VAR_CHECK_PACKAGE ChkPck;
  SHELL_ARG_LIST          *ArgItem;
  SHELL_VAR_CHECK_CODE    RetCode;
  CHAR16                  *Useful;

  Status  = EFI_SUCCESS;
  ZeroMem (&ChkPck, sizeof (SHELL_VAR_CHECK_PACKAGE));

  RetCode = LibCheckVariables (SI, RmCheckList, &ChkPck, &Useful);
  if (VarCheckOk != RetCode) {
    switch (RetCode) {
    case VarCheckDuplicate:

      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_DUP_FLAG), HiiRmHandle, L"rm/del", Useful);
      break;

    case VarCheckUnknown:

      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_UNKNOWN_FLAG), HiiRmHandle, L"rm/del", Useful);
      break;

    default:
      break;
    }

    LibCheckVarFreeVarList (&ChkPck);
    return EFI_INVALID_PARAMETER;
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
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiRmHandle, L"rm/del");
      Status = EFI_INVALID_PARAMETER;
    } else {
      if (IS_OLD_SHELL) {
        PrintToken (STRING_TOKEN (STR_NO_HELP), HiiRmHandle);
      } else {
        PrintToken (VHlpToken, HiiRmHandle);
      }

      Status = EFI_SUCCESS;
    }

    LibCheckVarFreeVarList (&ChkPck);
    return Status;
  }
  //
  // Local variable initializations
  //

  InitializeListHead (&FileList);
  CurDir      = NULL;
  TargetDev   = NULL;
  TargetPath  = NULL;
  CurFullName = NULL;
  TempChar    = 0;
  Quiet       = FALSE;
  Status      = EFI_SUCCESS;
  IsSameFile  = FALSE;

  //
  // Global variable initializations
  //
  mRmUserCanceled = FALSE;

  //
  // Parse command line arguments
  //
  if (ChkPck.ValueCount < 1) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_FEW), HiiRmHandle, L"rm/del");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  if (LibCheckVarGetFlag (&ChkPck, L"-q") != NULL) {
    Quiet = TRUE;
  }
  //
  // Expand each arg
  //
  ArgItem = ChkPck.VarList;
  while (NULL != ArgItem) {
    if (SI->RedirArgc) {
      for (Index = 1; Index < SI->RedirArgc; Index++) {
        if (!EFI_ERROR (LibCompareFile (ArgItem->VarStr, SI->RedirArgv[Index], &IsSameFile)) && IsSameFile == TRUE) {
          LibCheckVarFreeVarList (&ChkPck);
          ShellFreeFileList (&FileList);
          return EFI_REDIRECTION_SAME;
        }
      }
    }

    Status = ShellFileMetaArg (ArgItem->VarStr, &FileList);
    if (EFI_ERROR (Status)) {
      PrintToken (STRING_TOKEN (STR_RM_CANNOT_FIND), HiiRmHandle, ArgItem->VarStr, Status);
      goto Done;
    }

    if (IsListEmpty (&FileList)) {
      Status = EFI_NOT_FOUND;
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_FILE_NOT_FOUND), HiiRmHandle);
      goto Done;
    }
    //
    // Remove each file
    //
    for (Link = FileList.Flink; Link != &FileList; Link = Link->Flink) {
      //
      // Break the execution?
      //
      if (GetExecutionBreak ()) {
        goto Done;
      }

      Arg = CR (Link, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);
      if (Arg->Status != EFI_SUCCESS) {
        PrintToken (STRING_TOKEN (STR_RM_CANNOT_FIND), HiiRmHandle, Arg->FullName, Arg->Status);
        Status = Arg->Status;
        goto Done;
      }
      //
      // Check to see if we are removing a root dir
      //
      for (Ptr = Arg->FullName; (*Ptr); Ptr++) {
        if ((*Ptr) == ':') {
          break;
        }
      }

      if (*Ptr && *(Ptr + 1) == '\\' && *(Ptr + 2) == 0) {
        PrintToken (STRING_TOKEN (STR_RM_CANNOT_REMOVE_ROOT_DIR), HiiRmHandle);
        Status = EFI_ACCESS_DENIED;
        goto Done;
      }
      //
      // Check to see if we are removing current dir or its ancestor
      //
      if (TargetDev) {
        FreePool (TargetDev);
      }

      TargetDev = StrDuplicate (Arg->FullName);
      for (Index = 0; Index < StrLen (TargetDev); Index++) {
        if (TargetDev[Index] == ':') {
          break;
        }
      }

      TargetDev[Index] = 0;

      //
      // find out the current directory
      //
      CurDir = ShellCurDir (TargetDev);
      if (!CurDir) {
        PrintToken (STRING_TOKEN (STR_RM_CANNOT_GET_CURRENT_DIR), HiiRmHandle);
        Status = EFI_INVALID_PARAMETER;
        goto Done;
      }

      for (CurPath = CurDir; *CurPath && *CurPath != ':'; CurPath++) {
        ;
      }

      for (TargetPath = Arg->FullName; *TargetPath && *TargetPath != ':'; TargetPath++) {
        ;
      }

      if (CurFullName) {
        FreePool (CurFullName);
      }

      CurFullName = StrDuplicate (CurPath);
      if (!CurFullName) {
        PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_OUT_RESOURCE), HiiRmHandle);
        Status = EFI_OUT_OF_RESOURCES;
        goto Done;
      }

      for (Index = StrLen (CurFullName); Index > 0; Index--) {
        TempChar            = CurFullName[Index];
        CurFullName[Index]  = 0;
        if (StriCmp (TargetPath, CurFullName) == 0) {
          if (TargetPath[Index - 1] == '\\' || TempChar == '\\' || TempChar == 0) {
            PrintToken (STRING_TOKEN (STR_RM_CANNOT_REMOVE_CURRENT_DIR), HiiRmHandle);
            Status = EFI_ACCESS_DENIED;
            goto Done;
          }
        }
      }
      //
      // Remove it
      //
      Status = RemoveRM (Arg->Handle, Arg->FullName, Quiet);
      if (EFI_ERROR (Status)) {
        PrintToken (STRING_TOKEN (STR_RM_ERROR), HiiRmHandle, Status);
        Arg->Handle = NULL;
        goto Done;
      } else {
        if (!mRmUserCanceled) {
          PrintToken (STRING_TOKEN (STR_RM_OK), HiiRmHandle);
          Arg->Handle = NULL;
        } else {
          PrintToken (STRING_TOKEN (STR_RM_CANCELED), HiiRmHandle);
          mRmUserCanceled = FALSE;
        }

      }

      FreePool (CurDir);
      CurDir = NULL;
    }

    ShellFreeFileList (&FileList);
    ArgItem = ArgItem->Next;
  }

Done:
  if (CurDir) {
    FreePool (CurDir);
  }

  if (TargetDev) {
    FreePool (TargetDev);
  }

  if (CurFullName) {
    FreePool (CurFullName);
  }

  ShellFreeFileList (&FileList);
  LibCheckVarFreeVarList (&ChkPck);

  return Status;
}

EFI_STATUS
RemoveRM (
  IN EFI_FILE_HANDLE          FileHandle,
  IN CHAR16                   *FileName,
  IN BOOLEAN                  Quiet
  )
/*++

Routine Description:

  Remove the file or directory indicated by FileHandle

Arguments:

  FileHandle      The handle of the file or directory to be removed
  FileName        The name of the file or directory to be removed
  Quiet           Indicates if a prompt for user confirmation is required

Returns:

  EFI_SUCCESS     The function completed successfully
  Other Value     The function failed due to some reason

--*/
{
  EFI_STATUS      Status;
  EFI_FILE_HANDLE ChildHandle;
  UINTN           Size;
  CHAR16          Str[2];
  EFI_FILE_INFO   *Info;
  CHAR16          *ChildFileName;
  UINTN           ChildNameSize;

  //
  // Local variable initializations
  //
  Status        = EFI_SUCCESS;
  ChildHandle   = NULL;
  Str[0]        = 0;
  ChildFileName = NULL;
  ChildNameSize = 0;

  Size          = (SIZE_OF_EFI_FILE_INFO + 1024);
  Info          = AllocatePool (Size);
  if (!Info) {
    PrintToken (STRING_TOKEN (STR_RM_OUT_OF_RESOURCES), HiiRmHandle);
    Status = EFI_OUT_OF_RESOURCES;
    goto Done;
  }
  //
  // If the file is a directory check it
  //
  Status = FileHandle->GetInfo (FileHandle, &gEfiFileInfoGuid, &Size, Info);
  if (EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_RM_CANNOT_ACCESS), HiiRmHandle, FileName);
    goto Done;
  }

  if (Info->Attribute & EFI_FILE_DIRECTORY) {
    //
    // Remove all child entries from the directory
    //
    if (Info->Attribute & EFI_FILE_READ_ONLY) {
      Status = EFI_ACCESS_DENIED;
      PrintToken (STRING_TOKEN (STR_RM_CANNOT_ACCESS), HiiRmHandle, FileName);
      goto Done;
    }

    FileHandle->SetPosition (FileHandle, 0);
    for (;;) {
      //
      // Break the execution?
      //
      if (GetExecutionBreak ()) {
        goto Done;
      }

      Size    = (SIZE_OF_EFI_FILE_INFO + 1024);
      Status  = FileHandle->Read (FileHandle, &Size, Info);
      if (EFI_ERROR (Status) || Size == 0) {
        break;
      }
      //
      // Skip "." and ".."
      //
      if (StriCmp (Info->FileName, L".") == 0 || StriCmp (Info->FileName, L"..") == 0) {
        continue;
      }
      //
      // Open the child
      //
      Status = FileHandle->Open (
                            FileHandle,
                            &ChildHandle,
                            Info->FileName,
                            EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE,
                            0
                            );
      if (EFI_ERROR (Status)) {
        PrintToken (
          STRING_TOKEN (STR_RM_CANNOT_OPEN_UNDER),
          HiiRmHandle,
          Info->FileName,
          FileName
          );
        goto Done;
      }
      //
      // Prompt
      //
      if (!Quiet) {
Prompt:
        PrintToken (STRING_TOKEN (STR_RM_REMOVE_SUBTREE), HiiRmHandle, FileName);
        Input (NULL, Str, 2);
        Print (L"\n");

        switch (Str[0]) {
        case 'y':
        case 'Y':
          Status = EFI_SUCCESS;
          break;

        case 'n':
        case 'N':
          Status          = EFI_ACCESS_DENIED;
          mRmUserCanceled = TRUE;
          ChildHandle->Close (ChildHandle);
          Status = EFI_SUCCESS;
          goto Done;

        default:
          goto Prompt;
        }
      }
      //
      // Compose new file name
      //
      ChildNameSize = StrSize (FileName) + 1 + StrSize (Info->FileName) + 2;

      ChildFileName = AllocatePool (ChildNameSize);
      if (!ChildFileName) {
        PrintToken (STRING_TOKEN (STR_RM_OUT_OF_MEM), HiiRmHandle);
        ChildHandle->Close (ChildHandle);
        Status = EFI_OUT_OF_RESOURCES;
        goto Done;
      }

      StrCpy (ChildFileName, FileName);
      if (ChildFileName[StrLen (ChildFileName) - 1] != '\\') {
        StrCat (ChildFileName, L"\\");
      }

      StrCat (ChildFileName, Info->FileName);

      if (ChildNameSize > 2048) {
        PrintToken (STRING_TOKEN (STR_RM_PATH_TOO_LARGE), HiiRmHandle, ChildFileName);
        ChildHandle->Close (ChildHandle);
        Status = EFI_OUT_OF_RESOURCES;
        goto Done;
      }
      //
      // call myself
      //
      Quiet   = TRUE;
      Status  = RemoveRM (ChildHandle, ChildFileName, TRUE);
      if (EFI_ERROR (Status)) {
        goto Done;
      } else {
        PrintToken (STRING_TOKEN (STR_RM_OK), HiiRmHandle);
      }

      FreePool (ChildFileName);
      ChildFileName = NULL;
    }
    //
    // end of loop
    //
  }

  if (EFI_ERROR (Status)) {
    goto Done;
  }
  //
  // Remove the file
  //
  PrintToken (STRING_TOKEN (STR_RM_REMOVING), HiiRmHandle, FileName);

  Status = FileHandle->Delete (FileHandle);
  if (Status == EFI_WARN_DELETE_FAILURE) {
    Status = EFI_ACCESS_DENIED;
  }

Done:
  if (ChildFileName) {
    FreePool (ChildFileName);
  }

  if (Info) {
    FreePool (Info);
  }

  return Status;
}

EFI_STATUS
EFIAPI
InitializeRMGetLineHelp (
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
  return LibCmdGetStringByToken (STRING_ARRAY_NAME, &EfiRmGuid, STRING_TOKEN (STR_RM_LINE_HELP), Str);
}
