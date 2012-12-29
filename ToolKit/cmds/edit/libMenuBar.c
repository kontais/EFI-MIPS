/*++

  Copyright (c) 2000 Intel Corporation

  Module Name:
    libMenuBar.c

  Abstract:
    Definition of the Menu Bar for the text editor

--*/


#ifndef _LIB_MENU_BAR
#define _LIB_MENU_BAR

#include "editor.h"

STATIC  EFI_STATUS  MainCommandFileType(VOID);


EFI_EDITOR_MENU_ITEM    MainMenuItems[] = {
    {   L"Open File",       L"F1",  MainCommandOpenFile        },
    {   L"Save File",       L"F2",  MainCommandSaveFile        },
    {   L"Exit",            L"F3",  MainCommandExit      },

    {   L"Cut Line",        L"F4",  MainCommandCutLine         },
    {   L"Paste Line",      L"F5",  MainCommandPasteLine       },
    {   L"Go To Line",      L"F6",  MainCommandGotoLine        },

    {   L"Search",          L"F7",  MainCommandSearch     },
    {   L"Search/Replace",  L"F8",  MainCommandSearchReplace   },
    {   L"File Type",       L"F9",  MainCommandFileType        },

    {   L"",                L"",    NULL            }
};


EFI_EDITOR_MENU_BAR     MainMenuBar;

// basic initialize for MainMenuBar
EFI_EDITOR_MENU_BAR     MainMenuBarConst = {
	MainMenuItems
};


//
// Name:
//		MainMenuBarInit -- init function for MainMenuBar;
// In:
//		VOID
// Out:
// 		EFI_SUCCESS
//		

STATIC
EFI_STATUS
MainMenuBarInit    (
    VOID
    ) 
{
    CopyMem (&MainMenuBar, &MainMenuBarConst, sizeof(EFI_EDITOR_MENU_BAR));

    return EFI_SUCCESS;
}







//
// Name:
//		MainMenuBarCleanup -- Cleanup function for MainMenuBar;
// In:
//		VOID
// Out:
// 		EFI_SUCCESS
//		

STATIC
EFI_STATUS  
MainMenuBarCleanup (
    VOID
    )
{
    return EFI_SUCCESS;
}






//
// Name:
//		MainMenuBarBackup -- backup function for MainMenuBar;
// In:
//		VOID
// Out:
// 		EFI_SUCCESS
//		

STATIC
EFI_STATUS
MainMenuBarBackup ()
{
	return EFI_SUCCESS;
}


//
// Name:
//		MainMenuBarHide -- Clear menu bar
// In:
//		VOID
// Out:
// 		EFI_SUCCESS
//		

STATIC
EFI_STATUS
MainMenuBarHide ( VOID ) {
	EditorClearLine ( MENU_BAR_LOCATION );
	EditorClearLine ( MENU_BAR_LOCATION + 1 );
	EditorClearLine ( MENU_BAR_LOCATION + 2 );
	
	return EFI_SUCCESS;
}


//
// Name:
//		MainMenuBarRefresh -- refresh function for MainMenuBar;
// In:
//		VOID
// Out:
// 		EFI_SUCCESS
//		

STATIC
EFI_STATUS
MainMenuBarRefresh (
    VOID
    )
{
    EFI_EDITOR_MENU_ITEM    *Item;
    UINTN                   Col = 1;
    UINTN                   Row = MENU_BAR_LOCATION;
    UINTN                   Width;

	// since menu bar won't be changed after editor is launch. so it's refresh only when editor is
	// just launched.
	if ( EditorFirst == FALSE )
		return EFI_SUCCESS;

    // clear menu bar
    MainMenuBarHide();

	// print out the menu items
    for (Item = MainMenuBar.MenuItems;Item->Function;Item++) {

        Width = max((StrLen(Item->Name)+6),20);
        if (((Col + Width) > MAX_TEXT_COLUMNS)) {
            Row++;
            Col = 1;
        }
        PrintAt(Col- 1,Row - 1 ,L"%E%s%N  %H%s%N  ",Item->FunctionKey,Item->Name);
        Col += Width;
    }

	// restore cursor position in edit area
    FileBufferRestorePosition();

    return  EFI_SUCCESS;
}







//
// Name:
//		MainMenuBarHandleInput -- Dispatch input to different handlers
//			Input can handle: F1 ~ F9
// In:
//		VOID
// Out:
// 		EFI_SUCCESS
//		

