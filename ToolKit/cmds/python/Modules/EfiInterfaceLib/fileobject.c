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

    fileobject.c

Abstract:

    This file implements wrappers for the functions declared in fileobject.h.


Revision History

--*/


#include "EfiInterface.h"

extern EFI_PYTHON_API  *PythonApiFunctions;

/* These functions are wrappers around the function pointers in the EFI_PYTHON_API
 *  structure that make it so a module's source code doesn't have to be modified
 *  to be built as an dynamically loaded EFI driver.
 */

PyTypeObject *pPyFile_Type = NULL;

PyObject * PyFile_FromString (char *c1, char *c2)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyFile_FromString) {
        return PythonApiFunctions->EPA_PyFile_FromString(c1, c2);
    } else {
        return NULL;
    }
}

void PyFile_SetBufSize (PyObject *O, int i)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyFile_SetBufSize) {
        PythonApiFunctions->EPA_PyFile_SetBufSize(O, i);
    }
}

PyObject * PyFile_FromFile (FILE *F, char *c1, char *c2, int (*f)(FILE *))
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyFile_FromFile) {
        return PythonApiFunctions->EPA_PyFile_FromFile(F, c1, c2, f);
    } else {
        return NULL;
    }
}

FILE * PyFile_AsFile (PyObject *O)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyFile_AsFile) {
        return PythonApiFunctions->EPA_PyFile_AsFile(O);
    } else {
        return NULL;
    }
}

PyObject * PyFile_Name (PyObject *O)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyFile_Name) {
        return PythonApiFunctions->EPA_PyFile_Name(O);
    } else {
        return NULL;
    }
}

PyObject * PyFile_GetLine (PyObject *O, int i)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyFile_GetLine) {
        return PythonApiFunctions->EPA_PyFile_GetLine(O, i);
    } else {
        return NULL;
    }
}

int PyFile_WriteObject (PyObject *O1, PyObject *O2, int i)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyFile_WriteObject) {
        return PythonApiFunctions->EPA_PyFile_WriteObject(O1, O2, i);
    } else {
        return 0;
    }
}

int PyFile_SoftSpace (PyObject *O, int i)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyFile_SoftSpace) {
        return PythonApiFunctions->EPA_PyFile_SoftSpace(O, i);
    } else {
        return 0;
    }
}

int PyFile_WriteString (const char *c, PyObject *O)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyFile_WriteString) {
        return PythonApiFunctions->EPA_PyFile_WriteString(c, O);
    } else {
        return 0;
    }
}

int PyObject_AsFileDescriptor(PyObject *o)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyObject_AsFileDescriptor) {
        return PythonApiFunctions->EPA_PyObject_AsFileDescriptor(o);
    } else {
        return 0;
    }
}

EFI_STATUS InitFileobject() {
    if (PythonApiFunctions) {
        pPyFile_Type = PythonApiFunctions->EPA_PyFile_Type;
        return EFI_SUCCESS;
    } else {
        return EFI_INVALID_PARAMETER;
    }
}
