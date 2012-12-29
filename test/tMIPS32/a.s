#include "ArchDefs.h"

  .text
Test:
  dadd  t1,t2,t3
  add  t1,t2,t3
  li    v0,2
  jr    ra
  nop

