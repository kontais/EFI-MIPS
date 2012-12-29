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

Module Name:

    efi_init.c

Abstract:

    This file contains the entry point for a driver implementing a
    dynamically loaded Python module.


Revision History

--*/
#include "PythonModuleProtocol.h"
#include <atk_libc.h>


//
//  Initialization entry points
//
extern EFI_STATUS InitPythonModuleInterface (VOID);

extern EFI_PYTHON_MODULE_INTERFACE		EfiPythonModuleInterface;
extern struct _inittab Efi_InitTab;

//
//  Important EFI call table pointers
//
EFI_HANDLE				hImage;
EFI_SYSTEM_TABLE		*ModuleIfST;
EFI_BOOT_SERVICES		*ModuleIfBS;
EFI_RUNTIME_SERVICES	*ModuleIfRT;

static  EFI_GUID        PythonModuleGuid = EFI_PYTHON_MODULE_PROTOCOL;
static	EFI_GUID		LoadedImageProtocol = LOADED_IMAGE_PROTOCOL;

EFI_STATUS
CheckIfLoaded(void);

EFI_STATUS
PythonModuleUnload (
    IN EFI_HANDLE       ImageHandle
    );


EFI_STATUS
EFIAPI
EfiPythonModuleEntry(
	EFI_HANDLE			ImageHandle,
	EFI_SYSTEM_TABLE	*SystemTable
	)
/*++

Routine Description:

    Main entry point to a Python dynamically loaded module.

Arguments:
    
    ImageHandle		- Handle to our image
	SystemTable		- Pointer to EFI system table

Returns:

    EFI_STATUS

--*/
{
	EFI_STATUS	Status;

	//
	//  Remember our heritage
	//
	//  Note:  Normally protocols need to make a copy of their system
	//         tables because of an EFI Shell "load" command issue that
	//         frees the system table after it exits back to the shell.
	//         since Python modules are loaded and unloaded by the Python
	//         executive, this is not an issue so we don't make a copy here.
	//

	hImage = ImageHandle;
	ModuleIfST = SystemTable;
	ModuleIfBS = ModuleIfST->BootServices;
	ModuleIfRT = ModuleIfST->RuntimeServices;

	//
	//  Initialize Libc
	//

#ifndef _EFI_EMULATION_
	InitializeLibC (ImageHandle, SystemTable);
#endif

	//
	//  Make sure we are not already loaded
	//
    Status = CheckIfLoaded();
   	if (!EFI_ERROR(Status)) {
        EFI_LOADED_IMAGE        *Image;

        //
        // Add an unload handler to our image
        //
        Status = ModuleIfBS->HandleProtocol (ImageHandle, &LoadedImageProtocol, (void**)&Image);
        if (!EFI_ERROR(Status)) {
            Image->Unload = PythonModuleUnload;
 	   		//
			//  Initialize the interface
			//

			Status = InitPythonModuleInterface();
	    	if (!EFI_ERROR(Status)) {

	    		//
				//  Install our interface
				//
				Status = ModuleIfBS->InstallProtocolInterface (
										&ImageHandle,
										&PythonModuleGuid,
										EFI_NATIVE_INTERFACE,
										&EfiPythonModuleInterface);
			}
		}
	}

	if (EFI_ERROR (Status)) {
    	// Have libc do its cleanup stuff if there was an error
    	_LIBC_Cleanup();
    }

	return (Status);
}

EFI_STATUS
PythonModuleUnload (
    IN EFI_HANDLE       ImageHandle
    )
{
    // Uninstall our interface and do a graceful exit of libc
    ModuleIfBS->UninstallProtocolInterface (ImageHandle, &PythonModuleGuid, &EfiPythonModuleInterface);
    _LIBC_Cleanup();

    return EFI_SUCCESS;
}

EFI_STATUS
CheckIfLoaded()
{
    EFI_STATUS	Status;
    UINTN		NumHandles, BufferSize;
    EFI_HANDLE  *Buffer;
    EFI_PYTHON_MODULE_INTERFACE *ModuleInterface = NULL;
    
    BufferSize = sizeof(EFI_HANDLE) * 10;

RetryLocate:
    Buffer = (EFI_HANDLE*)malloc(BufferSize);
    if (Buffer == NULL)
    	return (EFI_OUT_OF_RESOURCES);

    Status = ModuleIfBS->LocateHandle(ByProtocol, &PythonModuleGuid, NULL, &BufferSize, Buffer);

    if (Status == EFI_NOT_FOUND) {
        // No modules loaded is just fine
        Status = EFI_SUCCESS;
        goto out;
    } else if (Status == EFI_BUFFER_TOO_SMALL) {
        BufferSize *= 2;
    	free (Buffer);
    	goto RetryLocate;
   	} else if (EFI_ERROR(Status)) {
   	    goto out;
   	}

    NumHandles = BufferSize / sizeof(EFI_HANDLE);
    if (!EFI_ERROR(Status)) {
        char    *Name;
        UINTN     i;
        for (i = 0; i < NumHandles; ++i) {
            //
            // See if this one has the right name
            //
            Status = ModuleIfBS->HandleProtocol(Buffer[i], &PythonModuleGuid, (void**)&ModuleInterface);
            if (!EFI_ERROR(Status)) {
                Status = ModuleInterface->ModuleName(ModuleInterface, &Name);
            }
            if (!EFI_ERROR(Status)) {
                if (strcmp(Efi_InitTab.name, Name) == 0) {
                	Status = EFI_ALREADY_STARTED;
                	goto out;
                }
            }
            ModuleInterface = NULL;
        }
    }

    //
    //  We didn't find this module already loaded.
    //
    Status = EFI_SUCCESS;

out:
    free (Buffer);
    return (Status);
}

//
//  Allow NT emulation debug
//

#ifdef __cplusplus
extern "C" {
#endif

EFI_DRIVER_ENTRY_POINT(EfiPythonModuleEntry);

#ifdef __cplusplus
}
#endif
