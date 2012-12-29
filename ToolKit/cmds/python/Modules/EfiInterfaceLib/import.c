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

    import.c

Abstract:

    This file implements wrappers for the functions declared in import.h.


Revision History

--*/


#include "EfiInterface.h"

extern EFI_PYTHON_API  *PythonApiFunctions;

/* These functions are wrappers around the function pointers in the EFI_PYTHON_API
 *  structure that make it so a module's source code doesn't have to be modified
 *  to be built as an dynamically loaded EFI driver.
 */

long PyImport_GetMagicNumber (void)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyImport_GetMagicNumber) {
        return PythonApiFunctions->EPA_PyImport_GetMagicNumber();
    } else {
        return 0;
    }
}

PyObject * PyImport_ExecCodeModule (char *name, PyObject *co)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyImport_ExecCodeModule) {
        return PythonApiFunctions->EPA_PyImport_ExecCodeModule(name, co);
    } else {
        return NULL;
    }
}

PyObject * PyImport_ExecCodeModuleEx (char *name, PyObject *co, char *pathname)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyImport_ExecCodeModuleEx) {
        return PythonApiFunctions->EPA_PyImport_ExecCodeModuleEx(name, co, pathname);
    } else {
        return NULL;
    }
}

PyObject * PyImport_GetModuleDict (void)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyImport_GetModuleDict) {
        return PythonApiFunctions->EPA_PyImport_GetModuleDict();
    } else {
        return NULL;
    }
}

PyObject * PyImport_AddModule (char *name)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyImport_AddModule) {
        return PythonApiFunctions->EPA_PyImport_AddModule(name);
    } else {
        return NULL;
    }
}

PyObject * PyImport_ImportModule (char *name)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyImport_ImportModule) {
        return PythonApiFunctions->EPA_PyImport_ImportModule(name);
    } else {
        return NULL;
    }
}

PyObject * PyImport_ImportModuleEx (char *name, PyObject *globals, PyObject *locals, PyObject *fromlist)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyImport_ImportModuleEx) {
        return PythonApiFunctions->EPA_PyImport_ImportModuleEx(name, globals, locals, fromlist);
    } else {
        return NULL;
    }
}

PyObject * PyImport_Import (PyObject *name)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyImport_Import) {
        return PythonApiFunctions->EPA_PyImport_Import(name);
    } else {
        return NULL;
    }
}

PyObject * PyImport_ReloadModule (PyObject *m)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyImport_ReloadModule) {
        return PythonApiFunctions->EPA_PyImport_ReloadModule(m);
    } else {
        return NULL;
    }
}

void PyImport_Cleanup (void)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyImport_Cleanup) {
        PythonApiFunctions->EPA_PyImport_Cleanup();
    }
}

int PyImport_ImportFrozenModule (char *c)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyImport_ImportFrozenModule) {
        return PythonApiFunctions->EPA_PyImport_ImportFrozenModule(c);
    } else {
        return 0;
    }
}

PyObject *_PyImport_FindExtension (char *c1, char *c2)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA__PyImport_FindExtension) {
        return PythonApiFunctions->EPA__PyImport_FindExtension(c1, c2);
    } else {
        return NULL;
    }
}

PyObject *_PyImport_FixupExtension (char *c1, char *c2)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA__PyImport_FixupExtension) {
        return PythonApiFunctions->EPA__PyImport_FixupExtension(c1, c2);
    } else {
        return NULL;
    }
}

struct _inittab ** pPyImport_Inittab = NULL;

int PyImport_AppendInittab (char *name, void (*initfunc)(void))
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyImport_AppendInittab) {
        return PythonApiFunctions->EPA_PyImport_AppendInittab(name, initfunc);
    } else {
        return 0;
    }
}

int PyImport_ExtendInittab (struct _inittab *newtab)
{
    if (PythonApiFunctions && PythonApiFunctions->EPA_PyImport_ExtendInittab) {
        return PythonApiFunctions->EPA_PyImport_ExtendInittab(newtab);
    } else {
        return 0;
    }
}

struct _frozen ** pPyImport_FrozenModules = NULL;

EFI_STATUS InitImport() {
    if (PythonApiFunctions) {
        pPyImport_Inittab = PythonApiFunctions->EPA_PyImport_Inittab;
        pPyImport_FrozenModules = PythonApiFunctions->EPA_PyImport_FrozenModules;
        return EFI_SUCCESS;
    } else {
        return EFI_INVALID_PARAMETER;
    }
}
