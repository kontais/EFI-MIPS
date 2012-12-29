;
; Copyright (c) 1999, 2000
; Intel Corporation.
; All rights reserved.
; 
; Redistribution and use in source and binary forms, with or without
; modification, are permitted provided that the following conditions
; are met:
; 
; 1. Redistributions of source code must retain the above copyright
;    notice, this list of conditions and the following disclaimer.
; 
; 2. Redistributions in binary form must reproduce the above copyright
;   notice, this list of conditions and the following disclaimer in the
;    documentation and/or other materials provided with the distributio
; 
; 3. All advertising materials mentioning features or use of this softw
;    must display the following acknowledgement:
; 
;    This product includes software developed by Intel Corporation and
;    its contributors.
; 
; 4. Neither the name of Intel Corporation or its contributors may be
;    used to endorse or promote products derived from this software
;    without specific prior written permission.
; 
; THIS SOFTWARE IS PROVIDED BY INTEL CORPORATION AND CONTRIBUTORS ``AS 
; AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
; IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PU
; ARE DISCLAIMED.  IN NO EVENT SHALL INTEL CORPORATION OR CONTRIBUTORS 
; LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
; CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
; SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSIN
; INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER 
; CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWIS
; ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED O
; THE POSSIBILITY OF SUCH DAMAGE.
; 
;

;
; Module Name:
;
;  mpx64asm.s
;
; Abstract:
;
;  MultiProcessor related routines for EM64T.


;------------------------------------------------------
;  UINT16 GetCurrentProcLocalID( void ) ;
;
;	Get current processor local ID
;
; Description:
;
;------------------------------------------------------

PUBLIC GetCurrentProcLocalID
_TEXT	SEGMENT
GetCurrentProcLocalID	PROC

	; save
	push	rbx
	push	rcx
	push	rdx

	; get initial APIC ID
	mov		rax, 1
	cpuid
	shr		ebx, 24
	mov		eax, ebx

	pop		rdx
	pop		rcx
	pop		rbx

    ret

GetCurrentProcLocalID	ENDP
_TEXT	ENDS


;------------------------------------------------------
; void BootRendezvous( void ) ;
;------------------------------------------------------

PUBLIC BootRendezvous
_TEXT	SEGMENT
BootRendezvous	PROC

	; return 0
	xor	rax, rax

	ret

BootRendezvous	ENDP
_TEXT	ENDS

;------------------------------------------------------
; UINT64 GetLocalID( void ) ;
;------------------------------------------------------
PUBLIC GetLocalID
_TEXT	SEGMENT
GetLocalID	PROC

	; return 0
	xor	rax, rax

	ret

GetLocalID	ENDP
_TEXT	ENDS


;------------------------------------------------------------------------------
;  UINT64
;  X64ReadMsr (
;    IN   UINT32  Index
;    )
;
;  luckily, rcx plays the role of this C function parameter as well as the index
;  of 'rdmsr'
;------------------------------------------------------------------------------
PUBLIC X64ReadMsr
_TEXT SEGMENT
X64ReadMsr PROC
    rdmsr
    sal     rdx, 32   ; shift high-order of MSR to high-bits of rdx (32 bits)
    or      rax, rdx  ; combine the high and low bits of MSR to rax
    ret               ; return rax as the value of msr
X64ReadMsr  ENDP
_TEXT ENDS


;------------------------------------------------------------------------------
;  VOID
;  X64WriteMsr (
;    IN   UINT32  Index,  // rcx
;    IN   UINT64  Value   // rdx
;    )
;------------------------------------------------------------------------------
PUBLIC X64WriteMsr
_TEXT SEGMENT
X64WriteMsr PROC
    mov     rax,  rdx   ; rdx = Value
    sar     rdx,  32    ; convert rdx to edx upper 32-bits    
    wrmsr               ; wrmsr[ecx] result = edx:eax
    ret
X64WriteMsr ENDP
_TEXT ENDS



END


