	.file	1 "sw.c"
	.section .mdebug.abi32
	.previous
	.gnu_attribute 4, 1
	.rdata
	.align	2
$LC0:
	.ascii	"\006\010\000"
	.text
	.align	2
	.globl	main
	.ent	main
	.type	main, @function
main:
	.set	nomips16
	.frame	$fp,24,$31		# vars= 16, regs= 1/0, args= 0, gp= 0
	.mask	0x40000000,-4
	.fmask	0x00000000,0
	.set	noreorder
	.set	nomacro
	
	addiu	$sp,$sp,-24
	sw	$fp,20($sp)
	move	$fp,$sp
	lui	$2,%hi($LC0)
	addiu	$2,$2,%lo($LC0)
	sw	$2,0($fp)
	lw	$2,0($fp)
	lbu	$2,0($2)
	move	$3,$2
	lw	$2,0($fp)
	addiu	$2,$2,1
	sw	$2,0($fp)
	addiu	$3,$3,-3
	sw	$3,12($fp)
	lw	$3,12($fp)
	sltu	$2,$3,6
	beq	$2,$0,$L2
	nop

	lw	$2,12($fp)
	sll	$3,$2,2
	lui	$2,%hi($L6)
	addiu	$2,$2,%lo($L6)
	addu	$2,$3,$2
	lw	$2,0($2)
	j	$2
	nop

	.rdata
	.align	2
	.align	2
$L6:
	.word	$L3
	.word	$L3
	.word	$L2
	.word	$L4
	.word	$L4
	.word	$L5
	.text
$L3:
	li	$3,1			# 0x1
	sw	$3,8($fp)
	j	$L7
	nop

$L5:
	li	$2,2			# 0x2
	sw	$2,8($fp)
	j	$L7
	nop

$L4:
	li	$3,3			# 0x3
	sw	$3,8($fp)
	j	$L7
	nop

$L2:
	li	$2,4			# 0x4
	sw	$2,8($fp)
$L7:
	lw	$2,8($fp)
	move	$sp,$fp
	lw	$fp,20($sp)
	addiu	$sp,$sp,24
	j	$31
	nop

	.set	macro
	.set	reorder
	.end	main
	.ident	"GCC: (RAYS 4.3.0-4.rays0) 4.3.1 20080501 (prerelease)"
