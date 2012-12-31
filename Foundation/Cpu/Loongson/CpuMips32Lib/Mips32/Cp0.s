/*++

Copyright (c) 2010, kontais               
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  Cp0.s

Abstract:

  CP0 access routine.

--*/

#include "Mips.h"


  .text
  .align 3
  .set   noreorder


  .globl CP0_GetPRID
  .ent   CP0_GetPRID
  .type  CP0_GetPRID, @function

CP0_GetPRID:
/*++
UINT32
CP0_GetPRID (
  VOID
  )

Routine Description:

  MIPS get cp0 processor revision identifier.

Arguments:

  None
  
Returns:

  Value of processor revision identifier.

--*/

  mfc0  v0, CP0_PRId 
  nop
  j	ra
  nop
  
  .end CP0_GetPRID

  .globl CP0_GetCount
  .ent   CP0_GetCount
  .type  CP0_GetCount, @function

CP0_GetCount:
/*++
UINT64
CP0_GetCount (
  VOID
  )

Routine Description:

  MIPS get cp0 count.

Arguments:

  None
  
Returns:

  Value of cp0 count.

--*/

  mfc0  v0, CP0_Count
  move  v1, zero
  j	ra
  nop
  
  .end CP0_GetCount

  .globl CP0_SetCount
  .ent   CP0_SetCount
  .type  CP0_SetCount, @function

CP0_SetCount:
/*++
UINT64
CP0_SetCount (
  UINT64 Count
  )

Routine Description:

  MIPS set cp0 count.

Arguments:

  None
  
Returns:

--*/

  mtc0  a0, CP0_Count
  nop
  j	ra
  nop
  
  .end CP0_SetCount

  .globl CP0_GetStatus
  .ent   CP0_GetStatus
  .type  CP0_GetStatus, @function

CP0_GetStatus:
/*++
UINT32
CP0_GetStatus (
  VOID
  )

Routine Description:

  MIPS get cp0 status.

Arguments:

  None
  
Returns:

  Value of cp0 status.

--*/

  mfc0  v0, CP0_Status
  nop
  j	ra
  nop
  
  .end CP0_GetStatus

  .globl CP0_SetStatus
  .ent   CP0_SetStatus
  .type  CP0_SetStatus, @function

CP0_SetStatus:
/*++
VOID
CP0_SetStatus (
  UINT32 Status
  )

Routine Description:

  MIPS set cp0 status.

Arguments:

  Value to set to cp0 status.
  
Returns:

  None

--*/

  mtc0  a0, CP0_Status
  nop
  j	ra
  nop
  
  .end CP0_SetStatus


  .globl CP0_GetCause
  .ent   CP0_GetCause
  .type  CP0_GetCause, @function

CP0_GetCause:
/*++
UINT32
CP0_GetCause (
  VOID
  )

Routine Description:

  MIPS get cp0 cause.

Arguments:

  None
  
Returns:

  Value of cp0 cause.

--*/

  mfc0  v0, CP0_Cause
  nop
  j	ra
  nop
  
  .end CP0_GetCause

  .globl CP0_SetCause
  .ent   CP0_SetCause
  .type  CP0_SetCause, @function

CP0_SetCause:
/*++
VOID
CP0_SetCause (
  UINT32 Cause
  )

Routine Description:

  MIPS set cp0 cause.

Arguments:

  Value to set to cp0 cause.
  
Returns:

  None

--*/

  mtc0  a0, CP0_Cause
  nop
  j	ra
  nop
  
  .end CP0_SetCause

  .globl CP0_GetCompare
  .ent   CP0_GetCompare
  .type  CP0_GetCompare, @function

CP0_GetCompare:
/*++
UINT32
CP0_GetCompare (
  VOID
  )

Routine Description:

  MIPS get cp0 compare register.

Arguments:

  None
  
Returns:

  Value of cp0 compare.

--*/

  mfc0  v0, CP0_Compare
  nop
  j	ra
  nop
  
  .end CP0_GetCompare

  .globl CP0_SetCompare
  .ent   CP0_SetCompare
  .type  CP0_SetCompare, @function

CP0_SetCompare:
/*++
VOID
CP0_SetCompare (
  UINT32 Compare
  )

Routine Description:

  MIPS set cp0 compare.

Arguments:

  Value to set to cp0 compare.
  
Returns:

  None

--*/

  mtc0  a0, CP0_Compare
  nop
  j	ra
  nop
  
  .end CP0_SetCompare




