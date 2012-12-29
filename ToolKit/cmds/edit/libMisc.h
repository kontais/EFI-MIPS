/*++

  Copyright (c) 1999 Intel Corporation

  Module Name:
    libMisc.h

  Abstract:
    Definitions for various line and string routines

--*/
#ifndef _LIB_MISC_H_
#define _LIB_MISC_H_

#include "editortype.h"


VOID EditorClearLine ( UINTN );
EFI_EDITOR_LINE* LineDup (EFI_EDITOR_LINE* );
VOID LineFree (EFI_EDITOR_LINE* );
VOID EditorFreePool (VOID *);
    

EFI_EDITOR_LINE* MoveLine ( INTN );
EFI_EDITOR_LINE* MoveCurrentLine ( INTN );

UINTN LineStrInsert ( EFI_EDITOR_LINE *,CHAR16,UINTN,UINTN);

VOID    LineCat (EFI_EDITOR_LINE*,EFI_EDITOR_LINE*);
    
VOID    LineDeleteAt (EFI_EDITOR_LINE*,UINTN);

UINTN UnicodeToAscii  (CHAR16  *,UINTN ,CHAR8   *);

UINTN StrStr  (CHAR16  *, CHAR16 *);

BOOLEAN IsValidFileName ( CHAR16 * ) ;


#endif