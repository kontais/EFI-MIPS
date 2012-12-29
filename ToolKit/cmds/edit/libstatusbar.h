/*++

  Copyright (c) 1999 Intel Corporation

  Module Name:
    libStatus.h

  Abstract:
    Definition for the Status Bar - the updatable display for the status of the editor

--*/

#ifndef _LIB_STATUS_BAR_H_
#define _LIB_STATUS_BAR_H_

#include "editortype.h"

STATIC  EFI_STATUS  MainStatusBarInit (VOID);
STATIC  EFI_STATUS  MainStatusBarCleanup (VOID);
STATIC  EFI_STATUS  MainStatusBarRefresh (VOID);
STATIC  EFI_STATUS  MainStatusBarHide (VOID);
STATIC  EFI_STATUS  MainStatusBarSetStatusString (CHAR16*);
STATIC  EFI_STATUS  MainStatusBarSetMode (BOOLEAN);
STATIC  EFI_STATUS  MainStatusBarBackup (VOID);

#endif