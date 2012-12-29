
a:     file format elf32-tradlittlemips


Disassembly of section .interp:

00400154 <.interp>:
  400154:	62696c2f 	daddi	t1,s3,27695
  400158:	2e646c2f 	sltiu	a0,s3,27695
  40015c:	312e6f73 	andi	t6,t1,0x6f73
	...

Disassembly of section .note.ABI-tag:

00400164 <.note.ABI-tag>:
  400164:	00000004 	sllv	zero,zero,zero
  400168:	00000010 	mfhi	zero
  40016c:	00000001 	0x1
  400170:	00554e47 	0x554e47
  400174:	00000000 	nop
  400178:	00000002 	srl	zero,zero,0x0
  40017c:	00000006 	srlv	zero,zero,zero
  400180:	00000012 	mflo	zero

Disassembly of section .reginfo:

00400184 <.reginfo>:
  400184:	b20001f6 	sdl	zero,502(s0)
	...
  400198:	00418d10 	0x418d10

Disassembly of section .note.gnu.build-id:

0040019c <.note.gnu.build-id>:
  40019c:	00000004 	sllv	zero,zero,zero
  4001a0:	00000014 	dsllv	zero,zero,zero
  4001a4:	00000003 	sra	zero,zero,0x0
  4001a8:	00554e47 	0x554e47
  4001ac:	4ce62d32 	0x4ce62d32
  4001b0:	97dd41cf 	lhu	sp,16847(s8)
  4001b4:	f9553976 	sdc2	$21,14710(t2)
  4001b8:	cf9f4095 	lwc3	$31,16533(gp)
  4001bc:	73f12c88 	0x73f12c88

Disassembly of section .dynamic:

004001c0 <_DYNAMIC>:
  4001c0:	00000001 	0x1
  4001c4:	00000068 	0x68
  4001c8:	0000000c 	syscall
  4001cc:	004004a8 	0x4004a8
  4001d0:	0000000d 	break
  4001d4:	00400c70 	tge	v0,zero,0x31
  4001d8:	00000004 	sllv	zero,zero,zero
  4001dc:	00400298 	0x400298
  4001e0:	00000005 	0x5
  4001e4:	004003c4 	0x4003c4
  4001e8:	00000006 	srlv	zero,zero,zero
  4001ec:	004002e4 	0x4002e4
  4001f0:	0000000a 	0xa
  4001f4:	000000a6 	0xa6
  4001f8:	0000000b 	0xb
  4001fc:	00000010 	mfhi	zero
  400200:	70000016 	0x70000016
  400204:	00410d10 	0x410d10
  400208:	00000015 	0x15
  40020c:	00000000 	nop
  400210:	00000003 	sra	zero,zero,0x0
  400214:	00410d20 	0x410d20
  400218:	70000001 	0x70000001
  40021c:	00000001 	0x1
  400220:	70000005 	0x70000005
  400224:	00000002 	srl	zero,zero,0x0
  400228:	70000006 	0x70000006
  40022c:	00400000 	0x400000
  400230:	7000000a 	0x7000000a
  400234:	00000007 	srav	zero,zero,zero
  400238:	70000011 	0x70000011
  40023c:	0000000e 	0xe
  400240:	70000012 	0x70000012
  400244:	00000026 	xor	zero,zero,zero
  400248:	70000013 	0x70000013
  40024c:	00000004 	sllv	zero,zero,zero
  400250:	6ffffffe 	ldr	ra,-2(ra)
  400254:	00400488 	0x400488
  400258:	6fffffff 	ldr	ra,-1(ra)
  40025c:	00000001 	0x1
  400260:	6ffffff0 	ldr	ra,-16(ra)
  400264:	0040046a 	0x40046a
	...

Disassembly of section .hash:

00400298 <.hash>:
  400298:	00000003 	sra	zero,zero,0x0
  40029c:	0000000e 	0xe
  4002a0:	00000004 	sllv	zero,zero,zero
  4002a4:	00000005 	0x5
  4002a8:	00000003 	sra	zero,zero,0x0
	...
  4002b8:	00000007 	srav	zero,zero,zero
  4002bc:	00000006 	srlv	zero,zero,zero
  4002c0:	00000008 	jr	zero
  4002c4:	0000000a 	0xa
  4002c8:	00000002 	srl	zero,zero,0x0
  4002cc:	00000009 	jalr	zero,zero
  4002d0:	00000001 	0x1
  4002d4:	0000000b 	0xb
  4002d8:	0000000c 	syscall
  4002dc:	0000000d 	break
  4002e0:	00000000 	nop

Disassembly of section .dynsym:

004002e4 <.dynsym>:
	...
  4002f4:	00000072 	tlt	zero,zero,0x1
  4002f8:	00000001 	0x1
  4002fc:	00000000 	nop
  400300:	fff10013 	sd	s1,19(ra)
  400304:	0000008d 	break	0x0,0x2
  400308:	00400cc0 	0x400cc0
  40030c:	00000004 	sllv	zero,zero,zero
  400310:	000f0011 	0xf0011
  400314:	00000083 	sra	zero,zero,0x2
  400318:	00410d10 	0x410d10
  40031c:	00000000 	nop
  400320:	00150011 	0x150011
  400324:	0000000b 	0xb
  400328:	004004a8 	0x4004a8
  40032c:	00000000 	nop
  400330:	000b0012 	0xb0012
  400334:	0000002e 	dneg	zero,zero
  400338:	00400878 	0x400878
  40033c:	00000124 	0x124
  400340:	000c0012 	0xc0012
  400344:	0000005c 	0x5c
  400348:	00410d80 	0x410d80
  40034c:	00000190 	0x190
  400350:	00190011 	0x190011
  400354:	00000061 	0x61
  400358:	00400c50 	0x400c50
  40035c:	00000000 	nop
  400360:	00000012 	mflo	zero
  400364:	00000001 	0x1
  400368:	00400b28 	0x400b28
  40036c:	000000b0 	tge	zero,zero,0x2
  400370:	000c0012 	0xc0012
  400374:	00000021 	move	zero,zero
  400378:	00400c40 	0x400c40
  40037c:	00000000 	nop
  400380:	00000012 	mflo	zero
  400384:	00000056 	0x56
  400388:	00410d68 	0x410d68
  40038c:	00000004 	sllv	zero,zero,zero
  400390:	00180011 	0x180011
  400394:	00000042 	srl	zero,zero,0x1
	...
  4003a0:	00000020 	add	zero,zero,zero
  4003a4:	00000033 	tltu	zero,zero
	...
  4003b0:	00000022 	neg	zero,zero
  4003b4:	00000011 	mthi	zero
  4003b8:	00400b20 	0x400b20
  4003bc:	00000008 	jr	zero
  4003c0:	000c0012 	0xc0012

Disassembly of section .dynstr:

004003c4 <.dynstr>:
  4003c4:	6c5f5f00 	ldr	ra,24320(v0)
  4003c8:	5f636269 	0x5f636269
  4003cc:	5f757363 	0x5f757363
  4003d0:	74696e69 	jalx	1a5b9a4 <_gp+0x1642c94>
  4003d4:	6c5f5f00 	ldr	ra,24320(v0)
  4003d8:	5f636269 	0x5f636269
  4003dc:	5f757363 	0x5f757363
  4003e0:	696e6966 	ldl	t6,26982(t3)
  4003e4:	6c5f5f00 	ldr	ra,24320(v0)
  4003e8:	5f636269 	0x5f636269
  4003ec:	72617473 	0x72617473
  4003f0:	616d5f74 	daddi	t5,t3,24436
  4003f4:	5f006e69 	bgtzl	t8,41bd9c <_gp+0x308c>
  4003f8:	6f6d675f 	ldr	t5,26463(k1)
  4003fc:	74735f6e 	jalx	1cd7db8 <_gp+0x18bf0a8>
  400400:	5f747261 	0x5f747261
  400404:	4a5f005f 	c2	0x5f005f
  400408:	65525f76 	daddiu	s2,t2,24438
  40040c:	74736967 	jalx	1cda59c <_gp+0x18c188c>
  400410:	6c437265 	ldr	v1,29285(v0)
  400414:	65737361 	daddiu	s3,t3,29537
  400418:	6e490073 	ldr	t1,115(s2)
  40041c:	00786564 	0x786564
  400420:	61746144 	daddi	s4,t3,24900
  400424:	69727000 	ldl	s2,28672(t3)
  400428:	0066746e 	0x66746e
  40042c:	6362696c 	daddi	v0,k1,26988
  400430:	2e6f732e 	sltiu	t7,s3,29486
  400434:	445f0036 	0x445f0036
  400438:	4d414e59 	0x4d414e59
  40043c:	4c5f4349 	0x4c5f4349
  400440:	494b4e49 	0x494b4e49
  400444:	5f00474e 	bgtzl	t8,412180 <_end+0x1270>
  400448:	444c525f 	0x444c525f
  40044c:	50414d5f 	beql	v0,at,4139cc <_end+0x2abc>
  400450:	4f495f00 	c3	0x1495f00
  400454:	6474735f 	daddiu	s4,v1,29535
  400458:	755f6e69 	jalx	57db9a4 <_gp+0x53c2c94>
  40045c:	00646573 	tltu	v1,a0,0x195
  400460:	42494c47 	c0	0x494c47
  400464:	2e325f43 	sltiu	s2,s1,24387
  400468:	Address 0x0000000000400468 is out of bounds.


