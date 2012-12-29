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
#include <string.h>

#include "acpi.h"
#include "proclist.h"

#include "debug.h"

//
// Function prototypes
//
static UINT32
ParseRSDTTable(
	RSDT	*RSDTable
	) ;

static UINT32
ParseSPICTable(
	SPIC	*SPICTable
	) ;

static UINT32
ParseProcLocalSPICTable(
	SAPIC_PROC_LOCAL *ProcLocalSPICTable
	) ;

static UINT8
ComputeChecksum(
	UINT8	*Buffer, 
	UINT32	Length
	) ;

//
//  Name:
// 		ParseACPITable
//
//  Description:
//		Parse ACPI description tables.
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
ParseACPITable(
	VOID
	)
{
	UINT32		RetValue ;
	RSD_PTR		*RSDPtr;	
	EFI_STATUS  Status;


	DPRINT(( L"Calling ParseACPITable\n" )) ;

	// Locate Root System Descriptor Pointer 
	Status = LibGetSystemConfigurationTable( &AcpiTableGuid, ( void ** ) &RSDPtr ) ;
	if (Status == EFI_SUCCESS)
	{

		// Check for RSD Pointer
		if( RSDPtr == NULL )
		{
			RetValue = ACPI_NO_RSDPTR ;
			goto Done ;
		}

		// Check RSD Pointer signature
		if( strncmp( RSDPtr->Signature, "RSD PTR ", 8 ) )
		{
			RetValue = ACPI_SIGNATURE ;
			goto Done ;
		}

		// Check RSD Pointer checksum
		if( ComputeChecksum( ( UINT8 * ) RSDPtr, sizeof( RSD_PTR ) ) )
		{
			RetValue = ACPI_CHECKSUM ;
			goto Done ;
		}

		RetValue = ParseRSDTTable( ( RSDT * ) RSDPtr->RSDT ) ;
	}
	else {
		//Read the data into LidList NO &
		Status = ParseAcpi2_0Table();

		if (EFI_ERROR(Status)) {
			DEBUG((D_INIT,"Out of here w/ Status code = 0x%X\n", Status));
			RetValue = ACPI_2_ERR;
			goto Done;
		}
	}
	Done:
	DPRINT(( L"ParseACPITable ( %x )\n", RetValue )) ;
	return( RetValue ) ;
}



//
//  Name:
// 		ParseRSDTTable
//
//  Description:
//		Parse RSDT description tables.
//
//  Arguments:
//		RSDTTable			- Pointer to RSDT table	
//
//  Returns:
//		ACPI_SUCCESS		- Successfully parsed RSDT structure
//		ACPI_SIGNATURE		- ACPI structure signature did not match expected value
//		ACPI_CHECKSUM		- ACPI structure did not checksum correctly
//		ACPI_NULL_PTR		- ACPI structure included unexpected NULL pointer
//		ACPI_PROCREC_ERR	- Error creating processor record from ACPI structure
//
static UINT32
ParseRSDTTable(
	RSDT	*RSDTTable
	)
{
	UINT32		RetValue,
				NumEntries,
				Signature ;
	UINTN		*Entry ;
	DESC_HEADER *DescHeader ;

	DPRINT(( L"Calling ParseRSDTTable\n" )) ;

	// Check for NULL pointer
	if( RSDTTable == NULL )
	{
		RetValue = ACPI_NULL_PTR ;
		goto Done ;
	}

	// Check RSD Table signature
	if(	( ( ACPI_SIG * ) RSDTTable->Header.Signature )->HexSignature != ACPI_RSDT_SIG )
	{
		RetValue = ACPI_SIGNATURE ;
		goto Done ;
	}

	// Check RSD Table checksum
	if( ComputeChecksum( ( UINT8 * ) RSDTTable, RSDTTable->Header.Length ) )
	{
		RetValue = ACPI_CHECKSUM ;
		goto Done ;
	}

	// Compute number of pointers to other description tables 
	NumEntries = ( RSDTTable->Header.Length - sizeof( DESC_HEADER ) ) / sizeof( void * ) ;
	Entry = ( UINTN * ) &( RSDTTable->Entry ) ; 
	RetValue = ACPI_SUCCESS ;

	DPRINT(( L"ParseRSDTTable:  Pointers = %d\n", NumEntries)) ;

	// Process pointers to other description tables
	while( ( NumEntries > 0 ) && ( RetValue == ACPI_SUCCESS ) )
	{

		DescHeader = ( DESC_HEADER * ) *Entry ;
		Signature = ( ( ACPI_SIG * ) DescHeader->Signature )->HexSignature ; 

		// Process description table based on signature
		switch( Signature )
		{
			case ACPI_SPIC_SIG:
			{
//				DPRINT(( L"ParseRSDTTable:  Descriptor Table( %x )\n", Signature )) ;
				RetValue = ParseSPICTable( ( SPIC * ) DescHeader ) ;
				break ;
			}

			case ACPI_DSDT_SIG:
			case ACPI_FACP_SIG:
			case ACPI_FACS_SIG:
			case ACPI_PSDT_SIG:
			case ACPI_RSDT_SIG:
			case ACPI_SSDT_SIG:
			case ACPI_SBST_SIG:
			{
//				DPRINT(( L"ParseRSDTTable:  Descriptor Table( %x )\n", Signature )) ;
				break ;
			}

			default:
			{
//				DPRINT(( L"ParseRSDTTable:  Unknown Descriptor Table\n", Signature )) ;
				break ;
			}
		}

		NumEntries-- ;
		Entry++ ;
	}

	Done:
	DPRINT(( L"ParseRSDTTable ( %x )\n", RetValue )) ;
	return( RetValue ) ;
}



