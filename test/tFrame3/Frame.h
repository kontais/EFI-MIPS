/*	$OpenBSD: frame.h,v 1.5 2009/10/22 22:08:52 miod Exp $ */

/*
 * Copyright (c) 1998-2003 Opsycon AB (www.opsycon.se)
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
#ifndef _FRAME_H_
#define	_FRAME_H_

typedef int register_t;

/*
 *  The layout of this *must* match with regnum.h or bad things
 *  will happen. libc setjmp/longjmp depends on this as well.
 */
struct trap_frame {
	register_t	zero;
	register_t	ast;
	register_t	v0;
	register_t	v1;
	register_t	a0;
	register_t	a1;
	register_t	a2;
	register_t	a3;
	register_t	t0;
	register_t	t1;
	register_t	t2;
	register_t	t3;
#define	a4 t0
#define a5 t1
#define a6 t2
#define a7 t3
	register_t	t4;
	register_t	t5;
	register_t	t6;
	register_t	t7;
	register_t	s0;
	register_t	s1;
	register_t	s2;
	register_t	s3;
	register_t	s4;
	register_t	s5;
	register_t	s6;
	register_t	s7;
	register_t	t8;
	register_t	t9;
	register_t	k0;
	register_t	k1;
	register_t	gp;
	register_t	sp;
	register_t	s8;
	register_t	ra;
	register_t	sr;
	register_t	mullo;
	register_t	mulhi;
	register_t	badvaddr;
	register_t	cause;
	register_t	pc;
	register_t	ic;
	register_t	ipl;

};

#endif	/* !_FRAME_H_ */

