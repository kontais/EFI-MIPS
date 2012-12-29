//
// Copyright (c) 2000
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

.global MessageBuffer
.global JumpBuffer
.global	AddressBuffer
.global	ArgumentBuffer
.global	GPBuffer
.global	longjmp
.global BspLID


#define	MASK_0_1		0x0000000000000003	// mask bits 0 through 1 
#define MIN_STATE_SAVE_AREA_IIP_OFFSET 0x198
#define MIN_STATE_SAVE_AREA_IPSR_OFFSET 0x1A0
#define MIN_STATE_SAVE_AREA_GR1_OFFSET 0x8

//
//
//	Name:
//		AP_INITHandler
//
//	Description:
//		INIT event handler to be installed for the AP
//
//	Arguments:
//		r1: Physical address of INIT handler's Global pointer (GP) registered by MPP.
//		r2-r7: unspecified.
//		r8: Physical address of PAL_PROC entry point
//		r9: Physical address of SAL_PROC entry point
//		r10: GP value (phys address) for SAL
//		r11: INIT reason code.
//			  0=Received INIT signal on this processor for reasons other than 
//			  mc rendezvous and CrashDump switch assertion
//			  1=Received INIT signal on this processor during machine check rendezvous
//			  2=received INIT signal on this processor due to CrashDump switch assertion
//		r12: Return address to a location within SAL_INIT procedure
//		r13-r34: unspecified
//		r24: PMI vector???
//		r25: pointer to Min-State Save Area
//		r26: saved RSC
//		r27: saved B0
//		r28: saved B1
//		r29: saved Predicate regs
//		r30, r31: scrach regs, can be used
//		b0: unspecified.. (actually return address into PAL_INIT
//		b1: unspecified.
//
//	Returns: (expected state)
//		r1-7: unspecified
//		r8: 0 if SAL must return to interrupted context using PAL_MC_RESUME
//			-1 if SAL must warm boot the system
//		r9: GP (phys addr) for SAL
//		r10: 0 if return will be to the same context
//			 1 if return will be to a new context
//		r11-12: unspecified
//		r22: Pointer to a structure containing new values of registers in the Min-State Save Area;
//			 PAL_MC_RESUME procedure will restore the register values from this structure;
//			 This INIT handler must supply this parameter even if it does not change the register 
//			 values in the Min-State Save Area.
//		r23-31: unspecified
//		PSR: same as on entry from SAL_INIT except that PSR.mc may be either 0 or 1.
//		b0: unspecified..
//
		.align 16

		.global		AP_INITHandler	
        .proc       AP_INITHandler

AP_INITHandler:
		movl		r2=BspLID;;
		ld8			r2=[r2];;		// BSP's Lid				
		
		// Which AP?
		mov				r30=lid;;				// r28 = id bits[31-24], eid bits[23-16]
		cmp.eq			p14, p15 = r30, r2		// is this the BSP??
(p14)	br.cond.sptk	AP_INITDone;;		// Yes, then nothig to do!!!

		shr				r30=r30,16;;			// r28 = id bits[15-8], eid bits[7-0]
		dep				r30=r30,r30,16,8;;	// r28 = eid bits[23-16], id bits[15-8], eid bits[7-0]
		shr				r30=r30,8;;			// r28 = eid bits[15-8], id bits[7-0]
		// multiply that with 8 to actually use it as an index into JumpBuffer. 
		shl				r30=r30,3;;			// r28 = eid bits[18-11], id bits[10-3] 


		// Retrieve Message For AP

		movl		r31=MessageBuffer;;		// r31 = address of pointer to message buffer 
		ld8			r31=[r31];;				// r31 = message buffer
		add			r31=r31,r30;;				// r31 = address of mailbox for AP

		ld8			r7=[r31];;				// r26 = message for AP
		cmp.eq		p14, p15 = MESSAGE_STOP, r7;;	// Start message?
(p14)	br.sptk		HandlerMessageStop	// yes, process it
(p15)	br.cond.sptk AP_INITDone;;		// no, nothing to do!