//
//  Name:
// 		ParseSPICTable
//
//  Description:
//		Parse SAPIC description tables.
//
//  Arguments:
//		SPICTable			- Pointer to SPIC table	
//
//  Returns:
//		ACPI_SUCCESS		- Successfully parsed SAPIC structure
//		ACPI_SIGNATURE		- ACPI structure signature did not match expected value
//		ACPI_CHECKSUM		- ACPI structure did not checksum correctly
//		ACPI_NULL_PTR		- ACPI structure included unexpected NULL pointer
//		ACPI_PROCREC_ERR	- Error creating processor record from ACPI structure
//
//  Notes:
//		Extra robustness is included in case the routine is called in a different context
//		than currently programmed.  Plus, the cost is pretty minimal.
//
static UINT32
ParseSPICTable(
	SPIC	*SPICTable
	)
{
	UINT32			RetValue,
					SAPICStructLen ;
	SAPIC_HEADER	*SAPICHeader ;

	DPRINT(( L"Calling ParseSPICTable\n" )) ;

	// Check for NULL pointer
	if( SPICTable == NULL )
	{
		RetValue = ACPI_NULL_PTR ;
		goto Done ;
	}

	// Check SAPIC Table signature
	if(	( ( ACPI_SIG * ) SPICTable->Header.Signature )->HexSignature != ACPI_SPIC_SIG )
	{
		RetValue = ACPI_SIGNATURE ;
		goto Done ;
	}

	// Check SAPIC Table checksum
	if( ComputeChecksum( ( UINT8 * ) SPICTable, SPICTable->Header.Length ) )
	{
		RetValue = ACPI_CHECKSUM ;
		goto Done ;
	}

	// Locate SAPIC structures
	SAPICHeader = ( SAPIC_HEADER * ) ( ( UINT8 * ) SPICTable + sizeof( SPIC ) ) ;
	SAPICStructLen = SPICTable->Header.Length - sizeof( SPIC ) ;
	RetValue = ACPI_SUCCESS ;

	// Process SAPIC structures
	while( ( SAPICStructLen > 0 ) && ( RetValue == ACPI_SUCCESS ) )
	{
		switch( SAPICHeader->Type )
		{
			case ACPI_PROC_LOCAL_TYPE:
			{
//				DPRINT(( L"ParseSPICTable:  SAPICHeader = %x SAPICHeader.Type = %x\n", SAPICHeader, SAPICHeader->Type )) ;
				RetValue = ParseProcLocalSPICTable( ( SAPIC_PROC_LOCAL * ) SAPICHeader ) ;
				break ;
			}

			case ACPI_IO_SAPIC_TYPE:
			case ACPI_INT_OVERRIDE_TYPE:
			case ACPI_INT_SOURCE_TYPE:
			{
//				DPRINT(( L"ParseSPICTable:  SAPICHeader.Type = %x\n", SAPICHeader->Type )) ;
				break ;
			}

			default:
			{
//				DPRINT(( L"ParseSPICTable:  SAPICHeader.Type = %x\n", SAPICHeader->Type )) ;
				break ;
			}
		}

		// Next SAPIC structure
		SAPICStructLen -= SAPICHeader->Length ;

		// BIOS workaround
		if( SAPICHeader->Length == 0 )
		{
			break ;
		}

		SAPICHeader = ( SAPIC_HEADER * ) ( ( ( UINT8 * ) SAPICHeader ) + SAPICHeader->Length ) ; 
	}

	Done:
	DPRINT(( L"ParseSPICTable ( %x )\n", RetValue )) ;
	return( RetValue ) ;
}



