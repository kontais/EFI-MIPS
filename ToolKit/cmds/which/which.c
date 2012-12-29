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

#include <atk_libc.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <sys/stat.h>
#include <sys/param.h>

/*
 *  Forward reference
 */
wchar_t* find( wchar_t* name );

int
main( int argc, wchar_t	**argv )
{
	wchar_t			buf[ MAXPATHLEN ];
	wchar_t			*path;
	int				NotFound = TRUE;

	/*
	 *  Find all files listed on the command line
	 */
	for ( argc--, argv++; argc; argc--, argv++ ) {
		/*
		 *  Try as specified
		 */
		path = find( *argv );
		if ( !path ) {
			/*
			 *  Try appending a .efi
			 */
			wcscpy( buf, *argv );
			wcscat( buf, L".efi" );
			path = find( buf );
			if ( !path ) {
				/*
				 *  Try appending a .nsh
				 */
				wcscpy( buf, *argv );
				wcscat( buf, L".nsh" );
				path = find( buf );
			}
		}

		/*
		 *  Print the full name if we were successful
		 */
		if ( path ) {
			wprintf( L"%s\n", path );
			NotFound = FALSE;
			free( path );
		}
	}

	return ( NotFound );
}


wchar_t*
find( wchar_t* name )
{
	wchar_t		*path;
	struct stat	dummy;

	/*
	 *  Get a fully qualified path
	 */
	path = ResolveFilename( name );

	if ( path ) {
		/*
		 *  See if file exists
		 */
		if ( wstat( path, &dummy ) == 0 ) {
			return ( path );
		} else {
			free ( path );
		}
	}

	return ( NULL );
}

