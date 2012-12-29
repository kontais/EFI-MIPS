#ifndef _CPU_H_
#define _CPU_H_

/*
 *  * Status register.
 *   */
#define SR_XX     0x80000000
#define SR_COP_USABILITY  0x30000000  /* CP0 and CP1 only */
#define SR_COP_0_BIT    0x10000000
#define SR_COP_1_BIT    0x20000000
#define SR_RP     0x08000000
#define SR_FR_32    0x04000000
#define SR_RE     0x02000000
#define SR_DSD      0x01000000  /* Only on R12000 */
#define SR_BOOT_EXC_VEC   0x00400000
#define SR_TLB_SHUTDOWN   0x00200000
#define SR_SOFT_RESET   0x00100000
#define SR_DIAG_CH    0x00040000
#define SR_DIAG_CE    0x00020000
#define SR_DIAG_DE    0x00010000
#define SR_KX     0x00000080
#define SR_SX     0x00000040
#define SR_UX     0x00000020
#define SR_KSU_MASK   0x00000018
#define SR_KSU_USER   0x00000010
#define SR_KSU_SUPER    0x00000008
#define SR_KSU_KERNEL   0x00000000
#define SR_ERL      0x00000004
#define SR_EXL      0x00000002
#define SR_INT_ENAB   0x00000001

#define SR_INT_MASK   0x0000ff00
#define SOFT_INT_MASK_0   0x00000100
#define SOFT_INT_MASK_1   0x00000200
#define SR_INT_MASK_0   0x00000400
#define SR_INT_MASK_1   0x00000800
#define SR_INT_MASK_2   0x00001000
#define SR_INT_MASK_3   0x00002000
#define SR_INT_MASK_4   0x00004000
#define SR_INT_MASK_5   0x00008000
/*
 *  * Interrupt control register in RM7000. Expansion of interrupts.
 *   */
#define IC_INT_MASK   0x00003f00  /* Two msb reserved */
#define IC_INT_MASK_6   0x00000100
#define IC_INT_MASK_7   0x00000200
#define IC_INT_MASK_8   0x00000400
#define IC_INT_MASK_9   0x00000800
#define IC_INT_TIMR   0x00001000  /* 12 Timer */
#define IC_INT_PERF   0x00002000  /* 13 Performance counter */
#define IC_INT_TE   0x00000080  /* Timer on INT11 */

#define ALL_INT_MASK    ((IC_INT_MASK << 8) | SR_INT_MASK)
#define SOFT_INT_MASK   (SOFT_INT_MASK_0 | SOFT_INT_MASK_1)
#define HW_INT_MASK   (ALL_INT_MASK & ~SOFT_INT_MASK)


/*
 *  * The bits in the cause register.
 *   *
 *    * CR_BR_DELAY Exception happened in branch delay slot.
 *     *  CR_COP_ERR  Coprocessor error.
 *      * CR_IP   Interrupt pending bits defined below.
 *       *  CR_EXC_CODE The exception type (see exception codes below).
 *        */
#define CR_BR_DELAY   0x80000000
#define CR_COP_ERR    0x30000000
#define CR_EXC_CODE   0x0000007c
#define CR_EXC_CODE_SHIFT 2
#define CR_IPEND    0x003fff00
#define CR_INT_SOFT0    0x00000100
#define CR_INT_SOFT1    0x00000200
#define CR_INT_0    0x00000400
#define CR_INT_1    0x00000800
#define CR_INT_2    0x00001000
#define CR_INT_3    0x00002000
#define CR_INT_4    0x00004000
#define CR_INT_5    0x00008000
/* Following on RM7000 */
#define CR_INT_6    0x00010000
#define CR_INT_7    0x00020000
#define CR_INT_8    0x00040000
#define CR_INT_9    0x00080000
#define CR_INT_HARD   0x000ffc00
#define CR_INT_TIMR   0x00100000  /* 12 Timer */
#define CR_INT_PERF   0x00200000  /* 13 Performance counter */

/*
 *  * The bits in the context register.
 *   */
#define CNTXT_PTE_BASE    0xff800000
#define CNTXT_BAD_VPN2    0x007ffff0

/*
 *  * Location of exception vectors.
 *   */
#define RESET_EXC_VEC   (CKSEG1_BASE + 0x1fc00000)
#define TLB_MISS_EXC_VEC  (CKSEG0_BASE + 0x00000000)
#define XTLB_MISS_EXC_VEC (CKSEG0_BASE + 0x00000080)
#define CACHE_ERR_EXC_VEC (CKSEG0_BASE + 0x00000100)
#define GEN_EXC_VEC   (CKSEG0_BASE + 0x00000180)

