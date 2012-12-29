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

    modsupport.c

Abstract:

    This file implements wrappers for the functions declared in modsupport.h.


Revision History

--*/


#include "EfiInterface.h"
//#include <stdarg.h>

extern EFI_PYTHON_API  *PythonApiFunctions;

/* These functions are wrappers around the function pointers in the EFI_PYTHON_API
 *  structure that make it so a module's source code doesn't have to be modified
 *  to be built as an dynamically loaded EFI driver.
 */

char * _Py_PackageContext = NULL;

int PyArg_Parse (PyObject *args, char *format, ...)
{
	int retval;
	va_list va;
	
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyArg_ParseEfi) {
	    va_start(va, format);
	    retval = PythonApiFunctions->EPA_PyArg_ParseEfi(args, format, va);
	    va_end(va);
	    return retval;
    } else {
        return 0;
    }
}

int PyArg_ParseTuple (PyObject *args, char *format, ...)
{
	int retval;
	va_list va;
	
	va_start(va, format);
	retval = PyArg_VaParse(args, format, va);
	va_end(va);
	return retval;
}

int PyArg_ParseTupleAndKeywords (PyObject *args, PyObject *keywords,
						 char *format, char **kwlist, ...)
{
	int retval;
	va_list va;

    if (PythonApiFunctions && PythonApiFunctions->EPA_PyArg_VaParseTupleAndKeywords) {
	    va_start(va, kwlist);
	    retval = PythonApiFunctions->EPA_PyArg_VaParseTupleAndKeywords(args, keywords, format, kwlist, va);
	    va_end(va);
	    return retval;
    } else {
        return 0;
    }
}

PyObject * Py_BuildValue (char *format, ...)
{
	va_list va;
	PyObject* retval;
	va_start(va, format);
	retval = Py_VaBuildValue(format, va);
	va_end(va);
	return retval;
}

int PyArg_VaParse (PyObject *args, char *format, va_list va)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyArg_VaParse) {
        return PythonApiFunctions->EPA_PyArg_VaParse(args, format, va);
    } else {
        return 0;
    }
}

PyObject * Py_VaBuildValue (char *format, va_list va)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_Py_VaBuildValue) {
        return PythonApiFunctions->EPA_Py_VaBuildValue(format, va);
    } else {
        return NULL;
    }
}

PyObject * Py_InitModule4 (char *name, 
                           PyMethodDef *methods, 
                           char *doc, 
                           PyObject *passthrough, 
                           int module_api_version) 
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_Py_InitModule4) {
        return PythonApiFunctions->EPA_Py_InitModule4(name, methods, doc, passthrough, module_api_version);
    } else {
        return NULL;
    }
}

int
PyModule_AddObject(PyObject *m, char *name, PyObject *o)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyModule_AddObject) {
        return PythonApiFunctions->EPA_PyModule_AddObject(m, name, o);
    } else {
        return 0;
    }
}

int 
PyModule_AddIntConstant(PyObject *m, char *name, long value)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyModule_AddIntConstant) {
        return PythonApiFunctions->EPA_PyModule_AddIntConstant(m, name, value);
    } else {
        return 0;
    }
}

int 
PyModule_AddStringConstant(PyObject *m, char *name, char *value)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyModule_AddStringConstant) {
        return PythonApiFunctions->EPA_PyModule_AddStringConstant(m, name, value);
    } else {
        return 0;
    }
}

EFI_STATUS InitModsupport() {
    if (PythonApiFunctions) {
        _Py_PackageContext = *PythonApiFunctions->EPA__Py_PackageContext;
        return EFI_SUCCESS;
    } else {
        return EFI_INVALID_PARAMETER;
    }
}