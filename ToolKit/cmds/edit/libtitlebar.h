/*++

  Copyright (c) 1999 Intel Corporation

  Module Name:
    libTitle.h

--*/

#ifndef _LIB_TITLE_BAR_H_
#define _LIB_TITLE_BAR_H_

#include "editortype.h"

STATIC  EFI_STATUS  MainTitleBarInit (VOID);
STATIC  EFI_STATUS  MainTitleBarCleanup (VOID);
STATIC  EFI_STATUS  MainTitleBarRefresh (VOID);
STATIC  EFI_STATUS  MainTitleBarSetTitle (CHAR16*);
STATIC  EFI_STATUS  MainTitleBarBackup (VOID);

#endif