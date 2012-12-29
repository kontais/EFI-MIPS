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

//
//	Name:
//		GetCurrentProcLocalID	
//
//	Description:
//		Gets the local id of the current processor	
//
//	Arguments:
//		None
//
//	Returns:
//		in0	- local id	
//
//	Notes:
//
//
		.global GetCurrentProcLocalID 
		.proc GetCurrentProcLocalID 

GetCurrentProcLocalID:

		NESTED_SETUP( 0,2,0,0 )

		mov			r2=lid						// r2 = id bits[31-24], eid bits[23-16]
		;;
//		extr.u		r8=r2,16,16					// r8 = local id
//		;;
		shr			r2=r2,16					// r2 = id bits[15-8], eid bits[7-0]
		;;
		dep			r2=r2,r2,16,8				// r2 = eid bits[23-16], id bits[15-8], eid bits[7-0]
		;;
		shr			r8=r2,8						// r8 = eid bits[15-8], id bits[7-0] 
		;;

		NESTED_RETURN

		.endp GetCurrentProcLocalID 



//
//	Name:
//		GetLocalID	
//
//	Description:
//		Gets the local id of the current processor	
//
//	Arguments:
//		None
//
//	Returns:
//		in0	- local id	
//
//	Notes:
//
//

		.global GetLocalID 
		.proc GetLocalID 

GetLocalID:

		NESTED_SETUP( 0,2,0,0 )

		mov			r8=lid						// r8 = local id 
		;;

		NESTED_RETURN

		.endp GetLocalID 
