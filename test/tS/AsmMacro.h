#ifndef _ASM_MACRO_H_
#define _ASM_MACRO_H_

#if defined(__ASSEMBLER__)

#define JMP_t9_CALL(x)      \
  la t9, x ;   \
  jr t9    ;   \
  nop      ;   \
  .global x##End ; \
x##End:    ;

#define JMP_t9_BACK(x)   \
  la t9, x##End ;  \
  jr t9         ;  \
  nop           ;

#endif /* (__ASSEMBLER__) */

#endif /* _ASM_MACRO_H_ */
