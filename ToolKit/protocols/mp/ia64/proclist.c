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

#include <efi.h>
#include <efilib.h>
#include <stdlib.h>

#include "debug.h"

#include "interface.h"
#include "acpi.h"
#include "asmapi.h"
#include "proclist.h"

//
// Local function prototypes
//
static VOID
AddProcListRecord(
	PROCREC		*ProcRec	
	) ;

static VOID
RemoveProcListRecord(
	PROCREC		*ProcRec	
	) ;

static PROCREC
*AllocateProcRecord(
	UINT16	ACPIProcessorID,
	UINT8	LocalSAPIC_ID,
	UINT8	LocalSAPIC_EID
	) ;

static VOID
FreeProcRecord(
	PROCREC		*ProcRec	
	) ;


//
// Local variables
//
static	PROCREC		*HeadProcRec = NULL ;

//
//  Name:
// 		CreateProcListRecords	
//
//  Description:
//		Create list of processor records.
//
//  Arguments:
//		None
//
//  Returns:
//		ACPI_SUCCESS	- Successfully parsed ACPI table 
//		ACPI_NO_RSDPTR	- Unable to find RSD PTR structure
//		ACPI_SIGNATURE	- ACPI structure signature did not match expected value
//		ACPI_CHECKSUM	- ACPI structure did not checksum correctly
//		ACPI_NULL_PTR	- ACPI structure included unexpected NULL pointer
//		ACPI_PROCREC_ERR	- Error creating processor record from ACPI structure
//
UINT32
CreateProcListRecords(
	VOID
	)
{
	UINT32			RetValue ;
	UINT16			LocalID ;
	PROCREC			*ProcRec ;	

	// Get list of processors
	if( ( RetValue = ParseACPITable() ) != ACPI_SUCCESS )
	{
		goto Done ;
	}

	// Identify BSP
	LocalID = GetCurrentProcLocalID() ;
	
	// Find the BSP and mark it
	ProcRec = FindProcListRecord( LocalID, LID_KEY_TYPE ) ;
	ProcRec->Flags = ( ProcRec->Flags & ~PROCESSOR_MASK ) | PROCESSOR_BSP ;

#ifdef LIBMP_DEBUG
	DisplayProcListRecords( ) ;
#endif

	Done:
	return( RetValue ) ;
}


//
//  Name:
// 		DeleteProcListRecords	
//
//  Description:
//		Free list of processor records.
//
//  Arguments:
//		None
//
//  Returns:
//		None
//
VOID
DeleteProcListRecords(
	VOID
	)
{
	PROCREC		*ProcRec ;	

	while( HeadProcRec )
	{
		ProcRec = HeadProcRec ;
		RemoveProcListRecord( ProcRec ) ;
		FreeProcRecord( ProcRec ) ; 
	}
}


//
//  Name:
// 		NewProcListRecord
//
//  Description:
//		Creates a new processor record and adds it to the processor record list
//
//  Arguments:
//		ACPIProcessorID	- ACPI processor identifier
//		Type			- Type
//
//  Returns:
//		TRUE			- Record successfully created
//		FALSE			- Record was not successfully created
//
BOOLEAN
NewProcListRecord(
	UINT16		ACPIProcessorID,
	UINT8		LocalSAPIC_ID,
	UINT8		LocalSAPIC_EID
	)
{
	PROCREC		*ProcRec ;

	if( ( ProcRec = AllocateProcRecord( ACPIProcessorID, LocalSAPIC_ID, LocalSAPIC_EID ) ) == NULL )
	{
		return( FALSE ) ;	
	}

	AddProcListRecord( ProcRec ) ;
	return( TRUE ) ;
}


//
//  Name:
// 		GetNextProcListRecord
//
//  Description:
//		Get next processor list record	
//
//  Arguments:
//		ProcRec			- Record pointer or NULL
//
//  Returns:
//		Processor record pointer
//		
PROCREC
*GetNextProcListRecord(
	PROCREC		*ProcRec
	)
{
	if( !ProcRec )
	{
		return( HeadProcRec ) ;
	}
	else
	{
		return( ProcRec->Next ) ;
	}
}



