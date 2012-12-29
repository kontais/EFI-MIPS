/*
 * Copyright (c) 1999, 2000
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

#include "locale_protocol.h"

//
//  Initialization entry points
//
extern EFI_STATUS				InitLocaleInterface (VOID);
extern EFI_LOCALE_INTERFACE		EfiLocaleInterface;

//
//  Global EFI table pointer
//
EFI_SYSTEM_TABLE		*gST;
EFI_BOOT_SERVICES		*gBS;

//
//  System Table corruption workaround
//
static	EFI_SYSTEM_TABLE	BackupSystemTable;

//
//  Important EFI call table pointers
//
EFI_HANDLE				hImage;

EFI_GUID	LocaleGuid = EFI_LOCALE_PROTOCOL;

EFI_STATUS
EFIAPI
EfiLocaleEntry (
	EFI_HANDLE			ImageHandle,
	EFI_SYSTEM_TABLE	*SystemTable
	)
/*++

Routine Description:

	Main entry point to the locale protocol interface.

Arguments:

	ImageHandle		- Handle to our image
	SystemTable		- Pointer to EFI system table

Returns:

	EFI_STATUS

--*/
{
	EFI_STATUS	Status;
	EFI_HANDLE	Handle;
	UINTN		BufferSize = sizeof(Handle);


	//
	//  Remember our heritage
	//
	
	hImage = ImageHandle;

#if 1
	//
	//  Workaround loaded protocol bug
	//

	BackupSystemTable = *SystemTable;
	gST = &BackupSystemTable;
#else
	gST = SystemTable;
#endif
	gBS = gST->BootServices;

	//
	//  Since multiple locals can be loaded at the same time, it is not an
	//  error if a local protocol is already loaded.
	//
	//Status = gBS->LocateHandle (ByProtocol, &LocaleGuid, NULL, &BufferSize, &Handle);
	//if (Status != EFI_NOT_FOUND) {
		//return (EFI_ALREADY_STARTED);
	//}

	//
	//  Initialize locale protocol interface
	//

	Status = InitLocaleInterface();

	if (!EFI_ERROR (Status)) {
		//
		//  Install our interface
		//

		Handle = NULL;
		Status = gBS->InstallProtocolInterface (
								&Handle,
								&LocaleGuid,
								EFI_NATIVE_INTERFACE,
								&EfiLocaleInterface);
	}

	return (Status);
}
