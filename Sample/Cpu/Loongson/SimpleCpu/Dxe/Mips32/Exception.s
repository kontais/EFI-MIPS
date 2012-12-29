/*    Copyright (c) 2010 kontais */
/*    $OpenBSD: exception.S,v 1.30 2010/02/01 05:26:17 miod Exp $ */

/*
 * Copyright (c) 2002-2003 Opsycon AB  (www.opsycon.se / www.opsycon.com)
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

/*
 *  This code handles exceptions and dispatches to the
 *  correct handler depending on the exception type.
 *
 *  Exceptions are directed to the following addresses:
 *  0xffffffffbfc00000  Cold Reset/NMI. Not handled by the kernel.
 *  0xffffffff80000000  TLB/XTLB refill, not in exception.
 *  0xffffffff80000180  Others...
 */

#include "Mips.h"

    .text

ExceptionTable:
    .word    ExceptionInterrupt
    .word    ExceptionGeneral
    .word    ExceptionGeneral
    .word    ExceptionGeneral
    .word    ExceptionGeneral
    .word    ExceptionGeneral
    .word    ExceptionGeneral
    .word    ExceptionGeneral
    .word    ExceptionGeneral
    .word    ExceptionGeneral
    .word    ExceptionGeneral
    .word    ExceptionGeneral
    .word    ExceptionGeneral
    .word    ExceptionGeneral
    .word    ExceptionGeneral
    .word    ExceptionGeneral
    .word    ExceptionGeneral
    .word    ExceptionGeneral
    .word    ExceptionGeneral
    .word    ExceptionGeneral
    .word    ExceptionGeneral
    .word    ExceptionGeneral
    .word    ExceptionGeneral
    .word    ExceptionGeneral
    .word    ExceptionGeneral
    .word    ExceptionGeneral
    .word    ExceptionGeneral
    .word    ExceptionGeneral
    .word    ExceptionGeneral
    .word    ExceptionGeneral
    .word    ExceptionGeneral
    .word    ExceptionGeneral


    .globl IntrServiceState
IntrServiceState:
    .word  0

    .set    noreorder

/*----------------------------------------------------------------
 *    General exception handler dispatcher. This code is copied
 *    to the vector area and must thus be PIC and less than 128
 *    bytes long to fit. Only k0 and k1 may be used at this time.
 *--------------------------------------------------------------*/
    .align 3
    .globl    Exception
Exception:

    .set    noat

    /*
     * To work around a branch prediction issue on earlier LS2F
     * chips, it is necessary to clear the BTB upon
     * userland->kernel boundaries.
     */
    li    k0, CP0_DIAG_BTB_CLEAR | CP0_DIAG_RAS_DISABLE
    dmtc0 k0, CP0_Diagnostic

    la    k1, IntrServiceState
    li    k0, 1       // TRUE
    sw    k0, 0(k1)

    mfc0  k1, CP0_Cause
    la    k0, ExceptionTable
    and   k1, k1, CP0_CAUSE_EXC_CODE
    addu  k0, k0, k1
    lw    k0, 0(k0)
    nop
    j     k0
    nop
    
    .set    at
    
    .globl    ExceptionEnd
ExceptionEnd:


/*----------------------------------------------------------------
 *    Handle an interrupt in kernel mode. This is easy since we
 *    just need to save away the 'save' registers and state.
 *    State is saved on kernel stack.
 *--------------------------------------------------------------*/
    .align 3
    .globl ExceptionInterrupt
    .ent   ExceptionInterrupt
    .type  ExceptionInterrupt, @function
ExceptionInterrupt:

    .set  noat
    sub   k0, sp, FRAMESZ(KERN_EXC_FRAME_SIZE)
    SAVE_CPU(k0, CF_RA_OFFS)
    .set  at

    //  Set New Stack
    move  sp, k0
    
    // Disable Interrupt, Set COP/EXL/KSU Value
    and   t0, a1, ~(CP0_STATUS_CU_CP1 | CP0_STATUS_EXL | CP0_STATUS_IE | CP0_STATUS_KSU)
    mtc0  t0, CP0_Status
    ITLBNOPFIX

    // a1 set in SAVE_CPU macro
    move  a0, zero
    jal   InterruptDispatcher
    nop

    .set  noat
    RESTORE_CPU(sp, CF_RA_OFFS)
    addu  sp, sp, FRAMESZ(KERN_EXC_FRAME_SIZE)

    la    k1, IntrServiceState
    sw    zero, 0(k1)  // FALSE

    sync
    eret
    .set  at
    
  .end ExceptionInterrupt


