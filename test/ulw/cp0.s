#include <ArchDefs.h>
 .text
 .align 2
 .globl GetWord
 .ent GetWord
 .type GetWord, @function
GetWord:
  .set noreorder

  move v0, zero
  ulw  v0, 0(a0)
  nop
  jr  ra
  nop

  .set reorder

  .end GetWord
