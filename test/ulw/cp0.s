#include <ArchDefs.h>
 .text
 .align 2
  .set noreorder


 .globl GetWord
 .ent GetWord
 .type GetWord, @function
GetWord:
  move v0, zero
  lwl  v0, 3(a0)
  lwr  v0, 0(a0)
  nop
  jr  ra
  nop
  .end GetWord

 .globl SetWord
 .ent SetWord
 .type SetWord, @function
SetWord:
  swl  a1, 3(a0)
  swr  a1, 0(a0)
  nop
  jr  ra
  nop
  .end SetWord

