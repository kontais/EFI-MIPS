/*++

  Copyright (c) 1999 Intel Corporation

  Module Name:
    libInputBar.h

  Abstract:
    Definition of the user input bar (multi-plexes with the Status Bar)

--*/

#ifndef _LIB_INPUT_BAR_H_
#define _LIB_INPUT_BAR_H_

#include "editortype.h"

STATIC  EFI_STATUS  MainInputBarInit (VOID);
STATIC  EFI_STATUS  MainInputBarCleanup (VOID);
STATIC  EFI_STATUS  MainInputBarRefresh (VOID);
STATIC  EFI_STATUS  MainInputBarSetPrompt (CHAR16*);
STATIC  EFI_STATUS	MainInputBarSetStringSize ( UINTN );

#endif