STATIC
EFI_STATUS
MainMenuBarHandleInput (
    EFI_INPUT_KEY   *Key
    )
{
    EFI_EDITOR_MENU_ITEM    *Item;
    EFI_STATUS              Status;
    UINTN   i = 0;
    UINTN   NumItems = 0;


    NumItems = sizeof(MainMenuItems)/sizeof(EFI_EDITOR_MENU_ITEM) - 1;

    Item = MainMenuBar.MenuItems;

    i = Key->ScanCode - SCAN_CODE_F1;

    if (i > (NumItems - 1) ) {
        MainStatusBarSetStatusString ( L"Unknown Command");
        return EFI_SUCCESS;
    }

    Item = &MainMenuBar.MenuItems[i];

    Status = Item->Function();
    
    return  Status;
}






//
// Name:
//		MainCommandOpenFile -- Load a file from disk to editor
// In:
//		VOID
// Out:
// 		EFI_SUCCESS
//		EFI_LOAD_ERROR
//		EFI_OUT_OF_RESOURCES
//		

STATIC
EFI_STATUS
MainCommandOpenFile    (
    VOID
    )
{
	BOOLEAN Done;
	EFI_STATUS	Status;
	
	
	
//	This command will open a file from current working directory. Read-only file can also be opened. But it can not be modIFied.
// Below is the scenario of Open File command:
// 1.	IF currently opened file has not been modIFied, directly go to step .
// 	IF currently opened file has been modIFied, an Input Bar will be prompted as :
// 			"File ModIFied. Save ( Yes/No/Cancel) ?"
// 					IF user press 'y' or 'Y', currently opened file will be saved.
// 					IF user press 'n' or 'N', currently opened file will not be saved.
// 					IF user press 'c' or 'C' or ESC, Open File command ends and currently opened file is still opened.
//
// 2.	An Input Bar will be prompted as :  "File Name to Open: "
// 			IF user press ESC, Open File command ends and currently opened file is still opened.
// 			Any other inputs with a Return will cause currently opened file close.
//
// 3.	IF user input file name is an existing file , this file will be read and opened.
//		IF user input file name is a new file, this file will be created and opened. This file's type ( UNICODE or ASCII ) is the same with the old file.


	// if current file is modified, so you need to choose whether to save it first.
	if ( MainEditor.FileBuffer->FileModified ) {

		Status = MainInputBarSetPrompt (L"File modified. Save (Yes/No/Cancel) ? ");
		if ( EFI_ERROR ( Status ) ) {
			return Status;
		}

		// the answer is just one characte
		Status = MainInputBarSetStringSize ( 1 );
		if ( EFI_ERROR ( Status ) ) {
			return Status;
		}

		// loop for user's answer
		// valid answer is just 'y' 'Y', 'n' 'N', 'c' 'C'
		Done = FALSE;
		while ( !Done ) {
			Status = MainInputBarRefresh ();
			
			// ESC pressed
			if ( Status == EFI_NOT_READY )  
				return EFI_SUCCESS;

			switch ( MainEditor.InputBar->ReturnString[0] ) {
				case L'y':
				case L'Y':
					// want to save this file first
					Status = FileBufferSave ( MainEditor.FileBuffer->FileName);
					if ( EFI_ERROR (Status ) ) 
						return Status;
					
					MainTitleBarRefresh ();
					Done = TRUE;
					break;

				case L'n':
				case L'N':
					// the file won't be saved
					Done = TRUE;
					break;

				case L'c':
				case L'C':
					return EFI_SUCCESS;				
			}
		}
	}


	// TO get the open file name
	Status = MainInputBarSetPrompt (L"File Name to Open: ");
	if ( EFI_ERROR ( Status ) ) {
		FileBufferRead ( MainEditor.FileBuffer->FileName , TRUE);
		return Status;
	}

	Status = MainInputBarSetStringSize ( 100 );
	if ( EFI_ERROR ( Status ) ) {
		FileBufferRead ( MainEditor.FileBuffer->FileName , TRUE);
		return Status;
	}

	
	while ( 1 ) {
		Status = MainInputBarRefresh ();
		
		// ESC pressed
		if ( Status == EFI_NOT_READY ) {
			return EFI_SUCCESS;
		}

		// THE input strng length should > 0
		if ( StrLen ( MainEditor.InputBar->ReturnString ) > 0 ) {
			
			// CHECK filename's validility
			if ( !IsValidFileName ( MainEditor.InputBar->ReturnString ) ) {
				FileBufferRead ( MainEditor.FileBuffer->FileName , TRUE);
				MainStatusBarSetStatusString (L"Invalid File Name");
				return EFI_SUCCESS;
			}
			
			break;
		}
	}

	
	// read from disk
	Status = FileBufferRead (MainEditor.InputBar->ReturnString, FALSE);
	
	if ( EFI_ERROR ( Status ) ) {
		FileBufferRead ( MainEditor.FileBuffer->FileName , TRUE);
		return EFI_LOAD_ERROR;
	}

    return EFI_SUCCESS;
}









