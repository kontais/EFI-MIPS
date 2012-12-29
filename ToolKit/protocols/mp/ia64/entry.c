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
#include <atk_mp.h>
#include <atk_libc.h>

#include <efilib.h>

#include "acpi.h"
#include "asmapi.h"
#include "interface.h"
#include "proclist.h"
#include "wakeup.h"
#include "comm.h"

//
//  Important EFI call table pointers
//
EFI_HANDLE				hImage;
EFI_SYSTEM_TABLE		*MpST;
EFI_BOOT_SERVICES		*MpBS;
EFI_RUNTIME_SERVICES	*MpRT;

EFI_GUID				MpGuid = EFI_MP_PROTOCOL;

UINT64					BspLID ;

static		EFI_SYSTEM_TABLE	BackupSystemTable;

// extern void Stub_IVT_ExternalInterrupt( void ) ;

//
//  Name:
//      MpProtocolEntry
//
//  Description:
//		Entry point to the MP protocol.
//
//  Arguments:
//		ImageHandle:	Image handle
//		SystemTable:	System table
//
//  Returns:
//		EFI_STATUS
//

EFI_STATUS
MpProtocolEntry(
	IN	EFI_HANDLE			ImageHandle,
	IN	EFI_SYSTEM_TABLE	*SystemTable
	)
{
	EFI_STATUS	Status = EFI_SUCCESS;
	EFI_HANDLE	Handle;
	UINTN		BufferSize = sizeof(Handle);

#ifdef EFI32
	Status = EFI_MP_FAILURE ;
	goto Done ;
#endif
	//
	//  Remember our heritage
	//
	hImage = ImageHandle;
	MpST = SystemTable;
	MpBS = MpST->BootServices;
	MpRT = MpST->RuntimeServices;

#if 1
	//
	//  Workaround loaded protocol bug
	//
	BackupSystemTable = *SystemTable;

	//
	//  Initialize Libc
	//
	if( InitializeLibC (ImageHandle, &BackupSystemTable) != 0 )
	{
		Status = EFI_MP_FAILURE ;
		goto Done ;
	}

	//
	// Initialize EFI LIB
	//
	InitializeLib( ImageHandle, &BackupSystemTable ) ;

#else

	//
	//  Initialize Libc
	//
	if( InitializeLibC (ImageHandle, SystemTable) !=0 )
	{
		Status = EFI_MP_FAILURE ;
		goto Done ;
	}

	//
	// Initialize EFI LIB
	//
	InitializeLib( ImageHandle, SystemTable ) ;

#endif

	//
	//  Make sure we are not already loaded
	//
	if( ( Status = MpBS->LocateHandle (ByProtocol, &MpGuid, NULL, &BufferSize, &Handle) ) != EFI_NOT_FOUND )
	{
		Status = EFI_ALREADY_STARTED ;
		goto Done ;
	}
	//
	//  Get local id
	//
	BspLID = GetLocalID() ;
	//
	//  Create processor records
	//
	if( CreateProcListRecords() != ACPI_SUCCESS )
	{
		Status = EFI_MP_ENUM_FAILURE;
		goto Done ;
	}

	//
	//  Create communication buffers
	//
	if( CreateAPCommunicationBuffers() != COMM_SUCCESS )
	{
		Status = EFI_MP_COMM_FAILURE ;
		goto Done ;
	}
	//
	// we register 2 handlers with the SAL, one for wakeup and the other for INIT event.
	// we do not wakeup the processor now (new design)
	// we issue a WakeUp IPI when we get a START request (from the user, through the APIs)
	// we issue an INIT IPI when we get a STOP request.
	// in the INIT, we take control from the user and return back to SAL rendezvous routine.
	// this way the control is always in the SAL rendezvous if the AP is not in use and so
	// we don't have to reboot to load an OS (since SAL is polling for the WakeUp IPI that 
	// may come from the OS)

	if( RegisterINITEventHandler() != WAKEUP_SUCCESS )
	{
		Status = EFI_MP_WAKEUP_FAILURE ;
		goto Done ;
	}

	//
	//  Wakeup APs
	//

	if( WakeupAPs( BootRendezvous ) != WAKEUP_SUCCESS )
	{
		Status = EFI_MP_WAKEUP_FAILURE ;
		goto Done ;
	}

	//
	//  Install our interface
	//
	Handle = NULL;
	if( ( Status = MpBS->InstallProtocolInterface ( &Handle, &MpGuid, EFI_NATIVE_INTERFACE, &MpInterface) ) != EFI_SUCCESS )
	{
		Status = EFI_MP_FAILURE ;
		goto Done ;
	}

	Done:

	return( Status ) ;
}
