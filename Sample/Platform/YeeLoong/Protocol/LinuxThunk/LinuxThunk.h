/*++

Copyright (c) 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  LinuxThunk.h

Abstract:

  This protocol allows an EFI driver  in the Linux emulation envirnment
  to make Linux API calls.

  NEVER make a Linux call directly, always make the call via this protocol.

  There are no This pointers on the protocol member functions as they map
  exactly into Linux system calls.

  YOU MUST include EfiLinux.h in place of Efi.h to make this file compile.

--*/

#ifndef _LINUX_THUNK_H_
#define _LINUX_THUNK_H_

#include EFI_PROTOCOL_DEPENDENCY(UgaDraw)

#define EFI_LINUX_THUNK_PROTOCOL_GUID \
  { \
    0x58c518b1, 0x76f3, 0x11d4, {0xbc, 0xea, 0x0, 0x80, 0xc7, 0x3c, 0x88, 0x81} \
  }

//
// The following APIs require EfiLinux.h. In some environmnets the GUID
// definitions are needed but the EfiLinux.h is not included.
// EfiLinux.h is needed to support WINDOWS API requirements.
//
//#ifdef _EFI_LINUX_H_



//
//  Linux Thunk Protocol
//
EFI_FORWARD_DECLARATION (EFI_LINUX_THUNK_PROTOCOL);

#define EFI_LINUX_THUNK_PROTOCOL_SIGNATURE EFI_SIGNATURE_32 ('L', 'N', 'X', 'T')

struct _EFI_LINUX_THUNK_PROTOCOL {
  UINT64                              Signature;
};

//#endif

extern EFI_GUID gEfiLinuxThunkProtocolGuid;

#endif
