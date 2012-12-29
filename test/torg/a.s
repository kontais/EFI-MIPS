  .text
  .global test
  .ent    test

test:
  move $2, $3
  nop

  .org  0x20,0xffffffff
  move $4, $5

  .end test

