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

    dictobject.c

Abstract:

    This file implements wrappers for the functions declared in dictobject.h.


Revision History

--*/


#include "EfiInterface.h"

extern EFI_PYTHON_API  *PythonApiFunctions;

/* These functions are wrappers around the function pointers in the EFI_PYTHON_API
 *  structure that make it so a module's source code doesn't have to be modified
 *  to be built as an dynamically loaded EFI driver.
 */

PyTypeObject *pPyDict_Type = NULL;

PyObject *PyDict_New(void)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyDict_New) {
        return PythonApiFunctions->EPA_PyDict_New();
    } else {
        return NULL;
    }
}

PyObject *PyDict_GetItem (PyObject *mp, PyObject *key)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyDict_GetItem) {
        return PythonApiFunctions->EPA_PyDict_GetItem(mp, key);
    } else {
        return NULL;
    }
}

int PyDict_SetItem (PyObject *mp, PyObject *key, PyObject *item)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyDict_SetItem) {
        return PythonApiFunctions->EPA_PyDict_SetItem(mp, key, item);
    } else {
        return 0;
    }
}

int PyDict_DelItem (PyObject *mp, PyObject *key)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyDict_DelItem) {
        return PythonApiFunctions->EPA_PyDict_DelItem(mp, key);
    } else {
        return 0;
    }
}

void PyDict_Clear (PyObject *mp)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyDict_Clear) {
        PythonApiFunctions->EPA_PyDict_Clear(mp);
    }
}

int PyDict_Next (PyObject *mp, int *pos, PyObject **key, PyObject **value)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyDict_Next) {
        return PythonApiFunctions->EPA_PyDict_Next(mp, pos, key, value);
    } else {
        return 0;
    }
}

PyObject *PyDict_Keys (PyObject *mp)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyDict_Keys) {
        return PythonApiFunctions->EPA_PyDict_Keys(mp);
    } else {
        return NULL;
    }
}

PyObject *PyDict_Values (PyObject *mp)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyDict_Values) {
        return PythonApiFunctions->EPA_PyDict_Values(mp);
    } else {
        return NULL;
    }
}

PyObject *PyDict_Items (PyObject *mp)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyDict_Items) {
        return PythonApiFunctions->EPA_PyDict_Items(mp);
    } else {
        return NULL;
    }
}

int PyDict_Size (PyObject *mp)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyDict_Size) {
        return PythonApiFunctions->EPA_PyDict_Size(mp);
    } else {
        return 0;
    }
}

PyObject *PyDict_GetItemString (PyObject *dp, const char *key)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyDict_GetItemString) {
        return PythonApiFunctions->EPA_PyDict_GetItemString(dp, key);
    } else {
        return NULL;
    }
}

int PyDict_SetItemString (PyObject *dp, const char *key, PyObject *item)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyDict_SetItemString) {
        return PythonApiFunctions->EPA_PyDict_SetItemString(dp, key, item);
    } else {
        return 0;
    }
}

int PyDict_DelItemString (PyObject *dp, const char *key)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyDict_DelItemString) {
        return PythonApiFunctions->EPA_PyDict_DelItemString(dp, key);
    } else {
        return 0;
    }
}

EFI_STATUS InitDictobject(void)
{
    if (PythonApiFunctions) {
        pPyDict_Type = PythonApiFunctions->EPA_PyDict_Type;
        return EFI_SUCCESS;
    } else {
        return EFI_INVALID_PARAMETER;
    }
}
