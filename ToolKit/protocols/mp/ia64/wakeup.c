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

#include <efi.h>
#include <efilib.h>

#include "debug.h"

#include "asmapi.h"
#include "proclist.h"
#include "util.h"
#include "wakeup.h"

#define	SAL_SET_VECTORS				0x01000000
#define	BOOT_RENDEZ					2
#define	INIT_HANDLER				1

UINT64 SAL_WakeUpInterrupt;

//
// Local function prototypes
//

UINT64
InitBootRendezvous(
	BOOT_RENDEZ_CODE_T BootRendezvousCode
	) ;


//
//  Name:
//		WakeupAPs
//
//  Description:
//		Wakes APs to the boot rendezvous code.
//
//  Arguments:
//		BootRendezvousCode	- Boot rendezvous code	
//
//  Returns:
//		WAKEUP_SUCCESS		- Successfully woke APs
//		WAKEUP_NO_EXTINT	- Unable to determine external interrupt to wake APs
//		WAKEUP_RENDEZ_ERR	- Unable to register boot rendezvous code with SAL
//
UINT64
WakeupAPs(
	BOOT_RENDEZ_CODE_T BootRendezvousCode
	)
{
	UINT64	ExternalInterrupt ;
	PROCREC	*ProcRec = NULL ;

	// Get SAL wakeup vector
	if( LibGetSalWakeupVector( &ExternalInterrupt ) != EFI_SUCCESS )
	{
		return( WAKEUP_NO_EXTINT ) ;
	}
	SAL_WakeUpInterrupt = ExternalInterrupt;

	// Register boot rendezvous entry
	if( InitBootRendezvous( BootRendezvousCode ) != 0 )
	{
		return( WAKEUP_RENDEZ_ERR ) ;
	}

  return( WAKEUP_SUCCESS ) ;
}



//
//  Name:
//      InitBootRendezvous
//
//  Description:
//		Register boot rendezvous code for APs
//
//  Arguments:
//		BootRendezvousCode	- Boot rendezvous code	
//
//  Returns:
//		0					- Call completed without error
//		-2					- Invalid argument
//		-3					- Call completed with error
//		-4					- Virtual address not registered
//
UINT64
InitBootRendezvous(
	BOOT_RENDEZ_CODE_T BootRendezvousCode
	)
{
	rArg	SalReturnVal ;
    UINT64  ProcAddress1;
	UINT64  ProcGP1;

	ProcAddress1 = ( UINT64 ) ( ( PLABEL * ) BootRendezvousCode)->ProcEntryPoint;
	ProcGP1 = ( UINT64 ) ( ( PLABEL * )BootRendezvousCode)->GP ;

	// Register rendezvous vector with SAL, this will be invoked for a Start message
	LibSalProc( SAL_SET_VECTORS, BOOT_RENDEZ,ProcAddress1, ProcGP1, 0, 0, 0, 0, &SalReturnVal ) ;
	return( SalReturnVal.p0 );
}
//
//  Name:
//      RegisterINITEventHandler
//
//  Description:
//		Register INIT code for APs
//
//  Arguments:
//		INITEventCode	- INIT event code	
//
//  Returns:
//		0					- Call completed without error
//		-2					- Invalid argument
//		-3					- Call completed with error
//		-4					- Virtual address not registered
//
UINT64
RegisterINITEventHandler()
{
	rArg	SalReturnVal ;
    UINT64  ProcAddress;
	UINT64  ProcGP;

	ProcAddress = ( UINT64 ) ( ( PLABEL * ) AP_INITHandler)->ProcEntryPoint;
	ProcGP = ( UINT64 ) ( ( PLABEL * )AP_INITHandler)->GP ;

	// Register INIT vector with SAL, this gets invoked for a STOP message
	// we are registering the same handler for the BSP, and AP

	LibSalProc( SAL_SET_VECTORS, INIT_HANDLER, ProcAddress, ProcGP, 0 , ProcAddress, ProcGP, 0, &SalReturnVal );

	return( SalReturnVal.p0 ) ;
}