//
// Name:
//		MainCommandSaveFile -- save current file to disk, you can save to current file name or 
//				save to another file name
// In:
//		VOID
// Out:
// 		EFI_SUCCESS
//		EFI_OUT_OF_RESOURCES
//		EFI_LOAD_ERROR
//		

STATIC
EFI_STATUS
MainCommandSaveFile    (
    VOID
    )
{
	EFI_STATUS Status;
	EFI_FILE_HANDLE Handle;
	BOOLEAN Done;
	CHAR16 *FileName;
	BOOLEAN OldFile;
	BOOLEAN ReadOnly;
	CHAR16 *Str;


    LIST_ENTRY              DirList;
    SHELL_FILE_ARG          *Arg;
    
    EFI_FILE_INFO		*Info;
	CHAR16 *FileNameTmp;	

	
	
// This command will save currently opened file to disk. You can choose save to another file name or just save to current file name. 
// Below is the scenario of Save File command: ( Suppose the old file name is A )
// 1.	An Input Bar will be prompted:    "File To Save: [ old file name]"
// 		IF user press ESC, Save File command ends .
// 		IF user press Enter, input file name will be A.
// 		IF user inputs a new file name B,  input file name will be B.
// 
// 2.	IF input file name is A, go to do Step 3.
// 		IF input file name is B, go to do Step 4.
// 
// 3.	IF A is read only, Status Bar will show "Access Denied" and Save File commands ends.
// 		IF A is not read only, save file buffer to disk and remove ModIFied flag in Title Bar , then Save File command ends.
//
// 4.	IF B does not exist, create this file and save file buffer to it. Go to do Step 7.
// 		IF B exits, do Step 5.
//
// 5.	An Input Bar will be prompted: "File Exists. Overwrite ( Yes/No/Cancel ) ?"
// 			IF user press 'y' or 'Y', do Step 6.
// 			IF user press 'n' or 'N', Save File commands ends.
// 			IF user press 'c' or 'C' or ESC, Save File commands ends.
//
// 6.	IF B is a read-only file, Status Bar will show "Access Denied" and Save File commands ends.
//		IF B is a read-writable file, save file buffer to B.
//
// 7.	Update File Name field in Title Bar to B and remove the ModIFied flag in Title Bar.
	
	Str = PoolPrint ( L"File to Save: [%s]", MainEditor.FileBuffer->FileName );
	if ( Str == NULL ) {
		return EFI_OUT_OF_RESOURCES;
	}

	if ( StrLen ( Str ) >= 50 ) {
		// replace the long file name with "..."
		Str[46] = L'.';
		Str[47] = L'.';
		Str[48] = L'.';
		Str[49] = L']';
		Str[50] = L'\0';
	}

	Status = MainInputBarSetPrompt (Str);
	if ( EFI_ERROR ( Status ) ) {
			return Status;
	}

	Status = MainInputBarSetStringSize ( 100 );
	if ( EFI_ERROR ( Status ) ) {
		return Status;
	}

	
	// get new file name
	Status = MainInputBarRefresh ();
	
	// if user pressed ESC
	if ( Status == EFI_NOT_READY ) {
		EditorFreePool ( Str );
		return EFI_SUCCESS;
	} 


	EditorFreePool ( Str );

	// if just enter pressed, so think save to current file name
	if ( StrLen ( MainEditor.InputBar -> ReturnString ) == 0 ) {
		FileName = PoolPrint ( L"%s", MainEditor.FileBuffer->FileName );
	} else {
		FileName = PoolPrint ( L"%s", MainEditor.InputBar->ReturnString );
	}
	
	if ( FileName == NULL ) 
		return EFI_OUT_OF_RESOURCES;

	if ( !IsValidFileName (FileName ) ) {
		MainStatusBarSetStatusString (L"Invalid File Name");
		FreePool ( FileName );
		return EFI_SUCCESS;
	}
			
	OldFile = FALSE;


	
	// save to the old file
	if ( StriCmp ( FileName, MainEditor.FileBuffer->FileName ) == 0 ) 
		OldFile = TRUE;

	if ( OldFile ) {
		
		// file has not been modified, so no need to write disk
		if ( MainEditor.FileBuffer->FileModified == FALSE ) {
			FreePool ( FileName );
			return EFI_SUCCESS;
		}

		// if the file is read only, so can not write back to it.
		if ( MainEditor.FileBuffer->ReadOnly == TRUE ) {
			MainStatusBarSetStatusString ( L"Access Denied");
			FreePool ( FileName );
			return EFI_SUCCESS;
		}
	} else {
		// save to a new file name
		
			FileNameTmp = PoolPrint(L"%s", FileName );
			if ( FileNameTmp == NULL ) {
				FreePool ( FileName );
				return EFI_OUT_OF_RESOURCES;
			}


	
			// GET CURRENT DIR HANDLE
			InitializeListHead (&DirList);
			Status = ShellFileMetaArg(FileNameTmp, &DirList);
			FreePool ( FileNameTmp );
			
			if ( EFI_ERROR ( Status ) ) {
				EditorFreePool ( FileName );
				return EFI_LOAD_ERROR ;
			}

			if ( DirList.Flink == &DirList ) {
				return EFI_LOAD_ERROR ;
			}


			Arg = CR(DirList.Flink, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);

		    // since when Status is returned correct, not everything is correct, 
			// so  I have to add these check (-:(((((
			if ( Arg == NULL ) {
				return EFI_LOAD_ERROR ;
			}

			if ( Arg->Parent == NULL ) {
				return EFI_LOAD_ERROR ;
			}
			
			Info = Arg->Info;


	
			// file exists
			if ( !Arg->Status ) {
				if ( !Arg->Handle ) 
			        Status = Arg->Parent->Open (
                    Arg->Parent,
                    &Arg->Handle,
                    Arg->FileName,
                    EFI_FILE_MODE_READ ,
                    0
                    );

			Handle = Arg->Handle;


#ifndef EFI_NT_EMULATOR		
				Status = FileBufferGetFileInfo ( Handle, Arg->FileName, &Info );
#else
				Status = FileBufferGetFileInfo ( Arg->Parent, Arg->FileName, &Info );
#endif

				if ( EFI_ERROR ( Status ) ) {
						EditorFreePool ( FileName );
						ShellFreeFileList (&DirList);
						return EFI_LOAD_ERROR;
					}

					// check if read only
					if ( Info->Attribute & EFI_FILE_READ_ONLY ) {
						ReadOnly = TRUE;
					} else {
						ReadOnly = FALSE;
					}


				// ask user whethe to overwrite this file
				Status = MainInputBarSetPrompt (L"File exists. Overwrite (Yes/No/Cancel) ? ");
				if ( EFI_ERROR ( Status ) ) {
					EditorFreePool ( FileName );
					ShellFreeFileList (&DirList);
					return Status;
				}

				Status = MainInputBarSetStringSize ( 1 );
				if ( EFI_ERROR ( Status ) ) {
					EditorFreePool ( FileName );
					ShellFreeFileList (&DirList);
					return Status;
				}

				Done = FALSE;
				while ( !Done ) {
					Status = MainInputBarRefresh ();

					// ESC pressed
					if ( Status == EFI_NOT_READY )  {
						EditorFreePool ( FileName );
						ShellFreeFileList (&DirList);
						return EFI_SUCCESS;
					}

					switch ( MainEditor.InputBar->ReturnString[0] ) {
						case L'y':
						case L'Y':
							if ( ReadOnly ) {
								// file is readonly, can not be overwrote
								MainStatusBarSetStatusString (L"Access Denied");
								EditorFreePool ( FileName );
								ShellFreeFileList (&DirList);
								return EFI_SUCCESS;
							}

							Done = TRUE;
							break;

						case L'n':
						case L'N':
							EditorFreePool ( FileName);		
							ShellFreeFileList (&DirList);
							return EFI_SUCCESS;

						case L'c':
						case L'C':
							EditorFreePool ( FileName );
							ShellFreeFileList (&DirList);
							return EFI_SUCCESS;
					} // end switch
				} // end while

			
			}  // end file exists

		ShellFreeFileList (&DirList);

		
	} // end of if OldFile

	// save file back to disk
	Status = FileBufferSave ( FileName );
	EditorFreePool ( FileName );

	if ( EFI_ERROR ( Status )) {
		return EFI_LOAD_ERROR;
	}

    return EFI_SUCCESS; 
}