/*
 *  * Coprocessor 0 registers:
 *   */
#define COP_0_TLB_INDEX   $0
#define COP_0_TLB_RANDOM  $1
#define COP_0_TLB_LO0   $2
#define COP_0_TLB_LO1   $3
#define COP_0_TLB_CONTEXT $4
#define COP_0_TLB_PG_MASK $5
#define COP_0_TLB_WIRED   $6
#define COP_0_BAD_VADDR   $8
#define CP0_Count   $9
#define COP_0_TLB_HI    $10
#define COP_0_COMPARE   $11
#define CP0_Status  $12
#define CP0_Cause   $13
#define COP_0_EXC_PC    $14
#define COP_0_PRID    $15
#define CP0_Config    $16
#define COP_0_LLADDR    $17
#define COP_0_WATCH_LO    $18
#define COP_0_WATCH_HI    $19
#define COP_0_TLB_XCONTEXT  $20
#define COP_0_TLB_FR_MASK $21 /* R10000 onwards */
#define COP_0_DIAG    $22 /* Loongson 2F */
#define COP_0_ECC   $26
#define COP_0_CACHE_ERR   $27
#define COP_0_TAG_LO    $28
#define COP_0_TAG_HI    $29
#define COP_0_ERROR_PC    $30

/*
 *  * RM7000 specific
 *   */
#define COP_0_WATCH_1   $18
#define COP_0_WATCH_2   $19
#define COP_0_WATCH_M   $24
#define COP_0_PC_COUNT    $25
#define COP_0_PC_CTRL   $22

#define COP_0_ICR   $20 /* Use cfc0/ctc0 to access */






/* Use cfc0/ctc0 to access */









/*  $OpenBSD: cpustate.h,v 1.8 2009/10/22 22:08:52 miod Exp $ */

/*
 *  * Copyright (c) 2002-2003 Opsycon AB  (www.opsycon.se / www.opsycon.com)
 *   *
 *    * Redistribution and use in source and binary forms, with or without
 *     * modification, are permitted provided that the following conditions
 *      * are met:
 *       * 1. Redistributions of source code must retain the above copyright
 *        *    notice, this list of conditions and the following disclaimer.
 *         * 2. Redistributions in binary form must reproduce the above copyright
 *          *    notice, this list of conditions and the following disclaimer in the
 *           *    documentation and/or other materials provided with the distribution.
 *            *
 *             * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 *              * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *               * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *                * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 *                 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *                  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 *                   * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 *                    * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *                     * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 *                      * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 *                       * SUCH DAMAGE.
 *                        *
 *                         */

#define KERN_REG_SIZE   (NUMSAVEREGS * REGSZ)
#define KERN_EXC_FRAME_SIZE (CF_SZ + KERN_REG_SIZE + 16)

#define SAVE_REG(reg, offs, base, bo) \
    REG_S reg, bo + (REGSZ * offs) (base)

#define RESTORE_REG(reg, offs, base, bo) \
    REG_L reg, bo + (REGSZ * offs) (base)

/*
 *  *  This macro saves the 'scratch' cpu state on stack.
 *   *  Macros are generic so no 'special' instructions!
 *    *  a0 will have a pointer to the 'frame' on return.
 *     *  a1 will have saved STATUS_REG on return.
 *      *  a3 will have the exception pc on 'return'.
 *       *  No traps, no interrupts if frame = k1 or k0!
 *        *  Temp regs are saved with their register number so
 *         *  branch emulation etc works properly.
 *          */
