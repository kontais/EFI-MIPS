/*++

Copyright (c) 2005 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  touch.c
  
Abstract:

  EFI Shell command "touch"

Revision History

--*/

#include "EfiShellLib.h"
#include "touch.h"

extern UINT8  STRING_ARRAY_NAME[];

//
// This is the generated header file which includes whatever needs to be exported (strings + IFR)
//
#include STRING_DEFINES_FILE

//
//
//
EFI_STATUS
EFIAPI
InitializeTouch (
  IN EFI_HANDLE             ImageHandle,
  IN EFI_SYSTEM_TABLE       *SystemTable
  );

VOID
TouchFile (
  IN SHELL_FILE_ARG         *Arg,
  IN BOOLEAN                Recursive
  );

SHELL_FILE_ARG  *
CreateChild (
  IN SHELL_FILE_ARG         *Parent,
  IN CHAR16                 *FileName,
  IN OUT EFI_LIST_ENTRY     *ListHead
  );

VOID
TouchFreeFileArg (
  IN SHELL_FILE_ARG         *Arg
  );

#define FILE_INFO_SIZE  (SIZE_OF_EFI_FILE_INFO + 1024)

//
// Global Variables
//
EFI_HII_HANDLE  HiiHandle;
EFI_GUID        EfiTouchGuid = EFI_TOUCH_GUID;
SHELL_VAR_CHECK_ITEM    TouchCheckList[] = {
  {
    L"-r",
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
    FlagTypeSingle
  }
};

EFI_BOOTSHELL_CODE(
  EFI_APPLICATION_ENTRY_POINT(InitializeTouch)
)

EFI_STATUS
EFIAPI
InitializeTouch (
  IN EFI_HANDLE             ImageHandle,
  IN EFI_SYSTEM_TABLE       *SystemTable
  )
/*++

Routine Description:
  Update time of file/directory with current time.

Arguments:
  ImageHandle     The image handle. 
  SystemTable     The system table.

Returns:
  EFI_SUCCESS             - Command completed successfully
  EFI_INVALID_PARAMETER   - Command usage error
  EFI_NOT_FOUND           - Files not found

--*/
{
  EFI_LIST_ENTRY          FileList;
  EFI_LIST_ENTRY          *Link;
  SHELL_FILE_ARG          *Arg;
  BOOLEAN                 Recursive;
  EFI_STATUS              Status;
  SHELL_VAR_CHECK_CODE    RetCode;
  CHAR16                  *Useful;
  SHELL_VAR_CHECK_PACKAGE ChkPck;
  SHELL_ARG_LIST          *Item;

  ZeroMem (&ChkPck, sizeof (SHELL_VAR_CHECK_PACKAGE));
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
  Status = LibInitializeStrings (&HiiHandle, STRING_ARRAY_NAME, &EfiTouchGuid);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (!EFI_PROPER_VERSION (0, 99)) {
    PrintToken (
      STRING_TOKEN (STR_SHELLENV_GNC_COMMAND_NOT_SUPPORT),
      HiiHandle,
      L"touch",
      EFI_VERSION_0_99 
      );
    Status = EFI_UNSUPPORTED;
    goto Done;
  }

  Status = LibFilterNullArgs ();
  if (EFI_ERROR (Status)) {
    goto Done;
  }

  RetCode = LibCheckVariables (SI, TouchCheckList, &ChkPck, &Useful);
  if (VarCheckOk != RetCode) {
    switch (RetCode) {
    case VarCheckUnknown:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_UNKNOWN_FLAG), HiiHandle, L"touch", Useful);
      break;

    case VarCheckDuplicate:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_DUP_FLAG), HiiHandle, L"touch", Useful);
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
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiHandle, L"touch");
      Status = EFI_INVALID_PARAMETER;
    } else {
      if (IS_OLD_SHELL) {
        PrintToken (STRING_TOKEN (STR_NO_HELP), HiiHandle);
      } else {
        PrintToken (STRING_TOKEN (STR_TOUCH_VERBOSE_HELP), HiiHandle);
      }

      Status = EFI_SUCCESS;
    }

    goto Done;
  }

  InitializeListHead (&FileList);
  Recursive = FALSE;

  if (ChkPck.ValueCount == 0) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_FEW), HiiHandle, L"touch");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }
  //
  // Expand each arg
  //
  if (LibCheckVarGetFlag (&ChkPck, L"-r") != NULL) {
    Recursive = TRUE;
  }

  Item = GetFirstArg (&ChkPck);
  while (NULL != Item) {
    ShellFileMetaArg (Item->VarStr, &FileList);
    Item = GetNextArg (Item);
  }
  //
  // if no file specified, get the whole directory
  //
  if (IsListEmpty (&FileList)) {
    Item = GetFirstArg (&ChkPck);
    ASSERT (Item);
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_FILE_NOT_FOUND), HiiHandle, L"touch", Item->VarStr);
    Status = EFI_NOT_FOUND;
    goto Done;
  }
  //
  // Touch each file
  //
  for (Link = FileList.Flink; Link != &FileList; Link = Link->Flink) {
    //
    // Break the execution?
    //
    if (GetExecutionBreak ()) {
      goto FreeDone;
    }

    Arg = CR (Link, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);
    TouchFile (Arg, Recursive);
  }

FreeDone:
  ShellFreeFileList (&FileList);
Done:
  LibCheckVarFreeVarList (&ChkPck);
  LibUnInitializeStrings ();
  return Status;
}

