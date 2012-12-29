/*++

  Copyright (c) 2000 Intel Corporation

  Module Name:
    libMenuBar.h

  Abstract:
    Definition of the Menu Bar for the text editor

--*/


#ifndef _LIB_MENU_BAR_H_
#define _LIB_MENU_BAR_H_

#include "editortype.h"

STATIC  EFI_STATUS  MainMenuBarInit (VOID);
STATIC  EFI_STATUS  MainMenuBarCleanup (VOID);
STATIC  EFI_STATUS  MainMenuBarHide (VOID);
STATIC  EFI_STATUS  MainMenuBarRefresh (VOID);
STATIC  EFI_STATUS  MainMenuBarHandleInput (EFI_INPUT_KEY*);
STATIC  EFI_STATUS  MainMenuBarBackup(VOID);

STATIC  EFI_STATUS  MainCommandOpenFile(VOID);
STATIC  EFI_STATUS  MainCommandSaveFile(VOID);
STATIC  EFI_STATUS  MainCommandExit(VOID);

STATIC  EFI_STATUS  MainCommandCutLine(VOID);
STATIC  EFI_STATUS  MainCommandPasteLine(VOID);

STATIC  EFI_STATUS  MainCommandSearch(VOID);
STATIC  EFI_STATUS  MainCommandSearchReplace(VOID);
STATIC  EFI_STATUS  MainCommandGotoLine(VOID);




#endif