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

    compile.c

Abstract:

    This file implements wrappers for the functions declared in compile.h.


Revision History

--*/


#include "EfiInterface.h"

extern EFI_PYTHON_API  *PythonApiFunctions;

/* These functions are wrappers around the function pointers in the EFI_PYTHON_API
 *  structure that make it so a module's source code doesn't have to be modified
 *  to be built as an dynamically loaded EFI driver.
 */

PyTypeObject *pPyCode_Type = NULL;

PyCodeObject * PyNode_Compile (struct _node *N, const char *c)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyNode_Compile) {
        return PythonApiFunctions->EPA_PyNode_Compile(N, c);
    } else {
        return NULL;
    }
}

PyCodeObject * PyCode_New (
	int i1, int i2, int i3, int i4, PyObject *O1, PyObject *O2, PyObject *O3, PyObject *O4,
	PyObject *O5, PyObject *O6, PyObject *O7, PyObject *O8, int i5, PyObject *O9)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyCode_New) {
        return PythonApiFunctions->EPA_PyCode_New(i1, i2, i3, i4, O1, O2, O3, O4, O5, O6, O7, O8,  i5, O9);
    } else {
        return NULL;
    }
}

int PyCode_Addr2Line (PyCodeObject *O, int i)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyCode_Addr2Line) {
        return PythonApiFunctions->EPA_PyCode_Addr2Line(O, i);
    } else {
        return 0;
    }
}

EFI_STATUS InitCompile() {
    if (PythonApiFunctions) {
        pPyCode_Type = PythonApiFunctions->EPA_PyCode_Type;
        return EFI_SUCCESS;
    } else {
        return EFI_INVALID_PARAMETER;
    }
}
