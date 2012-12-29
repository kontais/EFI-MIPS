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

#define NESTED_SETUP(i,l,o,r) \
         alloc loc1=ar##.##pfs,i,l,o,r ;\
         mov loc0=b0

#define NESTED_RETURN \
         mov b0=loc0 ;\
         mov ar##.##pfs=loc1 ;;\
         br##.##ret##.##dpnt  b0;;

//
// Macros to disbale/enable interrupts
//

#define	DISABLE_INTERRUPTS(reg)		\
		mov		reg=psr;;			\
		rsm		MASK(PSR_I, 1)

#define	ENABLE_INTERRUPTS(reg)		\
		tbit.nz	p6,p7=reg, PSR_I;;	\
(p6)	ssm		MASK(PSR_I,1);;		\
(p7)	rsm		MASK(PSR_I,1);;

#define MASK(bp,value)  (value << bp)

#define PSR_I       14
#define PSR_MC      35

//
// Human readable register names
//

#define	_iva				cr2			// interrupt vector address = cr2
#define	_ipsr			cr16		// interrupt processor status register = cr16
#define	_iip				cr19		// interrupt instruction bundle pointer = cr19
#define	lid				cr64		// local id = cr64
#define	ivr				cr65		// external interrupt vector register = cr65
#define	eoi				cr67		// end of external interrupt = cr67
#define brp				rp			// branch return pointer = rp
#define irr0			cr68		// external interrupt request register 0
#define irr1			cr69		// external interrupt request register 1
#define irr2			cr70		// external interrupt request register 2
#define irr3			cr71		// external interrupt request register 3
