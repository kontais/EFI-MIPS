#include <ArchDefs.h>
 .text
 .align 2
  .set noreorder


 .globl TestJ
 .ent TestJ
 .type TestJ, @function
TestJ:
  move v0, zero
  nop
  nop
  nop
  nop
  nop
  move v0, zero
  //j    JOut
  .word  0x081001a0 //当前PC只用前4位
  nop
  nop
  nop
  nop
  nop
  nop
  nop
  nop
  nop
  move v0, zero

JOut:

  jr  ra
  nop
  .end TestJ
