/*
 * Copyright (c) 1999, 2000
 * Intel Corporation.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 * 
 *    This product includes software developed by Intel Corporation and
 *    its contributors.
 * 
 * 4. Neither the name of Intel Corporation or its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY INTEL CORPORATION AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL INTEL CORPORATION OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */

/*++

Copyright (c) 2000  Intel Corporation

Module Name:

    efimisc.c

Abstract:

		Stubs for missing Free BSD functions    



Revision History

--*/

#include <atk_libc.h>
#include <efilib.h>
#include <sys/cdefs.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/signal.h>
#include "histedit.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <wchar.h>
#include <glob.h>
#include "ftp_var.h"
#include "efimisc.h"
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>

/* #define FTP_DEBUG	 uncomment to enable debug messages on stderr */

__sighandler_t *signal (int sig, __sighandler_t * func)
{

#ifdef FTP_DEBUG
	DEBUG((D_WARN,"ftp: signal stubbed\n"));
#endif
	return 0; 
}


struct passwd	*getpwuid (uid_t uid)
{
#ifdef FTP_DEBUG
	DEBUG((D_WARN,"ftp: getpwuid stubbed\n"));
#endif
	return NULL;
}

struct passwd	*getpwnam (const char *login)
{
#ifdef FTP_DEBUG
	DEBUG((D_WARN,"ftp: getpwnam stubbed\n"));
#endif
	return NULL;
}

uid_t	 getuid (void)
{
#ifdef FTP_DEBUG
	DEBUG((D_WARN,"ftp: getuid stubbed\n"));
#endif
	return 0;
}

char	*getlogin (void)
{
#ifdef FTP_DEBUG
	DEBUG((D_WARN,"ftp: getlogin stubbed\n"));
#endif
	return NULL;
}


int	setitimer (int which, const struct itimerval *value, struct itimerval *ovalue)
{
#ifdef FTP_DEBUG
	DEBUG((D_WARN,"ftp: setitimer stubbed\n"));
#endif
	return 0;
}



char * getpass(const char *prompt)
{
	static char 		pwbuff[128] = {0} ;
    EFI_INPUT_KEY       key;
    EFI_STATUS          status;
    UINTN               len, index;
    EFI_SYSTEM_TABLE	*SysTbl = _LIBC_EFISystemTable;
    EFI_BOOT_SERVICES	*BootSvr = SysTbl->BootServices;


	printf("%s",prompt);
	fflush (stdout);

    len = 0;
    while (1)
    {
        status = BootSvr->WaitForEvent( 1, &SysTbl->ConIn->WaitForKey, &index);
        if ( status != EFI_SUCCESS )
        {
        	printf("EFI service error 0x%p\n", (void*)status);
        	pwbuff[0] = 0;
        	return pwbuff;
        }

        status = SysTbl->ConIn->ReadKeyStroke(SysTbl->ConIn, &key);
        if (EFI_ERROR(status)) 
            break;

        if (key.UnicodeChar == '\n' || key.UnicodeChar == '\r') 
		{
			printf("\n");
            break;
		}        
        if (key.UnicodeChar == '\b') 
        {
            if (len) 
                len -= 1;
            continue;
        }

        if (key.UnicodeChar >= ' ') 
        {
            pwbuff[len] = (char)key.UnicodeChar;
            pwbuff[len+1] = 0;
            len += 1;
        }

		if ( len >= ( sizeof(pwbuff) -1 ))	/* can't fit any more in buffer */
			break;
    }

    pwbuff[len] = 0;
	return (char *) &pwbuff;
}
 
int	chmod (const char *path, mode_t mode)
{
#ifdef FTP_DEBUG
	DEBUG((D_WARN,"ftp: chmod stubbed\n"));
#endif
	return 0;
}


int	 access (const char *path, int mode)
{
#ifdef FTP_DEBUG
	DEBUG((D_WARN,"ftp: access stubbed\n"));
#endif
	return 0;
}
 

History * history_init (void)
{
#ifdef FTP_DEBUG
	DEBUG((D_WARN,"ftp: history_init stubbed\n"));
#endif
	return  NULL;
}

