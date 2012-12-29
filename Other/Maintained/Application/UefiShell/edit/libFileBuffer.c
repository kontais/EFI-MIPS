/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

  Module Name:
    libFileBuffer.c

  Abstract:
    Defines FileBuffer - the view of the file that is visible at any point, 
    as well as the event handlers for editing the file
--*/

#include "editor.h"

extern EFI_HANDLE       ImageHandleBackup;

EFI_EDITOR_FILE_BUFFER  FileBuffer;
EFI_EDITOR_FILE_BUFFER  FileBufferBackupVar;

//
// for basic initialization of FileBuffer
//
EFI_EDITOR_FILE_BUFFER  FileBufferConst = {
  NULL,
  UNICODE_FILE,
  NULL,
  NULL,
  0,
  {
    0,
    0
  },
  {
    0,
    0
  },
  {
    0,
    0
  },
  {
    0,
    0
  },
  FALSE,
  TRUE,
  FALSE,
  NULL
};

//
// the whole edit area needs to be refreshed
//
BOOLEAN                 FileBufferNeedRefresh;

//
// only the current line in edit area needs to be refresh
//
BOOLEAN                 FileBufferOnlyLineNeedRefresh;

BOOLEAN                 FileBufferMouseNeedRefresh;

extern BOOLEAN          EditorMouseAction;

//
// Name:
//   FileBufferInit -- Initialization function for FileBuffer
// In:
//   VOID
// Out:
//   EFI_SUCCESS
//   EFI_LOAD_ERROR
//
EFI_STATUS
FileBufferInit (
  VOID
  )
{
  //
  // basically initialize the FileBuffer
  //
  CopyMem (&FileBuffer, &FileBufferConst, sizeof (FileBuffer));
  CopyMem (
    &FileBufferBackupVar,
    &FileBufferConst,
    sizeof (FileBufferBackupVar)
    );

  //
  // default set FileName to NewFile.txt
  //
  FileBuffer.FileName = EditGetDefaultFileName ();
  if (FileBuffer.FileName == NULL) {
    return EFI_LOAD_ERROR;
  }

  FileBuffer.ListHead = AllocatePool (sizeof (EFI_LIST_ENTRY));
  if (FileBuffer.ListHead == NULL) {
    return EFI_LOAD_ERROR;
  }

  InitializeListHead (FileBuffer.ListHead);

  FileBuffer.DisplayPosition.Row    = TEXT_START_ROW;
  FileBuffer.DisplayPosition.Column = TEXT_START_COLUMN;
  FileBuffer.LowVisibleRange.Row    = TEXT_START_ROW;
  FileBuffer.LowVisibleRange.Column = TEXT_START_COLUMN;

  FileBufferNeedRefresh             = FALSE;
  FileBufferMouseNeedRefresh        = FALSE;
  FileBufferOnlyLineNeedRefresh     = FALSE;

  return EFI_SUCCESS;
}
//
// Name:
//   FileBufferBackup -- Backup function for FileBuffer
//   Only a few fields need to be backup.
//   This is for making the file buffer refresh
//   as few as possible.
// In:
//   VOID
// Out:
//   EFI_SUCCESS
//
EFI_STATUS
FileBufferBackup (
  VOID
  )
{
  //
  // backup the following items
  //      Mouse/Cursor position
  //      File Name, Type, ReadOnly, Modified
  //      Insert Mode
  //
  FileBufferBackupVar.MousePosition = FileBuffer.MousePosition;

  EditorFreePool (FileBufferBackupVar.FileName);
  FileBufferBackupVar.FileName        = StrDuplicate (FileBuffer.FileName);

  FileBufferBackupVar.ModeInsert      = FileBuffer.ModeInsert;
  FileBufferBackupVar.FileType        = FileBuffer.FileType;

  FileBufferBackupVar.FilePosition    = FileBuffer.FilePosition;
  FileBufferBackupVar.LowVisibleRange = FileBuffer.LowVisibleRange;

  FileBufferBackupVar.FileModified    = FileBuffer.FileModified;
  FileBufferBackupVar.ReadOnly        = FileBuffer.ReadOnly;

  return EFI_SUCCESS;
}

