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

#include <efi.h>
#include <efilib.h>
#include <atk_libc.h>
#include <atk_ppp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pppd.h"


int  main( int argc, char **argv);
void StopInterruptTimer( void );
void StopEventTimer( void );
void Exit(int ret);

#define _TICK_INTERVAL_	5000	// 500usec

static EFI_SYSTEM_TABLE     BackupSystemTable;
static EFI_BOOT_SERVICES	*BootServices;

EFI_GUID	LoadedImageProtocol = LOADED_IMAGE_PROTOCOL;

static EFI_STATUS
Unload( EFI_HANDLE ImageHandle )
{
	if (phase != PHASE_DEAD ) {
		StopInterruptTimer();
		StopEventTimer();
		cleanup();
		_LIBC_Cleanup();
	}

	return (EFI_SUCCESS);
}

static CHAR16	*name = L"pppd";

EFI_STATUS
EfiHook(
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
	)
{
	EFI_LOADED_IMAGE	*Image;
	EFI_STATUS			Status;
	CHAR16				**argv;
	int					argc, ret, i;

	//
	//  Make a copy of the system table
	//
	memcpy( &BackupSystemTable, SystemTable, sizeof(EFI_SYSTEM_TABLE) );

	//
	//  Initialize libc
	//
	InitializeLibC( ImageHandle, &BackupSystemTable );

	//
	//  Set a local pointer to boot services
	//
	BootServices = BackupSystemTable.BootServices;

	// 
	//  Get our image protocol so we can install an unload handler.
	//
   	Status = BootServices->HandleProtocol (
   					ImageHandle,
   					&LoadedImageProtocol,
   					(VOID*)&Image
   					);

	if (EFI_ERROR(Status)) {
		return (EFI_LOAD_ERROR);
	}

	// 
	//  Install unload handler
	//
	Image->Unload = Unload;

	//
	//  Get arguments from load options
	//
	_LIBC_GetArgsFromLoadOptions( &argc, &argv );

    if (argc == 0) {
        argc = 1;
        argv = &name;
    }

	//
	//  Convert them in place to ASCII
	//
	for ( i = 0; i < argc; i++ )
		wcstombs((char*)argv[i], (wchar_t*)argv[i], wcslen(argv[i]) + 1);

	//
	//  Start the app
	//
	if (ret = main( argc, (char**)argv )) {
		//
		//  There was an error - clean up
		//
		Exit(ret);
	} else {
		_LIBC_FreeArgs( argv );
	}

	return (EFI_SUCCESS);
}

void
Exit(int ret)
{
	StopInterruptTimer();
	StopEventTimer();
	_LIBC_EfiExit(EFIERR_OEM( ret ), 0, NULL);
}


static EFI_GUID pppGuid = PPP_PROTOCOL_GUID;

void*
pppGetLineDisc( void )
{
    EFI_HANDLE      Handle;
    UINTN           BufferSize;
    EFI_STATUS      Status;
    VOID            *pIface;

    BufferSize = sizeof(EFI_HANDLE);
    Status = BootServices->LocateHandle (
						ByProtocol,
						&pppGuid,
						NULL,
						&BufferSize,
						&Handle
						);
	if (EFI_ERROR (Status)) {
		return NULL;
	}

	Status = BootServices->HandleProtocol (
						Handle,
						&pppGuid,
						&pIface
						);
	if (EFI_ERROR (Status)) {
		return NULL;
	}

	return ( pIface );
}

static EFI_STATUS
StartTimer(
	IN  EFI_TIMER_DELAY		Type,
	IN  UINT64				Interval,
	IN  EFI_EVENT_NOTIFY	Function,
	IN  EFI_TPL				Tpl,
	OUT EFI_EVENT			*Event
	)
{
	EFI_STATUS	Status = EFI_SUCCESS;

	/*
	 *  Create and EFI event for timer operations
	 */
	if (*Event == NULL ) {
		Status = BootServices->CreateEvent(
									EVT_TIMER | EVT_NOTIFY_SIGNAL,
									Tpl,
									Function,
									NULL,
									Event
									);
	}

	if ( ! EFI_ERROR( Status ) ) {
		/*
		 * Set the timer
		 */
		Status =  BootServices->SetTimer( *Event, Type, Interval );
	}

	return Status;
}

static void
StopTimer( EFI_EVENT Event)
{
	if ( Event ) {
		BootServices->SetTimer( Event, TimerCancel, 0 );
		BootServices->CloseEvent( Event );
	}
}

static	EFI_EVENT	IntrClockEvent = NULL;

void
StartInterruptTimer(void)
{
	extern pseudoReadIntr(EFI_EVENT*, VOID*);

	if (EFI_ERROR( StartTimer( TimerPeriodic,
							   _TICK_INTERVAL_,
							   pseudoReadIntr,
							   TPL_NOTIFY,
							   &IntrClockEvent ))) {
		printf("Error starting pseudoReadIntr timer\n");
	}
}

void
StopInterruptTimer(void)
{
	StopTimer( IntrClockEvent );
	IntrClockEvent = NULL;
}

/*
 *  There should be only one outstanding timer event at any one time
 */
static EFI_EVENT	TimerEvent = NULL;
void
StartEventTimer( void *func, int usec )
{
	if (EFI_ERROR( StartTimer( TimerRelative,
							   usec * 10,
							   (EFI_EVENT_NOTIFY)func,
							   TPL_CALLBACK,
							   &TimerEvent ))) {
		printf("Error starting timer event timer\n");
	}
}

void
StopEventTimer(void)
{
	StopTimer( TimerEvent );
	TimerEvent = NULL;
}

void*
CalloutLock(void)
{
	return ((void*)BootServices->RaiseTPL( TPL_CALLBACK ));
}

void
CalloutUnlock(void* level)
{
	BootServices->RestoreTPL( (EFI_TPL) level );
}

static EFI_EVENT PacketEvent = NULL;
void*
CreatePacketEvent( void *Callback )
{
	EFI_STATUS	Status;

	Status = BootServices->CreateEvent(
								EVT_NOTIFY_SIGNAL,
								TPL_CALLBACK,
								Callback,
								NULL,
								&PacketEvent
								);
	if (EFI_ERROR(Status))
		return NULL;
	else
		return &PacketEvent;
}

void
FreePacketEvent( void *Event )
{
	if (Event)
		BootServices->CloseEvent( *((EFI_EVENT*)Event) );	
}


EFI_DRIVER_ENTRY_POINT( EfiHook );
