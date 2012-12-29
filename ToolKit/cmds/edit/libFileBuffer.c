/*++

  Copyright (c) 2000 Intel Corporation

  Module Name:
    libFileBuffer.c

  Abstract:
    Defines FileBuffer - the view of the file that is visible at any point, 
    as well as the event handlers for editing the file
--*/


#ifndef _LIB_FILE_BUFFER
#define _LIB_FILE_BUFFER

#include "editor.h"

extern EFI_HANDLE ImageHandleBackup;


EFI_EDITOR_FILE_BUFFER  FileBuffer;
EFI_EDITOR_FILE_BUFFER  FileBufferBackupVar;

// for basic initialization of FileBuffer
EFI_EDITOR_FILE_BUFFER  FileBufferConst = {
    NULL,
    UNICODE_FILE,
    NULL,
    NULL,
    0,
    USE_LF,
//    NULL,
    {0,0},
    {0,0},
    {0,0},
    FALSE,
    TRUE,
	FALSE,
    NULL
};


// the whole edit area needs to be refreshed
BOOLEAN FileBufferNeedRefresh ;

// only the current line in edit area needs to be refresh
BOOLEAN FileBufferOnlyLineNeedRefresh ;

//
// Name:
//		FileBufferInit -- Initialization function for FileBuffer
// In:
//		VOID
// Out:
//		EFI_SUCCESS
// 		EFI_LOAD_ERROR
//
STATIC
EFI_STATUS
FileBufferInit (
	VOID
    )
{ 
    /*UINTN   i;
    EFI_STATUS Status;
    CHAR16          *CurDir;
    EFI_DEVICE_PATH *DevicePath;
    EFI_LOADED_IMAGE        *LoadedImage;
    EFI_FILE_IO_INTERFACE   *Vol;
	*/

/*    LIST_ENTRY              DirList;
    SHELL_FILE_ARG          *Arg;
	EFI_STATUS Status;
*/

	//
    // basiclly initialize the FileBuffer
    //
    CopyMem (&FileBuffer, &FileBufferConst, sizeof(FileBuffer));

/*    Status = BS->HandleProtocol (ImageHandleBackup,&LoadedImageProtocol,(VOID**)&LoadedImage);
    if (EFI_ERROR(Status)) {
        Print (L"Could not obtain Loaded Image Protocol\n");
        return EFI_LOAD_ERROR;
    }
    
    if (LoadedImage->DeviceHandle != NULL) {
        Status = BS->HandleProtocol (LoadedImage->DeviceHandle,&DevicePathProtocol,(VOID**)&DevicePath);
        if (EFI_ERROR(Status)) {
            Print (L"Could not obtain Device Path Protocol\n");
            return EFI_LOAD_ERROR;
        }

        Status = BS->HandleProtocol (LoadedImage->DeviceHandle,&FileSystemProtocol,(VOID**)&Vol);
        if (EFI_ERROR(Status)) {
            Print (L"Could not obtain File System Protocol\n");
            return EFI_LOAD_ERROR;
        }

    } else {
        CurDir = ShellCurDir(NULL);
        if (CurDir == NULL) {
            Print (L"Could not get current working directory\n");
            return EFI_LOAD_ERROR;
        }
        
        for (i=0; i < StrLen(CurDir) && CurDir[i] != ':'; i++);
        CurDir[i] = 0;
        DevicePath = (EFI_DEVICE_PATH *)ShellGetMap (CurDir);

        if (DevicePath == NULL) {
            Print (L"Could not open volume for the filesystem\n");
            return EFI_LOAD_ERROR;
        }

        Status = LibDevicePathToInterface (&FileSystemProtocol, DevicePath,(VOID**)&Vol);

        if (EFI_ERROR(Status)) {
            Print (L"Could not obtain File System Protocol\n");
            return EFI_LOAD_ERROR;
        }
    }

    Status = Vol->OpenVolume(Vol,&FileBuffer.CurrentDir);
    if (EFI_ERROR(Status)) {
        Print (L"Could not open volume for the filesystem\n");
        return EFI_LOAD_ERROR;
    }
*/

/*	InitializeListHead (&DirList);
	Status = ShellFileMetaArg(L".", &DirList);
	if ( EFI_ERROR ( Status ) ) {
		return EFI_LOAD_ERROR ;
	}

	Arg = CR(DirList.Flink, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);
    FileBuffer.CurrentDir = Arg->Handle;
	FreePool ( Arg -> FileName );
*/	

    //
    // default set FileName to NewFile.txt
    //
    FileBuffer.FileName = PoolPrint(L"NewFile.txt");
    if ( FileBuffer.FileName == NULL ) {
    	return EFI_LOAD_ERROR;
    }

    FileBuffer.ListHead = AllocatePool(sizeof(LIST_ENTRY));
    if ( FileBuffer.ListHead == NULL ) {
    	return EFI_LOAD_ERROR;
    }
    
    InitializeListHead(FileBuffer.ListHead);

    FileBuffer.DisplayPosition.Row = TEXT_START_ROW;
    FileBuffer.DisplayPosition.Column = TEXT_START_COLUMN;
    FileBuffer.LowVisibleRange.Row = TEXT_START_ROW;
    FileBuffer.LowVisibleRange.Column = TEXT_START_COLUMN;


    FileBufferNeedRefresh = FALSE;
	FileBufferOnlyLineNeedRefresh = FALSE;

	return EFI_SUCCESS; 
}



//
// Name:
//		FileBufferBackup -- Backup function for FileBuffer
//			Only a few fields need to be backuped. This is for making the file buffer refresh 
//			as few as possible.
// In:
//		VOID
// Out:
//		EFI_SUCCESS
//

EFI_STATUS 
FileBufferBackup () {
	FileBufferBackupVar.FileName = StrDuplicate (FileBuffer.FileName );
	FileBufferBackupVar.ModeInsert = FileBuffer.ModeInsert;
	FileBufferBackupVar.FileType = FileBuffer.FileType;
	FileBufferBackupVar.FilePosition = FileBuffer.FilePosition;
	FileBufferBackupVar.LowVisibleRange = FileBuffer.LowVisibleRange;
	FileBufferBackupVar.FileModified = FileBuffer.FileModified;
	FileBufferBackupVar.ReadOnly = FileBuffer.ReadOnly;

	return EFI_SUCCESS;
}




//
// Name:
//		FileBufferFreeLines -- Free all the lines in FileBuffer
// 			Fields affected:
//				Lines
//				CurrentLine
//				NumLines
//				ListHead
// In:
//		VOID
// Out:
//		EFI_SUCCESS
//

STATIC
EFI_STATUS
FileBufferFreeLines (
	VOID 
)
{	LIST_ENTRY *Link;
	EFI_EDITOR_LINE *Line;
	
  	if ( FileBuffer.Lines != NULL ) {
    	
    	Line = FileBuffer.Lines;
    	Link = &(Line->Link);
    	do {
    			Line = CR(Link,EFI_EDITOR_LINE,Link,EFI_EDITOR_LINE_LIST);
				Link = Link -> Flink;
    			LineFree ( Line );
    	} while ( Link != FileBuffer.ListHead );
    }

  	FileBuffer.Lines = NULL;
  	FileBuffer.CurrentLine = NULL;
  	FileBuffer.NumLines = 0;
  	

	FileBuffer.ListHead->Flink = FileBuffer.ListHead;
	FileBuffer.ListHead->Blink = FileBuffer.ListHead;

    return EFI_SUCCESS; 
}





//
// Name:
//		FileBufferCleanup -- Cleanup function for FileBuffer
// In:
//		VOID
// Out:
//		EFI_SUCCESS
//

STATIC
EFI_STATUS
FileBufferCleanup   (
    VOID
    )
{
  	EFI_STATUS Status;
  	
    EditorFreePool ( FileBuffer.FileName );
    
    //
    // free all the lines
    //
    Status = FileBufferFreeLines (); 
    
    EditorFreePool ( FileBuffer.ListHead );
    FileBuffer.ListHead = NULL;
    
	EditorFreePool ( FileBufferBackupVar.FileName );
    return Status;
    
}





//
// Name:
//		FileBufferPrintLine -- Print Line on Row
// In:
//		Line -- Line to print
// 		Row -- Row on screen ( begin from 1 )
// Out:
//		EFI_SUCCESS
//

STATIC
EFI_STATUS
FileBufferPrintLine ( EFI_EDITOR_LINE *Line, UINTN Row ) {

	CHAR16 *Buffer;
	CHAR16 Backup;
	UINTN Limit;
		
	//
	// the line is left to current screen. 
	// it's invisible, so no need to print it out
	//
	if ( Line -> Size < FileBuffer.LowVisibleRange.Column ) {
		return EFI_SUCCESS;
	}
	
	//
	// print start from correct character
	//
	Buffer = Line -> Buffer + FileBuffer.LowVisibleRange.Column - 1;
	
	Limit = Line -> Size - FileBuffer.LowVisibleRange.Column + 1 ;
	
	//
	// it's longer than current screen
	//
	if ( Limit > NUM_TEXT_COLUMNS ) {
		Backup = Buffer[NUM_TEXT_COLUMNS];
		
		//
		// cut Buffer to fit current screen display
		//
		Buffer[NUM_TEXT_COLUMNS] = '\0';
	}

	PrintAt ( TEXT_START_COLUMN - 1 , Row - 1 , L"%s", Buffer );
	
	if ( Limit > NUM_TEXT_COLUMNS ) {
		//
		// restore the old buffer
		//
		Buffer[NUM_TEXT_COLUMNS] = Backup;
	}

	return EFI_SUCCESS;
}




