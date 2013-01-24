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

#include "heditor.h"

HEFI_EDITOR_COLOR_ATTRIBUTES    HOriginalColors;
INTN                            HOriginalMode;

//
// the first time editor launch
//
BOOLEAN                         HEditorFirst;

//
// it's time editor should exit
//
BOOLEAN                         HEditorExit;

BOOLEAN                         HEditorMouseAction;

extern HEFI_EDITOR_BUFFER_IMAGE HBufferImage;
extern HEFI_EDITOR_BUFFER_IMAGE HBufferImageBackupVar;

extern HEFI_EDITOR_TITLE_BAR    HMainTitleBar;
extern HEFI_EDITOR_STATUS_BAR   HMainStatusBar;
extern HEFI_EDITOR_INPUT_BAR    HMainInputBar;
extern HEFI_EDITOR_MENU_BAR     HMainMenuBar;
extern HEFI_EDITOR_CLIPBOARD    HClipBoard;

extern BOOLEAN                  HBufferImageMouseNeedRefresh;
extern BOOLEAN                  HBufferImageNeedRefresh;
extern BOOLEAN                  HBufferImageOnlyLineNeedRefresh;
extern BOOLEAN                  HStatusBarNeedRefresh;

HEFI_EDITOR_GLOBAL_EDITOR       HMainEditor;
HEFI_EDITOR_GLOBAL_EDITOR       HMainEditorBackupVar;

//
// basic initialization for MainEditor
//
HEFI_EDITOR_GLOBAL_EDITOR       HMainEditorConst = {
  &HMainTitleBar,
  &HMainMenuBar,
  &HMainStatusBar,
  &HMainInputBar,
  &HBufferImage,
  &HClipBoard,
  {
    0,
    0
  },
  {
    0,
    0
  },
  FALSE,
  NULL,
  0,
  0
};

EFI_STATUS
HMainEditorInit (
  VOID
  )
/*++

Routine Description: 

  Init function for MainEditor

Arguments:  

  None

Returns:  

  EFI_SUCCESS
  EFI_LOAD_ERROR

--*/
{
  EFI_STATUS  Status;
  EFI_HANDLE  *HandleBuffer;
  UINTN       HandleCount;

  //
  // basic initialization
  //
  CopyMem (&HMainEditor, &HMainEditorConst, sizeof (HMainEditor));

  //
  // set screen attributes
  //
  HMainEditor.ColorAttributes.Colors.Foreground = Out->Mode->Attribute & 0x000000ff;

  HMainEditor.ColorAttributes.Colors.Background = (UINT8) (Out->Mode->Attribute >> 4);

  HOriginalColors = HMainEditor.ColorAttributes.Colors;

  HOriginalMode = Out->Mode->Mode;

  //
  // query screen size
  //
  Out->QueryMode (
        Out,
        Out->Mode->Mode,
        &(HMainEditor.ScreenSize.Column),
        &(HMainEditor.ScreenSize.Row)
        );

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
                    &HMainEditor.MouseInterface
                    );
      if (EFI_ERROR (Status)) {
        HMainEditor.MouseInterface = NULL;
      } else {
        HMainEditor.MouseAccumulatorX = 0;
        HMainEditor.MouseAccumulatorY = 0;
        HMainEditor.MouseSupported    = TRUE;
      }
    }
  }

  if (HandleBuffer != NULL) {
    FreePool (HandleBuffer);
  }
  //
  // below will call the five components' init function
  //
  Status = HMainTitleBarInit ();
  if (EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_HEXEDIT_LIBEDITOR_MAINEDITOR_TITLE), HiiHandle);
    return EFI_LOAD_ERROR;
  }

  Status = HMainMenuBarInit ();
  if (EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_HEXEDIT_LIBEDITOR_MAINEDITOR_MAINMENU), HiiHandle);
    return EFI_LOAD_ERROR;
  }

  Status = HMainStatusBarInit ();
  if (EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_HEXEDIT_LIBEDITOR_MAINEDITOR_STATUS), HiiHandle);
    return EFI_LOAD_ERROR;
  }

  Status = HMainInputBarInit ();
  if (EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_HEXEDIT_LIBEDITOR_MAINEDITOR_INPUTBAR), HiiHandle);
    return EFI_LOAD_ERROR;
  }

  Status = HBufferImageInit ();
  if (EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_HEXEDIT_LIBEDITOR_MAINEDITOR_BUFFERIMAGE), HiiHandle);
    return EFI_LOAD_ERROR;
  }

  Status = HClipBoardInit ();
  if (EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_HEXEDIT_LIBEDITOR_MAINEDITOR_CLIPBOARD), HiiHandle);
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
  HEditorFirst        = TRUE;
  HEditorExit         = FALSE;
  HEditorMouseAction  = FALSE;

  return EFI_SUCCESS;
}

