//
// Copyright (c) 1999, 2000
// Intel Corporation.
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
// 
// 1. Redistributions of source code must retain the above copyright notice,
//    this list of conditions and the following disclaimer.
// 
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
// 
// 3. All advertising materials mentioning features or use of this software must
//    display the following acknowledgement:
// 
//    This product includes software developed by Intel Corporation and its
//    contributors.
// 
// 4. Neither the name of Intel Corporation or its contributors may be used to
//    endorse or promote products derived from this software without specific
//    prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY INTEL CORPORATION AND CONTRIBUTORS ``AS IS'' AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED.  IN NO EVENT SHALL INTEL CORPORATION OR CONTRIBUTORS BE LIABLE FOR
// ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
// ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 

#include "common.inc"
#include "message.h"

.global setjmp
.global MessageBuffer
.global JumpBuffer
.global	AddressBuffer
.global	ArgumentBuffer
.global	GPBuffer
.global BspLID


//
//	Name:
//		BootRendezvous
//
//	Description:
//		Boot rendezvous code for AP, starts the user's proc if the message is START	
//
//	Arguments:
//		None
//
//	Returns:
//		None
//
//
		.align 16
		.global BootRendezvous
		.proc BootRendezvous

BootRendezvous:

	// save the return pointer,
		mov				r4=b0;;

BeginHandler::
		// Which AP?
		mov				r28=lid;;				// r28 = id bits[31-24], eid bits[23-16]
		shr				r28=r28,16;;			// r28 = id bits[15-8], eid bits[7-0]
		dep				r28=r28,r28,16,8;;	// r28 = eid bits[23-16], id bits[15-8], eid bits[7-0]
		shr				r28=r28,8;;			// r28 = eid bits[15-8], id bits[7-0] 
		shl				r28=r28,3;;			// r28 = eid bits[18-11], id bits[10-3] 

		// Retrieve Message For AP

		movl		r27=MessageBuffer;;		// r27 = address of pointer to message buffer 
		ld8			r27=[r27];;				// r27 = message buffer
		add			r27=r27,r28;;			// r27 = address of mailbox for AP
		ld8			r26=[r27];;				// r26 = message for AP

		cmp.eq		p14, p15 = MESSAGE_START, r26;;	// Start message?
(p14)	br.sptk		HandlerMessageStart;;	// yes, then begin processing it
(p15)	br.cond.sptk BootRendezvous_Done;;  // no?? then there is nothing to process, get out

HandlerMessageStart::
		// before Starting the user's procedure, we need to create a jump buffer.
		// and save our context, registers preserved for longjmp are r4,r5,r6,r7 and sp

		// Which jump buffer?
		movl			r32=JumpBuffer;;		// r32 = address of pointer to jump buffer 
		ld8				r32=[r32];;			// r32 = jump buffer
		add				r32=r32,r28;;			// r32 = address of jump buffer for AP
		ld8				r32=[r32];;			// r32 = jump buffer for AP

		// Initialize jump buffer

		movl			r31=setjmp;;			// r31 = address of pointer to setjmp
		ld8				r31=[r31];;			// r31 = address of setjmp
		mov				b0=r31;;				// b0 = address of setjmp
		br.call.sptk	b0=b0;; 				// Fill in jump buffer	

		// check here if we returned from setjmp or longjmp!!

		cmp.eq			p14, p15 = r0, r8;;	// Return from setjmp (r8==0)?

		// if return from longjmp, must return to SAL_RENDEZVOUS

   (p15)	br.cond.sptk BootRendezvous_Done;;  // return from longjmp, user proc killed

		// return from setjmp? Make sure the INIT interrupts are enabled

// (p14)	ssm				MASK(PSR_MC,1);;		// yes, Turn on INIT interrupts 

		// call the user proc here..

		// Setup argument passing
		movl		r26=ArgumentBuffer;;		// r26 = address of pointer to argument buffer
		ld8			r26=[r26];;				// r26 = address of pointer to argument buffer
		add			r26=r26,r28;;				// r26 = address of argument buffer for AP
		ld8			r32=[r26];;				// r32 = argument buffer for AP

		// Set up gp register
		movl		r26=GPBuffer;;			// r26 = address of pointer to gp buffer
		ld8			r26=[r26];;				// r26 = address buffer
		add			r26=r26,r28;;				// r26 = address of gp buffer for AP
		ld8			r1=[r26];;				// r1 = gp buffer for AP

		// Get user Proc address
		movl		r26=AddressBuffer;;		// r26 = address of pointer to address buffer
		ld8			r26=[r26];;				// r26 = address buffer
		add			r26=r26,r28;;			// r26 = address of address for AP
		ld8			r26=[r26];;				// r26 = start address for user proc for this AP

		mov			b0=r26;;				// b0 = address of user proc
		mov			r26=MESSAGE_NONE;;		// r26 = message none
		st8			[r27]=r26				// Done processing message

		br.call.sptk	b0=b0;; 				// call user proc	

		// if at all returned from the user proc normally, go back to SAL

		// how can I change the processor state to stopped at this time??
		// so that it won't give any STOP messages!!

BootRendezvous_Done:

		mov		b0=r4;;		// saved b0
		br.ret.sptk b0;;

		.endp BootRendezvous
