/*++

Copyright (c) 2009, kontais                                                                   
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  EbcLowLevel.s

Abstract:

  Core stack switching routine, invoked when real system memory is
   discovered and installed.

--*/

#include "Mips.h"


  .text
  .align 3
  .set noreorder
  
/*---------------------------------------------------------------------------
 * EbcLLCALLEXNative
 *
 * This function is called to execute an EBC CALLEX instruction
 * to native code. 
 * This instruction requires that we thunk out to external native
 * code. For IA32, we simply switch stacks and jump to the 
 * specified function. On return, we restore the stack pointer
 * to its original location.
 *
 * Destroys no working registers.
 *
 * VOID EbcLLCALLEXNative(UINTN FuncAddr, UINTN NewStackPointer, VOID *FramePtr)
 *---------------------------------------------------------------------------*/
  .globl EbcLLCALLEXNative
  .ent   EbcLLCALLEXNative
  .type  EbcLLCALLEXNative, @function
EbcLLCALLEXNative:
  jr    ra
  nop
  .end  EbcLLCALLEXNative

/*---------------------------------------------------------------------------
 * UINTN EbcLLGetEbcEntryPoint(VOID);
 * Routine Description:
 *   The VM thunk code stuffs an EBC entry point into a processor
 *   register. Since we can't use inline assembly to get it from
 *   the interpreter C code, stuff it into the return value 
 *   register and return.
 *
 * Arguments:
 *     None.
 *
 * Returns:
 *     The contents of the register in which the entry point is passed.
 *
 *------------------------------------------------------------------------*/
  .globl EbcLLGetEbcEntryPoint
  .ent   EbcLLGetEbcEntryPoint
  .type  EbcLLGetEbcEntryPoint, @function
EbcLLGetEbcEntryPoint:
  jr    ra
  nop
  .end  EbcLLGetEbcEntryPoint

/*--------------------------------------------------------------------------
 * 
 * Routine Description:
 *   
 *   Return the caller's value of the stack pointer.
 * 
 * Arguments:
 * 
 *   None.
 * 
 * Returns:
 * 
 *   The current value of the stack pointer for the caller. We
 *   adjust it by 4 here because when they called us, the return address
 *   is put on the stack, thereby lowering it by 4 bytes.
 * 
 * 
 *  UINTN EbcLLGetStackPointer()    
 *------------------------------------------------------------------------*/
  .globl EbcLLGetStackPointer
  .ent   EbcLLGetStackPointer
  .type  EbcLLGetStackPointer, @function
EbcLLGetStackPointer:
  jr    ra
  nop
  .end  EbcLLGetStackPointer

/*--------------------------------------------------------------------------
 * UINT64 EbcLLGetReturnValue(VOID);
 * Routine Description:
 *   When EBC calls native, on return the VM has to stuff the return
 *   value into a VM register. It's assumed here that the value is still
 *    in the register, so simply return and the caller should get the
 *   return result properly.
 *
 * Arguments:
 *     None.
 *
 * Returns:
 *     The unmodified value returned by the native code.
 *
 *------------------------------------------------------------------------*/
  .globl EbcLLGetReturnValue
  .ent   EbcLLGetReturnValue
  .type  EbcLLGetReturnValue, @function
EbcLLGetReturnValue:
  jr    ra
  nop
  .end  EbcLLGetReturnValue
  
  .set reorder