//
// Name:
//		MainCommandExit -- exit editor
// In:
//		VOID
// Out:
// 		EFI_SUCCESS
//		EFI_OUT_OF_RESOURCES
//		EFI_LOAD_ERROR
//		

STATIC
EFI_STATUS 
MainCommandExit (
    VOID
    ) 
{ 
	EFI_STATUS Status;

	
// Below is the scenario of Exit command: 
// 1.	IF currently opened file is not modIFied, exit the editor and Exit command ends.
// 		IF currently opened file is modIFied, do Step 2
//
// 2.	An Input Bar will be prompted:   "File modIFied. Save ( Yes/No/Cancel )?"
// 			IF user press 'y' or 'Y', currently opened file will be saved and Editor exits
// 			IF user press 'n' or 'N', currently opened file will not be saved and Editor exits.
// 			IF user press 'c' or 'C' or ESC, Exit command ends.
	
	
	// if file has been modified, so will prompt user whether to save the changes
	if ( MainEditor.FileBuffer->FileModified ) {

		Status = MainInputBarSetPrompt (L"File modified. Save (Yes/No/Cancel) ? ");
		if ( EFI_ERROR ( Status ) ) {
			return Status;
		}

		Status = MainInputBarSetStringSize ( 1 );
		if ( EFI_ERROR ( Status ) ) {
			return Status;
		}

		while ( 1 ) {
			Status = MainInputBarRefresh ();

			// ESC pressed
			if ( Status == EFI_NOT_READY )  
				return EFI_SUCCESS;

			switch ( MainEditor.InputBar->ReturnString[0] ) {
				case L'y':
				case L'Y':
					// write file back to disk
					Status = FileBufferSave (MainEditor.FileBuffer->FileName);
					if ( !EFI_ERROR (Status )) 
						EditorExit = TRUE;

					return Status;

				case L'n':
				case L'N':
					EditorExit = TRUE;
					return EFI_SUCCESS;

				case L'c':
				case L'C':
					return EFI_SUCCESS;
				
			}
		}
	}


	EditorExit = TRUE;
   return EFI_SUCCESS;

}

  







