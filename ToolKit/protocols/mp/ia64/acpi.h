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

#ifndef _ACPI_H_
#define _ACPI_H_

#include <efi.h>


//
// Error codes
//
#define	ACPI_SUCCESS		0
#define	ACPI_NO_RSDPTR		1
#define	ACPI_SIGNATURE		2
#define	ACPI_CHECKSUM		3
#define	ACPI_NULL_PTR		4
#define	ACPI_PROCREC_ERR	5
#define	ACPI_2_ERR			6

//
// ACPI signatures
//
#define ACPI_FACP_SIG		0x50434146
#define ACPI_SPIC_SIG		0x43495053
#define ACPI_DSDT_SIG		0x54445344
#define ACPI_FACP_SIG		0x50434146
#define ACPI_FACS_SIG		0x53434146
#define ACPI_PSDT_SIG		0x54445350
#define ACPI_RSDT_SIG		0x54445352
#define ACPI_SSDT_SIG		0x54445353
#define ACPI_SBST_SIG		0x54534253

//
// SAPIC structure types
//
#define ACPI_PROC_LOCAL_TYPE	0
#define ACPI_IO_SAPIC_TYPE		1
#define ACPI_INT_OVERRIDE_TYPE	2
#define ACPI_INT_SOURCE_TYPE	3

//
// Processor local SAPIC structure bitmasks
//
#define ACPI_SAPIC_PROC_LOCAL_EN	0x0001
#define ACPI_SAPIC_PROC_LOCAL_RES	0x0002
#define ACPI_SAPIC_PROC_LOCAL_PRES	0x0004


#pragma pack(1)

//
// Structure definitions
//
typedef struct {
	UINT32	HexSignature ;
} ACPI_SIG ;

//
// RSD_PTR structure
//
typedef struct {
    UINT8		Signature[8] ;		/* "RSD PTR " */
    UINT8		Checksum ;			/* entire table must sum zero */
    UINT8		OEMId[6] ;			/* string that identifies OEM */
    UINT8		Reserved ;			/* must be zero */
    void		*RSDT ;				/* physical address of the RSDT table */
} RSD_PTR ;

//
// Generic description header
//
typedef struct {        
    UINT8		Signature[4];		/* unique signature */
    UINT32		Length;				/* length of the table */
    UINT8		Revision;			/* current rev is 1 */
    UINT8		Checksum;			/* entire table must sum zero */
    UINT8		OEMId[6];			/* string that identifies OEM */
    UINT8		OEMTableId[8];		/* manufacturer model ID */
    UINT32		OEMRevision;		/* OEM revsion of this table */
    UINT8		CreatorId[4];		/* string that identifies creator */
    UINT32		CreatorRevision;	/* creator rev of this table */
#ifdef EFI64
    UINT32		Reserved;			/* reserved in 64-bit tables */
#endif
} DESC_HEADER;

//
// Generic description header
//
typedef struct {
	DESC_HEADER	Header ;			/* header */
	UINTN		*Entry ;			/* array of physical address that point to other description */
} RSDT ;

//
// SAPIC description
//
typedef struct {
	DESC_HEADER Header ;			/* header */
	void		*InterruptBlock;	/* physical address of SAPIC interrupt block */
} SPIC ;

//
// SAPIC structure header
//
typedef struct {
	UINT8		Type ;				/* type */
	UINT8		Length ;			/* length of structure */	
} SAPIC_HEADER ;

//
// Processor local SAPIC structure
//
typedef struct {
	SAPIC_HEADER Header ;			/* header */
	UINT16		ACPIProcessorID ;	/* processor id */
	UINT16		Flags ;				/* local SAPIC flags */
	UINT8		LocalSAPIC_ID ;		/* processor's local SAPIC ID */
	UINT8		LocalSAPIC_EID ;	/* processor's local SAPIC EID */
} SAPIC_PROC_LOCAL ;

//
// Processor I/O SAPIC structure
//
typedef struct {
	SAPIC_HEADER Header ;			/* header */
	UINT16		Reserved ;			/* reserved */
	UINT32		SystemVectorBase ;	/* system interrupt vector where IO SAPIC INITI lines start */
	void		*LocalSAPIC_ID ;	/* physical address to access this SAPIC */
} SAPIC_IO ;

//
// Interrupt source override structure
//
typedef struct {
	SAPIC_HEADER Header ;			/* header */
	UINT8		Bus ;				/* bus */
	UINT8		Source ;			/* bus relative interrupt source */
	UINT32		GlobalSIV ;			/* global system intrerupt vector that interrupt source will trigger */
	UINT16		Flags ;				/* same as MPS INTI flags */
	UINT8		Reserved[6] ;		/* reserved */
} SAPIC_INT_OVERRIDE ;

