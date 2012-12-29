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
*   $Log: smic.h,v $
*   Revision 1.1.1.1  2006/05/30 06:16:12  hhzhou
*   no message
*
 * 
 *    Rev 1.3   14 Jul 2000 10:52:06   eamaya
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

#ifndef _SMIC_H
#define _SMIC_H

#include "efi.h"
#include "efilib.h"
#include "efilibc.h"


#ifdef CLOCKS_PER_SEC
#undef CLOCKS_PER_SEC
#endif

#define CLOCKS_PER_SEC  1000


#define SMIC_RX_DATA_READY	0x80	// Indicates BMC has data that can be delivered in response to a read control 
#define SMIC_TX_DATA_READY	0x40	// Indicates BMC is ready to accept a WRITE control code and data 
#define SMIC_SMI			0x10	// BMC has asserted the SMI signal and has internal SMI event flags set 
#define SMIC_SMM_I2C_READY	0x08	// Message for SMM received by BMC and ready to read. 
#define SMIC_SMS_I2C_READY	0x04	// Message for SMS received by BMC and ready to read 
#define SMIC_BUSY			0x01	// Provides arbitration for smic mailbox register access. 

#define SMIC_DATA			0x0CA9	// SMIC Data Reg. address  
#define SMIC_CNTL			0x0CAA	// SMIC Control Reg. address  
#define SMIC_FLAG			0x0CAB	// SMIC Flags Reg. address  

#define SMIC_TIMEOUT	100

#define outp(_Port, _DataByte)	(UINT8)WritePort(GlobalIoFncs,  IO_UINT8,  (UINTN)_Port, (UINTN)_DataByte)
#define inp(_Port)				(UINT8)ReadPort(GlobalIoFncs,   IO_UINT8,  (UINTN)_Port)
#define outpw(_Port, _DataByte)	(UINT16)WritePort(GlobalIoFncs, IO_UINT16, (UINTN)_Port, (UINTN)_DataByte)
#define inpw(_Port)				(UINT16)ReadPort(GlobalIoFncs,  IO_UINT16, (UINTN)_Port)
#define outpd(_Port, _DataByte)	(UINT32)WritePort(GlobalIoFncs, IO_UINT32, (UINTN)_Port, (UINTN)_DataByte)
#define inpd(_Port)				(UINT32)ReadPort(GlobalIoFncs,  IO_UINT32, (UINTN)_Port)

#define SMIC_DATA_READY		((inp(SMIC_FLAG) & 0x80)      == 0x80) // data ready bit set 
#define SMIC_Cbuf_READY		((inp(SMIC_FLAG) & 0x41)      == 0x40) // DMI2C TX bufr NOT FULL 
#define SMIC_Cbuf_EMPTY		((inp(SMIC_FLAG) & 0x21)      == 0x20) // DMI2C TX bufr EMPTY 
#define SMIC_SMI_READY		((inp(SMIC_FLAG) & 0x10)      == 0x10)
#define SMIC_SMM_MSG_READY	((inp(SMIC_FLAG) & 0x08)      == 0x08)
#define SMIC_SMS_MSG_READY	((inp(SMIC_FLAG) & 0x04)      == 0x04)
#define SMIC_READY			((inp(SMIC_FLAG) & SMIC_BUSY) == 0x00) // Busy Bit clear 

// SMS Transfer Stream Control Codes 
#define SMIC_CC_SMS_GET_STATUS		0x40	// temp 
#define SMIC_CC_SMS_WR_START		0x41
#define SMIC_CC_SMS_WR_NEXT			0x42
#define SMIC_CC_SMS_WR_END			0x43
#define SMIC_CC_SMS_RD_START		0x44
#define SMIC_CC_SMS_RD_NEXT			0x45
#define SMIC_CC_SMS_RD_END			0x46
#define SMIC_CC_SMS_ABORT			0x47

// SMS Transfer Stream Status Codes 
#define SMIC_SC_SMS_READY			0xC0
#define SMIC_SC_SMS_WR_START		0xC1
#define SMIC_SC_SMS_WR_NEXT			0xC2
#define SMIC_SC_SMS_WR_END			0xC3
#define SMIC_SC_SMS_RD_START		0xC4
#define SMIC_SC_SMS_RD_NEXT			0xC5
#define SMIC_SC_SMS_RD_END			0xC6

#define MAX_SMIC_LENGTH				67

typedef struct 
{
	UINT8	DataReg;
	UINT8	ControlReg;
} SMIC_RegsStruct;


// function prototypes

VOID SleepMs ( UINT32	Milliseconds );
	
UINT32 CheckMsTimeout (	UINT32	Goal );
	
VOID SetMsTimeout (	UINT32	*Goal,
                    UINT32 TimeoutInMs
	              );
	
UINT32 Msec ( VOID );

EFI_STATUS SMICSendMessage ( UINT8	*MsgBuf,
                             UINT32	Length
                           );

EFI_STATUS SMICReadBMCData ( UINT8	*MsgBuf,
                             UINT32	SizeOfBuf,
                             UINT32	*NumRead
                           );

UINT32 SmicOut ( SMIC_RegsStruct    *GetcRegs,
                 UINT8		    	SmicData, 
                 UINT8 			    SmicControl
	           );

UINT32 SMICMessageReady ( VOID );

#endif
