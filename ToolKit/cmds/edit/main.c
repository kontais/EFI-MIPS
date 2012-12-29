/*++

  Copyright (c) 2000 Intel Corporation

  Module Name:
    main.c

  Abstract:
    

--*/

#include "editor.h"

EFI_STATUS
InitializeEFIEditor (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    );

#ifdef EFI_BOOTSHELL
EFI_DRIVER_ENTRY_POINT(InitializeEFIEditor)
#endif


EFI_HANDLE 	ImageHandleBackup;


//
// Name:
//		InitializeEFIEditor -- Entry point of editor
// In:
//		ImageHandle
//		SystemTable
// Out:
// 		EFI_SUCCESS
//		

EFI_STATUS
InitializeEFIEditor (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    )
{
    EFI_STATUS      Status;
    CHAR16 *Buffer;

#ifdef EFI_BOOTSHELL
    InstallInternalShellCommand (
        ImageHandle,   SystemTable,   InitializeEFIEditor,
        L"edit",                      // command
        L"edit [file name]",          // command syntax
        L"Edit a file",               // 1 line descriptor
        NULL                          // command help page
        );
#endif

	//
	// backup this variable to transfter to FileBufferInit
	//
	ImageHandleBackup = ImageHandle;
	
	
    InitializeShellApplication (ImageHandle, SystemTable);

    if (SI->Argc > 1) {
    	if ( !IsValidFileName ( SI -> Argv[1] ) ) {
			Print (L"Invalid File Name\n");
			return EFI_SUCCESS;
		}
	}


    Status = MainEditorInit();
    if (EFI_ERROR(Status)) {
        Out->ClearScreen(Out);
        Out->EnableCursor(Out,TRUE);
        Print(L"EDIT : Initialization Failed\n");
        return EFI_SUCCESS;
    }
    
	MainEditorBackup ();

    //
    // if editor launched with file named
    //
    if (SI->Argc > 1) {

        FileBufferSetFileName(SI->Argv[1]);
    }

    
    Status = FileBufferRead( MainEditor.FileBuffer->FileName, FALSE);
    if ( !EFI_ERROR ( Status ) ) {
		MainEditorRefresh ();

		Status = MainEditorKeyInput ();
	}

	if ( Status != EFI_OUT_OF_RESOURCES ) {
		// back up the status string
		Buffer = PoolPrint (L"%s", MainEditor.StatusBar->StatusString);
	}
	
	MainEditorCleanup();
	
    // 
    // print editor exit code on screen
    //
    switch ( Status ) {
		case EFI_SUCCESS:
			break;

		case EFI_OUT_OF_RESOURCES:
			Print (L"Editor Error: Out of Resources!\n");
			break;

		default:
			if ( Buffer!= NULL ) {
				if ( StrCmp ( Buffer, L"" ) != 0 ) 
					// print out the status string
					Print (L"Editor Error: %s!\n", Buffer);
				else 
					Print (L"Unknown Editor Error!\n");
			} else {
				Print (L"Unknown Editor Error!\n");
			}
			
			break;
	}


	EditorFreePool ( Buffer );
	
	return EFI_SUCCESS;
}

