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

    object.c

Abstract:

    This file implements wrappers for the functions declared in object.h.


Revision History

--*/


#include "EfiInterface.h"

extern EFI_PYTHON_API  *PythonApiFunctions;

/* These functions are wrappers around the function pointers in the EFI_PYTHON_API
 *  structure that make it so a module's source code doesn't have to be modified
 *  to be built as an dynamically loaded EFI driver.
 */

int PyObject_Print (PyObject *Object, FILE *fp, int i)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyObject_Print) {
        return PythonApiFunctions->EPA_PyObject_Print(Object, fp, i);
    } else {
        return 0;
    }
}

PyObject *PyObject_Repr (PyObject *Object)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyObject_Repr) {
        return PythonApiFunctions->EPA_PyObject_Repr(Object);
    } else {
        return NULL;
    }
}

PyObject *PyObject_Str (PyObject *Object)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyObject_Str) {
        return PythonApiFunctions->EPA_PyObject_Str(Object);
    } else {
        return NULL;
    }
}

int PyObject_Compare (PyObject *Object1, PyObject *Object2)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyObject_Compare) {
        return PythonApiFunctions->EPA_PyObject_Compare(Object1, Object2);
    } else {
        return 0;
    }
}

PyObject *PyObject_GetAttrString (PyObject *O, char *c)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyObject_GetAttrString) {
        return PythonApiFunctions->EPA_PyObject_GetAttrString(O, c);
    } else {
        return NULL;
    }
}

int PyObject_SetAttrString (PyObject *O1, char *c, PyObject *O2)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyObject_SetAttrString) {
        return PythonApiFunctions->EPA_PyObject_SetAttrString(O1, c, O2);
    } else {
        return 0;
    }
}

int PyObject_HasAttrString (PyObject *O, char *c)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyObject_HasAttrString) {
        return PythonApiFunctions->EPA_PyObject_HasAttrString(O, c);
    } else {
        return 0;
    }
}

PyObject *PyObject_GetAttr (PyObject *O1, PyObject *O2)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyObject_GetAttr) {
        return PythonApiFunctions->EPA_PyObject_GetAttr(O1, O2);
    } else {
        return NULL;
    }
}

int PyObject_SetAttr (PyObject *O1, PyObject *O2, PyObject *O3)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyObject_SetAttr) {
        return PythonApiFunctions->EPA_PyObject_SetAttr(O1, O2, O3);
    } else {
        return 0;
    }
}

int PyObject_HasAttr (PyObject *O1, PyObject *O2)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyObject_HasAttr) {
        return PythonApiFunctions->EPA_PyObject_HasAttr(O1, O2);
    } else {
        return 0;
    }
}

long PyObject_Hash (PyObject *O)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyObject_Hash) {
        return PythonApiFunctions->EPA_PyObject_Hash(O);
    } else {
        return 0;
    }
}

int PyObject_IsTrue (PyObject *O)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyObject_IsTrue) {
        return PythonApiFunctions->EPA_PyObject_IsTrue(O);
    } else {
        return 0;
    }
}

int PyObject_Not (PyObject *O)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyObject_Not) {
        return PythonApiFunctions->EPA_PyObject_Not(O);
    } else {
        return 0;
    }
}

int PyCallable_Check (PyObject *O)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyCallable_Check) {
        return PythonApiFunctions->EPA_PyCallable_Check(O);
    } else {
        return 0;
    }
}

int PyNumber_Coerce (PyObject **O1, PyObject **O2)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyNumber_Coerce) {
        return PythonApiFunctions->EPA_PyNumber_Coerce(O1, O2);
    } else {
        return 0;
    }
}

int PyNumber_CoerceEx (PyObject **O1, PyObject **O2)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyNumber_CoerceEx) {
        return PythonApiFunctions->EPA_PyNumber_CoerceEx(O1, O2);
    } else {
        return 0;
    }
}

