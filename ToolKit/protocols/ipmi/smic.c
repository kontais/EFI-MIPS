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
*   $Log: smic.c,v $
*   Revision 1.1.1.1  2006/05/30 06:16:12  hhzhou
*   no message
*
 * 
 *    Rev 1.3   14 Jul 2000 10:52:04   eamaya
 * - Updated KCS protocol per addenda to IPMI spec.
 *   Note that this update is not backward compatible
 *   to previous KCS protocol.
 * - Added ability to unload the driver.
 * 
 *    Rev 1.2   14 Jun 2000 12:48:30   eamaya
 * - Updated to work with SMBIOS table to look up KCS
 * address in Record 38 and override default KCS address CA2.
 * - Updated revision from 1.0 to 1.1
*
******************************************************************************/

#include "efiipmi.h"
#include "smic.h"


//
// ----------------------------------------------------------------------------
//
// Name
//      WaitForDataReady
//
// Abstract:
//      wait for data ready
//
// Parameters:
//          
// Return Value:
//      TRUE	- data is not ready and time out
//      FALSE	- data is ready
//
// ----------------------------------------------------------------------------
//

UINT32 WaitForDataReady ( VOID )
{
	UINT32 OverallTimeout;  

	SetMsTimeout ( &OverallTimeout, SMIC_TIMEOUT );
	DEBUG ((D_LOAD, "overall Timeout %d\n\n", OverallTimeout));

	while (!SMIC_DATA_READY) 
    {
		DEBUG ((D_LOAD, "smic data not ready\n\n"));
		if (CheckMsTimeout(OverallTimeout)) 
        {
			return TRUE;
		}
	}
	return FALSE;
} 

//
// ----------------------------------------------------------------------------
//
// Name
//      WaitForBusy
//
// Abstract:
//      wait for busy
//
// Parameters:
//
// Return Value:
//      TRUE	- data is busy and time out
//      FALSE	- data is not busy
//
// ----------------------------------------------------------------------------
//

UINT32 WaitForBusy ( VOID )
{
	UINT32 OverallTimeout; 
	UINT8 Temp;

	SetMsTimeout(&OverallTimeout, SMIC_TIMEOUT);
	
	DEBUG ((D_LOAD, "set ms Timeout: %l\n", OverallTimeout));

	Temp = inp(SMIC_FLAG);
	DEBUG ((D_LOAD, "inp(SMIC_FLAG), %d\n", Temp));
	
	while (!SMIC_READY) 
    {
		if (CheckMsTimeout(OverallTimeout)) 
        {
			DEBUG ((D_LOAD, "check ms Timeout: %ld\n", OverallTimeout));
			return TRUE;
		}
	}
	
	DEBUG ((D_LOAD, "not busy\n"));
	return FALSE;
}

//
// ----------------------------------------------------------------------------
//
// Name
//      SmicOut
//
// Abstract:
//      SMIC out
//
// Parameters:
//      *GetcRegs		- SMIC register structure
//      SmicData		- SMIC data
//      SmicControl		- SMIC control code
// 
// Return Value:
//      TRUE	- data is not out and time out
//      FALSE	- data is out
//
// ----------------------------------------------------------------------------
//

UINT32 SmicOut ( SMIC_RegsStruct	*GetcRegs, 
	             UINT8  			SmicData, 
                 UINT8 	    		SmicControl
               )
{
	UINT32 OverallTimeout;
	UINT8 FlagReg;
	
	DEBUG ((D_LOAD, "SmicOut start\n"));
	
	if ( WaitForBusy() ) 
    {
        return TRUE;
    }
	
	// wait for transmit ready bit to clear

	SetMsTimeout(&OverallTimeout, SMIC_TIMEOUT);
	
	while(!SMIC_Cbuf_READY)	                // wait for control buffer not full
    {				 
		if(CheckMsTimeout(OverallTimeout)) 
        {
			return TRUE;
		}
	}
											  
	outp(SMIC_DATA, SmicData);		 		// send data UINT8 passed to smic
	outp(SMIC_CNTL, SmicControl);	 		// output control UINT8
	FlagReg = inp(SMIC_FLAG);
	outp(SMIC_FLAG, FlagReg | 0x01);		// set the busy flag

	if ( WaitForBusy() ) 
    {
        return TRUE;
    }

	GetcRegs->DataReg = inp(SMIC_DATA);	  // 2 UINT8 structure
	GetcRegs->ControlReg = inp(SMIC_CNTL);  // containing data and control

	return FALSE;
} 

