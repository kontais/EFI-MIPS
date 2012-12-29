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

    efi_interface.c

Abstract:

    This file implements EFI Python Module Protocol Interface.



Revision History

--*/

#include "EfiInterface.h"
#include "PythonModuleProtocol.h"
#include <stdlib.h>
#include <efidebug.h>

// Pointer to the table of Python's exported functions
EFI_PYTHON_API  *PythonApiFunctions = NULL;

// Each module has to define one of these so this library
//  can connect to it.
extern struct _inittab Efi_InitTab;

// Functions that initialize the various sections of
//  our interface library.
extern EFI_STATUS InitAbstract(void);
extern EFI_STATUS InitCeval(void);
extern EFI_STATUS InitClassobject(void);
extern EFI_STATUS InitCobject(void);
extern EFI_STATUS InitCompile(void);
extern EFI_STATUS InitComplexobject(void);
extern EFI_STATUS InitDictobject(void);
extern EFI_STATUS InitErrors(void);
extern EFI_STATUS InitFileobject(void);
extern EFI_STATUS InitFloatobject(void);
extern EFI_STATUS InitFuncobject(void);
extern EFI_STATUS InitImport(void);
extern EFI_STATUS InitIntobject(void);
extern EFI_STATUS InitListobject(void);
extern EFI_STATUS InitLongobject(void);
extern EFI_STATUS InitMethodobject(void);
extern EFI_STATUS InitModsupport(void);
extern EFI_STATUS InitModuleobject(void);
extern EFI_STATUS InitNode(void);
extern EFI_STATUS InitObject(void);
extern EFI_STATUS InitObjimpl(void);
extern EFI_STATUS InitPydebug(void);
extern EFI_STATUS InitPythonrun(void);
extern EFI_STATUS InitStringobject(void);
extern EFI_STATUS InitTupleobject(void);
extern EFI_STATUS InitSysmodule(void);


// Other external items
extern EFI_HANDLE				*hImage;

EFI_STATUS
ModuleName (
    IN  EFI_PYTHON_MODULE_INTERFACE *This,
    IN OUT CHAR8                    **Name
    )
/*++

Routine Description:

    Return a pointer to this module's name.

Arguments:

    This        -   A pointer to the EFI_PYTHON_MODULE_INTERFACE instance.
    ModuleName  -   A pointer to the pointer where the name is returned.

Returns:

    EFI_SUCCESS             The function completed successfully
    EFI_INVALID_PARAMETER   No pointer for the returned handle

--*/
{
    EFI_STATUS Status;

    if (Name) {
        *Name = Efi_InitTab.name;
        Status = EFI_SUCCESS;
    } else {
        Status = EFI_INVALID_PARAMETER;
    }
    return Status;
}

EFI_STATUS
InitModule (
    IN  EFI_PYTHON_MODULE_INTERFACE *This,
    IN  EFI_PYTHON_API              *ApiFunctions
    )