Disassembly of section .gnu.version:

0040046a <.gnu.version>:
  40046a:	00010000 	sll	zero,at,0x0
  40046e:	00010001 	0x10001
  400472:	00010001 	0x10001
  400476:	00020001 	0x20001
  40047a:	00020001 	0x20001
  40047e:	00000001 	0x1
  400482:	00010000 	sll	zero,at,0x0

Disassembly of section .gnu.version_r:

00400488 <.gnu.version_r>:
  400488:	00010001 	0x10001
  40048c:	00000068 	0x68
  400490:	00000010 	mfhi	zero
  400494:	00000000 	nop
  400498:	0d696910 	jal	5a5a440 <_gp+0x5641730>
  40049c:	00020000 	sll	zero,v0,0x0
  4004a0:	0000009c 	0x9c
  4004a4:	00000000 	nop

Disassembly of section .init:

004004a8 <_init>:
  4004a8:	3c1c0002 	lui	gp,0x2
  4004ac:	279c8868 	addiu	gp,gp,-30616
  4004b0:	0399e021 	addu	gp,gp,t9
  4004b4:	27bdffe0 	addiu	sp,sp,-32
  4004b8:	afbf001c 	sw	ra,28(sp)
  4004bc:	afbc0010 	sw	gp,16(sp)
  4004c0:	8f82804c 	lw	v0,-32692(gp)
  4004c4:	00200825 	move	at,at
  4004c8:	10400005 	beqz	v0,4004e0 <_init+0x38>
  4004cc:	00200825 	move	at,at
  4004d0:	8f99804c 	lw	t9,-32692(gp)
  4004d4:	00200825 	move	at,at
  4004d8:	0320f809 	jalr	t9
  4004dc:	00200825 	move	at,at
  4004e0:	04110001 	bal	4004e8 <_init+0x40>
  4004e4:	00200825 	move	at,at
  4004e8:	3c1c0042 	lui	gp,0x42
  4004ec:	279c8d10 	addiu	gp,gp,-29424
  4004f0:	8f998018 	lw	t9,-32744(gp)
  4004f4:	00200825 	move	at,at
  4004f8:	27390644 	addiu	t9,t9,1604
  4004fc:	04110051 	bal	400644 <frame_dummy>
  400500:	00200825 	move	at,at
  400504:	04110001 	bal	40050c <_init+0x64>
  400508:	00200825 	move	at,at
  40050c:	3c1c0042 	lui	gp,0x42
  400510:	279c8d10 	addiu	gp,gp,-29424
  400514:	8f998018 	lw	t9,-32744(gp)
  400518:	00200825 	move	at,at
  40051c:	27390be0 	addiu	t9,t9,3040
  400520:	041101af 	bal	400be0 <__do_global_ctors_aux>
  400524:	00200825 	move	at,at
  400528:	8fbf001c 	lw	ra,28(sp)
  40052c:	00200825 	move	at,at
  400530:	03e00008 	jr	ra
  400534:	27bd0020 	addiu	sp,sp,32

Disassembly of section .text:

00400540 <__start>:
  400540:	03e00021 	move	zero,ra
  400544:	04110001 	bal	40054c <__start+0xc>
  400548:	00200825 	move	at,at
  40054c:	3c1c0042 	lui	gp,0x42
  400550:	279c8d10 	addiu	gp,gp,-29424
  400554:	0000f821 	move	ra,zero
  400558:	8f848030 	lw	a0,-32720(gp)
  40055c:	8fa50000 	lw	a1,0(sp)
  400560:	27a60004 	addiu	a2,sp,4
  400564:	2401fff8 	li	at,-8
  400568:	03a1e824 	and	sp,sp,at
  40056c:	27bdffe0 	addiu	sp,sp,-32
  400570:	8f87803c 	lw	a3,-32708(gp)
  400574:	8f888050 	lw	t0,-32688(gp)
  400578:	00200825 	move	at,at
  40057c:	afa80010 	sw	t0,16(sp)
  400580:	afa20014 	sw	v0,20(sp)
  400584:	afbd0018 	sw	sp,24(sp)
  400588:	8f998040 	lw	t9,-32704(gp)
  40058c:	00200825 	move	at,at
  400590:	0320f809 	jalr	t9
  400594:	00200825 	move	at,at

00400598 <hlt>:
  400598:	1000ffff 	b	400598 <hlt>
  40059c:	00200825 	move	at,at

004005a0 <__do_global_dtors_aux>:
  4005a0:	27bdffd0 	addiu	sp,sp,-48
  4005a4:	afb30028 	sw	s3,40(sp)
  4005a8:	3c130041 	lui	s3,0x41
  4005ac:	92620d70 	lbu	v0,3440(s3)
  4005b0:	afbf002c 	sw	ra,44(sp)
  4005b4:	afb20024 	sw	s2,36(sp)
  4005b8:	afb10020 	sw	s1,32(sp)
  4005bc:	1440001a 	bnez	v0,400628 <__do_global_dtors_aux+0x88>
  4005c0:	afb0001c 	sw	s0,28(sp)
  4005c4:	3c120041 	lui	s2,0x41
  4005c8:	3c110041 	lui	s1,0x41
  4005cc:	26520cec 	addiu	s2,s2,3308
  4005d0:	26310cf0 	addiu	s1,s1,3312
  4005d4:	02328823 	subu	s1,s1,s2
  4005d8:	3c100041 	lui	s0,0x41
  4005dc:	00118883 	sra	s1,s1,0x2
  4005e0:	8e020d74 	lw	v0,3444(s0)
  4005e4:	2631ffff 	addiu	s1,s1,-1
  4005e8:	0051182b 	sltu	v1,v0,s1
  4005ec:	1060000c 	beqz	v1,400620 <__do_global_dtors_aux+0x80>
  4005f0:	24420001 	addiu	v0,v0,1
  4005f4:	00021880 	sll	v1,v0,0x2
  4005f8:	02431821 	addu	v1,s2,v1
  4005fc:	8c790000 	lw	t9,0(v1)
  400600:	00200825 	move	at,at
  400604:	0320f809 	jalr	t9
  400608:	ae020d74 	sw	v0,3444(s0)
  40060c:	8e020d74 	lw	v0,3444(s0)
  400610:	00200825 	move	at,at
  400614:	0051182b 	sltu	v1,v0,s1
  400618:	1460fff6 	bnez	v1,4005f4 <__do_global_dtors_aux+0x54>
  40061c:	24420001 	addiu	v0,v0,1
  400620:	24020001 	li	v0,1
  400624:	a2620d70 	sb	v0,3440(s3)
  400628:	8fbf002c 	lw	ra,44(sp)
  40062c:	8fb30028 	lw	s3,40(sp)
  400630:	8fb20024 	lw	s2,36(sp)
  400634:	8fb10020 	lw	s1,32(sp)
  400638:	8fb0001c 	lw	s0,28(sp)
  40063c:	03e00008 	jr	ra
  400640:	27bd0030 	addiu	sp,sp,48

00400644 <frame_dummy>:
  400644:	3c040041 	lui	a0,0x41
  400648:	8c820cf4 	lw	v0,3316(a0)
  40064c:	3c1c0042 	lui	gp,0x42
  400650:	10400007 	beqz	v0,400670 <frame_dummy+0x2c>
  400654:	279c8d10 	addiu	gp,gp,-29424
  400658:	8f998048 	lw	t9,-32696(gp)
  40065c:	00200825 	move	at,at
  400660:	13200003 	beqz	t9,400670 <frame_dummy+0x2c>
  400664:	00200825 	move	at,at
  400668:	03200008 	jr	t9
  40066c:	24840cf4 	addiu	a0,a0,3316
  400670:	03e00008 	jr	ra
  400674:	00200825 	move	at,at
  400678:	00200825 	move	at,at
  40067c:	00200825 	move	at,at

