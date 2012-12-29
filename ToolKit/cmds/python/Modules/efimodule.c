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

#ifdef EFI_LOADABLE_MODULE
#undef EFI_LOADABLE_MODULE
#endif

/* EFI module implementation */


static char efi__doc__ [] =
"This module provides access to operating system functionality that is\n\
provided by the EFI environment. Please refer to the corresponding EFI\n\
manual entries for more information on calls.";

#include "Python.h"

#include <sys/types.h>
#include <sys/stat.h>
#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>		/* For WNOHANG */
#endif

#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif


#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif /* HAVE_FCNTL_H */

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_UTIME_H
#include <utime.h>
#endif /* HAVE_UTIME_H */

#ifdef HAVE_SYS_UTIME_H
#include <sys/utime.h>
#define HAVE_UTIME_H /* pretend we do for the rest of this file */
#endif /* HAVE_SYS_UTIME_H */

#ifdef HAVE_SYS_TIMES_H
#include <sys/times.h>
#endif /* HAVE_SYS_TIMES_H */

#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif /* HAVE_SYS_PARAM_H */

#ifdef HAVE_SYS_UTSNAME_H
#include <sys/utsname.h>
#endif /* HAVE_SYS_UTSNAME_H */

#define HAVE_DIRENT 1
#ifdef HAVE_DIRENT
#include <dirent.h>
#define NAMLEN(dirent) strlen((dirent)->d_name)
#endif /* HAVE_DIRENT */

#include <utime.h>

#ifndef OLD_SHELL
#define HAVE_SYSTEM 1
#endif

#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
#endif /* MAXPATHLEN */

#include <wctype.h>
#include <ctype.h>
#define	MAX_ENVIRONMENT_NAME	256
#define MAX_ENVIRONMENT_DATA   0x8000

extern char **environ;

// Helper functions

/* Set a POSIX-specific error from errno, and return NULL */

static PyObject *
convertenviron()
{
	PyObject *d;
	char **e;
	d = PyDict_New();
	if (d == NULL)
		return NULL;
	if (environ == NULL)
		return d;
	/* XXX This part ignores errors */
	for (e = environ; *e != NULL; e++) {
		PyObject *v;
		char *p = strchr(*e, '=');
		if (p == NULL)
			continue;
		v = PyString_FromString(p+1);
		if (v == NULL)
			continue;
		*p = '\0';
		(void) PyDict_SetItemString(d, *e, v);
		*p = '=';
		Py_DECREF(v);
	}
	return d;
}

static PyObject *
efi_error()
{
	return PyErr_SetFromErrno(PyExc_OSError);
}

static PyObject *
efi_error_with_filename(name)
	char* name;
{
	return PyErr_SetFromErrnoWithFilename(PyExc_OSError, name);
}

static PyObject *
efi_do_stat(self, args, statfunc)
	PyObject *self;
	PyObject *args;
	int (*statfunc) Py_FPROTO((const char *, struct stat *));
{
	struct stat st;
	char *path;
	int res;
	if (!PyArg_Parse(args, "s", &path))
		return NULL;
    Py_BEGIN_ALLOW_THREADS
	res = (*statfunc)(path, &st);
	Py_END_ALLOW_THREADS
    // check here
    //fprintf(stderr, "--- efi libc stat returns:%d\n", res);
    
	if (res != 0)
		return efi_error_with_filename(path);
#if !defined(HAVE_LARGEFILE_SUPPORT)
	return Py_BuildValue("(llllllllll)",
			     (long)st.st_mode,
			     (long)st.st_ino,
			     (long)st.st_dev,
			     (long)st.st_nlink,
			     (long)st.st_uid,
			     (long)st.st_gid,
			     (long)st.st_size,
			     (long)st.st_atime,
			     (long)st.st_mtime,
			     (long)st.st_ctime);
#else
	return Py_BuildValue("(lLllllLlll)",
			     (long)st.st_mode,
			     (LONG_LONG)st.st_ino,
			     (long)st.st_dev,
			     (long)st.st_nlink,
			     (long)st.st_uid,
			     (long)st.st_gid,
			     (LONG_LONG)st.st_size,
			     (long)st.st_atime,
			     (long)st.st_mtime,
			     (long)st.st_ctime);
#endif
}

