/*++

Copyright (c) 2011, kontais                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  SimpleCpuMemLib.c
  
Abstract:
  
--*/

#include "EfiCommon.h"

UINT8
MemRead8 (
  IN UINT64  Address
  )
{
  return *(volatile UINT8*) (UINTN)Address;
}

UINT16
MemRead16 (
  IN UINT64  Address
  )
{
  return *(volatile UINT16*) (UINTN)Address;
}

UINT32
MemRead32 (
  IN  UINT64  Address
  )
{
  return *(volatile UINT32*) (UINTN)Address;
}

UINT64
MemRead64 (
  IN  UINT64  Address
  )
{
  return *(volatile UINT64*) (UINTN)Address;
}

VOID
MemWrite8 (
  IN UINT64  Address,
  IN UINT8   Data
  )
{
  *(volatile UINT8*) (UINTN)Address = Data;
}

VOID
MemWrite16 (
  IN UINT64  Address,
  IN UINT16  Data
  )
{
  *(volatile UINT16*) (UINTN)Address = Data;
}

VOID
MemWrite32 (
  IN  UINT64  Address,
  IN  UINT32  Data
  )
{
  *(volatile UINT32*) (UINTN)Address = Data;
}

VOID
MemWrite64 (
  IN  UINT64  Address,
  IN  UINT64  Data
  )
{
  *(volatile UINT64*) (UINTN)Address = Data;
}

