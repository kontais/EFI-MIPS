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
  Deal with Title Bar

--*/

#include "heditor.h"

HEFI_EDITOR_TITLE_BAR           HMainTitleBar;

HEFI_EDITOR_TITLE_BAR           HMainTitleBarConst = { NULL };

extern HEFI_EDITOR_BUFFER_IMAGE HBufferImageBackupVar;
extern HEFI_EDITOR_BUFFER_IMAGE HBufferImage;

EFI_STATUS
HMainTitleBarInit (
  VOID
  )
/*++

Routine Description: 

  Init function for MainTitleBar

Arguments:  

  None

Returns:  

  EFI_SUCCESS
  EFI_OUT_OF_RESOURCES

--*/
{
  //
  // basic initialization for MainTitleBar
  //
  CopyMem (&HMainTitleBar, &HMainTitleBarConst, sizeof (HMainTitleBar));

  //
  // set TitlePrefix
  //
  HMainTitleBar.TitlePrefix = PoolPrint (
                                L"  %s  %s",
                                EDITOR_NAME,
                                EDITOR_VERSION
                                );
  if (HMainTitleBar.TitlePrefix == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
HMainTitleBarCleanup (
  VOID
  )
/*++

Routine Description: 

  cleanup function for MainTitleBar

Arguments:  

  None

Returns:  

  EFI_SUCCESS

--*/
{
  //
  // free tileprefix
  //
  HEditorFreePool (HMainTitleBar.TitlePrefix);

  return EFI_SUCCESS;
}

EFI_STATUS
HMainTitleBarRefresh (
  VOID
  )
/*++

Routine Description: 

  Refresh function for MainTitleBar

Arguments:  

  None

Returns:  

  EFI_SUCCESS

--*/
{
  HEFI_EDITOR_COLOR_UNION Orig;
  HEFI_EDITOR_COLOR_UNION New;
  INTN                    Index;
  CHAR16                  *FileNameTmp;

  //
  // if it's first time when editor launch, so refresh is mandatory
  //
  if (!HEditorFirst) {
    //
    // if all the fields are unchanged, no need to refresh
    //
    if (HBufferImageBackupVar.BufferType == HBufferImage.BufferType &&
        HBufferImageBackupVar.Modified == HBufferImage.Modified
        ) {
      //
      // FOR FILE IMAGE, only file name is related
      //
      if (HBufferImageBackupVar.BufferType == FILE_BUFFER && StrCmp (
                                                              HBufferImageBackupVar.FileImage->FileName,
                                                              HBufferImage.FileImage->FileName
                                                              ) == 0) {
        return EFI_SUCCESS;
      }

      //
      // FOR DISK IMAGE, only offset & size is related
      //
      if (HBufferImageBackupVar.BufferType == DISK_BUFFER &&
          HBufferImageBackupVar.DiskImage->Offset == HBufferImage.DiskImage->Offset &&
          HBufferImageBackupVar.DiskImage->Size == HBufferImage.DiskImage->Size
          ) {
        return EFI_SUCCESS;
      }

      //
      // FOR MEMORY IMAGE, only offset & size is related
      //
      if (HBufferImageBackupVar.BufferType == MEM_BUFFER &&
          HBufferImageBackupVar.MemImage->Offset == HBufferImage.MemImage->Offset &&
          HBufferImageBackupVar.MemImage->Size == HBufferImage.MemImage->Size
          ) {
        return EFI_SUCCESS;
      }

    }
  }
  //
  // backup the old screen attributes
  //
  Orig                  = HMainEditor.ColorAttributes;
  New.Colors.Foreground = Orig.Colors.Background;
  New.Colors.Background = Orig.Colors.Foreground;

  Out->SetAttribute (Out, New.Data);

  //
  // clear the old title bar
  //
  HEditorClearLine (TITLE_BAR_LOCATION);

  //
  // print the new title bar prefix
  // totally 25 chars
  //
  PrintAt (
    TEXT_START_COLUMN - 1,
    TITLE_BAR_LOCATION - 1,
    L"%s   ",
    HMainTitleBar.TitlePrefix
    );

  if (HMainEditor.BufferImage->BufferType == NO_BUFFER) {
    Out->SetAttribute (Out, Orig.Data);
    HBufferImageRestorePosition ();
    return EFI_SUCCESS;
  }

  switch (HMainEditor.BufferImage->BufferType) {
  case FILE_BUFFER:
    //
    // First Extract the FileName from fullpath
    //
    FileNameTmp = HMainEditor.BufferImage->FileImage->FileName;
    for (Index = StrLen (FileNameTmp) - 1; Index >= 0; Index--) {
      if (FileNameTmp[Index] == L'\\') {
        break;
      }
    }

    Index++;

    FileNameTmp = FileNameTmp + Index;
    //
    // the space for file name is 35 characters
    //
    if (StrLen (FileNameTmp) <= 35) {
      PrintToken (STRING_TOKEN (STR_HEXEDIT_LIBTITLEBAR_MAINEDITOR), HiiHandle, FileNameTmp);

      for (Index = StrLen (FileNameTmp); Index < 35; Index++) {
        Print (L" ");
      }

    } else {
      for (Index = 0; Index < 32; Index++) {
        PrintToken (STRING_TOKEN (STR_HEXEDIT_LIBTITLEBAR_MAINEDITOR_C), HiiHandle, FileNameTmp[Index]);
      }
      //
      // print "..."
      //
      Print (L"...");
    }

    break;

  case DISK_BUFFER:
    if (StrLen (HMainEditor.BufferImage->DiskImage->Name) <= 9) {
      PrintToken (
        STRING_TOKEN (STR_HEXEDIT_LIBTITLEBAR_MAINEDITOR),
        HiiHandle,
        HMainEditor.BufferImage->DiskImage->Name
        );
    } else {
      for (Index = 0; Index < 6; Index++) {
        PrintToken (
          STRING_TOKEN (STR_HEXEDIT_LIBTITLEBAR_MAINEDITOR_C),
          HiiHandle,
          HMainEditor.BufferImage->DiskImage->Name[Index]
          );
      }
      //
      // print "..."
      //
      Print (L"...");
    }

    PrintToken (
      STRING_TOKEN (STR_HEXEDIT_LIBTITLEBAR_MAINEDITOR_TWOVARS),
      HiiHandle,
      HMainEditor.BufferImage->DiskImage->Offset,
      HMainEditor.BufferImage->DiskImage->Offset + HMainEditor.BufferImage->DiskImage->Size - 1
      );

    if (StrLen (HMainEditor.BufferImage->DiskImage->Name) < 9) {
      for (Index = StrLen (HMainEditor.BufferImage->DiskImage->Name); Index < 9; Index++) {
        Print (L" ");
      }
    }

    break;

  case MEM_BUFFER:
    PrintToken (
      STRING_TOKEN (STR_HEXEDIT_LIBTITLEBAR_MAINEDITOR_TWOVARS),
      HiiHandle,
      HMainEditor.BufferImage->MemImage->Offset,
      HMainEditor.BufferImage->MemImage->Offset + HMainEditor.BufferImage->MemImage->Size - 1
      );

    for (Index = 0; Index < 9; Index++) {
      Print (L" ");
    }

    break;

  }
  //
  // 9 characters
  //
  switch (HMainEditor.BufferImage->BufferType) {
  case FILE_BUFFER:
    PrintToken (STRING_TOKEN (STR_HEXEDIT_LIBTITLEBAR_FILE), HiiHandle);
    break;

  case DISK_BUFFER:
    PrintToken (STRING_TOKEN (STR_HEXEDIT_LIBTITLEBAR_DISK), HiiHandle);
    break;

  case MEM_BUFFER:
    PrintToken (STRING_TOKEN (STR_HEXEDIT_LIBTITLEBAR_MEM), HiiHandle);
    break;
  }
  //
  // print modified field
  // 11 characters
  //
  if (HMainEditor.BufferImage->Modified) {
    Print (L"   Modified");
  }
  //
  // restore the old attribute
  //
  Out->SetAttribute (Out, Orig.Data);

  //
  // restore edit area cursor position
  //
  HBufferImageRestorePosition ();

  return EFI_SUCCESS;
}

EFI_STATUS
HMainTitleBarBackup (
  VOID
  )
/*++

Routine Description: 

  Backup function for MainTitleBar

Arguments:  

  None

Returns:  

  EFI_SUCCESS

--*/
{
  //
  // keep this function, just to keep consistency
  //
  return EFI_SUCCESS;
}