00400680 <Print>:
  400680:	27bdffd8 	addiu	sp,sp,-40
  400684:	afbe0024 	sw	s8,36(sp)
  400688:	03a0f021 	move	s8,sp
  40068c:	3c1c0042 	lui	gp,0x42
  400690:	279c8d10 	addiu	gp,gp,-29424
  400694:	afbc0000 	sw	gp,0(sp)
  400698:	afc5002c 	sw	a1,44(s8)  ---a1 BufSize
  40069c:	afc60030 	sw	a2,48(s8)  ---a2 v0 4681 FileSize
  4006a0:	afc70034 	sw	a3,52(s8)  ---a3 v1 0
  4006a4:	afc40028 	sw	a0,40(s8)  ---a0 fmt的地址
  4006a8:	27c40028 	addiu	a0,s8,40   -a0是fmt的地址
  4006ac:	24840004 	addiu	a0,a0,4    --- args = (CHAR8 *) & (fmt) + 4; 这一行的 +4
  4006b0:	afc4001c 	sw	a0,28(s8)  a0 = 40+s8 + 4 = args 地址保存到28(sp)
  4006b4:	8fc40028 	lw	a0,40(s8)  a0 = pc
  4006b8:	00200825 	move	at,at
  4006bc:	afc40018 	sw	a0,24(s8)  a0 = pc保存到24(sp)
  4006c0:	10000061 	b	400848 <Print+0x1c8>
  4006c4:	00200825 	move	at,at
  4006c8:	8fc40018 	lw	a0,24(s8) a0 = pc
  4006cc:	00200825 	move	at,at
  4006d0:	94840000 	lhu	a0,0(a0)  ---switch (*pc)的*pc最值
  4006d4:	24050064 	li	a1,100   --------------------- a1 = 'd'
  4006d8:	1085002d 	beq	a0,a1,400790 <Print+0x110> ---> 处理'd'
  4006dc:	00200825 	move	at,at
  4006e0:	24050078 	li	a1,120   --------------------- a1 = 'x'
  4006e4:	14850054 	bne	a0,a1,400838 <Print+0x1b8>
  4006e8:	00200825 	move	at,at    -->处理'x'
  4006ec:	8fc4001c 	lw	a0,28(s8)  ---a0 = args = 40+s8+4 = 44+s8
  4006f0:	00200825 	move	at,at
  4006f4:	24840004 	addiu	a0,a0,4 ------ a0 = args +4 ,args+=4
  4006f8:	afc4001c 	sw	a0,28(s8)  保存args+4 = 48+s8
  4006fc:	8fc4001c 	lw	a0,28(s8)  又取出来
  400700:	00200825 	move	at,at
  400704:	2484fffc 	addiu	a0,a0,-4   a0 = 44+s8, uv = (*(UINTN *) ((args += 4) - 4));  这一行的 -4
  400708:	8c840000 	lw	a0,0(a0)  a0 = BufSize,一个UINT64是，这个是无意义
  40070c:	00200825 	move	at,at
  400710:	afc40010 	sw	a0,16(s8)
  400714:	afc00014 	sw	zero,20(s8)
  400718:	8f848044 	lw	a0,-32700(gp)
  40071c:	00200825 	move	at,at
  400720:	8c840000 	lw	a0,0(a0)
  400724:	8fc60010 	lw	a2,16(s8)      ------uv low
  400728:	8f878034 	lw	a3,-32716(gp)  ------uv low
  40072c:	00042880 	sll	a1,a0,0x2      ------uv low
  400730:	00e52821 	addu	a1,a3,a1       ------uv low
  400734:	aca60000 	sw	a2,0(a1)       ------uv low
  400738:	24850001 	addiu	a1,a0,1        ------uv low
  40073c:	8f848044 	lw	a0,-32700(gp)  ------uv low
  400740:	00200825 	move	at,at          ------uv low
  400744:	ac850000 	sw	a1,0(a0)       ------uv low
  400748:	8f848044 	lw	a0,-32700(gp)  ------uv low
  40074c:	00200825 	move	at,at          ------uv low
  400750:	8c840000 	lw	a0,0(a0)       ------uv low
  400754:	8fc50014 	lw	a1,20(s8)
  400758:	00200825 	move	at,at
  40075c:	00054002 	srl	t0,a1,0x0
  400760:	00004821 	move	t1,zero
  400764:	01003021 	move	a2,t0
  400768:	8f878034 	lw	a3,-32716(gp)
  40076c:	00042880 	sll	a1,a0,0x2
  400770:	00e52821 	addu	a1,a3,a1
  400774:	aca60000 	sw	a2,0(a1)
  400778:	24850001 	addiu	a1,a0,1
  40077c:	8f848044 	lw	a0,-32700(gp)
  400780:	00200825 	move	at,at
  400784:	ac850000 	sw	a1,0(a0)
  400788:	1000002b 	b	400838 <Print+0x1b8>
  40078c:	00200825 	move	at,at
  400790:	8fc4001c 	lw	a0,28(s8)   ---a0 = args = 40+s8 + 4 = 44+sp
  400794:	00200825 	move	at,at
  400798:	24840008 	addiu	a0,a0,8 --a0 = 44+sp+8 = 52+sp
  40079c:	afc4001c 	sw	a0,28(s8)  保存
  4007a0:	8fc4001c 	lw	a0,28(s8)  取出
  4007a4:	00200825 	move	at,at
  4007a8:	2484fff8 	addiu	a0,a0,-8   a0 = 52+sp -8 = 44+sp, sv = (*(UINT64 *) ((args += 8) - 8)); 这一行的 -8
  4007ac:	8c850004 	lw	a1,4(a0) 高位, 48+sp取了v0 4681 FileSize
  4007b0:	8c840000 	lw	a0,0(a0) 低位，一个UINT64是，随机值******************问题在这**********
  4007b4:	00200825 	move	at,at
  4007b8:	afc40008 	sw	a0,8(s8)
  4007bc:	afc5000c 	sw	a1,12(s8)
  4007c0:	8f848044 	lw	a0,-32700(gp)
  4007c4:	00200825 	move	at,at
  4007c8:	8c840000 	lw	a0,0(a0)
  4007cc:	8fc60008 	lw	a2,8(s8)         -----sv low
  4007d0:	8f878034 	lw	a3,-32716(gp)    -----sv low
  4007d4:	00042880 	sll	a1,a0,0x2        -----sv low
  4007d8:	00e52821 	addu	a1,a3,a1         -----sv low
  4007dc:	aca60000 	sw	a2,0(a1)         -----sv low
  4007e0:	24850001 	addiu	a1,a0,1          -----sv low
  4007e4:	8f848044 	lw	a0,-32700(gp)    -----sv low
  4007e8:	00200825 	move	at,at            -----sv low
  4007ec:	ac850000 	sw	a1,0(a0)         -----sv low
  4007f0:	8f848044 	lw	a0,-32700(gp)    -----sv low
  4007f4:	00200825 	move	at,at            -----sv low
  4007f8:	8c840000 	lw	a0,0(a0)         -----sv low
  4007fc:	8fc5000c 	lw	a1,12(s8)
  400800:	00200825 	move	at,at
  400804:	00051003 	sra	v0,a1,0x0
  400808:	8fc5000c 	lw	a1,12(s8)
  40080c:	00200825 	move	at,at
  400810:	00051fc3 	sra	v1,a1,0x1f
  400814:	00403021 	move	a2,v0
  400818:	8f878034 	lw	a3,-32716(gp)
  40081c:	00042880 	sll	a1,a0,0x2
  400820:	00e52821 	addu	a1,a3,a1
  400824:	aca60000 	sw	a2,0(a1)
  400828:	24850001 	addiu	a1,a0,1
  40082c:	8f848044 	lw	a0,-32700(gp)
  400830:	00200825 	move	at,at
  400834:	ac850000 	sw	a1,0(a0)
  400838:	8fc40018 	lw	a0,24(s8)
  40083c:	00200825 	move	at,at
  400840:	24840002 	addiu	a0,a0,2     ---- pc++
  400844:	afc40018 	sw	a0,24(s8)   ---- pc++
  400848:	8fc40018 	lw	a0,24(s8)   ---a0 = pc
  40084c:	00200825 	move	at,at
  400850:	94840000 	lhu	a0,0(a0)    ---- while (*pc)的*pc，取一个半字
  400854:	00200825 	move	at,at
  400858:	1480ff9b 	bnez	a0,4006c8 <Print+0x48>  while (*pc), 如果不是0
  40085c:	00200825 	move	at,at
  400860:	00001021 	move	v0,zero
  400864:	03c0e821 	move	sp,s8
  400868:	8fbe0024 	lw	s8,36(sp)
  40086c:	27bd0028 	addiu	sp,sp,40
  400870:	03e00008 	jr	ra
  400874:	00200825 	move	at,at