VOID
TouchFile (
  IN SHELL_FILE_ARG             *Arg,
  IN BOOLEAN                    Recursive
  )
{
  EFI_STATUS      Status;
  SHELL_FILE_ARG  *Child;
  EFI_LIST_ENTRY  FileList;
  EFI_FILE_INFO   *FileInfo;
  UINTN           Size;

  InitializeListHead (&FileList);

  if (Arg == NULL) {
    return ;
  }

  Status = Arg->Status;
  if (!EFI_ERROR (Status)) {
    RT->GetTime (&Arg->Info->ModificationTime, NULL);
    Status = Arg->Handle->SetInfo (
                            Arg->Handle,
                            &gEfiFileInfoGuid,
                            (UINTN) Arg->Info->Size,
                            Arg->Info
                            );
  }

  if (EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_TOUCH_CANNOT_TOUCH), HiiHandle, Arg->FullName, Status);
  } else {
    PrintToken (STRING_TOKEN (STR_TOUCH_OK), HiiHandle, Arg->FullName);
  }

  FileInfo = AllocatePool (FILE_INFO_SIZE);
  if (!FileInfo) {
    PrintToken (STRING_TOKEN (STR_TOUCH_OUT_OF_MEM), HiiHandle);
    return ;
  }
  //
  // if it's a directory, open it and recursive
  //
  if (Recursive && Arg->Info && Arg->Info->Attribute & EFI_FILE_DIRECTORY) {
    Arg->Handle->SetPosition (Arg->Handle, 0);
    for (;;) {
      //
      // Break the execution?
      //
      if (GetExecutionBreak ()) {
        goto TouchDone;
      }

      Size    = FILE_INFO_SIZE;
      Status  = Arg->Handle->Read (Arg->Handle, &Size, FileInfo);
      if (EFI_ERROR (Status) || Size == 0) {
        break;
      }
      //
      // Skip "." and "..", otherwise terrible thing will happen
      //
      if (StriCmp (FileInfo->FileName, L".") == 0 || StriCmp (FileInfo->FileName, L"..") == 0) {
        continue;
      }
      //
      // Build a shell_file_arg for the sub-entry
      //
      Child = CreateChild (Arg, FileInfo->FileName, &FileList);

      TouchFile (Child, Recursive);

      //
      // Close the handles
      //
      ShellFreeFileList (&FileList);
    }
  }

TouchDone:
  if (FileInfo) {
    FreePool (FileInfo);
  }
}

SHELL_FILE_ARG *
CreateChild (
  IN SHELL_FILE_ARG                 *Parent,
  IN CHAR16                         *FileName,
  IN OUT EFI_LIST_ENTRY             *ListHead
  )
{
  SHELL_FILE_ARG  *Arg;
  UINTN           Len;

  Arg = AllocateZeroPool (sizeof (SHELL_FILE_ARG));
  if (!Arg) {
    return NULL;
  }

  Arg->Signature = SHELL_FILE_ARG_SIGNATURE;
  Parent->Parent->Open (Parent->Handle, &Arg->Parent, L".", EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE, 0);

  Arg->ParentName = StrDuplicate (Parent->FullName);
  Arg->FileName   = StrDuplicate (FileName);

  //
  // append filename to parent's name to get the file's full name
  //
  Len = StrLen (Arg->ParentName);
  if (Len && Arg->ParentName[Len - 1] == '\\') {
    Len -= 1;
  }

  Arg->FullName = PoolPrint (L"%.*s\\%s", Len, Arg->ParentName, FileName);

  //
  // open it
  //
  Arg->Status = Parent->Handle->Open (
                                  Parent->Handle,
                                  &Arg->Handle,
                                  Arg->FileName,
                                  EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE,
                                  0
                                  );

  if (EFI_ERROR (Arg->Status)) {
    TouchFreeFileArg (Arg);
    return NULL;
  }

  Arg->Info = AllocatePool (FILE_INFO_SIZE);
  if (!Arg->Info) {
    PrintToken (STRING_TOKEN (STR_TOUCH_OUT_OF_MEM), HiiHandle);
    TouchFreeFileArg (Arg);
    return NULL;
  }

  Len         = FILE_INFO_SIZE;
  Arg->Status = Arg->Handle->GetInfo (Arg->Handle, &gEfiFileInfoGuid, &Len, Arg->Info);

  InsertTailList (ListHead, &Arg->Link);
  return Arg;
}

VOID
TouchFreeFileArg (
  IN SHELL_FILE_ARG             *Arg
  )
/*++

Routine Description:

Arguments:
  
  Arg  - The file argument
  
Returns:

--*/
{
  //
  // Free memory for all elements belonging to Arg
  //
  if (Arg->Parent) {
    Arg->Parent->Close (Arg->Parent);
  }

  if (Arg->ParentName) {
    FreePool (Arg->ParentName);
  }

  if (Arg->ParentDevicePath) {
    FreePool (Arg->ParentDevicePath);
  }

  if (Arg->FullName) {
    FreePool (Arg->FullName);
  }

  if (Arg->FileName) {
    FreePool (Arg->FileName);
  }

  if (Arg->Handle) {
    Arg->Handle->Close (Arg->Handle);
  }

  if (Arg->Info) {
    FreePool (Arg->Info);
  }

  if (Arg->Link.Flink) {
    RemoveEntryList (&Arg->Link);
  }
  //
  // Free memory for Arg
  //
  FreePool (Arg);
}

EFI_STATUS
EFIAPI
InitializeTouchGetLineHelp (
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
  return LibCmdGetStringByToken (STRING_ARRAY_NAME, &EfiTouchGuid, STRING_TOKEN (STR_TOUCH_LINE_HELP), Str);
}
