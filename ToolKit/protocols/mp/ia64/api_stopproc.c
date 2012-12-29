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

#include "asmapi.h"
#include "init.h"
#include "interface.h"
#include "message.h"
#include "proclist.h"
#include "util.h"

#include "debug.h"



//
//  Name:
//     	StopProcessor 
//
//  Description:
//		Stop AP.
//
//  Arguments:
//		This:				The EFI_MP_INTERFACE instance
//		ACPIProcessorID:	ACPI processor identifier
//
//  Returns:
//		EFI_STATUS
//
EFI_STATUS
StopProcessor(
    IN  EFI_MP_INTERFACE	*This,
	IN	UINT16				ACPIProcessorID
	)
{
	UINT16		LocalID ;
	PROCREC		*DstProcRec,
				*SrcProcRec ; 
	EFI_STATUS	RetValue = EFI_SUCCESS ;

	if( InitializedMpProtocol == FALSE )
	{
		RetValue = EFI_MP_FAILURE ;
		goto Done ;
	}

	if( This == NULL )
	{
		RetValue = EFI_INVALID_PARAMETER ;
		goto Done ;
	}

	// Find processor record for destination processor
	if( ( DstProcRec = FindProcListRecord( ACPIProcessorID, ACPI_KEY_TYPE ) ) == NULL )
	{
		RetValue = EFI_MP_ACPI_ID_FAILURE ;
		goto Done ;
	}

	// Destination processor must not be BSP
	if( IS_BSP( DstProcRec ) )
	{
		RetValue = EFI_MP_ACPI_ID_FAILURE ;
		goto Done ;
	}

	// Find processor record for source processor
	LocalID = GetCurrentProcLocalID() ;
	if( ( SrcProcRec = FindProcListRecord( LocalID, LID_KEY_TYPE ) ) == NULL )
	{
		RetValue = EFI_MP_ACPI_ID_FAILURE ;
		goto Done ;
	}

	// Source processor must be BSP
	if( !IS_BSP( SrcProcRec ) )
	{
		RetValue = EFI_MP_ACPI_ID_FAILURE ;
		goto Done ;
	}

	// AP must be STOPPED
	if( DstProcRec->State != STARTED )
	{
		RetValue = EFI_MP_AP_NOT_START ;
		goto Done ;
	}

	// Send message to AP
	SendIPIMessage( MESSAGE_STOP, DstProcRec->LocalID ) ;

	// Update state of AP
	DstProcRec->State = STOPPED ; 

	Done:

	return( RetValue ) ;
}