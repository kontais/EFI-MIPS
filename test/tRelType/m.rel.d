
m.rel:     file format elf32-tradlittlemips

SYMBOL TABLE:
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .gnu.attributes	00000000 .gnu.attributes
00000010 l    d  .mdebug.abi32	00000000 .mdebug.abi32
00000000 l    df *ABS*	00000000 m.c
00000000 l     O .bss	00000008 base
00000000 g     F .text	00000038 tm



Disassembly of section .text:

00000000 <tm>:
   0:	27bdfff8 	addiu	sp,sp,-8
   4:	afbe0004 	sw	s8,4(sp)
   8:	03a0f021 	move	s8,sp
   c:	3c030000 	lui	v1,0x0
			c: R_MIPS_HI16	.bss
  10:	3c020000 	lui	v0,0x0
			10: R_MIPS_HI16	.bss
  14:	24420000 	addiu	v0,v0,0
			14: R_MIPS_LO16	.bss
  18:	ac620000 	sw	v0,0(v1)
			18: R_MIPS_LO16	.bss
  1c:	3c020000 	lui	v0,0x0
			1c: R_MIPS_HI16	.bss
  20:	ac400004 	sw	zero,4(v0)
			20: R_MIPS_LO16	.bss
  24:	03c0e821 	move	sp,s8
  28:	8fbe0004 	lw	s8,4(sp)
  2c:	27bd0008 	addiu	sp,sp,8
  30:	03e00008 	jr	ra
  34:	00000000 	nop
	...

Disassembly of section .bss:

00000000 <base>:
	...

Disassembly of section .comment:

00000000 <.comment>:
   0:	43434700 	c0	0x1434700
   4:	5228203a 	0x5228203a
   8:	20535941 	addi	s3,v0,22849
   c:	2e332e34 	sltiu	s3,s1,11828
  10:	2e342d30 	sltiu	s4,s1,11568
  14:	73796172 	0x73796172
  18:	34202930 	ori	zero,at,0x2930
  1c:	312e332e 	andi	t6,t1,0x332e
  20:	30303220 	andi	s0,at,0x3220
  24:	30353038 	andi	s5,at,0x3038
  28:	70282031 	0x70282031
  2c:	65726572 	0x65726572
  30:	7361656c 	0x7361656c
  34:	Address 0x0000000000000034 is out of bounds.


Disassembly of section .gnu.attributes:

00000000 <.gnu.attributes>:
   0:	00000f41 	0xf41
   4:	756e6700 	jalx	5b99c00 <tm+0x5b99c00>
   8:	00070100 	sll	zero,a3,0x4
   c:	01040000 	0x1040000
