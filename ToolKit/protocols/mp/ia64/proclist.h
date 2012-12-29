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

#ifndef _PROCLIST_H_
#define _PROCLIST_H_

#include <efi.h>

//
// Definitions
//
#define	ACPI_KEY_TYPE	0
#define	LID_KEY_TYPE	1
#define	STOPPED			0
#define STARTED			1
#define PROCESSOR_AP	0
#define PROCESSOR_BSP	1
#define PROCESSOR_MASK	0x01	

//
// Processor information record
//
typedef struct ProcRecStruct {
	struct ProcRecStruct	*Next ;				/* next processor record */
	UINT16					ACPIProcessorID ;	/* ACPI processor id */
	UINT16					LocalID ;			/* local ID */
	UINT8					Flags ;				/* flags */
	UINT8					State ;				/* processor state */
} PROCREC ;

//
// Macros
//
#define	IS_BSP(X)	( ( ( X )->Flags & PROCESSOR_MASK ) ? 1 : 0 )

//
// Function prototypes
//
extern UINT32
CreateProcListRecords(
	VOID
	) ;

extern VOID
DeleteProcListRecords(
	VOID
	) ;

extern BOOLEAN
NewProcListRecord(
	UINT16	ACPIProcessorID, 
	UINT8	LocalSAPIC_ID,
	UINT8	LocalSAPIC_EID
	) ;

extern PROCREC
*GetNextProcListRecord(
	PROCREC		*ProcRec
	) ;

extern PROCREC
*FindProcListRecord(
	UINT16		Key,
	UINT8		KeyType
	) ;

#ifdef LIBMP_DEBUG
extern VOID
DisplayProcListRecords(
	VOID
	) ;
#endif

#endif /* !_PROCLIST_H_ */
