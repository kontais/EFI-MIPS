/*	$OpenBSD: asm.h,v 1.13 2010/10/01 05:02:19 guenther Exp $ */

/*
 * Copyright (c) 2001-2002 Opsycon AB  (www.opsycon.se / www.opsycon.com)
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
#ifndef _MIPS_ASM_H_
#define _MIPS_ASM_H_

#include "RegDef.h"

#define ITLBNOPFIX      nop;nop;nop;nop

#if !defined(__MIPSEL__) && !defined(__MIPSEB__)
#error "__MIPSEL__ or __MIPSEB__ must be defined"
#endif
/*
 * Define how to access unaligned data word
 */
#if defined(__MIPSEL__)
#define LWLO  lwl
#define LWHI  lwr
#define	SWLO	swl
#define	SWHI	swr
#endif

#if defined(__MIPSEB__)
#define LWLO  lwr
#define LWHI  lwl
#define	SWLO	swr
#define	SWHI	swl
#endif

#define ALIGNSZ     16  /* Stack layout alignment */
#define FRAMESZ(sz) (((sz) + (ALIGNSZ-1)) & ~(ALIGNSZ-1))

/*
 *  Basic register operations based on selected ISA
 */
#define REGSZ             4   /* 32 bit mode register size */
#define REG_S             sw
#define REG_L             lw
#define CF_SZ             24  /* Call frame size */
#define CF_RA_OFFS        20  /* Call ra save offset */
#define STAND_ARG_SIZE    16


#define PTR_L       lw
#define PTR_S       sw
#define PTR_SUB     sub
#define PTR_ADD     add
#define PTR_SUBU    subu
#define PTR_ADDU    addu
#define LI          li
#define LA          la
#define PTR_SLL     sll
#define PTR_SRL     srl
#define PTR_VAL     .word


/*------------------------------------------------------------------------
 *  宏名:  LEAF
 *  引用:  LEAF(x)
 *  参数:
 *         x  定义的函数符号名
 *  说明:  定义MIPS叶子函数的宏，叶子函数即该函数不再调用其它任何函数
 *         因此不需要进行堆栈调用设置，一般使用ra寄器返回
 *------------------------------------------------------------------------*/
#define LEAF(x)      \
  .align  3   ;      \
  .globl  x   ;      \
  .ent    x, 0;      \
x:

#define  ALEAF(x)    \
  .globl  x;    \
x:

/*--------------------------------------------------------------------
 * NON_LEAF(x)
 *
 *  Declare a non-leaf routine (a routine that makes other C calls).
 *-------------------------------------------------------------------*/
#define NON_LEAF(x, fsize, retpc) \
  .align  3;    \
  .globl x;    \
  .ent x, 0;    \
x: ;        \
  .frame sp, fsize, retpc

/*------------------------------------------------------------------------
 *  宏名:  END
 *  引用:  END(x)
 *  参数:
 *         x  函数符号名
 *  说明:  函数结束定义
 *------------------------------------------------------------------------*/
#define END(x) \
  .end x

/*------------------------------------------------------------------------
 *  宏名:  BRANCH_CALL
 *  引用:  BRANCH_CALL(x)
 *  参数:
 *         x  函数符号名
 *  说明:  256k范围跳转,并定义xEnd全局符号
 *------------------------------------------------------------------------*/
#define BRANCH_CALL(x)      \
  b x      ;   \
  nop      ;   \
  .global x##End ; \
x##End:    ;

/*------------------------------------------------------------------------
 *  宏名:  BRANCH_RETURN
 *  引用:  BRANCH_RETURN(x)
 *  参数:
 *         x  函数符号名
 *  说明:  256k范围跳转到xEnd
 *------------------------------------------------------------------------*/
#define BRANCH_RETURN(x)   \
  b x##End      ;  \
  nop           ;

#endif /* !_MIPS_ASM_H_ */
