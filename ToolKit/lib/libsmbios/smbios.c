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

 /*
Module Name:

    smbios.c
    
Abstract:
	implementation file for EFI SMBIOS
--*/

#include <efi.h>
#include <efilib.h>
#include <smbios.h>

#define GetValue( src, dst, length ) MEMCPY( dst, src, length )
#define FREE( buf ) gBS->FreePool( (VOID*)buf );

/*	Globals for application
*/

/*the physical address of the SMBIOS Table*/
static UINT8 *SMBIOSTable;

/*the boot services table*/
static EFI_BOOT_SERVICES	*gBS;

/* helper functions */
static VOID*
MALLOC( UINTN len )
{
	VOID	*buf;
	if ( EFI_ERROR( gBS->AllocatePool( EfiLoaderData, len, &buf ) ) )
		return NULL;
	return (buf);
}
static VOID
MEMCPY( VOID *dst, VOID *src, UINTN len )
{
	while( len-- ) {
		*(UINT8*)dst = *(UINT8*)src;
		(UINT8*)dst++;
		(UINT8*)src++;
  }
}

/*
Function  SMBIOS_Initialize()

  Parameters
	ImageHandle	Initialization parameter for Lib C
	SystemTable	Standard EFI SystemTable for this application

  Description
	This function Initializes EFI SMBIOS interfaces.
  
  Status Codes Returned
	EFI_SUCCESS					Successfully initialize SMBIOS interface 
	EFI_SMBIOSERR_FAILURE		Failed to initialize SMBIOS interface 
	EFI_SMBIOSERR_UNSUPPORTED	System does not support SMBIOS feature
*/

EFI_STATUS
SMBIOS_Initialize (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
	)
{
	EFI_GUID SMBIOSGuid = SMBIOS_TABLE_GUID;
	EFI_STATUS ret;

/* save global boot services tabel pointer */
	gBS = SystemTable->BootServices;

/*initialize EFI library to be able to use functions in libefi*/
	InitializeLib( ImageHandle, SystemTable );

/* query the system table to get the SMBIOS table addr */
	ret = LibGetSystemConfigurationTable( &SMBIOSGuid, (VOID**)&SMBIOSTable );
	
	if ( (ret == EFI_NOT_FOUND) || (SMBIOSTable == NULL) )
		ret = EFI_SMBIOSERR_UNSUPPORTED	;	/*system does not support SMBIOS*/
	else if (SMBIOSTable == NULL)
		ret = EFI_SMBIOSERR_FAILURE	;	/*failed to get SMBIOS table*/

	return ret;
}

/*
Function	SMBIOS_GetTableEntryPoint ()

  Parameters
	pSMBIOSInfo	Point to SMBIOS information table pointer
	from SMBIOS Structure Entry Point.

  Description
	This function gets the SMBIOS information from the 
	Structure Entry Point.
	
	User shall not allocate memory for the structure.  
	User is resposible for freeing the memoery al-located
	by *pSMBIOSInfo.

  Status Codes Returned
	EFI_SUCCESS						Successfully get the SMBIOS information 
	            					from the Struc-ture Entry Point.
	EFI_SMBIOSERR_STRUCT_NOT_FOUND	SMBIOS TableEntryPoint
	            					structure is not found
*/

EFI_STATUS
SMBIOS_GetTableEntryPoint(
	IN OUT SMBIOSTableEntryPoint **pSMBIOSInfo
	)
{
	UINT16 structLength;	/* structLength of EPS*/
	SMBIOSTableEntryPoint *tmpSMBIOSInfo;
	SMBIOSTableEntryPoint *tmp;

	/* there should be no alignment issue of SMBIOS Entry Point*/
	tmpSMBIOSInfo = (SMBIOSTableEntryPoint *) SMBIOSTable;

	/*get the EPS length*/
	structLength = tmpSMBIOSInfo->EPLength;

	/*allocate memory for EPS*/
	tmp = (SMBIOSTableEntryPoint *) MALLOC ((UINTN)structLength);

	/*copy EPS from system buffer to memory structure*/
	MEMCPY (tmp, tmpSMBIOSInfo, structLength);

	(*pSMBIOSInfo) = tmp;

	/*have a double-check before returning*/
	if ( (tmp->AnchorString[0] == '_') &&
		 (tmp->AnchorString[1] == 'S') &&
		 (tmp->AnchorString[2] == 'M') &&
		 (tmp->AnchorString[3] == '_') )
		 /*check for thE SMBIOS Anchor String "_SM_" */
		return EFI_SUCCESS;
	else
		return EFI_SMBIOSERR_STRUCT_NOT_FOUND;
}

