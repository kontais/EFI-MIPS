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

    abstract.c

Abstract:

    This file implements wrappers for the functions declared in abstract.h.


Revision History

--*/


#include "EfiInterface.h"

extern EFI_PYTHON_API  *PythonApiFunctions;

/* These functions are wrappers around the function pointers in the EFI_PYTHON_API
 *  structure that make it so a module's source code doesn't have to be modified
 *  to be built as an dynamically loaded EFI driver.
 */

int PyObject_Cmp (PyObject *o1, PyObject *o2, int *result)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyObject_Cmp) {
        return PythonApiFunctions->EPA_PyObject_Cmp(o1, o2, result);
    } else {
        return 0;
    }
}

PyObject * PyObject_CallObject (PyObject *callable_object, PyObject *args)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyObject_CallObject) {
        return PythonApiFunctions->EPA_PyObject_CallObject(callable_object, args);
    } else {
        return NULL;
    }
}

PyObject * PyObject_CallFunction (PyObject *callable, char *format, ...)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyObject_CallObject) {
	    va_list va;
	    PyObject *args, *retval;
	    va_start(va, format);

	    if (callable == NULL) {
		    va_end(va);
	        if (!PyErr_Occurred())
		        PyErr_SetString(PyExc_SystemError,
				        "null argument to internal routine");
	        return NULL;
	    }

	    if (format)
		    args = Py_VaBuildValue(format, va);
	    else
		    args = PyTuple_New(0);

	    va_end(va);
	    
	    if (args == NULL)
		    return NULL;

	    if (!PyTuple_Check(args)) {
		    PyObject *a;

		    a = PyTuple_New(1);
		    if (a == NULL)
			    return NULL;
		    if (PyTuple_SetItem(a, 0, args) < 0)
			    return NULL;
		    args = a;
	    }
	    retval = PyObject_CallObject(callable, args);

	    Py_DECREF(args);

	    return retval;
    } else {
        return NULL;
    }
}

PyObject * PyObject_CallMethod (PyObject *o, char *name, char *format, ...)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyObject_CallObject) {
	    va_list va;
	    PyObject *args, *func = 0, *retval;
	    va_start(va, format);

	    if (o == NULL || name == NULL) {
		    va_end(va);
	        if (!PyErr_Occurred())
		        PyErr_SetString(PyExc_SystemError,
				        "null argument to internal routine");
	        return NULL;
	    }

	    func = PyObject_GetAttrString(o, name);
	    if (func == NULL) {
		    va_end(va);
		    PyErr_SetString(PyExc_AttributeError, name);
		    return 0;
	    }

	    if (!PyCallable_Check(func)) {
		    va_end(va);
            PyErr_SetString(PyExc_TypeError, "call of non-callable attribute");
		    return NULL;
	    }

	    if (format && *format)
		    args = Py_VaBuildValue(format, va);
	    else
		    args = PyTuple_New(0);

	    va_end(va);

	    if (!args)
		    return NULL;

	    if (!PyTuple_Check(args)) {
		    PyObject *a;

		    a = PyTuple_New(1);
		    if (a == NULL)
			    return NULL;
		    if (PyTuple_SetItem(a, 0, args) < 0)
			    return NULL;
		    args = a;
	    }

	    retval = PyObject_CallObject(func, args);

	    Py_DECREF(args);
	    Py_DECREF(func);

	    return retval;
    } else {
        return NULL;
    }
}

PyObject * PyObject_Type (PyObject *o)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyObject_Type) {
        return PythonApiFunctions->EPA_PyObject_Type(o);
    } else {
        return NULL;
    }
}

int PyObject_Length (PyObject *o)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyObject_Length) {
        return PythonApiFunctions->EPA_PyObject_Length(o);
    } else {
        return 0;
    }
}

PyObject * PyObject_GetItem (PyObject *o, PyObject *key)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyObject_GetItem) {
        return PythonApiFunctions->EPA_PyObject_GetItem(o, key);
    } else {
        return NULL;
    }
}

int PyObject_SetItem (PyObject *o, PyObject *key, PyObject *v)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyObject_SetItem) {
        return PythonApiFunctions->EPA_PyObject_SetItem(o, key, v);
    } else {
        return 0;
    }
}

int PyObject_DelItem (PyObject *o, PyObject *key)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyObject_DelItem) {
        return PythonApiFunctions->EPA_PyObject_DelItem(o, key);
    } else {
        return 0;
    }
}

int PyNumber_Check (PyObject *o)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyNumber_Check) {
        return PythonApiFunctions->EPA_PyNumber_Check(o);
    } else {
        return 0;
    }
}

PyObject * PyNumber_Add (PyObject *o1, PyObject *o2)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyNumber_Add) {
        return PythonApiFunctions->EPA_PyNumber_Add(o1, o2);
    } else {
        return NULL;
    }
}

PyObject * PyNumber_Subtract (PyObject *o1, PyObject *o2)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyNumber_Subtract) {
        return PythonApiFunctions->EPA_PyNumber_Subtract(o1, o2);
    } else {
        return NULL;
    }
}

