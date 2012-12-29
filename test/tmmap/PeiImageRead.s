#include "Mips.h"

  .file  1 "PeiImageRead.s"
  .text
  .align  2
  .globl  PeiImageRead
  .ent    PeiImageRead
  .type   PeiImageRead, @function
PeiImageRead:
/*++
EFI_STATUS
PeiImageRead (
  IN     VOID    *FileHandle,
  IN     UINTN   FileOffset,
  IN OUT UINTN   *ReadSize,
  OUT    VOID    *Buffer
  )
Routine Description:

  Support routine for the PE/COFF Loader that reads a buffer from a PE/COFF file

Arguments:

  FileHandle - The handle to the PE/COFF file

  FileOffset - The offset, in bytes, into the file to read

  ReadSize   - The number of bytes to read from the file starting at FileOffset

  Buffer     - A pointer to the buffer to read the data into.

Returns:

  EFI_SUCCESS - ReadSize bytes of data were read into Buffer from the PE/COFF file starting at FileOffset

--*/

  .set  nomips16
  .frame  fp,24,ra    # vars= 16, regs= 1/0, args= 0, gp= 0
  .mask  0x40000000,-4
  .fmask  0x00000000,0
  .set  noreorder
  .set  nomacro
  
  addiu sp,sp,-24
  sw    fp,20(sp)
  move  fp,sp
  sw    a0,24(fp)
  sw    a1,28(fp)
  sw    a2,32(fp)
  sw    a3,36(fp)
  lw    v0,36(fp)
  nop
  sw    v0,8(fp)
  lw    v1,24(fp)
  lw    v0,28(fp)
  nop
  addu  v0,v1,v0
  sw    v0,4(fp)
  lw    v0,32(fp)
  nop
  lw    v0,0(v0)
  nop
  sw    v0,0(fp)
  b     $L2
  nop

$L3:
  lw    v0,4(fp)
  nop
  lbu   v1,0(v0)
  lw    v0,8(fp)
  nop
  sb    v1,0(v0)
  lw    v0,8(fp)
  nop
  addiu v0,v0,1
  sw    v0,8(fp)
  lw    v0,4(fp)
  nop
  addiu v0,v0,1
  sw    v0,4(fp)
$L2:
  lw    v0,0(fp)
  nop
  sltu  v0,$0,v0
  andi  v1,v0,0x00ff
  lw    v0,0(fp)
  nop
  addiu v0,v0,-1
  sw    v0,0(fp)
  bne   v1,$0,$L3
  nop

  move  v0,$0
  move  sp,fp
  lw    fp,20(sp)
  addiu sp,sp,24
  jr    ra
  nop

  .set  macro
  .set  reorder
  .end  PeiImageRead
