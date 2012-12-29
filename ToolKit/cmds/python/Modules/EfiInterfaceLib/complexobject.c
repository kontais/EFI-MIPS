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

    complexobject.c

Abstract:

    This file implements wrappers for the functions declared in complexobject.h.


Revision History

--*/


#include "EfiInterface.h"

extern EFI_PYTHON_API  *PythonApiFunctions;

/* These functions are wrappers around the function pointers in the EFI_PYTHON_API
 *  structure that make it so a module's source code doesn't have to be modified
 *  to be built as an dynamically loaded EFI driver.
 */

static Py_complex c_zero = {0., 0.};

Py_complex c_sum (Py_complex C1, Py_complex C2)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_c_sum) {
        return PythonApiFunctions->EPA_c_sum(C1, C2);
    } else {
        return c_zero;
    }
}

Py_complex c_diff (Py_complex C1, Py_complex C2)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_c_diff) {
        return PythonApiFunctions->EPA_c_diff(C1, C2);
    } else {
        return c_zero;
    }
}

Py_complex c_neg (Py_complex C)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_c_neg) {
        return PythonApiFunctions->EPA_c_neg(C);
    } else {
        return c_zero;
    }
}

Py_complex c_prod (Py_complex C1, Py_complex C2)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_c_prod) {
        return PythonApiFunctions->EPA_c_prod(C1, C2);
    } else {
        return c_zero;
    }
}

Py_complex c_quot (Py_complex C1, Py_complex C2)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_c_quot) {
        return PythonApiFunctions->EPA_c_quot(C1, C2);
    } else {
        return c_zero;
    }
}

Py_complex c_pow (Py_complex C1, Py_complex C2)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_c_pow) {
        return PythonApiFunctions->EPA_c_pow(C1, C2);
    } else {
        return c_zero;
    }
}

PyTypeObject *pPyComplex_Type = NULL;

PyObject * PyComplex_FromCComplex (Py_complex C)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyComplex_FromCComplex) {
        return PythonApiFunctions->EPA_PyComplex_FromCComplex(C);
    } else {
        return NULL;
    }
}

PyObject * PyComplex_FromDoubles (double real, double imag)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyComplex_FromDoubles) {
        return PythonApiFunctions->EPA_PyComplex_FromDoubles(real, imag);
    } else {
        return NULL;
    }
}

double PyComplex_RealAsDouble (PyObject *op)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyComplex_RealAsDouble) {
        return PythonApiFunctions->EPA_PyComplex_RealAsDouble(op);
    } else {
        return 0.0;
    }
}

double PyComplex_ImagAsDouble (PyObject *op)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyComplex_ImagAsDouble) {
        return PythonApiFunctions->EPA_PyComplex_ImagAsDouble(op);
    } else {
        return 0.0;
    }
}

Py_complex PyComplex_AsCComplex (PyObject *op)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyComplex_AsCComplex) {
        return PythonApiFunctions->EPA_PyComplex_AsCComplex(op);
    } else {
        return c_zero;
    }
}

EFI_STATUS InitComplexobject(void)
{
    if (PythonApiFunctions) {
        pPyComplex_Type = PythonApiFunctions->EPA_PyDict_Type;
        return EFI_SUCCESS;
    } else {
        return EFI_INVALID_PARAMETER;
    }
}