PyObject * PyNumber_Multiply (PyObject *o1, PyObject *o2)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyNumber_Multiply) {
        return PythonApiFunctions->EPA_PyNumber_Multiply(o1, o2);
    } else {
        return NULL;
    }
}

PyObject * PyNumber_Divide (PyObject *o1, PyObject *o2)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyNumber_Divide) {
        return PythonApiFunctions->EPA_PyNumber_Divide(o1, o2);
    } else {
        return NULL;
    }
}

PyObject * PyNumber_Remainder (PyObject *o1, PyObject *o2)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyNumber_Remainder) {
        return PythonApiFunctions->EPA_PyNumber_Remainder(o1, o2);
    } else {
        return NULL;
    }
}

PyObject * PyNumber_Divmod (PyObject *o1, PyObject *o2)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyNumber_Divmod) {
        return PythonApiFunctions->EPA_PyNumber_Divmod(o1, o2);
    } else {
        return NULL;
    }
}

PyObject * PyNumber_Power (PyObject *o1, PyObject *o2, PyObject *o3)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyNumber_Power) {
        return PythonApiFunctions->EPA_PyNumber_Power(o1, o2, o3);
    } else {
        return NULL;
    }
}

PyObject * PyNumber_Negative (PyObject *o)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyNumber_Negative) {
        return PythonApiFunctions->EPA_PyNumber_Negative(o);
    } else {
        return NULL;
    }
}

PyObject * PyNumber_Positive (PyObject *o)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyNumber_Positive) {
        return PythonApiFunctions->EPA_PyNumber_Positive(o);
    } else {
        return NULL;
    }
}

PyObject * PyNumber_Absolute (PyObject *o)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyNumber_Absolute) {
        return PythonApiFunctions->EPA_PyNumber_Absolute(o);
    } else {
        return NULL;
    }
}

PyObject * PyNumber_Invert (PyObject *o)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyNumber_Invert) {
        return PythonApiFunctions->EPA_PyNumber_Invert(o);
    } else {
        return NULL;
    }
}

PyObject * PyNumber_Lshift (PyObject *o1, PyObject *o2)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyNumber_Lshift) {
        return PythonApiFunctions->EPA_PyNumber_Lshift(o1, o2);
    } else {
        return NULL;
    }
}

PyObject * PyNumber_Rshift (PyObject *o1, PyObject *o2)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyNumber_Rshift) {
        return PythonApiFunctions->EPA_PyNumber_Rshift(o1, o2);
    } else {
        return NULL;
    }
}

PyObject * PyNumber_And (PyObject *o1, PyObject *o2)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyNumber_And) {
        return PythonApiFunctions->EPA_PyNumber_And(o1, o2);
    } else {
        return NULL;
    }
}

PyObject * PyNumber_Xor (PyObject *o1, PyObject *o2)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyNumber_Xor) {
        return PythonApiFunctions->EPA_PyNumber_Xor(o1, o2);
    } else {
        return NULL;
    }
}

PyObject * PyNumber_Or (PyObject *o1, PyObject *o2)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyNumber_Or) {
        return PythonApiFunctions->EPA_PyNumber_Or(o1, o2);
    } else {
        return NULL;
    }
}

PyObject * PyNumber_Int (PyObject *o)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyNumber_Int) {
        return PythonApiFunctions->EPA_PyNumber_Int(o);
    } else {
        return NULL;
    }
}

PyObject * PyNumber_Long (PyObject *o)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyNumber_Long) {
        return PythonApiFunctions->EPA_PyNumber_Long(o);
    } else {
        return NULL;
    }
}

PyObject * PyNumber_Float (PyObject *o)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyNumber_Float) {
        return PythonApiFunctions->EPA_PyNumber_Float(o);
    } else {
        return NULL;
    }
}

int PySequence_Check (PyObject *o)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PySequence_Check) {
        return PythonApiFunctions->EPA_PySequence_Check(o);
    } else {
        return 0;
    }
}

int PySequence_Length (PyObject *o)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PySequence_Length) {
        return PythonApiFunctions->EPA_PySequence_Length(o);
    } else {
        return 0;
    }
}

PyObject * PySequence_Concat (PyObject *o1, PyObject *o2)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PySequence_Concat) {
        return PythonApiFunctions->EPA_PySequence_Concat(o1, o2);
    } else {
        return NULL;
    }
}

PyObject * PySequence_Repeat (PyObject *o, int count)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PySequence_Repeat) {
        return PythonApiFunctions->EPA_PySequence_Repeat(o, count);
    } else {
        return NULL;
    }
}

PyObject * PySequence_GetItem (PyObject *o, int i)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PySequence_GetItem) {
        return PythonApiFunctions->EPA_PySequence_GetItem(o, i);
    } else {
        return NULL;
    }
}

PyObject * PySequence_GetSlice (PyObject *o, int i1, int i2)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PySequence_GetSlice) {
        return PythonApiFunctions->EPA_PySequence_GetSlice(o, i1, i2);
    } else {
        return NULL;
    }
}

