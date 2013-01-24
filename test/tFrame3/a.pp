  .text
  .set noreorder
  .align 3
  .globl SecMain
  .ent SecMain
  .type SecMain, @function
SecMain:
  .set nomacro
  li $2, 100
  jr $31
  nop
  nop
  .set macro
  .end SecMain
  .align 3
  .globl k_intr
  .ent k_intr
  .type k_intr, @function
k_intr:
  li $4, 1
  li $5, 2
  li $6, 9
  li $7, 3
  li $2, 100
  li $3, 200
  .set noat
  sub $26, $29, ((((24 + (40 * 4) + 16)) + (16 -1)) & ~(16 -1))
  sw $31, 20 + (4 * 31) ($26) ; sw $at, 20 + (4 * 1) ($26) ; sw $2, 20 + (4 * 2) ($26) ; sw $3, 20 + (4 * 3) ($26) ; sw $4, 20 + (4 * 4) ($26) ; sw $5, 20 + (4 * 5) ($26) ; sw $6, 20 + (4 * 6) ($26) ; sw $7, 20 + (4 * 7) ($26) ; addu $4, $26, 20
  .set at
  move $29, $26
  la $28, _gp
  jal interrupt
  nop
  li $2, 300
  .set noat
  lw $31, 20 + (4 * 31) ($29) ; lw $at, 20 + (4 * 1) ($29) ; lw $2, 20 + (4 * 2) ($29) ; lw $3, 20 + (4 * 3) ($29) ; lw $4, 20 + (4 * 4) ($29) ; lw $5, 20 + (4 * 5) ($29) ; lw $6, 20 + (4 * 6) ($29) ; lw $7, 20 + (4 * 7) ($29)
  addu $29, $29, ((((24 + (40 * 4) + 16)) + (16 -1)) & ~(16 -1))
  jr $31
  .set at
  .end k_intr
