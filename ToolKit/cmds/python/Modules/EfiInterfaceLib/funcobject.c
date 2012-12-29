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

    funcobject.c

Abstract:

    This file implements wrappers for the functions declared in funcobject.h.


Revision History

--*/


#include "EfiInterface.h"

extern EFI_PYTHON_API  *PythonApiFunctions;

/* These functions are wrappers around the function pointers in the EFI_PYTHON_API
 *  structure that make it so a module's source code doesn't have to be modified
 *  to be built as an dynamically loaded EFI driver.
 */

PyTypeObject *pPyFunction_Type = NULL;

PyObject * PyFunction_New (PyObject *O1, PyObject *O2)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyFunction_New) {
        return PythonApiFunctions->EPA_PyFunction_New(O1, O2);
    } else {
        return NULL;
    }
}

PyObject * PyFunction_GetCode (PyObject *O)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyFunction_GetCode) {
        return PythonApiFunctions->EPA_PyFunction_GetCode(O);
    } else {
        return NULL;
    }
}

PyObject * PyFunction_GetGlobals (PyObject *O)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyFunction_GetGlobals) {
        return PythonApiFunctions->EPA_PyFunction_GetGlobals(O);
    } else {
        return NULL;
    }
}

PyObject * PyFunction_GetDefaults (PyObject *O)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyFunction_GetDefaults) {
        return PythonApiFunctions->EPA_PyFunction_GetDefaults(O);
    } else {
        return NULL;
    }
}

int PyFunction_SetDefaults (PyObject *O1, PyObject *O2)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyFunction_SetDefaults) {
        return PythonApiFunctions->EPA_PyFunction_SetDefaults(O1, O2);
    } else {
        return 0;
    }
}

EFI_STATUS InitFuncobject() {
    if (PythonApiFunctions) {
        pPyFunction_Type = PythonApiFunctions->EPA_PyFunction_Type;
        return EFI_SUCCESS;
    } else {
        return EFI_INVALID_PARAMETER;
    }
}
