/*++

Copyright (c) 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:
  
  EfiJump.h

Abstract:

  This is the Setjump/Longjump pair for an MIPS32 processor.

--*/

#ifndef _EFI_JUMP_H_
#define _EFI_JUMP_H_

typedef struct {
  UINT32  ra;
  UINT32  sp;
  UINT32  regs[8];
  UINT32  fp;
  UINT32  gp;
} EFI_JUMP_BUFFER;

#endif
