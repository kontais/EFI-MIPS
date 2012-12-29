/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  shellenvguid.h

Abstract:

  declares interface functions
 
Revision History

--*/

#ifndef _SHELL_ENV_GUID_H
#define _SHELL_ENV_GUID_H

#include STRING_DEFINES_FILE

//
//
//
#define EFI_SHELLENV_GUID \
  { \
    0xfba5c4bc, 0xf12f, 0x4caa, 0xa0, 0x10, 0xc9, 0x4, 0x61, 0x56, 0x23, 0xb9 \
  }

extern EFI_HII_HANDLE HiiEnvHandle;

#endif
