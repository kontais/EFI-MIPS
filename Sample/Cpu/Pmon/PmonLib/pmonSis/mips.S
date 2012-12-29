

#include "Mips.h"



#define	NOP8	nop;nop;nop;nop;nop;nop;nop;nop

	.set	noreorder



	.text


//
//  Get CP0 count register value
//
LEAF(CPU_GetCOUNT)
	mfc0	v0, CP0_Count
	nop
	j	ra
	nop
END(CPU_GetCOUNT)


LEAF(CPU_SetCOUNT)
	mtc0	a0, CP0_Count
	nop
	j	ra
	nop
END(CPU_SetCOUNT)

//
// Get Cause
//
LEAF(CPU_GetCAUSE)
	mfc0	v0, CP0_Cause
	nop
	j	ra
	nop
END(CPU_GetCAUSE)


//
// Get Status
//
LEAF(CPU_GetSTATUS)
	mfc0	v0, CP0_Status
	nop
	j	ra
	nop
END(CPU_GetSTATUS)



//
//  Modify SR value, arg 1 = set bits, arg 2 = clear bits.
//
LEAF(CPU_SetSR)
	mfc0	v0, CP0_Status
	not	v1, a1
	and	v1, v0
	or	v1, a0
	mtc0	v1, CP0_Status
	NOP8
	j	ra
	nop
END(CPU_SetSR)

//
//  Get configuration register contents.
//
LEAF(CPU_GetCONFIG)
	mfc0	v0, CP0_Config
	j	ra
	nop
END(CPU_GetCONFIG)