//
// ----------------------------------------------------------------------------
// Name
//      CheckMsTimeout
//
// Abstract:
//      check time out in milliseconds
//
// Parameters:
//      Goal	- milliscond value to be time out
//
// Return Value:
//      TRUE	- time out
//      FALSE	- not time out
//
// ----------------------------------------------------------------------------
//

UINT32 CheckMsTimeout ( UINT32 Goal )
{
	if ( Msec() >= Goal)
    {
		return TRUE;
    }
	else
    {
		return FALSE;
    }
}

//
// ----------------------------------------------------------------------------
// Name
//      SetMsTimeout
//
// Abstract:
//      Set time out value
//
// Parameters:
//      *Goal		- time that to be time out
//      TimeoutInMs	- time out value
//
// Return Value:
//      
//	
// ----------------------------------------------------------------------------
//

VOID SetMsTimeout (	UINT32	*Goal,
                    UINT32	TimeoutInMs
                  )
{
	*Goal = Msec() + TimeoutInMs;
}

//
// ----------------------------------------------------------------------------
//
// Name
//      Msec
// 
// Abstract:
//      get time in milliseconds
//
// Parameters:
//
// Return Value:
//      time in milliseconds	
//
// ----------------------------------------------------------------------------
//

UINT32 Msec ( VOID )
{
	UINT32 Msecond = 0;

	EFI_TIME Time;
	EFI_TIME_CAPABILITIES cap;
	RT->GetTime(&Time, &cap);
	// Fix me, seems the Time.Nanosecond does not work
	Msecond = (Time.Day * 24 * 60 * 60 + Time.Hour * 60 * 60 + Time.Minute * 60 + Time.Second) * 1000 + Time.Nanosecond / 100000;
	
/*
	DEBUG ((D_LOAD, "Time.Day: %d\n", Time.Day));
	DEBUG ((D_LOAD, "Time.Hour: %d\n", Time.Hour));
	DEBUG ((D_LOAD, "Time.Minute: %d\n", Time.Minute));
	DEBUG ((D_LOAD, "Time.Second: %d\n", Time.Second));
	DEBUG ((D_LOAD, "Time.Nanosecond: %ld\n", Time.Nanosecond));
	DEBUG ((D_LOAD, "Msecond: %ld\n", Msecond));

*/
	return Msecond;
}

//
// ----------------------------------------------------------------------------
//
// Name
//      SMICSendMessage
//
// Abstract:
//      SMIC send message to BMC
//
// Parameters:
//      *MsgBuf	- message buffer to be sent to BMC
//      Length	- message buffer length
//
// Return Value:
//      EFI_SUCCESS				The request message is successfully sent to the BMC.
//  	EFI_IPMIERR_FAILURE		The request message is failed to send to the BMC.
//
// ----------------------------------------------------------------------------
//

