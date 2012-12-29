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

    longobject.c

Abstract:

    This file implements wrappers for the functions declared in longobject.h.


Revision History

--*/


#include "EfiInterface.h"

extern EFI_PYTHON_API  *PythonApiFunctions;

/* These functions are wrappers around the function pointers in the EFI_PYTHON_API
 *  structure that make it so a module's source code doesn't have to be modified
 *  to be built as an dynamically loaded EFI driver.
 */

PyTypeObject *pPyLong_Type = NULL;

PyObject * PyLong_FromLong (long l)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyLong_FromLong) {
        return PythonApiFunctions->EPA_PyLong_FromLong(l);
    } else {
        return NULL;
    }
}

PyObject * PyLong_FromUnsignedLong (unsigned long l)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyLong_FromUnsignedLong) {
        return PythonApiFunctions->EPA_PyLong_FromUnsignedLong(l);
    } else {
        return NULL;
    }
}

PyObject * PyLong_FromDouble (double d)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyLong_FromDouble) {
        return PythonApiFunctions->EPA_PyLong_FromDouble(d);
    } else {
        return NULL;
    }
}

long PyLong_AsLong (PyObject *O)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyLong_AsLong) {
        return PythonApiFunctions->EPA_PyLong_AsLong(O);
    } else {
        return 0L;
    }
}

unsigned long PyLong_AsUnsignedLong(PyObject *O)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyLong_AsUnsignedLong) {
        return PythonApiFunctions->EPA_PyLong_AsUnsignedLong(O);
    } else {
        return 0L;
    }
}

double PyLong_AsDouble (PyObject *O)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyLong_AsDouble) {
        return PythonApiFunctions->EPA_PyLong_AsDouble(O);
    } else {
        return 0.0;
    }
}

PyObject * PyLong_FromVoidPtr (void *v)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyLong_FromVoidPtr) {
        return PythonApiFunctions->EPA_PyLong_FromVoidPtr(v);
    } else {
        return NULL;
    }
}

void * PyLong_AsVoidPtr (PyObject *O)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyLong_AsVoidPtr) {
        return PythonApiFunctions->EPA_PyLong_AsVoidPtr(O);
    } else {
        return NULL;
    }
}


#ifdef HAVE_LONG_LONG
PyObject * PyLong_FromLongLong (LONG_LONG L)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyLong_FromLongLong) {
        return PythonApiFunctions->EPA_PyLong_FromLongLong(L);
    } else {
        return NULL;
    }
}

PyObject * PyLong_FromUnsignedLongLong (unsigned LONG_LONG L)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyLong_FromUnsignedLongLong) {
        return PythonApiFunctions->EPA_PyLong_FromUnsignedLongLong(L);
    } else {
        return NULL;
    }
}

LONG_LONG PyLong_AsLongLong (PyObject *O)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyLong_AsLongLong) {
        return PythonApiFunctions->EPA_PyLong_AsLongLong(O);
    } else {
        return (LONG_LONG) 0;
    }
}

unsigned LONG_LONG PyLong_AsUnsignedLongLong (PyObject *O)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyLong_AsUnsignedLongLong) {
        return PythonApiFunctions->EPA_PyLong_AsUnsignedLongLong(O);
    } else {
        return (LONG_LONG) 0;
    }
}

#endif /* HAVE_LONG_LONG */

PyObject * PyLong_FromString (char *c1, char **c2, int i)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyLong_FromString) {
        return PythonApiFunctions->EPA_PyLong_FromString(c1, c2, i);
    } else {
        return NULL;
    }
}

int
_PyLong_AsByteArray(PyLongObject* v,
		    unsigned char* bytes, size_t n,
		    int little_endian, int is_signed)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA__PyLong_AsByteArray) {
        return PythonApiFunctions->EPA__PyLong_AsByteArray(v, bytes, n, little_endian, is_signed);
    } else {
        return 0;
    }
}

size_t
_PyLong_NumBits(PyObject *vv)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA__PyLong_NumBits) {
        return PythonApiFunctions->EPA__PyLong_NumBits(vv);
    } else {
        return 0;
    }
}

int
_PyLong_Sign(PyObject *vv)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA__PyLong_Sign) {
        return PythonApiFunctions->EPA__PyLong_Sign(vv);
    } else {
        return 0;
    }
}

PyObject *
_PyLong_FromByteArray(const unsigned char* bytes, size_t n,
		      int little_endian, int is_signed)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA__PyLong_FromByteArray) {
        return PythonApiFunctions->EPA__PyLong_FromByteArray(bytes, n, little_endian, is_signed);
    } else {
        return NULL;
    }
}

double
_PyLong_AsScaledDouble(PyObject *vv, int *exponent)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA__PyLong_AsScaledDouble) {
        return PythonApiFunctions->EPA__PyLong_AsScaledDouble(vv, exponent);
    } else {
        return 0.0;
    }
}

EFI_STATUS InitLongobject() {
    if (PythonApiFunctions) {
        pPyLong_Type = PythonApiFunctions->EPA_PyLong_Type;
        return EFI_SUCCESS;
    } else {
        return EFI_INVALID_PARAMETER;
    }
}
