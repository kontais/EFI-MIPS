/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

  Module Name:
    libFileBuffer.h

  Abstract:
    Defines FileBuffer - the view of the file that is visible at any point, 
    as well as the event handlers for editing the file
--*/

#ifndef _LIB_FILE_BUFFER_H_
#define _LIB_FILE_BUFFER_H_

#include "editortype.h"

EFI_STATUS
FileBufferInit (
  VOID
  );
EFI_STATUS
FileBufferCleanup (
  VOID
  );
EFI_STATUS
FileBufferRefresh (
  VOID
  );
EFI_STATUS
FileBufferHide (
  VOID
  );
EFI_STATUS
FileBufferHandleInput (
  EFI_INPUT_KEY *
  );
EFI_STATUS
FileBufferBackup (
  VOID
  );

EFI_STATUS
FileBufferRestorePosition (
  VOID
  );
EFI_STATUS
FileBufferSetFileName (
  CHAR16 *
  );

EFI_STATUS
FileBufferHandleInput (
  EFI_INPUT_KEY *
  );

EFI_STATUS
FileBufferRead (
  CHAR16  *,
  BOOLEAN
  );
EFI_STATUS
FileBufferSave (
  CHAR16 *
  );

EFI_EDITOR_LINE *
FileBufferCreateLine (
  VOID
  );

EFI_STATUS
FileBufferDoCharInput (
  CHAR16
  );
EFI_STATUS
FileBufferAddChar (
  CHAR16
  );

BOOLEAN
InCurrentScreen (
  UINTN,
  UINTN
  );
BOOLEAN
AboveCurrentScreen (
  UINTN
  );
BOOLEAN
UnderCurrentScreen (
  UINTN
  );
BOOLEAN
LeftCurrentScreen (
  UINTN
  );
BOOLEAN
RightCurrentScreen (
  UINTN
  );

VOID
FileBufferMovePosition (
  UINTN,
  UINTN
  );

EFI_STATUS
FileBufferScrollRight (
  VOID
  );
EFI_STATUS
FileBufferScrollLeft (
  VOID
  );
EFI_STATUS
FileBufferScrollDown (
  VOID
  );
EFI_STATUS
FileBufferScrollUp (
  VOID
  );
EFI_STATUS
FileBufferPageUp (
  VOID
  );
EFI_STATUS
FileBufferPageDown (
  VOID
  );
EFI_STATUS
FileBufferHome (
  VOID
  );
EFI_STATUS
FileBufferEnd (
  VOID
  );

EFI_STATUS
FileBufferDoReturn (
  VOID
  );
EFI_STATUS
FileBufferDoBackspace (
  VOID
  );
EFI_STATUS
FileBufferDoDelete (
  VOID
  );

EFI_STATUS
FileBufferChangeMode (
  VOID
  );

EFI_STATUS
FileBufferCutLine (
  EFI_EDITOR_LINE **
  );
EFI_STATUS
FileBufferPasteLine (
  VOID
  );

EFI_STATUS
FileBufferGetFileInfo (
  EFI_FILE_HANDLE,
  CHAR16          *,
  EFI_FILE_INFO   **
  );

EFI_STATUS
FileBufferSearch (
  CHAR16  *,
  UINTN
  );
EFI_STATUS
FileBufferReplace (
  CHAR16  *,
  UINTN
  );
EFI_STATUS
FileBufferReplaceAll (
  CHAR16  *,
  CHAR16  *,
  UINTN
  );

VOID
FileBufferAdjustMousePosition (
  INT32,
  INT32
  );

CHAR16 *
EditGetDefaultFileName (
  VOID
  );

#endif