#pragma warning ( disable : 4312 )
/*
Function	SMBIOS_GetStructure()
  Parameters
	Type	SMBIOS Structure type
	Handle	Handle of specified SMBIOS structure
	pStructureBuffer	Pointer to the pointer of 
	        retrieved buffer of specified SMBIOS structure
	Length	actual length of the specified SMBIOS structure
	        buffer allocated
	Key		Opaque data used by the library to optimize
			search

  Description
	This function gets the corresponding SMBIOS structure 
	buffer according to the specified type and handle.

    The following table describes the behavior of this function
    for various combinations of Type, Handle, and Key values:

      Type        Handle      Key       Behavior

    != 0xFFFF   != 0xFFFF   ignored   Returns the structure
                                      with the Type and Handle
                                      specified if it exists.

    != 0xFFFF   == 0xFFFF   == 0xFFFF Returns the first structure
                                      with the Type specified if
                                      it exists.

    != 0xFFFF   == 0xFFFF   != 0xFFFF Returns the next structure
                                      with the Type specified if
                                      it exists.

    == 0xFFFF   != 0xFFFF   ignored   Returns the structure
                                      with the Handle specified
                                      if it exists.

    == 0xFFFF   == 0xFFFF   == 0xFFFF Returns the first structure.

    == 0xFFFF   == 0xFFFF   != 0xFFFF Returns the next structure if
                                      it exists.

    As noted in the parameter descriptions, the Key is opaque 
    data used by the library to optimize its searches. The 
    only thing callers should do with this parameter is set 
    it to 0xFFFF to indicate the start of a search. Where the
    table specifies the Key value != 0xFFFF it means that it
    is using the value returned by a previous successful call.

    At any time if this	function call returns with a status 
    code of EFI_SMBIOSERR_STRUCT_NOT_FOUND, it means there 
    are no more structures of the specified type. The user 
    shall not call the function again with the returned *Key.

	User shall not allocate memory for the structure.  
	User is responsible for freeing the memory allocated
	by *pStructureBuffer.

	If the type is unknown by the library, it returns
	EFI_SMBIOSERR_TYPE_UNKNOWN and the raw data
	will be put in the StructureBuffer.

    NOTE: The StructLength field in the SMBIOSHeader that
    is part of the returned structure must not be used to
    indicate the size of the returned structure. That field
    indicates the length of the raw structure in its packed, 
    unaligned form. The value returned in the function's Length
    argument reflects the true size of the returned structure.

  Status Codes Returned
	EFI_SUCCESS						Successfully got the specified SMBIOS 
									structure buffer
	EFI_SMBIOSERR_STRUCT_NOT_FOUND	The specified structure was
									not found
	EFI_SMBIOSERR_TYPE_UNKNOWN		The SMBIOS structure was not
									converted to a structure type known by the
									library
*/
EFI_STATUS
SMBIOS_GetStructure (
	IN		UINT16		Type,
	IN		UINT16		Handle,
	IN OUT	VOID		**pStructureBuffer,
	IN OUT	UINT16		*Length,
	IN OUT	UINT16		*pKey
	)
{
	EFI_STATUS				ret;
	SMBIOSTableEntryPoint	*SMBIOSInfo;
	UINT16					structureCount; 

	/*the address of SMBIOS structure Table*/
	UINT8					*tableAddress;
	 /*the total offset startinf from the address of SMBIOS structure Table*/
	UINT16					offset;
	/*checkHandle: =TRUE: we need to check the specified handle
				   =FALSE: don't check the specified handle
	*/
	BOOLEAN					checkHandle = TRUE;
    /*checkType: =TRUE: we need to check the specified type
                 =FALSE: don't check the specified type
    */
    BOOLEAN                 checkType = TRUE;

	/*	found	= TRUE;  the structure is found.
				=FALSE:  not found
	*/
	BOOLEAN					found = FALSE;
	UINT16					i, lastHandle = 0xFFFF;
	UINT16					lastType, lastLength;
	UINT16					tableLength;  /*total length of SMBIOS table*/

    /*  0xFFFF means we will find any structure with the specified handle */
    checkType = (Type != 0xFFFF);

	/*  0xFFFF means we will find all structures of the specified type*/
	if (Handle == 0xFFFF)
		checkHandle = FALSE;
	
	/*now get the address of SMBIOS structure Table*/
	ret = SMBIOS_GetTableEntryPoint ( &SMBIOSInfo ) ;
	if (ret != EFI_SUCCESS)
		return ret;
	tableAddress = (UINT8 *) (SMBIOSInfo->StructTableAddr);

	/*now set the offset to the value user passes in.  
		if checkHandle=false we will set his parameter to 0 */
	offset = *pKey;
	if ( !checkHandle && (offset != 0xFFFF) )
		tableAddress += offset;  /*get the new starting addr for search*/
	else 		/*in this case we don't need to move the table address*/
		offset = 0;  /*offset=0 means we will search from the start*/

	if ( checkType && CheckEnumType ((UINT8) Type) == FALSE ) {	/*Type is in enumStructureType?*/
        ret = EFI_SMBIOSERR_TYPE_UNKNOWN ;
	}

    /*pull information from SMBIOS Entry point then free the memory*/
	structureCount = SMBIOSInfo->NumStruct;		/*MAX_STRUCTURES*/
	tableLength = SMBIOSInfo->StructTableLen;		/*total length of structure table*/
	if (SMBIOSInfo)
		FREE (SMBIOSInfo);

	for (i = 0; (i < structureCount) && (offset < tableLength); i++ ) {
		/*first get the type and handle of current structure*/
		lastType = *tableAddress;
		GetValue( &tableAddress[2], &lastHandle, 2 );

		if ( ( !checkType   || (lastType == Type) ) && 
             ( !checkHandle || (lastHandle == Handle) ) ) {
			/* the specified structure is found here*/

			/* now call this function to get the total length of this structure */
			NextTableAddress (tableAddress, &lastLength);
			if (ret == EFI_SMBIOSERR_TYPE_UNKNOWN ){
				/*if the type is an unkown one we copy the 
						raw structure data back. */
				*Length = lastLength;  /*in this case *length returns the raw data length*/
				/* now allocate memory for the raw data buffer*/
				*pStructureBuffer = MALLOC (lastLength);
				if (*pStructureBuffer == NULL)
					return EFI_BUFFER_TOO_SMALL;
				/* now copy the raw structure data back. */
				MEMCPY (*pStructureBuffer, tableAddress, lastLength);
			} 
			else {
				/*otherwise we copy the struture members from system buffer one by one */
				*Length = CopyStruct ((UINT8) lastType, pStructureBuffer, tableAddress);
				ret = EFI_SUCCESS;
			}

			found = TRUE;
			/* now advance to the starting addr of next structure */
			tableAddress += lastLength;
			/*adjust the offset accordingly*/
			offset += lastLength;
			break;
		} /* Found first structure of the requested type */
		else { /* Increment address to start of next structure */
			NextTableAddress (tableAddress, &lastLength);
			tableAddress += lastLength;
			/*adjust the offset accordingly*/
			offset += lastLength;
		}
	} /* END while-loop looking for structure type */

	if ( !found ){	/*not found!*/
		ret = EFI_SMBIOSERR_STRUCT_NOT_FOUND ;
		offset = 0xFFFF;
	}
	if ( pKey != NULL )
		*pKey = offset;  /*return the acutal offset to *pKey*/
	return ret;
}

