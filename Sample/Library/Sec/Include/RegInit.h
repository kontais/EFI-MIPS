/*++

Copyright (c) 2011, kontais                                                                   
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  RegInit.h

Abstract:


--*/

#ifndef _REG_INIT_H_
#define _REG_INIT_H_

#define  REG_INIT_MOD_BYTE        0x00000000
#define  REG_INIT_MOD_HALFWORD    0x00000001
#define  REG_INIT_MOD_WORD        0x00000002
#define  REG_INIT_MOD_MASK        0x00000003

#define  REG_INIT_OP_MASK         0x000000fc
#define  REG_INIT_OP_EXIT         0x00000000
#define  REG_INIT_OP_DELAY        0x00000008
#define  REG_INIT_OP_READ         0x00000010
#define  REG_INIT_OP_WRITE        0x00000014
#define  REG_INIT_OP_RMW          0x00000018 /* read-modify-write */
#define  REG_INIT_OP_WAIT         0x00000020

#define  REG_INIT_WRITE(mod, addr, val) \
  .word  REG_INIT_OP_WRITE|mod, (addr); \
  .word  (val), 0

#define  REG_INIT_READ(mod, addr)       \
  .word  REG_INIT_OP_READ|mod, (addr);  \
  .word  0, 0

#define  REG_INIT_RMW(mod, addr, mask, val)  \
  .word  REG_INIT_OP_RMW|mod, (addr);        \
  .word  (mask), (val)

#define  REG_INIT_WAIT(mod, addr, mask, val)  \
  .word  REG_INIT_OP_WAIT|mod, (addr);        \
  .word  (mask), (val)

#define  REG_INT_DELAY(cycles)        \
  .word  REG_INIT_OP_DELAY, (cycles); \
  .word  0, 0

#define  REG_INIT_EXIT(status)       \
  .word  REG_INIT_OP_EXIT, (status); \
  .word  0, 0

#define REG_INIT_ITEM_FIELD0    0
#define REG_INIT_ITEM_FIELD1    4
#define REG_INIT_ITEM_FIELD2    8
#define REG_INIT_ITEM_FIELD3    12
#define REG_INIT_ITEM_SIZE      16


#endif /* _REG_INIT_H_ */

