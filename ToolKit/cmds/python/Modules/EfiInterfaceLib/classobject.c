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

    classobject.c

Abstract:

    This file implements wrappers for the functions declared in classobject.h.


Revision History

--*/


#include "EfiInterface.h"

extern EFI_PYTHON_API  *PythonApiFunctions;

/* These functions are wrappers around the function pointers in the EFI_PYTHON_API
 *  structure that make it so a module's source code doesn't have to be modified
 *  to be built as an dynamically loaded EFI driver.
 */

PyTypeObject *pPyClass_Type = NULL, *pPyInstance_Type = NULL, *pPyMethod_Type = NULL;

PyObject * PyClass_New (PyObject *O1, PyObject *O2, PyObject *O3)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyClass_New) {
        return PythonApiFunctions->EPA_PyClass_New(O1, O2, O3);
    } else {
        return NULL;
    }
}

PyObject * PyInstance_New (PyObject *O1, PyObject *O2, PyObject *O3)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyInstance_New) {
        return PythonApiFunctions->EPA_PyInstance_New(O1, O2, O3);
    } else {
        return NULL;
    }
}

PyObject * PyMethod_New (PyObject *O1, PyObject *O2, PyObject *O3)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyMethod_New) {
        return PythonApiFunctions->EPA_PyMethod_New(O1, O2, O3);
    } else {
        return NULL;
    }
}

PyObject * PyMethod_Function (PyObject *O)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyMethod_Function) {
        return PythonApiFunctions->EPA_PyMethod_Function(O);
    } else {
        return NULL;
    }
}

PyObject * PyMethod_Self (PyObject *O)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyMethod_Self) {
        return PythonApiFunctions->EPA_PyMethod_Self(O);
    } else {
        return NULL;
    }
}

PyObject * PyMethod_Class (PyObject *O)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyMethod_Class) {
        return PythonApiFunctions->EPA_PyMethod_Class(O);
    } else {
        return NULL;
    }
}

int PyClass_IsSubclass (PyObject *O1, PyObject *O2)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyClass_IsSubclass) {
        return PythonApiFunctions->EPA_PyClass_IsSubclass(O1, O2);
    } else {
        return 0;
    }
}

/* when upgrading to Python 2.4 */
#if 0
PyObject * PyInstance_DoBinOp (PyObject *O1, PyObject *O2,
		  char *c1, char *c2,
		  PyObject * (*f) (PyObject *, PyObject *) )
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyInstance_DoBinOp) {
        return PythonApiFunctions->EPA_PyInstance_DoBinOp(O1, O2, c1, c2, f);
    } else {
        return NULL;
    }
}
#endif

PyObject *
PyInstance_NewRaw(PyObject *klass, PyObject *dict)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyInstance_NewRaw) {
        return PythonApiFunctions->EPA_PyInstance_NewRaw(klass, dict);
    } else {
        return NULL;
    }
}

PyObject *
_PyInstance_Lookup(PyObject *pinst, PyObject *name)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA__PyInstance_Lookup) {
        return PythonApiFunctions->EPA__PyInstance_Lookup(pinst, name);
    } else {
        return NULL;
    }
}

EFI_STATUS InitClassobject() {
    if (PythonApiFunctions) {
        pPyClass_Type = PythonApiFunctions->EPA_PyClass_Type;
        pPyInstance_Type = PythonApiFunctions->EPA_PyInstance_Type;
        pPyMethod_Type = PythonApiFunctions->EPA_PyMethod_Type;
        return EFI_SUCCESS;
    } else {
        return EFI_INVALID_PARAMETER;
    }
}
