/*++

Copyright (c) 2009, kontais                                                    
Copyright (c) 2004 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  TianoBind.h

Abstract:

  处理器和编译器规格MIPS32的Tiano封装

--*/

#ifndef _TIANO_BIND_H_
#define _TIANO_BIND_H_

#include "EfiBind.h"

// Linux下模拟时，调用函数方法，暂时未实现
#ifdef EFI_DEBUG

#ifdef EFI_LINUX_EMULATOR

#define EFI_DXE_ENTRY_POINT(InitFunction)

#else

#define EFI_DXE_ENTRY_POINT(InitFunction)

#endif // EFI_LINUX_EMULATOR

#endif // EFI_DEBUG

#endif // _TIANO_BIND_H_

