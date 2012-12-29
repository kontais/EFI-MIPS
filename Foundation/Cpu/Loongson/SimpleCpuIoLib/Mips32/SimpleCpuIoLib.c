/*++

Copyright (c) 2011, kontais                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  SimpleCpuIoLib.c
  
Abstract:
  
--*/

#include "EfiCommon.h"
#include "SimpleCpuIoLib.h"

UINT8
IoRead8 (
  IN  UINT64  Address
  )
{
  return CpuIoRead8 ((UINT16) Address);
}

UINT16
IoRead16 (
  IN  UINT64  Address
  )
{
  return CpuIoRead16 ((UINT16) Address);
}

UINT32
IoRead32 (
  IN  UINT64  Address
  )
{
  return CpuIoRead32 ((UINT16) Address);
}

VOID
IoWrite8 (
  IN  UINT64  Address,
  IN  UINT8   Data
  )
{
  CpuIoWrite8 ((UINT16) Address, Data);
}

VOID
IoWrite16 (
  IN  UINT64  Address,
  IN  UINT16  Data
  )
{
  CpuIoWrite16 ((UINT16) Address, Data);
}

VOID
IoWrite32 (
  IN  UINT64  Address,
  IN  UINT32  Data
  )
{
  CpuIoWrite32 ((UINT16) Address, Data);
}