EFI_STATUS
FileBufferRestoreMousePosition (
  VOID
  )
{
  EFI_EDITOR_COLOR_UNION  Orig;
  EFI_EDITOR_COLOR_UNION  New;
  UINTN                   FRow;
  UINTN                   FColumn;
  BOOLEAN                 HasCharacter;
  EFI_EDITOR_LINE         *CurrentLine;
  EFI_EDITOR_LINE         *Line;
  CHAR16                  Value;

  //
  // variable initialization
  //
  Line = NULL;

  if (MainEditor.MouseSupported) {

    if (FileBufferMouseNeedRefresh) {

      FileBufferMouseNeedRefresh = FALSE;

      //
      // if mouse position not moved and only mouse action
      // so do not need to refresh mouse position
      //
      if ((FileBuffer.MousePosition.Row == FileBufferBackupVar.MousePosition.Row &&
          FileBuffer.MousePosition.Column == FileBufferBackupVar.MousePosition.Column)
          && EditorMouseAction) {
        return EFI_SUCCESS;
      }
      //
      // backup the old screen attributes
      //
      Orig                  = MainEditor.ColorAttributes;
      New.Colors.Foreground = Orig.Colors.Background;
      New.Colors.Background = Orig.Colors.Foreground;

      //
      // clear the old mouse position
      //
      FRow          = FileBuffer.LowVisibleRange.Row + FileBufferBackupVar.MousePosition.Row - TEXT_START_ROW;

      FColumn       = FileBuffer.LowVisibleRange.Column + FileBufferBackupVar.MousePosition.Column - TEXT_START_COLUMN;

      HasCharacter  = TRUE;
      if (FRow > FileBuffer.NumLines) {
        HasCharacter = FALSE;
      } else {
        CurrentLine = FileBuffer.CurrentLine;
        Line        = MoveLine (FRow - FileBuffer.FilePosition.Row);

        if (FColumn > Line->Size) {
          HasCharacter = FALSE;
        }

        FileBuffer.CurrentLine = CurrentLine;
      }

      PrintAt (
        FileBufferBackupVar.MousePosition.Column - 1,
        FileBufferBackupVar.MousePosition.Row - 1,
        L" "
        );

      if (HasCharacter) {
        Value = (Line->Buffer[FColumn - 1]);
        PrintAt (
          FileBufferBackupVar.MousePosition.Column - 1,
          FileBufferBackupVar.MousePosition.Row - 1,
          L"%c",
          Value
          );
      }
      //
      // set the new mouse position
      //
      Out->SetAttribute (Out, New.Data);

      //
      // clear the old mouse position
      //
      FRow          = FileBuffer.LowVisibleRange.Row + FileBuffer.MousePosition.Row - TEXT_START_ROW;
      FColumn       = FileBuffer.LowVisibleRange.Column + FileBuffer.MousePosition.Column - TEXT_START_COLUMN;

      HasCharacter  = TRUE;
      if (FRow > FileBuffer.NumLines) {
        HasCharacter = FALSE;
      } else {
        CurrentLine = FileBuffer.CurrentLine;
        Line        = MoveLine (FRow - FileBuffer.FilePosition.Row);

        if (FColumn > Line->Size) {
          HasCharacter = FALSE;
        }

        FileBuffer.CurrentLine = CurrentLine;
      }

      PrintAt (
        FileBuffer.MousePosition.Column - 1,
        FileBuffer.MousePosition.Row - 1,
        L" "
        );

      if (HasCharacter) {
        Value = Line->Buffer[FColumn - 1];
        PrintAt (
          FileBuffer.MousePosition.Column - 1,
          FileBuffer.MousePosition.Row - 1,
          L"%c",
          Value
          );
      }
      //
      // end of HasCharacter
      //
      Out->SetAttribute (Out, Orig.Data);
    }
    //
    // end of MouseNeedRefresh
    //
  }
  //
  // end of MouseSupported
  //
  return EFI_SUCCESS;
}
//
// Name:
//   FileBufferFreeLines -- Free all the lines in FileBuffer
//   Fields affected:
//     Lines
//     CurrentLine
//     NumLines
//     ListHead
// In:
//     VOID
// Out:
//     EFI_SUCCESS
//
EFI_STATUS
FileBufferFreeLines (
  VOID
  )
{
  EFI_LIST_ENTRY  *Link;
  EFI_EDITOR_LINE *Line;

  //
  // free all the lines
  //
  if (FileBuffer.Lines != NULL) {

    Line  = FileBuffer.Lines;
    Link  = &(Line->Link);
    do {
      Line  = CR (Link, EFI_EDITOR_LINE, Link, EFI_EDITOR_LINE_LIST);
      Link  = Link->Flink;

      //
      // free line's buffer and line itself
      //
      LineFree (Line);
    } while (Link != FileBuffer.ListHead);
  }
  //
  // clean the line list related structure
  //
  FileBuffer.Lines            = NULL;
  FileBuffer.CurrentLine      = NULL;
  FileBuffer.NumLines         = 0;

  FileBuffer.ListHead->Flink  = FileBuffer.ListHead;
  FileBuffer.ListHead->Blink  = FileBuffer.ListHead;

  return EFI_SUCCESS;
}
//
// Name:
//   FileBufferCleanup -- Cleanup function for FileBuffer
// In:
//   VOID
// Out:
//   EFI_SUCCESS
//
EFI_STATUS
FileBufferCleanup (
  VOID
  )
{
  EFI_STATUS  Status;

  EditorFreePool (FileBuffer.FileName);

  //
  // free all the lines
  //
  Status = FileBufferFreeLines ();

  EditorFreePool (FileBuffer.ListHead);
  FileBuffer.ListHead = NULL;

  EditorFreePool (FileBufferBackupVar.FileName);
  return Status;

}
//
// Name:
//   FileBufferPrintLine -- Print Line on Row
// In:
//   Line -- Line to print
//   Row -- Row on screen ( begin from 1 )
// Out:
//   EFI_SUCCESS
//
EFI_STATUS
FileBufferPrintLine (
  IN EFI_EDITOR_LINE  *Line,
  IN UINTN            Row
  )
{

  CHAR16  *Buffer;
  UINTN   Limit;
  CHAR16  PrintLine[200];

  //
  // print start from correct character
  //
  Buffer  = Line->Buffer + FileBuffer.LowVisibleRange.Column - 1;

  Limit   = Line->Size - FileBuffer.LowVisibleRange.Column + 1;
  if (Limit > Line->Size) {
    Limit = 0;
  }

  StrnCpy (PrintLine, Buffer, Limit > NUM_TEXT_COLUMNS ? NUM_TEXT_COLUMNS : Limit);
  for (; Limit < NUM_TEXT_COLUMNS; Limit++) {
    PrintLine[Limit] = L' ';
  }

  PrintLine[NUM_TEXT_COLUMNS] = '\0';

  PrintAt (
    TEXT_START_COLUMN - 1,
    Row - 1,
    L"%s",
    PrintLine
    );

  return EFI_SUCCESS;
}
//
// Name:
//   FileBufferRestorePosition --
//       Set cursor position according to FileBuffer.DisplayPosition.
// In:
//   VOID
// Out:
//   EFI_SUCCESS
//
EFI_STATUS
FileBufferRestorePosition (
  VOID
  )
{
  //
  // set cursor position
  //
  Out->SetCursorPosition (
        Out,
        FileBuffer.DisplayPosition.Column - 1,
        FileBuffer.DisplayPosition.Row - 1
        );

  return EFI_SUCCESS;
}
//
// Name:
//   FileBufferRefresh -- Refresh function for FileBuffer
// In:
//   VOID
// Out:
//   EFI_SUCCESS
//   EFI_LOAD_ERROR
//
EFI_STATUS
FileBufferRefresh (
  VOID
  )
{
  EFI_LIST_ENTRY  *Link;
  EFI_EDITOR_LINE *Line;
  UINTN           Row;

  //
  // if it's the first time after editor launch, so should refresh
  //
  if (EditorFirst == FALSE) {
    //
    // no definite required refresh
    // and file position displayed on screen has not been changed
    //
    if (FileBufferNeedRefresh == FALSE &&
        FileBufferOnlyLineNeedRefresh == FALSE &&
        FileBufferBackupVar.LowVisibleRange.Row == FileBuffer.LowVisibleRange.Row &&
        FileBufferBackupVar.LowVisibleRange.Column == FileBuffer.LowVisibleRange.Column
        ) {

      FileBufferRestoreMousePosition ();
      FileBufferRestorePosition ();

      return EFI_SUCCESS;
    }
  }

  Out->EnableCursor (Out, FALSE);

  //
  // only need to refresh current line
  //
  if (FileBufferOnlyLineNeedRefresh == TRUE &&
      FileBufferBackupVar.LowVisibleRange.Row == FileBuffer.LowVisibleRange.Row &&
      FileBufferBackupVar.LowVisibleRange.Column == FileBuffer.LowVisibleRange.Column
      ) {

    EditorClearLine (FileBuffer.DisplayPosition.Row);
    FileBufferPrintLine (
      FileBuffer.CurrentLine,
      FileBuffer.DisplayPosition.Row
      );
  } else {
    //
    // the whole edit area need refresh
    //
    //
    // no line
    //
    if (FileBuffer.Lines == NULL) {
      FileBufferRestoreMousePosition ();
      FileBufferRestorePosition ();
      Out->EnableCursor (Out, TRUE);

      return EFI_SUCCESS;
    }
    //
    // get the first line that will be displayed
    //
    Line = MoveLine (FileBuffer.LowVisibleRange.Row - FileBuffer.FilePosition.Row);
    if (Line == NULL) {
      Out->EnableCursor (Out, TRUE);

      return EFI_LOAD_ERROR;
    }

    Link  = &(Line->Link);
    Row   = TEXT_START_ROW;
    do {
      Line = CR (Link, EFI_EDITOR_LINE, Link, EFI_EDITOR_LINE_LIST);

      //
      // print line at row
      //
      FileBufferPrintLine (Line, Row);

      Link = Link->Flink;
      Row++;
    } while (Link != FileBuffer.ListHead && Row <= TEXT_END_ROW);
    //
    // while not file end and not screen full
    //
    while (Row <= TEXT_END_ROW) {
      EditorClearLine (Row);
      Row++;
    }
  }

  FileBufferRestoreMousePosition ();
  FileBufferRestorePosition ();

  FileBufferNeedRefresh         = FALSE;
  FileBufferOnlyLineNeedRefresh = FALSE;

  Out->EnableCursor (Out, TRUE);
  return EFI_SUCCESS;
}
//
// Name:
//     FileBufferCreateLine --
//         Create a new line and append it to the line list
//     Fields affected:
//     NumLines
//     Lines
// In:
//     VOID
// Out:
//     NULL -- create line failed
//     Not NULL -- the line created
//
EFI_EDITOR_LINE *
FileBufferCreateLine (
  VOID
  )
{
  EFI_EDITOR_LINE *Line;

  //
  // allocate a line structure
  //
  Line = AllocatePool (sizeof (EFI_EDITOR_LINE));
  if (Line == NULL) {
    return NULL;
  }
  //
  // initialize the structure
  //
  Line->Signature = EFI_EDITOR_LINE_LIST;
  Line->Size      = 0;
  Line->TotalSize = 0;
  Line->Type      = DEFAULT_TYPE;

  //
  // initial buffer of the line is "\0"
  //
  Line->Buffer = PoolPrint (L"\0");
  if (Line->Buffer == NULL) {
    return NULL;
  }

  FileBuffer.NumLines++;

  //
  // insert the line into line list
  //
  InsertTailList (FileBuffer.ListHead, &Line->Link);

  if (FileBuffer.Lines == NULL) {
    FileBuffer.Lines = CR (FileBuffer.ListHead->Flink, EFI_EDITOR_LINE, Link, EFI_EDITOR_LINE_LIST);
  }

  return Line;
}
//
// Name:
//   FileBufferSetFileName -- Set FileName field in FileBuffer
// In:
//   Str -- File name to set
// Out:
//   EFI_SUCCESS
//
EFI_STATUS
FileBufferSetFileName (
  IN CHAR16 *Str
  )
{
  UINTN Size;
  UINTN Index;

  //
  // free the old file name
  //
  EditorFreePool (FileBuffer.FileName);

  Size                = StrLen (Str);

  FileBuffer.FileName = AllocatePool (2 * (Size + 1));
  if (FileBuffer.FileName == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  for (Index = 0; Index < Size; Index++) {
    FileBuffer.FileName[Index] = Str[Index];
  }

  FileBuffer.FileName[Size] = L'\0';

  return EFI_SUCCESS;
}
//
// Name:
//   FileBufferFree --
//       Function called when load a new file in.
//       It will free all the old lines
//    and set FileModified field to FALSE
// In:
//   VOID
// Out:
//   EFI_SUCCESS
//
EFI_STATUS
FileBufferFree (
  VOID
  )
{
  //
  // free all the lines
  //
  FileBufferFreeLines ();
  FileBuffer.FileModified = FALSE;

  return EFI_SUCCESS;
}
//
// Name:
//   FileBufferGetFileInfo -- Get this file's information
// In:
//   Handle -- in NT32 mode Directory handle, in other mode File Handle
//   EFI_FILE_INFO ** -- parameter to pass file information out
// Out:
//   EFI_SUCCESS
//   EFI_OUT_OF_RESOURCES
//   EFI_LOAD_ERROR
//
EFI_STATUS
FileBufferGetFileInfo (
  IN  EFI_FILE_HANDLE Handle,
  IN  CHAR16          *FileName,
  OUT EFI_FILE_INFO   **InfoOut
  )
{

  VOID        *Info;
  UINTN       Size;
  EFI_STATUS  Status;

  Size  = SIZE_OF_EFI_FILE_INFO + 1024;
  Info  = AllocatePool (Size);
  if (!Info) {
    return EFI_OUT_OF_RESOURCES;
  }
  //
  // get file information
  //
  Status = Handle->GetInfo (Handle, &gEfiFileInfoGuid, &Size, Info);
  if (EFI_ERROR (Status)) {
    return EFI_LOAD_ERROR;
  }

  *InfoOut = (EFI_FILE_INFO *) Info;

  return EFI_SUCCESS;

}

EFI_STATUS
FileBufferRead (
  IN CHAR16  *FileName,
  IN BOOLEAN Recover
  )
/*++
Routine Description: 

  Read a file from disk into FileBuffer
  
Arguments: 

  FileName -- filename to read
  Recover -- if is for recover, no information print
  
Returns:  

  EFI_SUCCESS
  EFI_LOAD_ERROR
  EFI_OUT_OF_RESOURCES
  
--*/
{
  EFI_EDITOR_LINE                 *Line;
  UINTN                           i;
  UINTN                           j;
  UINTN                           LineSize;
  VOID                            *Buffer;
  CHAR16                          *UnicodeBuffer;
  CHAR8                           *AsciiBuffer;
  UINTN                           FileSize;
  EFI_FILE_HANDLE                 Handle;
  BOOLEAN                         CreateFile;
  EFI_STATUS                      Status;
  UINTN                           LineSizeBackup;
  EE_NEWLINE_TYPE                 Type;

  EFI_LIST_ENTRY                  DirList;
  SHELL_FILE_ARG                  *Arg;

  EFI_FILE_INFO                   *Info;
  EFI_FILE_INFO                   *Info2;
  BOOLEAN                         Found;
  CHAR16                          *FileNameTmp;

  EFI_DEVICE_PATH_PROTOCOL        *DevicePath;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *Vol;
  EFI_FILE_HANDLE                 RootFs;
  EFI_FILE_SYSTEM_INFO            *VolumeInfo;
  UINTN                           Size;
  CHAR16                          *CurDir;

  CHAR16                          *FSMappingPtr;
  CHAR16                          FSMapping[10];

  Line          = NULL;
  i             = 0;
  CreateFile    = FALSE;

  Handle        = NULL;
  FileSize      = 0;
  UnicodeBuffer = NULL;
  Type          = 0;

  //
  // in this function, when you return error ( except EFI_OUT_OF_RESOURCES )
  // you should set status string
  // since this function maybe called before the editorhandleinput loop
  // so any error will cause editor return
  // so if you want to print the error status
  // you should set the status string
  //
  FileNameTmp = PoolPrint (L"%s", FileName);
  if (FileNameTmp == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  //
  // GET CURRENT DIR HANDLE
  //
  InitializeListHead (&DirList);

  //
  // after that filename changed to path
  //
  Status = ShellFileMetaArgNoWildCard (FileNameTmp, &DirList);
  FreePool (FileNameTmp);

  if (EFI_ERROR (Status)) {
    MainStatusBarSetStatusString (L"Disk Error");
    return EFI_LOAD_ERROR;
  }

  if (DirList.Flink == &DirList) {
    MainStatusBarSetStatusString (L"Disk Error");
    return EFI_LOAD_ERROR;
  }

  Arg = CR (DirList.Flink, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);

  //
  // since when Status is returned correct, not everything is correct,
  // so  I have to add these check (-:(((((
  //
  if (Arg == NULL) {
    MainStatusBarSetStatusString (L"Disk Error");
    ShellFreeFileList (&DirList);
    return EFI_LOAD_ERROR;
  }

  if (Arg->Parent == NULL) {
    MainStatusBarSetStatusString (L"Disk Error");
    ShellFreeFileList (&DirList);
    return EFI_LOAD_ERROR;
  }

  Info = Arg->Info;

  //
  // check whether this file exists
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

    Status  = FileBufferGetFileInfo (Handle, Arg->FileName, &Info);
    if (EFI_ERROR (Status)) {
      MainStatusBarSetStatusString (L"Disk Error");
      ShellFreeFileList (&DirList);
      return EFI_LOAD_ERROR;
    }

    if (Info->Attribute & EFI_FILE_DIRECTORY) {
      MainStatusBarSetStatusString (L"Directory Can Not Be Edited");
      ShellFreeFileList (&DirList);
      FreePool (Info);
      return EFI_LOAD_ERROR;
    }
    //
    // check if read only
    //
    if (Info->Attribute & EFI_FILE_READ_ONLY) {
      FileBuffer.ReadOnly = TRUE;
    } else {
      FileBuffer.ReadOnly = FALSE;
    }
    //
    // get file size
    //
    FileSize = (UINTN) Info->FileSize;

    FreePool (Info);

  } else {
    //
    // file not exists, check whether this file can be created to aVOID illegal file name error
    //
    Status = Arg->Parent->Open (
                            Arg->Parent,
                            &Arg->Handle,
                            Arg->FileName,
                            FILE_CREATE,
                            0
                            );
    if (EFI_ERROR (Status)) {
      ShellFreeFileList (&DirList);
      if (Status == EFI_WRITE_PROTECTED ||
          Status == EFI_ACCESS_DENIED ||
          Status == EFI_NO_MEDIA ||
          Status == EFI_MEDIA_CHANGED
          ) {
        MainStatusBarSetStatusString (L"Access Denied");
      } else if (Status == EFI_DEVICE_ERROR || Status == EFI_VOLUME_CORRUPTED || Status == EFI_VOLUME_FULL) {
        MainStatusBarSetStatusString (L"Disk Error");
      } else {
        MainStatusBarSetStatusString (L"Invalid File Name");
      }

      return Status;
    } else {
      Status = Arg->Handle->Delete (Arg->Handle);
      if (Status == EFI_WARN_DELETE_FAILURE) {
        Status = EFI_ACCESS_DENIED;
      }

      Arg->Handle = NULL;
      if (EFI_ERROR (Status)) {
        ShellFreeFileList (&DirList);
        MainStatusBarSetStatusString (L"Access Denied");
        return Status;
      }
    }
    //
    // file not exists, so set CreateFile to TRUE
    //
    CreateFile          = TRUE;
    FileBuffer.ReadOnly = FALSE;

    //
    // all the check ends
    // so now begin to set file name, free lines
    //
    if (StrCmp (FileName, FileBuffer.FileName) != 0) {
      FileBufferSetFileName (FileName);
    }
    //
    // free the old lines
    //
    FileBufferFree ();

  }
  //
  // file exists
  //
  if (CreateFile == FALSE) {
    //
    // read parent to get the true name of the file
    //
    Size  = SIZE_OF_EFI_FILE_INFO + 1024;
    Info2 = AllocatePool (Size);
    if (Info2 == NULL) {
      ShellFreeFileList (&DirList);
      MainStatusBarSetStatusString (L"Read File Failed");
      return EFI_OUT_OF_RESOURCES;
    }

    Status = Arg->Parent->SetPosition (Arg->Parent, 0);
    if (EFI_ERROR (Status)) {
      ShellFreeFileList (&DirList);
      MainStatusBarSetStatusString (L"Read File Failed");
      FreePool (Info2);
      return EFI_LOAD_ERROR;
    }

    Found = FALSE;
    while (1) {
      Size    = SIZE_OF_EFI_FILE_INFO + 1024;
      Status  = Arg->Parent->Read (Arg->Parent, &Size, Info2);
      if (EFI_ERROR (Status)) {
        ShellFreeFileList (&DirList);
        MainStatusBarSetStatusString (L"Read File Failed");
        FreePool (Info2);
        return EFI_LOAD_ERROR;
      }

      if (Size == 0) {
        break;
      }

      if (StriCmp (Info2->FileName, Arg->Info->FileName) == 0) {
        Found       = TRUE;

        FileNameTmp = AllocatePool ((StrLen (Info2->FileName) + StrLen (Arg->ParentName) + 2) * sizeof (CHAR16));

        if (FileNameTmp == NULL) {
          FreePool (Info2);
          ShellFreeFileList (&DirList);
          MainStatusBarSetStatusString (L"Read File Failed");
          return EFI_OUT_OF_RESOURCES;
        }

        if (StrLen (Info2->FileName) == StrLen (FileName)) {
          StrCpy (FileNameTmp, Info2->FileName);
        } else {
          StrCpy (FileNameTmp, Arg->ParentName);
          if (FileNameTmp[StrLen (FileNameTmp) - 1] != L'\\') {
            StrCat (FileNameTmp, L"\\");
          }

          StrCat (FileNameTmp, Info2->FileName);
        }

        break;

      }
    }

    FreePool (Info2);

    if (!Found) {
      ShellFreeFileList (&DirList);
      MainStatusBarSetStatusString (L"Read File Failed");
      return EFI_LOAD_ERROR;
    }
    //
    // Now the file name is in FileNameTmp
    //
    //
    // allocate buffer to read file
    //
    Buffer = AllocatePool (FileSize);
    if (Buffer == NULL) {
      ShellFreeFileList (&DirList);
      FreePool (FileNameTmp);
      return EFI_OUT_OF_RESOURCES;
    }
    //
    // read file into Buffer
    //
    Status = Handle->Read (Handle, &FileSize, Buffer);
    if (EFI_ERROR (Status)) {
      MainStatusBarSetStatusString (L"Read File Failed");
      EditorFreePool (Buffer);
      ShellFreeFileList (&DirList);
      FreePool (FileNameTmp);
      return EFI_LOAD_ERROR;
    }
    //
    // nothing in this file
    //
    if (FileSize == 0) {
      EditorFreePool (Buffer);
      FileBufferSetFileName (FileNameTmp);
      FreePool (FileNameTmp);
      //
      // since has no head, so only can be an ASCII file
      //
      FileBuffer.FileType = ASCII_FILE;

      goto Done;
    }

    AsciiBuffer = Buffer;

    if (FileSize < 2) {
      //
      // size < Unicode file header, so only can be ASCII file
      //
      FileBuffer.FileType = ASCII_FILE;
    } else {
      //
      // Unicode file
      //
      if (AsciiBuffer[0] == 0xff && AsciiBuffer[1] == 0xfe) {
        //
        // Unicode file's size should be even
        //
        if ((FileSize % 2) != 0) {
          MainStatusBarSetStatusString (L"File Format Wrong");
          EditorFreePool (Buffer);
          ShellFreeFileList (&DirList);
          FreePool (FileNameTmp);
          return EFI_LOAD_ERROR;
        }

        FileSize /= 2;

        FileBuffer.FileType = UNICODE_FILE;
        UnicodeBuffer       = Buffer;

        //
        // pass this 0xff and 0xfe
        //
        UnicodeBuffer++;
        FileSize--;
      } else {
        FileBuffer.FileType = ASCII_FILE;
      }
      //
      // end of AsciiBuffer ==
      //
    }
    //
    // end of FileSize < 2
    // all the check ends
    // so now begin to set file name, free lines
    //
    if (StrCmp (FileNameTmp, FileBuffer.FileName) != 0) {
      FileBufferSetFileName (FileNameTmp);
    }

    FreePool (FileNameTmp);
    //
    // free the old lines
    //
    FileBufferFree ();

    //
    // parse file content line by line
    //
    for (i = 0; i < FileSize; i++) {
      Type = UNKNOWN;

      for (LineSize = i; LineSize < FileSize; LineSize++) {
        if (FileBuffer.FileType == ASCII_FILE) {
          if (AsciiBuffer[LineSize] == CHAR_CR) {
            Type = USE_CR;

            //
            // has LF following
            //
            if (LineSize < FileSize - 1) {
              if (AsciiBuffer[LineSize + 1] == CHAR_LF) {
                Type = USE_CRLF;
              }
            }

            break;
          } else if (AsciiBuffer[LineSize] == CHAR_LF) {
            Type = USE_LF;

            //
            // has CR following
            //
            if (LineSize < FileSize - 1) {
              if (AsciiBuffer[LineSize + 1] == CHAR_CR) {
                Type = USE_LFCR;
              }
            }

            break;
          }
        } else {
          if (UnicodeBuffer[LineSize] == CHAR_CR) {
            Type = USE_CR;

            //
            // has LF following
            //
            if (LineSize < FileSize - 1) {
              if (UnicodeBuffer[LineSize + 1] == CHAR_LF) {
                Type = USE_CRLF;
              }
            }

            break;
          } else if (UnicodeBuffer[LineSize] == CHAR_LF) {
            Type = USE_LF;

            //
            // has CR following
            //
            if (LineSize < FileSize - 1) {
              if (UnicodeBuffer[LineSize + 1] == CHAR_CR) {
                Type = USE_LFCR;
              }
            }

            break;
          }
        }
        //
        // endif == ASCII
        //
      }
      //
      // end of for LineSize
      //
      // if the type is wrong, then exit
      //
      if (Type == UNKNOWN) {
        //
        // Now if Type is UNKNOWN, it should be file end
        //
        Type = DEFAULT_TYPE;
      }

      LineSizeBackup = LineSize;

      //
      // create a new line
      //
      Line = FileBufferCreateLine ();
      if (Line == NULL) {
        EditorFreePool (Buffer);
        return EFI_OUT_OF_RESOURCES;
      }
      //
      // calculate file length
      //
      LineSize -= i;

      //
      // Unicode and one '\0'
      //
      EditorFreePool (Line->Buffer);
      Line->Buffer = AllocateZeroPool (LineSize * 2 + 2);

      if (Line->Buffer == NULL) {
        RemoveEntryList (&Line->Link);
        return EFI_OUT_OF_RESOURCES;
      }
      //
      // copy this line to Line->Buffer
      //
      for (j = 0; j < LineSize; j++) {
        if (FileBuffer.FileType == ASCII_FILE) {
          Line->Buffer[j] = (CHAR16) AsciiBuffer[i];
        } else {
          Line->Buffer[j] = UnicodeBuffer[i];
        }

        i++;
      }
      //
      // i now points to where CHAR_CR or CHAR_LF;
      //
      Line->Buffer[LineSize]  = 0;

      Line->Size              = LineSize;
      Line->TotalSize         = LineSize;
      Line->Type              = Type;

      if (Type == USE_CRLF || Type == USE_LFCR) {
        i++;
      }

      //
      // last character is a return, SO create a new line
      //
      if (((Type == USE_CRLF || Type == USE_LFCR) && LineSizeBackup == FileSize - 2) ||
          ((Type == USE_LF || Type == USE_CR) && LineSizeBackup == FileSize - 1)
          ) {
        Line = FileBufferCreateLine ();
        if (Line == NULL) {
          EditorFreePool (Buffer);
          return EFI_OUT_OF_RESOURCES;
        }
      }
      //
      // end of if
      //
    }
    //
    // end of i
    //
    EditorFreePool (Buffer);

  }
  //
  // end of if CreateFile
  //
Done:

  FileBuffer.DisplayPosition.Row    = TEXT_START_ROW;
  FileBuffer.DisplayPosition.Column = TEXT_START_COLUMN;
  FileBuffer.LowVisibleRange.Row    = 1;
  FileBuffer.LowVisibleRange.Column = 1;
  FileBuffer.FilePosition.Row       = 1;
  FileBuffer.FilePosition.Column    = 1;
  FileBuffer.MousePosition.Row      = TEXT_START_ROW;
  FileBuffer.MousePosition.Column   = TEXT_START_COLUMN;

  if (!Recover) {
    UnicodeBuffer = PoolPrint (L"%d Lines Read", FileBuffer.NumLines);
    if (UnicodeBuffer == NULL) {
      ShellFreeFileList (&DirList);
      return EFI_OUT_OF_RESOURCES;
    }

    MainStatusBarSetStatusString (UnicodeBuffer);
    EditorFreePool (UnicodeBuffer);

    //
    // check whether we have fs?: in filename
    //
    i             = 0;
    FSMappingPtr  = NULL;
    while (FileName[i] != 0) {
      if (FileName[i] == L':') {
        FSMappingPtr = &FileName[i];
        break;
      }

      i++;
    }

    if (FSMappingPtr == NULL) {
      CurDir = ShellCurDir (NULL);
    } else {
      i = 0;
      j = 0;
      while (FileName[i] != 0) {
        if (FileName[i] == L':') {
          break;
        }

        FSMapping[j++] = FileName[i];

        i++;
      }

      FSMapping[j]  = 0;
      CurDir        = ShellCurDir (FSMapping);
    }

    if (CurDir) {
      for (i = 0; i < StrLen (CurDir) && CurDir[i] != ':'; i++)
      { ;
      }

      CurDir[i]   = 0;
      DevicePath  = (EFI_DEVICE_PATH_PROTOCOL *) ShellGetMap (CurDir);
      FreePool (CurDir);
    } else {
      ShellFreeFileList (&DirList);
      return EFI_LOAD_ERROR;
    }

    Status = LibDevicePathToInterface (
              &gEfiSimpleFileSystemProtocolGuid,
              DevicePath,
              (VOID **) &Vol
              );
    if (EFI_ERROR (Status)) {
      ShellFreeFileList (&DirList);
      return EFI_LOAD_ERROR;
    }

    Status = Vol->OpenVolume (Vol, &RootFs);
    if (EFI_ERROR (Status)) {
      ShellFreeFileList (&DirList);
      return EFI_LOAD_ERROR;
    }
    //
    // Get volume information of file system
    //
    Size        = SIZE_OF_EFI_FILE_SYSTEM_INFO + 100;
    VolumeInfo  = (EFI_FILE_SYSTEM_INFO *) AllocatePool (Size);
    Status      = RootFs->GetInfo (RootFs, &gEfiFileSystemInfoGuid, &Size, VolumeInfo);
    if (EFI_ERROR (Status)) {
      RootFs->Close (RootFs);
      ShellFreeFileList (&DirList);
      return EFI_LOAD_ERROR;
    }

    if (VolumeInfo->ReadOnly) {
      MainStatusBarSetStatusString (L"WARNING: Volume Read Only");
    }

    FreePool (VolumeInfo);
    RootFs->Close (RootFs);
  }

  //
  // has line
  //
  if (FileBuffer.Lines != 0) {
    FileBuffer.CurrentLine = CR (FileBuffer.ListHead->Flink, EFI_EDITOR_LINE, Link, EFI_EDITOR_LINE_LIST);
  } else {
    //
    // create a dummy line
    //
    Line = FileBufferCreateLine ();
    if (Line == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }

    FileBuffer.CurrentLine = Line;
  }

  FileBuffer.FileModified       = FALSE;
  FileBufferNeedRefresh         = TRUE;
  FileBufferOnlyLineNeedRefresh = FALSE;
  FileBufferMouseNeedRefresh    = TRUE;

  ShellFreeFileList (&DirList);

  return EFI_SUCCESS;
}
//
// Name:
//   GetNewLine --
//       According to FileBuffer.NewLineType & FileBuffer.FileType,
//       get the return buffer and size
// In:
//   Buffer -- pass the return buffer out
//   Size -- pass the return buffer size out
// Out:
//   VOID
//
VOID
GetNewLine (
  IN  EE_NEWLINE_TYPE Type,
  OUT CHAR8           *Buffer,
  OUT UINT8           *Size
  )
{
  UINT8 NewLineSize;

  //
  // give new line buffer,
  // and will judge unicode or ascii
  //
  NewLineSize = 0;

  //
  // not legal new line type
  //
  if (Type != USE_LF && Type != USE_CR && Type != USE_CRLF && Type != USE_LFCR) {
    *Size = 0;
    return ;
  }
  //
  // use_cr: give 0x0d
  //
  if (Type == USE_CR) {
    if (MainEditor.FileBuffer->FileType == UNICODE_FILE) {
      Buffer[0]   = 0x0d;
      Buffer[1]   = 0;
      NewLineSize = 2;
    } else {
      Buffer[0]   = 0x0d;
      NewLineSize = 1;
    }

    *Size = NewLineSize;
    return ;
  }
  //
  // use_lf: give 0x0a
  //
  if (Type == USE_LF) {
    if (MainEditor.FileBuffer->FileType == UNICODE_FILE) {
      Buffer[0]   = 0x0a;
      Buffer[1]   = 0;
      NewLineSize = 2;
    } else {
      Buffer[0]   = 0x0a;
      NewLineSize = 1;
    }

    *Size = NewLineSize;
    return ;
  }
  //
  // use_crlf: give 0x0d 0x0a
  //
  if (Type == USE_CRLF) {
    if (MainEditor.FileBuffer->FileType == UNICODE_FILE) {
      Buffer[0]   = 0x0d;
      Buffer[1]   = 0;
      Buffer[2]   = 0x0a;
      Buffer[3]   = 0;

      NewLineSize = 4;
    } else {
      Buffer[0]   = 0x0d;
      Buffer[1]   = 0x0a;
      NewLineSize = 2;
    }

    *Size = NewLineSize;
    return ;
  }
  //
  // use_lfcr: give 0x0a 0x0d
  //
  if (Type == USE_LFCR) {
    if (MainEditor.FileBuffer->FileType == UNICODE_FILE) {
      Buffer[0]   = 0x0a;
      Buffer[1]   = 0;
      Buffer[2]   = 0x0d;
      Buffer[3]   = 0;

      NewLineSize = 4;
    } else {
      Buffer[0]   = 0x0a;
      Buffer[1]   = 0x0d;
      NewLineSize = 2;
    }

    *Size = NewLineSize;
    return ;
  }

}
//
// Name:
//   FileBufferSave -- Save lines in FileBuffer to disk
// In:
//   VOID
// Out:
//   EFI_SUCCESS
//   EFI_LOAD_ERROR
//   EFI_OUT_OF_RESOURCES
//
EFI_STATUS
FileBufferSave (
  IN CHAR16 *FileName
  )
{

  EFI_LIST_ENTRY  *Link;
  EFI_EDITOR_LINE *Line;
  CHAR16          *Str;

  EFI_STATUS      Status;
  UINTN           Length;
  UINTN           NumLines;
  CHAR8           NewLineBuffer[4];
  UINT8           NewLineSize;

  EFI_FILE_HANDLE FileHandle;

  EFI_LIST_ENTRY  DirList;
  SHELL_FILE_ARG  *Arg;
  EFI_FILE_INFO   *Info;
  CHAR16          *FileNameTmp;

  UINT64          Attribute;

  EE_NEWLINE_TYPE Type;

  UINTN           TotalSize;
  //
  // 2M
  //
  CHAR8           *Cache;
  UINTN           LeftSize;
  UINTN           Size;
  CHAR8           *Ptr;

  UINT8           Marker1;
  UINT8           Marker2;

  Length    = 0;
  TotalSize = 0x200000;
  //
  // 2M
  //
  // Unicode Symbol
  //
  Marker1     = 0xff;
  Marker2     = 0xfe;

  FileNameTmp = PoolPrint (L"%s", FileName);
  if (FileNameTmp == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // get file handle
  //
  // GET CURRENT DIR HANDLE
  //
  InitializeListHead (&DirList);
  Status = ShellFileMetaArgNoWildCard (FileNameTmp, &DirList);
  FreePool (FileNameTmp);

  if (EFI_ERROR (Status)) {
    return EFI_LOAD_ERROR;
  }

  if (DirList.Flink == &DirList) {
    return EFI_LOAD_ERROR;
  }

  Arg = CR (DirList.Flink, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);

  //
  // since when Status is returned correct, not everything is correct,
  // so  I have to add these check (-:(((((
  //
  if (Arg == NULL) {
    ShellFreeFileList (&DirList);
    return EFI_LOAD_ERROR;
  }

  if (Arg->Parent == NULL) {
    ShellFreeFileList (&DirList);
    return EFI_LOAD_ERROR;
  }

  //
  // if is the old file
  //
  if (StrCmp (FileName, FileBuffer.FileName) == 0) {
    //
    // check whether file exists on disk
    //
    if (!Arg->Status) {
      //
      // current file exists on disk
      // so if not modified, then not save
      //
      // file has not been modified
      //
      if (FileBuffer.FileModified == FALSE) {
        ShellFreeFileList (&DirList);
        return EFI_SUCCESS;
      }
      //
      // if file is read-only, set error
      //
      if (FileBuffer.ReadOnly == TRUE) {
        MainStatusBarSetStatusString (L"Read Only File Can Not Be Saved");
        ShellFreeFileList (&DirList);
        return EFI_SUCCESS;
      }
    }
  }
  //
  // now it's cases that you should write disk
  //
  Info      = Arg->Info;

  Attribute = 0;

  //
  // open current file
  //
  if (!Arg->Status) {
    if (!Arg->Handle) {
      Status = Arg->Parent->Open (
                              Arg->Parent,
                              &Arg->Handle,
                              Arg->FileName,
                              EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE,
                              0
                              );
    }

    FileHandle  = Arg->Handle;

    Status      = FileBufferGetFileInfo (FileHandle, Arg->FileName, &Info);
    if (EFI_ERROR (Status)) {
      ShellFreeFileList (&DirList);
      return EFI_LOAD_ERROR;
    }

    if (Info->Attribute & EFI_FILE_DIRECTORY) {
      MainStatusBarSetStatusString (L"Directory Can Not Be Saved");
      ShellFreeFileList (&DirList);
      return EFI_LOAD_ERROR;
    }

    if (Info->Attribute & EFI_FILE_READ_ONLY) {
      Attribute = Info->Attribute - EFI_FILE_READ_ONLY;
    } else {
      Attribute = Info->Attribute;
    }

    //
    // if file exits, so delete it
    //
    Status = FileHandle->Delete (FileHandle);
    if (Status == EFI_WARN_DELETE_FAILURE) {
      Status = EFI_ACCESS_DENIED;
    }

    Arg->Handle = NULL;
    if (EFI_ERROR (Status)) {
      ShellFreeFileList (&DirList);

      //
      // because in nt32 mode, when according file systems,
      // it will generate exception
      // so comment it
      //
      MainStatusBarSetStatusString (L"Write File Failed");
      return EFI_LOAD_ERROR;
    }
  }

  //
  // create this file
  //
  Status = Arg->Parent->Open (
                          Arg->Parent,
                          &Arg->Handle,
                          Arg->FileName,
                          FILE_CREATE,
                          Attribute
                          );
  if (EFI_ERROR (Status)) {
    ShellFreeFileList (&DirList);

    MainStatusBarSetStatusString (L"Create File Failed");

    return EFI_LOAD_ERROR;
  }

  FileHandle = Arg->Handle;

  //
  // if file is Unicode file, write Unicode header to it.
  //
  if (FileBuffer.FileType == UNICODE_FILE) {
    Length  = 1;
    Status  = FileHandle->Write (FileHandle, &Length, &Marker1);
    if (EFI_ERROR (Status)) {
      FileHandle->Delete (FileHandle);
      Arg->Handle = NULL;
      ShellFreeFileList (&DirList);

      return EFI_LOAD_ERROR;
    }

    Length  = 1;
    Status  = FileHandle->Write (FileHandle, &Length, &Marker2);
    if (EFI_ERROR (Status)) {
      FileHandle->Delete (FileHandle);
      Arg->Handle = NULL;
      ShellFreeFileList (&DirList);

      return EFI_LOAD_ERROR;
    }
  }

  Cache = AllocatePool (TotalSize);
  if (Cache == NULL) {
    FileHandle->Delete (FileHandle);
    Arg->Handle = NULL;
    ShellFreeFileList (&DirList);
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // write all the lines back to disk
  //
  NumLines  = 0;
  Type      = USE_CRLF;

  Ptr       = Cache;
  LeftSize  = TotalSize;

  for (Link = FileBuffer.ListHead->Flink; Link != FileBuffer.ListHead; Link = Link->Flink) {
    Line = CR (Link, EFI_EDITOR_LINE, Link, EFI_EDITOR_LINE_LIST);

    if (Line->Type != DEFAULT_TYPE) {
      Type = Line->Type;
    }
    //
    // newline character is at most 4 bytes ( two Unicode characters )
    //
    Length = 4;
    if (Line->Buffer != NULL && Line->Size != 0) {
      if (FileBuffer.FileType == ASCII_FILE) {
        Length += Line->Size;
      } else {
        Length += (Line->Size * 2);
      }
      //
      // end if ASCII_FILE
      //
    }

    //
    // no cache room left, so write cache to disk
    //
    if (LeftSize < Length) {
      Size    = TotalSize - LeftSize;
      Status  = FileHandle->Write (FileHandle, &Size, Cache);
      if (EFI_ERROR (Status)) {
        FileHandle->Delete (FileHandle);
        Arg->Handle = NULL;
        ShellFreeFileList (&DirList);
        FreePool (Cache);

        return EFI_LOAD_ERROR;
      }

      Ptr       = Cache;
      LeftSize  = TotalSize;
    }

    if (Line->Buffer != NULL && Line->Size != 0) {
      if (FileBuffer.FileType == ASCII_FILE) {
        UnicodeToAscii (Line->Buffer, Line->Size, Ptr);
        Length = Line->Size;
      } else {
        Length = (Line->Size * 2);
        CopyMem (Ptr, (CHAR8 *) Line->Buffer, Length);
      }
      //
      // end if ASCII_FILE
      //
      Ptr += Length;
      LeftSize -= Length;

    }
    //
    // end of if Line -> Buffer != NULL && Line -> Size != 0
    //
    // if not the last line , write return buffer to disk
    //
    if (Link->Flink != FileBuffer.ListHead) {
      GetNewLine (Type, NewLineBuffer, &NewLineSize);
      CopyMem (Ptr, (CHAR8 *) NewLineBuffer, NewLineSize);

      Ptr += NewLineSize;
      LeftSize -= NewLineSize;
    }

    NumLines++;
  }

  if (TotalSize != LeftSize) {
    Size    = TotalSize - LeftSize;
    Status  = FileHandle->Write (FileHandle, &Size, Cache);
    if (EFI_ERROR (Status)) {
      FileHandle->Delete (FileHandle);
      Arg->Handle = NULL;
      ShellFreeFileList (&DirList);
      FreePool (Cache);
      return EFI_LOAD_ERROR;
    }
  }

  FreePool (Cache);

  FileBuffer.FileModified = FALSE;

  //
  // set status string
  //
  Str = PoolPrint (L"%d Lines Wrote", NumLines);
  if (Str == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  MainStatusBarSetStatusString (Str);
  EditorFreePool (Str);

  //
  // now everything is ready , you can set the new file name to filebuffer
  //
  if (StrCmp (FileName, FileBuffer.FileName) != 0) {
    //
    // not the same
    //
    FileBufferSetFileName (FileName);
    if (FileBuffer.FileName == NULL) {
      FileHandle->Delete (FileHandle);
      Arg->Handle = NULL;
      ShellFreeFileList (&DirList);

      return EFI_OUT_OF_RESOURCES;
    }
  }

  FileBuffer.ReadOnly = FALSE;

  ShellFreeFileList (&DirList);

  return EFI_SUCCESS;
}
//
// Name:
//   FileBufferHandleInput -- Dispatch input to different handler
// In:
//   Key -- input key
//    the keys can be:
//      ASCII KEY
//      Backspace/Delete
//      Return
//      Direction key: up/down/left/right/pgup/pgdn
//      Home/End
//      INS
// Out:
//   EFI_SUCCESS
//   EFI_LOAD_ERROR
//   EFI_OUT_OF_RESOURCES
//
EFI_STATUS
FileBufferHandleInput (
  IN  EFI_INPUT_KEY *Key
  )
{
  EFI_STATUS  Status;

  Status = EFI_SUCCESS;

  switch (Key->ScanCode) {
  //
  // ordinary key input
  //
  case SCAN_CODE_NULL:
    if (FileBuffer.ReadOnly == FALSE) {
      Status = FileBufferDoCharInput (Key->UnicodeChar);
    } else {
      Status = MainStatusBarSetStatusString (L"Read Only File Can Not Be Modified");
    }

    break;

  //
  // up arrow
  //
  case SCAN_CODE_UP:
    Status = FileBufferScrollUp ();
    break;

  //
  // down arrow
  //
  case SCAN_CODE_DOWN:
    Status = FileBufferScrollDown ();
    break;

  //
  // right arrow
  //
  case SCAN_CODE_RIGHT:
    Status = FileBufferScrollRight ();
    break;

  //
  // left arrow
  //
  case SCAN_CODE_LEFT:
    Status = FileBufferScrollLeft ();
    break;

  //
  // page up
  //
  case SCAN_CODE_PGUP:
    Status = FileBufferPageUp ();
    break;

  //
  // page down
  //
  case SCAN_CODE_PGDN:
    Status = FileBufferPageDown ();
    break;

  //
  // delete
  //
  case SCAN_CODE_DEL:
    if (FileBuffer.ReadOnly == FALSE) {
      Status = FileBufferDoDelete ();
    } else {
      Status = MainStatusBarSetStatusString (L"Read Only File Can Not Be Modified");
    }

    break;

  //
  // home
  //
  case SCAN_CODE_HOME:
    Status = FileBufferHome ();
    break;

  //
  // end
  //
  case SCAN_CODE_END:
    Status = FileBufferEnd ();
    break;

  //
  // insert
  //
  case SCAN_CODE_INS:
    Status = FileBufferChangeMode ();
    break;

  default:
    Status = MainStatusBarSetStatusString (L"Unknown Command");
    break;
  }

  return Status;
}
//
// Name:
//   FileBufferDoCharInput -- ASCII key + Backspace + return
// In:
//   Char -- input char
// Out:
//   EFI_SUCCESS
//   EFI_LOAD_ERROR
//   EFI_OUT_OF_RESOURCES
//
EFI_STATUS
FileBufferDoCharInput (
  IN  CHAR16  Char
  )
{
  EFI_STATUS  Status;

  Status = EFI_SUCCESS;

  switch (Char) {
  case 0:
    break;

  case 0x08:
    Status = FileBufferDoBackspace ();
    break;

  case 0x09:
    //
    // Tabs are thought as nothing
    //
    break;

  case 0x0a:
  case 0x0d:
    //
    // 0x0a, 0x0d all be thought as return
    //
    Status = FileBufferDoReturn ();
    break;

  default:
    //
    // DEAL WITH ASCII CHAR, filter out thing like ctrl+f
    //
    if (Char > 127 || Char < 32) {
      Status = MainStatusBarSetStatusString (L"Unknown Command");
    } else {
      Status = FileBufferAddChar (Char);
    }

    break;

  }

  return Status;
}
//
// Name:
//   FileBufferAddChar -- Add character
// In:
//   Char -- input char
// Out:
//   EFI_SUCCESS
//
EFI_STATUS
FileBufferAddChar (
  IN  CHAR16  Char
  )
{
  EFI_EDITOR_LINE *Line;
  UINTN           FilePos;

  Line = FileBuffer.CurrentLine;

  //
  // only needs to refresh current line
  //
  FileBufferOnlyLineNeedRefresh = TRUE;

  //
  // when is insert mode, or cursor is at end of this line,
  // so insert this character
  // or replace the character.
  //
  FilePos = FileBuffer.FilePosition.Column - 1;
  if (FileBuffer.ModeInsert || FilePos + 1 > Line->Size) {
    LineStrInsert (Line, Char, FilePos, Line->Size + 1);
  } else {
    Line->Buffer[FilePos] = Char;
  }
  //
  // move cursor to right
  //
  FileBufferScrollRight ();

  if (!FileBuffer.FileModified) {
    FileBuffer.FileModified = TRUE;
  }

  return EFI_SUCCESS;
}
//
// Name:
//   InCurrentScreen --
//       Check user specified FileRow and FileCol is in current screen
// In:
//   FileRow -- Row of file position ( start from 1 )
//   FileCol -- Column of file position ( start from 1 )
// Out:
//   TRUE
//   FALSE
//
BOOLEAN
InCurrentScreen (
  IN UINTN FileRow,
  IN UINTN FileCol
  )
{
  //
  // if is just inside this screen
  //
  if (FileRow >= FileBuffer.LowVisibleRange.Row &&
      FileRow <= FileBuffer.LowVisibleRange.Row + NUM_TEXT_ROWS - 1 &&
      FileCol >= FileBuffer.LowVisibleRange.Column &&
      FileCol <= FileBuffer.LowVisibleRange.Column + NUM_TEXT_COLUMNS - 1
      ) {
    return TRUE;
  }

  return FALSE;
}
//
// Name:
//   AboveCurrentScreen --
//        Check user specified FileRow is above current screen
// In:
//   FileRow -- Row of file position ( start from 1 )
// Out:
//   TRUE
//   FALSE
//
BOOLEAN
AboveCurrentScreen (
  IN UINTN FileRow
  )
{
  //
  // if is to the above of the screen
  //
  if (FileRow < FileBuffer.LowVisibleRange.Row) {
    return TRUE;
  }

  return FALSE;
}
//
// Name:
//   UnderCurrentScreen --
//        Check user specified FileRow is under current screen
// In:
//   FileRow -- Row of file position ( start from 1 )
// Out:
//   TRUE
//   FALSE
//
BOOLEAN
UnderCurrentScreen (
  IN UINTN FileRow
  )
{
  //
  // if is to the under of the screen
  //
  if (FileRow > FileBuffer.LowVisibleRange.Row + NUM_TEXT_ROWS - 1) {
    return TRUE;
  }

  return FALSE;
}
//
// Name:
//   LeftCurrentScreen --
//       Check user specified FileCol is left to current screen
// In:
//   FileCol -- Column of file position ( start from 1 )
// Out:
//   TRUE
//   FALSE
//
BOOLEAN
LeftCurrentScreen (
  IN UINTN FileCol
  )
{
  //
  // if is to the left of the screen
  //
  if (FileCol < FileBuffer.LowVisibleRange.Column) {
    return TRUE;
  }

  return FALSE;
}
//
// Name:
//   RightCurrentScreen --
//        Check user specified FileCol is right to current screen
// In:
//   FileCol -- Column of file position ( start from 1 )
// Out:
//   TRUE
//   FALSE
//
BOOLEAN
RightCurrentScreen (
  IN UINTN FileCol
  )
{
  //
  // if is to the right of the screen
  //
  if (FileCol > FileBuffer.LowVisibleRange.Column + NUM_TEXT_COLUMNS - 1) {
    return TRUE;
  }

  return FALSE;
}
//
// Name:
//   FileBufferMovePosition --
//   According to cursor's file position, adjust screen display
// In:
//   NewFilePosRow -- Row of file position ( start from 1 )
//   NewFilePosCol -- Column of file position ( start from 1 )
// Out:
//   VOID
//
VOID
FileBufferMovePosition (
  IN UINTN NewFilePosRow,
  IN UINTN NewFilePosCol
  )
{
  INTN    RowGap;
  INTN    ColGap;
  UINTN   Abs;
  BOOLEAN Above;
  BOOLEAN Under;
  BOOLEAN Right;
  BOOLEAN Left;

  //
  // CALCULATE gap between current file position and new file position
  //
  RowGap  = NewFilePosRow - FileBuffer.FilePosition.Row;
  ColGap  = NewFilePosCol - FileBuffer.FilePosition.Column;

  Under   = UnderCurrentScreen (NewFilePosRow);
  Above   = AboveCurrentScreen (NewFilePosRow);
  //
  // if is below current screen
  //
  if (Under) {
    //
    // display row will be unchanged
    //
    FileBuffer.FilePosition.Row = NewFilePosRow;
  } else {
    if (Above) {
      //
      // has enough above line, so display row unchanged
      // not has enough above lines, so the first line is at the
      // first display line
      //
      if (NewFilePosRow < (FileBuffer.DisplayPosition.Row - TEXT_START_ROW + 1)) {
        FileBuffer.DisplayPosition.Row = NewFilePosRow + TEXT_START_ROW - 1;
      }

      FileBuffer.FilePosition.Row = NewFilePosRow;
    } else {
      //
      // in current screen
      //
      FileBuffer.FilePosition.Row = NewFilePosRow;
      if (RowGap < 0) {
        Abs = -RowGap;
        FileBuffer.DisplayPosition.Row -= Abs;
      } else {
        FileBuffer.DisplayPosition.Row += RowGap;
      }
    }
  }

  FileBuffer.LowVisibleRange.Row  = FileBuffer.FilePosition.Row - (FileBuffer.DisplayPosition.Row - TEXT_START_ROW);

  Right = RightCurrentScreen (NewFilePosCol);
  Left = LeftCurrentScreen (NewFilePosCol);

  //
  // if right to current screen
  //
  if (Right) {
    //
    // display column will be changed to end
    //
    FileBuffer.DisplayPosition.Column = TEXT_END_COLUMN;
    FileBuffer.FilePosition.Column    = NewFilePosCol;
  } else {
    if (Left) {
      //
      // has enough left characters , so display row unchanged
      // not has enough left characters,
      // so the first character is at the first display column
      //
      if (NewFilePosCol < (FileBuffer.DisplayPosition.Column - TEXT_START_COLUMN + 1)) {
        FileBuffer.DisplayPosition.Column = NewFilePosCol + TEXT_START_COLUMN - 1;
      }

      FileBuffer.FilePosition.Column = NewFilePosCol;
    } else {
      //
      // in current screen
      //
      FileBuffer.FilePosition.Column = NewFilePosCol;
      if (ColGap < 0) {
        Abs = -ColGap;
        FileBuffer.DisplayPosition.Column -= Abs;
      } else {
        FileBuffer.DisplayPosition.Column += ColGap;
      }
    }
  }

  FileBuffer.LowVisibleRange.Column = FileBuffer.FilePosition.Column - (FileBuffer.DisplayPosition.Column - TEXT_START_COLUMN);

  //
  // let CurrentLine point to correct line;
  //
  FileBuffer.CurrentLine = MoveCurrentLine (RowGap);

}
//
// Name:
//   FileBufferScrollRight -- Scroll cursor to right
// In:
//   VOID
// Out:
//   EFI_SUCCESS
//
EFI_STATUS
FileBufferScrollRight (
  VOID
  )
{
  EFI_EDITOR_LINE *Line;
  UINTN           FRow;
  UINTN           FCol;

  Line = FileBuffer.CurrentLine;
  if (Line->Buffer == NULL) {
    return EFI_SUCCESS;
  }

  FRow  = FileBuffer.FilePosition.Row;
  FCol  = FileBuffer.FilePosition.Column;

  //
  // if already at end of this line, scroll it to the start of next line
  //
  if (FCol > Line->Size) {
    //
    // has next line
    //
    if (Line->Link.Flink != FileBuffer.ListHead) {
      FRow++;
      FCol = 1;
    } else {
      return EFI_SUCCESS;
    }
  } else {
    //
    // if not at end of this line, just move to next column
    //
    FCol++;
  }

  FileBufferMovePosition (FRow, FCol);

  return EFI_SUCCESS;
}
//
// Name:
//   FileBufferScrollLeft -- Scroll cursor to left
// In:
//   VOID
// Out:
//   EFI_SUCCESS
//
EFI_STATUS
FileBufferScrollLeft (
  VOID
  )
{
  EFI_EDITOR_LINE *Line;
  UINTN           FRow;
  UINTN           FCol;

  Line  = FileBuffer.CurrentLine;

  FRow  = FileBuffer.FilePosition.Row;
  FCol  = FileBuffer.FilePosition.Column;

  //
  // if already at start of this line, so move to the end of previous line
  //
  if (FCol <= 1) {
    //
    // has previous line
    //
    if (Line->Link.Blink != FileBuffer.ListHead) {
      FRow--;
      Line  = CR (Line->Link.Blink, EFI_EDITOR_LINE, Link, EFI_EDITOR_LINE_LIST);
      FCol  = Line->Size + 1;
    } else {
      return EFI_SUCCESS;
    }
  } else {
    //
    // if not at start of this line, just move to previous column
    //
    FCol--;
  }

  FileBufferMovePosition (FRow, FCol);

  return EFI_SUCCESS;
}
//
// Name:
//   FileBufferScrollDown -- Scroll cursor to the next line
// In:
//   VOID
// Out:
//   EFI_SUCCESS
//
EFI_STATUS
FileBufferScrollDown (
  VOID
  )
{
  EFI_EDITOR_LINE *Line;
  UINTN           FRow;
  UINTN           FCol;

  Line = FileBuffer.CurrentLine;
  if (Line->Buffer == NULL) {
    return EFI_SUCCESS;
  }

  FRow  = FileBuffer.FilePosition.Row;
  FCol  = FileBuffer.FilePosition.Column;

  //
  // has next line
  //
  if (Line->Link.Flink != FileBuffer.ListHead) {
    FRow++;
    Line = CR (Line->Link.Flink, EFI_EDITOR_LINE, Link, EFI_EDITOR_LINE_LIST);

    //
    // if the next line is not that long, so move to end of next line
    //
    if (FCol > Line->Size) {
      FCol = Line->Size + 1;
    }

  } else {
    return EFI_SUCCESS;
  }

  FileBufferMovePosition (FRow, FCol);

  return EFI_SUCCESS;
}
//
// Name:
//   FileBufferScrollUp -- Scroll cursor to previous line
// In:
//   VOID
// Out:
//   EFI_SUCCESS
//
EFI_STATUS
FileBufferScrollUp (
  VOID
  )
{
  EFI_EDITOR_LINE *Line;
  UINTN           FRow;
  UINTN           FCol;

  Line  = FileBuffer.CurrentLine;

  FRow  = FileBuffer.FilePosition.Row;
  FCol  = FileBuffer.FilePosition.Column;

  //
  // has previous line
  //
  if (Line->Link.Blink != FileBuffer.ListHead) {
    FRow--;
    Line = CR (Line->Link.Blink, EFI_EDITOR_LINE, Link, EFI_EDITOR_LINE_LIST);

    //
    // if previous line is not that long, so move to the end of previous line
    //
    if (FCol > Line->Size) {
      FCol = Line->Size + 1;
    }

  } else {
    return EFI_SUCCESS;
  }

  FileBufferMovePosition (FRow, FCol);

  return EFI_SUCCESS;
}
//
// Name:
//   FileBufferPageDown -- Scroll cursor to next page
// In:
//   VOID
// Out:
//   EFI_SUCCESS
//
EFI_STATUS
FileBufferPageDown (
  VOID
  )
{
  EFI_EDITOR_LINE *Line;
  UINTN           FRow;
  UINTN           FCol;
  UINTN           Gap;

  Line  = FileBuffer.CurrentLine;

  FRow  = FileBuffer.FilePosition.Row;
  FCol  = FileBuffer.FilePosition.Column;

  //
  // has next page
  //
  if (FileBuffer.NumLines >= FRow + NUM_TEXT_ROWS) {
    Gap = NUM_TEXT_ROWS;
  } else {
    //
    // MOVE CURSOR TO LAST LINE
    //
    Gap = FileBuffer.NumLines - FRow;
  }
  //
  // get correct line
  //
  Line = MoveLine (Gap);

  //
  // if that line, is not that long, so move to the end of that line
  //
  if (FCol > Line->Size) {
    FCol = Line->Size + 1;
  }

  FRow += Gap;

  FileBufferMovePosition (FRow, FCol);

  return EFI_SUCCESS;
}
//
// Name:
//   FileBufferPageUp -- Scroll cursor to previous line
// In:
//   VOID
// Out:
//   EFI_SUCCESS
//
EFI_STATUS
FileBufferPageUp (
  VOID
  )
{
  EFI_EDITOR_LINE *Line;
  UINTN           FRow;
  UINTN           FCol;
  UINTN           Gap;
  INTN            Retreat;

  Line  = FileBuffer.CurrentLine;

  FRow  = FileBuffer.FilePosition.Row;
  FCol  = FileBuffer.FilePosition.Column;

  //
  // has previous page
  //
  if (FRow > NUM_TEXT_ROWS) {
    Gap = NUM_TEXT_ROWS;
  } else {
    //
    // the first line of file will displayed on the first line of screen
    //
    Gap = FRow - 1;
  }

  Retreat = Gap;
  Retreat = -Retreat;

  //
  // get correct line
  //
  Line = MoveLine (Retreat);

  //
  // if that line is not that long, so move to the end of that line
  //
  if (FCol > Line->Size) {
    FCol = Line->Size + 1;
  }

  FRow -= Gap;

  FileBufferMovePosition (FRow, FCol);

  return EFI_SUCCESS;
}
//
// Name:
//   FileBufferHome -- Scroll cursor to start of line
// In:
//   VOID
// Out:
//   EFI_SUCCESS
//
EFI_STATUS
FileBufferHome (
  VOID
  )
{
  EFI_EDITOR_LINE *Line;
  UINTN           FRow;
  UINTN           FCol;

  //
  // go to the first column of the line
  //
  Line  = FileBuffer.CurrentLine;

  FRow  = FileBuffer.FilePosition.Row;
  FCol  = 1;

  FileBufferMovePosition (FRow, FCol);

  return EFI_SUCCESS;
}
//
// Name:
//   FileBufferEnd -- Scroll cursor to end of line
// In:
//   VOID
// Out:
//   EFI_SUCCESS
//
EFI_STATUS
FileBufferEnd (
  VOID
  )
{
  EFI_EDITOR_LINE *Line;
  UINTN           FRow;
  UINTN           FCol;

  Line  = FileBuffer.CurrentLine;

  FRow  = FileBuffer.FilePosition.Row;

  //
  // goto the last column of the line
  //
  FCol = Line->Size + 1;

  FileBufferMovePosition (FRow, FCol);

  return EFI_SUCCESS;
}
//
// Name:
//   FileBufferDoReturn -- Add a return into line
// In:
//   VOID
// Out:
//   EFI_SUCCESS
//   EFI_OUT_OF_RESOURCES
//
EFI_STATUS
FileBufferDoReturn (
  VOID
  )
{
  EFI_EDITOR_LINE *Line;
  EFI_EDITOR_LINE *NewLine;
  UINTN           FileColumn;
  UINTN           Index;
  CHAR16          *Buffer;
  UINTN           Row;
  UINTN           Col;

  FileBufferNeedRefresh         = TRUE;
  FileBufferOnlyLineNeedRefresh = FALSE;

  Line                          = FileBuffer.CurrentLine;

  FileColumn                    = FileBuffer.FilePosition.Column;

  NewLine                       = AllocatePool (sizeof (EFI_EDITOR_LINE));
  if (NewLine == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  NewLine->Signature  = EFI_EDITOR_LINE_LIST;
  NewLine->Size       = Line->Size - FileColumn + 1;
  NewLine->TotalSize  = NewLine->Size;
  NewLine->Buffer     = PoolPrint (L"\0");
  if (NewLine->Buffer == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  NewLine->Type = DEFAULT_TYPE;

  if (NewLine->Size > 0) {
    //
    // UNICODE + '\0'
    //
    Buffer = AllocatePool (2 * (NewLine->Size + 1));
    if (Buffer == NULL) {
      FreePool (NewLine->Buffer);
      FreePool (NewLine);
      return EFI_OUT_OF_RESOURCES;
    }

    FreePool (NewLine->Buffer);

    NewLine->Buffer = Buffer;

    for (Index = 0; Index < NewLine->Size; Index++) {
      NewLine->Buffer[Index] = Line->Buffer[Index + FileColumn - 1];
    }

    NewLine->Buffer[NewLine->Size]  = L'\0';

    Line->Buffer[FileColumn - 1]    = L'\0';
    Line->Size                      = FileColumn - 1;
  }
  //
  // increase NumLines
  //
  FileBuffer.NumLines++;

  //
  // insert it into the correct position of line list
  //
  NewLine->Link.Blink     = &(Line->Link);
  NewLine->Link.Flink     = Line->Link.Flink;
  Line->Link.Flink->Blink = &(NewLine->Link);
  Line->Link.Flink        = &(NewLine->Link);

  //
  // move cursor to the start of next line
  //
  Row = FileBuffer.FilePosition.Row + 1;
  Col = 1;

  FileBufferMovePosition (Row, Col);

  //
  // set file is modified
  //
  if (!FileBuffer.FileModified) {
    FileBuffer.FileModified = TRUE;
  }

  return EFI_SUCCESS;
}
//
// Name:
//   FileBufferDoBackspace -- delete the previous character
// In:
//   VOID
// Out:
//   EFI_SUCCESS
//   EFI_OUT_OF_RESOURCES
//
EFI_STATUS
FileBufferDoBackspace (
  VOID
  )
{
  EFI_EDITOR_LINE *Line;
  EFI_EDITOR_LINE *End;
  EFI_LIST_ENTRY  *Link;
  UINTN           FileColumn;

  FileColumn  = FileBuffer.FilePosition.Column;

  Line        = FileBuffer.CurrentLine;

  //
  // the first column
  //
  if (FileColumn == 1) {
    //
    // the first row
    //
    if (FileBuffer.FilePosition.Row == 1) {
      return EFI_SUCCESS;
    }

    FileBufferScrollLeft ();

    Line  = FileBuffer.CurrentLine;
    Link  = Line->Link.Flink;
    End   = CR (Link, EFI_EDITOR_LINE, Link, EFI_EDITOR_LINE_LIST);

    //
    // concatenate this line with previous line
    //
    LineCat (Line, End);
    if (Line->Buffer == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }
    //
    // remove End from line list
    //
    RemoveEntryList (&End->Link);
    FreePool (End);

    FileBuffer.NumLines--;

    FileBufferNeedRefresh         = TRUE;
    FileBufferOnlyLineNeedRefresh = FALSE;

  } else {
    //
    // just delete the previous character
    //
    LineDeleteAt (Line, FileColumn - 1);
    FileBufferScrollLeft ();
    FileBufferOnlyLineNeedRefresh = TRUE;
  }

  if (!FileBuffer.FileModified) {
    FileBuffer.FileModified = TRUE;
  }

  return EFI_SUCCESS;
}
//
// Name:
//   FileBufferDelete -- Delete current character from line
// In:
//   VOID
// Out:
//   EFI_SUCCESS
//
EFI_STATUS
FileBufferDoDelete (
  VOID
  )
{
  EFI_EDITOR_LINE *Line;
  EFI_EDITOR_LINE *Next;
  EFI_LIST_ENTRY  *Link;
  UINTN           FileColumn;

  Line        = FileBuffer.CurrentLine;
  FileColumn  = FileBuffer.FilePosition.Column;

  //
  // the last column
  //
  if (FileColumn >= Line->Size + 1) {
    //
    // the last line
    //
    if (Line->Link.Flink == FileBuffer.ListHead) {
      return EFI_SUCCESS;
    }
    //
    // since last character,
    // so will add the next line to this line
    //
    Link  = Line->Link.Flink;
    Next  = CR (Link, EFI_EDITOR_LINE, Link, EFI_EDITOR_LINE_LIST);
    LineCat (Line, Next);
    if (Line->Buffer == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }

    RemoveEntryList (&Next->Link);
    FreePool (Next);

    FileBuffer.NumLines--;

    FileBufferNeedRefresh         = TRUE;
    FileBufferOnlyLineNeedRefresh = FALSE;

  } else {
    //
    // just delete current character
    //
    LineDeleteAt (Line, FileColumn);
    FileBufferOnlyLineNeedRefresh = TRUE;
  }

  if (!FileBuffer.FileModified) {
    FileBuffer.FileModified = TRUE;
  }

  return EFI_SUCCESS;
}
//
// Name:
//   FileBufferChangeMode -- Change between INS/OVR mode
// In:
//   VOID
// Out:
//   EFI_SUCCESS
//
EFI_STATUS
FileBufferChangeMode (
  VOID
  )
{
  //
  // Insert mode or Overwrite mode
  //
  FileBuffer.ModeInsert = (BOOLEAN)!FileBuffer.ModeInsert;
  return EFI_SUCCESS;
}
//
// Name:
//   FileBufferCutLine -- Cut current line out
// In:
//   CutLine -- to pass current line out
// Out:
//   EFI_SUCCESS
//   EFI_NOT_FOUND
//   EFI_OUT_OF_RESOURCES
//
EFI_STATUS
FileBufferCutLine (
  OUT EFI_EDITOR_LINE **CutLine
  )
{
  EFI_EDITOR_LINE *Line;
  EFI_EDITOR_LINE *NewLine;
  UINTN           Row;
  UINTN           Col;

  if (FileBuffer.ReadOnly) {
    MainStatusBarSetStatusString (L"Read Only File Can Not Be Modified");
    return EFI_SUCCESS;
  }

  Line = FileBuffer.CurrentLine;

  //
  // if is the last dummy line, SO CAN not cut
  //
  if (StrCmp (Line->Buffer, L"\0") == 0 && Line->Link.Flink == FileBuffer.ListHead
  //
  // last line
  //
  ) {
    //
    // LAST LINE AND NOTHING ON THIS LINE, SO CUT NOTHING
    //
    MainStatusBarSetStatusString (L"Nothing to Cut");
    return EFI_NOT_FOUND;
  }
  //
  // if is the last line, so create a dummy line
  //
  if (Line->Link.Flink == FileBuffer.ListHead) {
    //
    // last line
    // create a new line
    //
    NewLine = FileBufferCreateLine ();
    if (NewLine == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }
  }

  FileBuffer.NumLines--;
  Row = FileBuffer.FilePosition.Row;
  Col = 1;
  //
  // move home
  //
  FileBuffer.CurrentLine = CR (
                            FileBuffer.CurrentLine->Link.Flink,
                            EFI_EDITOR_LINE,
                            Link,
                            EFI_EDITOR_LINE_LIST
                            );

  RemoveEntryList (&Line->Link);

  FileBuffer.Lines = CR (FileBuffer.ListHead->Flink, EFI_EDITOR_LINE, Link, EFI_EDITOR_LINE_LIST);

  FileBufferMovePosition (Row, Col);

  FileBuffer.FileModified       = TRUE;
  FileBufferNeedRefresh         = TRUE;
  FileBufferOnlyLineNeedRefresh = FALSE;

  *CutLine                      = Line;

  return EFI_SUCCESS;
}
//
// Name:
//   FileBufferPasteLine -- Paste a line into line list
// In:
//   VOID
// Out:
//   EFI_SUCCESS
//   EFI_OUT_OF_RESOURCES
//
EFI_STATUS
FileBufferPasteLine (
  VOID
  )
{
  EFI_EDITOR_LINE *Line;
  EFI_EDITOR_LINE *NewLine;
  UINTN           Row;
  UINTN           Col;

  //
  // if nothing is on clip board
  // then do nothing
  //
  if (MainEditor.CutLine == NULL) {
    return EFI_SUCCESS;
  }
  //
  // read only file can not be pasted on
  //
  if (FileBuffer.ReadOnly) {
    MainStatusBarSetStatusString (L"Read Only File Can Not Be Modified");
    return EFI_SUCCESS;
  }

  NewLine = LineDup (MainEditor.CutLine);
  if (NewLine == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  //
  // insert it above current line
  //
  Line                    = FileBuffer.CurrentLine;
  NewLine->Link.Blink     = Line->Link.Blink;
  NewLine->Link.Flink     = &Line->Link;

  Line->Link.Blink->Flink = &NewLine->Link;
  Line->Link.Blink        = &NewLine->Link;

  FileBuffer.NumLines++;
  FileBuffer.CurrentLine  = NewLine;

  FileBuffer.Lines        = CR (FileBuffer.ListHead->Flink, EFI_EDITOR_LINE, Link, EFI_EDITOR_LINE_LIST);

  Col                     = 1;
  //
  // move home
  //
  Row = FileBuffer.FilePosition.Row;

  FileBufferMovePosition (Row, Col);

  //
  // after paste, set some value so that refresh knows to do something
  //
  FileBuffer.FileModified       = TRUE;
  FileBufferNeedRefresh         = TRUE;
  FileBufferOnlyLineNeedRefresh = FALSE;

  return EFI_SUCCESS;
}
//
// Name:
//   FileBufferSearch -- Search string from current position on in file
// In:
//   Str -- search string
//   Offset -- Offset from current position
// Out:
//   EFI_SUCCESS
//   EFI_NOT_FOUND: Such string not found
//
EFI_STATUS
FileBufferSearch (
  IN CHAR16 *Str,
  IN UINTN  Offset
  )
{
  CHAR16          *Current;
  UINTN           Position;
  UINTN           Row;
  UINTN           Column;
  EFI_EDITOR_LINE *Line;
  EFI_LIST_ENTRY  *Link;
  BOOLEAN         Found;

  Column = 0;

  //
  // search if in current line
  //
  Current = FileBuffer.CurrentLine->Buffer + FileBuffer.FilePosition.Column - 1 + Offset;

  if (Current >= (FileBuffer.CurrentLine->Buffer + FileBuffer.CurrentLine->Size)) {
    //
    // the end
    //
    Current = FileBuffer.CurrentLine->Buffer + FileBuffer.CurrentLine->Size;
  }

  Position = StrStr (Current, Str);

  //
  // found
  //
  if (Position) {
    Column  = (Position - 1) + FileBuffer.FilePosition.Column + Offset;
    Row     = FileBuffer.FilePosition.Row;
    Found   = TRUE;
  } else {
    //
    // not found so find through next lines
    //
    Link  = FileBuffer.CurrentLine->Link.Flink;

    Row   = FileBuffer.FilePosition.Row + 1;
    while (Link != FileBuffer.ListHead) {
      Line      = CR (Link, EFI_EDITOR_LINE, Link, EFI_EDITOR_LINE_LIST);
      Position  = StrStr (Line->Buffer, Str);
      if (Position) {
        //
        // found
        //
        Column = Position;
        break;
      }

      Row++;
      Link = Link->Flink;
    }

    if (Link == FileBuffer.ListHead) {
      Found = FALSE;
    } else {
      Found = TRUE;
    }
  }

  if (!Found) {
    return EFI_NOT_FOUND;
  }

  FileBufferMovePosition (Row, Column);

  //
  // call refresh to fresh edit area,
  // because the outer may loop to find multiply occurrence of this string
  //
  FileBufferRefresh ();

  return EFI_SUCCESS;
}
//
// Name:
//   FileBufferReplace --
//      Replace SearchLen characters from current position on with Replace
// In:
//   Replace -- string to replace
//   SearchLen -- Search string's length
// Out:
//   EFI_SUCCESS
//   EFI_OUT_OF_RESOURCES
//
EFI_STATUS
FileBufferReplace (
  IN CHAR16   *Replace,
  IN UINTN    SearchLen
  )
{
  UINTN   ReplaceLen;
  UINTN   Index;
  CHAR16  *Buffer;
  UINTN   NewSize;
  UINTN   OldSize;
  UINTN   Gap;

  ReplaceLen  = StrLen (Replace);

  OldSize     = FileBuffer.CurrentLine->Size + 1;
  //
  // include '\0'
  //
  NewSize = OldSize + (ReplaceLen - SearchLen);

  if (ReplaceLen > SearchLen) {
    //
    // do not have the enough space
    //
    if (FileBuffer.CurrentLine->TotalSize + 1 <= NewSize) {
      FileBuffer.CurrentLine->Buffer = ReallocatePool (
                                        FileBuffer.CurrentLine->Buffer,
                                        2 * OldSize,
                                        2 * NewSize
                                        );
      FileBuffer.CurrentLine->TotalSize = NewSize - 1;
    }

    if (FileBuffer.CurrentLine->Buffer == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }
    //
    // the end '\0' character;
    //
    Buffer  = FileBuffer.CurrentLine->Buffer + (NewSize - 1);
    Gap     = ReplaceLen - SearchLen;

    //
    // keep the latter part
    //
    for (Index = 0; Index < (FileBuffer.CurrentLine->Size - FileBuffer.FilePosition.Column - SearchLen + 2); Index++) {
      *Buffer = *(Buffer - Gap);
      Buffer--;
    }
    //
    // set replace into it
    //
    Buffer = FileBuffer.CurrentLine->Buffer + FileBuffer.FilePosition.Column - 1;
    for (Index = 0; Index < ReplaceLen; Index++) {
      Buffer[Index] = Replace[Index];
    }
  }

  if (ReplaceLen < SearchLen) {
    Buffer = FileBuffer.CurrentLine->Buffer + FileBuffer.FilePosition.Column - 1;

    for (Index = 0; Index < ReplaceLen; Index++) {
      Buffer[Index] = Replace[Index];
    }

    Buffer += ReplaceLen;
    Gap = SearchLen - ReplaceLen;

    //
    // set replace into it
    //
    for (Index = 0; Index < (FileBuffer.CurrentLine->Size - FileBuffer.FilePosition.Column - ReplaceLen + 2); Index++) {
      *Buffer = *(Buffer + Gap);
      Buffer++;
    }
  }

  if (ReplaceLen == SearchLen) {
    Buffer = FileBuffer.CurrentLine->Buffer + FileBuffer.FilePosition.Column - 1;
    for (Index = 0; Index < ReplaceLen; Index++) {
      Buffer[Index] = Replace[Index];
    }
  }

  FileBuffer.CurrentLine->Size += (ReplaceLen - SearchLen);

  FileBufferOnlyLineNeedRefresh = TRUE;

  FileBuffer.FileModified       = TRUE;

  MainTitleBarRefresh ();
  FileBufferRefresh ();

  return EFI_SUCCESS;
}

VOID
FileBufferAdjustMousePosition (
  IN INT32 TextX,
  IN INT32 TextY
  )
{
  UINTN X;
  UINTN Y;
  UINTN AbsX;
  UINTN AbsY;

  //
  // TextX and TextY is mouse movement data returned by mouse driver
  // This function will change it to MousePosition
  //
  //
  // get absolute value
  //
  if (TextX >= 0) {
    AbsX = TextX;
  } else {
    AbsX = -TextX;
  }

  if (TextY >= 0) {
    AbsY = TextY;
  } else {
    AbsY = -TextY;
  }

  X = FileBuffer.MousePosition.Column;
  Y = FileBuffer.MousePosition.Row;

  if (TextX >= 0) {
    X += TextX;
  } else {
    if (X >= AbsX) {
      X -= AbsX;
    } else {
      X = 0;
    }
  }

  if (TextY >= 0) {
    Y += TextY;
  } else {
    if (Y >= AbsY) {
      Y -= AbsY;
    } else {
      Y = 0;
    }
  }
  //
  // check whether new mouse column position is beyond screen
  // if not, adjust it
  //
  if (X >= TEXT_START_COLUMN && X <= TEXT_END_COLUMN) {
    FileBuffer.MousePosition.Column = X;
  } else if (X < TEXT_START_COLUMN) {
    FileBuffer.MousePosition.Column = TEXT_START_COLUMN;
  } else if (X > TEXT_END_COLUMN) {
    FileBuffer.MousePosition.Column = TEXT_END_COLUMN;
  }
  //
  // check whether new mouse row position is beyond screen
  // if not, adjust it
  //
  if (Y >= TEXT_START_ROW && Y <= TEXT_END_ROW) {
    FileBuffer.MousePosition.Row = Y;
  } else if (Y < TEXT_START_ROW) {
    FileBuffer.MousePosition.Row = TEXT_START_ROW;
  } else if (Y > TEXT_END_ROW) {
    FileBuffer.MousePosition.Row = TEXT_END_ROW;
  }

}

EFI_STATUS
FileBufferReplaceAll (
  IN CHAR16 *SearchStr,
  IN CHAR16 *ReplaceStr,
  IN UINTN  Offset
  )
{

  CHAR16          *Buffer;
  UINTN           Position;
  UINTN           Column;
  UINTN           ReplaceLen;
  UINTN           SearchLen;
  UINTN           Index;
  UINTN           NewSize;
  UINTN           OldSize;
  UINTN           Gap;
  EFI_EDITOR_LINE *Line;
  EFI_LIST_ENTRY  *Link;

  SearchLen   = StrLen (SearchStr);
  ReplaceLen  = StrLen (ReplaceStr);

  Column      = FileBuffer.FilePosition.Column + Offset - 1;

  if (Column > FileBuffer.CurrentLine->Size) {
    Column = FileBuffer.CurrentLine->Size;
  }

  Link = &(FileBuffer.CurrentLine->Link);

  while (Link != FileBuffer.ListHead) {
    Line      = CR (Link, EFI_EDITOR_LINE, Link, EFI_EDITOR_LINE_LIST);
    Position  = StrStr (Line->Buffer + Column, SearchStr);
    if (Position) {
      //
      // found
      //
      Column += Position - 1;

      OldSize = Line->Size + 1;
      //
      // include '\0'
      //
      NewSize = OldSize + (ReplaceLen - SearchLen);

      if (ReplaceLen > SearchLen) {
        //
        // do not have the enough space
        //
        if (Line->TotalSize + 1 <= NewSize) {
          Line->Buffer = ReallocatePool (
                          Line->Buffer,
                          2 * OldSize,
                          2 * NewSize
                          );
          Line->TotalSize = NewSize - 1;
        }

        if (Line->Buffer == NULL) {
          return EFI_OUT_OF_RESOURCES;
        }
        //
        // the end '\0' character;
        //
        Buffer  = Line->Buffer + (NewSize - 1);
        Gap     = ReplaceLen - SearchLen;

        //
        // keep the latter part
        //
        for (Index = 0; Index < (Line->Size - Column - SearchLen + 1); Index++) {
          *Buffer = *(Buffer - Gap);
          Buffer--;
        }

      } else {
        Buffer  = Line->Buffer + Column + ReplaceLen;
        Gap     = SearchLen - ReplaceLen;

        for (Index = 0; Index < (Line->Size - Column - ReplaceLen + 1); Index++) {
          *Buffer = *(Buffer + Gap);
          Buffer++;
        }
      }
      //
      // set replace into it
      //
      Buffer = Line->Buffer + Column;
      for (Index = 0; Index < ReplaceLen; Index++) {
        Buffer[Index] = ReplaceStr[Index];
      }

      Line->Size += (ReplaceLen - SearchLen);
      Column += ReplaceLen;
    } else {
      //
      // not found
      //
      Column  = 0;
      Link    = Link->Flink;
    }
  }
  //
  // call refresh to fresh edit area
  //
  FileBuffer.FileModified = TRUE;
  FileBufferNeedRefresh   = TRUE;
  FileBufferRefresh ();

  return EFI_SUCCESS;
}

CHAR16 *
EditGetDefaultFileName (
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
      FileNameTmp = PoolPrint (L"NewFile%d.txt", Suffix);
    } else {
      FileNameTmp = PoolPrint (L"NewFile.txt");
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