const HistEvent * history (History *h, int op, ...)
{

#ifdef FTP_DEBUG
	DEBUG((D_WARN,"ftp: history stubbed\n"));
#endif
	return  NULL;
}


void history_end (History *h)
{
#ifdef FTP_DEBUG
	DEBUG((D_WARN,"ftp: history_end stubbed\n"));
#endif
	return;
}


const char *el_gets	(EditLine *e, int *count)
{
#ifdef FTP_DEBUG
	DEBUG((D_WARN,"ftp: history_end stubbed\n"));
#endif
	return NULL ;
}

void el_end	(EditLine *e)
{
#ifdef FTP_DEBUG
	DEBUG((D_WARN,"ftp: el_end stubbed\n"));
#endif
	return;
}

int el_source (EditLine *e, const char *file)
{
#ifdef FTP_DEBUG
	DEBUG((D_WARN,"ftp: el_source stubbed\n"));
#endif
	return 0;
}

EditLine *el_init (const char *prog, FILE *fin, FILE *fout)
{

#ifdef FTP_DEBUG
	DEBUG((D_WARN,"ftp: el_init stubbed\n"));
#endif
	return NULL ;
}

int el_set (EditLine *e, int opt, ...)
{
#ifdef FTP_DEBUG
	DEBUG((D_WARN,"ftp: el_set stubbed\n"));
#endif
	return 0;
}


int el_insertstr (EditLine *e, char *str)
{
#ifdef FTP_DEBUG
	DEBUG((D_WARN,"ftp: el_insewrtstr stubbed\n"));
#endif
	return 0 ;
}

const LineInfo *el_line	(EditLine *e)
{
#ifdef FTP_DEBUG
	DEBUG((D_WARN,"ftp: el_line stubbed -- expect crash to follow\n"));
#endif
	return NULL ;
}

FILE *popen (const char *cmd, const char *type)
{
#ifdef FTP_DEBUG
	DEBUG((D_WARN,"ftp: popen stubbed\n"));
#endif
	return NULL ;
}

int pclose (FILE *stream)
{
#ifdef FTP_DEBUG
	DEBUG((D_WARN,"ftp: pclose stubbed\n"));
#endif
	return -1 ;
}


int issetugid (void)
{
#ifdef FTP_DEBUG
	DEBUG((D_WARN,"ftp: issetugid stubbed\n"));
#endif
	return 0 ;	/* calling process is not "tainted" */
}

static char newpath[ MAXPATHLEN ];

void
do_ls( int argc, char *argv[] )
{
	char			*files;
	glob_t			gl;
	struct	stat	sb;
	int				flags = GLOB_BRACE|GLOB_NOCHECK|GLOB_QUOTE|GLOB_MARK;

	/*
	 *  If no files were specified, list all files in current directory
	 */
	if ( argc == 0 )
		files = "*";
	else
		files = *argv;

	/*
	 *  If they gave a directory name, list it's contents
	 */
	if ( stat( files, &sb ) == 0 ) {
		if ( sb.st_mode & S_IFDIR ) {
			strcpy( newpath, files );
			strcat( newpath, "/*" );
			files = newpath;
		}
	}

	if ( glob( files, flags, NULL, &gl ) == 0 ) {
		int		i, longest, cols, mod;

		/*
		 *  Find the longest name
		 */
		for ( i = longest = 0; i < gl.gl_matchc; i++ ) {
			int len;

			len = (int)strlen(gl.gl_pathv[i]);
			if ( len > longest )
				longest = len;
		}

		/*
		 *  We have the longest string, find out how many columns we have
		 */
		cols = ttywidth / (longest + 2);

		/*
		 * Now list the names
		 */
		for ( i = mod = 0; i < gl.gl_matchc; i++ ) {
			printf("% *s ", -longest, gl.gl_pathv[i]);
			mod++;
			if ((mod % cols) == 0)
				printf("\n");
		}

		if (mod % cols)
			printf("\n");
		globfree(&gl);
	}
}

