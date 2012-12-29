/*++

Copyright (c) 2009, kontais                                                                   
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  SystemLib.h

Abstract:

  This protocol for POSIX lib for Linux emulation envirnment.

--*/

#ifndef _SYSTEM_LIB_H_
#define _SYSTEM_LIB_H_

#define EFI_SYSTEM_LIB_PROTOCOL_GUID \
  { \
    0xebe21012, 0x3ab3, 0x11de, {0xb4, 0xcf, 0x08, 0x80, 0xfe, 0xcc, 0x3a, 0xb4} \
  }

typedef
int
(*stdio_printf) (
  const char *restrict,
  ...
);

typedef
int
(*sys_cachectrl_flush_cache) (
  char *addr,
  const int nbytes,
  const int op
);

EFI_FORWARD_DECLARATION (EFI_SYSTEM_LIB_PROTOCOL);

#define EFI_SYSTEM_LIB_PROTOCOL_SIGNATURE EFI_SIGNATURE_32 ('S', 'T', 'L', 'B')

struct _EFI_SYSTEM_LIB_PROTOCOL {
  UINT64                            Signature;

  //
  // stdio.h
  //
  stdio_printf                   printf;
  
  //
  //  sys/cachectrl.h
  //
  sys_cachectrl_flush_cache      _flush_cache;

  } ;

extern EFI_GUID gEfiSystemLibProtocolGuid;

#endif /* _SYSTEM_LIB_H_ */
