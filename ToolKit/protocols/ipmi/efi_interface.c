/*
 * Copyright (c) 1999, 2000
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

/******************************************************************************
*
*   Revision History:
*   $Log: efi_interface.c,v $
*   Revision 1.1.1.1  2006/05/30 06:16:11  hhzhou
*   no message
*
 * 
 *    Rev 1.3   14 Jul 2000 10:51:54   eamaya
 * - Updated KCS protocol per addenda to IPMI spec.
 *   Note that this update is not backward compatible
 *   to previous KCS protocol.
 * - Added ability to unload the driver.
 * 
 *    Rev 1.2   14 Jun 2000 12:48:26   eamaya
 * - Updated to work with SMBIOS table to look up KCS
 * address in Record 38 and override default KCS address CA2.
 * - Updated revision from 1.0 to 1.1
*
******************************************************************************/

#include "efiipmi.h"
#include "stdlib.h"
#include "smic.h"
#include "kcs.h"

typedef enum
{
	Unknown,        // Unknown Interface
    SMIC_INTF,      // SMIC Interface
	KCS_INTF        // KCS Interface
} InterfaceType;

InterfaceType Interface = Unknown;

// ----------------------------------------------------------------------------
// Name
//		DetermineInterface
//
// Abstract:
//		Determines the Interface type
//
// Parameters:
//			
// Return Value:
//		
// ----------------------------------------------------------------------------

VOID DetermineInterface (VOID)
{
	if (IsKCS ()) 
    {
		Interface = KCS_INTF;
	}
	else 
    {
		Interface = SMIC_INTF;            
	}
}

// ----------------------------------------------------------------------------
// Name
//		SendMessage
//
// Abstract:
//		Send message to BMC.
//
// Parameters:
//		*This	-	A pointer to the EFI_IPMI_INTERFACE instance.
//		NetFn	-	Network Function code.
//		LUN		-	Logical Unit Number.
//		Cmd		-	Command code. 
//					Specifies the operation that is to be 
//					executed under the specified Network Function.
//		*Data	-	A pointer to zero or more bytes of data, 
//					as required by the given command. 
//					The general convention is to pass data LS-byte first, 
//					but check the individual command specifications to be sure.
//		DataLen	-	Length of the Data in bytes.
//
// Return Value:
//		EFI_SUCCESS				The request message is successfully sent to the BMC.
//		EFI_IPMIERR_FAILURE		The request message is failed to send to the BMC.
//		EFI_IPMIERR_TIMEOUT		Time out error.
//		EFI_IPMIERR_BMC_BUSY	BMC is busy.
//
// ----------------------------------------------------------------------------

static EFI_STATUS SendMessage (	IN	EFI_IPMI_INTERFACE	*This,
	                            IN	UINT8				NetFn,
	                            IN	UINT8				LUN,
	                            IN	UINT8				Cmd,
	                            IN	UINT8				*Data,
	                            IN	UINT32				DataLen
	                          )
{
	UINT8       *MsgBuf = NULL;
	UINT8       NetFnAndLun = 0;
	UINT32      Length = 0;
	UINT32      i = 0;
	EFI_STATUS  Status;

	DEBUG ((D_LOAD, "Executing IPMI SendMessage\n"));

	// Combine netfn and lun	
	NetFnAndLun = (NetFn << 2) | LUN;

	DEBUG ((D_LOAD, "in SendMessage, NetFnAndLun: 0x%x\n", NetFnAndLun));
	
    // Send NetfnLun and cmd along with Message Data
	Length = 2 + DataLen;

	DEBUG ((D_LOAD, "in SendMessage, Length: %d\n", Length));

	// Compose message buffer
	MsgBuf = calloc (Length, sizeof(UINT8));

	if (MsgBuf != NULL)	
    {
		DEBUG ((D_LOAD, "in SendMessage, Memory allocation success\n"));
	}
	
	MsgBuf[0] = NetFnAndLun;
	MsgBuf[1] = Cmd;

	for (i = 0; i < DataLen; i++) 
    {
		MsgBuf[i+2] = Data[i];
	}
	
	// determine interface
	if (Interface == Unknown) 
    {
		DetermineInterface ();
	}
	
	switch (Interface) 
    {
	case KCS_INTF:
		DEBUG ((D_LOAD, "KCS interface !!! \n\n"));
		Status = KCSSendMessage (MsgBuf, Length);
		break;

	case SMIC_INTF:
		DEBUG ((D_LOAD, "SMIC interface !!! \n\n"));
		Status = SMICSendMessage (MsgBuf, Length);
		break;

	case Unknown:
    default:
		DEBUG ((D_LOAD, "Unknown interface !!! \n\n"));
		Status = EFI_IPMIERR_FAILURE;
	}
	
	free (MsgBuf);

	return (Status);
}

