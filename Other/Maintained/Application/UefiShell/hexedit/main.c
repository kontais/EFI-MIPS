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
  Main entry point of editor
  

--*/

#include "EfiShellLib.h"
#include "heditor.h"

extern UINT8      STRING_ARRAY_NAME[];

//
// Global Variables
//
EFI_HII_HANDLE    HiiHandle;
EFI_HII_PROTOCOL  *Hii;
EFI_GUID          EfiHexeditGuid = EFI_HEXEDIT_GUID;
SHELL_VAR_CHECK_ITEM    HexeditCheckList[] = {
  {
    L"-f",
    0x01,
    0x06,
    FlagTypeSingle
  },
  {
    L"-d",
    0x02,
    0x05,
    FlagTypeSingle
  },
  {
    L"-m",
    0x04,
    0x03,
    FlagTypeSingle
  },
  {
    L"-?",
    0x08,
    0x0,
    FlagTypeSingle
  },
  {
    L"-b",
    0x100,
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

EFI_HANDLE        HImageHandleBackup;

EFI_STATUS
InitializeEFIHexEditor (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_BOOTSHELL_CODE(
  EFI_DRIVER_ENTRY_POINT (InitializeEFIHexEditor)
)

CHAR16 *
HexEditGetDefaultFileName (
  VOID
  );

VOID
PrintUsage (
  VOID
  )
{
  PrintToken (STRING_TOKEN (STR_HEXEDIT_USAGE), HiiHandle);
  PrintToken (STRING_TOKEN (STR_HEXEDIT_FILENAME), HiiHandle);
  PrintToken (STRING_TOKEN (STR_HEXEDIT_DISKNAME), HiiHandle);
  PrintToken (STRING_TOKEN (STR_HEXEDIT_OFFSET_SIZE), HiiHandle);
  Print (L"\n\n");
}

EFI_STATUS
InitializeEFIHexEditor (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
/*++

Routine Description: 

  Entry point of editor

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
  EFI_STATUS              Result;
  CHAR16                  *Buffer;
  CHAR16                  *Name;
  BOOLEAN                 FreeName;
  UINTN                   Offset;
  UINT32                  Key;
  UINTN                   Size;
  UINT64                  LastOffset;
  IMAGE_TYPE              WhatToDo;
  CHAR16                  *Useful;
  SHELL_ARG_LIST          *Item;
  SHELL_VAR_CHECK_CODE    RetCode;
  SHELL_VAR_CHECK_PACKAGE ChkPck;
  //
  // backup this variable to transfer to BufferImageInit
  //
  HImageHandleBackup  = ImageHandle;


  EFI_SHELL_APP_INIT (ImageHandle, SystemTable);

  Status = LibLocateProtocol (&gEfiHiiProtocolGuid, &Hii);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Register our string package with HII and return the handle to it.
  // If previously registered we will simply receive the handle
  //
  EFI_SHELL_STR_INIT (HiiHandle, STRING_ARRAY_NAME, EfiHexeditGuid);

  if (!EFI_PROPER_VERSION (1, 10)) {
    PrintToken (
      STRING_TOKEN (STR_SHELLENV_GNC_COMMAND_NOT_SUPPORT),
      HiiHandle,
      L"hexedit",
      EFI_VERSION_1_10
      );
    Status = EFI_UNSUPPORTED;
    goto done;
  }
  //
  // variable initialization
  //
  Buffer      = NULL;
  Name        = NULL;
  FreeName    = FALSE;
  Offset      = 0;
  Size        = 0;
  LastOffset  = 0;
  WhatToDo    = OPEN_FILE;
  ZeroMem (&ChkPck, sizeof (SHELL_VAR_CHECK_PACKAGE));

  RetCode = LibCheckVariables (SI, HexeditCheckList, &ChkPck, &Useful);
  if (VarCheckOk != RetCode) {
    switch (RetCode) {
    case VarCheckConflict:

      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_FLAG_CONFLICT), HiiHandle, L"hexedit", Useful);
      break;

    case VarCheckDuplicate:

      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_DUP_FLAG), HiiHandle, L"hexedit", Useful);
      break;

    case VarCheckUnknown:

      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_UNKNOWN_FLAG), HiiHandle, L"hexedit", Useful);
      break;

    default:
      break;
    }

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
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiHandle, L"hexedit");
      Status = EFI_INVALID_PARAMETER;
    } else {
      PrintToken (STRING_TOKEN (STR_HEXEDIT_VERBOSE_HELP), HiiHandle);
      Status = EFI_SUCCESS;
    }

    goto done;
  }

  Item = LibCheckVarGetFlag (&ChkPck, L"-d");
  if (Item) {
    if (3 < ChkPck.ValueCount) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiHandle, L"hexedit");
      Status = EFI_INVALID_PARAMETER;
      goto done;
    }

    if (3 > ChkPck.ValueCount) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_FEW), HiiHandle, L"hexedit");
      Status = EFI_INVALID_PARAMETER;
      goto done;
    }

    Item    = ChkPck.VarList;
    Name    = Item->VarStr;
    Item    = Item->Next;
    Result  = HXtoi (Item->VarStr, &Offset);
    if (EFI_ERROR (Result)) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_ARG), HiiHandle, L"hexedit", Item->VarStr);
      Status = EFI_INVALID_PARAMETER;
      goto done;
    }

    Item    = Item->Next;
    Result  = HXtoi (Item->VarStr, &Size);
    if (EFI_ERROR (Result)) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_ARG), HiiHandle, L"hexedit", Item->VarStr);
      Status = EFI_INVALID_PARAMETER;
      goto done;
    }

    if (Offset < 0 || Size <= 0) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_ARG), HiiHandle, L"hexedit", Item->VarStr);
      Status = EFI_INVALID_PARAMETER;
      goto done;
    }

    WhatToDo = OPEN_DISK;
  }

  Item = LibCheckVarGetFlag (&ChkPck, L"-m");
  if (Item) {
    if (2 < ChkPck.ValueCount) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiHandle, L"hexedit");
      Status = EFI_INVALID_PARAMETER;
      goto done;
    }

    if (2 > ChkPck.ValueCount) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_FEW), HiiHandle, L"hexedit");
      Status = EFI_INVALID_PARAMETER;
      goto done;
    }

    Item    = ChkPck.VarList;
    Result  = HXtoi (Item->VarStr, &Offset);
    if (EFI_ERROR (Result)) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_ARG), HiiHandle, L"hexedit", Item->VarStr);
      Status = EFI_INVALID_PARAMETER;
      goto done;
    }

    Item    = Item->Next;
    Result  = HXtoi (Item->VarStr, &Size);
    if (EFI_ERROR (Result)) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_ARG), HiiHandle, L"hexedit", Item->VarStr);
      Status = EFI_INVALID_PARAMETER;
      goto done;
    }

    if (Offset < 0 || Size <= 0) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_ARG), HiiHandle, L"hexedit", Item->VarStr);
      Status = EFI_INVALID_PARAMETER;
      goto done;
    }

    LastOffset = (UINT64) Offset + (UINT64) Size - (UINT64) 1;
    if (LastOffset > 0xffffffff) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_ARG), HiiHandle, L"hexedit", Item->VarStr);
      goto done;
    }

    WhatToDo = OPEN_MEMORY;
  }

  Item = LibCheckVarGetFlag (&ChkPck, L"-f");
  if (Item) {
    if (1 < ChkPck.ValueCount) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiHandle, L"hexedit");
      Status = EFI_INVALID_PARAMETER;
      goto done;
    }

    if (1 > ChkPck.ValueCount) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_FEW), HiiHandle, L"hexedit");
      Status = EFI_INVALID_PARAMETER;
      goto done;
    }

    Item  = ChkPck.VarList;
    Name  = Item->VarStr;
    if (!HIsValidFileName (Name)) {
      PrintToken (STRING_TOKEN (STR_HEXEDIT_FILE_NAME), HiiHandle);
      Status = EFI_INVALID_PARAMETER;
      goto done;
    }

    WhatToDo = OPEN_FILE;
  } else if (0 == ChkPck.FlagCount || (ChkPck.FlagCount == 1 && LibCheckVarGetFlag (&ChkPck, L"-b") != NULL)) {
    if (0 == ChkPck.ValueCount) {
      Name      = HexEditGetDefaultFileName ();
      FreeName  = TRUE;
    } else if (1 == ChkPck.ValueCount) {
      Name = ChkPck.VarList->VarStr;
    } else {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiHandle, L"hexedit");
      Status = EFI_INVALID_PARAMETER;
      goto done;
    }

    if (!HIsValidFileName (Name)) {
      PrintToken (STRING_TOKEN (STR_HEXEDIT_FILE_NAME), HiiHandle);
      Status = EFI_INVALID_PARAMETER;
      goto done;
    }

    WhatToDo = OPEN_FILE;
  }

  if (SI->RedirArgc != 0) {
    PrintToken (STRING_TOKEN (STR_HEXEDIT_NOREDIRECT), HiiHandle);
    Status = EFI_INVALID_PARAMETER;
    goto done;
  }

  Status = HMainEditorInit ();
  if (EFI_ERROR (Status)) {
    Out->ClearScreen (Out);
    Out->EnableCursor (Out, TRUE);
    PrintToken (STRING_TOKEN (STR_HEXEDIT_INIT_FAILED), HiiHandle);
    goto done;
  }

  HMainEditorBackup ();

  switch (WhatToDo) {
  case OPEN_FILE:
    Status = HBufferImageRead (
              Name,
              NULL,
              0,
              0,
              0,
              0,
              FILE_BUFFER,
              FALSE
              );
    break;

  case OPEN_DISK:
    Status = HBufferImageRead (
              NULL,
              Name,
              Offset,
              Size,
              0,
              0,
              DISK_BUFFER,
              FALSE
              );
    break;

  case OPEN_MEMORY:
    Status = HBufferImageRead (
              NULL,
              NULL,
              0,
              0,
              (UINT32) Offset,
              Size,
              MEM_BUFFER,
              FALSE
              );
    break;

  }

  if (!EFI_ERROR (Status)) {
    HMainEditorRefresh ();
    //
    // Set Key Filter, to be sure that the output will no pause
    //
    Key = SE2->GetKeyFilter ();
    SE2->SetKeyFilter (EFI_OUTPUT_SCROLL);

    Status = HMainEditorKeyInput ();
    //
    // Set Key Filter to original value
    //
    SE2->SetKeyFilter (Key);
  }

  if (Status != EFI_OUT_OF_RESOURCES) {
    //
    // back up the status string
    //
    Buffer = PoolPrint (L"%s", HMainEditor.StatusBar->StatusString);
  }

  HMainEditorCleanup ();

  //
  // print editor exit code on screen
  //
  if (Status == EFI_SUCCESS) {
  } else if (Status == EFI_OUT_OF_RESOURCES) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_OUT_RESOURCE), HiiHandle, L"hexedit");
  } else {
    if (Buffer != NULL) {
      if (StrCmp (Buffer, L"") != 0) {
        //
        // print out the status string
        //
        PrintToken (STRING_TOKEN (STR_HEXEDIT_ONE_VAR), HiiHandle, Buffer);
      } else {
        PrintToken (STRING_TOKEN (STR_HEXEDIT_UNKNOWN_EDITOR), HiiHandle);
      }
    } else {
      PrintToken (STRING_TOKEN (STR_HEXEDIT_UNKNOWN_EDITOR), HiiHandle);
    }
  }

  if (Status != EFI_OUT_OF_RESOURCES) {
    HEditorFreePool (Buffer);
  }

  if (FreeName) {
    FreePool (Name);
  }

done:
  LibCheckVarFreeVarList (&ChkPck);
  LibUnInitializeStrings ();
  return Status;
}

EFI_STATUS
InitializeEFIHexEditorGetLineHelp (
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
  return LibCmdGetStringByToken (STRING_ARRAY_NAME, &EfiHexeditGuid, STRING_TOKEN (STR_HEXEDIT_LINE_HELP), Str);
}

CHAR16 *
HexEditGetDefaultFileName (
  VOID
  )
{
  EFI_STATUS         Status;
  UINTN              Suffix;
  BOOLEAN            FoundNewFile;
  CHAR16             *FileNameTmp;
  EFI_LIST_ENTRY     DirList;
  SHELL_FILE_ARG     *Arg;

  Suffix       = 0;
  FoundNewFile = FALSE;

  do {
    if (Suffix != 0) {
      FileNameTmp = PoolPrint (L"NewFile%d.bin", Suffix);
    } else {
      FileNameTmp = PoolPrint (L"NewFile.bin");
    }

    //
    // GET CURRENT DIR HANDLE
    //
    InitializeListHead (&DirList);

    //
    // after that filename changed to path
    //
    Status = ShellFileMetaArgNoWildCard (FileNameTmp, &DirList);

    if (EFI_ERROR (Status)) {
      break;
    }

    if (DirList.Flink == &DirList) {
      break;
    }

    Arg = CR (DirList.Flink, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);

    //
    // Make sure Arg is valid
    //
    if (Arg == NULL || Arg->Parent == NULL) {
      ShellFreeFileList (&DirList);
      break;
    }

    if (Arg->Status == EFI_NOT_FOUND) {
      FoundNewFile = TRUE;
      ShellFreeFileList (&DirList);
      break;
    } else {
      FreePool (FileNameTmp);
      FileNameTmp = NULL;
    }

    ShellFreeFileList (&DirList);
    Suffix++;

  } while (Suffix != 0);

  if (!FoundNewFile && FileNameTmp != NULL) {
    FreePool (FileNameTmp);
    FileNameTmp = NULL;
  }
  return FileNameTmp;
}
