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

    errors.c

Abstract:

    This file implements wrappers for the functions declared in errors.h.


Revision History

--*/


#include "EfiInterface.h"

extern EFI_PYTHON_API  *PythonApiFunctions;

/* These functions are wrappers around the function pointers in the EFI_PYTHON_API
 *  structure that make it so a module's source code doesn't have to be modified
 *  to be built as an dynamically loaded EFI driver.
 */

/* Predefined exceptions */

PyObject *PyExc_Exception = NULL;
PyObject *PyExc_StandardError = NULL;
PyObject *PyExc_ArithmeticError = NULL;
PyObject *PyExc_LookupError = NULL;
PyObject *PyExc_AssertionError = NULL;
PyObject *PyExc_AttributeError = NULL;
PyObject *PyExc_EOFError = NULL;
PyObject *PyExc_FloatingPointError = NULL;
PyObject *PyExc_EnvironmentError = NULL;
PyObject *PyExc_IOError = NULL;
PyObject *PyExc_OSError = NULL;
PyObject *PyExc_ImportError = NULL;
PyObject *PyExc_IndexError = NULL;
PyObject *PyExc_KeyError = NULL;
PyObject *PyExc_KeyboardInterrupt = NULL;
PyObject *PyExc_MemoryError = NULL;
PyObject *PyExc_NameError = NULL;
PyObject *PyExc_OverflowError = NULL;
PyObject *PyExc_RuntimeError = NULL;
PyObject *PyExc_NotImplementedError = NULL;
PyObject *PyExc_SyntaxError = NULL;
PyObject *PyExc_SystemError = NULL;
PyObject *PyExc_SystemExit = NULL;
PyObject *PyExc_TypeError = NULL;
PyObject *PyExc_ValueError = NULL;
PyObject *PyExc_ZeroDivisionError = NULL;
PyObject *PyExc_MemoryErrorInst = NULL;


void PyErr_SetNone (PyObject *exception)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyErr_SetNone) {
        PythonApiFunctions->EPA_PyErr_SetNone(exception);
    }
}

void PyErr_SetObject (PyObject *exception, PyObject *value)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyErr_SetObject) {
        PythonApiFunctions->EPA_PyErr_SetObject(exception, value);
    }
}

void PyErr_SetString (PyObject *exception, const char *string)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyErr_SetString) {
        PythonApiFunctions->EPA_PyErr_SetString(exception, string);
    }
}

PyObject * PyErr_Occurred (void)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyErr_Occurred) {
        return PythonApiFunctions->EPA_PyErr_Occurred();
    } else {
        return NULL;
    }
}

void PyErr_Clear (void)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyErr_Clear) {
        PythonApiFunctions->EPA_PyErr_Clear();
    }
}

void PyErr_Fetch (PyObject **type, PyObject **value, PyObject **traceback)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyErr_Fetch) {
        PythonApiFunctions->EPA_PyErr_Fetch(type, value, traceback);
    }
}

void PyErr_Restore (PyObject *type, PyObject *value, PyObject *traceback)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyErr_Restore) {
        PythonApiFunctions->EPA_PyErr_Restore(type, value, traceback);
    }
}

int PyErr_GivenExceptionMatches (PyObject *error, PyObject *exception)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyErr_GivenExceptionMatches) {
        return PythonApiFunctions->EPA_PyErr_GivenExceptionMatches(error, exception);
    } else {
        return 0;
    }
}

int PyErr_ExceptionMatches (PyObject *exception)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyErr_ExceptionMatches) {
        return PythonApiFunctions->EPA_PyErr_ExceptionMatches(exception);
    } else {
        return 0;
    }
}

void PyErr_NormalizeException (PyObject **exception, PyObject **value, PyObject **traceback)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyErr_NormalizeException) {
        PythonApiFunctions->EPA_PyErr_NormalizeException(exception, value, traceback);
    }
}

int PyErr_BadArgument (void)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyErr_BadArgument) {
        return PythonApiFunctions->EPA_PyErr_BadArgument();
    } else {
        return 0;
    }
}

PyObject *PyErr_NoMemory (void)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyErr_NoMemory) {
        return PythonApiFunctions->EPA_PyErr_NoMemory();
    } else {
        return NULL;
    }
}

PyObject *PyErr_SetFromErrno (PyObject *exception)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyErr_SetFromErrno) {
        return PythonApiFunctions->EPA_PyErr_SetFromErrno(exception);
    } else {
        return NULL;
    }
}

PyObject *PyErr_SetFromErrnoWithFilename (PyObject *exception, char *filename)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyErr_SetFromErrnoWithFilename) {
        return PythonApiFunctions->EPA_PyErr_SetFromErrnoWithFilename(exception, filename);
    } else {
        return NULL;
    }
}