//
// Name:
//		MainCommandCutLine -- cut current line to clipboard
// In:
//		VOID
// Out:
// 		EFI_SUCCESS
//		EFI_OUT_OF_RESOURCES
//		EFI_LOAD_ERROR
//		

STATIC
EFI_STATUS
MainCommandCutLine (
    VOID
    )
{
	EFI_STATUS Status;
	EFI_EDITOR_LINE *Line;


// This command will cut current line ( where cursor is on ) to clip board. And cursor will move to the beginning of next line.
// Below is the scenario of Cut Line command:
// 1.	IF cursor is on valid line, current line will be cut to clip board.
// 		IF cursor is not on valid line, an Status String will be prompted : "Nothing to Cut".

	Status = FileBufferCutLine ( &Line ); 
	if ( Status == EFI_NOT_FOUND ) 
		return EFI_SUCCESS;

	if ( EFI_ERROR ( Status ) ) {
		return Status ;
	}


	MainEditor.CutLine  = Line;

    return EFI_SUCCESS; 
}







//
// Name:
//		MainCommandPasteLine -- paste line to file buffer
// In:
//		VOID
// Out:
// 		EFI_SUCCESS
//		EFI_OUT_OF_RESOURCES
//		EFI_LOAD_ERROR
//		

STATIC
EFI_STATUS  
MainCommandPasteLine   (
    VOID
    )
{
	EFI_STATUS Status;

// Below is the scenario of Paste Line command:
// 1.	IF nothing is on clipboard, a Status String will be prompted : "No Line to Paste" and Paste Line command ends.
// 		IF something is on clipboard, insert it above current line.

	// nothing on clipboard
	if ( MainEditor.CutLine == NULL ) {
		MainStatusBarSetStatusString (L"No Line to Paste");
		return EFI_SUCCESS;
	}


	Status = FileBufferPasteLine ();

    return Status;
}