//
//  Name:
// 		ParseProcLocalSPICTable
//
//  Description:
//		Parse processor local SAPIC description tables.
//
//  Arguments:
//		ProcLocalSPICTable	- Pointer to processor local SPIC table	
//
//  Returns:
//		ACPI_SUCCESS		- Successfully parsed SAPIC structure
//		ACPI_SIGNATURE		- ACPI structure signature did not match expected value
//		ACPI_NULL_PTR		- ACPI structure included unexpected NULL pointer
//		ACPI_PROCREC_ERR	- Error creating processor record from ACPI structure
//
//  Notes:
//		Extra robustness is included in case the routine is called in a different context
//		than currently programmed.  Plus, the cost is pretty minimal.
//
static UINT32
ParseProcLocalSPICTable(
	SAPIC_PROC_LOCAL *ProcLocalSPICTable
	)
{
	UINT32			RetValue = ACPI_SUCCESS ;

	DPRINT(( L"Calling ParseProcLocalSPICTable\n" )) ;

	// Check for NULL pointer
	if( ProcLocalSPICTable == NULL )
	{
		RetValue = ACPI_NULL_PTR ;
		goto Done ;
	}

	// Check for correct type
	if( ProcLocalSPICTable->Header.Type != ACPI_PROC_LOCAL_TYPE )
	{
		RetValue = ACPI_SIGNATURE ;
		goto Done ;
	}

	if( ProcLocalSPICTable->Flags & ( ACPI_SAPIC_PROC_LOCAL_PRES | ACPI_SAPIC_PROC_LOCAL_EN ) )
	{
		if( NewProcListRecord( ProcLocalSPICTable->ACPIProcessorID,
			ProcLocalSPICTable->LocalSAPIC_ID, ProcLocalSPICTable->LocalSAPIC_EID ) == FALSE )
		{
			RetValue = ACPI_PROCREC_ERR ;
			goto Done ;
		}
	}

	Done:
	DPRINT(( L"ParseProcLocalSPICTable ( %x )\n", RetValue )) ;
	return( RetValue ) ;
}



//
//  Name:
// 		ComputeChecksum	
//
//  Description:
//		Compute byte checksum on buffer of given length.
//
//  Arguments:
//		Buffer		- Pointer to buffer to compute checksum
//		Length		- Number of bytes to checksum
//
//  Returns:
//		Checksum	- Checksum of buffer
//
static UINT8
ComputeChecksum(
	UINT8	*Buffer,
	UINT32	Length
	)
{
	UINT8	Checksum = 0 ;

	for( ; Length != 0 && Buffer ; Length--, Buffer++ )
	{
		Checksum += *Buffer ;
	}

	return( Checksum ) ;
}


