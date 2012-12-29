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
*   $Log: kcs.c,v $
*   Revision 1.1.1.1  2006/05/30 06:16:12  hhzhou
*   no message
*
 * 
 *    Rev 1.3   14 Jul 2000 10:51:56   eamaya
 * - Updated KCS protocol per addenda to IPMI spec.
 *   Note that this update is not backward compatible
 *   to previous KCS protocol.
 * - Added ability to unload the driver.
 * 
 *    Rev 1.2   14 Jun 2000 12:48:28   eamaya
 * - Updated to work with SMBIOS table to look up KCS
 * address in Record 38 and override default KCS address CA2.
 * - Updated revision from 1.0 to 1.1
*
******************************************************************************/

#include "efiipmi.h"
#include "kcs.h"									
#include "smic.h"

volatile UINT8	Status;
UINT16			KCSAddress = KCS_DEFAULT_BASE_ADD;	// KCS Address

//
// ----------------------------------------------------------------------------
// Name
//      WaitWhileIbf
//
// Abstract:
//      IBF per IPMI spec, check time out
//
// Parameters:
//      Timeout - milliseconds for time out
//
// Return Value:
//  	TRUE	time out
//	    FALSE	not time out
//
// ----------------------------------------------------------------------------
//

UINT32 WaitWhileIbf ( UINT32 Timeout )
{	
  	while ( KCS_STATUS & KCS_IBF_FLAG ) 
    {
    	if ( --Timeout == 0 ) 
        {
    		return TRUE;
        }

		BS->Stall ( 1 );
  	}

  	return FALSE;
}

//
// ----------------------------------------------------------------------------
// Name
//      WaitTilObf
// 
// Abstract:
//      OBF per IPMI spec, check time out
//
// Arguments:
//      Timeout - milliseconds for time out
//
// Returns:
//      TRUE	time out
//      FALSE	not time out
//
// ----------------------------------------------------------------------------
//

UINT32  WaitTilObf ( UINT32 Timeout )
{
  	while ( !(KCS_STATUS & KCS_OBF_FLAG) )
    {
    	if ( --Timeout == 0 ) 
        {
    		return TRUE;
        }

		BS->Stall ( 1 );
  	}

  	return FALSE;
}

//
// ----------------------------------------------------------------------------
//
// Name
//      KCSMessageReady
//
// Abstract:
//      check kcs message is ready or not
//
// Parameters: 
//   
// Return Value:
//      TRUE	ready
//      FALSE	not ready
//
// ----------------------------------------------------------------------------
//

EFI_STATUS KCSMessageReady ( VOID )
{		 
	return KCS_SMS_MSG ? TRUE : FALSE;
}

//
// ----------------------------------------------------------------------------
//
// Name
//      CheckMessageReady
// Abstract:
//      Check message ready for specified target
//
// Parameters:
//      Target  - target that owns the message
//      Timeout - milliseconds for time out
//
// Return Value:
//      TRUE	message ready
//      FALSE	not ready
//
// ----------------------------------------------------------------------------
//

UINT32 CheckMessageReady ( UINT8 Target, 
                           UINT32 Timeout 
                         )
{		 
	UINT32 OverallTimeout;
	
	if (Target == BMC_SLAVE_ADDR) 
    {					
		SetMsTimeout (&OverallTimeout, Timeout);
		
		do 
        {		  
			if (CheckMsTimeout (OverallTimeout)) 
            {
				return TRUE;
			}			  
			Status = KCS_STATUS;
        } while ((Status & KCS_OBF_FLAG) == 0);
	
		return FALSE;
	}
	else 
    {				
		SetMsTimeout ( &OverallTimeout, Timeout);
	   
		while (!(KCS_SMS_MSG)) 
        {
			if (CheckMsTimeout (OverallTimeout)) 
            {					 
				return TRUE;
			}
		}
		return FALSE;
	}
}

//
// ----------------------------------------------------------------------------
//
// Name
//      KCSSendMessage
//
// Abstract:
//      KCS send message to BMC
//
// Parameters:
//      *MsgBuf	- message buffer to be sent
//  	Length	- message length
//
// Return Value:
//  	EFI_SUCCESS				The request message successfully sent to the BMC.
//	    EFI_IPMIERR_FAILURE		The request message failed to send to the BMC.
//  	EFI_IPMIERR_TIMEOUT		Time out error.
//  	EFI_IPMIERR_BMC_BUSY	BMC is busy.
//
// ----------------------------------------------------------------------------
//