//
// Name:
//		FileBufferRestorePosition -- Set cursor position according to FileBuffer.DisplayPosition.
// In:
//		VOID
// Out:
//		EFI_SUCCESS
//

STATIC
EFI_STATUS
FileBufferRestorePosition (
    VOID
    )
{
    Out->SetCursorPosition (Out,FileBuffer.DisplayPosition.Column - 1 ,FileBuffer.DisplayPosition.Row - 1 );

    return EFI_SUCCESS;
}





//
// Name:
//		FileBufferRefresh -- Refresh function for FileBuffer
// In:
//		VOID
// Out:
//		EFI_SUCCESS
// 		EFI_LOAD_ERROR
//

STATIC
EFI_STATUS
FileBufferRefresh (
    VOID
    )
{
    LIST_ENTRY          *Link;
    EFI_EDITOR_LINE		*Line;
    UINTN               Row;
	
	//
	// if it's the first time after editor launch, so should refresh
	//
	if ( EditorFirst == FALSE )
	
		//
		// no definite required refresh
		// and file position displayed on screen has not been changed
		//
		if ( FileBufferNeedRefresh == FALSE &&
		     FileBufferOnlyLineNeedRefresh == FALSE && 
			FileBufferBackupVar.LowVisibleRange.Row == FileBuffer.LowVisibleRange.Row && 
			FileBufferBackupVar.LowVisibleRange.Column == FileBuffer.LowVisibleRange.Column ) {
			
			FileBufferRestorePosition();
			return EFI_SUCCESS;
		}


	//
	// only need to refresh current line
	//
	if ( FileBufferOnlyLineNeedRefresh == TRUE 	&&
		 FileBufferBackupVar.LowVisibleRange.Row == FileBuffer.LowVisibleRange.Row && 
 		 FileBufferBackupVar.LowVisibleRange.Column == FileBuffer.LowVisibleRange.Column ) {
		
			EditorClearLine ( FileBuffer.DisplayPosition.Row );
			FileBufferPrintLine ( FileBuffer.CurrentLine, FileBuffer.DisplayPosition.Row );
	} else {
		//
		// the whole edit area need refresh
		//
		
		//
		// clear the whole edit area
		//
		for ( Row = TEXT_START_ROW; Row <= TEXT_END_ROW; Row++ ) {
    		EditorClearLine ( Row );
		}
    
    	//
    	// no line
    	//
		if ( FileBuffer.Lines == NULL ) {
    		FileBufferRestorePosition();
			return EFI_SUCCESS;
		}
    
    	
		//
		// get the first line that will be displayed    
		//
		Line = MoveLine ( FileBuffer.LowVisibleRange.Row - FileBuffer.FilePosition.Row);
		if ( Line == NULL ) {
			return EFI_LOAD_ERROR;
		}

		Link = &(Line -> Link);
		Row = TEXT_START_ROW;
   		do {
   			Line = CR(Link,EFI_EDITOR_LINE,Link,EFI_EDITOR_LINE_LIST);
			
			//
			// print line at row
			//
			FileBufferPrintLine ( Line , Row);
			
			Link = Link -> Flink;
   			Row++;
   		} while ( Link != FileBuffer.ListHead && Row <= TEXT_END_ROW);  // while not file end and not screen full
	}
    

    FileBufferRestorePosition();

	FileBufferNeedRefresh = FALSE;
	FileBufferOnlyLineNeedRefresh = FALSE;

    return EFI_SUCCESS; 
}






//
// Name:
//		FileBufferCreateLine -- Create a new line and append it to the line list
//			Fields affected:
//				NumLines
//				Lines
// In:
//		VOID
// Out:
//	 	NULL -- create line failed
//		Not NULL -- the line created
//

STATIC
EFI_EDITOR_LINE*
FileBufferCreateLine  (
    VOID
    )
{
    EFI_EDITOR_LINE     *Line;

    Line = AllocatePool (sizeof(EFI_EDITOR_LINE));
    if ( Line == NULL ) {
        return NULL;
    }
    
    Line->Signature = EFI_EDITOR_LINE_LIST;
    Line->Size = 0;
	Line -> TotalSize = 0;
    Line->Buffer = PoolPrint (L"\0");
    
    FileBuffer.NumLines++;

    InsertTailList(FileBuffer.ListHead,&Line->Link);

	if ( FileBuffer.Lines == NULL ) {
		FileBuffer.Lines = 
			CR(FileBuffer.ListHead->Flink,EFI_EDITOR_LINE,Link,EFI_EDITOR_LINE_LIST);
	}

    return Line;
}






//
// Name:
//		FileBufferSetFileName -- Set FileName field in FileBuffer
// In:
//		Str -- File name to set
// Out:
//		EFI_SUCCESS
//

STATIC
EFI_STATUS
FileBufferSetFileName (
	IN CHAR16 *Str )
{
	UINTN Size, i;

	//
	// free the old file name
	//
	EditorFreePool ( FileBuffer.FileName );
	
	Size = StrLen ( Str);

	FileBuffer.FileName = AllocatePool ( 2 * (Size + 1 ));
	if ( FileBuffer.FileName == NULL ) 
		return EFI_OUT_OF_RESOURCES;

	for ( i = 0 ; i < Size; i++) 
		FileBuffer.FileName[i] = Str[i];
	
	FileBuffer.FileName[Size] = L'\0';
	
	return EFI_SUCCESS;
}





//
// Name:
//		FileBufferFree -- Function called when load a new file in. It will free all the old lines
//							and set FileModified field to FALSE
// In:
//		VOID
// Out:
//		EFI_SUCCESS
//

STATIC 
EFI_STATUS
FileBufferFree () 
{
	FileBufferFreeLines ();
	FileBuffer.FileModified = FALSE;

	return EFI_SUCCESS;
}








//
// Name:
//		FileBufferGetFileInfo -- Get this file's information
// In:
//		Handle -- in NT32 mode Directory handle, in other mode File Handle
// 		EFI_FILE_INFO ** -- parameter to pass file information out
// Out:
//		EFI_SUCCESS
//		EFI_OUT_OF_RESOURCES
//		EFI_LOAD_ERROR
//

EFI_STATUS
FileBufferGetFileInfo ( 
	EFI_FILE_HANDLE Handle,
	CHAR16			*FileName,
	EFI_FILE_INFO ** InfoOut
) 
{

#ifndef EFI_NT_EMULATOR
	VOID * Info;
	UINTN Size;
	EFI_STATUS Status;

	Size = SIZE_OF_EFI_FILE_INFO + 1024;
    Info = AllocatePool (Size);
    if (!Info) {
        return EFI_OUT_OF_RESOURCES;
    }

	//
	// get file information
	//
	Status = Handle -> GetInfo ( Handle,&GenericFileInfo, &Size, Info );
	if ( EFI_ERROR ( Status ) ) {
		return EFI_LOAD_ERROR ;
	}

	*InfoOut = ( EFI_FILE_INFO * ) Info;

	return EFI_SUCCESS;

#else

	VOID * Info;
	EFI_FILE_INFO *File;
	UINTN Size;
	EFI_STATUS Status;

// 
 // because in nt32 mode, the above code can't get file info correctly
 // so have to loop in the directory to find this file and get file info
 // (:-((((((((( 
 //
	UINTN bs;
	
	Size = SIZE_OF_EFI_FILE_INFO + 1024;
    Info = AllocatePool (Size);
    if (!Info) {
        return EFI_OUT_OF_RESOURCES;
    }
    
    Handle->SetPosition (Handle, 0);

    for (; ;) {

       bs = Size;
       Status = Handle->Read (Handle, &bs, Info);

       if (EFI_ERROR(Status)) {
            return EFI_LOAD_ERROR;
        }

	    if (bs == 0) {
             break;
        }

		File = ( EFI_FILE_INFO * ) Info;     
			
		if ( StriCmp ( File->FileName, FileName ) ==  0 ) {
			*InfoOut = Info;
			return EFI_SUCCESS;
		}

	}
    
    return EFI_NOT_FOUND;

#endif

}







//
// Name:
//		FileBufferRead -- Read a file from disk into FileBuffer
// In:
//		FileName -- filename to read
//		Recover -- if is for recover, no infomration print
// Out:
//		EFI_SUCCESS
//		EFI_LOAD_ERROR
//		EFI_OUT_OF_RESOURCES
//

