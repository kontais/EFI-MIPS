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

    sysmodule.c

Abstract:

    This file implements wrappers for the functions declared in sysmodule.h.


Revision History

--*/


#include "EfiInterface.h"

extern EFI_PYTHON_API  *PythonApiFunctions;

/* These functions are wrappers around the function pointers in the EFI_PYTHON_API
 *  structure that make it so a module's source code doesn't have to be modified
 *  to be built as an dynamically loaded EFI driver.
 */

PyObject * PySys_GetObject (char *c)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PySys_GetObject) {
        return PythonApiFunctions->EPA_PySys_GetObject(c);
    } else {
        return NULL;
    }
}

int PySys_SetObject (char *c, PyObject *O)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PySys_SetObject) {
        return PythonApiFunctions->EPA_PySys_SetObject(c, O);
    } else {
        return 0;
    }
}

FILE * PySys_GetFile (char *c, FILE *F)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PySys_GetFile) {
        return PythonApiFunctions->EPA_PySys_GetFile(c, F);
    } else {
        return NULL;
    }
}

void PySys_SetArgv (int i, char **c)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PySys_SetArgv) {
        PythonApiFunctions->EPA_PySys_SetArgv(i, c);
    }
}

void PySys_SetPath (char *c)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PySys_SetPath) {
        PythonApiFunctions->EPA_PySys_SetPath(c);
    }
}

#if 0
void PySys_WriteStdout (const char *format, ...);
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PySys_WriteStdout) {
        PythonApiFunctions->EPA_PySys_WriteStdout(O);
    }
}

void PySys_WriteStderr (const char *format, ...);
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PySys_WriteStderr) {
        PythonApiFunctions->EPA_PySys_WriteStderr(O);
    }
}
#endif

void
PySys_WriteStderr(const char *format, ...)
{
	va_list va;

	va_start(va, format);
    if (PythonApiFunctions && PythonApiFunctions->EPA_PySys_WriteStderr) {
        PythonApiFunctions->EPA_PySys_WriteStderr(format, va);
    } 

	va_end(va);
}

EFI_STATUS InitSysmodule() {
    if (PythonApiFunctions) {
        return EFI_SUCCESS;
    } else {
        return EFI_INVALID_PARAMETER;
    }
}
