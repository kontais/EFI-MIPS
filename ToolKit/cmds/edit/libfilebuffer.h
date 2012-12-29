/*++

  Copyright (c) 2000 Intel Corporation

  Module Name:
    libFileBuffer.h

  Abstract:
    Defines FileBuffer - the view of the file that is visible at any point, 
    as well as the event handlers for editing the file
--*/


#ifndef _LIB_FILE_BUFFER_H_
#define _LIB_FILE_BUFFER_H_

#include "editortype.h"


STATIC  EFI_STATUS  FileBufferInit (VOID);
STATIC  EFI_STATUS  FileBufferCleanup (VOID);
STATIC  EFI_STATUS  FileBufferRefresh (VOID);
STATIC  EFI_STATUS  FileBufferHide (VOID);
STATIC  EFI_STATUS  FileBufferHandleInput (EFI_INPUT_KEY*);
STATIC  EFI_STATUS  FileBufferBackup (VOID);


STATIC  EFI_STATUS  FileBufferRestorePosition (VOID);
STATIC  EFI_STATUS 	FileBufferSetFileName ( CHAR16 *);

STATIC EFI_STATUS FileBufferHandleInput ( EFI_INPUT_KEY *);
    
STATIC  EFI_STATUS 	FileBufferRead ( CHAR16 * , BOOLEAN);
STATIC  EFI_STATUS 	FileBufferSave ( CHAR16 * );

STATIC 	EFI_EDITOR_LINE*	FileBufferCreateLine  ( VOID);


STATIC EFI_STATUS FileBufferDoCharInput (CHAR16 );
STATIC EFI_STATUS FileBufferAddChar ( CHAR16 );

BOOLEAN InCurrentScreen (UINTN ,UINTN ); 
BOOLEAN AboveCurrentScreen (UINTN );
BOOLEAN UnderCurrentScreen (UINTN );
BOOLEAN LeftCurrentScreen (UINTN );
BOOLEAN RightCurrentScreen (UINTN );
		

VOID FileBufferMovePosition ( UINTN ,UINTN);

STATIC EFI_STATUS FileBufferScrollRight ();
STATIC EFI_STATUS FileBufferScrollLeft ();
STATIC EFI_STATUS FileBufferScrollDown ();
STATIC EFI_STATUS FileBufferScrollUp ();
STATIC EFI_STATUS FileBufferPageUp ();
STATIC EFI_STATUS FileBufferPageDown ();
STATIC EFI_STATUS FileBufferHome ();
STATIC EFI_STATUS FileBufferEnd ();

STATIC EFI_STATUS FileBufferDoReturn ();
STATIC EFI_STATUS FileBufferDoBackspace ();
STATIC EFI_STATUS FileBufferDoDelete ();

STATIC EFI_STATUS FileBufferChangeMode ();

STATIC EFI_STATUS FileBufferCutLine ( EFI_EDITOR_LINE ** );
STATIC EFI_STATUS FileBufferPasteLine ( );

//EFI_STATUS FileBufferIsFileReadOnly ( EFI_FILE_HANDLE ,	BOOLEAN *);
EFI_STATUS FileBufferGetFileInfo ( 	EFI_FILE_HANDLE ,CHAR16 *, EFI_FILE_INFO ** );


STATIC EFI_STATUS FileBufferSearch ( CHAR16 * , UINTN);
STATIC EFI_STATUS FileBufferReplace ( CHAR16 * , UINTN);


#endif