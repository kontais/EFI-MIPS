#
# Switch from protected mode to real mode and jump to setup.S
# image located at %cx:0.
#
# This module must be placed into physical memory at 0:7C00h.
# EFI has some real mode thunking code at 2000:0h.
#
# Processor and non-maskable interrupts should be disabled
# before control is passed to this module.
#

.global _start

.code32
.text
_start:
	#
	# Load identity mapped GDT & real mode IDT.
	# Add 7C00h to the addresses since this is linked to start
	# at 0h and it is being placed at 7C00h.
	#

	lgdt	%cs:gdt_48 + 0x7C00
	lidt	%cs:idt_48 + 0x7C00

	#
	# Turn off PG bit in CR0 and set CR3 to zero.
	#

	movl	%cr0, %eax
	andl	$0x7FFFFFFF, %eax
	movl	%eax, %cr0

	xorl	%eax, %eax
	movl	%eax, %cr3

	#
	# Reload CS.
	# Now we add 7B00h because we need to force the segment
	# address and selector to be the same.
	#

	.byte	0xEA
	.long	pm_reload + 0x7B00
	.word	0x10

pm_reload:

.code16

	#
	# Reload DS, ES, FS, GS & SS.
	#

	movw	$0x18, %ax
	movw	%ax, %ds
	movw	%ax, %es
	movw	%ax, %fs
	movw	%ax, %gs
	movw	%ax, %ss

	#
	# Switch to real mode.  Clear PE bit in CR0.
	#

	movl	%cr0, %eax
	andl	$0xFFFFFFFE, %eax
	movl	%eax, %cr0

	#
	# Reload CS.
	#

	.byte	0xEA
	.word	rm_reload + 0x7C00
	.word	0

rm_reload:

	#
	# Reload SS & SP.
	#

	xorw	%ax, %ax
	movw	%ax, %ss
	movw	$0x7BFE, %sp

	#
	# Start running setup.S
	#

	.byte	0xEA
	.word	0
	.word	0x9020

	#
	# GDT & IDT stuff for switching into real mode.
	#

gdt:	.word	0, 0, 0, 0		# unused (00h)
	.word	0, 0, 0, 0		# dummy (08h)
	.word	0xFFFF, 0x100		# code (10h)
	.word	0x9A00, 0
	.word	0xFFFF, 0x180		# data (18h)
	.word	0x9200, 0

gdt_48:	.word	0x08 * 0x400
	.long	gdt + 0x7C00

idt_48:	.word	0x400
	.long	0

	#
	# Be careful not to exceed 1F0h or the the bootsect.S
	# parameters will be lost!
	#

.end
