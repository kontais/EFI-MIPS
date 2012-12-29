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

    floatobject.c

Abstract:

    This file implements wrappers for the functions declared in floatobject.h.


Revision History

--*/


#include "EfiInterface.h"

extern EFI_PYTHON_API  *PythonApiFunctions;

/* These functions are wrappers around the function pointers in the EFI_PYTHON_API
 *  structure that make it so a module's source code doesn't have to be modified
 *  to be built as an dynamically loaded EFI driver.
 */

PyTypeObject *pPyFloat_Type = NULL;

PyObject * PyFloat_FromDouble (double d)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyFloat_FromDouble) {
        return PythonApiFunctions->EPA_PyFloat_FromDouble(d);
    } else {
        return NULL;
    }
}

double PyFloat_AsDouble (PyObject * O)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyFloat_AsDouble) {
        return PythonApiFunctions->EPA_PyFloat_AsDouble(O);
    } else {
        return 0.0;
    }
}

int
_PyFloat_Pack8(double x, unsigned char *p, int le)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA__PyFloat_Pack8) {
        return PythonApiFunctions->EPA__PyFloat_Pack8(x, p, le);
    } else {
        return 0;
    }
}

double
_PyFloat_Unpack8(const unsigned char *p, int le)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA__PyFloat_Unpack8) {
        return PythonApiFunctions->EPA__PyFloat_Unpack8(p, le);
    } else {
        return 0.0;
    }
}

double
_PyFloat_Unpack4(const unsigned char *p, int le)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA__PyFloat_Unpack4) {
        return PythonApiFunctions->EPA__PyFloat_Unpack4(p, le);
    } else {
        return 0.0;
    }
}

int
_PyFloat_Pack4(double x, unsigned char *p, int le)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA__PyFloat_Pack4) {
        return PythonApiFunctions->EPA__PyFloat_Pack4(x, p, le);
    } else {
        return 0;
    }
}

EFI_STATUS InitFloatobject() {
    if (PythonApiFunctions) {
        pPyFloat_Type = PythonApiFunctions->EPA_PyFloat_Type;
        return EFI_SUCCESS;
    } else {
        return EFI_INVALID_PARAMETER;
    }
}