STATIC
EFI_STATUS
FileBufferRead ( 
    CHAR16 *FileName,
	BOOLEAN Recover
    )
{
    EFI_EDITOR_LINE     *Line = NULL;
    UINTN               i = 0;
    UINTN 				j;
    UINTN               LineSize;
    VOID                *Buffer;
    CHAR16              *UnicodeBuffer;
    CHAR8               *AsciiBuffer;
    UINTN               FileSize ;
    EFI_FILE_HANDLE 	Handle;
	BOOLEAN 			CreateFile = FALSE;
	UINT8   Marker1 = 0xff,Marker2 = 0xfe;
    EFI_STATUS 			Status;
	UINTN				LineSizeBackup;
	BOOLEAN				FirstLine;
	EE_NEWLINE_TYPE Type;

    LIST_ENTRY              DirList;
    SHELL_FILE_ARG          *Arg;
    
    
    EFI_FILE_INFO		*Info;
	CHAR16 *FileNameTmp;
	

//
// in this function, when you return error ( except EFI_OUT_OF_RESOURCES )
// you should set status string
// since this funtion maybe called before the editorhandleinput loop
// so any error will cause editor return
// so if you want to print the error status
// you should set the status string
//

	FileNameTmp = PoolPrint(L"%s", FileName );
	if ( FileNameTmp == NULL )
		return EFI_OUT_OF_RESOURCES;


	// GET CURRENT DIR HANDLE
	InitializeListHead (&DirList);

	// after that filename changed to path
	Status = ShellFileMetaArg(FileNameTmp, &DirList);
	FreePool ( FileNameTmp );
	
	if ( EFI_ERROR ( Status ) ) {
		MainStatusBarSetStatusString (L"Disk Error");
		return EFI_LOAD_ERROR ;
	}



	if ( DirList.Flink == &DirList ) {
		MainStatusBarSetStatusString (L"Disk Error");
		return EFI_LOAD_ERROR ;
	}

	Arg = CR(DirList.Flink, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);

    // since when Status is returned correct, not everything is correct, 
	// so  I have to add these check (-:(((((
    
	if ( Arg == NULL ) {
		MainStatusBarSetStatusString (L"Disk Error");
		ShellFreeFileList (&DirList);
		return EFI_LOAD_ERROR ;
	}

	if ( Arg->Parent == NULL ) {
		MainStatusBarSetStatusString (L"Disk Error");
		ShellFreeFileList (&DirList);
		return EFI_LOAD_ERROR ;
	}

	
	Info = Arg->Info;
	   
	
	//
	// check whether this file exists
	//
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
			MainStatusBarSetStatusString (L"Disk Error");
			ShellFreeFileList (&DirList);
			return EFI_LOAD_ERROR;
		}


		if ( Info->Attribute & EFI_FILE_DIRECTORY ) {
			MainStatusBarSetStatusString (L"Directory Can Not Be Edited");
			ShellFreeFileList (&DirList);
			return EFI_LOAD_ERROR;
		}
			
		
		// check if read only
		if ( Info->Attribute & EFI_FILE_READ_ONLY ) {
			FileBuffer.ReadOnly = TRUE;
		} else {
			FileBuffer.ReadOnly = FALSE;
		}

		//get file size
		FileSize = ( UINTN ) Info->FileSize;


	} else {
	
			//
			// file not exists, so set CreateFile to TRUE
			//
			CreateFile = TRUE;
			FileBuffer.ReadOnly = FALSE;

			// all the check ends
			// so now begin to set file name, free lines
			if ( StrCmp ( FileName, FileBuffer.FileName ) != 0 )
				FileBufferSetFileName  ( FileName );
	
			//
			// free the old lines
			//
			FileBufferFree ();

	}

    //
    // file exists
    //
    if ( CreateFile == FALSE ) {
        
        //
        // allocate buffer to read file
        //
	    Buffer = AllocatePool(FileSize);
    	if ( Buffer == NULL ) {
			ShellFreeFileList (&DirList);
        	return EFI_OUT_OF_RESOURCES;
    	}

		//
		// read file into Buffer
		//
    	Status = Handle->Read(Handle,&FileSize,Buffer);
    	if ( EFI_ERROR ( Status )) {
        	MainStatusBarSetStatusString(L"Read File Failed");
        	EditorFreePool ( Buffer );
			ShellFreeFileList (&DirList);
        	return EFI_LOAD_ERROR;
    	}
    	
    	
    	//
    	// nothing in this file
    	//
    	if (FileSize == 0) {
        	EditorFreePool (Buffer);
        	
        	//
        	// since has no head, so only can be an ascii file
        	//
        	FileBuffer.FileType = ASCII_FILE;
        	FileBuffer.NewLineType = USE_LF;
        	
						
        	goto Done;
    	}

 
    	AsciiBuffer = Buffer;
    	
    	
    	if ( FileSize < 2 ) { 
    		//
    		// size < unicode file header, so only can be ascii file
    		//
    		FileBuffer.FileType = ASCII_FILE;
    	} else {
    		
    		//
    		// unicode file
    		//
    		if (AsciiBuffer[0] == 0xff && AsciiBuffer[1] == 0xfe) {
    		
    			//
    			// unicode file's size should be even
    			//
        		if ( ( FileSize % 2 ) != 0 ) {
        			MainStatusBarSetStatusString(L"File Format Wrong");
	        		EditorFreePool (Buffer);
					ShellFreeFileList (&DirList);
        			return EFI_LOAD_ERROR;
    			}
        		
        		FileSize /= 2;
        	
        		FileBuffer.FileType = UNICODE_FILE;
        		UnicodeBuffer = Buffer;
        	
        		//
        		// pass this 0xff and 0xfe
        		//
        		++UnicodeBuffer;
        		FileSize--;
    		} else {
        		FileBuffer.FileType = ASCII_FILE;
    		}   // end of AsciiBuffer ==
    	} // end of FileSize < 2


	// all the check ends
	// so now begin to set file name, free lines
	if ( StrCmp ( FileName, FileBuffer.FileName ) != 0 )
		FileBufferSetFileName  ( FileName );
	
	//
	// free the old lines
	//
	FileBufferFree ();


    FirstLine = TRUE;
    
    //
    // parse file content line by line
    //
    for (i = 0; i < FileSize; i++) {
        for (LineSize = i; LineSize < FileSize; LineSize++) {
            if (FileBuffer.FileType == ASCII_FILE) {
                if (AsciiBuffer[LineSize] == CHAR_CR)   {
                    Type = USE_CRLF;
                    break;
                } else if (AsciiBuffer[LineSize] == CHAR_LF)    {
                    Type = USE_LF;
                    break;
             	}
            } else {
                if (UnicodeBuffer[LineSize] == CHAR_CR) {
                    Type = USE_CRLF;
                    break;
                } else if (UnicodeBuffer[LineSize] == CHAR_LF)  {
                    Type = USE_LF;
                    break;
                }
            }   // endif == ASCII
        }   // end of for LineSize

		//
		// this is the first line, so just set Type to NewLineType
		//
		if ( FirstLine ) {
			FileBuffer.NewLineType = Type;
		} else {
			
			// 
			//not the first line, so this line's type should be the same as previous lines
			//
			if ( Type != FileBuffer.NewLineType ) {
				// ignore it
//				return EFI_LOAD_ERROR;
			}
		}
			
		FirstLine = FALSE;
		
		LineSizeBackup = LineSize;

        
        // 
        // create a new line
        //
        Line = FileBufferCreateLine ();
        if (Line == NULL) {
            return EFI_OUT_OF_RESOURCES;
        }

        //
        // calculate file length
        //
        LineSize -= i;

        //
        // Unicode and one '\0'
        //
        Line->Buffer = AllocateZeroPool(LineSize*2+2);
        if (Line->Buffer == NULL) {
            RemoveEntryList(&Line->Link);
            return EFI_OUT_OF_RESOURCES;
        }
        
        //
        // copy this line to Line->Buffer
        //
        for (j = 0; j < LineSize; j++) {
            if (FileBuffer.FileType == ASCII_FILE) {
                Line->Buffer[j] = (CHAR16)AsciiBuffer[i];
            } else {
                Line->Buffer[j] = UnicodeBuffer[i];
            }
        
            i++;
       	}
    
		// i now points to where CHAR_CR or CHAR_LF;

    	Line->Buffer[LineSize] = 0;
    
            
        Line->Size = LineSize;
		Line -> TotalSize = LineSize;
		
		
        if (FileBuffer.NewLineType == USE_CRLF) {
            
			// should have a CHAR_LF, but acutually run into file end
			if ( i + 1  >= FileSize ) {
				// ignore it
//				return EFI_LOAD_ERROR;
			}

            // 
            //the next character should be CHAR_LF
            //
            if (FileBuffer.FileType == ASCII_FILE) {
                if ( AsciiBuffer[i + 1] != CHAR_LF ) {
                	// ignore it
//                	return EFI_LOAD_ERROR;
                }
            } else {
                if ( UnicodeBuffer[i + 1] != CHAR_LF ) {
					// ignore it
//                	return EFI_LOAD_ERROR;
                }
            }
            
            //
            // skip CHAR_CR, the CHAR_LF will be skipped by i++ in the loop
            //
            ++i;
        }
                
        

		// 
		// last character is a return, SO create a new line
		//
		if ( ( FileBuffer.NewLineType == USE_CRLF && LineSizeBackup == FileSize -2  ) || 
				( FileBuffer.NewLineType == USE_LF && LineSizeBackup == FileSize -1  ) 
				) {
				Line = FileBufferCreateLine ();
				if (Line == NULL) {
					return EFI_OUT_OF_RESOURCES;
				} 
		} // end of if

    }   // end of i

    EditorFreePool (Buffer);

    }   //end of if CreateFile

