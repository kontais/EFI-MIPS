#include "Mips.h"

	.file	1 "Math.c"
	.section .mdebug.abi32
	.previous
	.gnu_attribute 4, 1
	.text
	.align	2
	.globl	PeiImageRead
	.ent	PeiImageRead
	.type	PeiImageRead, @function
PeiImageRead:
	.set	nomips16
	.frame	fp,24,ra		# vars= 16, regs= 1/0, args= 0, gp= 0
	.mask	0x40000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	sp,sp,-24
	sw	fp,20(sp)
	move	fp,sp
	sw	a0,24(fp)  // FileHandle
	sw	a1,28(fp)  // FileOffset
	sw	a2,32(fp)  // ReadSize
	sw	a3,36(fp)  // Buffer
	lw	v0,36(fp)
	nop
	sw	v0,8(fp)   // 8(fp) Destination8 = v0 = Buffer
	lw	v1,24(fp)  // v1 = FileHandle
	lw	v0,28(fp)  // v0 = FileOffset
	nop
	addu	v0,v1,v0 // v0 = FileHandle + FileOffset
	sw	v0,4(fp)   // 4(fp) = Source8 = FileHandle + FileOffset
	lw	v0,32(fp)  // v0 = ReadSize
	nop
	lw	v0,0(v0)   // v0 = *ReadSize
	nop
	sw	v0,0(fp)   // 0(sp) = *ReadSize
	j	$L2
	nop

$L3:
	lw	v0,4(fp)
	nop
	lbu	v1,0(v0)
	lw	v0,8(fp)
	nop
	sb	v1,0(v0)
	lw	v0,8(fp)
	nop
	addiu	v0,v0,1
	sw	v0,8(fp)
	lw	v0,4(fp)
	nop
	addiu	v0,v0,1
	sw	v0,4(fp)
$L2:
	lw	v0,0(fp)
	nop
	sltu	v0,zero,v0
	andi	v1,v0,0x00ff
	lw	v0,0(fp)
	nop
	addiu	v0,v0,-1
	sw	v0,0(fp)
	bne	v1,zero,$L3
	nop

	move	v0,zero
	move	sp,fp
	lw	fp,20(sp)
	addiu	sp,sp,24
	j	ra
	nop

	.set	macro
	.set	reorder
	.end	PeiImageRead
	.ident	"GCC: (RAYS 4.3.0-4.rays0) 4.3.1 20080501 (prerelease)"
