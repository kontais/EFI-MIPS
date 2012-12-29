/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

  Module Name:
    main.c

  Abstract:
     

--*/

#include "EfiShellLib.h"
#include "editor.h"

extern UINT8  STRING_ARRAY_NAME[];

EFI_STATUS
InitializeEFIEditor (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_BOOTSHELL_CODE(
  EFI_DRIVER_ENTRY_POINT (InitializeEFIEditor)
)
//
// Global Variables
//
EFI_HII_HANDLE    HiiHandle;
EFI_HII_PROTOCOL  *Hii;
EFI_GUID          EfiEditGuid = EFI_EDIT_GUID;
SHELL_VAR_CHECK_ITEM    EditCheckList[] = {
  {
    L"-?",
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
    NULL,
    0,
    0,
    0
  }
};



EFI_HANDLE        ImageHandleBackup;

//
// Name:
//    InitializeEFIEditor -- Entry point of editor
// In:
//    ImageHandle
//    SystemTable
// Out:
//    EFI_SUCCESS
//
EFI_STATUS
InitializeEFIEditor (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
/*++

Routine Description:

  Command entry point

Arguments:

  ImageHandle - The image handle
  SystemTable - The system table

Returns:

  EFI_ABORTED - Aborted

--*/
{
  EFI_STATUS              Status;
  CHAR16                  *Buffer;
  UINT32                  Key;

  SHELL_VAR_CHECK_CODE    RetCode;
  CHAR16                  *Useful;
  SHELL_VAR_CHECK_PACKAGE ChkPck;

  Buffer                          = L"";
  ZeroMem (&ChkPck, sizeof (SHELL_VAR_CHECK_PACKAGE));

  //
  // backup this variable to transfer to FileBufferInit
  //
  ImageHandleBackup = ImageHandle;

  LibInitializeShellApplication (ImageHandle, SystemTable);

  //
  // Register our string package with HII and return the handle to it.
  // If previously registered we will simply receive the handle
  //
  //
  // There should only be one HII protocol
  //
  Status = LibLocateProtocol (&gEfiHiiProtocolGuid, &Hii);
  if (EFI_ERROR (Status) || NULL == Hii) {
    return EFI_ABORTED;
  }

  Status = LibInitializeStrings (&HiiHandle, STRING_ARRAY_NAME, &EfiEditGuid);

  if (EFI_ERROR (Status)) {
    return Status;
  }

  RetCode = LibCheckVariables (SI, EditCheckList, &ChkPck, &Useful);
  if (VarCheckOk != RetCode) {
    switch (RetCode) {
    case VarCheckUnknown:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_UNKNOWN_FLAG), HiiHandle, L"edit", Useful);
      break;

    default:
      break;
    }

    Status = EFI_INVALID_PARAMETER;
    goto done;
  }

  if (LibCheckVarGetFlag (&ChkPck, L"-b") != NULL) {
    if (LibCheckVarGetFlag (&ChkPck, L"-?") != NULL) {
      EnablePageBreak (DEFAULT_INIT_ROW, DEFAULT_AUTO_LF);
    }
  }
  //
  // Out put help.
  //
  if (LibCheckVarGetFlag (&ChkPck, L"-?") != NULL) {
    if (ChkPck.ValueCount > 0 ||
        ChkPck.FlagCount > 2 ||
        (ChkPck.FlagCount == 2 && LibCheckVarGetFlag (&ChkPck, L"-b") == NULL)
        ) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiHandle, L"edit");
      Status = EFI_INVALID_PARAMETER;
    } else {
      PrintToken (STRING_TOKEN (STR_EDIT_VERBOSE_HELP), HiiHandle);
      Status = EFI_SUCCESS;
    }

    goto done;
  }

  if (ChkPck.ValueCount > 1) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiHandle, L"edit");
    Status = EFI_INVALID_PARAMETER;
    goto done;
  }

  if (ChkPck.ValueCount == 1) {
    if (!IsValidFileName (ChkPck.VarList->VarStr)) {
      PrintToken (STRING_TOKEN (STR_EDIT_MAIN_INVALID_FILE_NAME), HiiHandle);
      Status = EFI_INVALID_PARAMETER;
      goto done;
    }
  }

  if (SI->RedirArgc != 0) {
    PrintToken (STRING_TOKEN (STR_EDIT_NOREDIRECT), HiiHandle);
    Status = EFI_INVALID_PARAMETER;
    goto done;
  }

  Status = MainEditorInit ();
  if (EFI_ERROR (Status)) {
    Out->ClearScreen (Out);
    Out->EnableCursor (Out, TRUE);
    PrintToken (STRING_TOKEN (STR_EDIT_MAIN_INIT_FAILED), HiiHandle);
    goto done;
  }

  MainEditorBackup ();

  //
  // if editor launched with file named
  //
  if (ChkPck.ValueCount == 1) {

    FileBufferSetFileName (ChkPck.VarList->VarStr);
  }

  Status = FileBufferRead (MainEditor.FileBuffer->FileName, FALSE);
  if (!EFI_ERROR (Status)) {
    MainEditorRefresh ();
    //
    // Set Key Filter, to be sure that the output will no pause
    //
    Key = SE2->GetKeyFilter ();
    SE2->SetKeyFilter (EFI_OUTPUT_SCROLL);

    Status = MainEditorKeyInput ();
    //
    // Set Key Filter to original value
    //
    SE2->SetKeyFilter (Key);

  }

  if (Status != EFI_OUT_OF_RESOURCES) {
    //
    // back up the status string
    //
    Buffer = PoolPrint (L"%s", MainEditor.StatusBar->StatusString);
  }

  MainEditorCleanup ();

  //
  // print editor exit code on screen
  //
  if (Status == EFI_SUCCESS) {
  } else if (Status == EFI_OUT_OF_RESOURCES) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_OUT_RESOURCE), HiiHandle, L"edit");
  } else {
    if (Buffer != NULL) {
      if (StrCmp (Buffer, L"") != 0) {
        //
        // print out the status string
        //
        PrintToken (STRING_TOKEN (STR_EDIT_MAIN_BUFFER), HiiHandle, Buffer);
      } else {
        PrintToken (STRING_TOKEN (STR_EDIT_MAIN_UNKNOWN_EDITOR_ERR), HiiHandle);
      }
    } else {
      PrintToken (STRING_TOKEN (STR_EDIT_MAIN_UNKNOWN_EDITOR_ERR), HiiHandle);
    }
  }

  if (Status != EFI_OUT_OF_RESOURCES) {
    EditorFreePool (Buffer);
  }

done:
  LibCheckVarFreeVarList (&ChkPck);
  LibUnInitializeStrings ();
  return Status;

}

EFI_STATUS
InitializeEFIEditorGetLineHelp (
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
  return LibCmdGetStringByToken (STRING_ARRAY_NAME, &EfiEditGuid, STRING_TOKEN (STR_EDIT_LINE_HELP), Str);
}