static PyObject *
efi_1str(args, func)
	PyObject *args;
	int (*func) Py_FPROTO((const char *));
{
	char *path1;
	int res;
	if (!PyArg_Parse(args, "s", &path1))
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	res = (*func)(path1);
	Py_END_ALLOW_THREADS
	if (res < 0)
		return efi_error_with_filename(path1);
	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *
efi_2str(args, func)
	PyObject *args;
	int (*func) Py_FPROTO((const char *, const char *));
{
	char *path1, *path2;
	int res;
	if (!PyArg_Parse(args, "(ss)", &path1, &path2))
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	res = (*func)(path1, path2);
	Py_END_ALLOW_THREADS
	if (res != 0)
		/* XXX how to report both path1 and path2??? */
		return efi_error();
	Py_INCREF(Py_None);
	return Py_None;
}



// Interface functions

static char efi_close__doc__[] =
"close(fd) -> None\n\
Close a file descriptor (for low level IO).";

static PyObject *
efi_close(self, args)
	PyObject *self;
	PyObject *args;
{
	int fd, res;
	if (!PyArg_Parse(args, "i", &fd))
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	res = close(fd);
	Py_END_ALLOW_THREADS
	if (res < 0)
		return efi_error();
	Py_INCREF(Py_None);
	return Py_None;
}


static char efi__exit__doc__[] =
"_exit(status)\n\
Exit to the system with specified status, no clean up.";

static PyObject *
efi__exit(self, args)
	PyObject *self;
	PyObject *args;
{
	int sts;
	if (!PyArg_Parse(args, "i", &sts))
		return NULL;
	_exit(sts);
	return NULL; /* Make gcc -Wall happy */
}

static char efi_fstat__doc__[]=
"fstat(fd) -> (mode, ino, dev, nlink, uid, gid, size, atime, mtime, ctime)\n\
Like stat(), but for an open file descriptor.";

static PyObject *
efi_fstat(self, args)
	PyObject *self;
	PyObject *args;
{
	int fd;
	struct stat st;
	int res;
	if (!PyArg_Parse(args, "i", &fd))
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	res = fstat(fd, &st);
	Py_END_ALLOW_THREADS
	if (res != 0)
		return efi_error();
#if !defined(HAVE_LARGEFILE_SUPPORT)
	return Py_BuildValue("(llllllllll)",
			     (long)st.st_mode,
			     (long)st.st_ino,
			     (long)st.st_dev,
			     (long)st.st_nlink,
			     (long)st.st_uid,
			     (long)st.st_gid,
			     (long)st.st_size,
			     (long)st.st_atime,
			     (long)st.st_mtime,
			     (long)st.st_ctime);
#else
	return Py_BuildValue("(lLllllLlll)",
			     (long)st.st_mode,
			     (LONG_LONG)st.st_ino,
			     (long)st.st_dev,
			     (long)st.st_nlink,
			     (long)st.st_uid,
			     (long)st.st_gid,
			     (LONG_LONG)st.st_size,
			     (long)st.st_atime,
			     (long)st.st_mtime,
			     (long)st.st_ctime);
#endif
}


static char efi_getpid__doc__[] =
"getpid() -> pid\n\
Return the current process id";

static PyObject *
efi_getpid(self, args)
	PyObject *self;
	PyObject *args;
{
	if (!PyArg_NoArgs(args))
		return NULL;
	return PyInt_FromLong((long)getpid());
}


static char efi_lseek__doc__[] =
"lseek(fd, pos, how) -> newpos\n\
Set the current position of a file descriptor.";

static PyObject *
efi_lseek(self, args)
	PyObject *self;
	PyObject *args;
{
	int fd, how;
	off_t pos, res;
	PyObject *posobj;
	if (!PyArg_Parse(args, "(iOi)", &fd, &posobj, &how))
		return NULL;
#ifdef SEEK_SET
	/* Turn 0, 1, 2 into SEEK_{SET,CUR,END} */
	switch (how) {
	case 0: how = SEEK_SET; break;
	case 1: how = SEEK_CUR; break;
	case 2: how = SEEK_END; break;
	}
#endif /* SEEK_END */

#if !defined(HAVE_LARGEFILE_SUPPORT)
	pos = PyInt_AsLong(posobj);
#else
	pos = PyLong_Check(posobj) ?
		PyLong_AsLongLong(posobj) : PyInt_AsLong(posobj);
#endif
	if (PyErr_Occurred())
		return NULL;

	Py_BEGIN_ALLOW_THREADS
	res = lseek(fd, pos, how);
	Py_END_ALLOW_THREADS
	if (res < 0)
		return efi_error();

#if !defined(HAVE_LARGEFILE_SUPPORT)
	return PyInt_FromLong((long) res);
#else
	return PyLong_FromLongLong(res);
#endif
}


static char efi_lstat__doc__[] =
"lstat(path) -> (mode,ino,dev,nlink,uid,gid,size,atime,mtime,ctime)\n\
Like stat(path), but do not follow symbolic links.";

static PyObject *
efi_lstat(self, args)
	PyObject *self;
	PyObject *args;
{
	return efi_do_stat(self, args, lstat);
}


static char efi_open__doc__[] =
"open(filename, flag [, mode=0777]) -> fd\n\
Open a file (for low level IO).";

static PyObject *
efi_open(self, args)
	PyObject *self;
	PyObject *args;
{
	char *file;
	int flag;
	int mode = 0777;
	int fd;
	if (!PyArg_ParseTuple(args, "si|i", &file, &flag, &mode))
		return NULL;

	Py_BEGIN_ALLOW_THREADS
	fd = open(file, flag, mode);
	Py_END_ALLOW_THREADS
	if (fd < 0)
		return efi_error_with_filename(file);
	return PyInt_FromLong((long)fd);
}


static char efi_read__doc__[] =
"read(fd, buffersize) -> string\n\
Read a file descriptor.";

static PyObject *
efi_read(self, args)
	PyObject *self;
	PyObject *args;
{
	int fd, size, n;
	PyObject *buffer;
	if (!PyArg_Parse(args, "(ii)", &fd, &size))
		return NULL;
	buffer = PyString_FromStringAndSize((char *)NULL, size);
	if (buffer == NULL)
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	n = (int)read(fd, PyString_AsString(buffer), size);
	Py_END_ALLOW_THREADS
	if (n < 0) {
		Py_DECREF(buffer);
		return efi_error();
	}
	if (n != size)
		_PyString_Resize(&buffer, n);
	return buffer;
}


static char efi_stat__doc__[] =
"stat(path) -> (mode,ino,dev,nlink,uid,gid,size,atime,mtime,ctime)\n\
Perform a stat system call on the given path.";

static PyObject *
efi_stat(self, args)
	PyObject *self;
	PyObject *args;
{
    // check here
    //Print(L"\nefi stat\n");
	return efi_do_stat(self, args, stat);
}


static char efi_unlink__doc__[] =
"unlink(path) -> None\n\
Remove a file (same as remove(path)).";

static char efi_remove__doc__[] =
"remove(path) -> None\n\
Remove a file (same as unlink(path)).";

static PyObject *
efi_unlink(self, args)
	PyObject *self;
	PyObject *args;
{
	return efi_1str(args, unlink);
}


static char efi_write__doc__[] =
"write(fd, string) -> byteswritten\n\
Write a string to a file descriptor.";

static PyObject *
efi_write(self, args)
	PyObject *self;
	PyObject *args;
{
	int fd, size;
	char *buffer;
	if (!PyArg_Parse(args, "(is#)", &fd, &buffer, &size))
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	size = (int)write(fd, buffer, size);
	Py_END_ALLOW_THREADS
	if (size < 0)
		return efi_error();
	return PyInt_FromLong((long)size);
}


static char efi_fdopen__doc__[] =
"fdopen(fd, [, mode='r' [, bufsize]]) -> file_object\n\
Return an open file object connected to a file descriptor.";

static PyObject *
efi_fdopen(self, args)
	PyObject *self;
	PyObject *args;
{
	extern int fclose Py_PROTO((FILE *));
	int fd;
	char *mode = "r";
	int bufsize = -1;
	FILE *fp;
	PyObject *f;
	if (!PyArg_ParseTuple(args, "i|si", &fd, &mode, &bufsize))
		return NULL;

	Py_BEGIN_ALLOW_THREADS
	fp = fdopen(fd, mode);
	Py_END_ALLOW_THREADS
	if (fp == NULL)
		return efi_error();
	f = PyFile_FromFile(fp, "(fdopen)", mode, fclose);
	if (f != NULL)
		PyFile_SetBufSize(f, bufsize);
	return f;
}

#if HAVE_DIRENT
static char efi_listdir__doc__[] =
"listdir(path) -> list_of_strings\n\
Return a list containing the names of the entries in the directory.\n\
\n\
	path: path of directory to list\n\
\n\
The list is in arbitrary order.  It does not include the special\n\
entries '.' and '..' even if they are present in the directory.";

static PyObject *
efi_listdir(self, args)
	PyObject *self;
	PyObject *args;
{
	char *name;
	PyObject *d, *v;
	DIR *dirp;
	struct dirent *ep;
	if (!PyArg_Parse(args, "s", &name))
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	if ((dirp = opendir(name)) == NULL) {
		Py_BLOCK_THREADS
		return efi_error();
	}
	if ((d = PyList_New(0)) == NULL) {
		closedir(dirp);
		Py_BLOCK_THREADS
		return NULL;
	}
	while ((ep = readdir(dirp)) != NULL) {
		if (ep->d_name[0] == '.' &&
		    (NAMLEN(ep) == 1 ||
		     (ep->d_name[1] == '.' && NAMLEN(ep) == 2)))
			continue;
		v = PyString_FromStringAndSize(ep->d_name, (int)NAMLEN(ep));
		if (v == NULL) {
			Py_DECREF(d);
			d = NULL;
			break;
		}
		if (PyList_Append(d, v) != 0) {
			Py_DECREF(v);
			Py_DECREF(d);
			d = NULL;
			break;
		}
		Py_DECREF(v);
	}
	closedir(dirp);
	Py_END_ALLOW_THREADS

	return d;
}
#endif /* HAVE_DIRENT */

#ifdef HAVE_SYSTEM
static char efi_system__doc__[] =
"system(command) -> exit_status\n\
Execute the command (a string) in a subshell.";

static PyObject *
efi_system(self, args)
	PyObject *self;
	PyObject *args;
{
	char *command;
	long sts;
	if (!PyArg_Parse(args, "s", &command))
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	sts = system(command);
	Py_END_ALLOW_THREADS
	return PyInt_FromLong(sts);
}
#endif

#ifdef HAVE_PUTENV
static char efi_putenv__doc__[] =
"putenv(key, value) -> None\n\
Change or add an environment variable.";

static PyObject * 
efi_putenv(PyObject *self, PyObject *args)
{
    char *s1, *s2;
    char *new;

	if (!PyArg_ParseTuple(args, "ss", &s1, &s2))
		return NULL;

	/* XXX This leaks memory -- not easy to fix :-( */
	if ((new = malloc(strlen(s1) + strlen(s2) + 2)) == NULL)
        return PyErr_NoMemory();
	(void) sprintf(new, "%s=%s", s1, s2);
	if (putenv(new)) {
        PyErr_SetString(PyExc_SystemError, "Error calling putenv");
        return NULL;
	}

	Py_INCREF(Py_None);
    return Py_None;
}
#endif /* putenv */

static char efi_getenv__doc__[] =
"getenv(key) -> value\n\
Get the value of an environment variable directly from the process' environment.";

static PyObject *
efi_getenv(PyObject *self, PyObject *args)
{
    char    *varName, *varValue;
    PyObject    *retValue = NULL;

    if (!PyArg_ParseTuple(args, "s", &varName)) {
        return NULL;
    }

    varValue = getenv(varName);
    if (varValue) {
        retValue = Py_BuildValue("s", varValue);
    } else {
        PyErr_SetString(PyExc_SystemError, "Error calling getenv");
    }
    return retValue;
}

static int
IsUnicodeStr( char *AsciiName, CHAR16 *Data, UINTN DataSize)
{
	UINTN i;

	for ( i = 0; i < DataSize / sizeof(CHAR16); i++ ) {
		if ( !iswprint(Data[ i ]) ) {
			//
			//  Sometimes the size includes the termination character
			//
			if ( (Data[ i ] == 0) && (i == ((DataSize / sizeof(CHAR16)) - 1)) )
				continue;
			else
				return ( FALSE );
		}
	}

	//
	//  Typecast will limit lengyth of string to be converted
	//
	wcstombs( AsciiName, Data, (UINT32)DataSize);
	return ( TRUE );
}

static int
IsAsciiStr( char *AsciiName, char *Data, UINTN DataSize)
{
	UINTN i;

	for ( i = 0; i < DataSize; i++ ) {
		if ( !isprint(Data[ i ]) ) {
			//
			//  Sometimes the size includes the termination character
			//
			if ( (Data[ i ] == 0) && (i == (DataSize - 1)) )
				continue;
			else
				return ( FALSE );
		}
	}

	strcpy( AsciiName, Data );
	return ( TRUE );
}

extern EFI_SYSTEM_TABLE *_LIBC_EFISystemTable;

char *
findenv( char *Name )
{
	EFI_STATUS				Status;
	CHAR16					*VariableName;
	void					*VariableData;
	char					*AsciiData;
	UINTN					VariableNameSize;
	UINTN					VariableDataSize;
	EFI_GUID				VendorGuid;
	EFI_RUNTIME_SERVICES	*pRT;
    int                     found;
    char                    AsciiName[MAX_ENVIRONMENT_NAME];

	pRT = _LIBC_EFISystemTable->RuntimeServices; 

	//
	//  Allocate memory for conversions
	//
	VariableName = (CHAR16*)calloc(MAX_ENVIRONMENT_NAME, sizeof(CHAR16));
	if ( VariableName == NULL ) {
		return NULL;
	}

	VariableData = (CHAR16*)calloc(MAX_ENVIRONMENT_DATA, sizeof(CHAR16));
	if ( VariableData == NULL ) {
		free( VariableName );
		return NULL;
	}

	AsciiData = (char*)calloc(MAX_ENVIRONMENT_DATA, sizeof(char));
	if ( AsciiData == NULL ) {
		free( VariableData );
		free( VariableName );
		return NULL;
	}

	//
	//  search all the variables
    //
    found = FALSE;
	do {
		VariableNameSize = MAX_ENVIRONMENT_NAME;
		Status = pRT->GetNextVariableName( &VariableNameSize,
										   VariableName,
										   &VendorGuid );
		if ( ! EFI_ERROR( Status )) {
            wcstombs(AsciiName, VariableName, MAX_ENVIRONMENT_NAME);
            if (strcasecmp(AsciiName, Name) == 0) {
			    VariableDataSize = MAX_ENVIRONMENT_DATA;
			    Status = pRT->GetVariable( VariableName,
									   &VendorGuid,
									   NULL,
									   &VariableDataSize,
									   VariableData );

			    if ( ! EFI_ERROR( Status ) ) {
				    //
				    //  EFI variables can be in ascii, unicode or raw data.
				    //  we are only interested in ascii and unicode strings
				    //
				    if ( IsUnicodeStr( AsciiData, VariableData, VariableDataSize) ||
					     IsAsciiStr( AsciiData, VariableData, VariableDataSize) ) {
			 
                        found = TRUE;
				    }
			    }
            }
		}
	} while (Status == EFI_SUCCESS && !found);

	free( VariableData );
	free( VariableName );
    if ( found ) {
        return ( AsciiData);
    } else {
	    free( AsciiData );
        return NULL;
    }
}

static char efi_GetVariable__doc__[] =
"GetVariable(name) -> value\n\
Get the value of an environment variable directly from the EFI variable store.";

static PyObject *
efi_GetVariable(PyObject *self, PyObject *args)
{
    char    *rawName;
    char    *AsciiValue;
    PyObject    *ReturnVal;

    if (!PyArg_ParseTuple(args, "s", &rawName)) {
        return NULL;
    }

    AsciiValue = findenv(rawName);
    if (AsciiValue) {
        ReturnVal = PyString_FromString(AsciiValue);
        free(AsciiValue);
    } else {
	    Py_INCREF(Py_None);
        ReturnVal = Py_None;
    }
    return ReturnVal;
}

static char efi_chdir__doc__[] =
"chdir(path) -> None\n\
Change the current working directory to the specified path.";

static PyObject *
efi_chdir(PyObject *self, PyObject *args)
{
    char    *path;
    int     result;

    if (!PyArg_ParseTuple(args, "s", &path)) {
        return NULL;
    }

    result = chdir(path);
    if (result < 0) {
        return PyErr_SetFromErrnoWithFilename(PyExc_SystemError, path);
    } else {
	    Py_INCREF(Py_None);
        return Py_None;
    }
}

static char efi_mkdir__doc__[] =
"mkdir(path [, mode=0777]) -> None\n\
Create a directory.";

static PyObject *
efi_mkdir(PyObject *self, PyObject *args)
{
	int res;
	char *path;
	mode_t mode = 0777;

    if (!PyArg_ParseTuple(args, "s|i", &path, &mode)) {
		return NULL;
    }
	res = mkdir(path, mode);
    if (res < 0) {
        return PyErr_SetFromErrnoWithFilename(PyExc_SystemError, path);
    } else {
	    Py_INCREF(Py_None);
	    return Py_None;
    }
}

static char efi_rename__doc__[] =
"rename(old, new) -> None\n\
Rename a file or directory.";

static PyObject *
efi_rename(PyObject *self, PyObject *args)
{
    char    *oldName, *newName;
    int     result;

    if (!PyArg_ParseTuple(args, "ss", &oldName, &newName)) {
        return NULL;
    }

    result = rename(oldName, newName);
    if (result < 0) {
        return PyErr_SetFromErrno(PyExc_SystemError);
    } else {
	    Py_INCREF(Py_None);
        return Py_None;
    }
}


static char efi_rmdir__doc__[] =
"rmdir(path) -> None\n\
Remove a directory.";

static PyObject *
efi_rmdir(PyObject *self, PyObject *args)
{
    char    *path;
    int     result;

    if (!PyArg_ParseTuple(args, "s", &path)) {
        return NULL;
    }

    result = rmdir(path);
    if (result < 0) {
        return PyErr_SetFromErrnoWithFilename(PyExc_SystemError, path);
    } else {
	    Py_INCREF(Py_None);
        return Py_None;
    }
}

static char efi_utime__doc__[] =
"utime(path, (atime, utime)) -> None\n\
Set the access and modified time of the file to the given values.";

static PyObject *
efi_utime(PyObject *self, PyObject *args)
{
	char *path;
	long atime, mtime;
	int res;

	struct utimbuf buf;
#define ATIME buf.actime
#define MTIME buf.modtime
#define UTIME_ARG &buf

    if (!PyArg_Parse(args, "(s(ll))", &path, &atime, &mtime)) {
		return NULL;
    }

	ATIME = atime;
	MTIME = mtime;
	res = utime(path, UTIME_ARG);

    if (res < 0) {
        return PyErr_SetFromErrnoWithFilename(PyExc_SystemError, path);
    } else {
	    Py_INCREF(Py_None);
        return Py_None;
    }
#undef UTIME_ARG
#undef ATIME
#undef MTIME
}

static char efi_getcwd__doc__[] =
"getcwd() -> path\n\
Return a string representing the current working directory.";

static PyObject *
efi_getcwd(PyObject *self, PyObject *args)
{
	char buf[1026];
	char *res;

    if (!PyArg_NoArgs(args))
		return NULL;

	res = getcwd(buf, sizeof buf);

    if (res == NULL) {
        return PyErr_SetFromErrno(PyExc_SystemError);
    } else {
	    return PyString_FromString(buf);
    }
}

static char efi_dup__doc__[] =
"dup(fd) -> fd2\n\
Return a duplicate of a file descriptor.";

static PyObject *
efi_dup(PyObject *self, PyObject *args)
{
	int fd;
	if (!PyArg_Parse(args, "i", &fd))
		return NULL;
	
	fd = dup(fd);
	
	if (fd < 0)
		return PyErr_SetFromErrno(PyExc_SystemError);
	return PyInt_FromLong((long)fd);
}


static char efi_dup2__doc__[] =
"dup2(fd, fd2) -> None\n\
Duplicate file descriptor.";

static PyObject *
efi_dup2(PyObject *self, PyObject *args)
{
	int fd, fd2, res;
	if (!PyArg_Parse(args, "(ii)", &fd, &fd2))
		return NULL;
	
	res = dup2(fd, fd2);
	
	if (res < 0)
		return PyErr_SetFromErrno(PyExc_SystemError);
	Py_INCREF(Py_None);
	return Py_None;
}


static PyMethodDef efi_methods[] = {
    // From unistd.h
    {"close",	efi_close,  0, efi_close__doc__},
	{"_exit",	efi__exit,  0, efi__exit__doc__},
	{"fstat",	efi_fstat,  0, efi_fstat__doc__},
	{"getpid",	efi_getpid, 0, efi_getpid__doc__},
	{"lseek",	efi_lseek,  0, efi_lseek__doc__},
	{"lstat",	efi_lstat,  0, efi_lstat__doc__},
	{"open",	efi_open,   1, efi_open__doc__},
	{"read",	efi_read,   0, efi_read__doc__},
	{"stat",	efi_stat,   0, efi_stat__doc__},
	{"unlink",	efi_unlink, 0, efi_unlink__doc__},
	{"write",	efi_write,  0, efi_write__doc__},

    // From stdio.h
	{"fdopen",	efi_fdopen,	1, efi_fdopen__doc__},
	{"remove",	efi_unlink, 0, efi_remove__doc__},
#ifdef HAVE_DIRENT
    // From dirent.h
    {"listdir",	efi_listdir, 0, efi_listdir__doc__},
#endif /* HAVE_DIRENT */

#ifdef HAVE_SYSTEM
    // From stdlib.h
    {"system",	efi_system, 0, efi_system__doc__},
#endif /* HAVE_DIRENT */
#ifdef HAVE_PUTENV
	{"putenv",	efi_putenv, 1, efi_putenv__doc__},
#endif
	{"getenv",	efi_getenv, 1, efi_getenv__doc__},
    {"GetVariable", efi_GetVariable, 1, efi_GetVariable__doc__},

    {"chdir",	efi_chdir,  1, efi_chdir__doc__},
	{"getcwd",	efi_getcwd, 0, efi_getcwd__doc__},
	{"mkdir",	efi_mkdir,  1, efi_mkdir__doc__},
	{"rename",	efi_rename, 0, efi_rename__doc__},
	{"rmdir",	efi_rmdir,  1, efi_rmdir__doc__},
	{"utime",	efi_utime,  0, efi_utime__doc__},
	{"dup",		efi_dup,    0, efi_dup__doc__},
	{"dup2",	efi_dup2,   0, efi_dup2__doc__},
	{NULL,		NULL}		 /* Sentinel */
};

static int
ins(d, symbol, value)
        PyObject* d;
        char* symbol;
        long value;
{
        PyObject* v = PyInt_FromLong(value);
        if (!v || PyDict_SetItemString(d, symbol, v) < 0)
                return -1;                   /* triggers fatal error */

        Py_DECREF(v);
        return 0;
}

static int
all_ins(d)
        PyObject* d;
{
#ifdef F_OK
        if (ins(d, "F_OK", (long)F_OK)) return -1;
#endif        
#ifdef R_OK
        if (ins(d, "R_OK", (long)R_OK)) return -1;
#endif        
#ifdef W_OK
        if (ins(d, "W_OK", (long)W_OK)) return -1;
#endif        
#ifdef X_OK
        if (ins(d, "X_OK", (long)X_OK)) return -1;
#endif        
#ifdef WNOHANG
        if (ins(d, "WNOHANG", (long)WNOHANG)) return -1;
#endif        
#ifdef O_RDONLY
        if (ins(d, "O_RDONLY", (long)O_RDONLY)) return -1;
#endif
#ifdef O_WRONLY
        if (ins(d, "O_WRONLY", (long)O_WRONLY)) return -1;
#endif
#ifdef O_RDWR
        if (ins(d, "O_RDWR", (long)O_RDWR)) return -1;
#endif
#ifdef O_NDELAY
        if (ins(d, "O_NDELAY", (long)O_NDELAY)) return -1;
#endif
#ifdef O_NONBLOCK
        if (ins(d, "O_NONBLOCK", (long)O_NONBLOCK)) return -1;
#endif
#ifdef O_APPEND
        if (ins(d, "O_APPEND", (long)O_APPEND)) return -1;
#endif
#ifdef O_DSYNC
        if (ins(d, "O_DSYNC", (long)O_DSYNC)) return -1;
#endif
#ifdef O_RSYNC
        if (ins(d, "O_RSYNC", (long)O_RSYNC)) return -1;
#endif
#ifdef O_SYNC
        if (ins(d, "O_SYNC", (long)O_SYNC)) return -1;
#endif
#ifdef O_NOCTTY
        if (ins(d, "O_NOCTTY", (long)O_NOCTTY)) return -1;
#endif
#ifdef O_CREAT
        if (ins(d, "O_CREAT", (long)O_CREAT)) return -1;
#endif
#ifdef O_EXCL
        if (ins(d, "O_EXCL", (long)O_EXCL)) return -1;
#endif
#ifdef O_TRUNC
        if (ins(d, "O_TRUNC", (long)O_TRUNC)) return -1;
#endif
#ifdef O_BINARY
        if (ins(d, "O_BINARY", (long)O_BINARY)) return -1;
#endif
#ifdef O_TEXT
        if (ins(d, "O_TEXT", (long)O_TEXT)) return -1;
#endif

#ifdef HAVE_SPAWNV
        if (ins(d, "P_WAIT", (long)_P_WAIT)) return -1;
        if (ins(d, "P_NOWAIT", (long)_P_NOWAIT)) return -1;
        if (ins(d, "P_OVERLAY", (long)_OLD_P_OVERLAY)) return -1;
        if (ins(d, "P_NOWAITO", (long)_P_NOWAITO)) return -1;
        if (ins(d, "P_DETACH", (long)_P_DETACH)) return -1;
#endif

        return 0;
}

DL_EXPORT(void)
initefi( void )
{
	PyObject *m, *d, *v;
	
	m = Py_InitModule4("efi",
			   efi_methods,
			   efi__doc__,
			   (PyObject *)NULL,
			   PYTHON_API_VERSION);
	
    d = PyModule_GetDict(m);

	/* Initialize environ dictionary */
	v = convertenviron();
	if (v == NULL || PyDict_SetItemString(d, "environ", v) != 0)
		return;
	
    Py_DECREF(v);
	
    if (all_ins(d))
        return;

	PyDict_SetItemString(d, "error", PyExc_OSError);
}