int PySequence_SetItem (PyObject *o, int i, PyObject *v)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PySequence_SetItem) {
        return PythonApiFunctions->EPA_PySequence_SetItem(o, i, v);
    } else {
        return 0;
    }
}

int PySequence_DelItem (PyObject *o, int i)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PySequence_DelItem) {
        return PythonApiFunctions->EPA_PySequence_DelItem(o, i);
    } else {
        return 0;
    }
}

int PySequence_SetSlice (PyObject *o, int i1, int i2, PyObject *v)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PySequence_SetSlice) {
        return PythonApiFunctions->EPA_PySequence_SetSlice(o, i1, i2, v);
    } else {
        return 0;
    }
}

int PySequence_DelSlice (PyObject *o, int i1, int i2)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PySequence_DelSlice) {
        return PythonApiFunctions->EPA_PySequence_DelSlice(o, i1, i2);
    } else {
        return 0;
    }
}

PyObject * PySequence_Tuple (PyObject *o)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PySequence_Tuple) {
        return PythonApiFunctions->EPA_PySequence_Tuple(o);
    } else {
        return NULL;
    }
}

PyObject * PySequence_List (PyObject *o)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PySequence_List) {
        return PythonApiFunctions->EPA_PySequence_List(o);
    } else {
        return NULL;
    }
}

int PySequence_Count (PyObject *o, PyObject *value)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PySequence_Count) {
        return PythonApiFunctions->EPA_PySequence_Count(o, value);
    } else {
        return 0;
    }
}

int PySequence_Contains (PyObject *o, PyObject *value)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PySequence_Contains) {
        return PythonApiFunctions->EPA_PySequence_Contains(o, value);
    } else {
        return 0;
    }
}

int PySequence_Index (PyObject *o, PyObject *value)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PySequence_Index) {
        return PythonApiFunctions->EPA_PySequence_Index(o, value);
    } else {
        return 0;
    }
}

int PyMapping_Check (PyObject *o)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyMapping_Check) {
        return PythonApiFunctions->EPA_PyMapping_Check(o);
    } else {
        return 0;
    }
}

int PyMapping_Length (PyObject *o)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyMapping_Length) {
        return PythonApiFunctions->EPA_PyMapping_Length(o);
    } else {
        return 0;
    }
}

int PyMapping_HasKeyString (PyObject *o, char *key)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyMapping_HasKeyString) {
        return PythonApiFunctions->EPA_PyMapping_HasKeyString(o, key);
    } else {
        return 0;
    }
}

int PyMapping_HasKey (PyObject *o, PyObject *key)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyMapping_HasKey) {
        return PythonApiFunctions->EPA_PyMapping_HasKey(o, key);
    } else {
        return 0;
    }
}

PyObject * PyMapping_GetItemString (PyObject *o, char *key)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyMapping_GetItemString) {
        return PythonApiFunctions->EPA_PyMapping_GetItemString(o, key);
    } else {
        return NULL;
    }
}

int PyMapping_SetItemString (PyObject *o, char *key, PyObject *value)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyMapping_SetItemString) {
        return PythonApiFunctions->EPA_PyMapping_SetItemString(o, key, value );
    } else {
        return 0;
    }
}

PyObject *
PyIter_Next(PyObject *iter)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyIter_Next) {
        return PythonApiFunctions->EPA_PyIter_Next(iter);
    } else {
        return 0;
    }
}

PyObject *
PyObject_GetIter( PyObject *o )
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyObject_GetIter) {
        return PythonApiFunctions->EPA_PyObject_GetIter(o);
    } else {
        return 0;
    }
}

PyObject *
PyObject_Call(PyObject *func, PyObject *arg, PyObject *kw)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyObject_Call) {
        return PythonApiFunctions->EPA_PyObject_Call( func, arg, kw );
    } else {
        return 0;
    }
}

int PyObject_AsReadBuffer(PyObject *obj,
			  const void **buffer,
			  int *buffer_len)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyObject_AsReadBuffer) {
        return PythonApiFunctions->EPA_PyObject_AsReadBuffer( obj, buffer, buffer_len );
    } else {
        return 0;
    }
}

PyObject *
PyNumber_FloorDivide(PyObject *v, PyObject *w)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyNumber_FloorDivide) {
        return PythonApiFunctions->EPA_PyNumber_FloorDivide( v, w );
    } else {
        return NULL;
    }
}

PyObject *
PyNumber_TrueDivide(PyObject *v, PyObject *w)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyNumber_TrueDivide) {
        return PythonApiFunctions->EPA_PyNumber_TrueDivide( v, w );
    } else {
        return NULL;
    }
}

PyObject *
PySequence_Fast(PyObject *v, const char *m)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PySequence_Fast) {
        return PythonApiFunctions->EPA_PySequence_Fast( v, m );
    } else {
        return NULL;
    }
}

EFI_STATUS InitAbstract() {
    if (PythonApiFunctions) {
        return EFI_SUCCESS;
    } else {
        return EFI_INVALID_PARAMETER;
    }
}

