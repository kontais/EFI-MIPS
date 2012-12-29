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

    ceval.c

Abstract:

    This file implements wrappers for the functions declared in ceval.h.


Revision History

--*/


#include "EfiInterface.h"

extern EFI_PYTHON_API  *PythonApiFunctions;

/* These functions are wrappers around the function pointers in the EFI_PYTHON_API
 *  structure that make it so a module's source code doesn't have to be modified
 *  to be built as an dynamically loaded EFI driver.
 */

PyObject * PyEval_CallObjectWithKeywords (PyObject *O1, PyObject *O2, PyObject *O3)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyEval_CallObjectWithKeywords) {
        return PythonApiFunctions->EPA_PyEval_CallObjectWithKeywords(O1, O2, O3);
    } else {
        return NULL;
    }
}

#if 0
PyObject * PyEval_CallObject (PyObject *O1, PyObject *O2)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyEval_CallObject) {
        return PythonApiFunctions->EPA_PyEval_CallObject(O1, O2);
    } else {
        return NULL;
    }
}
#endif

#if 0
PyObject * PyEval_CallFunction (PyObject *obj, char *format, ...)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyEval_CallFunction) {
        return PythonApiFunctions->EPA_PyEval_CallFunction(obj, format, ...);
    } else {
        return NULL;
    }
}

PyObject * PyEval_CallMethod (PyObject *obj, char *methodname, char *format, ...)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyEval_CallMethod) {
        return PythonApiFunctions->EPA_PyEval_CallMethod(obj, methodname, format, ...);
    } else {
        return NULL;
    }
}
#endif

PyObject * PyEval_GetBuiltins (void)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyEval_GetBuiltins) {
        return PythonApiFunctions->EPA_PyEval_GetBuiltins();
    } else {
        return NULL;
    }
}

PyObject * PyEval_GetGlobals (void)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyEval_GetGlobals) {
        return PythonApiFunctions->EPA_PyEval_GetGlobals();
    } else {
        return NULL;
    }
}

PyObject * PyEval_GetLocals (void)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyEval_GetLocals) {
        return PythonApiFunctions->EPA_PyEval_GetLocals();
    } else {
        return NULL;
    }
}

struct _frame * PyEval_GetFrame (void)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyEval_GetFrame) {
        return PythonApiFunctions->EPA_PyEval_GetFrame();
    } else {
        return NULL;
    }
}

int PyEval_GetRestricted (void)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyEval_GetRestricted) {
        return PythonApiFunctions->EPA_PyEval_GetRestricted();
    } else {
        return 0;
    }
}

int Py_FlushLine (void)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_Py_FlushLine) {
        return PythonApiFunctions->EPA_Py_FlushLine();
    } else {
        return 0;
    }
}

int Py_AddPendingCall (int (*func)(void *), void *arg)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_Py_AddPendingCall) {
        return PythonApiFunctions->EPA_Py_AddPendingCall(func, arg);
    } else {
        return 0;
    }
}

int Py_MakePendingCalls (void)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_Py_MakePendingCalls) {
        return PythonApiFunctions->EPA_Py_MakePendingCalls();
    } else {
        return 0;
    }
}

PyThreadState * PyEval_SaveThread (void)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyEval_SaveThread) {
        return PythonApiFunctions->EPA_PyEval_SaveThread();
    } else {
        return NULL;
    }
}

void PyEval_RestoreThread (PyThreadState *T)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyEval_RestoreThread) {
        PythonApiFunctions->EPA_PyEval_RestoreThread(T);
    }
}

#ifdef WITH_THREAD
void PyEval_InitThreads (void)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyEval_InitThreads) {
        PythonApiFunctions->EPA_PyEval_InitThreads();
    }
}

void PyEval_AcquireLock (void)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyEval_AcquireLock) {
        PythonApiFunctions->EPA_PyEval_AcquireLock();
    }
}

void PyEval_ReleaseLock (void)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyEval_ReleaseLock) {
        PythonApiFunctions->EPA_PyEval_ReleaseLock();
    }
}

void PyEval_AcquireThread (PyThreadState *tstate)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyEval_AcquireThread) {
        PythonApiFunctions->EPA_PyEval_AcquireThread(tstate);
    }
}

void PyEval_ReleaseThread (PyThreadState *tstate)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyEval_ReleaseThread) {
        PythonApiFunctions->EPA_PyEval_ReleaseThread(tstate);
    }
}

#endif /* !WITH_THREAD */

int Py_GetRecursionLimit( void )
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_Py_GetRecursionLimit) {
        return PythonApiFunctions->EPA_Py_GetRecursionLimit();
    } else {
        return 0;
    }
}

EFI_STATUS InitCeval() {
    if (PythonApiFunctions) {
        return EFI_SUCCESS;
    } else {
        return EFI_INVALID_PARAMETER;
    }
}
