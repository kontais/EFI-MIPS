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

/*++

Copyright (c) 2000  Intel Corporation

Module Name:

    stringobject.c

Abstract:

    This file implements wrappers for the functions declared in stringobject.h.


Revision History

--*/


#include "EfiInterface.h"

extern EFI_PYTHON_API  *PythonApiFunctions;

/* These functions are wrappers around the function pointers in the EFI_PYTHON_API
 *  structure that make it so a module's source code doesn't have to be modified
 *  to be built as an dynamically loaded EFI driver.
 */

PyTypeObject *pPyString_Type = NULL;

PyObject * PyString_FromStringAndSize (const char *c, int i)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyString_FromStringAndSize) {
        return PythonApiFunctions->EPA_PyString_FromStringAndSize(c, i);
    } else {
        return NULL;
    }
}

PyObject * PyString_FromString (const char *c)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyString_FromString) {
        return PythonApiFunctions->EPA_PyString_FromString(c);
    } else {
        return NULL;
    }
}

int PyString_Size (PyObject *O)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyString_Size) {
        return PythonApiFunctions->EPA_PyString_Size(O);
    } else {
        return 0;
    }
}

char * PyString_AsString (PyObject *O)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyString_AsString) {
        return PythonApiFunctions->EPA_PyString_AsString(O);
    } else {
        return NULL;
    }
}

void PyString_Concat (PyObject **O1, PyObject *O2)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyString_Concat) {
        PythonApiFunctions->EPA_PyString_Concat(O1, O2);
    }
}

void PyString_ConcatAndDel (PyObject **O1, PyObject *O2)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyString_ConcatAndDel) {
        PythonApiFunctions->EPA_PyString_ConcatAndDel(O1, O2);
    }
}

int _PyString_Resize (PyObject **O, int i)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA__PyString_Resize) {
        return PythonApiFunctions->EPA__PyString_Resize(O, i);
    } else {
        return 0;
    }
}

PyObject * PyString_Format (PyObject *O1, PyObject *O2)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyString_Format) {
        return PythonApiFunctions->EPA_PyString_Format(O1, O2);
    } else {
        return NULL;
    }
}


void PyString_InternInPlace (PyObject **O)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyString_InternInPlace) {
        PythonApiFunctions->EPA_PyString_InternInPlace(O);
    }
}

PyObject * PyString_InternFromString (const char *c)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyString_InternFromString) {
        return PythonApiFunctions->EPA_PyString_InternFromString(c);
    } else {
        return NULL;
    }
}

PyObject *PyString_DecodeEscape(const char *s,
				int len,
				const char *errors,
				int unicode,
				const char *recode_encoding)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyString_DecodeEscape) {
        return PythonApiFunctions->EPA_PyString_DecodeEscape(s, len, errors, unicode, recode_encoding);
    } else {
        return NULL;
    }
}

int
PyString_AsStringAndSize(register PyObject *obj,
			 register char **s,
			 register int *len)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyString_AsStringAndSize) {
        return PythonApiFunctions->EPA_PyString_AsStringAndSize(obj, s, len);
    } else {
        return 0;
    }
}

EFI_STATUS InitStringobject() {
    if (PythonApiFunctions) {
        pPyString_Type = PythonApiFunctions->EPA_PyString_Type;
        return EFI_SUCCESS;
    } else {
        return EFI_INVALID_PARAMETER;
    }
}