/*
Function	SMBIOS_GetRawStructure()

  Parameters
	Type	SMBIOS Structure type
	Handle	Handle of specified SMBIOS structure
	pRawBuffer	Pointer to the pointer of retrieved 
				raw buffer of specified SMBIOS structure
	Length	Actual length of the specified SMBIOS structure 
			buffer allo-cated
	Key	Opaque data used by the library to optimize search

  Description
	This function gets the corresponding SMBIOS structure
	raw buffer according to specified type and handle.

    The following table describes the behavior of this function
    for various combinations of Type, Handle, and Key values:

      Type        Handle      Key       Behavior

    != 0xFFFF   != 0xFFFF   ignored   Returns the structure
                                      with the Type and Handle
                                      specified if it exists.

    != 0xFFFF   == 0xFFFF   == 0xFFFF Returns the first structure
                                      with the Type specified if
                                      it exists.

    != 0xFFFF   == 0xFFFF   != 0xFFFF Returns the next structure
                                      with the Type specified if
                                      it exists.

    == 0xFFFF   != 0xFFFF   ignored   Returns the structure
                                      with the Handle specified
                                      if it exists.

    == 0xFFFF   == 0xFFFF   == 0xFFFF Returns the first structure.

    == 0xFFFF   == 0xFFFF   != 0xFFFF Returns the next structure if
                                      it exists.

    As noted in the parameter descriptions, the Key is opaque 
    data used by the library to optimize its searches. The 
    only thing callers should do with this parameter is set 
    it to 0xFFFF to indicate the start of a search. Where the
    table specifies the Key value != 0xFFFF it means that it
    is using the value returned by a previous successful call.

    At any time if this	function call returns with a status 
    code of EFI_SMBIOSERR_STRUCT_NOT_FOUND, it means there 
    are no more structures of the specified type. The user 
    shall not call the function again with the returned *Key.

	User shall not allocate memory for the structure.  User is 
	responsible for freeing the memory allocated by *pRawBuffer.

  Status Codes Returned
	EFI_SUCCESS						Successfully got the specified SMBIOS raw 
									structure buffer
	EFI_SMBIOSERR_STRUCT_NOT_FOUND	The specified structure was not found
*/
EFI_STATUS
SMBIOS_GetRawStructure (
	IN		UINT16		Type,
	IN		UINT16		Handle,
	IN OUT	VOID		**pRawBuffer,
	IN OUT	UINT16		*Length,
	IN OUT	UINT16		*pKey
	)
{
	EFI_STATUS				ret;
	SMBIOSTableEntryPoint	*SMBIOSInfo;
	UINT16					structureCount; 
	
	/*the address of SMBIOS structure Table*/
	UINT8					*tableAddress;
	 /*the total offset startinf from the address of SMBIOS structure Table*/
	UINT16					offset;
	/*checkHandle: =TRUE: we need to check the specified handle
				   =FALSE: don't check the specified handle*/
	BOOLEAN					checkHandle = TRUE;
	/*checkType: =TRUE: we need to check the specified type
				 =FALSE: don't check the specified type*/
	BOOLEAN					checkType = TRUE;
	/*the total offset startinf from the address of SMBIOS structure Table*/
	UINT16					i=0, lastHandle = 0xFFFF;
	UINT16					lastType, lastLength = 0;
	UINT16					tableLength;  /*total length of SMBIOS table*/

    /* 0xFFFF means we find all the structures or by the handle */
    checkType = (Type != 0xFFFF);

	/*  0xFFFF means we will find all structure of specified type*/
	if (Handle == 0xFFFF)
		checkHandle = FALSE;
	
	/*now get the address of SMBIOS structure Table*/
	ret = SMBIOS_GetTableEntryPoint ( &SMBIOSInfo ) ;
	if (ret != EFI_SUCCESS)
		return ret;
	tableAddress = (UINT8 *) (SMBIOSInfo->StructTableAddr);

	/*now set the offset to the value user passes in.  
		if checkHandle=false we will set his parameter to 0 */
	offset = *pKey;
	if ( !checkHandle && (offset != 0xFFFF) )
		tableAddress += offset;  /*get the new starting addr for search*/
	else 		/*in this case we don't need to move the table address*/
		offset = 0;  /*offset=0 means we will search from the start*/

/*pull information from SMBIOS Entry point then free the memory*/
	structureCount = SMBIOSInfo->NumStruct;		/*MAX_STRUCTURES*/
	tableLength = SMBIOSInfo->StructTableLen;		/*total length of structure table*/
	if (SMBIOSInfo)
		FREE (SMBIOSInfo);

	/*initially we assume the structure is not found*/
	ret = EFI_SMBIOSERR_STRUCT_NOT_FOUND ;
	while ( (i < structureCount) && (offset < tableLength) ) {
		i++;

		/*first get the type and handle of current structure*/
		lastType = *tableAddress;
		GetValue( &tableAddress[2], &lastHandle, 2 );
		
		if ( ( !checkType   || (lastType == Type) ) && 
             ( !checkHandle || (lastHandle == Handle) ) ) {
			/* the specified structure is found here*/
		
			/* now call this function to get the total length of this structure */
			NextTableAddress (tableAddress, &lastLength);
			*Length = lastLength;  /*  *length returns the raw data length*/
			
			/* now allocate memory for the raw data buffer*/
			*pRawBuffer = MALLOC (lastLength);
			if (*pRawBuffer == NULL)
				return EFI_BUFFER_TOO_SMALL;
			/* now copy the raw structure data back. */
			MEMCPY (*pRawBuffer, tableAddress, lastLength);

			/* now advance to the starting addr of next structure */
			tableAddress += lastLength;
			/*adjust the offset accordingly*/
			offset += lastLength;
			ret = EFI_SUCCESS;
			break;
		} /* Found first structure of the requested type */
		else { /* Increment address to start of next structure */
			/* now call this function to get the total length of this structure */
			NextTableAddress (tableAddress, &lastLength);
			/* now advance to the starting addr of next structure */
			tableAddress += lastLength;
			/*adjust the offset accordingly*/
			offset += lastLength;
		}
	} /* END while-loop looking for structure type */

	if ( ret == EFI_SMBIOSERR_STRUCT_NOT_FOUND )	/*not found the structure!*/
		offset = 0xFFFF;
	
	if (pKey != NULL)
		*pKey = offset;  /*return the acutal offset to *pKey*/
	return ret;
}
#pragma warning ( default : 4312 )

/*
Function  NextTableAddress()

  Parameters
	IN	UINT8 * tableAddress,
		the starting addr of the specified structure
	OUT	UINT16 *offset
		return the total length of the specified structure,
		including the pending strings.

  Description
	This function calculate the total length of a specified structure.
  
  Status Codes Returned
	EFI_SUCCESS	Successfully implemented
*/

EFI_STATUS
NextTableAddress(
	IN	UINT8  *tableAddress,
	OUT	UINT16 *offset
	)
{
	UINT16 nOffset;  /*the offset in byte from the structure starting point*/

	/*first get the structure length (excluding the pending strings*/
	nOffset = tableAddress[1]; 

	/*now advance to the starting offset of the pending srings area*/
	tableAddress += nOffset ;

	/*the pending string area ends with 2 bytes of 0 (0x0000h) */
	while ( tableAddress[0] != 0 || tableAddress[1] != 0 ) {
		tableAddress++;
		nOffset ++;
	} /* Get past trailing string-list */
	/*tableAddress += 2;*/
	nOffset +=2;  /*adjust for the last 2 bytes of 0;*/
	*offset = nOffset;  /*return the offset*/
	return EFI_SUCCESS;
}

