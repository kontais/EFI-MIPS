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
*   $Log: kcs.h,v $
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

#ifndef _KCS_H
#define _KCS_H

#include "efi.h"
#include "efilib.h"
#include "efilibc.h"

#define KCS_DEFAULT_BASE_ADD	0xCA2					// KCS default base address
#define KCS_BASE_ADDRESS		KCSAddress				// KCS base address
#define KCS_CMD_STATUS_ADDRESS	(KCS_BASE_ADDRESS+1)	// KCS command status address
#define KCS_DATA_ADDRESS		(KCS_BASE_ADDRESS)		// KCS data byte address

//#define KCS_BMC_CMD_STATUS		0xCA3
//#define KCS_BMC_DATA			0xCA2

#define KCS_BMC_CMD_STATUS		KCS_CMD_STATUS_ADDRESS
#define KCS_BMC_DATA			KCS_DATA_ADDRESS

#define KCS_STATE_MASK			0xC0
#define KCS_IDLE_STATE			0x00
#define KCS_READ_STATE			0x40
#define KCS_WRITE_STATE			0x80
#define KCS_ERROR_STATE			0xC0

#define KCS_OBF_FLAG			0x01
#define KCS_IBF_FLAG			0x02
#define KCS_SMS_MSG_FLAG		0x04

#define KCS_OBF					((UINT8)(inp(KCS_BMC_CMD_STATUS) & KCS_OBF_FLAG))
#define KCS_READ_BMC_DATA		((UINT8)(inp(KCS_BMC_DATA)))
#define KCS_WRITE_BMC_DATA(x)	(outp((unsigned)KCS_BMC_DATA,(unsigned)x))
#define KCS_IBF					((UINT8)(inp(KCS_BMC_CMD_STATUS) & KCS_IBF_FLAG))
#define KCS_WRITE_BMC_CMD(x)	(outp((unsigned)KCS_BMC_CMD_STATUS,(unsigned)x))
#define KCS_SMS_MSG				((UINT8)(inp(KCS_BMC_CMD_STATUS) & KCS_SMS_MSG_FLAG))
#define KCS_STATUS				((UINT8)(inp(KCS_BMC_CMD_STATUS)))
#define KCS_STATE				((UINT8)(inp(KCS_BMC_CMD_STATUS) & KCS_STATE_MASK))

// SMS Transfer Stream Control Codes
                                
#define KCS_GET_STATUS_ABORT	0x60
#define KCS_WRITE_START			0x61
#define KCS_WRITE_END			0x62
#define KCS_READ				0x68

#define BMC_SLAVE_ADDR			0x20                                 
#define MAX_KCS_LENGTH			35

#define KCS_TIMEOUT				(20 * 1000 * 1000)		// in  uSec

#define WAITBMCSTATECOUNT		100

#define OEM_FLAGS				0x30

//
// Global variables
//
extern UINT16 KCSAddress;

//
// Function prototypes
//

EFI_STATUS KCSSendMessage (	UINT8	*MsgBuf, 
                            UINT32	Length
                          );
	
EFI_STATUS KCSReadBMCData (	UINT8	*MsgBuf,
                            UINT32	SizeOfBuf,
                            UINT32	*NumRead
                          );

EFI_STATUS KCSMessageReady ( VOID );

BOOLEAN IsKCS( VOID	);

#endif