00400878 <main>:
  400878:	27bdffc8 	addiu	sp,sp,-56
  40087c:	afbf0034 	sw	ra,52(sp)
  400880:	afbe0030 	sw	s8,48(sp)
  400884:	03a0f021 	move	s8,sp
  400888:	3c1c0042 	lui	gp,0x42
  40088c:	279c8d10 	addiu	gp,gp,-29424
  400890:	afbc0010 	sw	gp,16(sp)
  400894:	24020064 	li	v0,100
  400898:	afc20028 	sw	v0,40(s8)
  40089c:	24021249 	li	v0,4681
  4008a0:	00001821 	move	v1,zero
  4008a4:	afc20020 	sw	v0,32(s8)
  4008a8:	afc30024 	sw	v1,36(s8)
  4008ac:	3c020040 	lui	v0,0x40
  4008b0:	24440cd0 	addiu	a0,v0,3280
  4008b4:	8fc50028 	lw	a1,40(s8)
  4008b8:	8fc60020 	lw	a2,32(s8)
  4008bc:	8fc70024 	lw	a3,36(s8)
  4008c0:	0c1001a0 	jal	400680 <Print>
  4008c4:	00200825 	move	at,at
  4008c8:	8fdc0010 	lw	gp,16(s8)
  4008cc:	3c020040 	lui	v0,0x40
  4008d0:	24440cd8 	addiu	a0,v0,3288
  4008d4:	8fc60020 	lw	a2,32(s8)
  4008d8:	8fc70024 	lw	a3,36(s8)
  4008dc:	0c1001a0 	jal	400680 <Print>
  4008e0:	00200825 	move	at,at
  4008e4:	8fdc0010 	lw	gp,16(s8)
  4008e8:	3c020040 	lui	v0,0x40
  4008ec:	24440cd0 	addiu	a0,v0,3280
  4008f0:	8fc50028 	lw	a1,40(s8)
  4008f4:	8fc60020 	lw	a2,32(s8)
  4008f8:	8fc70024 	lw	a3,36(s8)
  4008fc:	0c1001a0 	jal	400680 <Print>
  400900:	00200825 	move	at,at
  400904:	8fdc0010 	lw	gp,16(s8)
  400908:	afc00018 	sw	zero,24(s8)
  40090c:	10000014 	b	400960 <main+0xe8>
  400910:	00200825 	move	at,at
  400914:	3c020040 	lui	v0,0x40
  400918:	24430cdc 	addiu	v1,v0,3292
  40091c:	8fc20018 	lw	v0,24(s8)
  400920:	8f848034 	lw	a0,-32716(gp)
  400924:	00021080 	sll	v0,v0,0x2
  400928:	00821021 	addu	v0,a0,v0
  40092c:	8c420000 	lw	v0,0(v0)
  400930:	00602021 	move	a0,v1
  400934:	00402821 	move	a1,v0
  400938:	8f828038 	lw	v0,-32712(gp)
  40093c:	00200825 	move	at,at
  400940:	0040c821 	move	t9,v0
  400944:	0320f809 	jalr	t9
  400948:	00200825 	move	at,at
  40094c:	8fdc0010 	lw	gp,16(s8)
  400950:	8fc20018 	lw	v0,24(s8)
  400954:	00200825 	move	at,at
  400958:	24420001 	addiu	v0,v0,1
  40095c:	afc20018 	sw	v0,24(s8)
  400960:	8f828044 	lw	v0,-32700(gp)
  400964:	00200825 	move	at,at
  400968:	8c420000 	lw	v0,0(v0)
  40096c:	8fc30018 	lw	v1,24(s8)
  400970:	00200825 	move	at,at
  400974:	0062102b 	sltu	v0,v1,v0
  400978:	1440ffe6 	bnez	v0,400914 <main+0x9c>
  40097c:	00200825 	move	at,at
  400980:	00001021 	move	v0,zero
  400984:	03c0e821 	move	sp,s8
  400988:	8fbf0034 	lw	ra,52(sp)
  40098c:	8fbe0030 	lw	s8,48(sp)
  400990:	27bd0038 	addiu	sp,sp,56
  400994:	03e00008 	jr	ra
  400998:	00200825 	move	at,at
  40099c:	00200825 	move	at,at

004009a0 <DivU64x32>:
  4009a0:	0004203c 	dsll32	a0,a0,0x0
  4009a4:	0004203e 	dsrl32	a0,a0,0x0
  4009a8:	0005283c 	dsll32	a1,a1,0x0
  4009ac:	00a4282d 	daddu	a1,a1,a0
  4009b0:	14c00002 	bnez	a2,4009bc <DivU64x32+0x1c>
  4009b4:	00a6001f 	ddivu	zero,a1,a2
  4009b8:	0007000d 	break	0x7
  4009bc:	00001012 	mflo	v0
  4009c0:	00004010 	mfhi	t0
  4009c4:	0040182d 	move	v1,v0
  4009c8:	0003183e 	dsrl32	v1,v1,0x0
  4009cc:	10e00003 	beqz	a3,4009dc <DivU64x32+0x3c>
  4009d0:	00200825 	move	at,at
  4009d4:	ace80000 	sw	t0,0(a3)
  4009d8:	00200825 	move	at,at
  4009dc:	03e00008 	jr	ra
  4009e0:	00200825 	move	at,at

004009e4 <MultU64x32>:
  4009e4:	0004203c 	dsll32	a0,a0,0x0
  4009e8:	0004203e 	dsrl32	a0,a0,0x0
  4009ec:	0005283c 	dsll32	a1,a1,0x0
  4009f0:	00a4282d 	daddu	a1,a1,a0
  4009f4:	00a6001d 	dmultu	a1,a2
  4009f8:	00001012 	mflo	v0
  4009fc:	0040182d 	move	v1,v0
  400a00:	0003183e 	dsrl32	v1,v1,0x0
  400a04:	03e00008 	jr	ra
  400a08:	00200825 	move	at,at

00400a0c <LShiftU64>:
  400a0c:	0004203c 	dsll32	a0,a0,0x0
  400a10:	0004203e 	dsrl32	a0,a0,0x0
  400a14:	0005283c 	dsll32	a1,a1,0x0
  400a18:	00a4282d 	daddu	a1,a1,a0
  400a1c:	00c51014 	dsllv	v0,a1,a2
  400a20:	0040182d 	move	v1,v0
  400a24:	0003183e 	dsrl32	v1,v1,0x0
  400a28:	03e00008 	jr	ra
  400a2c:	00200825 	move	at,at

00400a30 <RShiftU64>:
  400a30:	0004203c 	dsll32	a0,a0,0x0
  400a34:	0004203e 	dsrl32	a0,a0,0x0
  400a38:	0005283c 	dsll32	a1,a1,0x0
  400a3c:	00a4282d 	daddu	a1,a1,a0
  400a40:	00c51016 	dsrlv	v0,a1,a2
  400a44:	0040182d 	move	v1,v0
  400a48:	0003183e 	dsrl32	v1,v1,0x0
  400a4c:	03e00008 	jr	ra
  400a50:	00200825 	move	at,at

00400a54 <Log2>:
  400a54:	240200ff 	li	v0,255
  400a58:	00854025 	or	t0,a0,a1
  400a5c:	1100000f 	beqz	t0,400a9c <Log2+0x48>
  400a60:	00200825 	move	at,at
  400a64:	0004203c 	dsll32	a0,a0,0x0
  400a68:	0004203e 	dsrl32	a0,a0,0x0
  400a6c:	0005283c 	dsll32	a1,a1,0x0
  400a70:	00a4282d 	daddu	a1,a1,a0
  400a74:	00001021 	move	v0,zero
  400a78:	0005203e 	dsrl32	a0,a1,0x0
  400a7c:	00852025 	or	a0,a0,a1
  400a80:	10800005 	beqz	a0,400a98 <Log2+0x44>
  400a84:	00200825 	move	at,at
  400a88:	0005287a 	dsrl	a1,a1,0x1
  400a8c:	20420001 	addi	v0,v0,1
  400a90:	1000fff9 	b	400a78 <Log2+0x24>
  400a94:	00200825 	move	at,at
  400a98:	2442ffff 	addiu	v0,v0,-1
  400a9c:	03e00008 	jr	ra
  400aa0:	00200825 	move	at,at

00400aa4 <GetPowerOfTwo>:
  400aa4:	00001021 	move	v0,zero
  400aa8:	00001821 	move	v1,zero
  400aac:	00854025 	or	t0,a0,a1
  400ab0:	11000019 	beqz	t0,400b18 <GetPowerOfTwo+0x74>
  400ab4:	00200825 	move	at,at
  400ab8:	0004203c 	dsll32	a0,a0,0x0
  400abc:	0004203e 	dsrl32	a0,a0,0x0
  400ac0:	0005283c 	dsll32	a1,a1,0x0
  400ac4:	00a4282d 	daddu	a1,a1,a0
  400ac8:	00001021 	move	v0,zero
  400acc:	0005203e 	dsrl32	a0,a1,0x0
  400ad0:	00852025 	or	a0,a0,a1
  400ad4:	10800005 	beqz	a0,400aec <GetPowerOfTwo+0x48>
  400ad8:	00200825 	move	at,at
  400adc:	0005287a 	dsrl	a1,a1,0x1
  400ae0:	24420001 	addiu	v0,v0,1
  400ae4:	1000fff9 	b	400acc <GetPowerOfTwo+0x28>
  400ae8:	00200825 	move	at,at
  400aec:	64a50001 	daddiu	a1,a1,1
  400af0:	2442ffff 	addiu	v0,v0,-1
  400af4:	10400005 	beqz	v0,400b0c <GetPowerOfTwo+0x68>
  400af8:	00200825 	move	at,at
  400afc:	00052878 	dsll	a1,a1,0x1
  400b00:	2442ffff 	addiu	v0,v0,-1
  400b04:	1000fffb 	b	400af4 <GetPowerOfTwo+0x50>
  400b08:	00200825 	move	at,at
  400b0c:	00a0102d 	move	v0,a1
  400b10:	00a0182d 	move	v1,a1
  400b14:	0003183e 	dsrl32	v1,v1,0x0
  400b18:	03e00008 	jr	ra
  400b1c:	00200825 	move	at,at