/*
Function  GetPendingString(

  Parameters
	IN	UINT8	*StructAddress,
		the starting addr of the specified structure
	IN	UINT16	StringNumber,
		the index of the pending string, starting from 1
	OUT CHAR8	StringBuf[64]
		the buffer of the specified string that returns.

  Description
	This function get the specifed number of pending string.
  
  Status Codes Returned
	EFI_SUCCESS				Successfully implemented 
	EFI_SMBIOSERR_FAILURE	Failed 
*/
EFI_STATUS
GetPendingString(
	IN	UINT8	*StructAddress,
	IN	UINT16	StringNumber,
	OUT CHAR8	StringBuf[64]
	)
{
	int i;

	/*first get the structure length (excluding the pending strings*/
	StructAddress += StructAddress[1];

	i = 1;
	while ( i<StringNumber) {
		/*go thru the pending strings until i = StringNumber*/
		if ( *StructAddress == 0 )  /*found one string!*/
			i++;
		StructAddress++;
	}

	/* now set i to the length of the specifed string*/
	for (i = 0; i < 64; i++, StringBuf++, StructAddress++) {
		*StringBuf = *StructAddress;
		if (*StructAddress == 0 )
			break;
	}

	/*according to SMBIOS spec, the string area shall not
	be longer than 64 bytes */
	if (i>=64)
		return EFI_SMBIOSERR_FAILURE;
	return EFI_SUCCESS;
}

/*
Function  BOOLEAN CheckEnumType(
  Parameters
	IN UINT8 Type
		type to query

  Description
	This function check if the specifed type is so far a known one
  
  Status Codes Returned
	TRUE:  yes we know the member info of this type.
	FALSE: no we don't know this type.
*/

BOOLEAN CheckEnumType(
	IN UINT8 Type
	)
{
	enum enumStructureType eType = (enum enumStructureType)Type;

	if ( (eType >= eSMBIOSType0 && eType <= eSMBIOSType30) ||
		(eType >= eSMBIOSType32 && eType <= eSMBIOSType39) ||
		(eType == eSMBIOSType126) || (eType == eSMBIOSType127)
		)
		return TRUE;
	else
		return FALSE;
}

/*
Function  DUMPMEM (void* v, int length)
  Parameters
	v	the starting addr for dumping
	length	length of dumping

  Description
	This internal function dump the specified memory buffer to both
	screen and debug port.
  
  Status Codes Returned
	none
*/

DUMPMEM (void* v, int length)
{
	UINT8 *x= (UINT8 *)v;
	int i=0;
	while (i<length) {
		Print(L"%02x ",*x);
		DEBUG((D_LOAD, "%02x ",*x++));

		i++;
		if (i%20 == 0) {
			DEBUG((D_LOAD, "\n"));
			Print(L"\n");

		}
	}
	Print(L"\n");
	DEBUG((D_LOAD, "\n"));
	return( 0 ) ;
}


/*
Function  EFI_STATUS SMBIOS_FreeStructure()

  Parameters
	IN VOID			*pStructureBuffer,
		the buffer returned from SMBIOS_GetStructure or SMBIOS_GetRawStructure

  Description
	This function:
		1. Free an SMBIO structure
  
  Returned Value:
	Return code from FreePool()
*/
EFI_STATUS
SMBIOS_FreeStructure (
	IN		VOID	*pStructureBuffer
)
{
	return FREE( pStructureBuffer );
}