Done:

	
	FileBuffer.DisplayPosition.Row = TEXT_START_ROW;
    FileBuffer.DisplayPosition.Column = TEXT_START_COLUMN;
    FileBuffer.LowVisibleRange.Row = 1;
    FileBuffer.LowVisibleRange.Column = 1;
    FileBuffer.FilePosition.Row = 1;
    FileBuffer.FilePosition.Column = 1;

    if ( !Recover ) {
		UnicodeBuffer = PoolPrint(L"%d Lines Read",FileBuffer.NumLines);
		if ( UnicodeBuffer == NULL ) {
    		return EFI_OUT_OF_RESOURCES;
		}
    
		MainStatusBarSetStatusString(UnicodeBuffer);
		EditorFreePool ( UnicodeBuffer );
	}
    
	// 
	// has line
	//
	if ( FileBuffer.Lines != 0  ) {
		FileBuffer.CurrentLine = 
			CR(FileBuffer.ListHead->Flink,EFI_EDITOR_LINE,Link,EFI_EDITOR_LINE_LIST);
	} else { 
		// 
		// create a dummy line
		//
		Line = FileBufferCreateLine ();
		if ( Line == NULL ) {
			return EFI_OUT_OF_RESOURCES ;
		}
		
		FileBuffer.CurrentLine = Line;
	}
		
	FileBuffer.FileModified = FALSE;
	FileBufferNeedRefresh = TRUE;
	FileBufferOnlyLineNeedRefresh = FALSE;

	if ( CreateFile )  
		// for new file , set modified to TRUE
		FileBuffer.FileModified = TRUE;

	ShellFreeFileList (&DirList);

    return EFI_SUCCESS;
}





//
// Name:
//		GetNewLine -- According to FileBuffer.NewLineType & FileBuffer.FileType, get the return buffer and size
// In:
//		Buffer -- pass the return buffer out
//		Size -- pass the return buffer size out
// Out:
//		VOID
//

STATIC
VOID
GetNewLine  (
    CHAR8   *Buffer,
    UINT8   *Size
    )
{
    UINT8           size = 0;

    if (MainEditor.FileBuffer->NewLineType == USE_CRLF) {
        //
        // CHAR_CR
        //
        Buffer[size] = 0x0d;
        size++;
        if (MainEditor.FileBuffer->FileType == UNICODE_FILE) {
            
            //
            // UNICODE FILE , CHAR_CR IS 0x0d, 0x00 ???
            //
            Buffer[size] = 0x00;
            size++;
        }
    }

    Buffer[size] = 0x0a;
    size++;
    if (MainEditor.FileBuffer->FileType == UNICODE_FILE) {
        
        // 
        // UNICODE FILE, CHAR_LF IS 0x0a, 0x00
        //
        Buffer[size] = 0x00;
        size++;
    }
    
	*Size = size;
}






//
// Name:
//		FileBufferSave -- Save lines in FileBuffer to disk
// In:
//		VOID
// Out:
//		EFI_SUCCESS
//		EFI_LOAD_ERROR
//		EFI_OUT_OF_RESOURCES
//		

STATIC
EFI_STATUS
FileBufferSave ( 
    CHAR16 *FileName
    )
{

    LIST_ENTRY      *Link;
    EFI_EDITOR_LINE *Line;
    CHAR16          *Str;
    VOID            *Buffer;

    EFI_STATUS      Status;
    UINTN           Length = 0;
    UINTN           NumLines;
    CHAR8           NewLineBuffer[4];
    UINT8           NewLineSize;

	EFI_FILE_HANDLE		FileHandle;

    LIST_ENTRY              DirList;
    SHELL_FILE_ARG          *Arg;
    EFI_FILE_INFO			*Info;
	CHAR16 *FileNameTmp;	

	UINT64 Attribute;

	
	FileNameTmp = PoolPrint(L"%s", FileName );
	if ( FileNameTmp == NULL )
		return EFI_OUT_OF_RESOURCES;


	// 
	//if is the old file
	//

	if ( StrCmp ( FileName, FileBuffer.FileName ) == 0 ) {

		// file has not been modified
		if ( FileBuffer.FileModified == FALSE ) {
			return EFI_SUCCESS;
		}


		//
		// if file is readonly, set error
		//
		if ( FileBuffer.ReadOnly == TRUE ) {
			MainStatusBarSetStatusString ( L"Read Only File Can Not Be Saved");
			return EFI_SUCCESS;
		}
	}

    // now it's cases that you should write disk 

	// GET CURRENT DIR HANDLE
	InitializeListHead (&DirList);
	Status = ShellFileMetaArg(FileNameTmp, &DirList);
	FreePool ( FileNameTmp );
	
	if ( EFI_ERROR ( Status ) ) {
		return EFI_LOAD_ERROR ;
	}


	if ( DirList.Flink == &DirList ) {
		return EFI_LOAD_ERROR ;
	}

	Arg = CR(DirList.Flink, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);
    
    // since when Status is returned correct, not everything is correct, 
	// so  I have to add these check (-:(((((
	if ( Arg == NULL ) {
		ShellFreeFileList (&DirList);
		return EFI_LOAD_ERROR ;
	}

	if ( Arg->Parent == NULL ) {
		ShellFreeFileList (&DirList);
		return EFI_LOAD_ERROR ;
	}
	
	
	Info = Arg -> Info;
	    
	Attribute = 0;

	//
	// open current file
	//
	if ( !Arg->Status ) {
		if ( ! Arg->Handle ) 
	        Status = Arg->Parent->Open (
                    Arg->Parent,
                    &Arg->Handle,
                    Arg->FileName,
                    EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE,
                    0
                    );

		FileHandle = Arg->Handle;

#ifndef EFI_NT_EMULATOR		
		Status = FileBufferGetFileInfo ( FileHandle, Arg->FileName, &Info );
#else
		Status = FileBufferGetFileInfo ( Arg->Parent, Arg->FileName, &Info );
#endif
		if ( EFI_ERROR ( Status ) ) {
			ShellFreeFileList (&DirList);
			return EFI_LOAD_ERROR;
		}
		
		if ( Info->Attribute & EFI_FILE_DIRECTORY ) {
			MainStatusBarSetStatusString (L"Directory Can Not Be Saved");
			ShellFreeFileList (&DirList);
			return EFI_LOAD_ERROR;
		}

		if ( Info->Attribute & EFI_FILE_READ_ONLY ) 
			Attribute = Info->Attribute - EFI_FILE_READ_ONLY;
		else 
			Attribute = Info ->Attribute;

    	// 
    	//if file exits, so delete it
    	//
        Status = FileHandle->Delete (FileHandle);
        Arg->Handle = NULL;
        if (EFI_ERROR(Status)) {
          		ShellFreeFileList (&DirList);

// because in nt32 mode, when accroding file systems, it will generate exception
		  // so comment it

		  //          FileHandle -> Close ( FileHandle );

          MainStatusBarSetStatusString (L"Write File Failed");
          return EFI_LOAD_ERROR;
        }
    }

	// 
	//create this file
	//
    Status = Arg->Parent->Open(Arg->Parent,&Arg->Handle,Arg->FileName,FILE_CREATE, Attribute);
    if (EFI_ERROR(Status)) {
   			ShellFreeFileList (&DirList);

          MainStatusBarSetStatusString (L"Create File Failed");

       	  return EFI_LOAD_ERROR;
    }

	FileHandle = Arg->Handle;
	
    GetNewLine(NewLineBuffer,&NewLineSize);


	//
	// if file is unicode file, write unicode header to it.
	//
    if (FileBuffer.FileType == UNICODE_FILE) {
        UINT8   Marker1 = 0xff,Marker2 = 0xfe;
        Length = 1;
        Status = FileHandle->Write(FileHandle,&Length,&Marker1);
        if ( EFI_ERROR ( Status ) ) {
			FileHandle->Delete (FileHandle);
			ShellFreeFileList (&DirList);

        	return EFI_LOAD_ERROR ;
        }
        
        Length = 1;
        FileHandle->Write(FileHandle,&Length,&Marker2);
        if ( EFI_ERROR ( Status ) ) {
			FileHandle->Delete (FileHandle);
			ShellFreeFileList (&DirList);

        	return EFI_LOAD_ERROR ;
        }
    }


	// 
	//write all the lines back to disk
	//
	NumLines = 0;
    for (Link = FileBuffer.ListHead->Flink;
            Link != FileBuffer.ListHead; Link = Link->Flink) {
        Line = CR(Link,EFI_EDITOR_LINE,Link,EFI_EDITOR_LINE_LIST);

        if ( Line -> Buffer != NULL && Line -> Size != 0 ) {
			if (FileBuffer.FileType == ASCII_FILE) {
				Buffer = (VOID*)AllocatePool(Line->Size);
				if ( Buffer == NULL ) {
					FileHandle->Delete (FileHandle);
					ShellFreeFileList (&DirList);

					return EFI_OUT_OF_RESOURCES;
				}

				UnicodeToAscii(Line->Buffer,Line->Size,(CHAR8*)Buffer);
				Length = Line->Size ;
			} else {
				Length = (Line->Size*2) ;
				Buffer = (VOID*)PoolPrint(L"%s",Line->Buffer);
				if ( Buffer == NULL ) {
					FileHandle->Delete (FileHandle);
					ShellFreeFileList (&DirList);

					return EFI_OUT_OF_RESOURCES;
				}

			}  // end if ASCII_FILE

			Status = FileHandle->Write(FileHandle,&Length,Buffer);
			FreePool(Buffer);
			if (EFI_ERROR(Status)) {
				FileHandle->Delete (FileHandle);
				ShellFreeFileList (&DirList);

				return EFI_LOAD_ERROR;
			}
		} // end of if Line -> Buffer != NULL && Line -> Size != 0

       //
        // if not the last line , write return buffer to disk
        //
		if ( Link->Flink != FileBuffer.ListHead ) {
			Length = NewLineSize;
			Status = FileHandle->Write(FileHandle,&Length,NewLineBuffer);
			if (EFI_ERROR(Status)) {
				FileHandle->Delete (FileHandle);
				ShellFreeFileList (&DirList);

				return EFI_LOAD_ERROR;
			}
		}

        NumLines++;
    }

    FileBuffer.FileModified = FALSE;

	//
	// set status string
	//
    Str = PoolPrint(L"%d Lines Wrote",NumLines);
    MainStatusBarSetStatusString(Str);
	EditorFreePool ( Str );
   
	// now everything is ready , you can set the new file name to filebuffer
	if ( StrCmp ( FileName, FileBuffer.FileName ) != 0 ) {
		// not the same
		FileBufferSetFileName ( FileName );
		if ( FileBuffer.FileName == NULL ) {
			FileHandle -> Delete ( FileHandle );
			ShellFreeFileList (&DirList);

			return EFI_OUT_OF_RESOURCES;
		}
	}
			
	FileBuffer.ReadOnly = FALSE;


	ShellFreeFileList (&DirList);
    
    return  EFI_SUCCESS;
}





