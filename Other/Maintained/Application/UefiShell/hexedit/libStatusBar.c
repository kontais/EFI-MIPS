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
    Definition for the Status Bar - the display for 
    the status of the editor

--*/

#include "heditor.h"

HEFI_EDITOR_STATUS_BAR          HMainStatusBar;
HEFI_EDITOR_STATUS_BAR          HMainStatusBarBackupVar;

BOOLEAN                         HStatusBarNeedRefresh;
BOOLEAN                         HStatusStringChanged;

extern HEFI_EDITOR_BUFFER_IMAGE HBufferImage;
extern HEFI_EDITOR_BUFFER_IMAGE HBufferImageBackupVar;

HEFI_EDITOR_STATUS_BAR          HMainStatusBarConst = { NULL };

EFI_STATUS
HMainStatusBarInit (
  VOID
  )
/*++

Routine Description: 

  Init function for HMainStatusBar

Arguments:  

  None

Returns:  

  EFI_SUCCESS

--*/
{

  CopyMem (&HMainStatusBar, &HMainStatusBarConst, sizeof (HMainStatusBar));

  HMainStatusBarSetStatusString (L"");

  HStatusBarNeedRefresh                 = TRUE;

  HMainStatusBarBackupVar.StatusString  = NULL;
  return EFI_SUCCESS;
}

EFI_STATUS
HMainStatusBarBackup (
  VOID
  )
/*++

Routine Description: 

  Backup function for HMainStatusBar

Arguments:  

  None

Returns:  

  EFI_SUCCESS

--*/
{
  HEditorFreePool (HMainStatusBarBackupVar.StatusString);
  HMainStatusBarBackupVar.StatusString = PoolPrint (
                                          L"%s",
                                          HMainStatusBar.StatusString
                                          );

  return EFI_SUCCESS;
}

EFI_STATUS
HMainStatusBarCleanup (
  VOID
  )
/*++

Routine Description: 

  Cleanup function for HMainStatusBar

Arguments:  

  None

Returns:  

  EFI_SUCCESS

--*/
{

  HEditorFreePool ((VOID *) HMainStatusBar.StatusString);
  HEditorFreePool ((VOID *) HMainStatusBarBackupVar.StatusString);

  return EFI_SUCCESS;
}

EFI_STATUS
HMainStatusBarRefresh (
  VOID
  )
/*++

Routine Description: 

  Refresh function for HMainStatusBar

Arguments:  

  None

Returns:  

  EFI_SUCCESS

--*/
{
  HEFI_EDITOR_COLOR_UNION Orig;
  HEFI_EDITOR_COLOR_UNION New;
  UINTN                   Offset;

  //
  // if the old status string is the same with current status string,
  // so clear it
  //
  if (!HStatusStringChanged && HStatusBarNeedRefresh && (StrCmp (HMainStatusBarBackupVar.StatusString, L"\0") != 0)) {
    HMainStatusBarSetStatusString (L"\0");
  }
  //
  // when it's called first time after editor launch, so refresh is mandatory
  //
  if (!HEditorFirst && !HStatusBarNeedRefresh) {
    //
    // all elements has been unchanged
    //
    if (!HStatusBarNeedRefresh &&
        !HStatusStringChanged &&
        HBufferImageBackupVar.BufferPosition.Row == HBufferImage.BufferPosition.Row &&
        HBufferImageBackupVar.BufferPosition.Column == HBufferImage.BufferPosition.Column &&
        StrCmp (
          HMainStatusBarBackupVar.StatusString,
        HMainStatusBarBackupVar.StatusString
        ) == 0
        ) {
      return EFI_SUCCESS;
    }

  }
  //
  // back up the screen attributes
  //
  Orig                  = HMainEditor.ColorAttributes;
  New.Colors.Foreground = Orig.Colors.Background;
  New.Colors.Background = Orig.Colors.Foreground;

  Out->SetAttribute (Out, New.Data);

  //
  // clear status bar
  //
  HEditorClearLine (STATUS_BAR_LOCATION);

  //
  // print offset fields. Offset starting from 0
  //
  Offset = (HMainEditor.BufferImage->BufferPosition.Row - 1) * 0x10 + (HMainEditor.BufferImage->BufferPosition.Column - 1);
  PrintAt (
    0,
    STATUS_BAR_LOCATION - 1,
    L"  Offset: %8X        %s",
    Offset,
    HMainStatusBar.StatusString
    );

  //
  // restore the old screen attributes
  //
  Out->SetAttribute (Out, Orig.Data);

  //
  // restore position in edit area
  //
  HBufferImageRestorePosition ();

  HStatusBarNeedRefresh = FALSE;
  HStatusStringChanged  = FALSE;

  return EFI_SUCCESS;
}

EFI_STATUS
HMainStatusBarSetStatusString (
  IN CHAR16 *Str
  )
/*++

Routine Description: 

  Set the StatusString field for HMainStatusBar

Arguments:  

  Str -- status string to set

Returns:  

  EFI_SUCCESS
  EFI_OUT_OF_RESOURCES

--*/
{
  HStatusStringChanged = TRUE;

  //
  // free the old status string
  //
  HEditorFreePool (HMainStatusBar.StatusString);

  HMainStatusBar.StatusString = StrDuplicate (Str);
  if (HMainStatusBar.StatusString == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  return EFI_SUCCESS;
}
