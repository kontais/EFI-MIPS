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
#include <string.h>
#include <wchar.h>
#include <unistd.h>
#include <sys/param.h>


#define DEFAULT_DISK_SIZE	32
#define	DEFAULT_DRIVER		"ramdisk.efi"

char	*usage = "%s: [-d ramdisk_driver] [-s size_in_MBs] drive_mapping ...\n";

void		MakeDisk( char *driver, int size, char *name );
EFI_HANDLE	LoadDriver( char *driver, int size );
int			MapDrive( EFI_HANDLE image, char *name );
int			HandleToIndex( EFI_HANDLE image );

int
main( int argc, char **argv )
{
	int		ch;
	int		size = DEFAULT_DISK_SIZE;
	char	*driver = DEFAULT_DRIVER;
	char *driverBack = NULL;
	int status = 0;

	if (argc == 1)
	{
		printf(usage, "Usage");
		return 0;
	}
	
	while ( argc > 1 ) {
		optind = 1;
		while ((ch = getopt(argc, argv, "s:n:d:?")) != -1) {
			switch(ch) {
				case 's':
					size = strtoul( optarg, NULL, 0 );
					break;

				case 'd':
					driver = optarg;
					break;

				case '?':
				default:
					printf(usage, *argv);
					exit (1);
			}
		}

		argc -= optind;
		argv += optind;
		if ( argc ) {
		//Add the default dir path
		       if (!strchr(driver, '\\'))
		       {
		       	driverBack = malloc(sizeof(char) * 256);
		       	if (!driverBack)
		       		{
		       			printf("Not enough memory\n");
		       			exit (1);
		       		}
		       	driverBack[0] = '.';
		       	driverBack[1] = '\\';
		       	strcpy((char*)(driverBack+2), driver);
			MakeDisk( driverBack, size, *argv );
			if (driverBack)
				free(driverBack);
		       }
		       else
		       {
	 			MakeDisk( driver, size, *argv );
			}
		}
		else
		{
			printf(usage, "parameter ");
		}
	}

	return 0;
}

void
MakeDisk( char *driver, int size, char *name )
{
	EFI_HANDLE	image;

	/*
	 *  Load the RAM disk device driver
	 */
	if ( image = LoadDriver( driver, size ) ) {
		if ( MapDrive( image, name ) == 0 ) {
			printf("Done: RAM Disk [ %s ] with size [ %dMB ]\n", name, size);
		}
	}
	else
		{
			printf("Please assign the ramdisk driver using -d option with full path\n");
		}
}

EFI_HANDLE
LoadDriver( char *driver, int size )
{
	wchar_t		*path;
	EFI_HANDLE	image;
	EFI_STATUS	status;
	size_t		len;

	/*
	 *  Allocate memory for large enough for command line
	 */
	len = strlen( driver ) + 10;
	path = calloc( len, sizeof(wchar_t) );
	if ( path == NULL ) {
		perror("calloc");
		return ( NULL );
	}

	/*
	 *  Convert drive name to Unicode and make sure it's null terminated.
	 */
	if ( mbstowcs( path, driver, len ) < 0 ) {
		printf("Error converting string to Unicode\n");
		return ( NULL );
	}

	status = LoadImage( path, &image );
	if ( EFI_ERROR( status ) ) {
		printf("LoadImage of %s returned EFI error %p\n", driver, (void*)status);
		return ( NULL );
	}

	/*
	 *  Add requested drive size to command line.
	 */
	swprintf( &path[ wcslen(path) ], L" %d", size );
	status = StartImage( image,
						 (UINT32)((wcslen( path ) + 1) * sizeof(wchar_t)),
						 path,
						 NULL,
						 NULL,
						 NULL );

	/*
	 *  We'll assume the RAM drive will not access load options after starting.
	 */
	free( path );

	if ( EFI_ERROR( status ) ) {
		printf("StartImage of %s returned EFI error %p\n", driver, (void*)status);
		return ( NULL );
	}

	return ( image );
}

int
MapDrive( EFI_HANDLE image, char *name )
{
	char cmdline[ MAXPATHLEN ];
	int	 index, ret;

	index = HandleToIndex( image );
	if ( index < 0 ) {
		printf("Could not convert image handle to index!\n");
		return -1;
	}

	sprintf( cmdline, "nshell map %s %x", name, index );

	if ( ret = system( cmdline ) )
		printf("EFI shell returned %x for map command\n", ret);

	return ( ret );
}

/*
 *  A little kludgey, but this is how the shell's map command creates the
 *  handle indexs.  Note that the handle we want is the one that follows
 *  the image handle of the ram disk driver.
 */
int
HandleToIndex( EFI_HANDLE image )
{
	EFI_STATUS	Status;
	EFI_HANDLE	*pHandles;
	UINTN		BufferSize;
	int			i, total;

	/*
	 *  Make a guess
	 */
	BufferSize = 2 * sizeof(EFI_HANDLE);

retry:
	pHandles = malloc( BufferSize );
	if ( pHandles == NULL ) {
		perror("malloc");
		return ( -1 );
	}

	Status = _LIBC_EFISystemTable->BootServices->LocateHandle(
							AllHandles,
							NULL,
							NULL,
							&BufferSize,
							pHandles
							);
	if ( EFI_ERROR( Status ) ) {
		if ( Status == EFI_BUFFER_TOO_SMALL ) {
			free( pHandles );
			goto retry;
		} else {
			printf("LocateHandle returned EFI error %p\n", (void*)Status);
		}
	} else {
		/*
		 *  See if we can find a match
		 */
		for ( i = 0, total = (int)(BufferSize / sizeof(EFI_HANDLE)); i < total; i++ ) {
			if ( pHandles[ i ] == image ) {
				free( pHandles );
				/*
				 *  The handle we really need is the one after this which
				 *  will be the handle with all the block io, fs, devicepath,
				 *  etc. on it.
				 */
				return ( i + 1 );
			}
		}
	}

	free( pHandles );
	return ( -1 );
}
