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
 .align 2
 .globl GetCP0
 .ent GetCP0
 .type GetCP0, @function
GetCP0:
 .set noreorder
 .set nomacro
 
 jr  ra
 /* li  v0, 0x1234 */
 mfc0 v0,$0

 .set macro
 .set reorder
 .end GetCP0