//
// Name:
//		FileBufferHandleInput -- Dispatch input to different handler
// In:
//		Key -- input key
// 				the keys can be:
//						ASCII KEY
//						Backspace/Delete
//						Return
//						Direction key: up/down/left/right/pgup/pgdn
//						Home/End
//						INS
// Out:
//		EFI_SUCCESS
//		EFI_LOAD_ERROR
//		EFI_OUT_OF_RESOURCES
//		

STATIC
EFI_STATUS
FileBufferHandleInput (
    IN  EFI_INPUT_KEY*  Key
    )
{ 
	EFI_STATUS Status;

	Status = EFI_SUCCESS;
	
    switch (Key->ScanCode) {
  	
    case    SCAN_CODE_NULL:
        if ( FileBuffer.ReadOnly == FALSE )
			Status = FileBufferDoCharInput (Key->UnicodeChar);
		else 
			Status = MainStatusBarSetStatusString (L"Read Only File Can Not Be Modified");
        break;
    case    SCAN_CODE_UP:
        Status = FileBufferScrollUp();
        break;
    case    SCAN_CODE_DOWN:
        Status = FileBufferScrollDown();
        break;
    case    SCAN_CODE_RIGHT:
        Status = FileBufferScrollRight();
        break;
    case    SCAN_CODE_LEFT:
        Status = FileBufferScrollLeft();
        break;
    case    SCAN_CODE_PGUP:
        Status = FileBufferPageUp();
        break;
    case    SCAN_CODE_PGDN:
        Status = FileBufferPageDown();
        break;
    case    SCAN_CODE_DEL:
        if ( FileBuffer.ReadOnly == FALSE )
			Status = FileBufferDoDelete();
		else 
			Status = MainStatusBarSetStatusString (L"Read Only File Can Not Be Modified");
        break;
    case    SCAN_CODE_HOME:
        Status = FileBufferHome();
        break;
    case    SCAN_CODE_END:
        Status = FileBufferEnd();
        break;
    case    SCAN_CODE_INS:
        Status = FileBufferChangeMode();
        break;
    default:
    	Status = MainStatusBarSetStatusString (L"Unknown Command");
        break;
    }

    return Status; 
}







//
// Name:
//		FileBufferDoCharInput -- Ascii key + Backspace + return
// In:
//		Char -- input char
// Out:
//		EFI_SUCCESS
//		EFI_LOAD_ERROR
//		EFI_OUT_OF_RESOURCES
//		

STATIC
EFI_STATUS
FileBufferDoCharInput (
    IN  CHAR16  Char
    )
{
	EFI_STATUS Status;
	
	Status = EFI_SUCCESS;
	
    switch (Char) {
    case 0:
        break;
    
    case 0x08:
        Status = FileBufferDoBackspace();
        break;   
    
    case 0x0a:
    case 0x0d:     
    //
    // 0x0a, 0x0d all be thought as return
    //
        Status = FileBufferDoReturn();
        break;
    
    default:
      //
        // DEAL WITH ASCII CHAR, filter out thing like ctrol+f
        //
		if ( Char > 127 || Char < 32 )
			Status = MainStatusBarSetStatusString (L"Unknown Command");
		else
			Status = FileBufferAddChar ( Char );
        break;
    }
    
    return Status; 
}










//
// Name:
//		FileBufferAddChar -- Add character
// In:
//		Char -- input char
// Out:
//		EFI_SUCCESS
//		

STATIC
EFI_STATUS
FileBufferAddChar ( 
	IN  CHAR16  Char
)
{ 
 EFI_EDITOR_LINE *Line;
 UINTN           FilePos;
 
 Line = FileBuffer.CurrentLine;
 
//
// only needs to refresh current line 
//
 FileBufferOnlyLineNeedRefresh = TRUE;

//
// when is insert mode, or cursor is at end of this line, so insert this characte
// or replace the character.
//
   FilePos = FileBuffer.FilePosition.Column - 1;
   if (FileBuffer.ModeInsert || FilePos+1 > Line->Size) {
	   LineStrInsert (Line,Char,FilePos,Line->Size+1);
    } else {
         Line->Buffer[FilePos] = Char;
    }
    
//
// move cursor to right 
//
 FileBufferScrollRight();

 if (!FileBuffer.FileModified) {
      FileBuffer.FileModified = TRUE;
 }

 return EFI_SUCCESS;
}








//
// Name:
//		InCurrentScreen -- Check user specified FileRow and FileCol is in current screen
// In:
//		FileRow -- Row of file position ( start from 1 )
//		FileCol -- Column of file position ( start from 1 )		
// Out:
//		TRUE
//		FALSE
//		

BOOLEAN
InCurrentScreen (
	IN	UINTN FileRow,
	IN 	UINTN FileCol
) 
{	
	if ( FileRow >= FileBuffer.LowVisibleRange.Row &&
		 FileRow <= FileBuffer.LowVisibleRange.Row + NUM_TEXT_ROWS - 1 &&
		 FileCol >= FileBuffer.LowVisibleRange.Column &&
		 FileCol <= FileBuffer.LowVisibleRange.Column + NUM_TEXT_COLUMNS - 1

		 )
		return TRUE;
		
	return FALSE;
}








//
// Name:
//		AboveCurrentScreen -- Check user specified FileRow is above current screen
// In:
//		FileRow -- Row of file position ( start from 1 )
// Out:
//		TRUE
//		FALSE
//		

BOOLEAN
AboveCurrentScreen (
	IN	UINTN FileRow 
)
{
	if ( FileRow < FileBuffer.LowVisibleRange.Row )
		return TRUE;
	
	return FALSE;
}
	
	
	
	
	
	
	
//
// Name:
//		UnderCurrentScreen -- Check user specified FileRow is under current screen
// In:
//		FileRow -- Row of file position ( start from 1 )
// Out:
//		TRUE
//		FALSE
//		
 
BOOLEAN
UnderCurrentScreen (
	IN	UINTN FileRow 
)
{
	if ( FileRow > FileBuffer.LowVisibleRange.Row + NUM_TEXT_ROWS - 1 )
		return TRUE;
	
	return FALSE;
}






//
// Name:
//		LeftCurrentScreen -- Check user specified FileCol is left to current screen
// In:
//		FileCol -- Column of file position ( start from 1 )		
// Out:
//		TRUE
//		FALSE
//		

BOOLEAN
LeftCurrentScreen (
	IN	UINTN FileCol
)
{
	if ( FileCol < FileBuffer.LowVisibleRange.Column )
		return TRUE;
	
	return FALSE;
}
	
 





//
// Name:
//		RightCurrentScreen -- Check user specified FileCol is right to current screen
// In:
//		FileCol -- Column of file position ( start from 1 )		
// Out:
//		TRUE
//		FALSE
//		

