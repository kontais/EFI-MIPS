/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

  Module Name:
    libInputBar.c

  Abstract:
    Definition of the user input bar ( the same position with the Status Bar)

--*/

#include "heditor.h"

HEFI_EDITOR_INPUT_BAR HMainInputBar;

//
// basic initialization for HMainInputBar
//
HEFI_EDITOR_INPUT_BAR HMainInputBarConst = {
  NULL,
  NULL,
  0
};

extern BOOLEAN        HStatusBarNeedRefresh;

EFI_STATUS
HMainInputBarInit (
  VOID
  )
/*++

Routine Description: 

  Initialize function for HMainInputBar

Arguments:  

  None

Returns:  

  EFI_SUCCESS

--*/
{
  CopyMem (&HMainInputBar, &HMainInputBarConst, sizeof (HMainInputBar));

  return EFI_SUCCESS;
}

EFI_STATUS
HMainInputBarCleanup (
  VOID
  )
/*++

Routine Description: 

  cleanup function for HMainInputBar

Arguments:  

  None

Returns:  

  EFI_SUCCESS

--*/
{

  HEditorFreePool ((VOID *) HMainInputBar.Prompt);

  HEditorFreePool ((VOID *) HMainInputBar.ReturnString);

  return EFI_SUCCESS;
}

VOID
HMainInputBarPrintInput (
  VOID
  )
/*++

Routine Description: 

  Print user input in Input Bar

Arguments:  

  None

Returns:  

  None

--*/
{
  UINTN   Limit;
  UINTN   Size;
  CHAR16  *Buffer;
  UINTN   Index;
  UINTN   PromptLen;

  PromptLen = StrLen (HMainInputBar.Prompt);
  Limit     = HMainEditor.ScreenSize.Column - PromptLen - 1;
  Size      = StrLen (HMainInputBar.ReturnString);

  //
  // if prompt + input string is larger than limit
  //  then cut it to limit
  //
  if (Size <= Limit) {
    Buffer = HMainInputBar.ReturnString;
  } else {
    Buffer = HMainInputBar.ReturnString + Size - Limit;
  }

  Out->EnableCursor (Out, FALSE);

  PrintAt (PromptLen, INPUT_BAR_LOCATION - 1, L"%s", Buffer);
  Size = StrLen (Buffer);

  //
  // after prompt, print " "
  //
  for (Index = Size; Index < Limit; Index++) {
    PrintAt (PromptLen + Size, INPUT_BAR_LOCATION - 1, L" ");
  }

  Out->EnableCursor (Out, TRUE);
  Out->SetCursorPosition (Out, Size + PromptLen, INPUT_BAR_LOCATION - 1);
}

EFI_STATUS
HMainInputBarRefresh (
  VOID
  )
