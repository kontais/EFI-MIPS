/*++

Copyright (c) 2005 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:
 
  mkdir.c

Abstract:

  EFI shell command "mkdir"

Revision History

--*/

#include "EfiShellLib.h"
#include "mkdir.h"

extern UINT8  STRING_ARRAY_NAME[];

//
// This is the generated header file which includes whatever needs to be exported (strings + IFR)
//
#include STRING_DEFINES_FILE

//
// Function declarations
//
EFI_STATUS
EFIAPI
InitializeMkDir (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
MkDir (
  IN SHELL_FILE_ARG       *Arg
  );

//
// Global Variables
//
EFI_HII_HANDLE  HiiHandle;
EFI_GUID        EfiMkdirGuid = EFI_MKDIR_GUID;
SHELL_VAR_CHECK_ITEM    MkdirCheckList[] = {
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
// Entry point
//
EFI_BOOTSHELL_CODE(
  EFI_APPLICATION_ENTRY_POINT(InitializeMkDir)
)

EFI_STATUS
EFIAPI
InitializeMkDir (
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
  EFI_SUCCESS             - The command completed successfully
  EFI_INVALID_PARAMETER   - Command usage error
  Other value             - Unknown error

--*/
{
  EFI_STATUS              Status;
  EFI_LIST_ENTRY          FileList;
  EFI_LIST_ENTRY          *Link;
  SHELL_FILE_ARG          *Arg;

  SHELL_VAR_CHECK_CODE    RetCode;
  CHAR16                  *Useful;
  SHELL_ARG_LIST          *Item;
  SHELL_VAR_CHECK_PACKAGE ChkPck;

  //
  // Local variable initializations
  //
  InitializeListHead (&FileList);
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
  Status = LibInitializeStrings (&HiiHandle, STRING_ARRAY_NAME, &EfiMkdirGuid);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (!EFI_PROPER_VERSION (0, 99)) {
    PrintToken (
      STRING_TOKEN (STR_SHELLENV_GNC_COMMAND_NOT_SUPPORT),
      HiiHandle,
      L"mkdir",
      EFI_VERSION_0_99 
      );
    Status = EFI_UNSUPPORTED;
    goto Done;
  }
  //
  // Parse command line arguments
  //
  RetCode = LibCheckVariables (SI, MkdirCheckList, &ChkPck, &Useful);
  if (VarCheckOk != RetCode) {
    switch (RetCode) {
    case VarCheckUnknown:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_UNKNOWN_FLAG), HiiHandle, L"mkdir", Useful);
      break;

    case VarCheckDuplicate:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_DUP_FLAG), HiiHandle, L"mkdir", Useful);
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
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiHandle, L"mkdir");
      Status = EFI_INVALID_PARAMETER;
    } else {
      if (IS_OLD_SHELL) {
        PrintToken (STRING_TOKEN (STR_NO_HELP), HiiHandle);
      } else {
        PrintToken (STRING_TOKEN (STR_MKDIR_VERBOSE_HELP), HiiHandle);
      }

      Status = EFI_SUCCESS;
    }

    goto Done;
  }

  if (ChkPck.ValueCount < 1) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_FEW), HiiHandle, L"mkdir");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }
  //
  // do not allow more than 1 file arguments to aVOID write to similar file.
  //
  //
  // Expand each dir
  //
  Item = ChkPck.VarList;
  while (NULL != Item) {
    Status = ShellFileMetaArgNoWildCard (Item->VarStr, &FileList);
    if (EFI_ERROR (Status)) {
      PrintToken (STRING_TOKEN (STR_MKDIR_CANNOT_FIND), HiiHandle, Item->VarStr, Status);
      goto Done;
    }

    Item = Item->Next;
  }

  if (IsListEmpty (&FileList)) {
    Status = EFI_NOT_FOUND;
    PrintToken (STRING_TOKEN (STR_MKDIR_NO_DIR), HiiHandle);
    goto Done;
  }
  //
  // Make each directory
  //
  for (Link = FileList.Flink; Link != &FileList; Link = Link->Flink) {
    Arg     = CR (Link, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);
    Status  = MkDir (Arg);
  }

Done:
  ShellFreeFileList (&FileList);
  LibCheckVarFreeVarList (&ChkPck);
  LibUnInitializeStrings ();
  return Status;
}

EFI_STATUS
MkDir (
  IN SHELL_FILE_ARG       *Arg
  )
{
  EFI_FILE_HANDLE NewDir;
  EFI_STATUS      Status;
  EFI_STATUS      OpenStatus;
  EFI_LIST_ENTRY  SrcList;
  SHELL_FILE_ARG  *SrcArg1;
  EFI_LIST_ENTRY  *Link;

  //
  // Local variable initializations
  //
  NewDir  = NULL;
  Status  = Arg->Status;
  InitializeListHead (&SrcList);
  //
  // if the directory already exists, we can not create it
  //
  if (!EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_MKDIR_DIR_EXISTS), HiiHandle, Arg->FullName);
    return Status;
  }
  //
  // this is what we want
  //
  if (Status == EFI_NOT_FOUND) {

    OpenStatus = ShellFileMetaArgNoWildCard (Arg->FullName, &SrcList);

    if (!EFI_ERROR (OpenStatus) && !IsListEmpty (&SrcList)) {

      for (Link = SrcList.Flink; Link != &SrcList; Link = Link->Flink) {

        SrcArg1 = CR (Link, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);

        if (!EFI_ERROR (SrcArg1->Status)) {
          PrintToken (STRING_TOKEN (STR_MKDIR_DIR_EXISTS), HiiHandle, Arg->FullName);
          Status = EFI_SUCCESS;
          goto Done;
        }
      }
    }

    Status = Arg->Parent->Open (
                            Arg->Parent,
                            &NewDir,
                            Arg->FileName,
                            EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE,
                            EFI_FILE_DIRECTORY
                            );
  }

Done:
  if (EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_MKDIR_CANNOT_CREATE), HiiHandle, Arg->FullName, Status);
  }

  if (NewDir) {
    NewDir->Close (NewDir);
  }

  ShellFreeFileList (&SrcList);
  return Status;
}

EFI_STATUS
EFIAPI
InitializeMkDirGetLineHelp (
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
  return LibCmdGetStringByToken (STRING_ARRAY_NAME, &EfiMkdirGuid, STRING_TOKEN (STR_MKDIR_LINE_HELP), Str);
}
