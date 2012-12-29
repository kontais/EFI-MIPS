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

    dictobject.c

Abstract:

    This file implements wrappers for the functions declared in pyerrors.h.


Revision History

--*/


#include "EfiInterface.h"

extern EFI_PYTHON_API  *PythonApiFunctions;

/* These functions are wrappers around the function pointers in the EFI_PYTHON_API
 *  structure that make it so a module's source code doesn't have to be modified
 *  to be built as an dynamically loaded EFI driver.
 */

PyTypeObject *pPyCFunction_Type = NULL;

PyCFunction PyCFunction_GetFunction (PyObject *O)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyCFunction_GetFunction) {
        return PythonApiFunctions->EPA_PyCFunction_GetFunction(O);
    } else {
        return NULL;
    }
}

PyObject * PyCFunction_GetSelf (PyObject *O)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyCFunction_GetSelf) {
        return PythonApiFunctions->EPA_PyCFunction_GetSelf(O);
    } else {
        return NULL;
    }
}

int PyCFunction_GetFlags (PyObject *O)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyCFunction_GetFlags) {
        return PythonApiFunctions->EPA_PyCFunction_GetFlags(O);
    } else {
        return 0;
    }
}

PyObject * Py_FindMethod(PyMethodDef M[], PyObject *O, char *c)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_Py_FindMethod) {
        return PythonApiFunctions->EPA_Py_FindMethod(M, O, c);
    } else {
        return NULL;
    }
}

PyObject * PyCFunction_New(PyMethodDef *M, PyObject *O)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyCFunction_New) {
        return PythonApiFunctions->EPA_PyCFunction_New(M, O);
    } else {
        return NULL;
    }
}

PyObject * Py_FindMethodInChain(PyMethodChain *M, PyObject *O, char *c)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_Py_FindMethodInChain) {
        return PythonApiFunctions->EPA_Py_FindMethodInChain(M, O, c);
    } else {
        return NULL;
    }
}

EFI_STATUS InitMethodobject(void)
{
    if (PythonApiFunctions) {
        pPyCFunction_Type = PythonApiFunctions->EPA_PyCFunction_Type;
        return EFI_SUCCESS;
    } else {
        return EFI_INVALID_PARAMETER;
    }
}
