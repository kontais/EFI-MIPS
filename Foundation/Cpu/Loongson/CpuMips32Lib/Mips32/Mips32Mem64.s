/*++

Copyright (c) 1998-2004 Opsycon AB (www.opsycon.se)
Copyright (c) 2009 Miodrag Vallat.
Copyright (c) 2010, kontais               
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  MipsCache.s

Abstract:

  MIPS Cache routines

--*/

#include "Mips.h"
#include "MipsCache.h"


  .text
  .align 3
  .set   push
  //.set   mips3
  .set   noreorder



 /*------------------------------------------------------------------------------
 *  UINT64
 *  Load64 (
 *    IN  UINT64  Address
 *    )
 *-----------------------------------------------------------------------------*/

  .globl Load64
  .ent   Load64
  .type  Load64, @function

Load64:
  dsll  a0, 32
  dsrl  a0, 32
  dsll  a1, 32
  daddu a1, a0

  ld    v0, 0(a1)
  nop

  daddu v1, v0, zero
  dsrl  v1, 32
  
  j  ra
  nop

  .end Load64
  
 /*------------------------------------------------------------------------------
 *  VOID
 *  Store64 (
 *    IN  UINT64  Address,
 *    IN  UINT64  Data
 *    )
 *-----------------------------------------------------------------------------*/
  .globl Store64
  .ent   Store64
  .type  Store64, @function

Store64:
  dsll  a0, 32
  dsrl  a0, 32
  dsll  a1, 32
  daddu a1, a0

  dsll  a2, 32
  dsrl  a2, 32
  dsll  a3, 32
  daddu a3, a2

  sd    a3, 0(a1)
  nop
  
  j  ra
  nop

  .end Store64

   
  .set pop
  
