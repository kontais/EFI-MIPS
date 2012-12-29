/*++

Copyright (c)  1999 - 2003 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

    efefind.h

Abstract:

    EFI to compile bindings




Revision History

--*/

#pragma pack()


//
// Basic int types of various widths
//

typedef unsigned long long  uint64_t;
typedef long long           int64_t;
typedef unsigned int        uint32_t;
typedef int                 int32_t;
typedef unsigned short      uint16_t;
typedef short               int16_t;
typedef unsigned char       uint8_t;
#ifndef __int8_t_defined
#define __int8_t_defined
typedef char               int8_t;
#endif

//
// Basic EFI types of various widths
//

typedef uint64_t   UINT64;
typedef int64_t    INT64;

#ifndef _BASETSD_H_
  typedef uint32_t   UINT32;
  typedef int32_t    INT32;
#endif

typedef uint16_t   UINT16;
typedef int16_t    INT16;
typedef uint8_t    UINT8;
typedef int8_t     INT8;


#undef VOID
#define VOID    void


typedef int32_t    INTN;
typedef uint32_t   UINTN;

#define POST_CODE(_Data)

#define EFIERR(a)           (0x80000000 | a)
#define EFI_ERROR_MASK      0x80000000
#define EFIERR_OEM(a)       (0xc0000000 | a)      


#define BAD_POINTER         0xFBFBFBFB
#define MAX_ADDRESS         0xFFFFFFFF

#define BREAKPOINT()  __asm__("break");

//
// Pointers must be aligned to these address to function
//

#define MIN_ALIGNMENT_SIZE  4

#define ALIGN_VARIABLE(Value ,Adjustment) \
            (UINTN)Adjustment = 0; \
            if((UINTN)Value % MIN_ALIGNMENT_SIZE) \
                (UINTN)Adjustment = MIN_ALIGNMENT_SIZE - ((UINTN)Value % MIN_ALIGNMENT_SIZE); \
            Value = (UINTN)Value + (UINTN)Adjustment


//
// Define macros to build data structure signatures from characters.
//

#define EFI_SIGNATURE_16(A,B)             ((A) | (B<<8))
#define EFI_SIGNATURE_32(A,B,C,D)         (EFI_SIGNATURE_16(A,B)     | (EFI_SIGNATURE_16(C,D)     << 16))
#define EFI_SIGNATURE_64(A,B,C,D,E,F,G,H) (EFI_SIGNATURE_32(A,B,C,D) | ((UINT64)(EFI_SIGNATURE_32(E,F,G,H)) << 32))

//
// EFIAPI - prototype calling convention for EFI function pointers
// BOOTSERVICE - prototype for implementation of a boot service interface
// RUNTIMESERVICE - prototype for implementation of a runtime service interface
// RUNTIMEFUNCTION - prototype for implementation of a runtime function that is not a service
//

#ifndef EFIAPI              // Forces EFI calling conventions reguardless of compiler options 
  #if _MSC_EXTENSIONS
    #define EFIAPI __cdecl  // Force C calling convention for Microsoft C compiler 
  #else
    #define EFIAPI          // Substitute expresion to force C calling convention 
  #endif
#endif

#define BOOTSERVICE
#define RUNTIMESERVICE
#define RUNTIMEFUNCTION



#define VOLATILE    volatile

#define MEMORY_FENCE()  __asm__("sync");

#ifdef EFI_NO_INTERFACE_DECL
  #define EFI_FORWARD_DECLARATION(x)
  #define EFI_INTERFACE_DECL(x)
#else
  #define EFI_FORWARD_DECLARATION(x) typedef struct _##x x
  #define EFI_INTERFACE_DECL(x) typedef struct x
#endif

#ifdef EFI_NT_EMULATOR
  #define EFI_DRIVER_ENTRY_POINT(InitFunction)
#else // EFI_NT_EMULATOR 
  #define EFI_DRIVER_ENTRY_POINT(InitFunction)
#endif // EFI_FW_NT 