BOOLEAN
RightCurrentScreen (
	IN	UINTN FileCol
)
{
	if ( FileCol > FileBuffer.LowVisibleRange.Column + NUM_TEXT_COLUMNS - 1 )
		return TRUE;
	
	return FALSE;
}


		





//
// Name:
//		FileBufferMovePosition -- According to cursor's file position, adjust screen display
// In:
//		NewFilePosRow -- Row of file position ( start from 1 )
//		NewFilePosCol -- Column of file position ( start from 1 )		
// Out:
// 		VOID
//		

VOID
FileBufferMovePosition ( 
	IN UINTN NewFilePosRow,
	IN UINTN NewFilePosCol
)
{
	INTN RowGap, ColGap;
	UINTN Abs;
	BOOLEAN Above, Under;
	BOOLEAN	Right, Left;
	
	//
	// CALCULATE gap between current file position and new file position
	//
	RowGap = NewFilePosRow - FileBuffer.FilePosition.Row;
	ColGap = NewFilePosCol - FileBuffer.FilePosition.Column;

	Under = UnderCurrentScreen ( NewFilePosRow );
	Above = AboveCurrentScreen ( NewFilePosRow );
	//
	// if is below current screen
	//
	if ( Under ) {
		// 
		//display row will be unchanged
		//
		FileBuffer.FilePosition.Row = NewFilePosRow;
	} else {
			if ( Above) {
				//
				// has engough above line, so display row unchanged
				// not has enough above lines, so the first line is at the first display line
				//
				if ( NewFilePosRow < ( FileBuffer.DisplayPosition.Row - TEXT_START_ROW + 1 ) ) {
					FileBuffer.DisplayPosition.Row = NewFilePosRow + TEXT_START_ROW - 1 ;
				}

				FileBuffer.FilePosition.Row = NewFilePosRow;
			} else {
				//
				// in current screen 
				//
				FileBuffer.FilePosition.Row = NewFilePosRow;
				if ( RowGap < 0 ) {
					Abs = -RowGap;
					FileBuffer.DisplayPosition.Row -= Abs;
				} else {
					FileBuffer.DisplayPosition.Row += RowGap;
				}
			

			}
	}

	FileBuffer.LowVisibleRange.Row = FileBuffer.FilePosition.Row - (FileBuffer.DisplayPosition.Row - TEXT_START_ROW ) ;	

	
	Right = RightCurrentScreen ( NewFilePosCol );
	Left = LeftCurrentScreen ( NewFilePosCol );
	// 
	//if right to current screen
	//
	if ( Right) {
		//
		// display column will be changed to end
		//
		FileBuffer.DisplayPosition.Column = TEXT_END_COLUMN;
		FileBuffer.FilePosition.Column = NewFilePosCol;
	} else {
			if ( Left ) {
				//
				// has engough left characters , so display row unchanged
				// not has enough left characters, so the first character is at the first display column
				//
				if ( NewFilePosCol < ( FileBuffer.DisplayPosition.Column - TEXT_START_COLUMN + 1 ) ) {
					FileBuffer.DisplayPosition.Column = NewFilePosCol + TEXT_START_COLUMN - 1 ;
				}

				FileBuffer.FilePosition.Column = NewFilePosCol;
			} else {
				//
				// in current screen 
				//
				FileBuffer.FilePosition.Column = NewFilePosCol;
				if ( ColGap < 0 ) {
					Abs = - ColGap;
					FileBuffer.DisplayPosition.Column -= Abs;
				} else {
					FileBuffer.DisplayPosition.Column += ColGap;
				}
			}
	}
		
	FileBuffer.LowVisibleRange.Column = FileBuffer.FilePosition.Column - ( FileBuffer.DisplayPosition.Column - TEXT_START_COLUMN ) ;

	
	// 
	//let CurrentLine point to correct line;
	//
	FileBuffer.CurrentLine = MoveCurrentLine ( RowGap );

}









//
// Name:
//		FileBufferScrollRight -- Scroll cursor to right
// In:
//		VOID
// Out:
// 		EFI_SUCCESS
//		

STATIC
EFI_STATUS
FileBufferScrollRight ( VOID )
{
	EFI_EDITOR_LINE *Line;
	UINTN FRow, FCol;
	
	Line = FileBuffer.CurrentLine;
	if ( Line ->Buffer == NULL ) {
		return EFI_SUCCESS;
	}
	
	FRow = FileBuffer.FilePosition.Row;
	FCol = FileBuffer.FilePosition.Column;
	
	// if already at end of this line, scroll it to the start of next line
	if ( FCol > Line->Size ) {
		// has next line
		if ( Line -> Link.Flink != FileBuffer.ListHead ) {
			FRow++;
			FCol = 1;
		} else {
			return EFI_SUCCESS;
		}
	} else {
		
		// if not at end of this line, just move to next column
		FCol++;
	}
	
	FileBufferMovePosition ( FRow, FCol );
	
	return EFI_SUCCESS;
}

	
	
	
	
	
	
	
	
	
//
// Name:
//		FileBufferScrollLeft -- Scroll cursor to left
// In:
//		VOID
// Out:
// 		EFI_SUCCESS
//		
	
STATIC
EFI_STATUS
FileBufferScrollLeft ( VOID )
{
	EFI_EDITOR_LINE *Line;
	UINTN FRow, FCol;
	
	Line = FileBuffer.CurrentLine;
	
	FRow = FileBuffer.FilePosition.Row;
	FCol = FileBuffer.FilePosition.Column;
	
	// if already at start of this line, so move to the end of previous line
	if ( FCol <= 1 ) {
		// has previous line
		if ( Line -> Link.Blink != FileBuffer.ListHead ) {
			FRow--;
			Line = CR(Line -> Link.Blink,EFI_EDITOR_LINE,Link,EFI_EDITOR_LINE_LIST); 
			FCol = Line -> Size + 1 ;
		} else {
			return EFI_SUCCESS;
		}
	} else {
		// if not at start of this line, just move to previous column
		FCol--;
	}
	
	FileBufferMovePosition ( FRow, FCol );
	
	return EFI_SUCCESS;
}









//
// Name:
//		FileBufferScrollDown -- Scroll cursor to the next line
// In:
//		VOID
// Out:
// 		EFI_SUCCESS
//		
	
STATIC
EFI_STATUS
FileBufferScrollDown ()
{
	EFI_EDITOR_LINE *Line;
	UINTN FRow, FCol;
	
	Line = FileBuffer.CurrentLine;
	if ( Line ->Buffer == NULL ) {
		return EFI_SUCCESS;
	}
	
	FRow = FileBuffer.FilePosition.Row;
	FCol = FileBuffer.FilePosition.Column;
	
	// has next line
	if ( Line -> Link.Flink != FileBuffer.ListHead ) {
			FRow++;
			Line = CR(Line -> Link.Flink,EFI_EDITOR_LINE,Link,EFI_EDITOR_LINE_LIST); 
			
			// if the next line is not tha long, so move to end of next line
			if ( FCol > Line -> Size ) {
				FCol = Line -> Size + 1;
			} 
			
	} else {
			return EFI_SUCCESS;
	}
	
	FileBufferMovePosition ( FRow, FCol );
	
	return EFI_SUCCESS;
}
	







//
// Name:
//		FileBufferScrollUp -- Scroll cursor to previous line
// In:
//		VOID
// Out:
// 		EFI_SUCCESS
//		

STATIC
EFI_STATUS
FileBufferScrollUp ()
{
	EFI_EDITOR_LINE *Line;
	UINTN FRow, FCol;
	
	Line = FileBuffer.CurrentLine;
	
	FRow = FileBuffer.FilePosition.Row;
	FCol = FileBuffer.FilePosition.Column;
	
	// has previous line
	if ( Line -> Link.Blink != FileBuffer.ListHead ) {
			FRow--;
			Line = CR(Line -> Link.Blink,EFI_EDITOR_LINE,Link,EFI_EDITOR_LINE_LIST); 
			
			// if previous line is not that long, so move to the end of previous line
			if ( FCol > Line -> Size ) {
				FCol = Line -> Size + 1;
			} 
			
	} else {
			return EFI_SUCCESS;
	}
	
	FileBufferMovePosition ( FRow, FCol );
	
	return EFI_SUCCESS;
}







//
// Name:
//		FileBufferPageDown -- Scroll cursor to next page
// In:
//		VOID
// Out:
// 		EFI_SUCCESS
//		

STATIC
EFI_STATUS
FileBufferPageDown ()
{
	EFI_EDITOR_LINE *Line;
	UINTN FRow, FCol;
	UINTN Gap;
	
	Line = FileBuffer.CurrentLine;
	
	FRow = FileBuffer.FilePosition.Row;
	FCol = FileBuffer.FilePosition.Column;
	
	// has next page
	if ( FileBuffer.NumLines >= FRow + NUM_TEXT_ROWS ) {
		Gap = NUM_TEXT_ROWS ;
	} else {
		// MOVE CURSOR TO LAST LINE
		Gap = FileBuffer.NumLines - FRow;
	}

	// get correct line
	Line = MoveLine ( Gap );
	
	// if that line, is not that long, so move to the end of that line
	if ( FCol > Line -> Size ) {
		FCol = Line -> Size + 1;
	}
	
	FRow += Gap;

	FileBufferMovePosition ( FRow, FCol );
	
	return EFI_SUCCESS;
}








