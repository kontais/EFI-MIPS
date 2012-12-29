/* Return the copyright string.  This is updated manually. */

#include "Python.h"

static char cprt[] = 
"\
Copyright (c) 2001-2005 Python Software Foundation.\n\r\
All Rights Reserved.\n\r\
\n\r\
Copyright (c) 2000 BeOpen.com.\n\r\
All Rights Reserved.\n\r\
\n\r\
Copyright (c) 1995-2001 Corporation for National Research Initiatives.\n\r\
All Rights Reserved.\n\r\
\n\r\
Copyright (c) 1991-1995 Stichting Mathematisch Centrum, Amsterdam.\n\r\
All Rights Reserved.\n\r\
\n\r\
Copyright (c) 1999-2006 Intel Corporation.\n\r\
All Rights Reserved.";

const char *
Py_GetCopyright(void)
{
	return cprt;
}
