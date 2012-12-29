#ifndef _ASM_MACRO_H_
#define _ASM_MACRO_H_


#define ITLBNOPFIX      nop;nop;nop;nop

#define ALIGNSZ   16  /* Stack layout alignment */
#define FRAMESZ(sz) (((sz) + (ALIGNSZ-1)) & ~(ALIGNSZ-1))

/*
 *  Basic register operations based on selected ISA
 */
#define REGSZ   4 /* 32 bit mode register size */
#define CF_SZ   24  /* Call frame size */
#define CF_RA_OFFS  20  /* Call ra save offset */

#endif /* _ASM_MACRO_H_ */