00400b20 <__libc_csu_fini>:
  400b20:	03e00008 	jr	ra
  400b24:	00200825 	move	at,at

00400b28 <__libc_csu_init>:
  400b28:	3c1c0002 	lui	gp,0x2
  400b2c:	279c81e8 	addiu	gp,gp,-32280
  400b30:	0399e021 	addu	gp,gp,t9
  400b34:	27bdffc8 	addiu	sp,sp,-56
  400b38:	afbf0034 	sw	ra,52(sp)
  400b3c:	afb50030 	sw	s5,48(sp)
  400b40:	afb4002c 	sw	s4,44(sp)
  400b44:	afb30028 	sw	s3,40(sp)
  400b48:	afb20024 	sw	s2,36(sp)
  400b4c:	afb10020 	sw	s1,32(sp)
  400b50:	afb0001c 	sw	s0,28(sp)
  400b54:	afbc0010 	sw	gp,16(sp)
  400b58:	8f99802c 	lw	t9,-32724(gp)
  400b5c:	00809821 	move	s3,a0
  400b60:	00a0a021 	move	s4,a1
  400b64:	0320f809 	jalr	t9
  400b68:	00c0a821 	move	s5,a2
  400b6c:	8fbc0010 	lw	gp,16(sp)
  400b70:	00200825 	move	at,at
  400b74:	8f91801c 	lw	s1,-32740(gp)
  400b78:	8f92801c 	lw	s2,-32740(gp)
  400b7c:	00200825 	move	at,at
  400b80:	02519023 	subu	s2,s2,s1
  400b84:	00129083 	sra	s2,s2,0x2
  400b88:	1240000a 	beqz	s2,400bb4 <__libc_csu_init+0x8c>
  400b8c:	00008021 	move	s0,zero
  400b90:	8e390000 	lw	t9,0(s1)
  400b94:	26100001 	addiu	s0,s0,1
  400b98:	02602021 	move	a0,s3
  400b9c:	02802821 	move	a1,s4
  400ba0:	0320f809 	jalr	t9
  400ba4:	02a03021 	move	a2,s5
  400ba8:	0212102b 	sltu	v0,s0,s2
  400bac:	1440fff8 	bnez	v0,400b90 <__libc_csu_init+0x68>
  400bb0:	26310004 	addiu	s1,s1,4
  400bb4:	8fbf0034 	lw	ra,52(sp)
  400bb8:	8fb50030 	lw	s5,48(sp)
  400bbc:	8fb4002c 	lw	s4,44(sp)
  400bc0:	8fb30028 	lw	s3,40(sp)
  400bc4:	8fb20024 	lw	s2,36(sp)
  400bc8:	8fb10020 	lw	s1,32(sp)
  400bcc:	8fb0001c 	lw	s0,28(sp)
  400bd0:	03e00008 	jr	ra
  400bd4:	27bd0038 	addiu	sp,sp,56
  400bd8:	00200825 	move	at,at
  400bdc:	00200825 	move	at,at

00400be0 <__do_global_ctors_aux>:
  400be0:	3c020041 	lui	v0,0x41
  400be4:	8c590ce4 	lw	t9,3300(v0)
  400be8:	27bdffd8 	addiu	sp,sp,-40
  400bec:	2402ffff 	li	v0,-1
  400bf0:	afbf0024 	sw	ra,36(sp)
  400bf4:	afb10020 	sw	s1,32(sp)
  400bf8:	1322000a 	beq	t9,v0,400c24 <__do_global_ctors_aux+0x44>
  400bfc:	afb0001c 	sw	s0,28(sp)
  400c00:	3c100041 	lui	s0,0x41
  400c04:	26100ce4 	addiu	s0,s0,3300
  400c08:	2411ffff 	li	s1,-1
  400c0c:	0320f809 	jalr	t9
  400c10:	2610fffc 	addiu	s0,s0,-4
  400c14:	8e190000 	lw	t9,0(s0)
  400c18:	00200825 	move	at,at
  400c1c:	1731fffb 	bne	t9,s1,400c0c <__do_global_ctors_aux+0x2c>
  400c20:	00200825 	move	at,at
  400c24:	8fbf0024 	lw	ra,36(sp)
  400c28:	8fb10020 	lw	s1,32(sp)
  400c2c:	8fb0001c 	lw	s0,28(sp)
  400c30:	03e00008 	jr	ra
  400c34:	27bd0028 	addiu	sp,sp,40
  400c38:	00200825 	move	at,at
  400c3c:	00200825 	move	at,at

Disassembly of section .MIPS.stubs:

00400c40 <.MIPS.stubs>:
  400c40:	8f998010 	lw	t9,-32752(gp)
  400c44:	03e07821 	move	t7,ra
  400c48:	0320f809 	jalr	t9
  400c4c:	24180009 	li	t8,9
  400c50:	8f998010 	lw	t9,-32752(gp)
  400c54:	03e07821 	move	t7,ra
  400c58:	0320f809 	jalr	t9
  400c5c:	24180007 	li	t8,7
	...

Disassembly of section .fini:

00400c70 <_fini>:
  400c70:	3c1c0002 	lui	gp,0x2
  400c74:	279c80a0 	addiu	gp,gp,-32608
  400c78:	0399e021 	addu	gp,gp,t9
  400c7c:	27bdffe0 	addiu	sp,sp,-32
  400c80:	afbf001c 	sw	ra,28(sp)
  400c84:	afbc0010 	sw	gp,16(sp)
  400c88:	04110001 	bal	400c90 <_fini+0x20>
  400c8c:	00200825 	move	at,at
  400c90:	3c1c0042 	lui	gp,0x42
  400c94:	279c8d10 	addiu	gp,gp,-29424
  400c98:	8f998018 	lw	t9,-32744(gp)
  400c9c:	00200825 	move	at,at
  400ca0:	273905a0 	addiu	t9,t9,1440
  400ca4:	0411fe3e 	bal	4005a0 <__do_global_dtors_aux>
  400ca8:	00200825 	move	at,at
  400cac:	8fbf001c 	lw	ra,28(sp)
  400cb0:	00200825 	move	at,at
  400cb4:	03e00008 	jr	ra
  400cb8:	27bd0020 	addiu	sp,sp,32

Disassembly of section .rodata:

00400cc0 <_IO_stdin_used>:
  400cc0:	00020001 	0x20001
	...
  400cd0:	00640078 	0x640078
  400cd4:	00000000 	nop
  400cd8:	00000064 	0x64
  400cdc:	000a6425 	0xa6425

Disassembly of section .eh_frame:

00400ce0 <__FRAME_END__>:
  400ce0:	00000000 	nop

Disassembly of section .ctors:

00410ce4 <__CTOR_LIST__>:
  410ce4:	ffffffff 	sd	ra,-1(ra)

00410ce8 <__CTOR_END__>:
  410ce8:	00000000 	nop

Disassembly of section .dtors:

00410cec <__DTOR_LIST__>:
  410cec:	ffffffff 	sd	ra,-1(ra)

00410cf0 <__DTOR_END__>:
  410cf0:	00000000 	nop

Disassembly of section .jcr:

00410cf4 <__JCR_END__>:
  410cf4:	00000000 	nop

Disassembly of section .data:

00410d00 <__data_start>:
	...

Disassembly of section .rld_map:

00410d10 <__RLD_MAP>:
  410d10:	00000000 	nop

Disassembly of section .got:

00410d20 <_GLOBAL_OFFSET_TABLE_>:
  410d20:	00000000 	nop
  410d24:	80000000 	lb	zero,0(zero)
  410d28:	00400000 	0x400000
  410d2c:	00410ce4 	0x410ce4
	...
  410d3c:	004004a8 	0x4004a8
  410d40:	00400878 	0x400878
  410d44:	00410d80 	0x410d80
  410d48:	00400c50 	0x400c50
  410d4c:	00400b28 	0x400b28
  410d50:	00400c40 	0x400c40
  410d54:	00410d68 	0x410d68
	...
  410d60:	00400b20 	0x400b20

Disassembly of section .sdata:

00410d64 <__dso_handle>:
  410d64:	00000000 	nop

Disassembly of section .sbss:

00410d68 <Index>:
  410d68:	00000000 	nop

Disassembly of section .bss:

00410d70 <completed.5789>:
  410d70:	00000000 	nop

00410d74 <dtor_idx.5791>:
	...

00410d80 <Data>:
	...

Disassembly of section .pdr:

