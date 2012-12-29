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

#include <setjmp.h>
#include <stdlib.h>
#include <efilib.h>

#include "comm.h"
#include "proclist.h"
#include "message.h"

#include "debug.h"

UINT64	*MessageBuffer ;
UINT64	*AddressBuffer ;
UINT64	*ArgumentBuffer ;
UINT64	*GPBuffer ;
UINT64	**JumpBuffer ;



//
//  Name:
//		CreateAPCommunicationBuffers
//
//  Description:
//		Create shared communication buffers between BSP and APs.
//
//  Arguments:
//		None
//
//  Returns:
//		COMM_SUCCESS		- Successfully created communication buffers
//		COMM_MESSAGE_BUFFER	- Unable to create message buffers 
//

UINT32
CreateAPCommunicationBuffers(
	VOID
	)
{
	PROCREC			*ProcRec = NULL ;	
	UINT32			RetValue = COMM_SUCCESS ;
	UINT16			MaxLocalID = 0 ;
	int				i ;

	// Determine maximum local id for APs
	while( ( ProcRec = GetNextProcListRecord( ProcRec ) ) != NULL )
	{
		if( !( IS_BSP( ProcRec ) ) )
		{
			if( ProcRec->LocalID > MaxLocalID )
			{
				MaxLocalID = ProcRec->LocalID ;
			}
		}
	}
	// Create shared message buffers
	if( ( MessageBuffer = ( UINT64 * ) calloc( MaxLocalID + 1, sizeof( UINT64 ) ) ) == NULL )
	{
		RetValue = COMM_MESSAGE_BUFFER ; 
		goto Done ;
	}

	// Create shared address buffers
	if( ( AddressBuffer = ( UINT64 * ) calloc( MaxLocalID + 1, sizeof( UINT64 ) ) ) == NULL )
	{
		RetValue = COMM_ADDRESS_BUFFER ; 
		goto Done ;
	}

	// Create shared gp buffers
	if( ( GPBuffer = ( UINT64 * ) calloc( MaxLocalID + 1, sizeof( UINT64 ) ) ) == NULL )
	{
		RetValue = COMM_GP_BUFFER ; 
		goto Done ;
	}

	// Create shared argument buffers
	if( ( ArgumentBuffer = ( UINT64 * ) calloc( MaxLocalID + 1, sizeof( UINT64 ) ) ) == NULL )
	{
		RetValue = COMM_ARGUMENT_BUFFER ; 
		goto Done ;
	}

	// Create shared jump buffers
	if( ( JumpBuffer = ( UINT64 ** ) calloc( MaxLocalID + 1, sizeof( UINT64 ) ) ) == NULL )
	{
		RetValue = COMM_JUMP_BUFFER ; 
		goto Done ;
	}

	// Create shared jump buffers
	for( i = 0 ; i <= MaxLocalID ; i++ )
	{
		if( ( JumpBuffer[i] = ( UINT64 * ) calloc( (sizeof( jmp_buf )+16), 1 ) ) == NULL )
		{
			RetValue = COMM_JUMP_BUFFER ;
			goto Done ;
		}
		DPRINT(( L"Jump Buffer[%x] = %x\n", i, &JumpBuffer[i] )) ;
		DPRINT(( L"Jump Buffer[%x] = %x\n", i, JumpBuffer[i] )) ;
	}

	InitAPCommunicationBuffers() ;

	Done:

	return( RetValue ) ;
}



//
//  Name:
//		InitAPCommunicationBuffers
//
//  Description:
//		Initialize shared communication buffers between BSP and APs.
//
//  Arguments:
//		None
//
//  Returns:
//		None
//

VOID
InitAPCommunicationBuffers(
	VOID
	)
{
	PROCREC			*ProcRec = NULL ;	
	UINT16			MaxLocalID = 0 ;
	int				i ;

	// Determine maximum local id for APs
	while( ( ProcRec = GetNextProcListRecord( ProcRec ) ) != NULL )
	{
		if( !( IS_BSP( ProcRec ) ) )
		{
			if( ProcRec->LocalID > MaxLocalID )
			{
				MaxLocalID = ProcRec->LocalID ;
			}
		}
	}

	// Initialize shared message buffers
	for( i = 0 ; i <= MaxLocalID ; i++ )
	{
		MessageBuffer[i] = MESSAGE_NONE ;
		AddressBuffer[i] = 0 ;
		ArgumentBuffer[i] = 0 ;
		GPBuffer[i] = 0 ;

		DPRINT(( L"Message Buffer[%x] = %x\n", i, &MessageBuffer[i] )) ;
		DPRINT(( L"Address Buffer[%x] = %x\n", i, &AddressBuffer[i] )) ;
		DPRINT(( L"Argument Buffer[%x] = %x\n", i, &ArgumentBuffer[i] )) ;
		DPRINT(( L"GP Buffer[%x] = %x\n", i, &GPBuffer[i] )) ;
	}

	return ;
}
