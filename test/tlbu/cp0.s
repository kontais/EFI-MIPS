#include <ArchDefs.h>
 .text
 .align 2
  .set noreorder


 .globl lbu
 .ent lbu
 .type lbu, @function
lbu:

  // v0 init value 0xffffffff
  move v0, zero
  add  v0, -1

  lbu  v0, 0(a0)
  nop
  jr  ra
  nop
  .end lbu

 .globl lb
 .ent lb
 .type lb, @function
lb:
  // v0 init value 0
  move v0, zero

  lb   v0, 0(a0)
  nop
  jr  ra
  nop
  .end lb