00000000 <.pdr>:
   0:	00400680 	0x400680
   4:	40000000 	mfc0	zero,c0_index
   8:	fffffffc 	sd	ra,-4(ra)
	...
  14:	00000028 	0x28
  18:	0000001e 	ddiv	zero,zero,zero
  1c:	0000001f 	ddivu	zero,zero,zero
  20:	00400878 	0x400878
  24:	c0000000 	ll	zero,0(zero)
  28:	fffffffc 	sd	ra,-4(ra)
	...
  34:	00000038 	dsll	zero,zero,0x0
  38:	0000001e 	ddiv	zero,zero,zero
  3c:	0000001f 	ddivu	zero,zero,zero
  40:	004009a0 	0x4009a0
	...
  60:	004009e4 	0x4009e4
	...
  80:	00400a0c 	syscall	0x10028
	...
  a0:	00400a30 	tge	v0,zero,0x28
	...
  c0:	00400a54 	0x400a54
	...
  e0:	00400aa4 	0x400aa4
	...
 100:	00400b20 	0x400b20
	...
 118:	0000001d 	dmultu	zero,zero
 11c:	0000001f 	ddivu	zero,zero,zero
 120:	00400b28 	0x400b28
 124:	803f0000 	lb	ra,0(at)
 128:	fffffffc 	sd	ra,-4(ra)
	...
 134:	00000038 	dsll	zero,zero,0x0
 138:	0000001d 	dmultu	zero,zero
 13c:	0000001f 	ddivu	zero,zero,zero

Disassembly of section .comment:

00000000 <.comment>:
   0:	3a434347 	xori	v1,s2,0x4347
   4:	65442820 	daddiu	a0,t2,10272
   8:	6e616962 	ldr	at,26978(s3)
   c:	342e3420 	ori	t6,at,0x3420
  10:	382d352e 	xori	t5,at,0x352e
  14:	2e342029 	sltiu	s4,s1,8233
  18:	00352e34 	teq	at,s5,0xb8
  1c:	3a434347 	xori	v1,s2,0x4347
  20:	65442820 	daddiu	a0,t2,10272
  24:	6e616962 	ldr	at,26978(s3)
  28:	342e3420 	ori	t6,at,0x3420
  2c:	312d352e 	andi	t5,t1,0x352e
  30:	34202930 	ori	zero,at,0x2930
  34:	352e342e 	ori	t6,t1,0x342e
	...

Disassembly of section .debug_aranges:

00000000 <.debug_aranges>:
   0:	0000001c 	dmult	zero,zero
   4:	00000002 	srl	zero,zero,0x0
   8:	00040000 	sll	zero,a0,0x0
   c:	00000000 	nop
  10:	00400680 	0x400680
  14:	0000031c 	0x31c
	...
  20:	0000001c 	dmult	zero,zero
  24:	01d60002 	0x1d60002
  28:	00040000 	sll	zero,a0,0x0
  2c:	00000000 	nop
  30:	004009a0 	0x4009a0
  34:	00000180 	sll	zero,zero,0x6
	...

Disassembly of section .debug_pubnames:

00000000 <.debug_pubnames>:
   0:	00000034 	teq	zero,zero
   4:	00000002 	srl	zero,zero,0x0
   8:	01d60000 	0x1d60000
   c:	00f40000 	0xf40000
  10:	72500000 	0x72500000
  14:	00746e69 	0x746e69
  18:	0000015c 	0x15c
  1c:	6e69616d 	ldr	t1,24941(s3)
  20:	0001a100 	sll	s4,at,0x4
  24:	646e4900 	daddiu	t6,v1,18688
  28:	c3007865 	ll	zero,30821(t8)
  2c:	44000001 	0x44000001
  30:	00617461 	0x617461
  34:	00000000 	nop

Disassembly of section .debug_info:

00000000 <.debug_info>:
   0:	000001d2 	0x1d2
   4:	00000002 	srl	zero,zero,0x0
   8:	01040000 	0x1040000
   c:	000000b1 	tgeu	zero,zero,0x2
  10:	632e6101 	daddi	t6,t9,24833
  14:	00000e00 	sll	at,zero,0x18
  18:	40068000 	mfc0	a2,c0_config
  1c:	40099c00 	0x40099c00
  20:	00000000 	nop
  24:	07040200 	0x7040200
  28:	00000089 	0x89
  2c:	3b080102 	xori	t0,t8,0x102
  30:	02000000 	0x2000000
  34:	00510702 	0x510702
  38:	04020000 	bltzl	zero,3c <_DYNAMIC-0x400184>
  3c:	00008407 	0x8407
  40:	06010200 	bgez	s0,844 <_DYNAMIC-0x3ff97c>
  44:	0000003d 	0x3d
  48:	cb050202 	lwc2	$5,514(t8)
  4c:	03000000 	0x3000000
  50:	6e690504 	ldr	t1,1284(s3)
  54:	08020074 	j	801d0 <_DYNAMIC-0x37fff0>
  58:	0000a305 	0xa305
  5c:	07080200 	tgei	t8,512
  60:	0000007f 	dsra32	zero,zero,0x1
  64:	a8050402 	swl	a1,1026(zero)
  68:	04000000 	bltz	zero,6c <_DYNAMIC-0x400154>
  6c:	01020704 	0x1020704
  70:	00004406 	0x4406
  74:	00320500 	0x320500
  78:	38020000 	xori	v0,zero,0x0
  7c:	0000005d 	0x5d
  80:	0000e305 	0xe305
  84:	253a0200 	addiu	k0,t1,512
  88:	05000000 	bltz	t0,8c <_DYNAMIC-0x400134>
  8c:	000000da 	0xda
  90:	00333c02 	0x333c02
  94:	96050000 	lhu	a1,0(s0)
  98:	02000000 	0x2000000
  9c:	00002c3e 	dsrl32	a1,zero,0x10
  a0:	00490500 	0x490500
  a4:	46020000 	add.s	$f0,$f0,$f2
  a8:	00000080 	sll	zero,zero,0x2
  ac:	00000805 	0x805
  b0:	a11e0300 	sb	s8,768(t0)
  b4:	05000000 	bltz	t0,b8 <_DYNAMIC-0x400108>
  b8:	000000bd 	0xbd
  bc:	00802703 	0x802703
  c0:	c5050000 	lwc1	$f5,0(t0)
  c4:	03000000 	0x3000000
  c8:	00005628 	0x5628
  cc:	00c40500 	0xc40500
  d0:	29030000 	slti	v1,t0,0
  d4:	00000075 	0x75
  d8:	00006b05 	0x6b05
  dc:	962a0300 	lhu	t2,768(s1)
  e0:	05000000 	bltz	t0,e4 <_DYNAMIC-0x4000dc>
  e4:	00000064 	0x64
  e8:	008b2b03 	0x8b2b03
  ec:	04060000 	0x4060000
  f0:	000000d8 	0xd8
  f4:	002c0107 	0x2c0107
  f8:	0c010000 	jal	40000 <_DYNAMIC-0x3c01c0>
  fc:	0000ac01 	0xac01
 100:	40068000 	mfc0	a2,c0_config
 104:	40087800 	mfc0	t0,c0_prid
 108:	00000000 	nop
 10c:	00015600 	sll	t2,at,0x18
 110:	6d660800 	ldr	a2,2048(t3)
 114:	0d010074 	jal	40401d0 <_gp+0x3c274c0>
 118:	00000156 	0x156
 11c:	09009102 	j	4024408 <_gp+0x3c0b6f8>
 120:	0000d50a 	0xd50a
 124:	ee120100 	swc3	$18,256(s0)
 128:	02000000 	0x2000000
 12c:	700b7491 	0x700b7491
 130:	13010063 	beq	t8,at,2c0 <_DYNAMIC-0x3fff00>
 134:	00000156 	0x156
 138:	0b709102 	j	dc24408 <_gp+0xd80b6f8>
 13c:	01007675 	0x1007675
 140:	0000cd14 	0xcd14
 144:	68910200 	ldl	s1,512(a0)
 148:	0076730b 	0x76730b
 14c:	00c21501 	0xc21501
 150:	91020000 	lbu	v0,0(t0)
 154:	04060060 	0x4060060
 158:	000000e3 	0xe3
 15c:	007a010c 	syscall	0x1e804
 160:	3f010000 	0x3f010000
 164:	0000004f 	0x4f
 168:	00400878 	0x400878
 16c:	0040099c 	0x40099c
 170:	0000002b 	sltu	zero,zero,zero
 174:	000001a1 	0x1a1
 178:	0000000a 	0xa
 17c:	ac450100 	sw	a1,256(v0)
 180:	02000000 	0x2000000
 184:	710a7091 	0x710a7091
 188:	01000000 	0x1000000
 18c:	0000cd46 	0xcd46
 190:	68910200 	ldl	s1,512(a0)
 194:	0100690b 	0x100690b
 198:	0000b747 	0xb747
 19c:	60910200 	daddi	s1,a0,512
 1a0:	00ec0d00 	0xec0d00
 1a4:	07010000 	bgez	t8,1a8 <_DYNAMIC-0x400018>
 1a8:	000000b7 	0xb7
 1ac:	68030501 	ldl	v1,1281(zero)
 1b0:	0e00410d 	jal	8010434 <_gp+0x7bf7724>
 1b4:	000000b7 	0xb7
 1b8:	000001c3 	sra	zero,zero,0x7
 1bc:	00006b0f 	0x6b0f
 1c0:	0d006300 	jal	4018c00 <_gp+0x3bffef0>
 1c4:	0000009e 	0x9e
 1c8:	01b30801 	0x1b30801
 1cc:	05010000 	bgez	t0,1d0 <_DYNAMIC-0x3ffff0>
 1d0:	410d8003 	0x410d8003
 1d4:	00580000 	0x580000
 1d8:	00020000 	sll	zero,v0,0x0
 1dc:	000000ce 	0xce
 1e0:	00eb0104 	0xeb0104
 1e4:	09a00000 	j	6800000 <_gp+0x63e72f0>
 1e8:	0b200040 	j	c800100 <_gp+0xc3e73f0>
 1ec:	742f0040 	jalx	bc0100 <_gp+0x7a73f0>
 1f0:	632f706d 	daddi	t7,t9,28781
 1f4:	4c517863 	0x4c517863
 1f8:	2e4b5436 	sltiu	t3,s2,21558
 1fc:	682f0073 	ldl	t7,115(at)
 200:	2f656d6f 	sltiu	a1,k1,28015
 204:	6e6f6f6c 	ldr	t7,28524(s3)
 208:	6e6f7367 	ldr	t7,29543(s3)
 20c:	4946452f 	0x4946452f
 210:	50494d2d 	beql	v0,t1,136c8 <_DYNAMIC-0x3ecaf8>
 214:	65742f53 	daddiu	s4,t3,12115
 218:	742f7473 	jalx	bdd1cc <_gp+0x7c44bc>
 21c:	6e697250 	ldr	t1,29264(s3)
 220:	4e470074 	c3	0x470074
 224:	53412055 	beql	k0,at,837c <_DYNAMIC-0x3f7e44>
 228:	322e3220 	andi	t6,s1,0x3220
 22c:	00312e30 	tge	at,s1,0xb8
 230:	Address 0x0000000000000230 is out of bounds.


