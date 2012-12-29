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

#include <atk_mp.h>
#include <atk_libc.h>
#include <efilib.h>
#include <stdlib.h>

#include "debug.h"

#define	MAX_PROCESSORS  2   // Max number of processors supported
#define	MAX_TESTS       1   // Max number of AP tests supported

typedef struct	ApArgumentStruct
{
	UINT64	Results ;	// Communicate results between APs and BSP
	UINT64	Done ;		// Communicate status between APs and BSP
} AP_ARGUMENT ;

EFI_MP_INTERFACE  *Mp        = NULL ;
EFI_GUID          MpProtocol = EFI_MP_PROTOCOL ;
EFI_HANDLE        *MpHandles = NULL ;

EFI_MP_PROC_INFO  *ApProcInfo[MAX_PROCESSORS] ;	// AP info structures
AP_ARGUMENT       ApArgument[MAX_PROCESSORS] ;	// AP argument structures


//
//  Name:
// 		InitializeMpProtocol
//
//  Description:
//		Initialize MP protocol.
//
//  Arguments:
//		None	
//
//  Returns:
//		TRUE			- Successfully initialized interface to MP protocol
//		FALSE			- Unable to successfully initialize interface to MP protocol 
//

BOOLEAN
InitializeMpProtocol(
	void
	)
{
	EFI_STATUS		Status ;

	if( ( Status = LibLocateProtocol( &MpProtocol, (VOID**)&Mp ) ) != EFI_SUCCESS )
	{
		return( FALSE ) ;
	}

  	return( TRUE ) ;
}



//
//	Name:
//		APTestSumAll
//
//	Description:
//		AP test loop to sum all integer numbers [0..100].
//
//	Arguments:
//		None
//
//	Returns:
//		None
//
//	Notes:
//		This routine is an AP test loop and as such it never returns to the caller.
//		Execution should be started via the StartProcessor API and terminated via
//		the StopProcessor API.
//
void
APTestSumAll(
	AP_ARGUMENT		*ApArgument
	)
{
	UINT64	Sum = 0 ;
	UINTN	i ;	

	// Set global variables to indicate status and results
	ApArgument->Results = 0 ;
	ApArgument->Done = FALSE ;

	// Sum all integers [0..100] 
	for( i= 0 ; i <= 100 ; i++ )
	{
		Sum += i ;
	}	

	// Set global variables to indicate status and results
	ApArgument->Results = Sum ;
	ApArgument->Done = TRUE ;

	while( 1 ) ;
}



//
//	Name:
//		APTestSumEven
//
//	Description:
//		AP test loop to sum all even integer numbers [0..100].
//
//	Arguments:
//		None
//
//	Returns:
//		None
//
//	Notes:
//		This routine is an AP test loop and as such it never returns to the caller.
//		Execution should be started via the StartProcessor API and terminated via
//		the StopProcessor API.
//
void
APTestSumEven(
	AP_ARGUMENT		*ApArgument
	)
{
	UINT64	Sum = 0 ;
	UINTN	i ;	

	// Set global variables to indicate status and results
	ApArgument->Results = 0 ;
	ApArgument->Done = FALSE ;

	// Sum all integers [0..100] 
	for( i= 0 ; i <= 100 ; i += 2 )
	{
		Sum += i ;
	}	

	// Set global variables to indicate status and results
	ApArgument->Results = Sum ;
	ApArgument->Done = TRUE ;

	while( 1 ) ;
}



