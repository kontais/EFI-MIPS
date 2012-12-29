/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

  Module Name:
  libMenuBar.c

  Abstract:
  Definition of the Menu Bar for the text editor

--*/

#include "heditor.h"

EFI_STATUS
HMainCommandFileType (
  VOID
  );

HEFI_EDITOR_MENU_ITEM HMainMenuItems[] = {
  {
    STRING_TOKEN(STR_HEXEDIT_LIBMENUBAR_GO_TO_OFFSET),
    STRING_TOKEN(STR_HEXEDIT_LIBMENUBAR_F1),
    HMainCommandGoToOffset
  },
  {
    STRING_TOKEN(STR_HEXEDIT_LIBMENUBAR_SAVE_BUFFER),
    STRING_TOKEN(STR_HEXEDIT_LIBMENUBAR_F2),
    HMainCommandSaveBuffer
  },
  {
    STRING_TOKEN(STR_HEXEDIT_LIBMENUBAR_EXIT),
    STRING_TOKEN(STR_HEXEDIT_LIBMENUBAR_F3),
    HMainCommandExit
  },

  {
    STRING_TOKEN(STR_HEXEDIT_LIBMENUBAR_SELECT_START),
    STRING_TOKEN(STR_HEXEDIT_LIBMENUBAR_F4),
    HMainCommandSelectStart
  },
  {
    STRING_TOKEN(STR_HEXEDIT_LIBMENUBAR_SELECT_END),
    STRING_TOKEN(STR_HEXEDIT_LIBMENUBAR_F5),
    HMainCommandSelectEnd
  },
  {
    STRING_TOKEN(STR_HEXEDIT_LIBMENUBAR_CUT),
    STRING_TOKEN(STR_HEXEDIT_LIBMENUBAR_F6),
    HMainCommandCut
  },
  {
    STRING_TOKEN(STR_HEXEDIT_LIBMENUBAR_PASTE),
    STRING_TOKEN(STR_HEXEDIT_LIBMENUBAR_F7),
    HMainCommandPaste
  },

  {
    STRING_TOKEN(STR_HEXEDIT_LIBMENUBAR_OPEN_FILE),
    STRING_TOKEN(STR_HEXEDIT_LIBMENUBAR_F8),
    HMainCommandOpenFile
  },
  {
    STRING_TOKEN(STR_HEXEDIT_LIBMENUBAR_OPEN_DISK),
    STRING_TOKEN(STR_HEXEDIT_LIBMENUBAR_F9),
    HMainCommandOpenDisk
  },
  {
    STRING_TOKEN(STR_HEXEDIT_LIBMENUBAR_OPEN_MEMORY),
    STRING_TOKEN(STR_HEXEDIT_LIBMENUBAR_F10),
    HMainCommandOpenMemory
  },

  {
    0,
    0,
    NULL
  }
};

HEFI_EDITOR_MENU_BAR  HMainMenuBar;

extern BOOLEAN        HBufferImageNeedRefresh;
extern BOOLEAN        HBufferImageOnlyLineNeedRefresh;
extern BOOLEAN        HBufferImageMouseNeedRefresh;

//
// basic initialize for HMainMenuBar
//
HEFI_EDITOR_MENU_BAR  HMainMenuBarConst = { HMainMenuItems };

EFI_STATUS
HMainMenuBarInit (
  VOID
  )
/*++

Routine Description: 

  init function for HMainMenuBar

Arguments:  

  None

Returns:  

  EFI_SUCCESS

--*/
{
  CopyMem (&HMainMenuBar, &HMainMenuBarConst, sizeof (HEFI_EDITOR_MENU_BAR));

  return EFI_SUCCESS;
}

EFI_STATUS
HMainMenuBarCleanup (
  VOID
  )
/*++

Routine Description: 

  Cleanup function for HMainMenuBar

Arguments:  

  None

Returns:  

  EFI_SUCCESS

--*/
{
  return EFI_SUCCESS;
}

EFI_STATUS
HMainMenuBarBackup (
  VOID
  )
/*++

Routine Description: 

  backup function for HMainMenuBar

Arguments:  

  None

Returns:  

  EFI_SUCCESS

--*/
{
  return EFI_SUCCESS;
}

EFI_STATUS
HMainMenuBarHide (
  VOID
  )
/*++

Routine Description: 

  Clear menu bar

Arguments:  

  None

Returns:  

  EFI_SUCCESS

--*/
{
  HEditorClearLine (MENU_BAR_LOCATION);
  HEditorClearLine (MENU_BAR_LOCATION + 1);
  HEditorClearLine (MENU_BAR_LOCATION + 2);

  return EFI_SUCCESS;
}

EFI_STATUS
HMainMenuBarRefresh (
  VOID
  )
/*++

Routine Description: 

  refresh function for HMainMenuBar

Arguments:  

  None

Returns:  

  EFI_SUCCESS

--*/
{
  HEFI_EDITOR_MENU_ITEM *Item;
  UINTN                 Col;
  UINTN                 Row;
  UINTN                 Width;
  UINTN                 Index;
  CHAR16                *NameString;
  CHAR16                *FunctionKeyString;
  UINTN                 BufferSize;
  EFI_STATUS            Status;

  //
  // variable initialization
  //
  Col = 1;
  Row = MENU_BAR_LOCATION;

  //
  // since menu bar won't be changed after editor is launch.
  // so it's refresh only when editor is
  // just launched.
  //
  if (!HEditorFirst) {
    return EFI_SUCCESS;
  }
  //
  // clear menu bar
  //
  HMainMenuBarHide ();

  Index = 1;

  //
  // print out the menu items
  //
  for (Item = HMainMenuBar.MenuItems; Item->Function; Item++) {

    BufferSize        = 0x1000;
    NameString        = AllocatePool (BufferSize);
    FunctionKeyString = AllocatePool (BufferSize);

    Status            = Hii->GetString (Hii, HiiHandle, Item->NameToken, FALSE, NULL, &BufferSize, NameString);

    Width             = max ((StrLen (NameString) + 6), 18);
    if (((Col + Width) > MAX_TEXT_COLUMNS)) {
      Row++;
      Col = 1;
    }

    Status = Hii->GetString (Hii, HiiHandle, Item->FunctionKeyToken, FALSE, NULL, &BufferSize, FunctionKeyString);

    if (Index >= 10) {
      PrintAt (Col - 1, Row - 1, L"%E%s%N %H%s%N  ", FunctionKeyString, NameString);
    } else {
      PrintAt (Col - 1, Row - 1, L"%E%s%N  %H%s%N  ", FunctionKeyString, NameString);
    }

    FreePool (NameString);
    FreePool (FunctionKeyString);
    Col += Width;

    Index++;
  }
  //
  // restore cursor position in edit area
  //
  HBufferImageRestorePosition ();

  return EFI_SUCCESS;
}

