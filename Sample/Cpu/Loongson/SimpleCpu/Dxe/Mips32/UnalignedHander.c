/*++

Copyright (c) 2012, kontais                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:
  Unaligned.c

Abstract:

  Mips32 Unaligned lw/sw access.

--*/

#include "CpuDxe.h"
#include "Mips.h"
#include "Exception.h"
#include "MipsOpcode.h"

BOOLEAN
ExceptionInDelaySlot (
  IN EFI_SYSTEM_CONTEXT    SystemContext
  )
{
  if (SystemContext.SystemContextMips32->cause & CP0_CAUSE_BD) {
    return TRUE;
  }

  return FALSE;
}


UINT32 ExceptionGetEpc (
  IN EFI_SYSTEM_CONTEXT    SystemContext
  )
{
  if (ExceptionInDelaySlot (SystemContext)) {
    return SystemContext.SystemContextMips32->pc + 4;
  }

  return SystemContext.SystemContextMips32->pc;
}

EFI_STATUS
ExceptionGetNextEpc (
  IN EFI_SYSTEM_CONTEXT    SystemContext,
  OUT UINT32               *Pc
  )
{
  // Not in delay slot
  if (!ExceptionInDelaySlot (SystemContext)) {
    *Pc = SystemContext.SystemContextMips32->pc + 4;
    return EFI_SUCCESS;
  }

  // So bad, the inst in the delay slot
  return ExceptionExcuteBranch(SystemContext, Pc);
}

EFI_STATUS
ExceptionHandlerDoADESL (
  IN EFI_EXCEPTION_TYPE    InterruptType,
  IN EFI_SYSTEM_CONTEXT    SystemContext
  )
{
  UINT32     CurrentPc;
  InstFmt    Inst;
  UINT32     OpRegNum;
  UINT32     OpCode;
  UINT32     *Context;
  UINT32     Value, Backup;
  UINT32     Addr;
  EFI_STATUS Status;

  CurrentPc = ExceptionGetEpc(SystemContext);  
  Inst      = *(InstFmt *)CurrentPc;
  OpRegNum  = Inst.IType.rt;
  OpCode    = Inst.IType.op;
  Context   = (UINT32*)SystemContext.SystemContextMips32;
  Addr      = SystemContext.SystemContextMips32->badvaddr;
  Backup    = Context[OpRegNum];

  Status = UnalignedAccess (OpCode, Addr, &Context[OpRegNum]);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  Status = ExceptionGetNextEpc(SystemContext, &Value);
  if (EFI_ERROR(Status)) {
    Context[OpRegNum] = Backup; // if failed, fill back the context for excepton debug print
  } else {
    SystemContext.SystemContextMips32->pc = Value;
  }

  return Status;
}

VOID
ExceptionHandlerADESL (
  IN EFI_EXCEPTION_TYPE    InterruptType,
  IN EFI_SYSTEM_CONTEXT    SystemContext
  )
{
  EFI_STATUS Status;

  Status = ExceptionHandlerDoADESL (InterruptType, SystemContext);
  if (Status == EFI_SUCCESS) {
    return ;
  }

  //
  // Failed, Call default excption handler
  //
  ExceptionHandler (InterruptType, SystemContext);
}

