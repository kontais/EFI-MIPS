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

  EfiBind.h

Abstract:

  MIPS32处理器或编译器类型绑定，方便EfiTypes.h引用移植。

--*/

// Linux下模拟时，调用函数方法，暂时未实现
#ifndef _EFI_BIND_H_
#define _EFI_BIND_H_

#ifdef EFI_DEBUG

#ifdef EFI_LINUX_EMULATOR

#define EFI_DRIVER_ENTRY_POINT(InitFunction)
#define EFI_APPLICATION_ENTRY_POINT EFI_DRIVER_ENTRY_POINT

#else

#define EFI_DRIVER_ENTRY_POINT(InitFunction)
#define EFI_APPLICATION_ENTRY_POINT EFI_DRIVER_ENTRY_POINT

#endif // EFI_LINUX_EMULATOR

#else

#define EFI_DRIVER_ENTRY_POINT(InitFunction)                  
#define EFI_APPLICATION_ENTRY_POINT EFI_DRIVER_ENTRY_POINT

#endif // EFI_DEBUG

//
// Make sure we are useing the correct packing rules per EFI specification
//
#pragma pack()

//
// BugBug: 需要检查 long long结构
//
typedef unsigned long long uint64_t;
typedef long long          int64_t;
typedef unsigned int       uint32_t;
typedef int                int32_t;
typedef unsigned short     uint16_t;
typedef short              int16_t;
typedef unsigned char      uint8_t;

// GNU 会定义这些类型，需要避免重复定义 sys/types,h 或stddef.h
#ifndef __int8_t_defined
#define __int8_t_defined
typedef char               int8_t;
#endif

typedef uint32_t  uintn_t;
typedef int32_t   intn_t;


typedef int       register_t;


//
// Processor specific defines
//
#define EFI_MAX_BIT       0x80000000
#define MAX_2_BITS        0xC0000000

//
// MIPS32 最大合法地址
//
#define EFI_MAX_ADDRESS   0xFFFFFFFF

//
//  Bad pointer value to use in check builds.
//  if you see this value you are using uninitialized or free'ed data
//
#define EFI_BAD_POINTER          0xAFAFAFAF
#define EFI_BAD_POINTER_AS_BYTE  0xAF

//
// BugBug: __asm__("break") 仅仅只针对 GNU for MIPS
// Don't do a while(1) because the compiler will optimize away the rest 
// of the function following, so that you run out in the weeds if you 
// skip over it with a debugger.
//
#define EFI_BREAKPOINT()  __asm__("break");
#define EFI_DEADLOOP()    { volatile UINTN __iii; __iii = 1; while (__iii); }

//
// Memory Fence forces serialization, and is needed to support out of order
//  memory transactions. The Memory Fence is mainly used to make sure IO
//  transactions complete in a deterministic sequence, and to syncronize locks
//  an other MP code. Currently no memory fencing is required.
//
#define MEMORY_FENCE() __asm__("sync");

//
// Some compilers don't support the forward reference construct:
//  typedef struct XXXXX. The forward reference is required for 
//  ANSI compatibility.
//
// The following macro provide a workaround for such cases.
//


#ifdef EFI_NO_INTERFACE_DECL
  #define EFI_FORWARD_DECLARATION(x)
#else
  #define EFI_FORWARD_DECLARATION(x) typedef struct _##x x
#endif


//
// Some C compilers optimize the calling conventions to increase performance.
// _EFIAPI is used to make all public APIs follow the standard C calling 
// convention.
//

// #define _EFIAPI __cdecl
#define _EFIAPI


#endif // _EFI_BIND_H_

