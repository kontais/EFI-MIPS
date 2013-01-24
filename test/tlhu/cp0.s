#include <ArchDefs.h>
 .text
 .align 2
  .set noreorder


 .globl lhu
 .ent lhu
 .type lhu, @function
lhu:

  // v0 init value 0xffffffff
  move v0, zero
  add  v0, -1

  lhu  v0, 0(a0)
  nop
  jr  ra
  nop
  .end lhu

 .globl lh
 .ent lh
 .type lh, @function
lh:
  // v0 init value 0
  move v0, zero

  lh   v0, 0(a0)
  nop
  jr  ra
  nop
  .end lh