EFI_STATUS
HMainEditorCleanup (
  VOID
  )
/*++

Routine Description: 

  cleanup function for MainEditor

Arguments:  

  None

Returns:  

  EFI_SUCCESS
  EFI_LOAD_ERROR

--*/
{
  EFI_STATUS  Status;

  //
  // call the five components' cleanup function
  //
  Status = HMainTitleBarCleanup ();
  if (EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_HEXEDIT_LIBEDITOR_TITLEBAR_CLEAN), HiiHandle);
  }

  Status = HMainMenuBarCleanup ();
  if (EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_HEXEDIT_LIBEDITOR_MENUBAR_CLEAN), HiiHandle);
  }

  Status = HMainStatusBarCleanup ();
  if (EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_HEXEDIT_LIBEDITOR_STATUSBAR_CLEAN), HiiHandle);
  }

  Status = HMainInputBarCleanup ();
  if (EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_HEXEDIT_LIBEDITOR_INPUTBAR_CLEAN), HiiHandle);
  }

  Status = HBufferImageCleanup ();
  if (EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_HEXEDIT_LIBEDITOR_BUFFERIMAGE_CLEAN), HiiHandle);
  }

  Status = HClipBoardCleanup ();
  if (EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_HEXEDIT_LIBEDITOR_CLIPBOARD_CLEAN), HiiHandle);
  }
  //
  // restore old mode
  //
  if (HOriginalMode != Out->Mode->Mode) {
    Out->SetMode (Out, HOriginalMode);
  }

  Out->SetAttribute (
        Out,
        EFI_TEXT_ATTR (HOriginalColors.Foreground, HOriginalColors.Background)
        );
  Out->ClearScreen (Out);

  return EFI_SUCCESS;
}

EFI_STATUS
HMainEditorRefresh (
  VOID
  )