EFI_STATUS KCSSendMessage ( UINT8 *MsgBuf, 
                            UINT32 Length 
                          )
{
	UINT32  i;
    CHAR8   Data;
	
	if ( WaitWhileIbf(KCS_TIMEOUT) )                    // BMC could be busy
    {					
		return EFI_IPMIERR_BMC_BUSY;
	}

	// Read status - if OBF == 1, read data byte to clear OBF
	if ( KCS_STATUS & KCS_OBF_FLAG ) 
    {
        Data = KCS_READ_BMC_DATA;
	}

	// Set BMC to Write state.
	KCS_WRITE_BMC_CMD ( KCS_WRITE_START );

    // Wait for IBF == 0
	if ( WaitWhileIbf ( KCS_TIMEOUT ) )					
    {
		return EFI_IPMIERR_TIMEOUT;
	}

	// Is BMC in Write State ?
	if ( (KCS_STATUS & KCS_STATE_MASK) != KCS_WRITE_STATE ) 
    {
		return EFI_IPMIERR_FAILURE;
	}

	// Clear OBF if set
	if ( KCS_STATUS & KCS_OBF_FLAG ) 
    {
        Data = KCS_READ_BMC_DATA;		// Clear by reading data
	}

    // Send message - 1 byte (last byte is handled after the loop.
	for (i = 0; i < Length-1; ++i) 					
    {
		// Output data byte
		KCS_WRITE_BMC_DATA ( MsgBuf[i] );

		// Wait for IBF == 0
		if ( WaitWhileIbf ( KCS_TIMEOUT ) )					
		{
			return EFI_IPMIERR_TIMEOUT;
		}

		// Is BMC in Write State ?
		if ( (KCS_STATUS & KCS_STATE_MASK) != KCS_WRITE_STATE ) 
		{
			return EFI_IPMIERR_FAILURE;
		}

		// Clear OBF if set
		if ( KCS_STATUS & KCS_OBF_FLAG ) 
		{
			Data = KCS_READ_BMC_DATA;		// Clear by reading data
		}
	}

	KCS_WRITE_BMC_CMD ( KCS_WRITE_END );	// Write-End command

    // Wait for IBF == 0
	if ( WaitWhileIbf ( KCS_TIMEOUT ) )					
    {
		return EFI_IPMIERR_TIMEOUT;
	}

	// Is BMC in Write State ?
	if ( (KCS_STATUS & KCS_STATE_MASK) != KCS_WRITE_STATE ) 
	{
		return EFI_IPMIERR_FAILURE;
	}

	// Clear OBF if set
	if ( KCS_STATUS & KCS_OBF_FLAG ) 
	{
		Data = KCS_READ_BMC_DATA;		// Clear by reading data
	}
	
	// Output last data byte
	KCS_WRITE_BMC_DATA ( MsgBuf[i] );

	return EFI_SUCCESS;
}

//
// ----------------------------------------------------------------------------
// Name
//      KCSReadBMCData
//
// Abstract:
//      KCS read data from BMC
//
// Parameters:
//      *MsgBuf		- message buffer to be returned
//  	BufSize		- size of message buffer
//      *NumRead	- actual number of bytes read from BMC
//
// Return Value:
//
//      EFI_SUCCESS				Successfully received response message from the BMC.
//      EFI_IPMIERR_FAILURE		Response data didn't successfully return.
//      EFI_BUFFER_TOO_SMALL	Buffer is too small.
//      EFI_IPMIERR_TIMEOUT		Time out error.
//      EFI_IPMIERR_BMC_BUSY	BMC is busy.
//
// ----------------------------------------------------------------------------
//

EFI_STATUS KCSReadBMCData ( UINT8 *MsgBuf, 
                            UINT32 BufSize, 
                            UINT32 *NumRead 
                          )
{					 
    // Read the message
	for ( *NumRead=0; *NumRead < BufSize; ++*NumRead )
    {
		// Wait for IBF == 0
		if ( WaitWhileIbf ( KCS_TIMEOUT ) )					
		{
			return EFI_IPMIERR_TIMEOUT;
		}

		// Read BMC status
		Status = KCS_STATUS & KCS_STATE_MASK;

		// Is BMC in Read State ?
		if ( Status != KCS_READ_STATE )
		{
			// Is BMC in idle state ?
			if ( Status != KCS_IDLE_STATE )
			{
				return EFI_IPMIERR_FAILURE;
			}

			// Wait for the OBF
			if ( WaitTilObf ( KCS_TIMEOUT ) )
			{
				return EFI_IPMIERR_TIMEOUT;
			}
			
			KCS_READ_BMC_DATA;		// Read dummy byte

			return EFI_SUCCESS;
		}

		// Wait for the OBF
		if ( WaitTilObf ( KCS_TIMEOUT ) )
		{
			return EFI_IPMIERR_TIMEOUT;
		}

		MsgBuf[*NumRead] = KCS_READ_BMC_DATA;		// Read data byte

		// Send the READ command to the BMC
	  	KCS_WRITE_BMC_DATA ( KCS_READ );
	}

	//
	// If we fell thru, the buffer is too small!
	return EFI_BUFFER_TOO_SMALL;		
}

//
// ----------------------------------------------------------------------------
//
// Name 
//      IsKCS
//
// Abstract:
//      Check if it is KCS interface
//
// Parameters:
//
// Return Value:
//      TRUE	is KCS interface
//      FALSE	is not KCS interface
//
// ----------------------------------------------------------------------------
//

BOOLEAN IsKCS ( VOID )
{
	if ( (inp(KCS_BMC_CMD_STATUS) == 0xFF) 
        && (inp(KCS_BMC_DATA) == 0xFF) )
    {
		return(FALSE);
	}
	else 
    {
		return(TRUE);
	}
}

// -------------------------------END------------------------------------------