//
// Name:
//		FileBufferPageUp -- Scroll cursor to previous line
// In:
//		VOID
// Out:
// 		EFI_SUCCESS
//		

STATIC
EFI_STATUS
FileBufferPageUp ()
{
	EFI_EDITOR_LINE *Line;
	UINTN FRow, FCol;
	UINTN Gap;
	INTN  Retreat;
	
	Line = FileBuffer.CurrentLine;
	
	FRow = FileBuffer.FilePosition.Row;
	FCol = FileBuffer.FilePosition.Column;
	
	
	// has previous page
	if ( FRow > NUM_TEXT_ROWS ) {
		Gap = NUM_TEXT_ROWS ;
	} else {
		// the first line of file will displayed on the first line of screen
		Gap = FRow - 1 ;
	}

	Retreat = Gap;
	Retreat = -Retreat;

	// get correct line
	Line = MoveLine ( Retreat );
	
	// if that line is not that long, so move to the end of that line
	if ( FCol > Line -> Size ) {
		FCol = Line -> Size + 1;
	}
	
	FRow -= Gap;
	
	FileBufferMovePosition ( FRow, FCol );
	
	return EFI_SUCCESS;
}








//
// Name:
//		FileBufferHome -- Scroll cursor to start of line
// In:
//		VOID
// Out:
// 		EFI_SUCCESS
//		

STATIC
EFI_STATUS
FileBufferHome ()
{
	EFI_EDITOR_LINE *Line;
	UINTN FRow, FCol;
	
	Line = FileBuffer.CurrentLine;
	
	FRow = FileBuffer.FilePosition.Row;
	FCol = 1;

	FileBufferMovePosition ( FRow, FCol );
	
	return EFI_SUCCESS;
}







//
// Name:
//		FileBufferEnd -- Scroll cursor to end of line
// In:
//		VOID
// Out:
// 		EFI_SUCCESS
//		
	
STATIC
EFI_STATUS
FileBufferEnd ()
{
	EFI_EDITOR_LINE *Line;
	UINTN FRow, FCol;
	
	Line = FileBuffer.CurrentLine;
	
	FRow = FileBuffer.FilePosition.Row;
	
	FCol = Line -> Size + 1;

	FileBufferMovePosition ( FRow, FCol );
	
	return EFI_SUCCESS;
}








//
// Name:
//		FileBufferDoReturn -- Add a return into line
// In:
//		VOID
// Out:
// 		EFI_SUCCESS
//		EFI_OUT_OF_RESOURCES
//		

STATIC
EFI_STATUS
FileBufferDoReturn (
    VOID
    )
{
    EFI_EDITOR_LINE *Line;
    EFI_EDITOR_LINE *NewLine;
    UINTN           FileColumn;
	UINTN			i;
	CHAR16			*Buffer;
	UINTN			Row, Col;

	FileBufferNeedRefresh = TRUE;
	FileBufferOnlyLineNeedRefresh = FALSE;

    Line = FileBuffer.CurrentLine ;

    FileColumn = FileBuffer.FilePosition.Column ;

	NewLine = AllocatePool (sizeof(EFI_EDITOR_LINE));
    if ( NewLine == NULL ) {
        return EFI_OUT_OF_RESOURCES;
    }
    
    NewLine->Signature = EFI_EDITOR_LINE_LIST;
    NewLine->Size = Line->Size - FileColumn + 1 ;
	NewLine -> TotalSize = NewLine -> Size;
    NewLine->Buffer = PoolPrint (L"\0");

	if ( NewLine -> Size > 0 ) {
		// UNICODE + '\0'
		Buffer = AllocatePool ( 2 * ( NewLine->Size + 1 ));
		if ( Buffer == NULL ) {
			FreePool ( NewLine -> Buffer );
			FreePool ( NewLine );
			return EFI_OUT_OF_RESOURCES;
		}
		
		FreePool ( NewLine -> Buffer );
		
		NewLine->Buffer = Buffer;

		for ( i = 0 ; i < NewLine -> Size; i++ ) {
			NewLine -> Buffer[i] = Line -> Buffer [ i + FileColumn - 1];
		}

		NewLine -> Buffer [ NewLine -> Size ] = L'\0';
		
		Line->Buffer[FileColumn - 1] = L'\0';
		Line->Size = FileColumn - 1;
	} 
    
	// increase NumLines
	++FileBuffer.NumLines;

	// insert it into the correct position of line list
    NewLine->Link.Blink = &(Line->Link);
    NewLine->Link.Flink = Line->Link.Flink;
    Line->Link.Flink->Blink = &(NewLine->Link);
    Line->Link.Flink = &(NewLine->Link);


	// move cursor to the start of next line
	Row = FileBuffer.FilePosition.Row + 1;
	Col = 1;
	
	FileBufferMovePosition (Row, Col);	
    
    // set file is modified
    if (!FileBuffer.FileModified) {
        FileBuffer.FileModified = TRUE;
     }

    return EFI_SUCCESS;
}







//
// Name:
//		FileBufferDoBackspace -- delete the previous character
// In:
//		VOID
// Out:
// 		EFI_SUCCESS
//		EFI_OUT_OF_RESOURCES
//		

STATIC
EFI_STATUS
FileBufferDoBackspace (
    VOID
    )
{
    EFI_EDITOR_LINE *Line;
    EFI_EDITOR_LINE *End;
    LIST_ENTRY      *Link;
    UINTN           FileColumn;
	
	FileColumn = FileBuffer.FilePosition.Column ;

	Line = FileBuffer.CurrentLine;

	// the first column
    if (FileColumn == 1) {
		
		// the first row
        if (FileBuffer.FilePosition.Row == 1) {
            return EFI_SUCCESS;
        }

        FileBufferScrollLeft();
        
		Line = FileBuffer.CurrentLine;
		Link = Line->Link.Flink;
		End = CR(Link,EFI_EDITOR_LINE,Link,EFI_EDITOR_LINE_LIST);
		
		// concatecate this line with previous line	
		LineCat(Line, End);
		if ( Line -> Buffer == NULL ) {
			return EFI_OUT_OF_RESOURCES;
		}

		// remove End from line list
		RemoveEntryList(&End->Link);
		FreePool ( End );

        --FileBuffer.NumLines;
		
		FileBufferNeedRefresh = TRUE;
		FileBufferOnlyLineNeedRefresh = FALSE;
    
    } else {
		// just delete the previous character
        LineDeleteAt(Line,FileColumn - 1);
        FileBufferScrollLeft();
		FileBufferOnlyLineNeedRefresh = TRUE;
    }


    if (!FileBuffer.FileModified) {
        FileBuffer.FileModified = TRUE;
    }

    return EFI_SUCCESS; 
}









//
// Name:
//		FileBufferDelete -- Delete current character from line
// In:
//		VOID
// Out:
// 		EFI_SUCCESS
//		

STATIC
EFI_STATUS  
FileBufferDoDelete (
    VOID
    )
{
    EFI_EDITOR_LINE *Line;
    EFI_EDITOR_LINE *Next;
    LIST_ENTRY      *Link;
    UINTN           FileColumn;

    
	
	Line = FileBuffer.CurrentLine;
    FileColumn = FileBuffer.FilePosition.Column;


	// the last column
    if (FileColumn >= Line->Size + 1) {
		// the last line
		if (Line->Link.Flink == FileBuffer.ListHead) {
			return EFI_SUCCESS;
		}

        Link = Line->Link.Flink;
        Next = CR(Link,EFI_EDITOR_LINE,Link,EFI_EDITOR_LINE_LIST);
		LineCat(Line,Next);
		if ( Line -> Buffer == NULL ) {
			return EFI_OUT_OF_RESOURCES;
		}

        RemoveEntryList(&Next->Link);
        FreePool(Next);
        
        --FileBuffer.NumLines;
		
		FileBufferNeedRefresh = TRUE;
		FileBufferOnlyLineNeedRefresh = FALSE;
        
    } else {
    	// just delete current character
        LineDeleteAt (Line,FileColumn);
		FileBufferOnlyLineNeedRefresh = TRUE;
    }

    if (!FileBuffer.FileModified) {
        FileBuffer.FileModified = TRUE;
    }

    return EFI_SUCCESS; 
}







//
// Name:
//		FileBufferChangeMode -- Change between INS/OVR mode
// In:
//		VOID
// Out:
// 		EFI_SUCCESS
//		

STATIC
EFI_STATUS
FileBufferChangeMode    (
    VOID
    )
{

    FileBuffer.ModeInsert = !FileBuffer.ModeInsert;
    return EFI_SUCCESS;
}






//
// Name:
//		FileBufferCutLine -- Cut current line out
// In:
//		CutLine -- to pass current line out
// Out:
// 		EFI_SUCCESS
//		EFI_NOT_FOUND
//		EFI_OUT_OF_RESOURCES
//		