HandlerMessageStop::

		// clear the Message.
		mov			r7=MESSAGE_NONE;;		// r26 = message none
		st8			[r31]=r7;;				// Done processing message

		// Set up for rfi to CleanupRoutine
		movl		r5=CleanupRoutine;;				// r26 = address of pointer to longjmp 
        
		// change the return iip address, in the MinState Save Area offset 0x198

		// change the iip in the Min-State Save Area
		movl		r22=MIN_STATE_SAVE_AREA_IIP_OFFSET;;					// Min-State Save Area is given in R17
		add			r22=r22, r17;;
		st8			[r22]=r5;;
		// change the GP in the Min-State Save Area
		movl		r22=MIN_STATE_SAVE_AREA_GR1_OFFSET;;					// Min-State Save Area is given in R17
		add			r22=r22, r17;;
		st8			[r22]=r1;;		// Current GP  would be good for our CleanupRoutine

		// this is to fix a BUG which is causing the return address to execute from the 
		// 2nd instruction of the 1st bundle of our new return address (sometimes)
		// make sure that the IPSR.ri (restart instruction) to point to slot 0 of our new
		// return address (in the Min-State Save Area)

		movl		r22=MIN_STATE_SAVE_AREA_IPSR_OFFSET;;	
		add			r22=r22, r17;;			// Min-State Save Area is given in R17
		ld8			r5 = [r22];;			// old ipsr value
		dep         r5 = r0, r5, 41, 2;;	// set the bits 41, 42 to 0 (to set slot 0)
		st8			[r22]=r5;;				// save the new ipsr in the MSSA.


		// prepare for return!!
AP_INITDone:
		mov			r8= 0;;					// go back to the interrupted context
		mov			r9= r10;;				// SAL GP 
		mov			r10= 1;;				// we have changed the return context..
		mov			r22=r17;; 				// Min-State Save Area
				
		mov			b0=r12;;				// b0 = return address into the SAL

		// make sure that the PSR.mc is on to get further INIT interrupts.

		br.cond.sptk.few b0;;

		.endp		AP_INITHandler

//

//
//
//	Name:
//		CleanupRoutine
//
//	Description:
//		This is the return context set by the init handler (to implement the STOP message)
// we jumped here from PAL_MC_Resume 
// all the register contents are user proc's that we just killed
// nothing usefull there
// all we have to do is set the parameters for longjmp and jmp..

		.align 16

		.global		CleanupRoutine	
        .proc       CleanupRoutine

CleanupRoutine:

		// Which AP?
		mov				r30=lid;;				// r28 = id bits[31-24], eid bits[23-16]
		shr				r30=r30,16;;			// r28 = id bits[15-8], eid bits[7-0]
		dep				r30=r30,r30,16,8;;	// r28 = eid bits[23-16], id bits[15-8], eid bits[7-0]
		shr				r30=r30,8;;			// r28 = eid bits[15-8], id bits[7-0]
		// multiply that with 8 to actually use it as an index into JumpBuffer. 
		shl				r30=r30,3;;			// r28 = eid bits[18-11], id bits[10-3] 

		// Which jump buffer?

		// Allocate two registers on the stack to prepare for longjmp
		alloc		r26=ar.pfs,2,0,0,0;;		// r26 = previous function state

		movl			r32=JumpBuffer;;		// r32 = address of pointer to jump buffer 
		ld8				r32=[r32];;			// r32 = jump buffer
		add				r32=r32,r30;;			// r32 = address of jump buffer for AP
		ld8				r32=[r32];;			// r32 = jump buffer for AP

		// Update r32 and r33 to prepare for longjmp
		mov			r33=1;;					// r33 = return value for longjmp

		// Set up for branch to longjmp
		movl		r5=longjmp;;				// r26 = address of pointer to longjmp 
		ld8			r5=[r5];;				// r5 = address of longjmp

		mov			b0=r5;;
		br.cond.sptk b0;;			// jump off to long jump...

		.endp		CleanupRoutine