EFI_STATUS
HMainMenuBarHandleInput (
  IN EFI_INPUT_KEY   *Key
  )
/*++

Routine Description: 

  Dispatch input to different handlers
  Input can handle: F1 ~ F10

Arguments:  

  Key -- Input key

Returns:  
  EFI_SUCCESS  -- Correctly handled
  Other  -- Failed

--*/
{
  HEFI_EDITOR_MENU_ITEM *Item;
  EFI_STATUS            Status;
  UINTN                 Index;
  UINTN                 NumItems;

  //
  // variable initialization
  //
  Index     = 0;
  NumItems  = 0;
  NumItems  = sizeof (HMainMenuItems) / sizeof (HEFI_EDITOR_MENU_ITEM) - 1;

  Item      = HMainMenuBar.MenuItems;

  Index     = Key->ScanCode - SCAN_CODE_F1;

  //
  // check whether is from F1~F10
  //
  if (Index > (NumItems - 1) || Index < 0) {
    HMainStatusBarSetStatusString (L"Unknown Command");
    return EFI_SUCCESS;
  }

  Item    = &HMainMenuBar.MenuItems[Index];

  Status  = Item->Function ();

  return Status;
}

EFI_STATUS
HMainCommandOpenFile (
  VOID
  )
/*++

Routine Description: 

  Load a file from disk to editor

Arguments:  

  None

Returns:  

   EFI_SUCCESS
   EFI_LOAD_ERROR
   EFI_OUT_OF_RESOURCES

--*/
{
  BOOLEAN                         Done;
  EFI_STATUS                      Status;
  HEFI_EDITOR_ACTIVE_BUFFER_TYPE  BufferType;

  BufferType = HMainEditor.BufferImage->BufferType;

  //
  //  This command will open a file from current working directory.
  //  Read-only file can also be opened. But it can not be modified.
  // Below is the scenario of Open File command:
  // 1. IF currently opened file has not been modified, directly go to step .
  //  IF currently opened file has been modified, an Input Bar will be
  //     prompted as :
  //      "File Modified. Save ( Yes/No/Cancel) ?"
  //          IF user press 'y' or 'Y', currently opened file will be saved.
  //          IF user press 'n' or 'N', currently opened file will
  //             not be saved.
  //          IF user press 'c' or 'C' or ESC, Open File command ends and
  //             currently opened file is still opened.
  //
  // 2. An Input Bar will be prompted as :  "File Name to Open: "
  //      IF user press ESC, Open File command ends and
  //         currently opened file is still opened.
  //      Any other inputs with a Return will cause
  //          currently opened file close.
  //
  // 3. IF user input file name is an existing file ,
  //       this file will be read and opened.
  //    IF user input file name is a new file, this file will be created
  //       and opened. This file's type ( UNICODE or ASCII ) is the same with
  //       the old file.
  //
  //
  // if current file is modified, so you need to choose whether to
  //    save it first.
  //
  if (HMainEditor.BufferImage->Modified) {

    Status = HMainInputBarSetPrompt (L"Buffer modified. Save (Yes/No/Cancel) ? ");
    if (EFI_ERROR (Status)) {
      return Status;
    }
    //
    // the answer is just one character
    //
    Status = HMainInputBarSetStringSize (1);
    if (EFI_ERROR (Status)) {
      return Status;
    }
    //
    // loop for user's answer
    // valid answer is just 'y' 'Y', 'n' 'N', 'c' 'C'
    //
    Done = FALSE;
    while (!Done) {
      Status = HMainInputBarRefresh ();

      //
      // ESC pressed
      //
      if (Status == EFI_NOT_READY) {
        return EFI_SUCCESS;
      }

      switch (HMainEditor.InputBar->ReturnString[0]) {
      case L'y':
      case L'Y':
        //
        // want to save this buffer first
        //
        Status = HBufferImageSave (
                  HMainEditor.BufferImage->FileImage->FileName,
                  HMainEditor.BufferImage->DiskImage->Name,
                  HMainEditor.BufferImage->DiskImage->Offset,
                  HMainEditor.BufferImage->DiskImage->Size,
                  HMainEditor.BufferImage->MemImage->Offset,
                  HMainEditor.BufferImage->MemImage->Size,
                  HMainEditor.BufferImage->BufferType
                  );
        if (EFI_ERROR (Status)) {
          return Status;
        }

        HMainTitleBarRefresh ();
        Done = TRUE;
        break;

      case L'n':
      case L'N':
        //
        // the file won't be saved
        //
        Done = TRUE;
        break;

      case L'c':
      case L'C':
        return EFI_SUCCESS;
      }
    }
  }
  //
  // TO get the open file name
  //
  Status = HMainInputBarSetPrompt (L"File Name to Open: ");
  if (EFI_ERROR (Status)) {
    HBufferImageRead (
      HMainEditor.BufferImage->FileImage->FileName,
      HMainEditor.BufferImage->DiskImage->Name,
      HMainEditor.BufferImage->DiskImage->Offset,
      HMainEditor.BufferImage->DiskImage->Size,
      HMainEditor.BufferImage->MemImage->Offset,
      HMainEditor.BufferImage->MemImage->Size,
      BufferType,
      TRUE
      );
    return Status;
  }

  Status = HMainInputBarSetStringSize (100);
  if (EFI_ERROR (Status)) {
    Status = HBufferImageRead (
              HMainEditor.BufferImage->FileImage->FileName,
              HMainEditor.BufferImage->DiskImage->Name,
              HMainEditor.BufferImage->DiskImage->Offset,
              HMainEditor.BufferImage->DiskImage->Size,
              HMainEditor.BufferImage->MemImage->Offset,
              HMainEditor.BufferImage->MemImage->Size,
              BufferType,
              TRUE
              );
    return Status;
  }

  while (1) {
    Status = HMainInputBarRefresh ();

    //
    // ESC pressed
    //
    if (Status == EFI_NOT_READY) {
      Status = HBufferImageRead (
                HMainEditor.BufferImage->FileImage->FileName,
                HMainEditor.BufferImage->DiskImage->Name,
                HMainEditor.BufferImage->DiskImage->Offset,
                HMainEditor.BufferImage->DiskImage->Size,
                HMainEditor.BufferImage->MemImage->Offset,
                HMainEditor.BufferImage->MemImage->Size,
                BufferType,
                TRUE
                );

      return Status;
    }
    //
    // THE input string length should > 0
    //
    if (StrLen (HMainEditor.InputBar->ReturnString) > 0) {
      //
      // CHECK if filename's valid
      //
      if (!HIsValidFileName (HMainEditor.InputBar->ReturnString)) {
        HBufferImageRead (
          HMainEditor.BufferImage->FileImage->FileName,
          HMainEditor.BufferImage->DiskImage->Name,
          HMainEditor.BufferImage->DiskImage->Offset,
          HMainEditor.BufferImage->DiskImage->Size,
          HMainEditor.BufferImage->MemImage->Offset,
          HMainEditor.BufferImage->MemImage->Size,
          BufferType,
          TRUE
          );

        HMainStatusBarSetStatusString (L"Invalid File Name");
        return EFI_SUCCESS;
      }

      break;
    }
  }
  //
  // read from disk
  //
  Status = HBufferImageRead (
            HMainEditor.InputBar->ReturnString,
            HMainEditor.BufferImage->DiskImage->Name,
            HMainEditor.BufferImage->DiskImage->Offset,
            HMainEditor.BufferImage->DiskImage->Size,
            HMainEditor.BufferImage->MemImage->Offset,
            HMainEditor.BufferImage->MemImage->Size,
            FILE_BUFFER,
            FALSE
            );

  if (EFI_ERROR (Status)) {
    HBufferImageRead (
      HMainEditor.BufferImage->FileImage->FileName,
      HMainEditor.BufferImage->DiskImage->Name,
      HMainEditor.BufferImage->DiskImage->Offset,
      HMainEditor.BufferImage->DiskImage->Size,
      HMainEditor.BufferImage->MemImage->Offset,
      HMainEditor.BufferImage->MemImage->Size,
      BufferType,
      TRUE
      );

    return EFI_LOAD_ERROR;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
HMainCommandOpenDisk (
  VOID
  )
/*++

Routine Description: 

  Load a disk buffer editor

Arguments:  

  None

Returns:  

   EFI_SUCCESS
   EFI_LOAD_ERROR
   EFI_OUT_OF_RESOURCES
   EFI_NOT_FOUND

--*/
{
  UINTN                           Size;
  UINTN                           Offset;
  CHAR16                          *DeviceName;
  EFI_STATUS                      Status;
  BOOLEAN                         Done;

  HEFI_EDITOR_ACTIVE_BUFFER_TYPE  BufferType;

  //
  // variable initialization
  //
  Size        = 0;
  Offset      = 0;
  BufferType  = HMainEditor.BufferImage->BufferType;

  //
  // if current file is modified, so you need to choose
  // whether to save it first.
  //
  if (HMainEditor.BufferImage->Modified) {

    Status = HMainInputBarSetPrompt (L"Buffer modified. Save (Yes/No/Cancel) ? ");
    if (EFI_ERROR (Status)) {
      return Status;
    }
    //
    // the answer is just one character
    //
    Status = HMainInputBarSetStringSize (1);
    if (EFI_ERROR (Status)) {
      return Status;
    }
    //
    // loop for user's answer
    // valid answer is just 'y' 'Y', 'n' 'N', 'c' 'C'
    //
    Done = FALSE;
    while (!Done) {
      Status = HMainInputBarRefresh ();

      //
      // ESC pressed
      //
      if (Status == EFI_NOT_READY) {
        return EFI_SUCCESS;
      }

      switch (HMainEditor.InputBar->ReturnString[0]) {
      case L'y':
      case L'Y':
        //
        // want to save this buffer first
        //
        Status = HBufferImageSave (
                  HMainEditor.BufferImage->FileImage->FileName,
                  HMainEditor.BufferImage->DiskImage->Name,
                  HMainEditor.BufferImage->DiskImage->Offset,
                  HMainEditor.BufferImage->DiskImage->Size,
                  HMainEditor.BufferImage->MemImage->Offset,
                  HMainEditor.BufferImage->MemImage->Size,
                  BufferType
                  );
        if (EFI_ERROR (Status)) {
          return Status;
        }

        HMainTitleBarRefresh ();
        Done = TRUE;
        break;

      case L'n':
      case L'N':
        //
        // the file won't be saved
        //
        Done = TRUE;
        break;

      case L'c':
      case L'C':
        return EFI_SUCCESS;
      }
    }
  }
  //
  // get disk block device name
  //
  Status = HMainInputBarSetPrompt (L"Block Device to Open: ");
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = HMainInputBarSetStringSize (100);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  while (1) {
    Status = HMainInputBarRefresh ();

    //
    // ESC pressed
    //
    if (Status == EFI_NOT_READY) {

      return EFI_SUCCESS;
    }
    //
    // THE input string length should > 0
    //
    if (StrLen (HMainEditor.InputBar->ReturnString) > 0) {
      break;
    }
  }

  DeviceName = PoolPrint (L"%s", HMainEditor.InputBar->ReturnString);
  if (DeviceName == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  //
  // get starting offset
  //
  Status = HMainInputBarSetPrompt (L"First Block No.: ");
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = HMainInputBarSetStringSize (16);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  while (1) {
    Status = HMainInputBarRefresh ();

    //
    // ESC pressed
    //
    if (Status == EFI_NOT_READY) {

      return EFI_SUCCESS;
    }
    //
    // THE input string length should > 0
    //
    if (StrLen (HMainEditor.InputBar->ReturnString) > 0) {
      Status = HXtoi (HMainEditor.InputBar->ReturnString, &Offset);
      if (EFI_ERROR (Status)) {
        continue;
      }

      if (Offset >= 0) {
        break;
      }
    }
  }
  //
  // get Number of Blocks:
  //
  Status = HMainInputBarSetPrompt (L"Number of Blocks: ");
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = HMainInputBarSetStringSize (8);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  while (1) {
    Status = HMainInputBarRefresh ();

    //
    // ESC pressed
    //
    if (Status == EFI_NOT_READY) {

      return EFI_SUCCESS;
    }
    //
    // THE input string length should > 0
    //
    if (StrLen (HMainEditor.InputBar->ReturnString) > 0) {
      Status = HXtoi (HMainEditor.InputBar->ReturnString, &Size);
      if (EFI_ERROR (Status)) {
        continue;
      }

      if (Size == 0) {
        continue;
      }

      break;
    }
  }

  Status = HBufferImageRead (
            NULL,
            DeviceName,
            Offset,
            Size,
            0,
            0,
            DISK_BUFFER,
            FALSE
            );

  if (EFI_ERROR (Status)) {

    HBufferImageRead (
      HMainEditor.BufferImage->FileImage->FileName,
      HMainEditor.BufferImage->DiskImage->Name,
      HMainEditor.BufferImage->DiskImage->Offset,
      HMainEditor.BufferImage->DiskImage->Size,
      HMainEditor.BufferImage->MemImage->Offset,
      HMainEditor.BufferImage->MemImage->Size,
      BufferType,
      TRUE
      );
    return EFI_NOT_FOUND;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
HMainCommandOpenMemory (
  VOID
  )
/*++

Routine Description: 

  Load memory content to editor

Arguments:  

  None

Returns:  

   EFI_SUCCESS
   EFI_LOAD_ERROR
   EFI_OUT_OF_RESOURCES
   EFI_NOT_FOUND

--*/
{
  UINTN                           Size;
  UINTN                           Offset;
  EFI_STATUS                      Status;
  BOOLEAN                         Done;
  UINT64                          LastOffset;

  HEFI_EDITOR_ACTIVE_BUFFER_TYPE  BufferType;

  //
  // variable initialization
  //
  Size        = 0;
  Offset      = 0;
  BufferType  = HMainEditor.BufferImage->BufferType;

  //
  // if current buffer is modified, so you need to choose
  // whether to save it first.
  //
  if (HMainEditor.BufferImage->Modified) {

    Status = HMainInputBarSetPrompt (L"Buffer modified. Save (Yes/No/Cancel) ? ");
    if (EFI_ERROR (Status)) {
      return Status;
    }
    //
    // the answer is just one character
    //
    Status = HMainInputBarSetStringSize (1);
    if (EFI_ERROR (Status)) {
      return Status;
    }
    //
    // loop for user's answer
    // valid answer is just 'y' 'Y', 'n' 'N', 'c' 'C'
    //
    Done = FALSE;
    while (!Done) {
      Status = HMainInputBarRefresh ();

      //
      // ESC pressed
      //
      if (Status == EFI_NOT_READY) {
        return EFI_SUCCESS;
      }

      switch (HMainEditor.InputBar->ReturnString[0]) {
      case L'y':
      case L'Y':
        //
        // want to save this buffer first
        //
        Status = HBufferImageSave (
                  HMainEditor.BufferImage->FileImage->FileName,
                  HMainEditor.BufferImage->DiskImage->Name,
                  HMainEditor.BufferImage->DiskImage->Offset,
                  HMainEditor.BufferImage->DiskImage->Size,
                  HMainEditor.BufferImage->MemImage->Offset,
                  HMainEditor.BufferImage->MemImage->Size,
                  BufferType
                  );
        if (EFI_ERROR (Status)) {
          return Status;
        }

        HMainTitleBarRefresh ();
        Done = TRUE;
        break;

      case L'n':
      case L'N':
        //
        // the file won't be saved
        //
        Done = TRUE;
        break;

      case L'c':
      case L'C':
        return EFI_SUCCESS;
      }
    }
  }
  //
  // get starting offset
  //
  Status = HMainInputBarSetPrompt (L"Starting Offset: ");
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = HMainInputBarSetStringSize (8);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  while (1) {
    Status = HMainInputBarRefresh ();

    //
    // ESC pressed
    //
    if (Status == EFI_NOT_READY) {

      return EFI_SUCCESS;
    }
    //
    // THE input string length should > 0
    //
    if (StrLen (HMainEditor.InputBar->ReturnString) > 0) {
      Status = HXtoi (HMainEditor.InputBar->ReturnString, &Offset);
      if (EFI_ERROR (Status)) {
        continue;
      }

      break;
    }
  }
  //
  // get Number of Blocks:
  //
  Status = HMainInputBarSetPrompt (L"Buffer Size: ");
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = HMainInputBarSetStringSize (8);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  while (1) {
    Status = HMainInputBarRefresh ();

    //
    // ESC pressed
    //
    if (Status == EFI_NOT_READY) {

      return EFI_SUCCESS;
    }
    //
    // THE input string length should > 0
    //
    if (StrLen (HMainEditor.InputBar->ReturnString) > 0) {
      Status = HXtoi (HMainEditor.InputBar->ReturnString, &Size);
      if (EFI_ERROR (Status)) {
        continue;
      }

      if (Size == 0) {
        continue;
      }

      break;
    }
  }

  LastOffset = (UINT64) Offset + (UINT64) Size - (UINT64) 1;
  if (LastOffset > 0xffffffff) {
    HMainStatusBarSetStatusString (L"Invalid parameter");
    return EFI_LOAD_ERROR;
  }

  Status = HBufferImageRead (
            NULL,
            NULL,
            0,
            0,
            Offset,
            Size,
            MEM_BUFFER,
            FALSE
            );

  if (EFI_ERROR (Status)) {
    HMainStatusBarSetStatusString (L"Read Device Error!");
    HBufferImageRead (
      HMainEditor.BufferImage->FileImage->FileName,
      HMainEditor.BufferImage->DiskImage->Name,
      HMainEditor.BufferImage->DiskImage->Offset,
      HMainEditor.BufferImage->DiskImage->Size,
      HMainEditor.BufferImage->MemImage->Offset,
      HMainEditor.BufferImage->MemImage->Size,
      BufferType,
      TRUE
      );
    return EFI_NOT_FOUND;
  }

  return EFI_SUCCESS;

}

EFI_STATUS
HMainCommandSaveBuffer (
  VOID
  )
/*++

Routine Description: 

  save current opened buffer . 
  if is file buffer, you can save to current file name or 
  save to another file name

Arguments:  

  None

Returns:  

  EFI_SUCCESS
  EFI_OUT_OF_RESOURCES
  EFI_LOAD_ERROR

--*/
{
  EFI_STATUS      Status;
  EFI_FILE_HANDLE Handle;
  BOOLEAN         Done;
  CHAR16          *FileName;
  BOOLEAN         OldFile;
  BOOLEAN         ReadOnly;
  CHAR16          *Str;
  EFI_LIST_ENTRY  DirList;
  SHELL_FILE_ARG  *Arg;
  EFI_FILE_INFO   *Info;
  CHAR16          *FileNameTmp;

  if (HMainEditor.BufferImage->BufferType != FILE_BUFFER) {
    if (HMainEditor.BufferImage->Modified == FALSE) {
      return EFI_SUCCESS;
    }

    Status = HMainInputBarSetPrompt (L"Dangerous to save disk/mem buffer. Save (Yes/No/Cancel) ? ");
    if (EFI_ERROR (Status)) {
      return Status;
    }
    //
    // the answer is just one character
    //
    Status = HMainInputBarSetStringSize (1);
    if (EFI_ERROR (Status)) {
      return Status;
    }
    //
    // loop for user's answer
    // valid answer is just 'y' 'Y', 'n' 'N', 'c' 'C'
    //
    while (1) {
      Status = HMainInputBarRefresh ();

      //
      // ESC pressed
      //
      if (Status == EFI_NOT_READY) {
        return EFI_SUCCESS;
      }

      switch (HMainEditor.InputBar->ReturnString[0]) {
      case L'y':
      case L'Y':
        //
        // want to save this buffer first
        //
        Status = HBufferImageSave (
                  NULL,
                  HMainEditor.BufferImage->DiskImage->Name,
                  HMainEditor.BufferImage->DiskImage->Offset,
                  HMainEditor.BufferImage->DiskImage->Size,
                  HMainEditor.BufferImage->MemImage->Offset,
                  HMainEditor.BufferImage->MemImage->Size,
                  HMainEditor.BufferImage->BufferType
                  );

        if (EFI_ERROR (Status)) {
          HMainStatusBarSetStatusString (L"BufferSave: Problems Writing");
          return Status;
        }

        return EFI_SUCCESS;

      case L'n':
      case L'N':
        //
        // the file won't be saved
        //
        return EFI_SUCCESS;
        break;

      case L'c':
      case L'C':
        return EFI_SUCCESS;
      }
      //
      // end of switch
      //
    }
    //
    // ENDOF WHILE
    //
  }
  //
  // ENDOF != FILEBUFFER
  //
  // This command will save currently opened file to disk.
  // You can choose save to another file name or just save to
  // current file name.
  // Below is the scenario of Save File command: (
  //    Suppose the old file name is A )
  // 1. An Input Bar will be prompted:    "File To Save: [ old file name]"
  //    IF user press ESC, Save File command ends .
  //    IF user press Enter, input file name will be A.
  //    IF user inputs a new file name B,  input file name will be B.
  //
  // 2. IF input file name is A, go to do Step 3.
  //    IF input file name is B, go to do Step 4.
  //
  // 3. IF A is read only, Status Bar will show "Access Denied"
  //       and Save File commands ends.
  //    IF A is not read only, save file buffer to disk
  //       and remove Modified flag in Title Bar , then Save File command ends.
  //
  // 4. IF B does not exist, create this file and save file buffer to it.
  //       Go to do Step 7.
  //    IF B exits, do Step 5.
  //
  // 5. An Input Bar will be prompted:
  //       "File Exists. Overwrite ( Yes/No/Cancel ) ?"
  //      IF user press 'y' or 'Y', do Step 6.
  //      IF user press 'n' or 'N', Save File commands ends.
  //      IF user press 'c' or 'C' or ESC, Save File commands ends.
  //
  // 6. IF B is a read-only file, Status Bar will show "Access Denied"
  //       and Save File commands ends.
  //    IF B can be read and write, save file buffer to B.
  //
  // 7. Update File Name field in Title Bar to B
  //       and remove the Modified flag in Title Bar.
  //
  Str = PoolPrint (
          L"File to Save: [%s]",
          HMainEditor.BufferImage->FileImage->FileName
          );
  if (Str == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  if (StrLen (Str) >= 50) {
    //
    // replace the long file name with "..."
    //
    Str[46] = L'.';
    Str[47] = L'.';
    Str[48] = L'.';
    Str[49] = L']';
    Str[50] = L'\0';
  }

  Status = HMainInputBarSetPrompt (Str);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = HMainInputBarSetStringSize (100);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // get new file name
  //
  Status = HMainInputBarRefresh ();

  //
  // if user pressed ESC
  //
  if (Status == EFI_NOT_READY) {
    HEditorFreePool (Str);
    return EFI_SUCCESS;
  }

  HEditorFreePool (Str);

  //
  // if just enter pressed, so think save to current file name
  //
  if (StrLen (HMainEditor.InputBar->ReturnString) == 0) {
    FileName = PoolPrint (
                L"%s",
                HMainEditor.BufferImage->FileImage->FileName
                );
  } else {
    FileName = PoolPrint (L"%s", HMainEditor.InputBar->ReturnString);
  }

  if (FileName == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  if (!HIsValidFileName (FileName)) {
    HMainStatusBarSetStatusString (L"Invalid File Name");
    FreePool (FileName);
    return EFI_SUCCESS;
  }

  OldFile = FALSE;

  //
  // save to the old file
  //
  if (StriCmp (
        FileName,
        HMainEditor.BufferImage->FileImage->FileName
        ) == 0) {
    OldFile = TRUE;
  }

  if (OldFile) {
    //
    // if the file is read only, so can not write back to it.
    //
    if (HMainEditor.BufferImage->FileImage->ReadOnly == TRUE) {
      HMainStatusBarSetStatusString (L"Access Denied");
      FreePool (FileName);
      return EFI_SUCCESS;
    }
  } else {
    //
    // save to a new file name
    //
    FileNameTmp = PoolPrint (L"%s", FileName);
    if (FileNameTmp == NULL) {
      FreePool (FileName);
      return EFI_OUT_OF_RESOURCES;
    }
    //
    // GET CURRENT DIR HANDLE
    //
    InitializeListHead (&DirList);
    Status = ShellFileMetaArgNoWildCard (FileNameTmp, &DirList);
    FreePool (FileNameTmp);

    if (EFI_ERROR (Status)) {
      HEditorFreePool (FileName);
      return EFI_LOAD_ERROR;
    }

    if (DirList.Flink == &DirList) {
      HEditorFreePool (FileName);
      return EFI_LOAD_ERROR;
    }

    Arg = CR (DirList.Flink, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);

    //
    // since when Status is returned correct, not everything is correct,
    // so  I have to add these check (-:(((((
    //
    if (Arg == NULL) {
      HEditorFreePool (FileName);
      ShellFreeFileList (&DirList);
      return EFI_LOAD_ERROR;
    }

    if (Arg->Parent == NULL) {
      HEditorFreePool (FileName);
      ShellFreeFileList (&DirList);
      return EFI_LOAD_ERROR;
    }

    Info = Arg->Info;

    //
    // file exists
    //
    if (!Arg->Status) {
      if (!Arg->Handle) {
        Status = Arg->Parent->Open (
                                Arg->Parent,
                                &Arg->Handle,
                                Arg->FileName,
                                EFI_FILE_MODE_READ,
                                0
                                );
      }

      Handle  = Arg->Handle;

      Status  = HFileImageGetFileInfo (Handle, Arg->FileName, &Info);
      if (EFI_ERROR (Status)) {
        HEditorFreePool (FileName);
        ShellFreeFileList (&DirList);
        return EFI_LOAD_ERROR;
      }
      //
      // check if read only
      //
      if (Info->Attribute & EFI_FILE_READ_ONLY) {
        ReadOnly = TRUE;
      } else {
        ReadOnly = FALSE;
      }

      //
      // ask user whether to overwrite this file
      //
      Status = HMainInputBarSetPrompt (L"File exists. Overwrite (Yes/No/Cancel) ? ");
      if (EFI_ERROR (Status)) {
        HEditorFreePool (FileName);
        ShellFreeFileList (&DirList);
        return Status;
      }

      Status = HMainInputBarSetStringSize (1);
      if (EFI_ERROR (Status)) {
        HEditorFreePool (FileName);
        ShellFreeFileList (&DirList);
        return Status;
      }

      Done = FALSE;
      while (!Done) {
        Status = HMainInputBarRefresh ();

        //
        // ESC pressed
        //
        if (Status == EFI_NOT_READY) {
          HEditorFreePool (FileName);
          ShellFreeFileList (&DirList);
          return EFI_SUCCESS;
        }

        switch (HMainEditor.InputBar->ReturnString[0]) {
        case L'y':
        case L'Y':
          if (ReadOnly) {
            //
            // file is read-only, can not be overwrote
            //
            HMainStatusBarSetStatusString (L"Access Denied");
            HEditorFreePool (FileName);
            ShellFreeFileList (&DirList);
            return EFI_SUCCESS;
          }

          Done = TRUE;
          break;

        case L'n':
        case L'N':
          HEditorFreePool (FileName);
          ShellFreeFileList (&DirList);
          return EFI_SUCCESS;

        case L'c':
        case L'C':
          HEditorFreePool (FileName);
          ShellFreeFileList (&DirList);
          return EFI_SUCCESS;
        }
        //
        // end switch
        //
      }
      //
      // end while
      //
    }
    //
    // end file exists
    //
    ShellFreeFileList (&DirList);

  }
  //
  // end of if OldFile
  //
  // save file back to disk
  //
  Status = HBufferImageSave (
            FileName,
            HMainEditor.BufferImage->DiskImage->Name,
            HMainEditor.BufferImage->DiskImage->Offset,
            HMainEditor.BufferImage->DiskImage->Size,
            HMainEditor.BufferImage->MemImage->Offset,
            HMainEditor.BufferImage->MemImage->Size,
            FILE_BUFFER
            );
  HEditorFreePool (FileName);

  if (EFI_ERROR (Status)) {
    return EFI_LOAD_ERROR;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
HMainCommandSelectStart (
  VOID
  )
/*++

Routine Description: 

  Load a disk buffer editor

Arguments:  

  None

Returns:  

   EFI_SUCCESS
   EFI_LOAD_ERROR
   EFI_OUT_OF_RESOURCES

--*/
{
  UINTN Start;

  Start = (HMainEditor.BufferImage->BufferPosition.Row - 1) * 0x10 + HMainEditor.BufferImage->BufferPosition.Column;

  //
  // last line
  //
  if (HMainEditor.BufferImage->CurrentLine->Link.Flink == HMainEditor.BufferImage->ListHead) {
    if (HMainEditor.BufferImage->BufferPosition.Column > HMainEditor.BufferImage->CurrentLine->Size) {
      HMainStatusBarSetStatusString (L"Invalid Block Start");
      return EFI_LOAD_ERROR;
    }
  }

  if (HMainEditor.SelectEnd != 0 && Start > HMainEditor.SelectEnd) {
    HMainStatusBarSetStatusString (L"Invalid Block Start");
    return EFI_LOAD_ERROR;
  }

  HMainEditor.SelectStart = Start;

  HBufferImageNeedRefresh = TRUE;

  return EFI_SUCCESS;
}

EFI_STATUS
HMainCommandSelectEnd (
  VOID
  )
/*++

Routine Description: 

  Load a disk buffer editor

Arguments:  

  None

Returns:  

   EFI_SUCCESS
   EFI_LOAD_ERROR
   EFI_OUT_OF_RESOURCES

--*/
{
  UINTN End;

  End = (HMainEditor.BufferImage->BufferPosition.Row - 1) * 0x10 + HMainEditor.BufferImage->BufferPosition.Column;

  //
  // last line
  //
  if (HMainEditor.BufferImage->CurrentLine->Link.Flink == HMainEditor.BufferImage->ListHead) {
    if (HMainEditor.BufferImage->BufferPosition.Column > HMainEditor.BufferImage->CurrentLine->Size) {
      HMainStatusBarSetStatusString (L"Invalid Block End");
      return EFI_LOAD_ERROR;
    }
  }

  if (HMainEditor.SelectStart != 0 && End < HMainEditor.SelectStart) {
    HMainStatusBarSetStatusString (L"Invalid Block End");
    return EFI_SUCCESS;
  }

  HMainEditor.SelectEnd   = End;

  HBufferImageNeedRefresh = TRUE;

  return EFI_SUCCESS;
}

EFI_STATUS
HMainCommandCut (
  VOID
  )
/*++

Routine Description: 

  cut current line to clipboard

Arguments:  

  None

Returns:  

  EFI_SUCCESS
  EFI_OUT_OF_RESOURCES
  EFI_LOAD_ERROR

--*/
{
  UINTN             Index;
  HEFI_EDITOR_LINE  *Line;
  EFI_LIST_ENTRY    *Link;
  UINT8             *Buffer;
  UINTN             Count;

  //
  // not select, so not allowed to cut
  //
  if (HMainEditor.SelectStart == 0) {
    HMainStatusBarSetStatusString (L"No Block is Selected");
    return EFI_SUCCESS;
  }
  //
  // not select, so not allowed to cut
  //
  if (HMainEditor.SelectEnd == 0) {
    HMainStatusBarSetStatusString (L"No Block is Selected");
    return EFI_SUCCESS;
  }

  Link = HMainEditor.BufferImage->ListHead->Flink;
  for (Index = 0; Index < (HMainEditor.SelectEnd - 1) / 0x10; Index++) {
    Link = Link->Flink;
  }

  Line    = CR (Link, HEFI_EDITOR_LINE, Link, EFI_EDITOR_LINE_LIST);

  Count   = HMainEditor.SelectEnd - HMainEditor.SelectStart + 1;
  Buffer  = AllocatePool (Count);
  if (Buffer == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  //
  // cut the selected area
  //
  HBufferImageDeleteCharacterFromBuffer (
    HMainEditor.SelectStart - 1,
    Count,
    Buffer
    );

  //
  // put to clipboard
  //
  HClipBoardSet (Buffer, Count);

  HBufferImageNeedRefresh         = TRUE;
  HBufferImageOnlyLineNeedRefresh = FALSE;

  if (!HMainEditor.BufferImage->Modified) {
    HMainEditor.BufferImage->Modified = TRUE;
  }

  //
  // now no select area
  //
  HMainEditor.SelectStart = 0;
  HMainEditor.SelectEnd   = 0;

  return EFI_SUCCESS;
}

EFI_STATUS
HMainCommandPaste (
  VOID
  )
/*++

Routine Description: 

  paste line to file buffer

Arguments:  

  None

Returns:  

  EFI_SUCCESS
  EFI_OUT_OF_RESOURCES
  EFI_LOAD_ERROR


--*/
{

  BOOLEAN           OnlyLineRefresh;
  HEFI_EDITOR_LINE  *Line;
  UINT8             *Buffer;
  UINTN             Count;
  UINTN             FPos;

  Count = HClipBoardGet (&Buffer);
  if (Count == 0 || Buffer == NULL) {
    HMainStatusBarSetStatusString (L"Nothing to Paste");
    return EFI_SUCCESS;
  }

  Line            = HMainEditor.BufferImage->CurrentLine;

  OnlyLineRefresh = FALSE;
  if (Line->Link.Flink == HMainEditor.BufferImage->ListHead && Line->Size + Count < 0x10) {
    //
    // is at last line, and after paste will not exceed
    // so only this line need to be refreshed
    //
    // if after add, the line is 0x10, then will append a new line
    // so the whole page will need be refreshed
    //
    OnlyLineRefresh = TRUE;

  }

  FPos = 0x10 * (HMainEditor.BufferImage->BufferPosition.Row - 1) + HMainEditor.BufferImage->BufferPosition.Column - 1;

  HBufferImageAddCharacterToBuffer (FPos, Count, Buffer);

  if (OnlyLineRefresh) {
    HBufferImageNeedRefresh         = FALSE;
    HBufferImageOnlyLineNeedRefresh = TRUE;
  } else {
    HBufferImageNeedRefresh         = TRUE;
    HBufferImageOnlyLineNeedRefresh = FALSE;
  }

  if (!HMainEditor.BufferImage->Modified) {
    HMainEditor.BufferImage->Modified = TRUE;
  }

  return EFI_SUCCESS;

}

EFI_STATUS
HMainCommandGoToOffset (
  VOID
  )
/*++

Routine Description: 

  move cursor to specified lines

Arguments:  

  None

Returns:  

  EFI_SUCCESS

--*/
{
  UINTN       Size;
  UINTN       Offset;
  EFI_STATUS  Status;
  UINTN       FRow;
  UINTN       FCol;

  //
  // variable initialization
  //
  Size    = 0;
  Offset  = 0;
  FRow    = 0;
  FCol    = 0;

  //
  // get offset
  //
  Status = HMainInputBarSetPrompt (L"Go To Offset: ");
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = HMainInputBarSetStringSize (8);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  while (1) {
    Status = HMainInputBarRefresh ();

    //
    // ESC pressed
    //
    if (Status == EFI_NOT_READY) {

      return EFI_SUCCESS;
    }
    //
    // THE input string length should > 0
    //
    if (StrLen (HMainEditor.InputBar->ReturnString) > 0) {
      Status = HXtoi (HMainEditor.InputBar->ReturnString, &Offset);
      if (EFI_ERROR (Status)) {
        HMainStatusBarSetStatusString (L"Invalid Offset");
        return EFI_SUCCESS;
      }

      break;
    }
  }

  Size = HBufferImageGetTotalSize ();
  if (Offset >= Size) {
    HMainStatusBarSetStatusString (L"Invalid Offset");
    return EFI_SUCCESS;
  }

  FRow  = Offset / 0x10 + 1;
  FCol  = Offset % 0x10 + 1;

  HBufferImageMovePosition (FRow, FCol, TRUE);

  HBufferImageNeedRefresh         = TRUE;
  HBufferImageOnlyLineNeedRefresh = FALSE;
  HBufferImageMouseNeedRefresh    = TRUE;

  return EFI_SUCCESS;
}

EFI_STATUS
HMainCommandExit (
  VOID
  )
/*++

Routine Description: 

  exit editor

Arguments:  

  None

Returns:  

  EFI_SUCCESS
  EFI_OUT_OF_RESOURCES
  EFI_LOAD_ERROR

--*/
{
  EFI_STATUS  Status;

  //
  // Below is the scenario of Exit command:
  // 1. IF currently opened file is not modified, exit the editor and
  //       Exit command ends.
  //    IF currently opened file is modified, do Step 2
  //
  // 2. An Input Bar will be prompted:
  //       "File modified. Save ( Yes/No/Cancel )?"
  //      IF user press 'y' or 'Y', currently opened file will be saved and
  //         Editor exits
  //      IF user press 'n' or 'N', currently opened file will not be saved
  //         and Editor exits.
  //      IF user press 'c' or 'C' or ESC, Exit command ends.
  //
  //
  // if file has been modified, so will prompt user
  //       whether to save the changes
  //
  if (HMainEditor.BufferImage->Modified) {

    Status = HMainInputBarSetPrompt (L"Buffer modified. Save (Yes/No/Cancel) ? ");
    if (EFI_ERROR (Status)) {
      return Status;
    }

    Status = HMainInputBarSetStringSize (1);
    if (EFI_ERROR (Status)) {
      return Status;
    }

    while (1) {
      Status = HMainInputBarRefresh ();

      //
      // ESC pressed
      //
      if (Status == EFI_NOT_READY) {
        return EFI_SUCCESS;
      }

      switch (HMainEditor.InputBar->ReturnString[0]) {
      case L'y':
      case L'Y':
        //
        // write file back to disk
        //
        Status = HBufferImageSave (
                  HMainEditor.BufferImage->FileImage->FileName,
                  HMainEditor.BufferImage->DiskImage->Name,
                  HMainEditor.BufferImage->DiskImage->Offset,
                  HMainEditor.BufferImage->DiskImage->Size,
                  HMainEditor.BufferImage->MemImage->Offset,
                  HMainEditor.BufferImage->MemImage->Size,
                  HMainEditor.BufferImage->BufferType
                  );
        if (!EFI_ERROR (Status)) {
          HEditorExit = TRUE;
        }

        return Status;

      case L'n':
      case L'N':
        HEditorExit = TRUE;
        return EFI_SUCCESS;

      case L'c':
      case L'C':
        return EFI_SUCCESS;

      }
    }
  }

  HEditorExit = TRUE;
  return EFI_SUCCESS;

}
