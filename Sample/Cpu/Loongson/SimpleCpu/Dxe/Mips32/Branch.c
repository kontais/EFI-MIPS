/*	$OpenBSD: trap.c,v 1.77 2011/11/16 20:50:19 deraadt Exp $	*/

/*
 * Copyright (c) 1988 University of Utah.
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * the Systems Programming Group of the University of Utah Computer
 * Science Department and Ralph Campbell.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * from: Utah Hdr: trap.c 1.32 91/04/06
 *
 *	from: @(#)trap.c	8.5 (Berkeley) 1/11/94
 */



#include "CpuDxe.h"
#include "Mips.h"
#include "Exception.h"
#include "MipsOpcode.h"

/*
 * Return the resulting PC as if the branch was executed.
 * Update Context ra if branch and link
 */
UINT32
ExceptionExcuteBranch (
   IN EFI_SYSTEM_CONTEXT    SystemContext,
   OUT UINT32               *Pc
  )
{
  UINT32   BranchPc;
  InstFmt  BranchInst;
  UINT32   *Context;
  UINT32   NextPc;

#define  GetBranchDest(InstPtr, Inst) \
      (InstPtr + 4 + ((short)Inst.IType.imm << 2))

  Context  = (UINT32*)SystemContext.SystemContextMips32;
  BranchPc = SystemContext.SystemContextMips32->pc;
  if (BranchPc & 3) {
    return EFI_UNSUPPORTED;
  }

  BranchInst = *(InstFmt*) BranchPc; 

  switch (BranchInst.JType.op) {
  case OP_SPECIAL:
    switch (BranchInst.RType.func) {
    case OP_JALR:
      Context[BranchInst.RType.rd] = BranchPc + 8;
    case OP_JR:
      NextPc = Context[BranchInst.RType.rs];
      break;
    default:
      return EFI_UNSUPPORTED;
    }
    break;
  case OP_BCOND:
    switch (BranchInst.IType.rt) {
    case OP_BLTZ:
    case OP_BLTZL:
      if ((int)Context[BranchInst.IType.rs] < 0) {
        NextPc = GetBranchDest(BranchPc, BranchInst);
      } else {
        NextPc = BranchPc + 8;
      }
      break;

    case OP_BGEZ:
    case OP_BGEZL:
      if ((int)Context[BranchInst.IType.rs] >= 0) {
        NextPc = GetBranchDest(BranchPc, BranchInst);
      } else {
        NextPc = BranchPc + 8;
      }
      break;

    case OP_BLTZAL:
    case OP_BLTZALL:
      Context[RA] = BranchPc + 8;
      if ((int)Context[BranchInst.IType.rs] < 0) {
        NextPc = GetBranchDest(BranchPc, BranchInst);
      } else {
        NextPc = BranchPc + 8;
      }
      break;

    case OP_BGEZAL:
    case OP_BGEZALL:
      Context[RA] = BranchPc + 8;
      if ((int)Context[BranchInst.IType.rs] >= 0) {
        NextPc = GetBranchDest(BranchPc, BranchInst);
      } else {
        NextPc = BranchPc + 8;
      }
      break;
    default:
      return EFI_UNSUPPORTED;
    }
    break;
  case OP_JAL:
    Context[RA] = BranchPc + 8;
  case OP_J:
    NextPc = BranchPc + 4; // delay slot Inst
    NextPc >>= 28;
    NextPc <<= 28;
    NextPc |= (BranchInst.JType.target << 2);
    break;
  case OP_BEQ:
  case OP_BEQL:
    if (Context[BranchInst.IType.rs] == Context[BranchInst.IType.rt]) {
      NextPc = GetBranchDest(BranchPc, BranchInst);
    } else {
      NextPc = BranchPc + 8;
    }
    break;
  case OP_BNE:
  case OP_BNEL:
    if (Context[BranchInst.IType.rs] != Context[BranchInst.IType.rt]) {
      NextPc = GetBranchDest(BranchPc, BranchInst);
    } else {
      NextPc = BranchPc + 8;
    }
    break;
  case OP_BLEZ:
  case OP_BLEZL:
    if ((int)Context[BranchInst.IType.rs] <= 0) {
      NextPc = GetBranchDest(BranchPc, BranchInst);
    } else {
      NextPc = BranchPc + 8;
    }
    break;
  case OP_BGTZ:
  case OP_BGTZL:
    if ((int)Context[BranchInst.IType.rs] > 0) {
      NextPc = GetBranchDest(BranchPc, BranchInst);
    } else {
      NextPc = BranchPc + 8;
    }
    break;
#if 0
  case OP_COP1:
    switch (BranchInst.RType.rs) {
    case OP_BC:
      cc = (BranchInst.RType.rt & COPz_BC_CC_MASK) >>
          COPz_BC_CC_SHIFT;
      if ((BranchInst.RType.rt & COPz_BC_TF_MASK) == COPz_BC_TRUE)
        condition = fsr & FPCSR_CONDVAL(cc);
      else
        condition = !(fsr & FPCSR_CONDVAL(cc));
      if (condition)
        NextPc = GetBranchDest(BranchPc, BranchInst);
      else
        NextPc = BranchPc + 8;
      break;
    default:
      return EFI_UNSUPPORTED;
    }
    break;
#endif
  default:
    return EFI_UNSUPPORTED;
  }

  *Pc = NextPc;
  return EFI_SUCCESS;
}

