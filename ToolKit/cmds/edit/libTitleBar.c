/*++

  Copyright (c) 1999 Intel Corporation

  Module Name:
    libTitle.c

--*/

#ifndef _LIB_TITLE_BAR
#define _LIB_TITLE_BAR

#include "editor.h"


EFI_EDITOR_TITLE_BAR    MainTitleBar;


extern EFI_EDITOR_FILE_BUFFER FileBufferBackupVar;
extern EFI_EDITOR_FILE_BUFFER FileBuffer;






//
// Name:
//		MainTitleBarInit -- Init function for MainTitleBar
// In:
// 		VOID
// Out:
// 		EFI_SUCCESS
//		EFI_OUT_OF_RESOURCES
//

STATIC
EFI_STATUS
MainTitleBarInit    (
    VOID
    )
{
	// basic inititaliztion for MainTitleBar
    MainTitleBar.TitlePrefix = NULL;

	
	// set TitlePrefix
	MainTitleBar.TitlePrefix = PoolPrint ( L"  %s  %s", EDITOR_NAME, EDITOR_VERSION);
	if ( MainTitleBar.TitlePrefix == NULL ) {
		return EFI_OUT_OF_RESOURCES;
	}
    
    return EFI_SUCCESS;
}






//
// Name:
//		MainTitleBarCleanup -- cleanup function for MainTitleBar
// In:
// 		VOID
// Out:
// 		EFI_SUCCESS
//

STATIC
EFI_STATUS
MainTitleBarCleanup (
    VOID
    )
{
    
    EditorFreePool (MainTitleBar.TitlePrefix);
    
    return EFI_SUCCESS;
}







//
// Name:
//		MainTitleBarRefresh -- Refresh function for MainTitleBar
// In:
// 		VOID
// Out:
// 		EFI_SUCCESS
//

STATIC
EFI_STATUS
MainTitleBarRefresh (
    VOID
    )
{
    EFI_EDITOR_COLOR_UNION  Orig,New;
    UINTN i;
	
	// if it's first time when editor launch, so refresh is mandatory
	if ( EditorFirst == FALSE )
		// if all the fields are unchanged, no need to refresh
		if ( StrCmp ( FileBufferBackupVar.FileName , FileBuffer.FileName ) == 0 &&
			  FileBufferBackupVar.FileType == FileBuffer.FileType &&
			  FileBufferBackupVar.FileModified == FileBuffer.FileModified &&
			  FileBufferBackupVar.ReadOnly == FileBuffer.ReadOnly
		) 
			return EFI_SUCCESS;
	
	
	// backup the old screen attributes
    Orig = MainEditor.ColorAttributes;
    New.Colors.Foreground = Orig.Colors.Background;
    New.Colors.Background = Orig.Colors.Foreground;

    Out->SetAttribute (Out,New.Data);

    // clear the old title bar
    EditorClearLine (TITLE_BAR_LOCATION);
    
    // print the new title bar prefix
    // totally 25 chars
    PrintAt (TEXT_START_COLUMN - 1 ,TITLE_BAR_LOCATION - 1 ,L"%s     ",MainTitleBar.TitlePrefix);

	if ( MainEditor.FileBuffer->FileName == NULL ) {
		return EFI_SUCCESS;
	}
    
    // the space for file name is 20 characters
    if ( StrLen ( MainEditor.FileBuffer->FileName ) <= 20 ) {
    	Print ( L"%s   ",MainEditor.FileBuffer->FileName);

		for ( i = StrLen ( MainEditor.FileBuffer->FileName ); i < 20; i++ )
			Print(L" ");

    } else {
    	for ( i = 0; i < 17; i++ )
    	    Print ( L"%c",MainEditor.FileBuffer->FileName[i]);
    	   
    	 // print "..."
    	 Print (L"...");
   	}
    
    
    // print file type field
    if (MainEditor.FileBuffer->FileType == ASCII_FILE) {
        Print(L"     ASCII     ");
    } else {
        Print(L"     UNICODE   ");
    }

	// print readonly field
    if (MainEditor.FileBuffer->ReadOnly == TRUE) {
        Print(L"ReadOnly   ");
    } else {
        Print(L"           ");
    }


	// print modifed field
	if (MainEditor.FileBuffer->FileModified) {
        Print(L"Modified");
    }

	// restor the old attribute
    Out->SetAttribute (Out,Orig.Data);

	// restore edit area cursor position
    FileBufferRestorePosition();
    
    return EFI_SUCCESS;
}



//
// Name:
//		MainTitleBarBackup -- Backup function for MainTitleBar
// In:
// 		VOID
// Out:
// 		EFI_SUCCESS
//

EFI_STATUS
MainTitleBarBackup () {
	return EFI_SUCCESS;
}


#endif  //_LIB_TITLE_BAR