// ----------------------------------------------------------------------------
// Name
//     GetMessage
// 
// Abstract:
//      Get message from BMC
//
// Parameters:
//   
//  	*This			-	A pointer to the EFI_IPMI_INTERFACE instance
//  	*NetFn			-	Network Function code, which was passed in the Request Mes-sage.
//  	*LUN			-	Logical Unit Number, which was passed in the Request Mes-sage. 
//  	*Cmd			-	Command code. Specifies the operation that is to be executed
//  	    				under the specified Network Function. 
//  		    			This is a return of the Cmd code that was passed in the Request Message.
//  	*CompletionCode	-	The IPMI Completion Code.
//  	*Data			-	A pointer to zero or more bytes of data, as required
//  						by the given command. The general convention is to 
//  						pass data LS-byte first, but check the individual command 
//  						specifications to be sure.
//  	*DataLen		-	IN: Size of the buffer that used to store data read from the BMC, 
//  						not including the bytes of NetFn, LUN and Cmd.
//  						OUT: The actually bytes of data read from BMC, 
//  						not including the bytes of NetFn, LUN and Cmd.
//
// Return Value:
//
//  	EFI_SUCCESS				Successfully get the response message from the BMC.
//  	EFI_IPMIERR_FAILURE		Response data didn't successfully return.
//  	EFI_BUFFER_TOO_SMALL	The size of returned buffer is larger than the size of allocated buffer.
//  	EFI_IPMIERR_TIMEOUT		Time out error.
//  	EFI_IPMIERR_BMC_BUSY	BMC is busy.
//
// ----------------------------------------------------------------------------
 
static EFI_STATUS GetMessage (  IN		EFI_IPMI_INTERFACE	*This,
	                            OUT		UINT8				*NetFn,
                                OUT		UINT8				*LUN,
                                OUT		UINT8				*Cmd,
                                OUT		UINT8				*CompletionCode,
                                OUT		UINT8				*Data,
                                IN OUT	UINT32				*DataLen
                             )
{
	UINT8       *MsgBuf = NULL;
	UINT32      Length = 0;
	EFI_STATUS  Status;
	UINT32      OutLength = 0;
	UINT8       NetFnAndLun = 0;
	UINT32      i = 0;

	DEBUG ((D_LOAD, "Executing IPMI GetMessage\n"));

    // Read NetfnLun, cmd and completion code along with Message data
	Length = 3 + *DataLen;
	
	DEBUG ((D_LOAD, "In IPMI GetMessage, Length, %d\n", Length));

	// allocate memory for composed message buffer
	MsgBuf = calloc (Length, sizeof(UINT8));
	
	if (MsgBuf == NULL)	
    {
		DEBUG ((D_LOAD, "In IPMI GetMessage, calloc success, %d\n"));
	}

	// determine interface
	if (Interface == Unknown) 
    {
		DetermineInterface();
	}
	
	switch (Interface) 
    {
	case KCS_INTF:
		Status = KCSReadBMCData (MsgBuf, Length, &OutLength);
		break;

	case SMIC_INTF:
		Status = SMICReadBMCData (MsgBuf, Length, &OutLength);
		break;

	case Unknown:
    default:
		Status = EFI_IPMIERR_FAILURE;
	}

	if (Status != EFI_SUCCESS)	
    {
		free (MsgBuf);
		return (Status);
	}

	DEBUG ((D_LOAD, "in IPMI_GetMessage, OutLength: %d\n", OutLength));

	DEBUG ((D_LOAD, "in IPMI_GetMessage, MsgBuf[0]: 0x%x\n", MsgBuf[0]));
	DEBUG ((D_LOAD, "in IPMI_GetMessage, MsgBuf[1]: 0x%x\n", MsgBuf[1]));
	DEBUG ((D_LOAD, "in IPMI_GetMessage, MsgBuf[2]: 0x%x\n", MsgBuf[2]));
	DEBUG ((D_LOAD, "in IPMI_GetMessage, MsgBuf[3]: 0x%x\n", MsgBuf[3]));
	
	// decompose returned data.
	NetFnAndLun = MsgBuf[0];
	*Cmd = MsgBuf[1];
	*CompletionCode = MsgBuf[2];

	DEBUG ((D_LOAD, "in IPMI_GetMessage, NetFnAndLun: 0x%x\n", NetFnAndLun));
	DEBUG ((D_LOAD, "in IPMI_GetMessage, Cmd: 0x%x\n", *Cmd));
	DEBUG ((D_LOAD, "in IPMI_GetMessage, CompletionCode: 0x%x\n", *CompletionCode));

	for (i = 0; i < OutLength - 1 - 1 - 1; i++)
	{
		Data[i] = MsgBuf[i+3];
	}

	*DataLen = OutLength - 1 - 1 - 1;

	DEBUG ((D_LOAD, "in IPMI_GetMessage, DataLen: %d\n", *DataLen));

	*NetFn = NetFnAndLun >> 2;
	*LUN = NetFnAndLun & 0x03;

	free (MsgBuf);
	
	return (EFI_SUCCESS);
}

// ----------------------------------------------------------------------------
//
// Name
//      InitIPMIInterface
//
// Abstract:
//      Initialize IPMI interface.
//
// Arguments:
//      TBD
//
// Return Value:
//      EFI_SUCCESS
//
// ----------------------------------------------------------------------------


EFI_STATUS InitIPMIInterface( VOID	)
{
	DEBUG ((D_LOAD, "Executing InitIPMIInterface\n"));
	return (EFI_SUCCESS);
}


//
//  The EFI IPMI Interface Table
//
EFI_IPMI_INTERFACE EfiIPMIInterface = {

	EFI_IPMI_INTERFACE_REVISION,

	SendMessage,
	GetMessage,
};

// ----------------------------------------------------------------------------
// Name
//      GetEfiIPMIInterface
//
// Abstract:
//      EFI emulation routine.
//
// Parameters:
//      TBD
//
// Return Value:
//      Pointer to an EFI IPMI interface
//
// ----------------------------------------------------------------------------

EFI_IPMI_INTERFACE* GetEfiIPMIInterface() 
{ 
    return (&EfiIPMIInterface); 
}
