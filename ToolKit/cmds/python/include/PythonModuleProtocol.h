/*
 * Copyright (c) 2000
 * Intel Corporation.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 
 * 3. All advertising materials mentioning features or use of this software must
 *    display the following acknowledgement:
 * 
 *    This product includes software developed by Intel Corporation and its
 *    contributors.
 * 
 * 4. Neither the name of Intel Corporation or its contributors may be used to
 *    endorse or promote products derived from this software without specific
 *    prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY INTEL CORPORATION AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL INTEL CORPORATION OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */

/*++

Copyright (c) 2000  Intel Corporation

Module Name:

    PythonModuleProtocol.h

Abstract:

    This protocol defines the interface between the main body of the Pyhton
    implementation and modules set up as EFI drivers so they may be loaded
    dynamically.

Revision History

--*/
#include <atk_libc.h>
#include "EfiInterface.h"

//
// Python module protocol
//

#define EFI_PYTHON_MODULE_PROTOCOL    \
    { 0x959b5a60, 0x2b58, 0x11d4, 0xa3, 0xbe, 0x0, 0x4, 0xac, 0x9b, 0x7a, 0xe1 }

#define EFI_PYTHON_MODULE_INTERFACE_REVISION   ((UINT64) 0x00010000)


typedef
EFI_STATUS
(EFIAPI *PYTHON_MODULE_NAME) (
    IN struct _PYTHON_MODULE            *This,
    IN OUT CHAR8         		**ModuleName
    );

typedef
EFI_STATUS
(EFIAPI *PYTHON_INIT_MODULE) (
    IN struct _PYTHON_MODULE            *This,
    IN EFI_PYTHON_API			*ApiFunctions
    );

typedef
EFI_STATUS
(EFIAPI *PYTHON_GET_UNLOAD_HANDLE) (
    IN struct _PYTHON_MODULE    *This,
    IN OUT EFI_HANDLE           *UnloadHandle
    );

typedef struct _PYTHON_MODULE {
    UINT64                          	Revision;

    PYTHON_MODULE_NAME			ModuleName;
    PYTHON_INIT_MODULE			InitModule;
    PYTHON_GET_UNLOAD_HANDLE         	GetUnloadHandle;
} EFI_PYTHON_MODULE_INTERFACE;



