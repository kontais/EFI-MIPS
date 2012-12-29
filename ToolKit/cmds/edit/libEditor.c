/*++

  Copyright (c) 1999 Intel Corporation

  Module Name:
    libEditor.c

  Abstract:
    Defines the Main Editor data type - 
     - Global variables 
     - Instances of the other objects of the editor
     - Main Interfaces

--*/

#ifndef _LIB_EDITOR
#define _LIB_EDITOR

#include "editor.h"

STATIC  EFI_EDITOR_COLOR_ATTRIBUTES OriginalColors;
STATIC  INTN                        OriginalMode;

// the first time editor launch
BOOLEAN EditorFirst;

// it's time editor should exit
BOOLEAN	EditorExit;

extern  EFI_EDITOR_FILE_BUFFER      FileBuffer;
extern  EFI_EDITOR_TITLE_BAR        MainTitleBar;
extern  EFI_EDITOR_STATUS_BAR       MainStatusBar;
extern  EFI_EDITOR_INPUT_BAR        MainInputBar;
extern  EFI_EDITOR_MENU_BAR         MainMenuBar;


EFI_EDITOR_GLOBAL_EDITOR MainEditor;

// basic initiliaztion for MainEditor
EFI_EDITOR_GLOBAL_EDITOR MainEditorConst = {
    &MainTitleBar,
    &MainMenuBar,
    &MainStatusBar,
    &MainInputBar,
    &FileBuffer,
    {0,0},
    {0,0},
    NULL
};


//
// Name:
//		MainEditorInit -- Init function for MainEditor
// In:
// 		VOID
// Out:
// 		EFI_SUCCESS
//		EFI_LOAD_ERROR
//		

STATIC
EFI_STATUS
MainEditorInit (VOID)
{
    EFI_STATUS  Status;

	// basic initializtipon
    CopyMem (&MainEditor, &MainEditorConst, sizeof(MainEditor));

	// set screen attributes
    MainEditor.ColorAttributes.Colors.Foreground = Out->Mode->Attribute & 0x000000ff;
    MainEditor.ColorAttributes.Colors.Background = (UINT8)(Out->Mode->Attribute >> 4);
    OriginalColors = MainEditor.ColorAttributes.Colors;

    OriginalMode = Out->Mode->Mode;

    // query screen size
    Out->QueryMode(Out,Out->Mode->Mode,&(MainEditor.ScreenSize.Column),&(MainEditor.ScreenSize.Row));
    
    
    // below will call the five components' init function
    
    Status = MainTitleBarInit ();
    if ( EFI_ERROR(Status) ) {
        Print (L"%EMainEditor init failed on TitleBar init\n%N");
        return EFI_LOAD_ERROR;
    }
    
    Status = MainMenuBarInit();
    if ( EFI_ERROR(Status)) {
        Print (L"%EMainEditor init failed on MainMenu init\n%N");
        return EFI_LOAD_ERROR;
    }
     
    Status = MainStatusBarInit ();
    if ( EFI_ERROR(Status) ) {
        Print (L"%EMainEditor init failed on StatusBar init\n%N");
        return EFI_LOAD_ERROR;
    }
    
    Status = MainInputBarInit ();
    if ( EFI_ERROR(Status)) {
        Print (L"%EMainEditor init failed on InputBar init\n%N");
        return EFI_LOAD_ERROR;
    }

    Status = FileBufferInit();
    if ( EFI_ERROR(Status) ) {
        Print (L"%EMainEditor init failed on FileBuffer init\n%N");
        return EFI_LOAD_ERROR;
    }
    
    // reset input
    Status = In->Reset(In,FALSE);
    if (EFI_ERROR(Status)) {
        Print (L"%ECould not obtain input device!%N\n");
        return EFI_LOAD_ERROR;
    }

	// clear whole screen and enable cursor
    Out->ClearScreen(Out);
    Out->EnableCursor(Out,TRUE);

	// inititalzie EditorFirst and EditorExit
	EditorFirst = TRUE;
	EditorExit = FALSE;

    return EFI_SUCCESS;
}




//
// Name:
//		MainEditorCleanup -- cleanup function for MainEditor
// In:
// 		VOID
// Out:
// 		EFI_SUCCESS
//		EFI_LOAD_ERROR
//		

