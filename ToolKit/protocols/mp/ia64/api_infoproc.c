/*
 * Copyright (c) 1999, 2000
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
#include <efilib.h>

#include "interface.h"
#include "init.h"
#include "proclist.h"

#include "debug.h"

//
//  Name:
//      GetEnabledProcessorsInfo
//
//  Description:
//		Returns information about the enabled processors on the platform.
//
//  Arguments:
//		This:			The EFI_MP_INTERFACE instance
//		Buffer:			Pointer to data buffer to return
//		BufferSize:		Size of buffer in MP_PROC_INFO records 
//
//  Returns:
//		EFI_STATUS
//
EFI_STATUS
GetEnabledProcessorsInfo(
    IN EFI_MP_INTERFACE		*This,
	IN OUT EFI_MP_PROC_INFO	*Buffer,
	IN OUT UINTN			*BufferSize
	)
{
	EFI_STATUS			RetValue = EFI_SUCCESS ;
	PROCREC				*ProcRec = NULL ;
	UINT8				Count = 0 ;
	EFI_MP_PROC_INFO	*MpProcInfo = Buffer ;

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

	// Count number of processors
	while( ( ProcRec = GetNextProcListRecord( ProcRec ) ) != NULL )
	{
		Count++ ;
	}

	// Required buffer size is zero 
	if( Count == 0 )
	{
		RetValue = EFI_MP_FAILURE ;
		goto Done ;
	}

	// Buffer size is less than the required buffer size 
	if( *BufferSize < Count )
	{
		RetValue = EFI_BUFFER_TOO_SMALL ;
		goto Done ;
	}

	// Buffer size is equal to or greater than the required buffer size
	while( ( ProcRec = GetNextProcListRecord( ProcRec ) ) != NULL )
	{
		MpProcInfo->ACPIProcessorID = ProcRec->ACPIProcessorID ;

		if( IS_BSP( ProcRec ) )
		{
			MpProcInfo->Flags = EFI_BSP_FLAG ;
		}
		else
		{
			MpProcInfo->Flags = EFI_AP_FLAG ;
		}
		MpProcInfo++ ;
	}

	Done:

	*BufferSize = Count ;

	return( RetValue ) ;
}