STATIC 
EFI_STATUS
FileBufferCutLine ( EFI_EDITOR_LINE **CutLine )
{
	EFI_EDITOR_LINE *Line;
	EFI_EDITOR_LINE *NewLine;
	UINTN Row, Col;

	if ( FileBuffer.ReadOnly ) {
		MainStatusBarSetStatusString (L"Read Only File Can Not Be Modified");
		return EFI_SUCCESS;
	}

	Line = FileBuffer.CurrentLine;

	// if is the last dummy line, SO CAN not cut
	if ( StrCmp ( Line->Buffer, L"\0") == 0 &&
		 Line->Link.Flink == FileBuffer.ListHead // last line
		 ) {
			// LAST LINE AND NOTHING ON THIS LINE, SO CUT NOTHING
			MainStatusBarSetStatusString (L"Nothing to Cut");
			return EFI_NOT_FOUND; 
	}

	// if is the last line, so create a dummy line
	if ( Line->Link.Flink == FileBuffer.ListHead ) { // last line
		// create a new line
		NewLine = FileBufferCreateLine ();
		if (NewLine == NULL) {
				return EFI_OUT_OF_RESOURCES;
		}
	}

    FileBuffer.NumLines--;
	Row = FileBuffer.FilePosition.Row;
	Col = 1; // move home
	FileBuffer.CurrentLine = CR(FileBuffer.CurrentLine->Link.Flink,EFI_EDITOR_LINE,Link,EFI_EDITOR_LINE_LIST);
	RemoveEntryList(&Line->Link);

	FileBuffer.Lines = CR(FileBuffer.ListHead->Flink,EFI_EDITOR_LINE,Link,EFI_EDITOR_LINE_LIST);

	FileBufferMovePosition ( Row, Col);

	FileBuffer.FileModified = TRUE;
	FileBufferNeedRefresh = TRUE;
	FileBufferOnlyLineNeedRefresh = FALSE;

	*CutLine = Line;

	return EFI_SUCCESS;
}







//
// Name:
//		FileBufferPasteLine -- Paste a line into line list
// In:
//		VOID
// Out:
// 		EFI_SUCCESS
// 		EFI_OUT_OF_RESOURCES
//		

STATIC 
EFI_STATUS
FileBufferPasteLine ( VOID )
{
	EFI_EDITOR_LINE *Line;
	EFI_EDITOR_LINE *NewLine;
	UINTN Row, Col;

	// if nothing is on clip board
	if ( MainEditor.CutLine == NULL ) {
		return EFI_SUCCESS;
	}

	if ( FileBuffer.ReadOnly ) {
		MainStatusBarSetStatusString (L"Read Only File Can Not Be Modified");
		return EFI_SUCCESS;
	}


	NewLine = LineDup ( MainEditor.CutLine );
	if ( NewLine == NULL ) {
		return EFI_OUT_OF_RESOURCES;
	}

	// insert it above current line
	Line = FileBuffer.CurrentLine;
    NewLine->Link.Blink = Line->Link.Blink;
    NewLine->Link.Flink = &Line->Link;

    Line->Link.Blink->Flink = &NewLine->Link;
    Line->Link.Blink = &NewLine->Link;

    FileBuffer.NumLines++;
    FileBuffer.CurrentLine = NewLine;


	FileBuffer.Lines = CR(FileBuffer.ListHead->Flink,EFI_EDITOR_LINE,Link,EFI_EDITOR_LINE_LIST);

	Col  = 1; // move home
	Row = FileBuffer.FilePosition.Row;

	FileBufferMovePosition ( Row, Col );

	FileBuffer.FileModified = TRUE;
	
	
	FileBufferNeedRefresh = TRUE;
	FileBufferOnlyLineNeedRefresh = FALSE;

	return EFI_SUCCESS;
}








//
// Name:
//		FileBufferSearch -- Search string from current position on in file
// In:
//		Str -- search string
//		Offset -- Offset from current position
// Out:
// 		EFI_SUCCESS
//		EFI_NOT_FOUND: Such string not found
//		

STATIC 
EFI_STATUS
FileBufferSearch ( CHAR16 *Str , UINTN Offset)
{
	CHAR16 *Current;
	UINTN Position;
	UINTN Row, Column;
	EFI_EDITOR_LINE *Line;
    LIST_ENTRY      *Link;
	BOOLEAN Found;

	
	// search if in current line
	Current = FileBuffer.CurrentLine -> Buffer + FileBuffer.FilePosition.Column - 1 + Offset;
	if ( Current >= FileBuffer.CurrentLine->Buffer + FileBuffer.CurrentLine->Size ) 
			Current = FileBuffer.CurrentLine->Buffer + FileBuffer.CurrentLine->Size ; // the end
	
	
	Position = StrStr ( Current, Str );
	
	// found
	if ( Position ) {
		Column = ( Position - 1 ) + FileBuffer.FilePosition.Column + Offset;
		Row = FileBuffer.FilePosition.Row;
		Found = TRUE;
	} else { // not found so find through next lines
			Link = FileBuffer.CurrentLine->Link.Flink;

			Row = FileBuffer.FilePosition.Row + 1;
			while ( Link != FileBuffer.ListHead ) {
				Line = CR(Link,EFI_EDITOR_LINE,Link,EFI_EDITOR_LINE_LIST);
				Position = StrStr ( Line -> Buffer, Str );
				if ( Position ) { // found
					Column = Position ;
					break;
				}

				Row++;
				Link = Link->Flink;
			}

			if ( Link == FileBuffer.ListHead ) {
				Found = FALSE;
			} else {
				Found = TRUE;
			}
	}

	if ( !Found ) {
		return EFI_NOT_FOUND;
	}

	FileBufferMovePosition ( Row, Column );
	
	// call refresh to fresh edit area, because the outter may loop to find multiply ocurrence of this string
	FileBufferRefresh ();

	return EFI_SUCCESS;
}







//
// Name:
//		FileBufferReplace -- Replace SearchLen characters from current position on with Replace
// In:
//		Replace -- string to replace
//		SearchLen -- Search string's length
// Out:
// 		EFI_SUCCESS
//		EFI_OUT_OF_RESOURCES
//		

STATIC 
EFI_STATUS
FileBufferReplace ( CHAR16 *Replace , UINTN SearchLen)
{
	UINTN ReplaceLen;
	UINTN i;
	CHAR16 *Buffer;
	UINTN NewSize;
	UINTN OldSize;
	UINTN Gap;

	ReplaceLen = StrLen ( Replace );
	
	OldSize =  FileBuffer.CurrentLine ->Size  + 1; // include '\0'
	NewSize = OldSize +  ( ReplaceLen - SearchLen );

	if ( ReplaceLen > SearchLen ) {

		// do not have the enough space
		if ( FileBuffer.CurrentLine -> TotalSize + 1 <= NewSize ) {
			FileBuffer.CurrentLine->Buffer = ReallocatePool( FileBuffer.CurrentLine->Buffer, 2 * OldSize , 2 * NewSize);
			FileBuffer.CurrentLine->TotalSize = NewSize - 1;
		}

		if ( FileBuffer.CurrentLine->Buffer == NULL )  
			return EFI_OUT_OF_RESOURCES;

		// the end '\0' character;
		Buffer = FileBuffer.CurrentLine->Buffer + ( NewSize - 1 ) ;
		Gap = ReplaceLen - SearchLen;

		// keep the latter part
		for ( i = 0 ; i < ( FileBuffer.CurrentLine->Size - FileBuffer.FilePosition.Column - SearchLen + 2); i++ ) {
			*Buffer = *(Buffer - Gap);
			Buffer--;
		}

		// set replace into it
		Buffer = FileBuffer.CurrentLine -> Buffer + FileBuffer.FilePosition.Column - 1;
		for ( i = 0 ; i < ReplaceLen ; i++ ) 
			Buffer[i] = Replace[i];
		
	}

	if ( ReplaceLen < SearchLen ) {
		Buffer = FileBuffer.CurrentLine -> Buffer + FileBuffer.FilePosition.Column - 1;
		for ( i = 0 ; i < ReplaceLen ; i++ ) 
			Buffer[i] = Replace[i];

		Buffer += ReplaceLen;
		Gap = SearchLen - ReplaceLen;
		
		// set replace into it
		for ( i = 0 ; i < ( FileBuffer.CurrentLine->Size - FileBuffer.FilePosition.Column - ReplaceLen + 2); i++ ) {
			*Buffer = *(Buffer + Gap );
			Buffer++;
		}
	}

	if ( ReplaceLen == SearchLen ) {
		Buffer = FileBuffer.CurrentLine -> Buffer + FileBuffer.FilePosition.Column - 1;
		for ( i = 0 ; i < ReplaceLen; i++ )
			Buffer[i] = Replace[i];
	}

	FileBuffer.CurrentLine->Size += ( ReplaceLen - SearchLen );

	
	FileBufferOnlyLineNeedRefresh = TRUE;
	
	FileBuffer.FileModified = TRUE;

	MainTitleBarRefresh ();
	FileBufferRefresh ();


	return EFI_SUCCESS;
}
		
			


#endif  // _LIB_FILE_BUFFER
