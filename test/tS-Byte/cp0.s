#include <ArchDefs.h>

/******************************************************************************
 *
 *     GetCP0
 *  input:   a0  0-31
 * output:   v0  CPx value
 *
 *  int GetCP0(int Reg)
 *
 *****************************************************************************/
 .text
 .align 3
 .globl GetCP0
 .ent GetCP0
 .type GetCP0, @function
GetCP0:
 .set noreorder

  la    t0, var32
  lwu   v0, 0(t0)
  nop
 
  jr     ra
  nop

 .set reorder
 .end GetCP0

  .data
  .align 3
  .global var32,vars1
var32: .word 0x12345678
var8: .byte 0xee
vars1: .asciiz "abcdef"