//
// Platform interrupt sources structure
//
typedef struct {
	SAPIC_HEADER Header ;			/* header */
	UINT16		Flags ;				/* same as MPS INTI flags */
	UINT8		Type ;				/* interrupt type */
	UINT8		ProcessorID	;		/* processor ID of destination */
	UINT8		ProcessorEID ;		/* processor EID of destination */
	UINT8		IO_SAPICVector ;	/* value OS must use to program the vecor field of IO SAPIC */
	UINT8		Reserved[4] ;		/* reserved */
	UINT32		GlobalSIV ;			/* global system intrerupt vector that interrupt source will trigger */
} SAPIC_PLATFORM_INT ;

#pragma pack()

#pragma pack(1)
//
// ACPI 2.0 or 0.71 typedefs
//

typedef struct {
    UINT32      Signature;
    UINT32      Length;
    UINT8       Revision;
    UINT8       Checksum;
    UINT8       OemId[6];
    UINT8       OemTableId[8];
    UINT32      OemRevision;
    UINT32      CreatorId;
    UINT32      CreatorRevision;
} ACPI_DESCRITPTION_HEADER;

typedef struct {
    UINT32      Signature;
    UINT32      Length;
    UINT8       Revision;
    UINT8       Checksum;
    UINT8       OemId[6];
    UINT8       OemTableId[8];
    UINT32      OemRevision;
    UINT32      CreatorId;
    UINT32      CreatorRevision;
	UINT64		Entry[10];
} ACPI_DESCRITPTION_HEADER_64;

#define ACPI_RSDP_SIGNATURE_LOW  ' DSR'
#define ACPI_RSDP_SIGNATURE_HIGH ' RTP'

typedef struct {
    UINT32      SignatureLow;
    UINT32      SignatureHigh;
    UINT8       Checksum;
    UINT8       OemId[6];
    UINT8       Revision;       // 0 = 0.71 2 = ACPI 2.0
    UINT32      RsdtAddress;    // This is 64-bit in 0.71
    UINT32      Length;
    UINT64      XsdtAddress;
    UINT8       ExtendedChecksum;
    UINT8       Reserved[3];
} ACPI_RSDP20;

#define ACPI_RSDP_REV_71_10     0
#define ACPI_RSDP_REV_20        2

// APIC  0.71
#define ACPI_71_APIC_SIGNATURE 'CIPS'

typedef struct {
    ACPI_DESCRITPTION_HEADER    Header;
    UINT32                      Reserved;
    UINT64                      InterruptBlock;
} ACPI_71_MSAPIC_DT;

#define ACPI_71_LOCAL_SAPIC                 0

typedef struct {
    UINT8   Type;   // 0 - ACPI_71_LOCAL_SAPIC & 7 - ACPI_20_LOCAL_SAPIC
    UINT8   Length;
    UINT16  AcpiProcessorId;
    UINT16  Flags;
    UINT8   SapicId;
    UINT8   SapicEid;
} ACPI_LOCAL_SAPIC_ENTRY;
	
#define ACPI_APIC_FLAGS_ENABLE                  0x00000001
#define ACPI_APIC_FLAGS_PERFORMANCE_RESTRICTED  0x00000002
#define ACPI_APIC_FLAGS_PRESENT                 0x00000004 
#define ACPI_APIC_FLAGS_GOOD_PROCESSOR  (ACPI_APIC_FLAGS_ENABLE | ACPI_APIC_FLAGS_PRESENT)

//
// ACPI 2.0 version
//
#define ACPI_20_APIC_SIGNATURE 'CIPA'

typedef struct {
    UINT8   Type;   // 0 - ACPI_71_LOCAL_SAPIC & 7 - ACPI_20_LOCAL_SAPIC
    UINT8   Length;
    UINT8   AcpiProcessorId;
    UINT8   SapicId;
    UINT8   SapicEid;
	UINT8	Reserved[3];
    UINT32  Flags;
} ACPI_20_LOCAL_SAPIC_ENTRY;

typedef struct {
    ACPI_DESCRITPTION_HEADER    Header;
    UINT32                      LocalApicAddress;
    UINT32                      Flags;
} ACPI_20_MAPIC_DT;

#define ACPI_20_LOCAL_SAPIC                     7

#pragma pack()


//
// Function prototypes
//
UINT32
ParseACPITable(
	VOID
	) ;

EFI_STATUS
ParseAcpi2_0Table (
	VOID
	);

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

#endif /* !_ACPI_H_ */
