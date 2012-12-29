/*++

Copyright (c) 1999 - 2006  Intel Corporation

Module Name:

    EfiMain.c

Abstract:

    Provides an EFi compiant entry point for the Python application

--*/
/* This file was cloned from WinMain.c in the distibution of Python 1.5.2 */


#include <atk_libc.h>
#include <atk_libm.h>
#include <atk_libtty.h>
#include <wchar.h>

#ifdef EFI_NT_EMULATOR
#include <efilib.h>
#endif

#include "Python.h"

EFI_DRIVER_ENTRY_POINT(_LIBC_Start_A);

extern int Py_Main();

/*++

Routine Description: Main entry point.

Arguments:  argc
            argv

Returns:  EFI_SUCCESS if the interpretor exited normally
          EFI_NOT_FOUND on any error exit

--*/

main(int argc, char **argv)
{
	int         PythonStatus;
    EFI_STATUS  EfiStatus;   

#ifdef EFI_NT_EMULATOR
    InitializeLib( _LIBC_EFIImageHandle, _LIBC_EFISystemTable );
#endif

    InitializeLibM( _LIBC_EFIImageHandle, _LIBC_EFISystemTable );

	ttyInit(_LIBC_EFIImageHandle, _LIBC_EFISystemTable);

	PythonStatus = Py_Main( argc, argv );
    if ( PythonStatus == 0) {
        EfiStatus = EFI_SUCCESS;
    } else {

        // This is quite arbitrary, but most error returns
        // are because SOMETHING wasn't found.

        EfiStatus = EFI_NOT_FOUND;
    }

	//
	//  Will not return
	//
	_LIBC_EfiExit( EfiStatus, 0, NULL );
	return( 0 ) ;
}