//
// Name:
//		MainCommandGotoLine -- move cursor to specified lines
// In:
//		VOID
// Out:
// 		EFI_SUCCESS
//		
STATIC
EFI_STATUS
MainCommandGotoLine    (
    VOID
    )
{
	EFI_STATUS Status;
	UINTN Row;


// Below is the scenario of Go To Line command:
// 1.	An Input Bar will be prompted : "Go To Line:".
// 			IF user press ESC, Go To Line command ends.
// 			IF user just press Enter, cursor remains unchanged.
// 			IF user inputs line number, do Step 2.
//
// 2.	IF input line number is valid, move cursor to the beginning of specIFied line and Go To Line command ends.
//		IF input line number is invalid, a Status String will be prompted: "No Such Line" and Go To Line command ends.

	Status = MainInputBarSetPrompt (L"Go To Line: ");
	if ( EFI_ERROR ( Status ) ) {
		return Status;
	}

	// line numbe's digit <= 6
	Status = MainInputBarSetStringSize ( 6 );
	if ( EFI_ERROR ( Status ) ) {
		return Status;
	}

	Status = MainInputBarRefresh ();
	
	// press ESC
	if ( Status == EFI_NOT_READY ) {
		return EFI_SUCCESS;
	}

	// if JUST press enter
	if ( StrLen ( MainEditor.InputBar->ReturnString ) == 0 ) {
			return EFI_SUCCESS;
	}
	

    Row = Atoi(MainEditor.InputBar->ReturnString);

	// invliad line number
    if ( Row > MainEditor.FileBuffer->NumLines || Row <= 0) {
        MainStatusBarSetStatusString (L"No Such Line");
		return EFI_SUCCESS;
    }
	
	
	// move cursor to that line's start
	FileBufferMovePosition ( Row, 1 );

    return EFI_SUCCESS; 
}









//
// Name:
//		MainCommandSearch -- search string in file buffer
// In:
//		VOID
// Out:
// 		EFI_SUCCESS
//		EFI_OUT_OF_RESOURCES
//		EFI_LOAD_ERROR
//		

STATIC
EFI_STATUS
MainCommandSearch  (
    VOID
    )
{
	EFI_STATUS Status;
	CHAR16 *Buffer;
	BOOLEAN Done;
	UINTN Offset;


// Below is the scenario of Search command:
// 1.	An Input Bar will be prompted : "Enter Search String:".
// 			IF user press ESC, Search command ends.
// 			IF user just press Enter, Search command ends.
// 			IF user inputs the search string,  do Step 2.
//
// 2.	IF input search string is found, cursor will move to the first occurrence and do Step 3.
// 		IF input search string is not found, a Status String "Search String Not Found" will be prompted and Search command ends.
//
// 3.	An Input Bar will be prompted: "Find Next (Yes/No/Cancel ) ?".
//			IF user press ESC, Search command ends.
// 			IF user press 'y' or 'Y', do Step 2.
// 			IF user press 'n' or 'N', Search command ends.
// 			IF user press 'c' or 'C', Search comman ends.

	Status = MainInputBarSetPrompt (L"Enter Search String: ");
	if ( EFI_ERROR ( Status ) ) {
		return Status;
	}

	Status = MainInputBarSetStringSize ( 40 );
	if ( EFI_ERROR ( Status ) ) {
		return Status;
	}

	Status = MainInputBarRefresh ();
	
	// ESC
	if ( Status == EFI_NOT_READY ) {
		return EFI_SUCCESS;
	}

	// just enter pressed
	if ( StrLen ( MainEditor.InputBar->ReturnString ) == 0 ) {
			return EFI_SUCCESS;
	}

	Buffer = PoolPrint (L"%s", MainEditor.InputBar->ReturnString );
	if ( Buffer == NULL ) {
		return EFI_OUT_OF_RESOURCES;
	}

	
	// the first time , search from current positon
	Offset = 0;
	do {
		
		// since search may be continued to search multiple times
		// so we need to backup editor each time
		MainEditorBackup ();
		
		Status = FileBufferSearch ( Buffer, Offset );
		if ( Status == EFI_NOT_FOUND ) 
			break;

	
		// Find next 
		Status = MainInputBarSetPrompt (L"Find Next (Yes/No/Cancel) ?");
		if ( EFI_ERROR ( Status ) ) {
			FreePool ( Buffer );
			return Status;
		}

		Status = MainInputBarSetStringSize ( 1 );
		if ( EFI_ERROR ( Status ) ) {
			FreePool ( Buffer );
			return Status;
		}

		Done = FALSE;
		while ( !Done ) {
			Status = MainInputBarRefresh ();
			
			// ESC pressed
			if ( Status == EFI_NOT_READY )  {
				FreePool ( Buffer );
				return EFI_SUCCESS;
			}

			switch ( MainEditor.InputBar->ReturnString[0] ) {
				case L'y':
				case L'Y':
					Done = TRUE;
					break;

				case L'n':
				case L'N':
				case L'c':
				case L'C':
					FreePool ( Buffer );
					return EFI_SUCCESS;
				
			} // end of which
		} // end of while !Done

 		// for search second, third time, search from current position + strlen
		Offset = StrLen ( Buffer ); 

	} while ( 1 ); // endof do


	FreePool ( Buffer );
	MainStatusBarSetStatusString (L"Search String Not Found");
	
	return EFI_SUCCESS;
}









