/*
 * Copyright (c) 2000
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

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

int
gethostid(void)
{
	return 0x10227410;
}

static FILE *syslog_fd = NULL;
static int	logpri;
void
openlog( const char *ident, int logopt, int facility )
{
	char  filename[ 64 ];
#ifdef foo

	strcpy( filename, ident );
	strcat( filename, ".log");
	syslog_fd = fopen( filename, "a+");
#endif
syslog_fd = stdout;
	if ( syslog_fd == NULL ) {
		printf("Error opening %s\n", filename);
	} else {
		fprintf(syslog_fd, "\r\n\r\n***********************************\r\n\r\n");
		fflush(syslog_fd);
	}
}

int
setlogmask (int maskpri)
{
	int oldmask = logpri;
	logpri = maskpri;
	return oldmask;
}

void
syslog(int level, const char *fmt, ... )
{
	va_list	pArg;

	if ( syslog_fd == NULL || level > logpri)
		return;

	va_start( pArg, fmt );
	vfprintf( syslog_fd, fmt, pArg );
	va_end( pArg );

	fprintf( syslog_fd, "\r\n");
	fflush(syslog_fd);
}