Disassembly of section .debug_abbrev:

00000000 <.debug_abbrev>:
   0:	25011101 	addiu	at,t0,4353
   4:	030b130e 	0x30b130e
   8:	110e1b08 	beq	t0,t6,6c2c <_DYNAMIC-0x3f9594>
   c:	10011201 	beq	zero,at,4814 <_DYNAMIC-0x3fb9ac>
  10:	02000006 	srlv	zero,zero,s0
  14:	0b0b0024 	j	c2c0090 <_gp+0xbea7380>
  18:	0e030b3e 	jal	80c2cf8 <_gp+0x7ca9fe8>
  1c:	24030000 	li	v1,0
  20:	3e0b0b00 	0x3e0b0b00
  24:	0008030b 	0x8030b
  28:	00240400 	0x240400
  2c:	0b3e0b0b 	j	cf82c2c <_gp+0xcb69f1c>
  30:	16050000 	bne	s0,a1,34 <_DYNAMIC-0x40018c>
  34:	3a0e0300 	xori	t6,s0,0x300
  38:	490b3b0b 	0x490b3b0b
  3c:	06000013 	bltz	s0,8c <_DYNAMIC-0x400134>
  40:	0b0b000f 	j	c2c003c <_gp+0xbea732c>
  44:	00001349 	0x1349
  48:	3f012e07 	0x3f012e07
  4c:	3a0e030c 	xori	t6,s0,0x30c
  50:	270b3b0b 	addiu	t3,t8,15115
  54:	1113490c 	beq	t0,s3,12488 <_DYNAMIC-0x3edd38>
  58:	40011201 	0x40011201
  5c:	00130106 	0x130106
  60:	00050800 	sll	at,a1,0x0
  64:	0b3a0803 	j	ce8200c <_gp+0xca692fc>
  68:	13490b3b 	beq	k0,t1,2d58 <_DYNAMIC-0x3fd468>
  6c:	00000a02 	srl	at,zero,0x8
  70:	00001809 	jalr	v1,zero
  74:	00340a00 	0x340a00
  78:	0b3a0e03 	j	ce8380c <_gp+0xca6aafc>
  7c:	13490b3b 	beq	k0,t1,2d6c <_DYNAMIC-0x3fd454>
  80:	00000a02 	srl	at,zero,0x8
  84:	0300340b 	0x300340b
  88:	3b0b3a08 	xori	t3,t8,0x3a08
  8c:	0213490b 	0x213490b
  90:	0c00000a 	jal	28 <_DYNAMIC-0x400198>
  94:	0c3f012e 	jal	fc04b8 <_gp+0xba77a8>
  98:	0b3a0e03 	j	ce8380c <_gp+0xca6aafc>
  9c:	13490b3b 	beq	k0,t1,2d8c <_DYNAMIC-0x3fd434>
  a0:	01120111 	0x1120111
  a4:	13010640 	beq	t8,at,19a8 <_DYNAMIC-0x3fe818>
  a8:	340d0000 	li	t5,0x0
  ac:	3a0e0300 	xori	t6,s0,0x300
  b0:	490b3b0b 	0x490b3b0b
  b4:	020c3f13 	0x20c3f13
  b8:	0e00000a 	jal	8000028 <_gp+0x7be7318>
  bc:	13490101 	beq	k0,t1,4c4 <_DYNAMIC-0x3ffcfc>
  c0:	00001301 	0x1301
  c4:	4900210f 	bc2f	8504 <_DYNAMIC-0x3f7cbc>
  c8:	000b2f13 	0xb2f13
  cc:	11010000 	beq	t0,at,d0 <_DYNAMIC-0x4000f0>
  d0:	11061000 	beq	t0,a2,40d4 <_DYNAMIC-0x3fc0ec>
  d4:	03011201 	0x3011201
  d8:	25081b08 	addiu	t0,t0,6920
  dc:	00051308 	0x51308
	...

Disassembly of section .debug_line:

00000000 <.debug_line>:
   0:	000000e7 	0xe7
   4:	00960002 	0x960002
   8:	01010000 	0x1010000
   c:	000d0efb 	dsra	at,t5,0x1b
  10:	01010101 	0x1010101
  14:	01000000 	0x1000000
  18:	2f010000 	sltiu	at,t8,0
  1c:	656d6f68 	daddiu	t5,t3,28520
  20:	6f6f6c2f 	ldr	t7,27695(k1)
  24:	6f73676e 	ldr	s3,26478(k1)
  28:	46452f6e 	c1	0x452f6e
  2c:	494d2d49 	0x494d2d49
  30:	462f5350 	c1	0x2f5350
  34:	646e756f 	daddiu	t6,v1,30063
  38:	6f697461 	ldr	t1,29793(k1)
  3c:	6e492f6e 	ldr	t1,12142(s2)
  40:	64756c63 	daddiu	s5,v1,27747
  44:	694d2f65 	ldl	t5,12133(t2)
  48:	32337370 	andi	s3,s1,0x7370
  4c:	6f682f00 	ldr	t0,12032(k1)
  50:	6c2f656d 	ldr	t7,25965(at)
  54:	676e6f6f 	daddiu	t6,k1,28527
  58:	2f6e6f73 	sltiu	t6,k1,28531
  5c:	2d494645 	sltiu	t1,t2,17989
  60:	5350494d 	beql	k0,s0,12598 <_DYNAMIC-0x3edc28>
  64:	756f462f 	jalx	5bd18bc <_gp+0x57b8bac>
  68:	7461646e 	jalx	18591b8 <_gp+0x14404a8>
  6c:	2f6e6f69 	sltiu	t6,k1,28521
  70:	2f696645 	sltiu	t1,k1,26181
  74:	6c636e49 	ldr	v1,28233(v1)
  78:	00656475 	0x656475
  7c:	632e6100 	daddi	t6,t9,24832
  80:	00000000 	nop
  84:	42696645 	c0	0x696645
  88:	2e646e69 	sltiu	a0,s3,28265
  8c:	00010068 	0x10068
  90:	69664500 	ldl	a2,17664(t3)
  94:	65707954 	daddiu	s0,t3,31060
  98:	00682e73 	tltu	v1,t0,0xb9
  9c:	00000002 	srl	zero,zero,0x0
  a0:	80020500 	lb	v0,1280(zero)
  a4:	03004006 	srlv	t0,zero,t8
  a8:	28020110 	slti	v0,zero,272
  ac:	83bbbd19 	lb	k1,-17127(sp)
  b0:	02142402 	0x2142402
  b4:	3002132c 	andi	v0,zero,0x132c
  b8:	13400213 	beqz	k0,908 <_DYNAMIC-0x3ff8b8>
  bc:	13300285 	beq	t9,s0,ad4 <_DYNAMIC-0x3ff6ec>
  c0:	02133002 	0x2133002
  c4:	71031648 	0x71031648
  c8:	081403f2 	j	500fc8 <_gp+0xe82b8>
  cc:	11034b74 	beq	t0,v1,12ea0 <_DYNAMIC-0x3ed320>
  d0:	09033c08 	j	40cf020 <_gp+0x3cb6310>
  d4:	f483ac08 	sdc1	$f3,-21496(a0)
  d8:	ae08e608 	sw	t0,-6648(s0)
  dc:	02bbe608 	0x2bbe608
  e0:	3002113c 	andi	v0,zero,0x113c
  e4:	18024b17 	0x18024b17
  e8:	9f010100 	lwu	at,256(t8)
  ec:	02000000 	0x2000000
  f0:	00002600 	sll	a0,zero,0x18
  f4:	fb010100 	sdc2	$1,256(t8)
  f8:	01000d0e 	0x1000d0e
  fc:	00010101 	0x10101
 100:	00010000 	sll	zero,at,0x0
 104:	742f0100 	jalx	bc0400 <_gp+0x7a76f0>
 108:	0000706d 	0x706d
 10c:	51786363 	beql	t3,t8,18e9c <_DYNAMIC-0x3e7324>
 110:	4b54364c 	c2	0x154364c
 114:	0100732e 	0x100732e
 118:	00000000 	nop
 11c:	09a00205 	j	6800814 <_gp+0x63e7b04>
 120:	4b190040 	c2	0x1190040
 124:	f34b4b4b 	scd	t3,19275(k0)
 128:	4b4b4b4b 	c2	0x14b4b4b
 12c:	4b4c4b4b 	c2	0x14c4b4b
 130:	4b4a0903 	c2	0x14a0903
 134:	4b4b4b4b 	c2	0x14b4b4b
 138:	4b4b4b4b 	c2	0x14b4b4b
 13c:	4b4a0903 	c2	0x14a0903
 140:	4b4b4b4b 	c2	0x14b4b4b
 144:	034b4b4b 	0x34b4b4b
 148:	4b4b4a09 	c2	0x14b4a09
 14c:	4b4b4b4b 	c2	0x14b4b4b
 150:	09034b4b 	j	40d2d2c <_gp+0x3cba01c>
 154:	4b4b4b4a 	c2	0x14b4b4a
 158:	4b4b4b4b 	c2	0x14b4b4b
 15c:	4b4b4c4b 	c2	0x14b4c4b
 160:	4b4b4b4b 	c2	0x14b4b4b
 164:	4b4c4c4b 	c2	0x14c4c4b
 168:	4b4a0903 	c2	0x14a0903
 16c:	4b4b4b4b 	c2	0x14b4b4b
 170:	4b4b4b4b 	c2	0x14b4b4b
 174:	4b4b4b4c 	c2	0x14b4b4c
 178:	4b4b4b4b 	c2	0x14b4b4b
 17c:	4b4c4b4c 	c2	0x14c4b4c
 180:	4b4b4b4b 	c2	0x14b4b4b
 184:	4c4b4b4c 	0x4c4b4b4c
 188:	0004024b 	0x4024b
 18c:	Address 0x000000000000018c is out of bounds.


