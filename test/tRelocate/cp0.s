#include <ArchDefs.h>
  .set noreorder
 .text
 .align 2
 .globl GetWord
 .ent GetWord
 .type GetWord, @function
GetWord:
  b  main
  nop

// 有了.globl后，main是全局可见，会变成 R_MIPS_PC16
  .globl main
main:
  move v0, zero
  ulw  v0, 0(a0)
  nop
  jr  ra
  nop
  b   GetWord
  nop

  .set reorder

  .end GetWord
