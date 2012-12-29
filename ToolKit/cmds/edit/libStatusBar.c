/*++

  Copyright (c) 1999 Intel Corporation

  Module Name:
    libStatus.c

  Abstract:
    Definition for the Status Bar - the updatable display for the status of the editor

--*/

#ifndef _LIB_STATUS_BAR
#define _LIB_STATUS_BAR

#include "editor.h"


EFI_EDITOR_STATUS_BAR MainStatusBar;


BOOLEAN StatusBarNeedRefresh ;
BOOLEAN StatusStringChanged;

extern EFI_EDITOR_FILE_BUFFER FileBuffer;
extern EFI_EDITOR_FILE_BUFFER FileBufferBackupVar;




//
// Name:
//		MainStatusBarInit -- Init function for MainStatusBar
// In:
// 		VOID
// Out:
// 		EFI_SUCCESS
//
 
STATIC
EFI_STATUS
MainStatusBarInit   (
    VOID
    )
{     

    MainStatusBar.StatusString = NULL;
    
    MainStatusBarSetStatusString(L"");

	StatusBarNeedRefresh = TRUE;
    
    return EFI_SUCCESS;
}






//
// Name:
//		MainStatusBarBackup -- Backup function for MainStatusBar
// In:
// 		VOID
// Out:
// 		EFI_SUCCESS
//
 

STATIC
EFI_STATUS
MainStatusBarBackup (){
	
	
	return EFI_SUCCESS;
}








//
// Name:
//		MainStatusBarCleanup -- Cleanup function for MainStatusBar
// In:
// 		VOID
// Out:
// 		EFI_SUCCESS
//
 

STATIC
EFI_STATUS
MainStatusBarCleanup    (
    VOID
    )
{
        
    EditorFreePool ((VOID*)MainStatusBar.StatusString);
    

    return EFI_SUCCESS;
}






//
// Name:
//		MainStatusBarRefresh -- Refresh function for MainStatusBar
// In:
// 		VOID
// Out:
// 		EFI_SUCCESS
//
 

STATIC
EFI_STATUS
MainStatusBarRefresh    (
    VOID
    )
{
    EFI_EDITOR_COLOR_UNION  Orig,New;
    
	// if the old status string is the same with curren status string, so clear it
	if ( !StatusStringChanged )
		MainStatusBarSetStatusString ( L"\0");
    
    // when it's called first time after editor launch, so refresh is mandatory
	if ( EditorFirst == FALSE && !StatusBarNeedRefresh) {
		

		// all elements has been unchanged 
		if ( !StatusBarNeedRefresh &&
			 !StatusStringChanged &&
			FileBufferBackupVar.FilePosition.Row == FileBuffer.FilePosition.Row && 
			FileBufferBackupVar.FilePosition.Column == FileBuffer.FilePosition.Column &&
			FileBufferBackupVar.ModeInsert == FileBuffer.ModeInsert
			)	{
				return EFI_SUCCESS;
		}
		
	}

	// back up the screen attributes
    Orig = MainEditor.ColorAttributes;
    New.Colors.Foreground = Orig.Colors.Background;
    New.Colors.Background = Orig.Colors.Foreground;

    Out->SetAttribute (Out,New.Data);

	// clear status bar
    EditorClearLine(STATUS_BAR_LOCATION);
    
    // print row, column fields
    PrintAt (0,STATUS_BAR_LOCATION - 1 ,L"  Row: %d  Col: %d       %s",
        MainEditor.FileBuffer->FilePosition.Row,MainEditor.FileBuffer->FilePosition.Column,MainStatusBar.StatusString);
        
    // print insert mode field
    if ( MainEditor.FileBuffer->ModeInsert ) {
        PrintAt (MAX_TEXT_COLUMNS-10,STATUS_BAR_LOCATION -1 ,L"|%s|",L"INS");
    } else {
        PrintAt (MAX_TEXT_COLUMNS-10,STATUS_BAR_LOCATION - 1,L"|%s|",L"OVR");
    }
    
    // restore the old screen attributes
    Out->SetAttribute (Out,Orig.Data);

	// restore position in edit area
    FileBufferRestorePosition();

	StatusBarNeedRefresh = FALSE;
	StatusStringChanged = FALSE;

	return EFI_SUCCESS;
}







//
// Name:
//		MainStatusBarSetStatusString -- Set the StatusString field for MainStatusBar
// In:
// 		Str -- status string to set
// Out:
// 		EFI_SUCCESS
//		EFI_OUT_OF_RESOURCES
//
 

STATIC
EFI_STATUS
MainStatusBarSetStatusString (
    IN CHAR16* Str
    )
{
    StatusStringChanged = TRUE;

	// free the old status string
    EditorFreePool (MainStatusBar.StatusString);
    
    MainStatusBar.StatusString = StrDuplicate ( Str );
	if ( MainStatusBar.StatusString == NULL ) {
		return EFI_OUT_OF_RESOURCES;
	}
       
    return EFI_SUCCESS;
}


#endif  //_LIB_STATUS_BAR