//
//  Name:
// 		FindProcListRecord
//
//  Description:
//		Find matching processor record	
//
//  Arguments:
//		Key				- Search key
//		KeyType			- Search key type (either ACPI_KEY_TYPE or LID_KEY_TYPE)
//
//  Returns:
//		Processor record pointer
//		
PROCREC
*FindProcListRecord(
	UINT16		Key,
	UINT8		KeyType
	)
{
	PROCREC	*ProcRec = NULL ;

	while( ( ProcRec = GetNextProcListRecord( ProcRec ) ) != NULL )
	{
		if( ( KeyType == ACPI_KEY_TYPE ) && ( ProcRec->ACPIProcessorID == Key ) )
		{
			break ;
		}
		else if( ( KeyType == LID_KEY_TYPE ) && ( ProcRec->LocalID == Key ) )
		{
			break ;
		}
	}
	return( ProcRec ) ;
}



//
//  Name:
// 		AddProcListRecord
//
//  Description:
//		Adds processor record to processor record list
//
//  Arguments:
//		ProcRec	- Processor record pointer
//
//  Returns:
//		None	
//
static VOID
AddProcListRecord(
	PROCREC		*ProcRec	
	)
{
	// Check for valid record
	if( !ProcRec )
	{
		return ;
	}

	// Add to list
	ProcRec->Next = HeadProcRec ;
	HeadProcRec = ProcRec ;	
}


//
//  Name:
// 		RemoveProcListRecord
//
//  Description:
//		Removes processor record from processor record list
//
//  Arguments:
//		ProcRec	- Processor record pointer
//
//  Returns:
//		None	
//
static VOID
RemoveProcListRecord(
	PROCREC		*ProcRec	
	)
{
	PROCREC		*PrevRec ;

	// Check for valid record
	if( !ProcRec )
	{
		return ;
	}

	// Remove from list head
	if( HeadProcRec == ProcRec )
	{
		HeadProcRec = ProcRec->Next ;
	}

	// Remove from list middle
	else
	{
		for( PrevRec = HeadProcRec ; PrevRec->Next != NULL ; PrevRec = PrevRec->Next )
		{
			if( PrevRec->Next == ProcRec )
			{
				PrevRec->Next = ProcRec->Next ;
				break ;
			}
		}
	}
}


//
//  Name:
// 		AllocateProcRecord
//
//  Description:
//		Creates a processor record
//
//  Arguments:
//		ACPIProcessorID	- ACPI processor identifier
//		Type			- Type
//
//  Returns:
//		Processor record pointer
//
static PROCREC
*AllocateProcRecord(
	UINT16	ACPIProcessorID,
	UINT8	LocalSAPIC_ID,
	UINT8	LocalSAPIC_EID
	)
{
	PROCREC	*ProcRec ;

	if( ( ProcRec = ( PROCREC * ) calloc( 1, sizeof( PROCREC ) ) ) != NULL )	
	{
		ProcRec->ACPIProcessorID = ACPIProcessorID ;		
		ProcRec->LocalID = ( ( ( ( UINT16 ) LocalSAPIC_EID ) << 8 ) | LocalSAPIC_ID ) ;
		ProcRec->State = STOPPED ;
		ProcRec->Flags = PROCESSOR_AP ;
	}

	return( ProcRec ) ;
}


//
//  Name:
// 		FreeProcRecord
//
//  Description:
//		Frees a processor record
//
//  Arguments:
//		ProcRec	- Processor record pointer
//
//  Returns:
//		None
//
static VOID
FreeProcRecord(
	PROCREC		*ProcRec	
	)
{
	if( ProcRec != NULL )
	{
		free( ProcRec ) ;
	}
}



#ifdef LIBMP_DEBUG
//
//  Name:
// 		DisplayProcListRecords	
//
//  Description:
//		Displays all processor list records	
//
//  Arguments:
//		None	
//
//  Returns:
//		None
//
VOID
DisplayProcListRecords(
	VOID
	)
{
	PROCREC	*ProcRec = NULL ;

	while( ( ProcRec = GetNextProcListRecord( ProcRec ) ) != NULL )
	{
		Print( L"ACPIID = %x, LocalID = %x, ", ProcRec->ACPIProcessorID, ProcRec->LocalID ) ;
		if( ( ProcRec->Flags & PROCESSOR_MASK ) == PROCESSOR_BSP )
		{
			Print( L"BSP" ) ;
		}
		else
		{
			Print( L"AP, " ) ;
			if( ProcRec->State == STOPPED )
			{
				Print( L"Stopped" ) ;
			}
			else
			{
				Print( L"Started" ) ;
			}
		}
		Print( L"\n" ) ;
	}
}
#endif