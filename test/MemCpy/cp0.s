#include <ArchDefs.h>
 .text
 .align 2
 .globl MemCpy
 .ent MemCpy
 .type MemCpy, @function
MemCpy:
  .set noreorder
  .set nomacro

/*
* int MemCpy(void *dest, void *src, unsigned int count);
*/

  move t2, a2
  move t4, a1
  move t5, a0

//
// First of all, make sure we have no overlap. That is to say,
// if (Source == Destination)          => do nothing
// if (Source + Count <= Destination)  =>  regular copy
// if (Destination + Count <= Source)  =>  regular copy
// otherwise, do a reverse copy
//
  move t0, t4
  addu t0, t2                  // Source + Count ?<= Dest
  beq  t0, t5, _StartByteCopy  // =
  nop
  sltu t1, t0, t5
  bnez t1, _StartByteCopy      // < 
  nop

  move t0, t5
  addu t0, t2                  // Dest + Count ? <= Source
  beq  t0, t4, _StartByteCopy  // =
  nop
  sltu t1, t0, t4
  bnez t1, _StartByteCopy      // <
  nop

  beq t4, t5, _CopyMemDone     // Dest == Source
  nop
  sltu t1, t4, t5
  bnez t1, _CopyOverlapped      // too bad -- overlaps
  nop
 
  // Pick up misaligned start bytes to get destination pointer 4-byte aligned
_StartByteCopy:
  beqz t2, _CopyMemDone         // Count == 0, all done
  nop
  move t3, t5
  andi t3, 3                    // check lower 2 bits of address
  beqz t3, _CopyBlocks          // already aligned
  nop

  // Copy a bye
  lbu t0, 0(t4)            // get byte from Source
  nop
  sb t0, 0(t5)            // write byte to Destination
  addiu t2, -1
  addiu t4, 1
  addiu t5, 1
  b _StartByteCopy
  nop

_CopyBlocks:
  // Compute how many 32-byte blocks we can clear
  move t0, t2         // get Count in t0
  srl  t0, 5          // convert to 32-byte count
  sll  t0, 5          // convert back to bytes
  subu t2, t0         // subtract from the original count, t2 is last < 32 byte
  srl  t0, 5          // t0 is how many 32-bytes blocks

  // if no 32-byte blocks, then skip
  beqz  t0, _CopyRemainingDWords
  nop

_B:
  ulw  t6, 0(t4)
  ulw  t7, 4(t4)
  ulw  t8, 8(t4)
  ulw  t9, 12(t4)

  sw   t6, 0(t5)
  sw   t7, 4(t5)
  sw   t8, 8(t5)
  sw   t9, 12(t5)

  ulw  t6, 16(t4)
  ulw  t7, 20(t4)
  ulw  t8, 24(t4)
  ulw  t9, 28(t4)

  sw   t6, 16(t5)
  sw   t7, 20(t5)
  sw   t8, 24(t5)
  sw   t9, 28(t5)

  addiu t5, 32
  addiu t4, 32
  addiu t0, -1
  bnez  t0, _B
  nop

_CopyRemainingDWords:
  sltiu t1, t2, 4
  bnez  t1, _CopyRemainingBytes
  nop
  ulw   t6, 0(t4)
  nop
  sw    t6, 0(t5)
  addiu t2, -4
  addiu t4, 4
  addiu t5, 4
  b  _CopyRemainingDWords
  nop

_CopyRemainingBytes:
  beqz  t2,  _CopyMemDone
  nop
  lbu    t0, 0(t4)
  nop
  sb    t0, 0(t5)
  addiu t2, -1
  addiu t4, 1
  addiu t5, 1
  b  _CopyRemainingBytes
  nop

//
// We do this block if the source and destination buffers overlap. To
// handle it, copy starting at the end of the source buffer and work
// your way back. Since this is the atypical case, this code has not
// been optimized, and thus simply copies bytes.
//
_CopyOverlapped:
  //Move the source and destination pointers to the end of the range
  addu  t4, t2      // Source + Count
  addiu t4, -1
  addu  t5, t2
  addiu t5, -1

_CopyOverLappedLoop:
  beqz  t2, _CopyMemDone
  nop
  lbu    t0, 0(t4)      // get byte from Source
  nop
  sb    t0, 0(t5)      // write byte to Destination
  addiu t2, -1
  addiu t4, -1
  addiu t5, -1
  b  _CopyOverLappedLoop
  nop
  
_CopyMemDone:
  jr  ra
  nop

  .set macro
  .set reorder

  .end MemCpy
