/*++

Copyright (c) 2004 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  PeiBind.h

Abstract:

  Tiano PEI core and PEIM binding macros

--*/

#ifndef _PEI_BIND_H_
#define _PEI_BIND_H_

#ifdef EFI_DEBUG

#ifdef EFI_LINUX_EMULATOR
#define EFI_PEI_CORE_ENTRY_POINT(InitFunction)
#define EFI_PEIM_ENTRY_POINT(InitFunction)

#else

#define EFI_PEI_CORE_ENTRY_POINT(InitFunction)                
#define EFI_PEIM_ENTRY_POINT(InitFunction)

#endif

#else

#ifdef EFI_LINUX_EMULATOR
#define EFI_PEI_CORE_ENTRY_POINT(InitFunction)
#define EFI_PEIM_ENTRY_POINT(InitFunction)

#else

#define EFI_PEI_CORE_ENTRY_POINT(InitFunction)                
#define EFI_PEIM_ENTRY_POINT(InitFunction)

#endif
#endif
#endif
