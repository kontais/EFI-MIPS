/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

  Module Name:
    libEditor.c

  Abstract:
    Defines the Main Editor data type - 
     - Global variables 
     - Instances of the other objects of the editor
     - Main Interfaces

--*/

#include "editor.h"

EFI_EDITOR_COLOR_ATTRIBUTES   OriginalColors;
INTN                          OriginalMode;

//
// the first time editor launch
//
BOOLEAN                       EditorFirst;

//
// it's time editor should exit
//
BOOLEAN                       EditorExit;

BOOLEAN                       EditorMouseAction;

extern EFI_EDITOR_FILE_BUFFER FileBuffer;
extern EFI_EDITOR_TITLE_BAR   MainTitleBar;
extern EFI_EDITOR_STATUS_BAR  MainStatusBar;
extern EFI_EDITOR_INPUT_BAR   MainInputBar;
extern EFI_EDITOR_MENU_BAR    MainMenuBar;

extern BOOLEAN                FileBufferMouseNeedRefresh;
extern BOOLEAN                FileBufferNeedRefresh;
extern BOOLEAN                StatusBarNeedRefresh;

extern EFI_EDITOR_FILE_BUFFER FileBufferBackupVar;

EFI_EDITOR_GLOBAL_EDITOR      MainEditor;

//
// basic initialization for MainEditor
//
EFI_EDITOR_GLOBAL_EDITOR      MainEditorConst = {
  &MainTitleBar,
  &MainMenuBar,
  &MainStatusBar,
  &MainInputBar,
  &FileBuffer,
  {
    0,
    0
  },
  {
    0,
    0
  },
  NULL,
  FALSE,
  NULL
};