PyObject *PyErr_Format (PyObject *exception, const char *format, ...)
{
	char buffer[500]; /* Caller is responsible for limiting the format */
	va_list va;
	
	va_start(va, format);
    vsprintf(buffer, format, va);
	PyErr_SetString(exception, buffer);
    va_end(va);
	return NULL;
}

void _PyErr_BadInternalCall (char *filename, int lineno)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyErr_BadInternalCall) {
        PythonApiFunctions->EPA_PyErr_BadInternalCall(filename, lineno);
    }
}

PyObject *PyErr_NewException (char *name, PyObject *base, PyObject *dict)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyErr_NewException) {
        return PythonApiFunctions->EPA_PyErr_NewException(name, base, dict);
    } else {
        return NULL;
    }
}

#if 0 // The EFI version of Python doesn't support this
int PyErr_CheckSignals (void)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyErr_CheckSignals) {
        return PythonApiFunctions->EPA_PyErr_CheckSignals();
    } else {
        return 0;
    }
}
#endif

#if 0 // The EFI version of Python doesn't support this
void PyErr_SetInterrupt (void)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyErr_SetInterrupt) {
        PythonApiFunctions->EPA_PyErr_SetInterrupt();
    }
}
#endif


int
PyErr_Warn(PyObject *category, char *message)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyErr_Warn) {
        return PythonApiFunctions->EPA_PyErr_Warn( category, message );
    } else {
        return 0;
    }
}

void
PyErr_WriteUnraisable(PyObject *obj)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyErr_WriteUnraisable) {
        PythonApiFunctions->EPA_PyErr_WriteUnraisable( obj );
    }
}

EFI_STATUS InitErrors( void ) {
    if (PythonApiFunctions) {
        PyExc_Exception = *PythonApiFunctions->EPA_PyExc_Exception;
        PyExc_StandardError = *PythonApiFunctions->EPA_PyExc_StandardError;
        PyExc_ArithmeticError = *PythonApiFunctions->EPA_PyExc_ArithmeticError;
        PyExc_LookupError = *PythonApiFunctions->EPA_PyExc_LookupError;
        PyExc_AssertionError = *PythonApiFunctions->EPA_PyExc_AssertionError;
        PyExc_AttributeError = *PythonApiFunctions->EPA_PyExc_AttributeError;
        PyExc_EOFError = *PythonApiFunctions->EPA_PyExc_EOFError;
        PyExc_FloatingPointError = *PythonApiFunctions->EPA_PyExc_FloatingPointError;
        PyExc_EnvironmentError = *PythonApiFunctions->EPA_PyExc_EnvironmentError;
        PyExc_IOError = *PythonApiFunctions->EPA_PyExc_IOError;
        PyExc_OSError = *PythonApiFunctions->EPA_PyExc_OSError;
        PyExc_ImportError = *PythonApiFunctions->EPA_PyExc_ImportError;
        PyExc_IndexError = *PythonApiFunctions->EPA_PyExc_IndexError;
        PyExc_KeyError = *PythonApiFunctions->EPA_PyExc_KeyError;
        PyExc_KeyboardInterrupt = *PythonApiFunctions->EPA_PyExc_KeyboardInterrupt;
        PyExc_MemoryError = *PythonApiFunctions->EPA_PyExc_MemoryError;
        PyExc_NameError = *PythonApiFunctions->EPA_PyExc_NameError;
        PyExc_OverflowError = *PythonApiFunctions->EPA_PyExc_OverflowError;
        PyExc_RuntimeError = *PythonApiFunctions->EPA_PyExc_RuntimeError;
        PyExc_NotImplementedError = *PythonApiFunctions->EPA_PyExc_NotImplementedError;
        PyExc_SyntaxError = *PythonApiFunctions->EPA_PyExc_SyntaxError;
        PyExc_SystemError = *PythonApiFunctions->EPA_PyExc_SystemError;
        PyExc_SystemExit = *PythonApiFunctions->EPA_PyExc_SystemExit;
        PyExc_TypeError = *PythonApiFunctions->EPA_PyExc_TypeError;
        PyExc_ValueError = *PythonApiFunctions->EPA_PyExc_ValueError;
        PyExc_ZeroDivisionError = *PythonApiFunctions->EPA_PyExc_ZeroDivisionError;
        PyExc_MemoryErrorInst = *PythonApiFunctions->EPA_PyExc_MemoryErrorInst;

        return EFI_SUCCESS;
    } else {
        return EFI_INVALID_PARAMETER;
    }
}
