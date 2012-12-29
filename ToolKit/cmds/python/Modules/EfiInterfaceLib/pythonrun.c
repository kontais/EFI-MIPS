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

    pythonrun.c

Abstract:

    This file implements wrappers for the functions declared in pythonrun.h.


Revision History

--*/


#include "EfiInterface.h"

extern EFI_PYTHON_API  *PythonApiFunctions;

/* These functions are wrappers around the function pointers in the EFI_PYTHON_API
 *  structure that make it so a module's source code doesn't have to be modified
 *  to be built as an dynamically loaded EFI driver.
 */

void Py_SetProgramName (char *c)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_Py_SetProgramName) {
        PythonApiFunctions->EPA_Py_SetProgramName(c);
    }
}

char * Py_GetProgramName (void)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_Py_GetProgramName) {
        return PythonApiFunctions->EPA_Py_GetProgramName();
    } else {
        return NULL;
    }
}

void Py_SetPythonHome (char *c)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_Py_SetPythonHome) {
        PythonApiFunctions->EPA_Py_SetPythonHome(c);
    }
}

char * Py_GetPythonHome (void)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_Py_GetPythonHome) {
        return PythonApiFunctions->EPA_Py_GetPythonHome();
    } else {
        return NULL;
    }
}

void Py_Initialize (void)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_Py_Initialize) {
        PythonApiFunctions->EPA_Py_Initialize();
    }
}

void Py_Finalize (void)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_Py_Finalize) {
        PythonApiFunctions->EPA_Py_Finalize();
   }
}

int Py_IsInitialized (void)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_Py_IsInitialized) {
        return PythonApiFunctions->EPA_Py_IsInitialized();
    } else {
        return 0;
    }
}

PyThreadState * Py_NewInterpreter (void)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_Py_NewInterpreter) {
        return PythonApiFunctions->EPA_Py_NewInterpreter();
    } else {
        return NULL;
    }
}

void Py_EndInterpreter (PyThreadState *T)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_Py_EndInterpreter) {
        PythonApiFunctions->EPA_Py_EndInterpreter(T);
    }
}

int PyRun_AnyFile (FILE *F, const char *c)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyRun_AnyFile) {
        return PythonApiFunctions->EPA_PyRun_AnyFile(F, c);
    } else {
        return 0;
    }
}

int PyRun_SimpleString (const char *c)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyRun_SimpleString) {
        return PythonApiFunctions->EPA_PyRun_SimpleString(c);
    } else {
        return 0;
    }
}

int PyRun_SimpleFile (FILE *F, const char *c)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyRun_SimpleFile) {
        return PythonApiFunctions->EPA_PyRun_SimpleFile(F, c);
    } else {
        return 0;
    }
}

int PyRun_InteractiveOne (FILE *F, const char *c)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyRun_InteractiveOne) {
        return PythonApiFunctions->EPA_PyRun_InteractiveOne(F, c);
    } else {
        return 0;
    }
}

int PyRun_InteractiveLoop (FILE *F, const char *c)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyRun_InteractiveLoop) {
        return PythonApiFunctions->EPA_PyRun_InteractiveLoop(F, c);
    } else {
        return 0;
    }
}

struct _node * PyParser_SimpleParseString (const char *c, int i)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyParser_SimpleParseString) {
        return PythonApiFunctions->EPA_PyParser_SimpleParseString(c, i);
    } else {
        return NULL;
    }
}

struct _node * PyParser_SimpleParseFile (FILE *F, const char *c, int i)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyParser_SimpleParseFile) {
        return PythonApiFunctions->EPA_PyParser_SimpleParseFile(F, c, i);
    } else {
        return NULL;
    }
}

PyObject * PyRun_String (const char *c, int i, PyObject *O1, PyObject *O2)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyRun_String) {
        return PythonApiFunctions->EPA_PyRun_String(c, i, O1, O2);
    } else {
        return NULL;
    }
}

PyObject * PyRun_File (FILE *F, const char *c, int i, PyObject *O1, PyObject *O2)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyRun_File) {
        return PythonApiFunctions->EPA_PyRun_File(F, c, i, O1, O2);
    } else {
        return NULL;
    }
}

PyObject * Py_CompileString (const char *c1, const char *c2, int i)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_Py_CompileString) {
        return PythonApiFunctions->EPA_Py_CompileString(c1, c2, i);
    } else {
        return NULL;
    }
}

void PyErr_Print (void)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyErr_Print) {
        PythonApiFunctions->EPA_PyErr_Print();
    }
}