//
// Name:
//   MainEditorInit -- Init function for MainEditor
// In:
//   VOID
// Out:
//   EFI_SUCCESS
//   EFI_LOAD_ERROR
//
EFI_STATUS
MainEditorInit (
  VOID
  )
{
  EFI_STATUS  Status;
  EFI_HANDLE  *HandleBuffer;
  UINTN       HandleCount;

  //
  // basic initialization
  //
  CopyMem (&MainEditor, &MainEditorConst, sizeof (MainEditor));

  //
  // set screen attributes
  //
  MainEditor.ColorAttributes.Colors.Foreground  = Out->Mode->Attribute & 0x000000ff;

  MainEditor.ColorAttributes.Colors.Background  = (UINT8) (Out->Mode->Attribute >> 4);
  OriginalColors = MainEditor.ColorAttributes.Colors;

  OriginalMode = Out->Mode->Mode;

  //
  // query screen size
  //
  Out->QueryMode (
        Out,
        Out->Mode->Mode,
        &(MainEditor.ScreenSize.Column),
        &(MainEditor.ScreenSize.Row)
        );

  //
  // Find mouse
  //
  HandleBuffer = NULL;
  Status = LibLocateHandle (
            ByProtocol,
            &gEfiPrimaryConsoleInDeviceGuid,
            NULL,
            &HandleCount,
            &HandleBuffer
            );

  if (!EFI_ERROR (Status)) {
    if (HandleCount > 0) {
      Status = BS->HandleProtocol (
                    HandleBuffer[0],
                    &gEfiSimplePointerProtocolGuid,
                    &MainEditor.MouseInterface
                    );

      if (EFI_ERROR (Status)) {
        MainEditor.MouseInterface = NULL;
      } else {
        MainEditor.MouseAccumulatorX  = 0;
        MainEditor.MouseAccumulatorY  = 0;
        MainEditor.MouseSupported     = TRUE;
      }
    }
  }

  if (HandleBuffer) {
    FreePool (HandleBuffer);
  }
  //
  // below will call the five components' init function
  //
  Status = MainTitleBarInit ();
  if (EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_EDIT_LIBEDITOR_TITLEBAR), HiiHandle);
    return EFI_LOAD_ERROR;
  }

  Status = MainMenuBarInit ();
  if (EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_EDIT_LIBEDITOR_MAINMENU), HiiHandle);
    return EFI_LOAD_ERROR;
  }

  Status = MainStatusBarInit ();
  if (EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_EDIT_LIBEDITOR_STATUSBAR), HiiHandle);
    return EFI_LOAD_ERROR;
  }

  Status = MainInputBarInit ();
  if (EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_EDIT_LIBEDITOR_INPUTBAR), HiiHandle);
    return EFI_LOAD_ERROR;
  }

  Status = FileBufferInit ();
  if (EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_EDIT_LIBEDITOR_FILEBUFFER), HiiHandle);
    return EFI_LOAD_ERROR;
  }
  //
  // clear whole screen and enable cursor
  //
  Out->ClearScreen (Out);
  Out->EnableCursor (Out, TRUE);

  //
  // initialize EditorFirst and EditorExit
  //
  EditorFirst       = TRUE;
  EditorExit        = FALSE;
  EditorMouseAction = FALSE;

  return EFI_SUCCESS;
}
//
// Name:
//   MainEditorCleanup -- cleanup function for MainEditor
// In:
//   VOID
// Out:
//   EFI_SUCCESS
//   EFI_LOAD_ERROR
//
EFI_STATUS
MainEditorCleanup (
  VOID
  )
{
  EFI_STATUS  Status;

  //
  // call the five components' cleanup function
  // if error, do not exit
  // just print some warning
  //
  Status = MainTitleBarCleanup ();
  if (EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_EDIT_LIBEDITOR_TILEBAR_CLEANUP), HiiHandle);
  }

  Status = MainMenuBarCleanup ();
  if (EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_EDIT_LIBEDITOR_MENUBAR_CLEANUP), HiiHandle);
  }

  Status = MainStatusBarCleanup ();
  if (EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_EDIT_LIBEDITOR_STATUSBAR_CLEANUP), HiiHandle);
  }

  Status = MainInputBarCleanup ();
  if (EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_EDIT_LIBEDITOR_INPUTBAR_CLEANUP), HiiHandle);
  }

  Status = FileBufferCleanup ();
  if (EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_EDIT_LIBEDITOR_FILEBUFFER_CLEANUP), HiiHandle);
  }
  //
  // restore old mode
  //
  if (OriginalMode != Out->Mode->Mode) {
    Out->SetMode (Out, OriginalMode);
  }
  //
  // restore old screen color
  //
  Out->SetAttribute (
        Out,
        EFI_TEXT_ATTR (OriginalColors.Foreground, OriginalColors.Background)
        );

  Out->ClearScreen (Out);

  return EFI_SUCCESS;
}
//
// Name:
//   MainEditorRefresh -- Refresh function for MainEditor
// In:
//   VOID
// Out:
//   EFI_SUCCESS
//
EFI_STATUS
MainEditorRefresh (
  VOID
  )
{
  //
  // to aVOID screen flicker
  // The Stall Value is from experience. NOT from spec
  //
  BS->Stall (50);

  //
  // call the four components refresh function
  //
  MainTitleBarRefresh ();
  FileBufferRefresh ();
  MainStatusBarRefresh ();
  MainMenuBarRefresh ();

  //
  // EditorFirst is now set to FALSE
  //
  EditorFirst = FALSE;

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
MainEditorHandleMouseInput (
  IN EFI_SIMPLE_POINTER_STATE       MouseState
  )
{

  INT32           TextX;
  INT32           TextY;
  UINTN           FRow;
  UINTN           FCol;

  EFI_LIST_ENTRY  *Link;
  EFI_EDITOR_LINE *Line;

  UINTN           Index;
  BOOLEAN         Action;

  //
  // mouse action means:
  //    mouse movement
  //    mouse left button
  //
  Action = FALSE;

  //
  // have mouse movement
  //
  if (MouseState.RelativeMovementX || MouseState.RelativeMovementY) {
    //
    // handle
    //
    TextX = GetTextX (MouseState.RelativeMovementX);
    TextY = GetTextY (MouseState.RelativeMovementY);

    FileBufferAdjustMousePosition (TextX, TextY);

    Action = TRUE;

  }

  //
  // if left button pushed down
  //
  if (MouseState.LeftButton) {

    FCol = MainEditor.FileBuffer->MousePosition.Column - TEXT_START_COLUMN + 1;

    FRow = MainEditor.FileBuffer->FilePosition.Row +
      MainEditor.FileBuffer->MousePosition.Row -
      MainEditor.FileBuffer->DisplayPosition.Row;

    //
    // beyond the file line length
    //
    if (MainEditor.FileBuffer->NumLines < FRow) {
      FRow = MainEditor.FileBuffer->NumLines;
    }

    Link = MainEditor.FileBuffer->ListHead->Flink;
    for (Index = 0; Index < FRow - 1; Index++) {
      Link = Link->Flink;
    }

    Line = CR (Link, EFI_EDITOR_LINE, Link, EFI_EDITOR_LINE_LIST);

    //
    // beyond the line's column length
    //
    if (FCol > Line->Size + 1) {
      FCol = Line->Size + 1;
    }

    FileBufferMovePosition (FRow, FCol);

    MainEditor.FileBuffer->MousePosition.Row    = MainEditor.FileBuffer->DisplayPosition.Row;

    MainEditor.FileBuffer->MousePosition.Column = MainEditor.FileBuffer->DisplayPosition.Column;

    Action = TRUE;
  }
  //
  // mouse has action
  //
  if (Action) {
    return EFI_SUCCESS;
  }

  //
  // no mouse action
  //
  return EFI_NOT_FOUND;
}
//
// Name:
//   MainEditorKeyInput --
//     Handle user key input. will route it to other components
//     handle function
// In:
//   VOID
// Out:
//   EFI_SUCCESS
//   EFI_LOAD_ERROR
//   EFI_OUT_OF_RESOURCES
//
EFI_STATUS
MainEditorKeyInput (
  VOID
  )
{
  EFI_INPUT_KEY             Key;
  EFI_STATUS                Status;
  EFI_SIMPLE_POINTER_STATE  MouseState;

  do {

    Status            = EFI_SUCCESS;
    EditorMouseAction = FALSE;

    //
    // backup some key elements, so that can aVOID some refresh work
    //
    MainEditorBackup ();

    //
    // change priority of checking mouse/keyboard activity dynamically
    // so prevent starvation of keyboard.
    // if last time, mouse moves then this time check keyboard
    //
    if (MainEditor.MouseSupported) {
      Status = MainEditor.MouseInterface->GetState (
                                            MainEditor.MouseInterface,
                                            &MouseState
                                            );
      if (!EFI_ERROR (Status)) {

        Status = MainEditorHandleMouseInput (MouseState);

        if (!EFI_ERROR (Status)) {
          EditorMouseAction           = TRUE;
          FileBufferMouseNeedRefresh  = TRUE;
        } else if (Status == EFI_LOAD_ERROR) {
          MainStatusBarSetStatusString (L"Invalid Mouse Movement ");
        }
      }
    }

    Status = In->ReadKeyStroke (In, &Key);
    if (!EFI_ERROR (Status)) {
      //
      // dispatch to different components' key handling function
      // so not everywhere has to set this variable
      //
      FileBufferMouseNeedRefresh = TRUE;
      //
      // clear previous status string
      //
      StatusBarNeedRefresh = TRUE;

      //
      // dispatch to different components' key handling function
      //
      if (IS_VALID_CHAR (Key.ScanCode)) {
        Status = FileBufferHandleInput (&Key);
      } else if (IS_DIRECTION_KEY (Key.ScanCode)) {
        Status = FileBufferHandleInput (&Key);
      } else if (IS_FUNCTION_KEY (Key.ScanCode)) {
        Status = MainMenuBarHandleInput (&Key);
      } else {
        MainStatusBarSetStatusString (L"Unknown Command");
        FileBufferMouseNeedRefresh = FALSE;
      }

      if (Status != EFI_SUCCESS && Status != EFI_OUT_OF_RESOURCES) {
        //
        // not already has some error status
        //
        if (StrCmp (L"", MainEditor.StatusBar->StatusString) == 0) {
          MainStatusBarSetStatusString (L"Disk Error. Try Again");
        }
      }

    }
    //
    // after handling, refresh editor
    //
    MainEditorRefresh ();

  } while (Status != EFI_OUT_OF_RESOURCES && !EditorExit);

  return Status;
}
//
// Name:
//   MainEditorSetCutLine -- Set clipboard
// In:
//   Line -- Line to be set to clipboard
// Out:
//   EFI_SUCCESS
//   EFI_OUT_OF_RESOURCES
//
EFI_STATUS
MainEditorSetCutLine (
  EFI_EDITOR_LINE *Line
  )
{
  if (Line == NULL) {
    return EFI_SUCCESS;
  }

  if (MainEditor.CutLine != NULL) {
    //
    // free the old clipboard
    //
    LineFree (MainEditor.CutLine);
  }
  //
  // duplicate the line to clipboard
  //
  MainEditor.CutLine = LineDup (Line);
  if (MainEditor.CutLine == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  return EFI_SUCCESS;
}
//
// Name:
//   MainEditorBackup -- Backup function for MainEditor
// In:
//   VOID
// Out:
//   EFI_SUCCESS
//
EFI_STATUS
MainEditorBackup (
  VOID
  )
{
  //
  // call the four components' backup function
  //
  MainTitleBarBackup ();
  FileBufferBackup ();
  MainStatusBarBackup ();
  MainMenuBarBackup ();

  return EFI_SUCCESS;
}
