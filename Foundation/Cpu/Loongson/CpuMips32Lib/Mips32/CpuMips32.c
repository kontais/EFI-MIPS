/*++

Copyright (c) 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  CpuMips32.c

Abstract:

--*/

#include "CpuMips32.h"
#include "Mips.h"

VOID
EfiHalt (VOID)
{
  return ;
}

VOID
EfiWbinvd (VOID)
{
  return ;
}

VOID
EfiInvd (VOID)
{
  return ;
}

UINT32
EfiCpuid (VOID)
{
  return CP0_GetPRID();
}

UINT64
EfiReadMsr (IN UINT32 Index)
{
  return 0;
}

VOID
EfiWriteMsr (
  IN   UINT32  Index,
  IN   UINT64  Value
  )
{
  return ;
}

UINT32
EfiReadTsc (VOID)
{
  //
  // Use MIPS Count as IA32 TSC
  //
  return CP0_GetCount();
}

VOID
EfiDisableCache (VOID)
{
  return ;
}

VOID
EfiEnableCache (VOID)
{
  return ;
}

UINT32
EfiGetEflags (
  VOID
  )
{
  return 0;
}

VOID
EfiDisableInterrupts (VOID)
{
  UINT32 Status;

  Status =  CP0_GetStatus();
  Status &= ~CP0_STATUS_IE;

  CP0_SetStatus(Status);

  return ;
}

VOID
EfiEnableInterrupts (
  VOID
  )
{
  UINT32 Status;

  Status =  CP0_GetStatus();
  Status |= CP0_STATUS_IE;

  CP0_SetStatus(Status);
  
  return ;
}

VOID
EfiCpuidExt (
  IN   UINT32              RegisterInEax,
  IN   UINT32              CacheLevel,
  OUT  EFI_CPUID_REGISTER  *Regs              
  )
{
  return ;
}