void
do_dir( int argc, char *argv[] )
{
	char			*files;
	glob_t			gl;
	struct	stat	sb;
	int				flags = GLOB_BRACE|GLOB_NOCHECK|GLOB_QUOTE;

	/*
	 *  If no files were specified, list all files in current directory
	 */
	if ( argc == 0 )
		files = "*";
	else
		files = *argv;

	/*
	 *  If they gave a directory name, list it's contents
	 */
	if ( stat( files, &sb ) == 0 ) {
		if ( sb.st_mode & S_IFDIR ) {
			strcpy( newpath, files );
			strcat( newpath, "/*" );
			files = newpath;
		}
	}

	if ( glob( files, flags, NULL, &gl ) == 0 ) {
		int			i;
		struct tm	*tm;

		/*
		 *  Now list the names
		 */
		for ( i = 0; i < gl.gl_matchc; i++ ) {
			char	buf[ 32 ];

			if ( stat( gl.gl_pathv[i], &sb ) < 0 ) {
				printf("Can't get file info on %s\n", gl.gl_pathv[i]);
				continue;
			}

			tm = localtime( &sb.st_mtime );
			strftime( buf, sizeof(buf)-1, "%D  %I:%M", tm );

			printf("%s%c % -6s % 16d  %s\n",
					buf,
					tm->tm_hour < 12 ? 'a' : 'p',
					sb.st_mode & S_IFDIR ? "<DIR>" : " ",
					(unsigned int)sb.st_size,
					gl.gl_pathv[i]);
		}

		globfree(&gl);
	}
}

#define CP_BUFSIZE	0x10000
void
do_cp( int argc, char *argv[] )
{
	char		*src, *dst;
	int 		srcfd, dstfd;
	char		*buf;
	ssize_t		bytes;
	struct stat	sb;

	/*
	 *  Make sure two arguments were supplied
	 */
	if ( argc != 2 ) {
		printf("cp: must supply src and dst filenames\n");
		return; 
	}
	
	src = argv[0];
	dst = argv[1];

	/*
	 *  If the destination is a directory, copy it there.
	 */
	if ( stat( dst, &sb ) == 0 ) {
		if ( sb.st_mode & S_IFDIR ) {
			strcpy( newpath, dst );
			strcat( newpath, "/" );
			strcat( newpath, src );
			dst = newpath;
		}
	}

	/*
	 *  Open source
	 */
	if ((srcfd = open( src, O_RDONLY )) < 0) {
		printf("Error opening source file %s: %s\n", src, strerror(errno));
		return;
	}

	/*
	 *  Create destination
	 */
	if ((dstfd = open( dst, O_CREAT | O_WRONLY | O_TRUNC, 0777)) < 0) {
		printf("Error opening destination file %s: %s\n", dst, strerror(errno));
		close(srcfd);
		return;
	}

	/*
	 *  Copy the file
	 */
	buf = malloc( CP_BUFSIZE );
	if ( buf == NULL ) {
		printf("Could not allocate memory\n");
	} else {
		while ( (bytes = read( srcfd, buf, CP_BUFSIZE )) > 0 ) {
			if ( write( dstfd, buf, bytes ) < 0 ) {
				printf("Error writing file %s: %s\n", dst, strerror(errno));
			}
		}

		if ( bytes < 0 ) {
			printf("Error reading file %s: %s\n", dst, strerror(errno));
		}
	}

	free( buf );
	close( srcfd );
	close( dstfd );
}

void
do_mkdir( int argc, char *dirname[] )
{
	if (argc == 0) { 
		printf("mkdir: no directories specified\n");
		return;
	}

	for (; argc; argc--, dirname++) {
		if ( mkdir( *dirname, 0777 ) < 0 ) {
			printf("Error creating directory %s: %s\n", *dirname, strerror(errno));
		}
	}
}

