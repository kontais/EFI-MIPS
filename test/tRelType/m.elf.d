
m.elf:     file format elf32-tradlittlemips


Disassembly of section .text:

00400000 <tm>:
  400000:	27bdfff8 	addiu	sp,sp,-8
  400004:	afbe0004 	sw	s8,4(sp)
  400008:	03a0f021 	move	s8,sp
  40000c:	3c030040 	lui	v1,0x40
  400010:	3c020040 	lui	v0,0x40
  400014:	24420040 	addiu	v0,v0,64
  400018:	ac620040 	sw	v0,64(v1)
  40001c:	3c020040 	lui	v0,0x40
  400020:	ac400044 	sw	zero,68(v0)
  400024:	03c0e821 	move	sp,s8
  400028:	8fbe0004 	lw	s8,4(sp)
  40002c:	27bd0008 	addiu	sp,sp,8
  400030:	03e00008 	jr	ra
  400034:	00000000 	nop
	...
