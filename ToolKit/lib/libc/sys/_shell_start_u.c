/*
 * Copyright (c) 1999, 2000
 * Intel Corporation.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 
 * 3. All advertising materials mentioning features or use of this software must
 *    display the following acknowledgement:
 * 
 *    This product includes software developed by Intel Corporation and its
 *    contributors.
 * 
 * 4. Neither the name of Intel Corporation or its contributors may be used to
 *    endorse or promote products derived from this software without specific
 *    prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY INTEL CORPORATION AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL INTEL CORPORATION OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */

/*++

Module Name:

    _shell_start_u.c
    
Abstract:

   	C run-time library startup code for EFI shell dependant applications
   	using Unicode args to main()



Revision History

--*/

#include <efilibc.h>
#include <efi_interface.h>
#include <shell.h>

#include <stdlib.h>
#include <wchar.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "libc_debug.h"
#include "libc_private.h"

//
//  External declarations
//
extern char	*__progname ;
extern int main( int, void ** );

//
//  Name:
//		_LIBC_Start_Shellap_U
//
//  Description:
//	   	C run-time library startup code
//
//  Arguments:
//		ImgHandle:	Image handle	
//		SysTable:	System table	
//
//  Returns:
//		Nothing
//
VOID
_LIBC_Start_Shellapp_U(
    IN EFI_HANDLE           ImgHandle,
    IN EFI_SYSTEM_TABLE     *SysTable
    )
{
	int			ReturnValue, 
				Argc; 
	size_t		Length = 0 ;
    CHAR16		**Argv ;
	char		*ArgBuf ;

	// Initialize shell application
	InitializeShellApplication( ImgHandle, SysTable ) ;

	//
	// Set pointers to shell method for getting/freeing CWD.
	// This must be done before calling InitializeLibC
	//
	_GetCwd  = (GetCwd_t)  ShellCurDir;
	_FreeCwd = (FreeCwd_t) FreePool;

	// Initialize C library
	InitializeLibC( ImgHandle, SysTable );

	Argv = SI->Argv ;
	Argc = SI->Argc ;

	Length = wcslen( Argv[0] ) + 1 ;

	// Allocate buffer for arguments
	ArgBuf = calloc( Length, sizeof( char ) ) ;
	if( !ArgBuf ) {
		DPRINT((L"crt0:  Memory Allocation Error (%d)\n", errno)) ;
		ReturnValue = errno ;
		goto Done ;
	}

	// Convert each argument to ASCII
	wcstombs( ArgBuf, Argv[0], wcslen( Argv[0] ) ) ;

	__progname = ArgBuf ;

	// Run the application
	ReturnValue = main( Argc, (void**)Argv ) ;

	//
	//  BugBug - If main() calls exit(), we depend on it to free the vector
	//  and arguments during malloc cleanup.
	//
Done:
	if( ArgBuf ) {
		free( ArgBuf ) ;
	}

	// Cleanup
	exit( ReturnValue ) ;
}
