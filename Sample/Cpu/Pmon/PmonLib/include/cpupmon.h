#ifndef _MIPS_CPU_H_
#define _MIPS_CPU_H_



#define UNCACHED_MEMORY_ADDR	0xa0000000
#define	UNCACHED_TO_PHYS(x) 	((UINTN)(x) & 0x1fffffff)
#define	PHYS_TO_UNCACHED(x) 	((UINTN)(x) | UNCACHED_MEMORY_ADDR)
#define CP0_Count		$9
#define CP0_Status	$12
#define CP0_Cause		$13
#define CP0_Config	$16
#endif /* !_MIPS_CPU_H_ */
