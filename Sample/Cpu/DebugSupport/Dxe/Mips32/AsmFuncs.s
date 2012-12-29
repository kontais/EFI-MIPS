/*++

Copyright (c) 2009, kontais                                                                   
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  ProcessorAsms.s

Abstract:

  Core stack switching routine, invoked when real system memory is
   discovered and installed.

--*/

#include "Mips.h"

  .data
  .align 3
  
  .global OrigVector, InterruptEntryStub, StubSize, CommonIdtEntry, FxStorSupport

StubSize:    .word    InterruptEntryStubEnd - InterruptEntryStub
AppEsp:      .word    0x11111111
DebugEsp:    .word    0x22222222
ExtraPush:   .word    0x33333333
ExceptData:  .word    0x44444444
Eflags:      .word    0x55555555
OrigVector:  .word    0x66666666
  
// The declarations below define the memory region that will be used for the debug stack.
// The context record will be built by pushing register values onto this stack.
// It is imparitive that alignment be carefully managed, since the FXSTOR and
// FXRSTOR instructions will GP fault if their memory operand is not 16 byte aligned.
//
// The stub will switch stacks from the application stack to the debuger stack
// and pushes the exception number.
//
// Then we building the context record on the stack. Since the stack grows down,
// we push the fields of the context record from the back to the front.  There
// are 132 bytes of stack used prior allocating the 512 bytes of stack to be
// used as the memory buffer for the fxstor instruction. Therefore address of
// the buffer used for the FXSTOR instruction is &Eax - 132 - 512, which
// must be 16 byte aligned.
//
// We carefully locate the stack to make this happen.
//
// For reference, the context structure looks like this:
//      struct {
//        UINT32             ExceptionData;
//        FX_SAVE_STATE_IA32 FxSaveState;    // 512 bytes, must be 16 byte aligned
//        UINT32             Dr0, Dr1, Dr2, Dr3, Dr6, Dr7;
//        UINT32             Cr0, Cr1, Cr2, Cr3, Cr4;
//        UINT32             EFlags;
//        UINT32             Ldtr, Tr;
//        UINT32             Gdtr[2], Idtr[2];
//        UINT32             Eip;
//        UINT32             Gs, Fs, Es, Ds, Cs, Ss;
//        UINT32             Edi, Esi, Ebp, Esp, Ebx, Edx, Ecx, Eax;
//      } SYSTEM_CONTEXT_IA32;  // 32 bit system context record


  .align  4
DebugStackEnd:   .ascii     "DbgStkEnd >>>>>>"  // 16 byte long string - must be 16 bytes to preserve alignment
                 .word      0x00000000:0x1ffd   // 32K should be enough stack
                                                //   This allocation is coocked to insure 
                                                //   that the the buffer for the FXSTORE instruction
                                                //   will be 16 byte aligned also.
                                                //
ExceptionNumber: .word      0x00000000          // first entry will be the vector number pushed by the stub

DebugStackBegin: .ascii     "<<<< DbgStkBegin"  // initial debug ESP == DebugStackBegin, set in stub


  
  .text
  .align 3
  
// externdef InterruptDistrubutionHub:near

/*------------------------------------------------------------------------------
 *  VOID
 *  EfiWbinvd (
 *    VOID
 *    )
 *
 * Abstract: Writeback and invalidate cache
 *-----------------------------------------------------------------------------*/
  .globl EfiWbinvd
  .ent   EfiWbinvd
  .type  EfiWbinvd, @function
  .set   noreorder
EfiWbinvd:
  nop
  jr    ra
  nop
  .set   reorder
  .end   EfiWbinvd
  
  
  
/*------------------------------------------------------------------------------
 * BOOLEAN
 * FxStorSupport (
 *   void
 *   )
 *
 * Abstract: Returns TRUE if FxStor instructions are supported
 *
 *----------------------------------------------------------------------------*/
  .globl FxStorSupport
  .ent   FxStorSupport
  .type  FxStorSupport, @function
  .set   noreorder
FxStorSupport:
  nop
  jr    ra
  li    v0, 0
  .set   reorder
  .end   FxStorSupport


/*------------------------------------------------------------------------------
 * DESCRIPTOR *
 * GetIdtr (
 *   void
 *   )
 *
 * Abstract: Returns physical address of IDTR
 *
 *----------------------------------------------------------------------------*/
  .globl GetIdtr
  .ent   GetIdtr
  .type  GetIdtr, @function
  .set   noreorder
GetIdtr:
  nop
  jr    ra
  li    v0, 0
  .set   reorder
  .end   GetIdtr

/*------------------------------------------------------------------------------
 * BOOLEAN                                                                      
 * WriteInterruptFlag (                                                         
 *   BOOLEAN NewState                                                           
 *   )                                                                          
 *                                                                              
 * Abstract: Programs interrupt flag to the requested state and returns previous
 *           state.                                                             
 *----------------------------------------------------------------------------*/
  .globl WriteInterruptFlag
  .ent   WriteInterruptFlag
  .type  WriteInterruptFlag, @function
  .set   noreorder
WriteInterruptFlag:
  nop
  jr    ra
  li    v0, 0
  .set   reorder
  .end   WriteInterruptFlag



/*------------------------------------------------------------------------------
 * void                                                               
 * Vect2Desc (                                                        
 *   DESCRIPTOR * DestDesc,                                           
 *   void (*Vector) (void)                                            
 *   )                                                                
 *                                                                    
 * Abstract: Encodes an IDT descriptor with the given physical address
 *
 *----------------------------------------------------------------------------*/
  .globl Vect2Desc
  .ent   Vect2Desc
  .type  Vect2Desc, @function
  .set   noreorder
Vect2Desc:
  nop
  jr    ra
  li    v0, 0
  .set   reorder
  .end   Vect2Desc
  

 /*------------------------------------------------------------------------------
 * InterruptEntryStub
 *
 * Abstract: This code is not a function, but is a small piece of code that is
 *               copied and fixed up once for each IDT entry that is hooked.
 *----------------------------------------------------------------------------*/
  .global InterruptEntryStub
  .set   noreorder
InterruptEntryStub:
  la    a0, AppEsp                  // save stack top
  sw    sp, 0(a0)
  la    sp, DebugStackBegin         // switch to debugger stack
  li    a0, 0                       // vector number - will be modified before installed
  la    v0, CommonIdtEntry          // fixed up to relative address of CommonIdtEntry
  nop
  jr    v0
  nop
  .set   reorder
  .global InterruptEntryStubEnd
InterruptEntryStubEnd:



 /*-----------------------------------------------------------------------------
 * CommonIdtEntry
 *
 * Abstract: This code is not a function, but is the common part for all IDT
 *               vectors.
 *----------------------------------------------------------------------------*/
  .global CommonIdtEntry
  .set noreorder
CommonIdtEntry:
  nop
  la    a0, AppEsp
  nop
  lw    sp, 0(a0)
  nop
  eret
  .set reorder