void PyErr_PrintEx (int i)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyErr_PrintEx) {
        PythonApiFunctions->EPA_PyErr_PrintEx(i);
    }
}

int Py_AtExit (void (*func) (void))
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_Py_AtExit) {
        return PythonApiFunctions->EPA_Py_AtExit(func);
    } else {
        return 0;
    }
}

void Py_Exit (int i)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_Py_Exit) {
        PythonApiFunctions->EPA_Py_Exit(i);
    }
}

int Py_FdIsInteractive (FILE *F, const char *c)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_Py_FdIsInteractive) {
        return PythonApiFunctions->EPA_Py_FdIsInteractive(F, c);
    } else {
        return 0;
    }
}

char * Py_GetProgramFullPath (void)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_Py_GetProgramFullPath) {
        return PythonApiFunctions->EPA_Py_GetProgramFullPath();
    } else {
        return NULL;
    }
}

char * Py_GetPrefix (void)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_Py_GetPrefix) {
        return PythonApiFunctions->EPA_Py_GetPrefix();
    } else {
        return NULL;
    }
}

char * Py_GetExecPrefix (void)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_Py_GetExecPrefix) {
        return PythonApiFunctions->EPA_Py_GetExecPrefix();
    } else {
        return NULL;
    }
}

char * Py_GetPath (void)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_Py_GetPath) {
        return PythonApiFunctions->EPA_Py_GetPath();
    } else {
        return NULL;
    }
}

const char * Py_GetVersion (void)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_Py_GetVersion) {
        return PythonApiFunctions->EPA_Py_GetVersion();
    } else {
        return NULL;
    }
}

const char * Py_GetPlatform (void)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_Py_GetPlatform) {
        return PythonApiFunctions->EPA_Py_GetPlatform();
    } else {
        return NULL;
    }
}

const char * Py_GetCopyright (void)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_Py_GetCopyright) {
        return PythonApiFunctions->EPA_Py_GetCopyright();
    } else {
        return NULL;
    }
}

const char * Py_GetCompiler (void)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_Py_GetCompiler) {
        return PythonApiFunctions->EPA_Py_GetCompiler();
    } else {
        return NULL;
    }
}

const char * Py_GetBuildInfo (void)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_Py_GetBuildInfo) {
        return PythonApiFunctions->EPA_Py_GetBuildInfo();
    } else {
        return NULL;
    }
}

PyObject * _PySys_Init(void)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA__PySys_Init) {
        return PythonApiFunctions->EPA__PySys_Init();
    } else {
        return NULL;
    }
}

void _PyImport_Init (void)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA__PyImport_Init) {
        PythonApiFunctions->EPA__PyImport_Init();
    }
}

void _PyImport_Fini (void)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA__PyImport_Fini) {
        PythonApiFunctions->EPA__PyImport_Fini();
    }
}

void PyMethod_Fini (void)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyMethod_Fini) {
        PythonApiFunctions->EPA_PyMethod_Fini();
    }
}

void PyFrame_Fini (void)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyFrame_Fini) {
        PythonApiFunctions->EPA_PyFrame_Fini();
    }
}

void PyCFunction_Fini (void)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyCFunction_Fini) {
        PythonApiFunctions->EPA_PyCFunction_Fini();
    }
}

void PyTuple_Fini (void)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyTuple_Fini) {
        PythonApiFunctions->EPA_PyTuple_Fini();
    }
}

void PyString_Fini (void)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyString_Fini) {
        PythonApiFunctions->EPA_PyString_Fini();
    }
}

void PyInt_Fini (void)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyInt_Fini) {
        PythonApiFunctions->EPA_PyInt_Fini();
    }
}

void PyFloat_Fini (void)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyFloat_Fini) {
        PythonApiFunctions->EPA_PyFloat_Fini();
    }
}

void PyOS_FiniInterrupts (void)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyOS_FiniInterrupts) {
        PythonApiFunctions->EPA_PyOS_FiniInterrupts();
    }
}

char * PyOS_Readline (FILE *f1, FILE *f2, char *c)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyOS_Readline) {
        return PythonApiFunctions->EPA_PyOS_Readline(f1, f2, c);
    } else {
        return NULL;
    }
}

EFI_STATUS InitPythonrun() {
    if (PythonApiFunctions) {
        pPyOS_InputHook = PythonApiFunctions->EPA_PyOS_InputHook;
        pPyOS_ReadlineFunctionPointer = PythonApiFunctions->EPA_PyOS_ReadlineFunctionPointer;
        return EFI_SUCCESS;
    } else {
        return EFI_INVALID_PARAMETER;
    }
}