EFI_STATUS SMICSendMessage ( UINT8	*MsgBuf,
                             UINT32	Length
                           )
{
	SMIC_RegsStruct Netcom;		 
	UINT32 i;

	DEBUG ((D_LOAD, "SMICSendMessage started\n"));

/*
	DEBUG ((D_LOAD, "0x%x\n", MsgBuf[0]));
	DEBUG ((D_LOAD, "0x%x\n", MsgBuf[1]));
*/
	
	if (SmicOut(&Netcom, *MsgBuf++, SMIC_CC_SMS_WR_START)) 
    {
		DEBUG ((D_LOAD, "SmicOut error\n"));
		return EFI_IPMIERR_FAILURE;
	}
			
	if (Netcom.ControlReg != SMIC_SC_SMS_WR_START) 
    {
		return EFI_IPMIERR_FAILURE;
	}
	
	// send the rest
	for (i = 0; i < Length - 2; i++) 
    {
		if (SmicOut(&Netcom, *MsgBuf++, SMIC_CC_SMS_WR_NEXT)) 
        {
			return EFI_IPMIERR_FAILURE;
		}

		if (Netcom.ControlReg != SMIC_SC_SMS_WR_NEXT) 
        {
			return EFI_IPMIERR_FAILURE;
		}
	}

	if(SmicOut(&Netcom, *MsgBuf, SMIC_CC_SMS_WR_END)) 
    {
		return EFI_IPMIERR_FAILURE;
	}

	if (Netcom.ControlReg != SMIC_SC_SMS_WR_END) 
    {
		return EFI_IPMIERR_FAILURE;
	}

	return EFI_SUCCESS;
}

//
// ----------------------------------------------------------------------------
//
// Name
//      SMICReadBMCData
// Abstract:
//      SMIC read data from BMC
//
// Parameters:
//  	*MsgBuf		- message buffer to be returned
//  	SizeOfBuf	- size of message buffer
//  	*NumRead		- actual number of bytes read from BMC
//
// Return Value:
//      EFI_SUCCESS				Successfully get the response message from the BMC.
//  	EFI_IPMIERR_FAILURE		Response data didn't successfully return.
//  	EFI_BUFFER_TOO_SMALL	The size of returned buffer is larger than the size of allocated buffer.
//
// ----------------------------------------------------------------------------
//

EFI_STATUS SMICReadBMCData ( UINT8	*MsgBuf,
                             UINT32	SizeOfBuf,
                             UINT32	*NumRead
                           )
{
	SMIC_RegsStruct Netcom;		 
	UINT32 Count = 0;

	if (WaitForDataReady()) 
    {
		return EFI_IPMIERR_FAILURE;
	}

	if (SmicOut(&Netcom, 0, SMIC_CC_SMS_RD_START)) 
    {
		return EFI_IPMIERR_FAILURE;
	}

	if (Netcom.ControlReg != SMIC_SC_SMS_RD_START)	
    {
		return EFI_IPMIERR_FAILURE;
	}
	else 
    {
		*MsgBuf++ = Netcom.DataReg;
	}

	if ( WaitForDataReady() ) 
    {
        return EFI_IPMIERR_FAILURE;
    }

	for (Count = 1; Count < MAX_SMIC_LENGTH; Count++)		// get the data
    {
		if (Count>SizeOfBuf) 
        {
			return EFI_BUFFER_TOO_SMALL;
		}
		
		if (SmicOut(&Netcom, 0, SMIC_CC_SMS_RD_NEXT)) 
        {
			return EFI_IPMIERR_FAILURE;
		}
		
		if (Netcom.ControlReg != SMIC_SC_SMS_RD_NEXT) 
        {
			break;
		}
		else 
        {
			*MsgBuf++ = Netcom.DataReg;
		}
	}

	if (Netcom.ControlReg != SMIC_SC_SMS_RD_END) 
    {
		return EFI_IPMIERR_FAILURE;
	}
	else 
    {
		*MsgBuf++ = Netcom.DataReg;
	}

	if (SmicOut(&Netcom, 0, SMIC_CC_SMS_RD_END)) 
    {
		return EFI_IPMIERR_FAILURE;
	}

	if ((Netcom.ControlReg != SMIC_SC_SMS_READY) 
        || (Netcom.DataReg != 0) || (Count == 0))
    {
		return EFI_IPMIERR_FAILURE;
	}
	
	*NumRead = Count;
	return EFI_SUCCESS;
}

//
// ----------------------------------------------------------------------------
// Name
//      SMICMessageReady
// 
// Abstract:
//      check SMIC message is ready or not
//
// Parameters:
//
// 
// Return Value:
//      1	ready
//      0	not ready
//
// ----------------------------------------------------------------------------
//

UINT32 SMICMessageReady ( VOID	)
{		 
	return SMIC_DATA_READY ? 1 : 0;
}

// -------------------------------END------------------------------------------
