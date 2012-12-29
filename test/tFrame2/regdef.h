#ifndef _MIPS_REGDEF_H_
#define _MIPS_REGDEF_H_

#define zero  $0  /* always zero */
#define AT  $at /* assembler temp */
#define v0  $2  /* return value */
#define v1  $3
#define a0  $4  /* argument registers */
#define a1  $5
#define a2  $6
#define a3  $7
#if defined(__mips_n32) || defined(__mips_n64)
#define a4  $8  /* expanded register arguments */
#define a5  $9
#define a6  $10
#define a7  $11
#define ta0 $8  /* alias */
#define ta1 $9
#define ta2 $10
#define ta3 $11
#define t0  $12 /* temp registers (not saved across subroutine calls) */
#define t1  $13
#define t2  $14
#define t3  $15
#else
#define t0  $8  /* temp registers (not saved across subroutine calls) */
#define t1  $9
#define t2  $10
#define t3  $11
#define t4  $12
#define t5  $13
#define t6  $14
#define t7  $15
#define ta0 $12 /* alias */
#define ta1 $13
#define ta2 $14
#define ta3 $15
#endif
#define s0  $16 /* saved across subroutine calls (callee saved) */
#define s1  $17
#define s2  $18
#define s3  $19
#define s4  $20
#define s5  $21
#define s6  $22
#define s7  $23
#define t8  $24 /* two more temp registers */
#define t9  $25
#define k0  $26 /* kernel temporary */
#define k1  $27
#define gp  $28 /* global pointer */
#define sp  $29 /* stack pointer */
#define s8  $30 /* one more callee saved */
#define ra  $31 /* return address */

#endif /* !_MIPS_REGDEF_H_ */