STATIC
EFI_STATUS
MainEditorCleanup (
    VOID
    )
{
    EFI_STATUS      Status;

	// call the five components' cleanup function
	
    Status = MainTitleBarCleanup();
    if (EFI_ERROR (Status)) {
        Print (L"TitleBar cleanup failed\n");
    }

    Status = MainMenuBarCleanup();
    if (EFI_ERROR (Status)) {
        Print (L"MenuBar cleanup failed\n");
    }

    Status = MainStatusBarCleanup();
    if (EFI_ERROR (Status))  {
        Print (L"StatusBar cleanup failed\n");
    }

    Status = MainInputBarCleanup();
    if (EFI_ERROR (Status)) {
        Print (L"InputBar cleanup failed\n");
    }

    Status = FileBufferCleanup();
    if (EFI_ERROR (Status)) {
        Print (L"FileBuffer cleanup failed\n");
    }

    // restore old mode
    if (OriginalMode != Out->Mode->Mode) {
        Out->SetMode(Out,OriginalMode);
    }

    Out->SetAttribute(Out,EFI_TEXT_ATTR(OriginalColors.Foreground,OriginalColors.Background));
    Out->ClearScreen (Out);

    return EFI_SUCCESS; 
}




//
// Name:
//		MainEditorRefresh -- Refresh function for MainEditor
// In:
// 		VOID
// Out:
// 		EFI_SUCCESS
//
	
STATIC
EFI_STATUS
MainEditorRefresh (
    VOID
    )
{
	// to avoid screen flicker
	BS -> Stall ( 50 );
	
	// call the four components refresh function
    MainTitleBarRefresh();
    FileBufferRefresh();
    MainStatusBarRefresh();
    MainMenuBarRefresh();
    
    
    // EditorFirst is now set to FALSE
	EditorFirst = FALSE;

    return EFI_SUCCESS;
}




//
// Name:
//		MainEditorKeyInput -- Handle user key input. will route it to othe components handle function
// In:
// 		VOID
// Out:
// 		EFI_SUCCESS
//		EFI_LOAD_ERROR
// 		EFI_OUT_OF_RESOURCES
//

STATIC
EFI_STATUS
MainEditorKeyInput (
    VOID
    )
{
    EFI_INPUT_KEY   Key;
    EFI_STATUS      Status;

    do {
    	
    	// backup some key elements, so that can avoid some refesh work
		MainEditorBackup ();

		// wait for user key input
        WaitForSingleEvent(In->WaitForKey,0);
        Status = In->ReadKeyStroke(In,&Key);
        if ( EFI_ERROR(Status)) {
            continue;
        }

		// dispatch to different components' key handling function
        if (IS_VALID_CHAR(Key.ScanCode)) {
            Status = FileBufferHandleInput(&Key);
        } else if (IS_DIRECTION_KEY(Key.ScanCode)) {
            Status = FileBufferHandleInput(&Key);
        } else if (IS_FUNCTION_KEY(Key.ScanCode)) {
            Status = MainMenuBarHandleInput(&Key);
        } else {
            MainStatusBarSetStatusString(L"Unknown Command");
        }
		
			
		if ( Status != EFI_SUCCESS && Status != EFI_OUT_OF_RESOURCES ) {
			
			// not already has some error status
			if ( StrCmp ( L"", MainEditor.StatusBar->StatusString ) == 0 )
				MainStatusBarSetStatusString ( L"Disk Error. Try Again");
//			MainStatusBarRefresh();
		}

		// after handling, refresh editor
		MainEditorRefresh();
		
	}
    while ( Status != EFI_OUT_OF_RESOURCES && !EditorExit );


    return  Status;
}




//
// Name:
//		MainEditorSetCutLine -- Set clipboard
// In:
// 		Line -- Line to be set to clipboard
// Out:
// 		EFI_SUCCESS
//		EFI_OUT_OF_RESOURCES
//

STATIC
EFI_STATUS
MainEditorSetCutLine (
    EFI_EDITOR_LINE *Line
    )
{
	if ( Line == NULL ) {
		return EFI_SUCCESS;
	}
	
	if ( MainEditor.CutLine != NULL ) {
		
		// free the old clipboard
		LineFree ( MainEditor.CutLine );
	}
	
	// dulicate the line to clipboard
	MainEditor.CutLine = LineDup ( Line );
	if ( MainEditor.CutLine == NULL  ) {
		return EFI_OUT_OF_RESOURCES;
	}
	
	return EFI_SUCCESS;	
}




//
// Name:
//		MainEditorBackup -- Backup function for MainEditor
// In:
// 		VOID
// Out:
// 		EFI_SUCCESS
//

STATIC
EFI_STATUS
MainEditorBackup (
)
{
	// call the four components' backup function
	MainTitleBarBackup ();
	FileBufferBackup ();
	MainStatusBarBackup ();
	MainMenuBarBackup ();

	return EFI_SUCCESS;
}


#endif  //._LIB_EDITOR