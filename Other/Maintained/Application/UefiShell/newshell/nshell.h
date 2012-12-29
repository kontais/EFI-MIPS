/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  nshell.h 
  
Abstract:

  Shell

Revision History

--*/

#ifndef _EFI_NEW_SHELL_H_
#define _EFI_NEW_SHELL_H_

#include "EfiShellLib.h"
#include "shellenv.h"

#define EFI_NSHELL_GUID \
  { \
    0x2398750f, 0xec93, 0x4124, 0xa3, 0x64, 0xb, 0x2d, 0xb8, 0x20, 0x59, 0x6 \
  }

//
// Globals
//
extern EFI_SHELL_ENVIRONMENT  *SE;

//
// Some definition to indicate current Shell status
//
#define EFI_SHELL_PROMPT          1
#define EFI_SHELL_PREEXECUTION    2
#define EFI_SHELL_POSTEXECUTION   3
#define STARTUP_FILE_NAME         L"startup.nsh"
#define EFI_NSHELL_MAJOR_VERSION  0x00000001
#define EFI_NSHELL_MINOR_VERSION  0x00000000

#undef EFI_MONOSHELL_CODE
#undef EFI_NO_MONOSHELL_CODE

#if EFI_MONOSHELL == 1
#define EFI_MONOSHELL_CODE(code)    code
#define EFI_NO_MONOSHELL_CODE(code)
#else
#define EFI_MONOSHELL_CODE(code)
#define EFI_NO_MONOSHELL_CODE(code) code
#endif

#endif