//
// Parse ACPI 2.0 Table
//
#pragma warning ( disable : 4133 )
EFI_STATUS
ParseAcpi2_0Table ()
{
    EFI_STATUS                  Status;
    ACPI_RSDP20                 *AcpiRsdp;
    ACPI_DESCRITPTION_HEADER    *Header;
    ACPI_DESCRITPTION_HEADER_64 *Header64;
    UINT32                      Length;
    UINT32                      Index;
    //UINT64                      *Entry;
    UINT8                       *SpicEnd;
    UINT8                       *SpicCurrent;
    UINTN                       SpicLength;
    //ACPI_LOCAL_SAPIC_ENTRY      *SpicLocal;
    ACPI_20_LOCAL_SAPIC_ENTRY   *SpicLocal20;


    EFI_GUID LocalAcpiTableGuid = {0x8868E871, 0xE4F1, 0x11D3, 0xBC, 0x22, 0x0, 0x80, 0xC7, 0x3C, 0x88, 0x81};

    Status = LibGetSystemConfigurationTable(&LocalAcpiTableGuid, (VOID*)&AcpiRsdp);

    if (EFI_ERROR(Status)) {
        DEBUG((D_INIT,"Didn't find the table\n"));
        return EFI_NOT_FOUND;
    }

        if (AcpiRsdp->Revision == ACPI_RSDP_REV_20) {
          DEBUG((D_INIT,"ACPI rev = 2.0\n"));
        }  else {
           Print (L"\nACPI RSDP revision bad %d\n", AcpiRsdp->Revision);
           DEBUG((D_INIT,"ACPI RSDP revision bad %d\n", AcpiRsdp->Revision));
           Status = EFI_NOT_FOUND;
          goto Done;
		}

    if (AcpiRsdp->SignatureLow != ACPI_RSDP_SIGNATURE_LOW || 
        AcpiRsdp->SignatureHigh != ACPI_RSDP_SIGNATURE_HIGH) {
        Print (L"\nACPI RSDP signature bad %x %x\n", AcpiRsdp->SignatureLow, ACPI_RSDP_SIGNATURE_LOW);
        DEBUG((D_INIT,"\nACPI RSDP signature bad %x %x\n", AcpiRsdp->SignatureLow, ACPI_RSDP_SIGNATURE_LOW));
        Status = EFI_NOT_FOUND;
        goto Done;
    }

    //
    // Parse the 2.0 ACPI Tables
    //


	DEBUG ((D_INIT, "About to parse the tables\n"));
    //
    // Save max buffer size and return actual count
    //
 
    Header64 = (ACPI_DESCRITPTION_HEADER_64 *)AcpiRsdp->XsdtAddress;

    Length = (Header64->Length - sizeof(ACPI_DESCRITPTION_HEADER))/sizeof(VOID*);

    for (Index = 0; Index < Length; Index++) {

        Header = (ACPI_DESCRITPTION_HEADER *)Header64->Entry[Index];

        if ((Header->Signature == ACPI_20_APIC_SIGNATURE)) {
            SpicEnd = ((UINT8 *)Header) + Header->Length;  
            SpicCurrent = ((UINT8 *)Header) + sizeof(ACPI_20_MAPIC_DT);

            while (SpicCurrent < SpicEnd) {
                if ((*SpicCurrent == ACPI_20_LOCAL_SAPIC)) {
                    SpicLocal20 = (ACPI_20_LOCAL_SAPIC_ENTRY *)SpicCurrent;                

                    if (SpicLocal20->Flags == 1){
                        //
                        //  Put this call into the new ACPI cracking code from ipi.h/c.  When found local SAPIC, make 
                        //  this call to register the processor LID.
                        //          
                            
 					  DEBUG((D_INIT, "Parsing table via NewProcListRecord\n"));
                          NewProcListRecord(  SpicLocal20->AcpiProcessorId,
                                              SpicLocal20->SapicId, 
                                              SpicLocal20->SapicEid);
                    }
                }

                //
                // Length is in byte two of the structure.
                //
                SpicLength = *(SpicCurrent+1);
                if (SpicLength == 0) {
                    Print (L"Print Type %x is bad length == 0.\n", *SpicCurrent);
                    DEBUG((D_INIT, L"Print Type %x is bad length == 0.\n", *SpicCurrent));
                    break;
                }
                SpicCurrent += SpicLength;
            }
        }
    }
Done:
    DEBUG((D_INIT,"Out of here w/ Status code = 0x%X\n", Status));
    return Status;
}
#pragma warning ( default : 4133 )
