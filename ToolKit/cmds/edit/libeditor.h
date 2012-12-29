/*++

  Copyright (c) 1999 Intel Corporation

  Module Name:
    libEditor.h

  Abstract:
    Defines the Main Editor data type - 
     - Global variables 
     - Instances of the other objects of the editor
     - Main Interfaces

--*/

#ifndef _LIB_EDITOR_H_
#define _LIB_EDITOR_H_

#include "editortype.h"

STATIC  EFI_STATUS  MainEditorInit (VOID);
STATIC  EFI_STATUS  MainEditorCleanup (VOID);
STATIC  EFI_STATUS  MainEditorRefresh (VOID);
STATIC  EFI_STATUS  MainEditorKeyInput (VOID);
STATIC  EFI_STATUS  MainEditorBackup (VOID);

STATIC EFI_STATUS MainEditorSetCutLine ( EFI_EDITOR_LINE *);
    

#endif
