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

    intobject.c

Abstract:

    This file implements wrappers for the functions declared in intobject.h.


Revision History

--*/


#include "EfiInterface.h"

extern EFI_PYTHON_API  *PythonApiFunctions;

/* These functions are wrappers around the function pointers in the EFI_PYTHON_API
 *  structure that make it so a module's source code doesn't have to be modified
 *  to be built as an dynamically loaded EFI driver.
 */

PyTypeObject *pPyInt_Type = NULL;

PyObject * PyInt_FromLong (long l)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyInt_FromLong) {
        return PythonApiFunctions->EPA_PyInt_FromLong(l);
    } else {
        return NULL;
    }
}
long PyInt_AsLong (PyObject *O)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyInt_AsLong) {
        return PythonApiFunctions->EPA_PyInt_AsLong(O);
    } else {
        return 0L;
    }
}
long PyInt_GetMax (void)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyInt_GetMax) {
        return PythonApiFunctions->EPA_PyInt_GetMax();
    } else {
        return 0L;
    }
}
PyIntObject *_pPy_ZeroStruct = NULL;

PyIntObject *_pPy_TrueStruct = NULL;

unsigned long PyOS_strtoul (char *c1, char **c2, int i)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyOS_strtoul) {
        return PythonApiFunctions->EPA_PyOS_strtoul(c1, c2, i);
    } else {
        return 0L;
    }
}
long PyOS_strtol (char *c1, char **c2, int i)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyOS_strtol) {
        return PythonApiFunctions->EPA_PyOS_strtol(c1, c2, i);
    } else {
        return 0L;
    }
}


EFI_STATUS InitIntobject(void)
{
    if (PythonApiFunctions) {
        pPyInt_Type = PythonApiFunctions->EPA_PyInt_Type;
        _pPy_ZeroStruct = PythonApiFunctions->EPA__Py_ZeroStruct;
        _pPy_TrueStruct = PythonApiFunctions->EPA__Py_TrueStruct;
        return EFI_SUCCESS;
    } else {
        return EFI_INVALID_PARAMETER;
    }
}
