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

#include "util.h"
#include "message.h"
#include <efilib.h>

#include "debug.h"

#define	PROCESSOR_INT_BLOCK	0xfee00000
#define BIT63				0x8000000000000000

extern UINT64	*MessageBuffer;
extern UINT64 SAL_WakeUpInterrupt;

UINT64	IPIBlock = 0 ;


//
//  Name:
//		SendIPIMessage	
//
//  Description:
//		Sends a message to a specified processor.
//
//  Arguments:
//		Message				- Message to send
//		DestProcessor		- Destination processor (eid bits[15-8], id bits[7-0])
//
//  Returns:
//		None
//
void
SendIPIMessage(
	UINT64	Message,
	UINT16	DestProcessor
	)
{
	// Copy message to message buffer for specified processor
	MessageBuffer[DestProcessor] = Message ;

	// Send the IPI to the specified processor
	DPRINT(( L"Send IPI (%x)\n", DestProcessor )); 
	switch (Message) {
	case MESSAGE_START:
		SendIPI( DestProcessor, SAL_WakeUpInterrupt);
		break;
	case MESSAGE_STOP:
		SendIPI( DestProcessor, MESSAGE_INIT);
		break;
	}
	DPRINT(( L"Done Send IPI (%x)\n", DestProcessor )) ; 

	// Wait for processor to consume message
	while( MessageBuffer[DestProcessor] != MESSAGE_NONE ) ; 
}



//
//  Name:
//		SendIPI	
//
//  Description:
//		Sends an IPI to a processor.
//
//  Arguments:
//		DestProcessor		- Destination processor (eid bits[15-8], id bits[7-0])
//		InterruptVector		- Interrupt vector
//
//  Returns:
//		None
//
void
SendIPI(
	UINT16 DestProcessor,
	UINT64 InterruptVector
	)
{
	UINT64	InterruptAddr ;

	// Retrieve IPI block address on initial call
	if( IPIBlock == 0 )
	{
		if( LibGetSalIpiBlock( &IPIBlock ) != EFI_SUCCESS )	
		{
			return ;
		}

	}

	// Destination processor (id bits[15-8], eid bits[7-0])
	DestProcessor = ( ( DestProcessor >> 8 ) & 0x00FF ) | ( ( DestProcessor << 8 ) & 0xFF00 ) ;

	// Compute address in the processor interrupt block and force non-cacheable
	InterruptAddr = ( UINT64 ) ( ( ( ( UINT64 ) IPIBlock ) +
		( ( ( UINT64 ) DestProcessor ) << 4 ) ) | BIT63 ) ;

	DPRINT(( L"InterruptVector = %lX\n", InterruptVector )) ;
	DPRINT(( L"InterruptAddr   = %lX\n", InterruptAddr )) ;


	// Send the IPI
	*( ( UINT64 * ) InterruptAddr ) = InterruptVector ;
}
