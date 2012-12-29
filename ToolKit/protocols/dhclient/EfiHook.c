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

#include <atk_libc.h>
#include <stdio.h>
#include <stdlib.h>


#define DHCP_CLIENT_PROTOCOL \
    { 0x36b31990, 0x3292, 0x11d4, 0x87, 0x97, 0x0, 0x6, 0x29, 0x2e, 0x8a, 0x3b }

#define DHCP_CLIENT_INTERFACE_REVISION   0x00010000

static EFI_GUID DhcpGuid        = DHCP_CLIENT_PROTOCOL;
static EFI_GUID LoadedImageGuid = LOADED_IMAGE_PROTOCOL;

typedef
EFI_STATUS
(EFIAPI *DHCP_GET_IMAGE_HANDLE) (
	IN	struct _DHCP_CLIENT	*This,
	OUT	EFI_HANDLE			*ImageHandle
	);

typedef struct _DHCP_CLIENT {
	UINT64					Revision;
	DHCP_GET_IMAGE_HANDLE	GetImageHandle;
} DHCP_CLIENT_INTERFACE;

extern DHCP_CLIENT_INTERFACE DhcpClientInterface;

// int	main( int argc, char **argv);

char	*argv[] = {
	"dhcpclient",
	"-q",
	/*
	 * add/remove arguments as needed
	 */
};

static EFI_HANDLE			hImage;
static EFI_SYSTEM_TABLE		*DhcpST;
static EFI_BOOT_SERVICES	*DhcpBS;
static EFI_RUNTIME_SERVICES	*DhcpRT;
static EFI_SYSTEM_TABLE		BackupSystemTable;
static EFI_EVENT			DispatchClockEvent = NULL;

extern EFI_STATUS _LIBC_Start_A(EFI_HANDLE ImgHandle, EFI_SYSTEM_TABLE *SysTable);

EFI_STATUS
EfiHook(
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
	)
{
	EFI_STATUS	Status;
	EFI_HANDLE	Handle;
	UINTN		BufferSize = sizeof(Handle);

	/*
	 *  Workaround loaded protocol bug
	 */

	BackupSystemTable = *SystemTable;

	/*
	 *  Remember our heritage
	 */

	hImage = ImageHandle;
	DhcpST = &BackupSystemTable;
	DhcpBS = DhcpST->BootServices;
	DhcpRT = DhcpST->RuntimeServices;

	/*
	 *  Initialize Libc
	 *  InitializeLibC (ImageHandle, &BackupSystemTable);
	 */

	

	/*
	 *  Make sure we are not already loaded
	 */

	Status = DhcpBS->LocateHandle (ByProtocol, &DhcpGuid, NULL, &BufferSize, &Handle);
	if (Status == EFI_NOT_FOUND) {

		/*
		 *  Start the app
		 *
		 *  The protocol interface will get installed in StartTimersAndExit()
		 */

		Status = EFI_SUCCESS;
	      /* if ( main( sizeof(argv) / sizeof(char*), argv ) ) {
	       * Status = EFI_LOAD_ERROR;
               */ 
		_LIBC_Start_A(ImageHandle,&BackupSystemTable);
	} else {
		Status = EFI_ALREADY_STARTED;
	}

	/*
	 *  Check for errors
	 */

	return Status;
}

static void
StopTimer()
{
	/*
	 *  If the timer is running, stop it.
	 */
	if ( DispatchClockEvent ) {
		(void)DhcpBS->SetTimer( DispatchClockEvent, TimerCancel, 0 );
		(void)DhcpBS->CloseEvent( DispatchClockEvent );
	}
}

static EFI_STATUS
DhcpUnload(
	EFI_HANDLE	ImageHandle
	)
{
	/*
	 *  Cleanup libc.  Along with freeing all memory, this should close
	 *  all open handles for us.
	 */
	_LIBC_Cleanup();

	/*
	 *  Stop the timer.
	 */
	StopTimer();

    /*
     *  Unload our interface
     */
    DhcpBS->UninstallProtocolInterface (ImageHandle, 
				                   		&DhcpGuid,
				                   		&DhcpClientInterface
                                        );
	return ( EFI_SUCCESS );
}

void
StartTimersAndExit()
{
	extern DispatchProc( EFI_HANDLE, void *);

	EFI_STATUS			Status;
	EFI_LOADED_IMAGE	*Image;

	/*
	 *  Create and EFI event for timer operations
	 */

	Status = DhcpBS->CreateEvent (EVT_TIMER | EVT_NOTIFY_SIGNAL,
								   TPL_CALLBACK,
								   (EFI_EVENT_NOTIFY)DispatchProc,
								   NULL,
								   &DispatchClockEvent);
	if ( ! EFI_ERROR( Status ) ) {

		/*
		 * Set the timer
		 */

		Status = DhcpBS->SetTimer (DispatchClockEvent,
								   TimerPeriodic,
								   100000000); /* ten seconds */

		if ( ! EFI_ERROR( Status ) ) {
			EFI_HANDLE Handle = NULL;

			/*
			 *  Install our interface
			 */

			Status = DhcpBS->InstallProtocolInterface (
									&hImage,
									&DhcpGuid,
									EFI_NATIVE_INTERFACE,
									&DhcpClientInterface);

			/* 
			 *  Get our image protocol so we can install an unload handler.
			 */
    		if ( ! EFI_ERROR( Status ) ) {
    			Status = DhcpBS->HandleProtocol(
    							hImage,
    							&LoadedImageGuid,
    							(VOID*)&Image);

    			if ( ! EFI_ERROR( Status ) ) {
					/* 
					 *  Install unload handler
					 */
			    	Image->Unload = DhcpUnload;
    			}
    		}
    	}
	}
	
	if ( EFI_ERROR( Status ) ) {
		/*
		 *  There's been trouble and we're not going to remain in memory.
		 *  Shut the timer down and cleanup libc.
		 */
		StopTimer();
		_LIBC_Cleanup();
	}

	DhcpBS->Exit( hImage, Status, 0, NULL );
}

void
EfiExit( int val )
{
	/*
	 *  Stop the timer and exit.
	 */
	StopTimer();
	_LIBC_EfiExit ( EFIERR_OEM( val ), 0, NULL );
}

// EFI_DRIVER_ENTRY_POINT( EfiHook );

static EFI_STATUS
GetImageHandle(
	IN	DHCP_CLIENT_INTERFACE	*This,
	OUT	EFI_HANDLE				*ImageHandle
	)
{
	*ImageHandle = hImage;
	return ( EFI_SUCCESS );
}

DHCP_CLIENT_INTERFACE DhcpClientInterface = {
	DHCP_CLIENT_INTERFACE_REVISION,
	GetImageHandle,
};
