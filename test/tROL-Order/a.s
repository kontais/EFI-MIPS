  .text

  .align 3

  .global tf
  .ent tf
tf:
  rol $3, $4, 4

  addu $3,8

  jr $30
  nop
  .end tf

