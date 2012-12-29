/*++

Copyright (c) 2005 - 2007, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

    efidebug.h

Abstract:

    EFI library debug functions



Revision History

--*/
#ifndef _SHELL_DEBUG_H
#define _SHELL_DEBUG_H

extern UINTN  EFIDebug;

#ifdef EFI_DEBUG
#ifdef DEBUG
#undef DEBUG
#define DBGASSERT(a)  EfiDebugAssert (__FILE__, __LINE__, #a)
#define DEBUG(a)      DbgPrint a
#define UPDATE_DEBUG_MASK() { \
    VOID  *p; \
    p = LibGetVariable (L"EFIDebug", &gEfiGenericVariableGuid); \
    if (NULL == p) { \
      EFIDebug = EFI_D_ERROR; \
    } else { \
      EFIDebug = *(UINTN *) p; \
    } \
  }
#endif
#else
#ifdef DEBUG
#undef DEBUG
#define DBGASSERT(a)
#define DEBUG(a)
#define UPDATE_DEBUG_MASK()
#endif
#endif

#ifdef EFI_DBUG_MASK
#undef EFI_DBUG_MASK
#define EFI_DBUG_MASK (EFI_D_ERROR)
#endif

#ifdef EFI_DEBUG_CLEAR_MEMORY

#define DBGSETMEM(a, l) SetMem (a, l, (CHAR8) BAD_POINTER)

#else

#define DBGSETMEM(a, l)

#endif

#ifdef EFI_DEBUG
#undef ASSERT
#undef ASSERT_LOCKED
#undef ASSERT_STRUCT
#endif

#ifdef EFI_DEBUG

#define ASSERT(a) \
  if (!(a)) DBGASSERT (a)
#define ASSERT_LOCKED(l) \
  if (!(l)->Lock) DBGASSERT (l not locked)
#define ASSERT_STRUCT(p, t) DBGASSERT (t not structure), p

#else

#define ASSERT(a)
#define ASSERT_LOCKED(l)
#define ASSERT_STRUCT(p, t)

#endif

EFI_STATUS
EfiDebugAssertInit (
  VOID
  );

VOID
EFIDebugVariable (
  VOID
  );
#endif
    