void
do_rm( int argc, char *files[] )
{
	glob_t	gl;
	int		flags = GLOB_BRACE|GLOB_NOCHECK|GLOB_QUOTE;

	if (argc == 0) { 
		printf("rm: no files specified\n");
		return;
	}

	for (; argc; argc--, files++) {
		if ( glob( *files, flags, NULL, &gl ) == 0 ) {
			int			i;
			struct stat	sb;

			if ( gl.gl_matchc == 0 ) {
				printf("rm: File %s does not exist\n", *files);
				return;
			}

			/*
			 *  Process the returned list
			 */
			for ( i = 0; i < gl.gl_matchc; i++ ) {
				char	*file;

				file = gl.gl_pathv[i];

				if ( stat( file, &sb ) < 0 ) {
					printf("Can't get file info on %s\n", file);
					continue;
				}

				if ( sb.st_mode & S_IFDIR ) {
					printf("%s is a directory - skipping\n", file);
					continue;
				}
				if ( unlink( file) < 0 ) {
					printf("Error removing %s: %s\n", file, strerror(errno));
				} else {
					printf("%s removed\n", file);
				}
			}

			globfree(&gl);
		}
	}
}

void
do_rmdir( int argc, char *dirname[] )
{
	glob_t	gl;
	int		flags = GLOB_BRACE|GLOB_NOCHECK|GLOB_QUOTE;

	if (argc == 0) { 
		printf("rmdir: no directores specified\n");
		return;
	}

	for (; argc; argc--, dirname++) {
		if ( glob( *dirname, flags, NULL, &gl ) == 0 ) {
			int			i;
			struct stat	sb;

			if ( gl.gl_matchc == 0 ) {
				printf("rmdir: Directory %s does not exist\n", *dirname);
				return;
			}

			/*
			 *  Process the returned list
			 */
			for ( i = 0; i < gl.gl_matchc; i++ ) {
				char	*dir;

				dir = gl.gl_pathv[i];

				if ( stat( dir, &sb ) < 0 ) {
					printf("Can't get directory info on %s\n", dir);
					continue;
				}

				if ( (sb.st_mode & S_IFDIR) == 0 ) {
					printf("%s is a file - skipping\n", dir);
					continue;
				}
				if ( rmdir( dir) < 0 ) {
					printf("Error removing %s: %s\n", dir, strerror(errno));
				} else {
					printf("%s removed\n", dir);
				}
			}

			globfree(&gl);
		}
	}
}

struct cmd_tbl {
	char	*cmd;
	void	(*func)(int argc, char *argv[]);
	char	*help;
} shellcmds [] = {
	{ "ls",		do_ls,		"Simple column directory listing"	},
	{ "dir",	do_dir,		"Full directory listing"			},
	{ "cp",		do_cp,		"Copy a file"						},
	{ "rm",		do_rm,		"Remove one or more files"			},
	{ "mkdir",	do_mkdir,	"Create a directory"				},
	{ "rmdir",	do_rmdir,	"Remove an emtpy directory"			},
	{ NULL,		NULL,		NULL								},	
};


void
shellemulate( int argc, char *argv[] )
{
	struct cmd_tbl	*p;
	char			cmdline[  MAXPATHLEN ];
	size_t			cmdlen;

	/*
	 *  Regardless of wheter we are compiled for new or old shell, we still
	 *  emulate the more "populare" shell commands
	 */
	for ( p = shellcmds; p->cmd; p++ ) {
		if ( strcmp( p->cmd, *argv ) == 0 ) {
			p->func( --argc, ++argv );
			return;
		}
	}

#ifdef OLD_SHELL
	/*
	 *  Give them some help
	 */
	printf("Available emulated shell commands are:\n");
	for ( p = shellcmds; p->cmd; p++ ) {
		printf("% -10s %s\n", p->cmd, p->help ? p->help : "");
	}
#else
	/*
	 *  Not a built-in.  Let's do a system() call
	 */
	for ( cmdlen = 0, cmdline[0] = 0; argc && cmdlen < (sizeof(cmdline) - 1); argc--, argv++ ) {
		/*
		 *  Include space and null termination in size check
		 */
		if ( strlen(*argv) + 2 + cmdlen > sizeof(cmdline) )
			goto error;
		strcat( cmdline, " " );
		strcat( cmdline, *argv );
		cmdlen += strlen( cmdline );
	}

	(void)system( cmdline );
	return;

error:
	printf("Command line too long\n");
#endif
	return;
}