/*++

Routine Description: 

   refresh function for HMainInputBar;
   it will wait for user input

Arguments:  

  None

Returns:  

  EFI_SUCCESS
  EFI_OUT_OF_RESOURCES

--*/
{
  HEFI_EDITOR_COLOR_UNION Orig;
  HEFI_EDITOR_COLOR_UNION New;
  EFI_INPUT_KEY           Key;
  UINTN                   Size;
  EFI_STATUS              Status;
  BOOLEAN                 NoDisplay;
  UINTN                   Limit;
  UINTN                   PromptLen;

  //
  // variable initialization
  //
  Size    = 0;
  Status  = EFI_SUCCESS;

  //
  // free status string
  //
  HEditorFreePool (HMainEditor.StatusBar->StatusString);
  HMainEditor.StatusBar->StatusString = PoolPrint (L"");
  if (HMainEditor.StatusBar->StatusString == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  //
  // back up the old screen attributes
  //
  Orig                  = HMainEditor.ColorAttributes;
  New.Colors.Foreground = Orig.Colors.Background;
  New.Colors.Background = Orig.Colors.Foreground;

  Out->SetAttribute (Out, New.Data);

  //
  // clear input bar
  //
  HEditorClearLine (INPUT_BAR_LOCATION);

  Out->SetCursorPosition (Out, 0, INPUT_BAR_LOCATION - 1);
  PrintToken (STRING_TOKEN (STR_HEXEDIT_LIBINPUTBAR_MAININPUTBAR), HiiHandle, HMainInputBar.Prompt);

  //
  // that's the maximum input length that can be displayed on screen
  //
  PromptLen = StrLen (HMainInputBar.Prompt);
  Limit     = HMainEditor.ScreenSize.Column - PromptLen;

  //
  // this is a selection prompt, cursor will stay in edit area
  // actually this is for search , search/replace
  //
  if (HStrStr (HMainInputBar.Prompt, L"Yes/No")) {
    NoDisplay = TRUE;
    HBufferImageRestorePosition ();
  } else {
    NoDisplay = FALSE;
  }
  //
  // wait for user input
  //
  for (;;) {
    WaitForSingleEvent (In->WaitForKey, 0);
    Status = In->ReadKeyStroke (In, &Key);
    if (EFI_ERROR (Status)) {
      continue;
    }
    //
    // pressed ESC
    //
    if (Key.ScanCode == SCAN_CODE_ESC) {
      Size    = 0;
      Status  = EFI_NOT_READY;
      break;
    }
    //
    // return pressed
    //
    if (Key.UnicodeChar == CHAR_LF || Key.UnicodeChar == CHAR_CR) {
      break;
    } else if (Key.UnicodeChar == CHAR_BS) {
      //
      // backspace
      //
      if (Size > 0) {
        Size--;
        HMainInputBar.ReturnString[Size] = L'\0';
        if (!NoDisplay) {
          HMainInputBarPrintInput ();
        }
      }
    } else if (Key.UnicodeChar <= 127 && Key.UnicodeChar >= 32) {
      //
      // VALID ASCII char pressed
      //
      HMainInputBar.ReturnString[Size] = Key.UnicodeChar;

      //
      // should be less than specified length
      //
      if (Size >= HMainInputBar.StringSize) {
        continue;
      }

      Size++;

      HMainInputBar.ReturnString[Size] = L'\0';

      if (!NoDisplay) {

        HMainInputBarPrintInput ();
      } else {
        //
        // if just choose yes/no
        //
        break;
      }

    }
  }

  HMainInputBar.ReturnString[Size] = 0;

  HBufferImageRestorePosition ();

  //
  // restore screen attributes
  //
  Out->SetAttribute (Out, Orig.Data);

  HStatusBarNeedRefresh = TRUE;

  return Status;
}

EFI_STATUS
HMainInputBarHide (
  VOID
  )
/*++

Routine Description: 

  Clear input bar and restore file buffer cursor position

Arguments:  

  None

Returns:  

  EFI_SUCCESS

--*/
{
  HEditorClearLine (INPUT_BAR_LOCATION);
  HBufferImageRestorePosition ();

  return EFI_SUCCESS;
}

EFI_STATUS
HMainInputBarSetPrompt (
  IN  CHAR16 *Str
  )
/*++

Routine Description: 

  Set Prompt field for HMainInputBar;
  it will wait for user input

Arguments:  

  Str -- Prompt string to set  

Returns:  

  EFI_SUCCESS
  EFI_OUT_OF_RESOURCES

--*/
{
  //
  // FREE the old prompt string
  //
  HEditorFreePool (HMainInputBar.Prompt);

  HMainInputBar.Prompt = PoolPrint (L"%s ", Str);
  if (HMainInputBar.Prompt == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
HMainInputBarSetStringSize (
  IN UINTN   Size
  )
/*++

Routine Description: 

  Set StringSize field for HMainInputBar and allocate space for it

Arguments:  

  Size -- string size ( it's unit is Unicode character)
  
Returns:  

  EFI_SUCCESS
  EFI_OUT_OF_RESOURCES 

--*/
{
  //
  // free the old ReturnStirng
  //
  HEditorFreePool (HMainInputBar.ReturnString);

  HMainInputBar.StringSize    = Size;

  HMainInputBar.ReturnString  = AllocatePool (2 * (Size + 1));
  if (HMainInputBar.ReturnString == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  return EFI_SUCCESS;
}