//
//	Name:
//		APTestSumOdd
//
//	Description:
//		AP test loop to sum all odd integer numbers [0..100].
//
//	Arguments:
//		None
//
//	Returns:
//		None
//
//	Notes:
//		This routine is an AP test loop and as such it never returns to the caller.
//		Execution should be started via the StartProcessor API and terminated via
//		the StopProcessor API.
//
void
APTestSumOdd(
	AP_ARGUMENT		*ApArgument
	)
{
	UINT64	Sum = 0 ;
	UINTN	i ;	

	// Set global variables to indicate status and results
	ApArgument->Results = 0 ;
	ApArgument->Done = FALSE ;

	// Sum all integers [0..100] 
	for( i= 1 ; i <= 100 ; i += 2 )
	{
		Sum += i ;
	}	

	// Set global variables to indicate status and results
	ApArgument->Results = Sum ;
	ApArgument->Done = TRUE ;

	while( 1 ) ;
}



//
//  Name:
// 		Main	
//
//  Description:
//		Main entry point for mptest.
//
//  Arguments:
//		None	
//
//  Returns:
//		0 - Successful	
//		1 - Error
//

int
main(
	int		Argc,
	char	**Argv
	)
{
	UINT8				NumProcessors = 0,
						NumApProcessors = 0,
						NumApStoppedProcessors = 0 ;
	UINTN				BufferCount,
						ApTestIndex,
						i,
						j ;
	EFI_MP_PROC_INFO	*Buffer,
						*MpProcInfo ;
	EFI_STATUS			Status ;

	// Initialize libefi
	InitializeLib( _LIBC_EFIImageHandle, _LIBC_EFISystemTable );

	// Initialize protocol interface
	if( InitializeMpProtocol() == FALSE )
	{
		Print( L"InitializeMpProtocol:  Unable to initialize MP protocol interface\n" ) ;
		goto Done ;
	}

	// Initialize protocol
	if( ( Status = Mp->InitMpProtocol( Mp ) ) != EFI_SUCCESS )
	{
		Print( L"Mp->InitMpProtocol:  Unable to initialize MP protocol instance\n" ) ;
		goto Done ;
	}

	// Determine number of processors
	if( ( Status = Mp->GetNumEnabledProcessors( Mp, &NumProcessors ) ) != EFI_SUCCESS )
	{
		Print( L"Mp->GetNumEnabledProcessors:  Unable to determine number of processors\n" ) ;
		goto Cleanup ;
	}

	// Get processor information
	BufferCount = 0 ;
	Buffer = NULL ;
	if( ( Status = Mp->GetEnabledProcessorsInfo( Mp, Buffer, &BufferCount ) ) != EFI_BUFFER_TOO_SMALL )
	{
		Print( L"Mp->GetEnabledProcessorsInfo:  Unable to get processor info\n" ) ;
		goto Cleanup ;
	}

	// Allocate required buffer
	if( ( Buffer = calloc( BufferCount, sizeof( EFI_MP_PROC_INFO ) ) ) == NULL )
	{
		Print( L"calloc:  Buffer allocation failed\n" ) ;
		goto Cleanup ;
	}

	// Get processor information
	if( ( Status = Mp->GetEnabledProcessorsInfo( Mp, Buffer, &BufferCount ) ) != EFI_SUCCESS )
	{
		Print( L"Mp->GetEnabledProcessorsInfo:  Unable to get processor info\n" ) ;
		goto Cleanup ;
	}

	// Display buffer information
	for( i = 0, MpProcInfo = Buffer ; i < BufferCount ; i++, MpProcInfo++ )
	{
		Print( L"MpProcInfo Record #%x: ", i ) ;
		Print( L"  ACPI Processor ID = %x, Flags = %x\n", MpProcInfo->ACPIProcessorID, MpProcInfo->Flags ) ;
	}
	Print( L"\n" ) ;

	// Initialize AP arrays 
	for( i = 0 ; i < MAX_PROCESSORS ; i++ )
	{
		ApProcInfo[i]			= NULL ;
		ApArgument[i].Results	= 0 ;
		ApArgument[i].Done		= FALSE ;
	}

	// Enumerate APs 
	for( i = 0, j = 0, MpProcInfo = Buffer ; ( i < MAX_PROCESSORS && i < BufferCount ) ; i++, MpProcInfo++ )
	{
		if( MpProcInfo->Flags & EFI_AP_FLAG )
		{
			ApProcInfo[j] = MpProcInfo ;
			NumApProcessors++ ;
			j++ ;
		}
	}

	// AP not present 
	if( NumApProcessors == 0 )
	{
		Print( L"Unable to identify an AP\n" ) ;
		goto Cleanup ;
	}

	// Set starting address for APs
	for( i = 0 ; i < NumApProcessors ; i++ )
	{
		ApTestIndex = i % MAX_TESTS ;

		if( ApTestIndex == 0 )
		{
			if( ( Status = Mp->StartProcessorAddress( Mp, ApProcInfo[i]->ACPIProcessorID, ( void * ) APTestSumAll, ( void * ) &ApArgument[i] ) ) != EFI_SUCCESS )
			{
				goto Cleanup ;
			}
			Print( L"Initialized AP(%x) with APTestSumAll()\n", ApProcInfo[i]->ACPIProcessorID ) ;
		}
		else if( ApTestIndex == 1 )
		{
			if( ( Status = Mp->StartProcessorAddress( Mp, ApProcInfo[i]->ACPIProcessorID, ( void * ) APTestSumEven, ( void * ) &ApArgument[i] ) ) != EFI_SUCCESS )
			{
				goto Cleanup ;
			}
			Print( L"Initialized AP(%x) with APTestSumEven()\n", ApProcInfo[i]->ACPIProcessorID ) ;
		}
		else if( ApTestIndex == 2 )
		{
			if( ( Status = Mp->StartProcessorAddress( Mp, ApProcInfo[i]->ACPIProcessorID, ( void * ) APTestSumOdd, ( void * ) &ApArgument[i] ) ) != EFI_SUCCESS )
			{
				goto Cleanup ;
			}
			Print( L"Initialized AP(%x) with APTestSumOdd()\n", ApProcInfo[i]->ACPIProcessorID ) ;
		}
	}
	Print( L"\n" ) ;

	// Send start message to APs
	for( i = 0 ; i < NumApProcessors ; i++ )
	{
		Print( L"Preparing To Send AP(%x) Start Message\n", ApProcInfo[i]->ACPIProcessorID ) ;
		if( ( Status = Mp->StartProcessor( Mp, ApProcInfo[i]->ACPIProcessorID ) ) != EFI_SUCCESS )
		{
			goto Cleanup ;
		}
		Print( L"Sent AP(%x) Start Message\n", ApProcInfo[i]->ACPIProcessorID ) ;
	}
	Print( L"\n" ) ;

	// Check AP status
	while( NumApStoppedProcessors != NumApProcessors )
	{
		for( i = 0 ; i < NumApProcessors ; i++ )
		{
			if( ApArgument[i].Done == TRUE )
			{
				// Stop the processor
				Print( L"Preparing To Send AP(%x) Stop Message\n", ApProcInfo[i]->ACPIProcessorID ) ;
				if( ( Status = Mp->StopProcessor( Mp, ApProcInfo[i]->ACPIProcessorID ) ) != EFI_SUCCESS )
				{
					goto Cleanup ;
				}
				Print( L"Sent AP(%x) Stop Message\n", ApProcInfo[i]->ACPIProcessorID ) ;
				Print( L"AP(%x) Returned = %d\n", ApProcInfo[i]->ACPIProcessorID, ApArgument[i].Results ) ;

				// Increment number of stopped processors
				NumApStoppedProcessors++ ;

				// Make sure don't check it again
				ApArgument[i].Done = FALSE ;
			}
		}
	}
	Print( L"\n" ) ;

	Cleanup:

	// Deinitialize protocol
	if( ( Status = Mp->DeInitMpProtocol( Mp ) ) != EFI_SUCCESS )
	{
		Print( L"Mp->DeInitMpProtocol:  Unable to deinitialize MP protocol instance\n" ) ;
	}

	Done:

	return( 0 ) ;
}