/*----------------------------------------------------------------
 *    Handle a kernel general trap. This is very much like
 *    ExceptionInterrupt except that we call ExceptionHandle 
 *    instead of interrupt.
 *--------------------------------------------------------------*/
    .align 3
    .globl ExceptionGeneral
    .ent   ExceptionGeneral
    .type  ExceptionGeneral, @function
ExceptionGeneral:

    .set  noat
    sub   k0, sp, FRAMESZ(KERN_EXC_FRAME_SIZE)
    SAVE_CPU(k0, CF_RA_OFFS)
    .set  at

    //  Set New Stack
    move  sp, k0
    
    // Disable Interrupt, Set COP/EXL/KSU Value
    and   t0, a1, ~(CP0_STATUS_CU_CP1 | CP0_STATUS_EXL | CP0_STATUS_IE | CP0_STATUS_KSU)
    mtc0  t0, CP0_Status
    ITLBNOPFIX

    // a1 set in SAVE_CPU macro
    RESTORE_REG(a0, CAUSE, sp, CF_RA_OFFS)
    nop
    and   a0, a0, CP0_CAUSE_EXC_CODE
    srl   a0, CP0_CAUSE_EXC_SHIFT
    jal   ExceptionDispatcher
    nop

    .set  noat
    RESTORE_CPU(sp, CF_RA_OFFS)
    addu  sp, sp, FRAMESZ(KERN_EXC_FRAME_SIZE)

    la    k1, IntrServiceState
    sw    zero, 0(k1)  // FALSE

    sync
    eret
    .set  at
    
  .end ExceptionGeneral



/*----------------------------------------------------------------
 *    TLB miss exception handler dispatcher. This code is copied
 *    to the vector area and must thus be PIC and less than 128
 *    bytes long to fit. Only k0 and k1 may be used at this time.
 *--------------------------------------------------------------*/
    .align 3
ExceptionTlbMissTable:
    .word    ExceptionTlbMissGeneral
    
    
    .globl    ExceptionTlbMiss
ExceptionTlbMiss:

    .set    noat

    la    k1, IntrServiceState
    li    k0, 1       // TRUE
    sw    k0, 0(k1)

    la    k0, ExceptionTlbMissTable
    nop   
    lw    k0, 0(k0)
    nop
    j     k0
    nop
    
    .set    at
    
    .globl    ExceptionTlbMissEnd
ExceptionTlbMissEnd:

    .align 3
    .globl ExceptionTlbMissGeneral
    .ent   ExceptionTlbMissGeneral
    .type  ExceptionTlbMissGeneral, @function
ExceptionTlbMissGeneral:

    .set  noat
    sub   k0, sp, FRAMESZ(KERN_EXC_FRAME_SIZE)
    SAVE_CPU(k0, CF_RA_OFFS)
    .set  at

    //  Set New Stack
    move  sp, k0
    
    // Disable Interrupt, Set COP/EXL/KSU Value
    and   t0, a1, ~(CP0_STATUS_CU_CP1 | CP0_STATUS_EXL | CP0_STATUS_IE | CP0_STATUS_KSU)
    mtc0  t0, CP0_Status
    ITLBNOPFIX

    // a1 set in SAVE_CPU macro
    move  a0, zero
    ori   a0, 40      // EXCEPT_MIPS32_TLB_MISS
    jal   ExceptionDispatcher
    nop

    .set  noat
    RESTORE_CPU(sp, CF_RA_OFFS)
    addu  sp, sp, FRAMESZ(KERN_EXC_FRAME_SIZE)

    la    k1, IntrServiceState
    sw    zero, 0(k1)  // FALSE

    sync
    eret
    .set  at
    
  .end ExceptionTlbMissGeneral
