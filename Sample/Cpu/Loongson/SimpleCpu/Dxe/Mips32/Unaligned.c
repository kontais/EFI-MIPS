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

  Mips32 Unaligned lw/sw/sh/lh/lhu access.

--*/

#include "CpuDxe.h"
#include "MipsOpcode.h"

EFI_STATUS
UnalignedAccess (
  IN UINT32        OpCode,
  IN UINT32        Addr,
  IN OUT UINT32    *Data
  )
{
  UINT32 Value;
  
  switch (OpCode) {
  case OP_LW:
    Value = *(UINT8*)(Addr + 3);
    Value = Value << 8;
    Value |= *(UINT8*)(Addr + 2);
    Value = Value << 8;
    Value |= *(UINT8*)(Addr + 1);
    Value = Value << 8;
    Value |= *(UINT8*)(Addr);
    *Data = Value;
    break;
  case OP_SW:
    Value = *Data;
    *(UINT8*)(Addr)     = Value & 0xff;
    *(UINT8*)(Addr + 1) = (Value >> 8) & 0xff;
    *(UINT8*)(Addr + 2) = (Value >> 16) & 0xff;
    *(UINT8*)(Addr + 3) = (Value >> 24) & 0xff;
    break;
  case OP_LHU:
    Value = *(UINT8*)(Addr + 1);
    Value = Value << 8;
    Value |= *(UINT8*)(Addr);
    *Data = Value;
    break;
  case OP_SH:
    Value = *Data;
    *(UINT8*)(Addr)     = Value & 0xff;
    *(UINT8*)(Addr + 1) = (Value >> 8) & 0xff;
    break;
  case OP_LH:
    Value = *(INT8*)(Addr + 1); // lb
    Value = Value << 8;
    Value |= *(UINT8*)(Addr);   // lbu
    *Data = Value;
    break;
  default:
    return EFI_UNSUPPORTED;
  }

  return EFI_SUCCESS;
}

