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

    floatobject.c

Abstract:

    This file implements wrappers for the functions declared in floatobject.h.


Revision History

--*/


#include "EfiInterface.h"

extern EFI_PYTHON_API  *PythonApiFunctions;

/* These functions are wrappers around the function pointers in the EFI_PYTHON_API
 *  structure that make it so a module's source code doesn't have to be modified
 *  to be built as an dynamically loaded EFI driver.
 */

PyTypeObject *pPyTuple_Type = NULL;

PyObject * PyTuple_New (int size)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyTuple_New) {
        return PythonApiFunctions->EPA_PyTuple_New(size);
    } else {
        return NULL;
    }
}
int PyTuple_Size (PyObject * O)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyTuple_Size) {
        return PythonApiFunctions->EPA_PyTuple_Size(O);
    } else {
        return 0;
    }
}
PyObject * PyTuple_GetItem (PyObject *O, int i)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyTuple_GetItem) {
        return PythonApiFunctions->EPA_PyTuple_GetItem(O, i);
    } else {
        return NULL;
    }
}
int PyTuple_SetItem (PyObject *O1, int i, PyObject *O2)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyTuple_SetItem) {
        return PythonApiFunctions->EPA_PyTuple_SetItem(O1, i, O2);
    } else {
        return 0;
    }
}
PyObject * PyTuple_GetSlice (PyObject *O, int i1, int i2)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyTuple_GetSlice) {
        return PythonApiFunctions->EPA_PyTuple_GetSlice(O, i1, i2);
    } else {
        return NULL;
    }
}
int _PyTuple_Resize (PyObject **O, int i1 )
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyTuple_Resize) {
        return PythonApiFunctions->EPA_PyTuple_Resize(O, i1);
    } else {
        return 0;
    }
}

PyObject *
PyTuple_Pack(int n, ...)
{
    PyObject *rc;
	va_list va;

	va_start(va, n);
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyTuple_Pack) {
        rc = PythonApiFunctions->EPA_PyTuple_Pack(n, va);
    } else {
        rc = 0;
    }

	va_end(va);
	return rc;
}

EFI_STATUS InitTupleobject() {
    if (PythonApiFunctions) {
        pPyTuple_Type = PythonApiFunctions->EPA_PyTuple_Type;
        return EFI_SUCCESS;
    } else {
        return EFI_INVALID_PARAMETER;
    }
}