/*++

Routine Description: 

  Refresh function for MainEditor

Arguments:  

  None

Returns:  

  EFI_SUCCESS

--*/
{
  //
  // to aVOID screen flicker
  // the stall value is from experience
  //
  BS->Stall (50);

  //
  // call the four components refresh function
  //
  HMainTitleBarRefresh ();
  HBufferImageRefresh ();
  HMainStatusBarRefresh ();
  HMainMenuBarRefresh ();

  //
  // EditorFirst is now set to FALSE
  //
  HEditorFirst = FALSE;

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
HMainEditorHandleMouseInput (
  IN  EFI_SIMPLE_POINTER_STATE       MouseState,
  OUT BOOLEAN                        *BeforeLeftButtonDown
  )
{

  INT32             TextX;
  INT32             TextY;
  UINTN             FRow;
  UINTN             FCol;
  BOOLEAN           HighBits;
  EFI_LIST_ENTRY    *Link;
  HEFI_EDITOR_LINE  *Line;
  UINTN             Index;
  BOOLEAN           Action;

  Action = FALSE;

  //
  // have mouse movement
  //
  if (MouseState.RelativeMovementX || MouseState.RelativeMovementY) {
    //
    // handle
    //
    TextX = HGetTextX (MouseState.RelativeMovementX);
    TextY = HGetTextY (MouseState.RelativeMovementY);

    HBufferImageAdjustMousePosition (TextX, TextY);

    Action = TRUE;

  }

  if (MouseState.LeftButton) {
    HighBits = HBufferImageIsAtHighBits (
                HMainEditor.BufferImage->MousePosition.Column,
                &FCol
                );

    //
    // not at an movable place
    //
    if (FCol == 0) {
      //
      // now just move mouse pointer to legal position
      //
      //
      // move mouse position to legal position
      //
      HMainEditor.BufferImage->MousePosition.Column -= HEX_POSITION;
      if (HMainEditor.BufferImage->MousePosition.Column > 24) {
        HMainEditor.BufferImage->MousePosition.Column--;
        FCol = HMainEditor.BufferImage->MousePosition.Column / 3 + 1 + 1;
      } else {
        if (HMainEditor.BufferImage->MousePosition.Column < 24) {
          FCol = HMainEditor.BufferImage->MousePosition.Column / 3 + 1 + 1;
        } else {
          //
          // == 24
          //
          FCol = 9;
        }
      }

      HighBits = TRUE;

    }

    FRow = HMainEditor.BufferImage->BufferPosition.Row +
      HMainEditor.BufferImage->MousePosition.Row -
      HMainEditor.BufferImage->DisplayPosition.Row;

    if (HMainEditor.BufferImage->NumLines < FRow) {
      //
      // dragging
      //
      //
      // now just move mouse pointer to legal position
      //
      FRow      = HMainEditor.BufferImage->NumLines;
      HighBits  = TRUE;
    }

    Link = HMainEditor.BufferImage->ListHead->Flink;
    for (Index = 0; Index < FRow - 1; Index++) {
      Link = Link->Flink;
    }

    Line = CR (Link, HEFI_EDITOR_LINE, Link, EFI_EDITOR_LINE_LIST);

    //
    // dragging
    //
    //
    // now just move mouse pointer to legal position
    //
    if (FCol > Line->Size) {
      if (*BeforeLeftButtonDown) {
        HighBits = FALSE;

        if (Line->Size == 0) {
          if (FRow > 1) {
            FRow--;
            FCol = 16;
          } else {
            FRow  = 1;
            FCol  = 1;
          }

        } else {
          FCol = Line->Size;
        }
      } else {
        FCol      = Line->Size + 1;
        HighBits  = TRUE;
      }
    }

    HBufferImageMovePosition (FRow, FCol, HighBits);

    HMainEditor.BufferImage->MousePosition.Row    = HMainEditor.BufferImage->DisplayPosition.Row;

    HMainEditor.BufferImage->MousePosition.Column = HMainEditor.BufferImage->DisplayPosition.Column;

    *BeforeLeftButtonDown                         = TRUE;

    Action = TRUE;
  } else {
    //
    // else of if LButton
    //
    // release LButton
    //
    if (*BeforeLeftButtonDown == TRUE) {
      Action = TRUE;
    }
    //
    // mouse up
    //
    *BeforeLeftButtonDown = FALSE;
  }

  if (Action) {
    return EFI_SUCCESS;
  }

  return EFI_NOT_FOUND;
}

EFI_STATUS
HMainEditorKeyInput (
  VOID
  )
/*++

Routine Description: 

  Handle user key input. will route it to other components handle function

Arguments:  

  None

Returns:  

  EFI_SUCCESS
  EFI_LOAD_ERROR
  EFI_OUT_OF_RESOURCES

--*/
{
  EFI_INPUT_KEY             Key;
  EFI_STATUS                Status;
  EFI_SIMPLE_POINTER_STATE  MouseState;
  BOOLEAN                   LengthChange;
  UINTN                     Size;
  UINTN                     OldSize;
  BOOLEAN                   BeforeMouseIsDown;
  BOOLEAN                   MouseIsDown;
  BOOLEAN                   FirstDown;
  BOOLEAN                   MouseDrag;
  UINTN                     FRow;
  UINTN                     FCol;
  UINTN                     SelectStartBackup;
  UINTN                     SelectEndBackup;

  //
  // variable initialization
  //
  OldSize       = 0;
  FRow          = 0;
  FCol          = 0;
  LengthChange  = FALSE;

  MouseIsDown   = FALSE;
  FirstDown     = FALSE;
  MouseDrag     = FALSE;

  do {

    Status              = EFI_SUCCESS;

    HEditorMouseAction  = FALSE;

    //
    // backup some key elements, so that can aVOID some refresh work
    //
    HMainEditorBackup ();

    //
    // wait for user key input
    //
    //
    // change priority of checking mouse/keyboard activity dynamically
    // so prevent starvation of keyboard.
    // if last time, mouse moves then this time check keyboard
    //
    if (HMainEditor.MouseSupported) {
      Status = HMainEditor.MouseInterface->GetState (
                                            HMainEditor.MouseInterface,
                                            &MouseState
                                            );
      if (!EFI_ERROR (Status)) {

        BeforeMouseIsDown = MouseIsDown;

        Status            = HMainEditorHandleMouseInput (MouseState, &MouseIsDown);

        if (!EFI_ERROR (Status)) {
          if (BeforeMouseIsDown == FALSE) {
            //
            // mouse down
            //
            if (MouseIsDown == TRUE) {
              FRow              = HBufferImage.BufferPosition.Row;
              FCol              = HBufferImage.BufferPosition.Column;
              SelectStartBackup = HMainEditor.SelectStart;
              SelectEndBackup   = HMainEditor.SelectEnd;

              FirstDown         = TRUE;
            }
          } else {

            SelectStartBackup = HMainEditor.SelectStart;
            SelectEndBackup   = HMainEditor.SelectEnd;

            //
            // begin to drag
            //
            if (MouseIsDown == TRUE) {
              if (FirstDown == TRUE) {
                if (MouseState.RelativeMovementX || MouseState.RelativeMovementY) {
                  HMainEditor.SelectStart = 0;
                  HMainEditor.SelectEnd   = 0;
                  HMainEditor.SelectStart = (FRow - 1) * 0x10 + FCol;

                  MouseDrag               = TRUE;
                  FirstDown               = FALSE;
                }
              } else {
                if ((
                      (HBufferImage.BufferPosition.Row - 1) *
                    0x10 +
                    HBufferImage.BufferPosition.Column
                      ) >= HMainEditor.SelectStart
                        ) {
                  HMainEditor.SelectEnd = (HBufferImage.BufferPosition.Row - 1) *
                    0x10 +
                    HBufferImage.BufferPosition.Column;
                } else {
                  HMainEditor.SelectEnd = 0;
                }
              }
              //
              // end of if RelativeX/Y
              //
            } else {
              //
              // mouse is up
              //
              if (MouseDrag) {
                if (HBufferImageGetTotalSize () == 0) {
                  HMainEditor.SelectStart = 0;
                  HMainEditor.SelectEnd   = 0;
                  FirstDown               = FALSE;
                  MouseDrag               = FALSE;
                }

                if ((
                      (HBufferImage.BufferPosition.Row - 1) *
                    0x10 +
                    HBufferImage.BufferPosition.Column
                      ) >= HMainEditor.SelectStart
                        ) {
                  HMainEditor.SelectEnd = (HBufferImage.BufferPosition.Row - 1) *
                    0x10 +
                    HBufferImage.BufferPosition.Column;
                } else {
                  HMainEditor.SelectEnd = 0;
                }

                if (HMainEditor.SelectEnd == 0) {
                  HMainEditor.SelectStart = 0;
                }
              }

              FirstDown = FALSE;
              MouseDrag = FALSE;
            }

            if (SelectStartBackup != HMainEditor.SelectStart || SelectEndBackup != HMainEditor.SelectEnd) {
              if ((SelectStartBackup - 1) / 0x10 != (HMainEditor.SelectStart - 1) / 0x10) {
                HBufferImageNeedRefresh = TRUE;
              } else {
                if ((SelectEndBackup - 1) / 0x10 != (HMainEditor.SelectEnd - 1) / 0x10) {
                  HBufferImageNeedRefresh = TRUE;
                } else {
                  HBufferImageOnlyLineNeedRefresh = TRUE;
                }
              }
            }
          }

          HEditorMouseAction            = TRUE;
          HBufferImageMouseNeedRefresh  = TRUE;

        } else if (Status == EFI_LOAD_ERROR) {
          HMainStatusBarSetStatusString (L"Invalid Mouse Movement ");
        }
      }
    }

    Status = In->ReadKeyStroke (In, &Key);
    if (!EFI_ERROR (Status)) {
      //
      // dispatch to different components' key handling function
      // so not everywhere has to set this variable
      //
      HBufferImageMouseNeedRefresh = TRUE;
      //
      // clear previous status string
      //
      HStatusBarNeedRefresh = TRUE;

      if (IS_VALID_CHAR (Key.ScanCode)) {
        Status = HBufferImageHandleInput (&Key);
      } else if (IS_DIRECTION_KEY (Key.ScanCode)) {
        Status = HBufferImageHandleInput (&Key);
      } else if (IS_FUNCTION_KEY (Key.ScanCode)) {
        Status = HMainMenuBarHandleInput (&Key);
      } else {
        HMainStatusBarSetStatusString (L"Unknown Command");

        HBufferImageMouseNeedRefresh = FALSE;
      }

      if (Status != EFI_SUCCESS && Status != EFI_OUT_OF_RESOURCES) {
        //
        // not already has some error status
        //
        if (StrCmp (L"", HMainEditor.StatusBar->StatusString) == 0) {
          HMainStatusBarSetStatusString (L"Disk Error. Try Again");
        }
      }
    }
    //
    // decide if has to set length warning
    //
    if (HBufferImage.BufferType != HBufferImageBackupVar.BufferType) {
      LengthChange = FALSE;
    } else {
      //
      // still the old buffer
      //
      if (HBufferImage.BufferType != FILE_BUFFER) {
        Size = HBufferImageGetTotalSize ();

        switch (HBufferImage.BufferType) {
        case DISK_BUFFER:
          OldSize = HBufferImage.DiskImage->Size * HBufferImage.DiskImage->BlockSize;
          break;

        case MEM_BUFFER:
          OldSize = HBufferImage.MemImage->Size;
          break;
        }

        if (LengthChange == FALSE) {
          if (OldSize != Size) {
            HMainStatusBarSetStatusString (L"Disk/Mem Buffer Length should not be changed");
          }
        }

        if (OldSize != Size) {
          LengthChange = TRUE;
        } else {
          LengthChange = FALSE;
        }
      }
    }
    //
    // after handling, refresh editor
    //
    HMainEditorRefresh ();

  } while (Status != EFI_OUT_OF_RESOURCES && !HEditorExit);

  return Status;
}

EFI_STATUS
HMainEditorBackup (
  VOID
  )
/*++

Routine Description: 

  Backup function for MainEditor

Arguments:  

  None

Returns:  

  EFI_SUCCESS

--*/
{

  HMainEditorBackupVar.SelectStart  = HMainEditor.SelectStart;
  HMainEditorBackupVar.SelectEnd    = HMainEditor.SelectEnd;

  //
  // call the four components' backup function
  //
  HMainTitleBarBackup ();
  HBufferImageBackup ();
  HMainStatusBarBackup ();
  HMainMenuBarBackup ();

  return EFI_SUCCESS;
}
