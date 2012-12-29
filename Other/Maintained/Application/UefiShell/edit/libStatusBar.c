/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

  Module Name:
    libStatusBar.c

  Abstract:
    Definition for the Status Bar - 
    the display for the status of the editor

--*/

#include "editor.h"

EFI_EDITOR_STATUS_BAR         MainStatusBar;
EFI_EDITOR_STATUS_BAR         MainStatusBarBackupVar;

BOOLEAN                       StatusBarNeedRefresh;
BOOLEAN                       StatusStringChanged;

extern EFI_EDITOR_FILE_BUFFER FileBuffer;
extern EFI_EDITOR_FILE_BUFFER FileBufferBackupVar;

EFI_EDITOR_STATUS_BAR         MainStatusBarConst = { NULL };

//
// Name:
//    MainStatusBarInit -- Init function for MainStatusBar
// In:
//    VOID
// Out:
//    EFI_SUCCESS
//
EFI_STATUS
MainStatusBarInit (
  VOID
  )
{
  //
  // initialize the statusbar and statusbarbackupvar
  //
  CopyMem (&MainStatusBar, &MainStatusBarConst, sizeof (MainStatusBar));
  CopyMem (
    &MainStatusBarBackupVar,
    &MainStatusBarConst,
    sizeof (MainStatusBarBackupVar)
    );

  //
  // status string set to ""
  //
  MainStatusBarSetStatusString (L"");

  StatusBarNeedRefresh = TRUE;

  return EFI_SUCCESS;
}
//
// Name:
//    MainStatusBarBackup -- Backup function for MainStatusBar
// In:
//    VOID
// Out:
//    EFI_SUCCESS
//
EFI_STATUS
MainStatusBarBackup (
  VOID
  )
{
  //
  // backup the status string
  //
  EditorFreePool (MainStatusBarBackupVar.StatusString);
  MainStatusBarBackupVar.StatusString = PoolPrint (L"%s", MainStatusBar.StatusString);

  return EFI_SUCCESS;
}
//
// Name:
//    MainStatusBarCleanup -- Cleanup function for MainStatusBar
// In:
//    VOID
// Out:
//    EFI_SUCCESS
//
EFI_STATUS
MainStatusBarCleanup (
  VOID
  )
{
  //
  // free the status string and backvar's status string
  //
  EditorFreePool ((VOID *) MainStatusBar.StatusString);
  EditorFreePool ((VOID *) MainStatusBarBackupVar.StatusString);

  return EFI_SUCCESS;
}

EFI_STATUS
MainStatusBarRefresh (
  VOID
  )
/*++
Routine Description:

    Refresh function for MainStatusBar
    
Arguments:

    VOID
    
Returns:

    EFI_SUCCESS

--*/
{
  EFI_EDITOR_COLOR_UNION  Orig;

  EFI_EDITOR_COLOR_UNION  New;

  //
  // if the old status string is the same with current
  // status string, so clear it
  //
  if (!StatusStringChanged && StatusBarNeedRefresh && (StrCmp (MainStatusBarBackupVar.StatusString, L"\0") != 0)) {
    MainStatusBarSetStatusString (L"\0");
  }
  //
  // when it's called first time after editor launch, so refresh is mandatory
  //
  if (!EditorFirst && !StatusBarNeedRefresh) {
    //
    // all elements has been unchanged
    //
    if (!StatusBarNeedRefresh &&
        !StatusStringChanged &&
        FileBufferBackupVar.FilePosition.Row == FileBuffer.FilePosition.Row &&
        FileBufferBackupVar.FilePosition.Column == FileBuffer.FilePosition.Column &&
        FileBufferBackupVar.ModeInsert == FileBuffer.ModeInsert
        ) {
      return EFI_SUCCESS;
    }

  }
  //
  // back up the screen attributes
  //
  Orig                  = MainEditor.ColorAttributes;
  New.Colors.Foreground = Orig.Colors.Background;
  New.Colors.Background = Orig.Colors.Foreground;

  Out->EnableCursor (Out, FALSE);
  Out->SetAttribute (Out, New.Data);

  //
  // clear status bar
  //
  EditorClearLine (STATUS_BAR_LOCATION);

  //
  // print row, column fields
  //
  PrintAt (
    0,
    STATUS_BAR_LOCATION - 1,
    L"  Row: %d  Col: %d       %s",
    MainEditor.FileBuffer->FilePosition.Row,
    MainEditor.FileBuffer->FilePosition.Column,
    MainStatusBar.StatusString
    );

  //
  // print insert mode field
  //
  if (MainEditor.FileBuffer->ModeInsert) {
    PrintAt (MAX_TEXT_COLUMNS - 10, STATUS_BAR_LOCATION - 1, L"|%s|", L"INS");
  } else {
    PrintAt (MAX_TEXT_COLUMNS - 10, STATUS_BAR_LOCATION - 1, L"|%s|", L"OVR");
  }
  //
  // restore the old screen attributes
  //
  Out->SetAttribute (Out, Orig.Data);

  //
  // restore position in edit area
  //
  FileBufferRestorePosition ();
  Out->EnableCursor (Out, TRUE);

  StatusBarNeedRefresh  = FALSE;
  StatusStringChanged   = FALSE;

  return EFI_SUCCESS;
}
//
// Name:
//     MainStatusBarSetStatusString --
//         Set the StatusString field for MainStatusBar
// In:
//     Str -- status string to set
// Out:
//     EFI_SUCCESS
//     EFI_OUT_OF_RESOURCES
//
EFI_STATUS
MainStatusBarSetStatusString (
  IN CHAR16 *Str
  )
{
  StatusStringChanged = TRUE;

  //
  // free the old status string
  //
  EditorFreePool (MainStatusBar.StatusString);

  MainStatusBar.StatusString = StrDuplicate (Str);
  if (MainStatusBar.StatusString == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  return EFI_SUCCESS;
}