Disassembly of section .debug_frame:

00000000 <.debug_frame>:
   0:	0000000c 	syscall
   4:	ffffffff 	sd	ra,-1(ra)
   8:	7c010001 	0x7c010001
   c:	001d0c1f 	0x1d0c1f
  10:	00000018 	mult	zero,zero
  14:	00000000 	nop
  18:	00400680 	0x400680
  1c:	000001f8 	dsll	zero,zero,0x7
  20:	44280e44 	0x44280e44
  24:	44011e11 	0x44011e11
  28:	00001e0d 	break	0x0,0x78
  2c:	0000001c 	dmult	zero,zero
  30:	00000000 	nop
  34:	00400878 	0x400878
  38:	00000124 	0x124
  3c:	48380e44 	0x48380e44
  40:	11021e11 	beq	t0,v0,7888 <_DYNAMIC-0x3f8938>
  44:	0d44011f 	jal	510047c <_gp+0x4ce776c>
  48:	0000001e 	ddiv	zero,zero,zero

Disassembly of section .debug_str:

00000000 <.debug_str>:
   0:	53667542 	beql	k1,a2,1d50c <_DYNAMIC-0x3e2cb4>
   4:	00657a69 	0x657a69
   8:	544e4955 	bnel	v0,t6,12560 <_DYNAMIC-0x3edc60>
   c:	682f004e 	ldl	t7,78(at)
  10:	2f656d6f 	sltiu	a1,k1,28015
  14:	6e6f6f6c 	ldr	t7,28524(s3)
  18:	6e6f7367 	ldr	t7,29543(s3)
  1c:	4946452f 	0x4946452f
  20:	50494d2d 	beql	v0,t1,134d8 <_DYNAMIC-0x3ecce8>
  24:	65742f53 	daddiu	s4,t3,12115
  28:	742f7473 	jalx	bdd1cc <_gp+0x7c44bc>
  2c:	6e697250 	ldr	t1,29264(s3)
  30:	69750074 	ldl	s5,116(t3)
  34:	3436746e 	ori	s6,at,0x746e
  38:	7500745f 	jalx	401d17c <_gp+0x3c0446c>
  3c:	6769736e 	daddiu	t1,k1,29550
  40:	2064656e 	addi	a0,v1,25966
  44:	72616863 	0x72616863
  48:	6e697500 	ldr	t1,29952(s3)
  4c:	745f6e74 	jalx	17db9d0 <_gp+0x13c2cc0>
  50:	6f687300 	ldr	t0,29440(k1)
  54:	75207472 	jalx	481d1c8 <_gp+0x44044b8>
  58:	6769736e 	daddiu	t1,k1,29550
  5c:	2064656e 	addi	a0,v1,25966
  60:	00746e69 	0x746e69
  64:	52414843 	beql	s2,at,12174 <_DYNAMIC-0x3ee04c>
  68:	43003631 	c0	0x1003631
  6c:	38524148 	xori	s2,v0,0x4148
  70:	6c694600 	ldr	t1,17920(v1)
  74:	7a695365 	0x7a695365
  78:	616d0065 	daddi	t5,t3,101
  7c:	6c006e69 	ldr	zero,28265(zero)
  80:	20676e6f 	addi	a3,v1,28271
  84:	676e6f6c 	daddiu	t6,k1,28524
  88:	736e7520 	0x736e7520
  8c:	656e6769 	daddiu	t6,t3,26473
  90:	6e692064 	ldr	t1,8292(s3)
  94:	69750074 	ldl	s5,116(t3)
  98:	5f38746e 	0x5f38746e
  9c:	61440074 	daddi	a0,t2,116
  a0:	6c006174 	ldr	zero,24948(zero)
  a4:	20676e6f 	addi	a3,v1,28271
  a8:	676e6f6c 	daddiu	t6,k1,28524
  ac:	746e6920 	jalx	1b9a480 <_gp+0x1781770>
  b0:	554e4700 	bnel	t2,t6,11cb4 <_DYNAMIC-0x3ee50c>
  b4:	34204320 	ori	zero,at,0x4320
  b8:	352e342e 	ori	t6,t1,0x342e
  bc:	4e495500 	c3	0x495500
  c0:	00323354 	0x323354
  c4:	544e4955 	bnel	v0,t6,1261c <_DYNAMIC-0x3edba4>
  c8:	73003436 	0x73003436
  cc:	74726f68 	jalx	1c9bda0 <_gp+0x1883090>
  d0:	746e6920 	jalx	1b9a480 <_gp+0x1781770>
  d4:	67726100 	daddiu	s2,k1,24832
  d8:	69750073 	ldl	s5,115(t3)
  dc:	3631746e 	ori	s1,s1,0x746e
  e0:	7500745f 	jalx	401d17c <_gp+0x3c0446c>
  e4:	33746e69 	andi	s4,k1,0x6e69
  e8:	00745f32 	tlt	v1,s4,0x17c
  ec:	65646e49 	daddiu	a0,t3,28233
  f0:	Address 0x00000000000000f0 is out of bounds.


Disassembly of section .debug_loc:

00000000 <.debug_loc>:
   0:	00000000 	nop
   4:	00000004 	sllv	zero,zero,zero
   8:	046d0001 	0x46d0001
   c:	0c000000 	jal	0 <_DYNAMIC-0x4001c0>
  10:	02000000 	0x2000000
  14:	0c288d00 	jal	a23400 <_gp+0x60a6f0>
  18:	f8000000 	sdc2	$0,0(zero)
  1c:	02000001 	0x2000001
  20:	00288e00 	0x288e00
  24:	00000000 	nop
  28:	f8000000 	sdc2	$0,0(zero)
  2c:	fc000001 	sd	zero,1(zero)
  30:	01000001 	0x1000001
  34:	01fc6d00 	0x1fc6d00
  38:	02080000 	0x2080000
  3c:	00020000 	sll	zero,v0,0x0
  40:	0208388d 	break	0x208,0xe2
  44:	031c0000 	0x31c0000
  48:	00020000 	sll	zero,v0,0x0
  4c:	0000388e 	0x388e
  50:	00000000 	nop
	...

Disassembly of section .gnu.attributes:

00000000 <.gnu.attributes>:
   0:	00000f41 	0xf41
   4:	756e6700 	jalx	5b99c00 <_gp+0x5780ef0>
   8:	00070100 	sll	zero,a3,0x4
   c:	01040000 	0x1040000
