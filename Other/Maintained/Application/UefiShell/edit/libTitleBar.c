/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

  Module Name:
    libTitleBar.c

Abstract:

  The Title bar library

--*/

#include "editor.h"

EFI_EDITOR_TITLE_BAR          MainTitleBar;

EFI_EDITOR_TITLE_BAR          MainTitleBarConst = { NULL };

extern EFI_EDITOR_FILE_BUFFER FileBufferBackupVar;
extern EFI_EDITOR_FILE_BUFFER FileBuffer;

//
// Name:
//     MainTitleBarInit -- Init function for MainTitleBar
// In:
//     VOID
// Out:
//     EFI_SUCCESS
//     EFI_OUT_OF_RESOURCES
//
EFI_STATUS
MainTitleBarInit (
  VOID
  )
{
  //
  // basic initialization for MainTitleBar
  //
  CopyMem (&MainTitleBar, &MainTitleBarConst, sizeof (MainTitleBar));

  //
  // set TitlePrefix
  //
  MainTitleBar.TitlePrefix = PoolPrint (L"  %s  %s", EDITOR_NAME, EDITOR_VERSION);
  if (MainTitleBar.TitlePrefix == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  return EFI_SUCCESS;
}
//
// Name:
//     MainTitleBarCleanup -- cleanup function for MainTitleBar
// In:
//     VOID
// Out:
//     EFI_SUCCESS
//
EFI_STATUS
MainTitleBarCleanup (
  VOID
  )
{
  //
  // free the title prefix
  //
  EditorFreePool (MainTitleBar.TitlePrefix);

  return EFI_SUCCESS;
}
EFI_STATUS
MainTitleBarRefresh (
  VOID
  )
/*++
Routine Description:
    
    Refresh function for MainTitleBar

Arguments:

    VOID

Returns:

    EFI_SUCCESS
   
--*/
{
  EFI_EDITOR_COLOR_UNION  Orig;

  EFI_EDITOR_COLOR_UNION  New;
  CHAR16                  *FileNameTmp;
  INTN                    i;

  //
  // if it's first time when editor launch, so refresh is mandatory
  //
  if (!EditorFirst) {
    //
    // if all the fields are unchanged, no need to refresh
    //
    if (StrCmp (FileBufferBackupVar.FileName, FileBuffer.FileName) == 0 &&
        FileBufferBackupVar.FileType == FileBuffer.FileType &&
        FileBufferBackupVar.FileModified == FileBuffer.FileModified &&
        FileBufferBackupVar.ReadOnly == FileBuffer.ReadOnly
        ) {
      return EFI_SUCCESS;
    }
  }
  //
  // backup the old screen attributes
  //
  Orig                  = MainEditor.ColorAttributes;
  New.Colors.Foreground = Orig.Colors.Background;
  New.Colors.Background = Orig.Colors.Foreground;

  Out->SetAttribute (Out, New.Data);

  //
  // clear the old title bar
  //
  EditorClearLine (TITLE_BAR_LOCATION);

  //
  // print the new title bar prefix
  // totally 25 chars
  //
  PrintAt (
    TEXT_START_COLUMN - 1,
    TITLE_BAR_LOCATION - 1,
    L"%s     ",
    MainTitleBar.TitlePrefix
    );

  if (MainEditor.FileBuffer->FileName == NULL) {
    Out->SetAttribute (Out, Orig.Data);
    FileBufferRestorePosition ();
    return EFI_SUCCESS;
  }
  //
  // First Extract the FileName from fullpath
  //
  FileNameTmp = MainEditor.FileBuffer->FileName;
  for (i = StrLen (FileNameTmp) - 1; i >= 0; i--) {
    if (FileNameTmp[i] == L'\\') {
      break;
    }
  }

  i++;

  FileNameTmp = FileNameTmp + i;

  //
  // the space for file name is 20 characters
  //
  if (StrLen (FileNameTmp) <= 20) {
    Print (L"%s   ", FileNameTmp);

    for (i = StrLen (FileNameTmp); i < 20; i++) {
      Print (L" ");
    }

  } else {
    for (i = 0; i < 17; i++) {
      Print (L"%c", FileNameTmp[i]);
    }
    //
    // print "..."
    //
    Print (L"...   ");
  }
  //
  // print file type field
  //
  if (MainEditor.FileBuffer->FileType == ASCII_FILE) {
    Print (L"     ASCII     ");
  } else {
    Print (L"     UNICODE   ");
  }
  //
  // print read-only field
  //
  if (MainEditor.FileBuffer->ReadOnly) {
    Print (L"ReadOnly   ");
  } else {
    Print (L"           ");
  }
  //
  // print modified field
  //
  if (MainEditor.FileBuffer->FileModified) {
    Print (L"Modified");
  }
  //
  // restore the old attribute
  //
  Out->SetAttribute (Out, Orig.Data);

  //
  // restore edit area cursor position
  //
  FileBufferRestorePosition ();

  return EFI_SUCCESS;
}
//
// Name:
//     MainTitleBarBackup -- Backup function for MainTitleBar
// In:
//     VOID
// Out:
//     EFI_SUCCESS
//
EFI_STATUS
MainTitleBarBackup (
  VOID
  )
{
  //
  // nothing to backup for title bar
  // keep this just to keep consistency
  //
  return EFI_SUCCESS;
}