#define SAVE_CPU(frame, bo)      \
    SAVE_REG(AT, AST, frame, bo)    ;\
  SAVE_REG(v0, V0, frame, bo)   ;\
  SAVE_REG(v1, V1, frame, bo)   ;\
  SAVE_REG(a0, A0, frame, bo)   ;\
  SAVE_REG(a1, A1, frame, bo)   ;\
  SAVE_REG(a2, A2, frame, bo)   ;\
  SAVE_REG(a3, A3, frame, bo)   ;\
  SAVE_REG($8, T0, frame, bo)   ;\
  SAVE_REG($9, T1, frame, bo)   ;\
  SAVE_REG($10, T2, frame, bo)    ;\
  SAVE_REG($11, T3, frame, bo)    ;\
  SAVE_REG($12, T4, frame, bo)    ;\
  SAVE_REG($13, T5, frame, bo)    ;\
  SAVE_REG($14, T6, frame, bo)    ;\
  SAVE_REG($15, T7, frame, bo)    ;\
  SAVE_REG(t8, T8, frame, bo)   ;\
  SAVE_REG(t9, T9, frame, bo)   ;\
  SAVE_REG(gp, GP, frame, bo)   ;\
  SAVE_REG(ra, RA, frame, bo)   ;\
  mflo  v0        ;\
  mfhi  v1        ;\
  mfc0  a0, CP0_Cause   ;\
  mfc0  a1, CP0_Status    ;\
  dmfc0 a2, COP_0_BAD_VADDR   ;\
  dmfc0 a3, COP_0_EXC_PC    ;\
  SAVE_REG(v0, MULLO, frame, bo)    ;\
  SAVE_REG(v1, MULHI, frame, bo)    ;\
  SAVE_REG(a0, CAUSE, frame, bo)    ;\
  SAVE_REG(a1, SR, frame, bo)   ;\
  SAVE_REG(a2, BADVADDR, frame, bo) ;\
  SAVE_REG(a3, PC, frame, bo)   ;\
  SAVE_REG(sp, SP, frame, bo)   ;\
  PTR_ADDU a0, frame, bo      ;\
  SAVE_REG(a2, CPL, frame, bo)

/*
 *  *  Save 'callee save' registers in frame to aid DDB.
 *   */
#define SAVE_CPU_SREG(frame, bo)     \
    SAVE_REG(s0, S0, frame, bo)   ;\
  SAVE_REG(s1, S1, frame, bo)   ;\
  SAVE_REG(s2, S2, frame, bo)   ;\
  SAVE_REG(s3, S3, frame, bo)   ;\
  SAVE_REG(s4, S4, frame, bo)   ;\
  SAVE_REG(s5, S5, frame, bo)   ;\
  SAVE_REG(s6, S6, frame, bo)   ;\
  SAVE_REG(s7, S7, frame, bo)   ;\
  SAVE_REG(s8, S8, frame, bo)

/*
 *  *  Restore cpu state. When called a0 = EXC_PC.
 *   */
#define RESTORE_CPU(frame, bo)       \
    RESTORE_REG(t1, SR, frame, bo)    ;\
  RESTORE_REG(t2, MULLO, frame, bo) ;\
  RESTORE_REG(t3, MULHI, frame, bo) ;\
  mtc0  t1, CP0_Status    ;\
  mtlo  t2        ;\
  mthi  t3        ;\
  dmtc0 a0, COP_0_EXC_PC    ;\
  RESTORE_REG(AT, AST, frame, bo)   ;\
  RESTORE_REG(v0, V0, frame, bo)    ;\
  RESTORE_REG(v1, V1, frame, bo)    ;\
  RESTORE_REG(a0, A0, frame, bo)    ;\
  RESTORE_REG(a1, A1, frame, bo)    ;\
  RESTORE_REG(a2, A2, frame, bo)    ;\
  RESTORE_REG(a3, A3, frame, bo)    ;\
  RESTORE_REG($8, T0, frame, bo)    ;\
  RESTORE_REG($9, T1, frame, bo)    ;\
  RESTORE_REG($10, T2, frame, bo)   ;\
  RESTORE_REG($11, T3, frame, bo)   ;\
  RESTORE_REG($12, T4, frame, bo)   ;\
  RESTORE_REG($13, T5, frame, bo)   ;\
  RESTORE_REG($14, T6, frame, bo)   ;\
  RESTORE_REG($15, T7, frame, bo)   ;\
  RESTORE_REG(t8, T8, frame, bo)    ;\
  RESTORE_REG(t9, T9, frame, bo)    ;\
  RESTORE_REG(gp, GP, frame, bo)    ;\
  RESTORE_REG(ra, RA, frame, bo)

/*
 *  *  Restore 'callee save' registers
 *   */
#define RESTORE_CPU_SREG(frame, bo)    \
    RESTORE_REG(s0, S0, frame, bo)    ;\
  RESTORE_REG(s1, S1, frame, bo)    ;\
  RESTORE_REG(s2, S2, frame, bo)    ;\
  RESTORE_REG(s3, S3, frame, bo)    ;\
  RESTORE_REG(s4, S4, frame, bo)    ;\
  RESTORE_REG(s5, S5, frame, bo)    ;\
  RESTORE_REG(s6, S6, frame, bo)    ;\
  RESTORE_REG(s7, S7, frame, bo)    ;\
  RESTORE_REG(s8, S8, frame, bo)


#endif /* _CPU_H_ */