/*
Function  UINT16 CopyStruct()

  Parameters
	IN		UINT8			Type,
		the type of the structure to be copied
	IN OUT	VOID			**pStructureBuffer,
		the buffer the the structure to be copied to
	IN		UINT8			*tableAddress
		the starting addr of the specified type
		of structure in system SMBIOS table

  Description
	This function:
		1. allocate memory for *pStructureBuffer
		2. copy strurtured data from system buffer to
			memory structure buffer
		3. check for type and copy formatted 
			aligned data accordingly.
  
  Returned Value:
	actual length allocated
*/
UINT16
CopyStruct(
	IN		UINT8			Type,
	IN OUT	VOID			**pStructureBuffer,
	IN		UINT8			*tableAddress
	)
{
	UINT8	*tmpBuf;
	UINT16	i;
	UINT16	count;
	UINT16	length;
	VOID	*StructureBuffer;

	tmpBuf = (CHAR8 *)tableAddress;

	/* the macro to copy a pending string to structure member*/
#define AlignedStringCopy( type, element ) \
	do{\
	GetPendingString (tableAddress, *tmpBuf, ((type*)StructureBuffer)->element);\
	tmpBuf++;\
	}while(0);

/* the macro to copy a data member to structure member*/
#define AlignedQuickCopy( type , element ) \
	do{\
    MEMCPY((UINT8*)&( ((type*)StructureBuffer)->element), tmpBuf, sizeof(((type*)StructureBuffer)->element));\
    tmpBuf += sizeof(((type*)StructureBuffer)->element);\
	}while(0);

#define AlignedByteCopy( type , element, len ) \
	do{\
    MEMCPY((UINT8*)&( ((type*)StructureBuffer)->element), tmpBuf, len);\
    tmpBuf += len;\
	}while(0);

/* now check for type and copy formatted aligned data accordingly.*/
	switch (Type) {
	case 0:
		count = tableAddress[1] - 0x12;
		length = sizeof (SMBIOSType0) + (count - 1);
		StructureBuffer = (VOID *) MALLOC ( length );
		AlignedQuickCopy( SMBIOSType0, Header );
		AlignedStringCopy( SMBIOSType0, Vendor );
		AlignedStringCopy( SMBIOSType0, BIOSVersion );
		AlignedQuickCopy( SMBIOSType0, BIOSStartAddrSeg );
		AlignedStringCopy( SMBIOSType0, BIOSReleaseDate );
		AlignedQuickCopy( SMBIOSType0, BIOSROMSize );
		AlignedQuickCopy( SMBIOSType0, BIOSCharacteristics );
		AlignedByteCopy( SMBIOSType0, BIOSCharacteristicsExt, count );
		((SMBIOSType0*)StructureBuffer)->CharacteristicsExtSize = (UINT8)count;
		break;

	case 1:
		length = sizeof ( SMBIOSType1 );
		StructureBuffer = (VOID *) MALLOC ( length );
		AlignedQuickCopy( SMBIOSType1, Header );
		AlignedStringCopy( SMBIOSType1, Manufacturer );
		AlignedStringCopy( SMBIOSType1, ProductName );
		AlignedStringCopy( SMBIOSType1, Version );
		AlignedStringCopy( SMBIOSType1, SerialNumber );
		AlignedQuickCopy( SMBIOSType1, UUID );
		AlignedQuickCopy( SMBIOSType1, WakeUpType );
		break;

	case 2:
		length = sizeof ( SMBIOSType2 );
		StructureBuffer = (VOID *) MALLOC ( length );
		AlignedQuickCopy( SMBIOSType2, Header );
		AlignedStringCopy( SMBIOSType2, Manufacturer );
		AlignedStringCopy( SMBIOSType2, ProductName );
		AlignedStringCopy( SMBIOSType2, Version );
		AlignedStringCopy( SMBIOSType2, SerialNumber );
		break;

	case 3:
		length = sizeof ( SMBIOSType3 );
		StructureBuffer = (VOID *) MALLOC ( length );
		AlignedQuickCopy( SMBIOSType3, Header );
		AlignedStringCopy( SMBIOSType3, Manufacturer );
		AlignedQuickCopy( SMBIOSType3, ChassisType );
		AlignedStringCopy( SMBIOSType3, Version );
		AlignedStringCopy( SMBIOSType3, SerialNumber );
		AlignedStringCopy( SMBIOSType3, AssetTagNumber );
		AlignedQuickCopy( SMBIOSType3, BootupState );
		AlignedQuickCopy( SMBIOSType3, PowerSupplyState );
		AlignedQuickCopy( SMBIOSType3, ThermalState );
		AlignedQuickCopy( SMBIOSType3, SecurityStatus );
		AlignedQuickCopy( SMBIOSType3, OEMDefined );
		break;

	case 4:
		length = sizeof ( SMBIOSType4 );
		StructureBuffer = (VOID *) MALLOC ( length );
		AlignedQuickCopy( SMBIOSType4, Header );
		AlignedStringCopy( SMBIOSType4, SocketDesignation );
		AlignedQuickCopy( SMBIOSType4, ProcessorType );
		AlignedQuickCopy( SMBIOSType4, ProcessorFamily );
		AlignedStringCopy( SMBIOSType4, ProcessorManufacturer );
		AlignedQuickCopy( SMBIOSType4, ProcessorID );
		AlignedStringCopy( SMBIOSType4, ProcessorVersion );
		AlignedQuickCopy( SMBIOSType4, Voltage );
		AlignedQuickCopy( SMBIOSType4, ExternalClock );
		AlignedQuickCopy( SMBIOSType4, MaxSpeed );
		AlignedQuickCopy( SMBIOSType4, CurrentSpeed );
		AlignedQuickCopy( SMBIOSType4, Status );
		AlignedQuickCopy( SMBIOSType4, ProcessorUpgrade );
		AlignedQuickCopy( SMBIOSType4, L1CacheHandle );
		AlignedQuickCopy( SMBIOSType4, L2CacheHandle );
		AlignedQuickCopy( SMBIOSType4, L3CacheHandle );
		AlignedStringCopy( SMBIOSType4, SerialNumber );
		AlignedStringCopy( SMBIOSType4, AssetTag );
		AlignedStringCopy( SMBIOSType4, PartNumber );
		break;

	case 5:
		length = sizeof ( SMBIOSType5 );
		StructureBuffer = (VOID *) MALLOC ( length );
		AlignedQuickCopy( SMBIOSType5, Header );
		AlignedQuickCopy( SMBIOSType5, ErrorDetectingMethod );
		AlignedQuickCopy( SMBIOSType5, ErrorCorrectingCapability );
		AlignedQuickCopy( SMBIOSType5, SupportedInterleave );
		AlignedQuickCopy( SMBIOSType5, CurrentInterleave );
		AlignedQuickCopy( SMBIOSType5, MaximumMemoryModuleSize );
		AlignedQuickCopy( SMBIOSType5, SupportedSpeeds );
		AlignedQuickCopy( SMBIOSType5, SupportedMemoryTypes );
		AlignedQuickCopy( SMBIOSType5, MemoryModuleVoltage );
		AlignedQuickCopy( SMBIOSType5, AssociatedMemorySlots );
		AlignedQuickCopy( SMBIOSType5, MemoryModuleConfigHandle );
		AlignedQuickCopy( SMBIOSType5, EnabledErrorCorrectingCapabilities );
		break;

	case 6:
		length = sizeof ( SMBIOSType6 );
		StructureBuffer = (VOID *) MALLOC ( length );
		AlignedQuickCopy( SMBIOSType6, Header );
		AlignedStringCopy( SMBIOSType6, SocketDesignation );
		AlignedQuickCopy( SMBIOSType6, BankConnections );
		AlignedQuickCopy( SMBIOSType6, CurrentSpeed );
		AlignedQuickCopy( SMBIOSType6, CurrentMemoryType );
		AlignedQuickCopy( SMBIOSType6, InstalledSize );
		AlignedQuickCopy( SMBIOSType6, EnabledSize );
		AlignedQuickCopy( SMBIOSType6, ErrorStatus );
		break;

	case 7:
		length = sizeof ( SMBIOSType7 );
		StructureBuffer = (VOID *) MALLOC ( length );
		AlignedQuickCopy( SMBIOSType7, Header );
		AlignedStringCopy( SMBIOSType7, SocketDesignation );
		AlignedQuickCopy( SMBIOSType7, CacheConfiguration );
		AlignedQuickCopy( SMBIOSType7, MaximumCacheSize );
		AlignedQuickCopy( SMBIOSType7, InstalledSize );
		AlignedQuickCopy( SMBIOSType7, SupportedSRAMType );
		AlignedQuickCopy( SMBIOSType7, CurrentSRAMType );
		AlignedQuickCopy( SMBIOSType7, CacheSpeed );
		AlignedQuickCopy( SMBIOSType7, ErrorCorrectionType );
		AlignedQuickCopy( SMBIOSType7, SystemCacheType );
		AlignedQuickCopy( SMBIOSType7, Associativity );
		break;

	case 8:
		length = sizeof ( SMBIOSType8 );
		StructureBuffer = (VOID *) MALLOC ( length );
		AlignedQuickCopy( SMBIOSType8, Header );
		AlignedStringCopy( SMBIOSType8, InternalReferenceDesignator );
		AlignedQuickCopy( SMBIOSType8, InternalConnectorType );
		AlignedStringCopy( SMBIOSType8, ExternalReferenceDesignator );
		AlignedQuickCopy( SMBIOSType8,  ExternalConnectorType );
		AlignedQuickCopy( SMBIOSType8,  PortType );
		break;

	case 9:
		length = sizeof ( SMBIOSType9 );
		StructureBuffer = (VOID *) MALLOC ( length );
		AlignedQuickCopy( SMBIOSType9, Header );
		AlignedStringCopy( SMBIOSType9, SlotDesignation );
		AlignedQuickCopy( SMBIOSType9, SlotType );
		AlignedQuickCopy( SMBIOSType9, SlotDataBusWidth );
		AlignedQuickCopy( SMBIOSType9, CurrentUsage );
		AlignedQuickCopy( SMBIOSType9, SlotLength );
		AlignedQuickCopy( SMBIOSType9, SlotID );
		AlignedQuickCopy( SMBIOSType9, SlotCharacteristics );
		AlignedQuickCopy( SMBIOSType9, SlotCharacteristics2 );
		break;

	case 10:
		length = sizeof ( SMBIOSType10 );
		StructureBuffer = (VOID *) MALLOC ( length );
		AlignedQuickCopy( SMBIOSType10, Header );
		AlignedQuickCopy( SMBIOSType10, Device );
		break;

	case 11:
		count = tableAddress[4];
		length = sizeof (SMBIOSType11) + 64 * (count-1);
		StructureBuffer = (VOID *) MALLOC ( length );
		AlignedQuickCopy( SMBIOSType11, Header );
		AlignedQuickCopy( SMBIOSType11, Count );
		for ( i=0; i<count; i++)
			GetPendingString (tableAddress, (UINT16)(i+1), 
					((SMBIOSType11*)StructureBuffer)->bufOEMString[i] );
		break;

	case 12:
		count = tableAddress[4];
		length = sizeof (SMBIOSType12) + 64 * (count-1);
		StructureBuffer = (VOID *) MALLOC ( length );
		AlignedQuickCopy( SMBIOSType12, Header );
		AlignedQuickCopy( SMBIOSType12, Count );
		for ( i=0; i<count; i++)
			GetPendingString (tableAddress, (UINT16)(i+1), 
					((SMBIOSType12*)StructureBuffer)->bufSysConfigurations[i] );
		break;
	
	case 13:
		count = tableAddress[4];
		length = sizeof (SMBIOSType13) + 64 * (count-1);
		StructureBuffer = (VOID *) MALLOC ( length );
		AlignedQuickCopy( SMBIOSType13, Header );
		AlignedQuickCopy( SMBIOSType13, InstallableLanguages );
		AlignedQuickCopy( SMBIOSType13, Flags );
		AlignedQuickCopy( SMBIOSType13, reserved );
		AlignedQuickCopy( SMBIOSType13, CurrentLanguageIndex );
		for ( i=0; i<count; i++)
			GetPendingString (tableAddress, (UINT16)(i+1), 
					((SMBIOSType13*)StructureBuffer)->IntalledLanguages[i] );
		break;

	case 14:
		length = sizeof ( SMBIOSType14 );
		StructureBuffer = (VOID *) MALLOC ( length );
		AlignedQuickCopy( SMBIOSType14, Header );
		AlignedStringCopy( SMBIOSType14, GroupName );
		AlignedQuickCopy( SMBIOSType14, ItemType );
		AlignedQuickCopy( SMBIOSType14, ItemHandle );
		break;

	case 15:
		count = tableAddress[0x15];
		length = sizeof (SMBIOSType15) + sizeof (EVENTLOGTYPE) * (count-1) ;
		StructureBuffer = (VOID *) MALLOC ( length );
		AlignedQuickCopy( SMBIOSType15, Header );
		AlignedQuickCopy( SMBIOSType15, LogAreaLength );
		AlignedQuickCopy( SMBIOSType15, LogHeaderStartOffset );
		AlignedQuickCopy( SMBIOSType15, LogDataStartOffset );
		AlignedQuickCopy( SMBIOSType15, AccessMethod );
		AlignedQuickCopy( SMBIOSType15, LogStatus );
		AlignedQuickCopy( SMBIOSType15, LogChangeToken );
		AlignedQuickCopy( SMBIOSType15, AccessMethodAddress );
		AlignedQuickCopy( SMBIOSType15, LogHeaderFormat );
		AlignedQuickCopy( SMBIOSType15, NumberOfSupportedLogTypeDescriptors );
		AlignedQuickCopy( SMBIOSType15, LengthOfLogTypeDescriptor );
		for ( i=0; i<count; i++)
			AlignedQuickCopy( SMBIOSType15, EventLogTypeDescriptors[i] );
		break;

	case 16:
		length = sizeof ( SMBIOSType16 );
		StructureBuffer = (VOID *) MALLOC ( length );
		AlignedQuickCopy( SMBIOSType16, Header );
		AlignedQuickCopy( SMBIOSType16, Location );
		AlignedQuickCopy( SMBIOSType16, Use );
		AlignedQuickCopy( SMBIOSType16, MemoryErrorCorrection );
		AlignedQuickCopy( SMBIOSType16, MaximumCapacity );
		AlignedQuickCopy( SMBIOSType16, MemoryErrorInformationHandle );
		AlignedQuickCopy( SMBIOSType16, NumberOfMemoryDevices );
		break;

	case 17:
		length = sizeof ( SMBIOSType17 );
		StructureBuffer = (VOID *) MALLOC ( length );
		AlignedQuickCopy( SMBIOSType17, Header );
		AlignedQuickCopy( SMBIOSType17, MemoryArrayHandle );
		AlignedQuickCopy( SMBIOSType17, MemoryErrorInformationHandle );
		AlignedQuickCopy( SMBIOSType17, TotalWidth );
		AlignedQuickCopy( SMBIOSType17, DataWidth );
		AlignedQuickCopy( SMBIOSType17, Size );
		AlignedQuickCopy( SMBIOSType17, FormFactor );
		AlignedQuickCopy( SMBIOSType17, DeviceSet );
		AlignedStringCopy( SMBIOSType17, DeviceLocator );
		AlignedStringCopy( SMBIOSType17, BankLocator );
		AlignedQuickCopy( SMBIOSType17, MemoryType );
		AlignedQuickCopy( SMBIOSType17, TypeDetail );
		AlignedQuickCopy( SMBIOSType17, Speed );
		AlignedStringCopy( SMBIOSType17, Manufacturer );
		AlignedStringCopy( SMBIOSType17, SerialNumber );
		AlignedStringCopy( SMBIOSType17, AssetTag );
		AlignedStringCopy( SMBIOSType17, PartNumber );
		break;

	case 18:
		length = sizeof ( SMBIOSType18 );
		StructureBuffer = (VOID *) MALLOC ( length );
		AlignedQuickCopy( SMBIOSType18, Header );
		AlignedQuickCopy( SMBIOSType18, ErrorType );
		AlignedQuickCopy( SMBIOSType18, ErrorGranularity );
		AlignedQuickCopy( SMBIOSType18, ErrorOperation );
		AlignedQuickCopy( SMBIOSType18, VendorSyndrome );
		AlignedQuickCopy( SMBIOSType18, MemoryArrayErrorAddress );
		AlignedQuickCopy( SMBIOSType18, DeviceErrorAddress );
		AlignedQuickCopy( SMBIOSType18, ErrorResolution );
		break;

	case 19:
		length = sizeof ( SMBIOSType19 );
		StructureBuffer = (VOID *) MALLOC ( length );
		AlignedQuickCopy( SMBIOSType19, Header );
		AlignedQuickCopy( SMBIOSType19, StartingAddress );
		AlignedQuickCopy( SMBIOSType19, EndingAddress );
		AlignedQuickCopy( SMBIOSType19, MemoryArrayHandle );
		AlignedQuickCopy( SMBIOSType19, PartitionWidth );
		break;

	case 20:
		length = sizeof ( SMBIOSType20 );
		StructureBuffer = (VOID *) MALLOC ( length );
		AlignedQuickCopy( SMBIOSType20, Header );
		AlignedQuickCopy( SMBIOSType20, StartingAddress );
		AlignedQuickCopy( SMBIOSType20, EndingAddress );
		AlignedQuickCopy( SMBIOSType20, MemoryDeviceHandle );
		AlignedQuickCopy( SMBIOSType20, MemoryArrayMappedAddressHandle );
		AlignedQuickCopy( SMBIOSType20, PartitionRowPosition );
		AlignedQuickCopy( SMBIOSType20, InterleavePosition );
		AlignedQuickCopy( SMBIOSType20, InterleavedDataDepth );
		break;

	case 21:
		length = sizeof ( SMBIOSType21 );
		StructureBuffer = (VOID *) MALLOC ( length );
		AlignedQuickCopy( SMBIOSType21, Header );
		AlignedQuickCopy( SMBIOSType21, Type );
		AlignedQuickCopy( SMBIOSType21, Interface );
		AlignedQuickCopy( SMBIOSType21, NumberOfButtons );
		break;

	case 22:
		length = sizeof ( SMBIOSType22 );
		StructureBuffer = (VOID *) MALLOC ( length );
		AlignedQuickCopy( SMBIOSType22, Header );
		AlignedStringCopy( SMBIOSType22, Location );
		AlignedStringCopy( SMBIOSType22, Manufacturer );
		AlignedStringCopy( SMBIOSType22, ManufactureDate );
		AlignedStringCopy( SMBIOSType22, SerialNumber );
		AlignedStringCopy( SMBIOSType22, DeviceName );
		AlignedStringCopy( SMBIOSType22, BankLocator );
		AlignedQuickCopy( SMBIOSType22, DeviceChemistry );
		AlignedQuickCopy( SMBIOSType22, DeviceCapacity );
		AlignedQuickCopy( SMBIOSType22, DesignVoltage );
		AlignedStringCopy( SMBIOSType22, SBDSVersionNumber );
		AlignedQuickCopy( SMBIOSType22, MaximumErrorInBatteryData );
		AlignedQuickCopy( SMBIOSType22, SBDSSerialNumber );
		AlignedQuickCopy( SMBIOSType22, SBDSManufactureDate );
		AlignedStringCopy( SMBIOSType22, SBDSDeviceChemistry );
		AlignedQuickCopy( SMBIOSType22, DesignCapacityMultiplier );
		AlignedQuickCopy( SMBIOSType22, OEMSpecific );
		break;

	case 23:
		length = sizeof ( SMBIOSType23 );
		StructureBuffer = (VOID *) MALLOC ( length );
		AlignedQuickCopy( SMBIOSType23, Header );
		AlignedQuickCopy( SMBIOSType23, Capabilities );
		AlignedQuickCopy( SMBIOSType23, ResetCount );
		AlignedQuickCopy( SMBIOSType23, ResetLimit );
		AlignedQuickCopy( SMBIOSType23, TimerInterval );
		AlignedQuickCopy( SMBIOSType23, Timeout );
		break;

	case 24:
		length = sizeof ( SMBIOSType24 );
		StructureBuffer = (VOID *) MALLOC ( length );
		AlignedQuickCopy( SMBIOSType24, Header );
		AlignedQuickCopy( SMBIOSType24, HardwareSecuritySettings );
		break;

	case 25:
		length = sizeof ( SMBIOSType25 );
		StructureBuffer = (VOID *) MALLOC ( length );
		AlignedQuickCopy( SMBIOSType25, Header );
		AlignedQuickCopy( SMBIOSType25, NextScheduledPowerOnMonth );
		AlignedQuickCopy( SMBIOSType25, NextScheduledPowerOnDayOfMonth );
		AlignedQuickCopy( SMBIOSType25, NextScheduledPowerOnHour );
		AlignedQuickCopy( SMBIOSType25, NextScheduledPowerOnMinute );
		AlignedQuickCopy( SMBIOSType25, NextScheduledPowerOnSecond );
		break;

	case 26:
		length = sizeof ( SMBIOSType26 );
		StructureBuffer = (VOID *) MALLOC ( length );
		AlignedQuickCopy( SMBIOSType26, Header );
		AlignedStringCopy( SMBIOSType26, Description );
		AlignedQuickCopy( SMBIOSType26, LocationAndStatus );
		AlignedQuickCopy( SMBIOSType26, MaximumValue );
		AlignedQuickCopy( SMBIOSType26, MinimumValue );
		AlignedQuickCopy( SMBIOSType26, Resolution );
		AlignedQuickCopy( SMBIOSType26, Tolerance );
		AlignedQuickCopy( SMBIOSType26, Accuracy );
		AlignedQuickCopy( SMBIOSType26, OEMDefined );
		AlignedQuickCopy( SMBIOSType26, NominalValue );
		break;

	case 27:
		length = sizeof ( SMBIOSType27 );
		StructureBuffer = (VOID *) MALLOC ( length );
		AlignedQuickCopy( SMBIOSType27, Header );
		AlignedQuickCopy( SMBIOSType27, TemperatureProbeHandle );
		AlignedQuickCopy( SMBIOSType27, DeviceTypeAndStatus );
		AlignedQuickCopy( SMBIOSType27, CoolingUnitGroup );
		AlignedQuickCopy( SMBIOSType27, OEMDefined );
		AlignedQuickCopy( SMBIOSType27, NominalSpeed );
		break;

	case 28:
		length = sizeof ( SMBIOSType28 );
		StructureBuffer = (VOID *) MALLOC ( length );
		AlignedQuickCopy( SMBIOSType28, Header );
		AlignedStringCopy( SMBIOSType28, Description );
		AlignedQuickCopy( SMBIOSType28, LocationAndStatus );
		AlignedQuickCopy( SMBIOSType28, MaximumValue );
		AlignedQuickCopy( SMBIOSType28, MinimumValue );
		AlignedQuickCopy( SMBIOSType28, Resolution );
		AlignedQuickCopy( SMBIOSType28, Tolerance );
		AlignedQuickCopy( SMBIOSType28, Accuracy );
		AlignedQuickCopy( SMBIOSType28, OEMDefined );
		AlignedQuickCopy( SMBIOSType28, NominalValue );
		break;

	case 29:
		length = sizeof ( SMBIOSType29 );
		StructureBuffer = (VOID *) MALLOC ( length );
		AlignedQuickCopy( SMBIOSType29, Header );
		AlignedStringCopy( SMBIOSType29, Description );
		AlignedQuickCopy( SMBIOSType29, LocationAndStatus );
		AlignedQuickCopy( SMBIOSType29, MaximumValue );
		AlignedQuickCopy( SMBIOSType29, MinimumValue );
		AlignedQuickCopy( SMBIOSType29, Resolution );
		AlignedQuickCopy( SMBIOSType29, Tolerance );
		AlignedQuickCopy( SMBIOSType29, Accuracy );
		AlignedQuickCopy( SMBIOSType29, OEMDefined );
		AlignedQuickCopy( SMBIOSType29, NominalValue );
		break;

	case 30:
		length = sizeof ( SMBIOSType30 );
		StructureBuffer = (VOID *) MALLOC ( length );
		AlignedQuickCopy( SMBIOSType30, Header );
		AlignedStringCopy( SMBIOSType30, ManufacturerName );
		AlignedQuickCopy( SMBIOSType30, Connections );
		break;

	case 32:
		length = sizeof ( SMBIOSType32 );
		StructureBuffer = (VOID *) MALLOC ( length );
		AlignedQuickCopy( SMBIOSType32, Header );
		AlignedQuickCopy( SMBIOSType32, Reserved );
		AlignedQuickCopy( SMBIOSType32, BootStatus );
		break;

	case 33:
		length = sizeof ( SMBIOSType33 );
		StructureBuffer = (VOID *) MALLOC ( length );
		AlignedQuickCopy( SMBIOSType33, Header );
		AlignedQuickCopy( SMBIOSType33, ErrorType );
		AlignedQuickCopy( SMBIOSType33, ErrorGranularity );
		AlignedQuickCopy( SMBIOSType33, ErrorOperation );
		AlignedQuickCopy( SMBIOSType33, VendorSyndrome );
		AlignedQuickCopy( SMBIOSType33, MemoryArrayErrorAddress );
		AlignedQuickCopy( SMBIOSType33, DeviceErrorAddress );
		AlignedQuickCopy( SMBIOSType33, ErrorResolution );
		break;

	case 34:
		length = sizeof ( SMBIOSType34 );
		StructureBuffer = (VOID *) MALLOC ( length );
		AlignedQuickCopy( SMBIOSType34, Header );
		AlignedStringCopy( SMBIOSType34, Description );
		AlignedQuickCopy( SMBIOSType34, Type );
		AlignedQuickCopy( SMBIOSType34, Address );
		AlignedQuickCopy( SMBIOSType34, AddressType );
		break;

	case 35:
		length = sizeof ( SMBIOSType35 );
		StructureBuffer = (VOID *) MALLOC ( length );
		AlignedQuickCopy( SMBIOSType35, Header );
		AlignedStringCopy( SMBIOSType35, Description );
		AlignedQuickCopy( SMBIOSType35, ManagementDeviceHandle );
		AlignedQuickCopy( SMBIOSType35, ComponentHandle );
		AlignedQuickCopy( SMBIOSType35, ThresholdHandle );
		break;

	case 36:
		length = sizeof ( SMBIOSType36 );
		StructureBuffer = (VOID *) MALLOC ( length );
		AlignedQuickCopy( SMBIOSType36, Header );
		AlignedQuickCopy( SMBIOSType36, LowerThresholdNonCritical );
		AlignedQuickCopy( SMBIOSType36, UpperThresholdNonCritical );
		AlignedQuickCopy( SMBIOSType36, LowerThresholdCritical );
		AlignedQuickCopy( SMBIOSType36, UpperThreaholdCritical );
		AlignedQuickCopy( SMBIOSType36, LowerThresholdNonRecoverable );
		AlignedQuickCopy( SMBIOSType36, UpperThresholdNonRecoverable );
		break;

	case 37:
		count = tableAddress[6];
		length = sizeof (SMBIOSType37) + sizeof (MEMORYDEVICE) * (count-1) ;
		StructureBuffer = (VOID *) MALLOC ( length );
		AlignedQuickCopy( SMBIOSType37, Header );
		AlignedQuickCopy( SMBIOSType37, ChannelType );
		AlignedQuickCopy( SMBIOSType37, MaximumChannelLoad );
		AlignedQuickCopy( SMBIOSType37, MemoryDeviceCount );
		for ( i=0; i<count; i++)
			AlignedQuickCopy( SMBIOSType37, MemoryDevice[i] );
		break;

	case 38:
		length = sizeof ( SMBIOSType38 );
		StructureBuffer = (VOID *) MALLOC ( length );
		AlignedQuickCopy( SMBIOSType38, Header );
		AlignedQuickCopy( SMBIOSType38, InterfaceType );
		AlignedQuickCopy( SMBIOSType38, IPMISpecificationRevision );
		AlignedQuickCopy( SMBIOSType38, I2CSlaveAddress );
		AlignedQuickCopy( SMBIOSType38, NVStorageDeviceAddress );
		AlignedQuickCopy( SMBIOSType38, BaseAddress );
		/* check if structure supports Rev 3 Addendum */
		if (((SMBIOSType38*)(StructureBuffer))->Header.StructLength > 0x10 ) {
			AlignedQuickCopy( SMBIOSType38, BaseAddressModifier_InterruptInfo);
			AlignedQuickCopy( SMBIOSType38, InterruptNumber);
		}
		break;

	case 39:
		length = sizeof ( SMBIOSType39 );
		StructureBuffer = (VOID *) MALLOC ( length );
		AlignedQuickCopy( SMBIOSType39, Header );
		AlignedQuickCopy( SMBIOSType39, PowerUnitGroup );
		AlignedStringCopy( SMBIOSType39, Location );
		AlignedStringCopy( SMBIOSType39, DeviceName );
		AlignedStringCopy( SMBIOSType39, Manufacturer );
		AlignedStringCopy( SMBIOSType39, SerialNumber );
		AlignedStringCopy( SMBIOSType39, AssetTagNumber );
		AlignedStringCopy( SMBIOSType39, ModelPartNumber );
		AlignedStringCopy( SMBIOSType39, RevisionLevel );
		AlignedStringCopy( SMBIOSType39, Description );
		AlignedQuickCopy( SMBIOSType39, MaxPowerCapacity );
		AlignedQuickCopy( SMBIOSType39, PowerSupplyCharacteristics );
		AlignedQuickCopy( SMBIOSType39, InputVoltageProbeHandle );
		AlignedQuickCopy( SMBIOSType39, CoolingDeviceHandle );
		AlignedQuickCopy( SMBIOSType39, InputCurrentProbeHandle );
		break;

	case 126:
		length = sizeof ( SMBIOSType126 );
		StructureBuffer = (VOID *) MALLOC ( length );
		AlignedQuickCopy( SMBIOSType126, Header );
		break;
		
	case 127:
		length = sizeof ( SMBIOSType127 );
		StructureBuffer = (VOID *) MALLOC ( length );
		AlignedQuickCopy( SMBIOSType127, Header );
		break;
	}  /*swtich (type) */

	*pStructureBuffer = StructureBuffer;
	return length;
}