/*++

Routine Description:

    Establish connections between Python and this module.

Arguments:

    This         -   A pointer to the EFI_PYTHON_MODULE_INTERFACE instance.
    ApiFunctions -   A pointer to the table of functions in the Python API.

Returns:

    EFI_SUCCESS             The function completed successfully
    EFI_INVALID_PARAMETER   No pointer to the API functions table

--*/
{
    EFI_STATUS Status = EFI_SUCCESS;

    if (ApiFunctions) {

        // Save the pointer to the function table.
        PythonApiFunctions = ApiFunctions;

        // Call the functions that initialize our interface library
        if (!EFI_ERROR(Status)) {
            Status = InitModuleobject();
        }
        if (!EFI_ERROR(Status)) {
            Status = InitAbstract();
        }
        if (!EFI_ERROR(Status)) {
            Status = InitCeval();
        }
        if (!EFI_ERROR(Status)) {
            Status = InitClassobject();
        }
        if (!EFI_ERROR(Status)) {
            Status = InitCobject();
        }
        if (!EFI_ERROR(Status)) {
            Status = InitCompile();
        }
        if (!EFI_ERROR(Status)) {
            Status = InitComplexobject();
        }
        if (!EFI_ERROR(Status)) {
            Status = InitDictobject();
        }
        if (!EFI_ERROR(Status)) {
            Status = InitFileobject();
        }
        if (!EFI_ERROR(Status)) {
            Status = InitFloatobject();
        }
        if (!EFI_ERROR(Status)) {
            Status = InitFuncobject();
        }
        if (!EFI_ERROR(Status)) {
            Status = InitImport();
        }
        if (!EFI_ERROR(Status)) {
            Status = InitIntobject();
        }
        if (!EFI_ERROR(Status)) {
            Status = InitListobject();
        }
        if (!EFI_ERROR(Status)) {
            Status = InitLongobject();
        }
        if (!EFI_ERROR(Status)) {
            Status = InitMethodobject();
        }
        if (!EFI_ERROR(Status)) {
            Status = InitModsupport();
        }
        if (!EFI_ERROR(Status)) {
            Status = InitModuleobject();
        }
        if (!EFI_ERROR(Status)) {
            Status = InitNode();
        }
        if (!EFI_ERROR(Status)) {
            Status = InitObject();
        }
        if (!EFI_ERROR(Status)) {
            Status = InitObjimpl();
        }
        if (!EFI_ERROR(Status)) {
            Status = InitPydebug();
        }
        if (!EFI_ERROR(Status)) {
            Status = InitErrors();
        }
        if (!EFI_ERROR(Status)) {
            Status = InitPythonrun();
        }
        if (!EFI_ERROR(Status)) {
            Status = InitStringobject();
        }
        if (!EFI_ERROR(Status)) {
            Status = InitSysmodule();
        }
        if (!EFI_ERROR(Status)) {
            Status = InitTupleobject();
        }

        // Call the module's init function so it can register its methods
        if (!EFI_ERROR(Status)) {
            Efi_InitTab.initfunc();
        }
    } else {
        Status = EFI_INVALID_PARAMETER;
    }
    return Status;
}

EFI_STATUS
GetUnloadHandle (
    IN  EFI_PYTHON_MODULE_INTERFACE *This,
    IN  OUT EFI_HANDLE              *UnloadHandle
    )
/*++

Routine Description:

    Return the handle needed to unload this module.

Arguments:

    This         -   A pointer to the EFI_PYTHON_MODULE_INTERFACE instance.
    UnloadHandle -   A pointer to where to return the handle.

Returns:

    EFI_SUCCESS             The function completed successfully
    EFI_INVALID_PARAMETER   No pointer for the returned handle

--*/
{
    EFI_STATUS Status;

    if (UnloadHandle) {
        *UnloadHandle = *hImage;
        Status = EFI_SUCCESS;
    } else {
        Status = EFI_INVALID_PARAMETER;
    }
    return Status;
}


EFI_STATUS
InitPythonModuleInterface(
	VOID
	)
/*++

Routine Description:

    Initialize the Python module interface.

Arguments:
    
    TBD

Returns:

    EFI_SUCCESS

--*/
{
	DEBUG ((D_LOAD, "Executing InitPythonModuleInterface\n"));
	    
	return (EFI_SUCCESS);
}






//
//  The EFI Python Module Interface Table
//
EFI_PYTHON_MODULE_INTERFACE EfiPythonModuleInterface = {

	EFI_PYTHON_MODULE_INTERFACE_REVISION,

	ModuleName,
	InitModule,
    GetUnloadHandle
};

/*++

Routine Description:

	EFI emulation routine.

Arguments:
    
	TBD

Returns:

	Pointer to an EFI Python Module interface

--*/
EFI_PYTHON_MODULE_INTERFACE*
GetEfiPythonModuleInterface() { return &EfiPythonModuleInterface; }