int Py_ReprEnter (PyObject *O)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_Py_ReprEnter) {
        return PythonApiFunctions->EPA_Py_ReprEnter(O);
    } else {
        return 0;
    }
}

void Py_ReprLeave (PyObject *O)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_Py_ReprLeave) {
        PythonApiFunctions->EPA_Py_ReprLeave(O);
    }
}

#ifdef Py_TRACE_REFS
void _Py_Dealloc (PyObject *O)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA__Py_Dealloc) {
        PythonApiFunctions->EPA__Py_Dealloc(O);
    }
}

void _Py_NewReference (PyObject *O)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA__Py_NewReference) {
        PythonApiFunctions->EPA__Py_NewReference(O);
    }
}

void _Py_ForgetReference (PyObject *O)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA__Py_ForgetReference) {
        PythonApiFunctions->EPA__Py_ForgetReference(O);
    }
}

void _Py_PrintReferences (FILE *fp)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA__Py_PrintReferences) {
        PythonApiFunctions->EPA__Py_PrintReferences(fp);
    }
}
#endif

#ifdef COUNT_ALLOCS
void inc_count (PyTypeObject *O)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_inc_count) {
        PythonApiFunctions->EPA_inc_count(O);
    }
}
#endif

PyObject *PyObject_SelfIter(PyObject *obj)
{
    /* we can optimize this routine check, check here */
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyObject_SelfIter) {
        return PythonApiFunctions->EPA_PyObject_SelfIter( obj );
    } else {
        return 0;
    }
}

void
PyObject_Free(void *p)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyObject_Free) {
        PythonApiFunctions->EPA_PyObject_Free( p );
    }
}

PyObject *
PyObject_RichCompare(PyObject *v, PyObject *w, int op)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyObject_RichCompare) {
        return PythonApiFunctions->EPA_PyObject_RichCompare( v, w, op );
    } else {
        return 0;
    }
}

int
PyObject_RichCompareBool(PyObject *v, PyObject *w, int op)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyObject_RichCompareBool) {
        return PythonApiFunctions->EPA_PyObject_RichCompareBool( v, w, op );
    } else {
        return 0;
    }
}

void *
PyMem_Realloc(void *p, size_t nbytes)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyMem_Realloc) {
        return PythonApiFunctions->EPA_PyMem_Realloc( p, nbytes );
    } else {
        return 0;
    }
}

int
PyObject_GenericSetAttr(PyObject *obj, PyObject *name, PyObject *value)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyObject_GenericSetAttr) {
        return PythonApiFunctions->EPA_PyObject_GenericSetAttr( obj, name, value );
    } else {
        return 0;
    }
}

void *
PyMem_Malloc(size_t nbytes)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyMem_Malloc) {
        return PythonApiFunctions->EPA_PyMem_Malloc( nbytes );
    } else {
        return NULL;
    }
}

void
PyMem_Free(void *p)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyMem_Free) {
        PythonApiFunctions->EPA_PyMem_Free( p );
    }
}

void _PyObject_Dump(PyObject* op)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA__PyObject_Dump) {
        PythonApiFunctions->EPA__PyObject_Dump( op );
    } 
}

PyTypeObject *pPyType_Type          = NULL;
long     *_pPy_RefTotal             = NULL;
PyObject *_pPy_NoneStruct           = NULL;
PyObject *_pPy_NotImplementedStruct = NULL;

EFI_STATUS InitObject() {
    if (PythonApiFunctions) {
        pPyType_Type              = PythonApiFunctions->EPA_PyType_Type;
        _pPy_RefTotal             = PythonApiFunctions->EPA__Py_RefTotal;
        _pPy_NoneStruct           = PythonApiFunctions->EPA__Py_NoneStruct;
        _pPy_NotImplementedStruct = PythonApiFunctions->EPA__NotImplementedStruct;
        return EFI_SUCCESS;
    } else {
        return EFI_INVALID_PARAMETER;
    }
}