//
// Name:
//		MainCommandSearchReplace -- search string in file buffer, and replace it with another str
// In:
//		VOID
// Out:
// 		EFI_SUCCESS
//		EFI_OUT_OF_RESOURCES
//		EFI_LOAD_ERROR
//		

STATIC
EFI_STATUS
MainCommandSearchReplace   (
    VOID
    )
{
	EFI_STATUS Status;
	CHAR16 *Search, *Replace;
	BOOLEAN Done;
	BOOLEAN First;
	BOOLEAN ReplaceOption;
	UINTN SearchLen;
	UINTN ReplaceLen;
	BOOLEAN ReplaceAll;

// Below is the scenario of Search/Replace command:
// 1.	An Input Bar is prompted : "Enter Search String:".
// 			IF user press ESC, Search/Replace command ends.
// 			IF user just press Enter, Search/Replace command ends.
// 			IF user inputs the search string S, do Step 2.
//
// 2.	An Input Bar is prompted: "Replace With:".
// 			IF user press ESC, Search/Replace command ends.
//			IF user inputs the replace string R, do Step 3.
//
// 3.	IF input search string is not found, an Status String "Search String Not Found" will be prompted and Search/Replace command ends
//		 IF input search string is found, do Step 4.
//
//4.	An Input Bar will be prompted:"Replace ( Yes/No/All/Cancel )?"
// 			IF user press 'y' or 'Y', S will be replaced with R and do Step 5
// 			IF user press 'n' or 'N', S will not be replaced and do Step 5.
// 			IF user press 'a' or 'A', all the S from file current position on will be replaced with R and Search/Replace command ends.
// 			IF user press 'c' or 'C' or ESC, Search/Replace command ends.
//
// 5.	An Input Bar will be prompted: "Find Next (Yes/No/Cancel) ?".
// 			IF user press ESC, Search/Replace command ends.
// 			IF user press 'y' or 'Y', do Step 3.
// 			IF user press 'n' or 'N', Search/Replace command ends.
// 			IF user press 'c' or 'C', Search/Replace comman ends.


	// input search string
	Status = MainInputBarSetPrompt (L"Enter Search String: ");
	if ( EFI_ERROR ( Status ) ) {
		return Status;
	}

	Status = MainInputBarSetStringSize ( 40 );
	if ( EFI_ERROR ( Status ) ) {
		return Status;
	}

	Status = MainInputBarRefresh ();
	
	// ESC
	if ( Status == EFI_NOT_READY ) {
		return EFI_SUCCESS;
	}

	// if just pressed enter
	if ( StrLen ( MainEditor.InputBar->ReturnString ) == 0 ) {
			return EFI_SUCCESS;
	}

	Search = PoolPrint (L"%s", MainEditor.InputBar->ReturnString );
	if ( Search == NULL ) {
		return EFI_OUT_OF_RESOURCES;
	}

	SearchLen = StrLen( Search );

	// input replace string
	Status = MainInputBarSetPrompt (L"Replace With: ");
	if ( EFI_ERROR ( Status ) ) {
		return Status;
	}

	Status = MainInputBarSetStringSize ( 40 );
	if ( EFI_ERROR ( Status ) ) {
		return Status;
	}

	Status = MainInputBarRefresh ();
	
	// ESC
	if ( Status == EFI_NOT_READY ) {
		return EFI_SUCCESS;
	}


	Replace = PoolPrint (L"%s", MainEditor.InputBar->ReturnString );
	if ( Replace == NULL ) {
		FreePool ( Search );
		return EFI_OUT_OF_RESOURCES;
	}
	
	
	ReplaceLen = StrLen ( Replace );


	First = TRUE;
	ReplaceAll = FALSE;
	do {

		// since search may be continued to search multiple times
		// so we need to backup editor each time
		MainEditorBackup ();


		if ( First ) 
			Status = FileBufferSearch ( Search, 0 );
		else {
				// if just replace, so skip this replace string
				// if replace string is an empty string, so skip to next character
				if (ReplaceOption )
					Status = FileBufferSearch ( Search, ( ReplaceLen == 0 ) ? 1:ReplaceLen );
				else 
					Status = FileBufferSearch ( Search, SearchLen );
		}

		if ( Status == EFI_NOT_FOUND ) 
			break;

		if ( !ReplaceAll ) {
		// replace or not? 
		Status = MainInputBarSetPrompt (L"Replace (Yes/No/All/Cancel) ?");
			
		if ( EFI_ERROR ( Status ) ) {
			FreePool ( Search );
			FreePool ( Replace );
			return Status;
		}

		Status = MainInputBarSetStringSize ( 1 );
		if ( EFI_ERROR ( Status ) ) {
			FreePool ( Search );
			FreePool ( Replace );
			return Status;
		}

		Done = FALSE;
		while ( !Done ) {
			Status = MainInputBarRefresh ();

			// ESC pressed
			if ( Status == EFI_NOT_READY )  {
				FreePool ( Search );
				FreePool ( Replace );
				return EFI_SUCCESS;
			}

			switch ( MainEditor.InputBar->ReturnString[0] ) {
				case L'y':
				case L'Y':
					Done = TRUE;
					ReplaceOption = TRUE;
					break;

				case L'n':
				case L'N':
					Done = TRUE;
					ReplaceOption = FALSE;
					break;

				case L'a':
				case L'A':
					Done = TRUE;
					ReplaceOption = TRUE;
					ReplaceAll = TRUE;
					break;

				case L'c':
				case L'C':
					FreePool ( Search );
					FreePool ( Replace );
					return EFI_SUCCESS;
				
			} // end of which
		} // end of while !Done
		} // end of if ReplaceAll

		//Decide to Replace
		if ( ReplaceOption ) {
			// file is readonly
			if ( MainEditor.FileBuffer->ReadOnly == TRUE ) {
				MainStatusBarSetStatusString (L"Read Only File Can Not Be Modified");
				return EFI_SUCCESS;
			}

			// replace
			Status = FileBufferReplace ( Replace, SearchLen);
			if ( EFI_ERROR ( Status ) ) {
					FreePool ( Search );
					FreePool ( Replace );
					return Status;
			}
		}

		if ( !ReplaceAll ) {
		// Find next 
		Status = MainInputBarSetPrompt (L"Find Next (Yes/No/Cancel) ?");
		if ( EFI_ERROR ( Status ) ) {
			FreePool ( Search );
			FreePool ( Replace );
			return Status;
		}

		Status = MainInputBarSetStringSize ( 1 );
		if ( EFI_ERROR ( Status ) ) {
			FreePool ( Search );
			FreePool ( Replace );
			return Status;
		}

		Done = FALSE;
		while ( !Done ) {
			Status = MainInputBarRefresh ();

			// ESC pressed
			if ( Status == EFI_NOT_READY )  {
				FreePool ( Search );
				FreePool ( Replace );
				return EFI_SUCCESS;
			}

			switch ( MainEditor.InputBar->ReturnString[0] ) {
				case L'y':
				case L'Y':
					Done = TRUE;
					break;

				case L'n':
				case L'N':
				case L'c':
				case L'C':
					FreePool ( Search );
					FreePool ( Replace );
					return EFI_SUCCESS;
				
			} // end of which
		} // end of while !Done
		}  // end of if ReplaceAll
		First = FALSE;

	} while ( 1 ); // endof do


	FreePool ( Search );
	FreePool ( Replace );

	MainStatusBarSetStatusString (L"Search String Not Found");

    return EFI_SUCCESS; 
}









//
// Name:
//		MainCommandFileType -- set file type
// In:
//		VOID
// Out:
// 		EFI_SUCCESS
//		EFI_OUT_OF_RESOURCES
//		EFI_LOAD_ERROR
//		

STATIC  
EFI_STATUS  
MainCommandFileType    (
    VOID
    )
{

	
// Below is the scenario of File Type command:
// After File Type is executed, file type will be changed to another type
	

		
	// if file is readonly, can not be modified
	if ( MainEditor.FileBuffer->ReadOnly == TRUE ) {
		MainStatusBarSetStatusString (L"Read Only File Can Not Be Modified");
		return EFI_SUCCESS;
	}

	if ( MainEditor.FileBuffer->FileType == UNICODE_FILE ) {
		MainEditor.FileBuffer->FileType = ASCII_FILE;
	} else {
		MainEditor.FileBuffer->FileType = UNICODE_FILE;
	}
	  
	
	MainEditor.FileBuffer ->FileModified = TRUE; 

    return EFI_SUCCESS;
}




#endif  //_LIB_MENU_BAR
