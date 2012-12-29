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

    listobject.c

Abstract:

    This file implements wrappers for the functions declared in listobject.h.


Revision History

--*/


#include "EfiInterface.h"

extern EFI_PYTHON_API  *PythonApiFunctions;

/* These functions are wrappers around the function pointers in the EFI_PYTHON_API
 *  structure that make it so a module's source code doesn't have to be modified
 *  to be built as an dynamically loaded EFI driver.
 */

PyTypeObject *pPyList_Type = NULL;

PyObject * PyList_New (int size)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyList_New) {
        return PythonApiFunctions->EPA_PyList_New(size);
    } else {
        return NULL;
    }
}

int PyList_Size (PyObject *O)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyList_Size) {
        return PythonApiFunctions->EPA_PyList_Size(O);
    } else {
        return 0;
    }
}

PyObject * PyList_GetItem (PyObject *O, int i)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyList_GetItem) {
        return PythonApiFunctions->EPA_PyList_GetItem(O, i);
    } else {
        return NULL;
    }
}

int PyList_SetItem (PyObject *O1, int i, PyObject *O2)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyList_SetItem) {
        return PythonApiFunctions->EPA_PyList_SetItem(O1, i, O2);
    } else {
        return 0;
    }
}

int PyList_Insert (PyObject *O1, int i, PyObject *O2)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyList_Insert) {
        return PythonApiFunctions->EPA_PyList_Insert(O1, i, O2);
    } else {
        return 0;
    }
}

int PyList_Append (PyObject *O1, PyObject *O2)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyList_Append) {
        return PythonApiFunctions->EPA_PyList_Append(O1, O2);
    } else {
        return 0;
    }
}

PyObject * PyList_GetSlice (PyObject *O, int i1, int i2)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyList_GetSlice) {
        return PythonApiFunctions->EPA_PyList_GetSlice(O, i1, i2);
    } else {
        return NULL;
    }
}

int PyList_SetSlice (PyObject *O1, int i1, int i2, PyObject *O2)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyList_SetSlice) {
        return PythonApiFunctions->EPA_PyList_SetSlice(O1, i1, i2, O2);
    } else {
        return 0;
    }
}

int PyList_Sort (PyObject *O)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyList_Sort) {
        return PythonApiFunctions->EPA_PyList_Sort(O);
    } else {
        return 0;
    }
}

int PyList_Reverse (PyObject *O)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyList_Reverse) {
        return PythonApiFunctions->EPA_PyList_Reverse(O);
    } else {
        return 0;
    }
}

PyObject * PyList_AsTuple (PyObject *O)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyList_AsTuple) {
        return PythonApiFunctions->EPA_PyList_AsTuple(O);
    } else {
        return NULL;
    }
}


EFI_STATUS InitListobject(void)
{
    if (PythonApiFunctions) {
        pPyList_Type = PythonApiFunctions->EPA_PyList_Type;
        return EFI_SUCCESS;
    } else {
        return EFI_INVALID_PARAMETER;
    }
}
