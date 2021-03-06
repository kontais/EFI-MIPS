/*++

Copyright (c) 2010, kontais                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:
  Cpu.c

Abstract:

--*/

#include "CpuDxe.h"
#include "Mips.h"
#include "Exception.h"

//
// Global Variables
//
BOOLEAN                              mInterruptState = FALSE;
EFI_MIPS32_INTRRUPT_PROTOCOL         *gMips32Intr = NULL;

//
// The Cpu Architectural Protocol that this Driver produces
//
EFI_HANDLE              mHandle = NULL;
EFI_CPU_ARCH_PROTOCOL   mCpu = {
  CpuFlushCpuDataCache,
  CpuEnableInterrupt,
  CpuDisableInterrupt,
  CpuGetInterruptState,
  CpuInit,
  CpuRegisterInterruptHandler,
  CpuGetTimerValue,
  CpuSetMemoryAttributes,
  1,          // NumberOfTimers
  4,          // DmaBufferAlignment
};

EXCEPTION_VECTOR_TABLE mExcVecTable[EXCEPTION_VECTOR_NUMBER];

EFI_STATUS
EFIAPI
CpuFlushCpuDataCache (
  IN EFI_CPU_ARCH_PROTOCOL           *This,
  IN EFI_PHYSICAL_ADDRESS            Start,
  IN UINT64                          Length,
  IN EFI_CPU_FLUSH_TYPE              FlushType
  )
/*++

Routine Description:
  Flush CPU data cache. If the instruction cache is fully coherent
  with all DMA operations then function can just return EFI_SUCCESS.

Arguments:
  This                - Protocol instance structure
  Start               - Physical address to start flushing from.
  Length              - Number of bytes to flush. Round up to chipset 
                         granularity.
  FlushType           - Specifies the type of flush operation to perform.

Returns: 

  EFI_SUCCESS           - If cache was flushed
  EFI_UNSUPPORTED       - If flush type is not supported.
  EFI_DEVICE_ERROR      - If requested range could not be flushed.

--*/
{
  MipsCacheSync();

  return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
CpuEnableInterrupt (
  IN EFI_CPU_ARCH_PROTOCOL          *This
  )
/*++

Routine Description:
  Enables CPU interrupts.

Arguments:
  This                - Protocol instance structure

Returns: 
  EFI_SUCCESS           - If interrupts were enabled in the CPU
  EFI_DEVICE_ERROR      - If interrupts could not be enabled on the CPU.

--*/
{
  EfiEnableInterrupts (); 
  mInterruptState  = TRUE;
  return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
CpuDisableInterrupt (
  IN EFI_CPU_ARCH_PROTOCOL          *This
  )
/*++

Routine Description:
  Disables CPU interrupts.

Arguments:
  This                - Protocol instance structure

Returns: 
  EFI_SUCCESS           - If interrupts were disabled in the CPU.
  EFI_DEVICE_ERROR      - If interrupts could not be disabled on the CPU.

--*/
{
  EfiDisableInterrupts ();
  
  mInterruptState = FALSE;
  return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
CpuGetInterruptState (
  IN  EFI_CPU_ARCH_PROTOCOL         *This,
  OUT BOOLEAN                       *State
  )
/*++

Routine Description:
  Return the state of interrupts.

Arguments:
  This                - Protocol instance structure
  State               - Pointer to the CPU's current interrupt state

Returns: 
  EFI_SUCCESS           - If interrupts were disabled in the CPU.
  EFI_INVALID_PARAMETER - State is NULL.
  
--*/
{
  if (State == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  *State = mInterruptState;
  return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
CpuInit (
  IN EFI_CPU_ARCH_PROTOCOL           *This,
  IN EFI_CPU_INIT_TYPE               InitType
  )

/*++

Routine Description:
  Generates an INIT to the CPU

Arguments:
  This                - Protocol instance structure
  InitType            - Type of CPU INIT to perform

Returns: 
  EFI_SUCCESS           - If CPU INIT occurred. This value should never be
        seen.
  EFI_DEVICE_ERROR      - If CPU INIT failed.
  EFI_NOT_SUPPORTED     - Requested type of CPU INIT not supported.

--*/
{
  return EFI_UNSUPPORTED;
}


EFI_STATUS
EFIAPI
CpuRegisterInterruptHandler (
  IN EFI_CPU_ARCH_PROTOCOL          *This,
  IN EFI_EXCEPTION_TYPE             InterruptType,
  IN EFI_CPU_INTERRUPT_HANDLER      InterruptHandler
  )
/*++

Routine Description:
  Registers a function to be called from the CPU interrupt handler.

Arguments:
  This                - Protocol instance structure
  InterruptType       - Defines which interrupt to hook. IA-32 valid range
                         is 0x00 through 0xFF
  InterruptHandler    - A pointer to a function of type 
      EFI_CPU_INTERRUPT_HANDLER that is called when a
      processor interrupt occurs. A null pointer
      is an error condition.

Returns: 
  EFI_SUCCESS           - If handler installed or uninstalled.
  EFI_ALREADY_STARTED   - InterruptHandler is not NULL, and a handler for 
        InterruptType was previously installed
  EFI_INVALID_PARAMETER - InterruptHandler is NULL, and a handler for 
        InterruptType was not previously installed.
  EFI_UNSUPPORTED       - The interrupt specified by InterruptType is not
        supported.

--*/
{
  if ((InterruptType >= EXCEPTION_VECTOR_NUMBER) || (InterruptHandler == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  mExcVecTable[InterruptType].Handle = InterruptHandler;
    
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
CpuGetTimerValue (
  IN  EFI_CPU_ARCH_PROTOCOL          *This,
  IN  UINT32                         TimerIndex,
  OUT UINT64                         *TimerValue,
  OUT UINT64                         *TimerPeriod   OPTIONAL
  )
/*++

Routine Description:
  Returns a timer value from one of the CPU's internal timers. There is no
  inherent time interval between ticks but is a function of the CPU 
  frequency.

Arguments:
  This                - Protocol instance structure
  TimerIndex          - Specifies which CPU timer ie requested
  TimerValue          - Pointer to the returned timer value
  TimerPeriod         - 

Returns: 
  EFI_SUCCESS           - If the CPU timer count was returned.
  EFI_UNSUPPORTED       - If the CPU does not have any readable timers
  EFI_DEVICE_ERROR      - If an error occurred reading the timer.
  EFI_INVALID_PARAMETER - TimerIndex is not valid

--*/
{  
  if (TimerValue == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  
  if (TimerIndex == 0) {
    *TimerValue = EfiReadTsc ();
    if (TimerPeriod != NULL) {
      //
      // BugBug: Hard coded. Don't know how to do this generically
      //
      *TimerPeriod = 1000000000;
    }
    return EFI_SUCCESS;
  }
  return EFI_INVALID_PARAMETER;
}

EFI_STATUS
EFIAPI
CpuSetMemoryAttributes (
  IN EFI_CPU_ARCH_PROTOCOL     *This,
  IN EFI_PHYSICAL_ADDRESS      BaseAddress,
  IN UINT64                    Length,
  IN UINT64                    Attributes
  )
/*++

Routine Description:
  Set memory cacheability attributes for given range of memeory

Arguments:
  This                - Protocol instance structure
  BaseAddress         - Specifies the start address of the memory range
  Length              - Specifies the length of the memory range
  Attributes          - The memory cacheability for the memory range

Returns: 
  EFI_SUCCESS           - If the cacheability of that memory range is set successfully
  EFI_UNSUPPORTED       - If the desired operation cannot be done
  EFI_INVALID_PARAMETER - The input parameter is not correct, such as Length = 0

--*/
{
  return EFI_UNSUPPORTED;
}

#if CPU_EXCEPTION_DEBUG_OUTPUT
STATIC
VOID
DumpExceptionDataDebugOut (
  IN EFI_EXCEPTION_TYPE   InterruptType,
  IN EFI_SYSTEM_CONTEXT   SystemContext
  )
{
  UINT32        ErrorCodeFlag;

  ErrorCodeFlag = 0x00027d00;

  DEBUG ((
    EFI_D_ERROR,
    "!!!! MIPS32 Exception Type - %08x !!!!\n",
    InterruptType
    ));

  DEBUG ((
    EFI_D_ERROR,
    "$0-%08x at-%08x v0-%08x v1-%08x\n",
    SystemContext.SystemContextMips32->zero,
    SystemContext.SystemContextMips32->ast,
    SystemContext.SystemContextMips32->v0,
    SystemContext.SystemContextMips32->v1
    ));

  DEBUG ((
    EFI_D_ERROR,
    "a0-%08x a1-%08x a2-%08x a3-%08x\n",
    SystemContext.SystemContextMips32->a0,
    SystemContext.SystemContextMips32->a1,
    SystemContext.SystemContextMips32->a2,
    SystemContext.SystemContextMips32->a3
    ));

  DEBUG ((
    EFI_D_ERROR,
    "t0-%08x t1-%08x t2-%08x t3-%08x\n",
    SystemContext.SystemContextMips32->t0,
    SystemContext.SystemContextMips32->t1,
    SystemContext.SystemContextMips32->t2,
    SystemContext.SystemContextMips32->t3
    ));

  DEBUG ((
    EFI_D_ERROR,
    "t4-%08x t5-%08x t6-%08x t7-%08x\n",
    SystemContext.SystemContextMips32->t4,
    SystemContext.SystemContextMips32->t5,
    SystemContext.SystemContextMips32->t6,
    SystemContext.SystemContextMips32->t7
    ));

  DEBUG ((
    EFI_D_ERROR,
    "s0-%08x s1-%08x s2-%08x s3-%08x\n",
    SystemContext.SystemContextMips32->s0,
    SystemContext.SystemContextMips32->s1,
    SystemContext.SystemContextMips32->s2,
    SystemContext.SystemContextMips32->s3
    ));

  DEBUG ((
    EFI_D_ERROR,
    "s4-%08x s5-%08x s6-%08x s7-%08x\n",
    SystemContext.SystemContextMips32->s4,
    SystemContext.SystemContextMips32->s5,
    SystemContext.SystemContextMips32->s6,
    SystemContext.SystemContextMips32->s7
    ));

  DEBUG ((
    EFI_D_ERROR,
    "t8-%08x t9-%08x\n",
    SystemContext.SystemContextMips32->t8,
    SystemContext.SystemContextMips32->t9
    ));

  DEBUG ((
    EFI_D_ERROR,
    "gp-%08x sp-%08x ra-%08x\n",
    SystemContext.SystemContextMips32->gp,
    SystemContext.SystemContextMips32->sp,
    SystemContext.SystemContextMips32->ra
    ));

  DEBUG ((
    EFI_D_ERROR,
    "status-%08x mullo-%08x mulhi-%08x badvaddr-%08x\n",
    SystemContext.SystemContextMips32->status,
    SystemContext.SystemContextMips32->mullo,
    SystemContext.SystemContextMips32->mulhi,
    SystemContext.SystemContextMips32->badvaddr
    ));

  DEBUG ((
    EFI_D_ERROR,
    "cause-%08x epc-%08x%s\n",
    SystemContext.SystemContextMips32->cause,
    SystemContext.SystemContextMips32->pc,
    (SystemContext.SystemContextMips32->cause & CP0_CAUSE_BD) ? "(+4)" : ""
    ));

  DEBUG ((
    EFI_D_ERROR,
    "*epc-%08x *(epc+4)-%08x\n",
    *(UINT32*)SystemContext.SystemContextMips32->pc,
    *(UINT32*)(SystemContext.SystemContextMips32->pc + 4)
    ));

  return ;
}
#endif

STATIC
UINTN
SPrint (
  IN OUT    CHAR16  *Buffer,
  IN CONST  CHAR16  *Format,
  ...
  )
{
  VA_LIST   Marker;
  UINTN     Index;
  UINTN     Flags;
  UINTN     Width;
  UINT64    Value;

  VA_START (Marker, Format);

  //
  // Process the format string. Stop if Buffer is over run.
  //

  for (Index = 0; *Format != 0; Format++) {
    if (*Format != L'%') {
      Buffer[Index++] = *Format;
    } else {
      
      //
      // Now it's time to parse what follows after %
      // Support: % [ 0 width ] [ l ] x
      // width - fill 0, to ensure the width of x will be "width"
      // l        - UINT64 instead of UINT32
      //
      Width = 0;
      Flags = 0;
      Format ++;

      if (*Format == L'0') {
        Flags |= PREFIX_ZERO;
        do {
          Width += Width * 10 + (*Format - L'0');
          Format ++;
        } while (*Format >= L'1' && *Format <= L'9');
      }

      if (*Format == L'l') {
        Flags |= LONG_TYPE;
        Format ++;
      }

      
      switch (*Format) {
      case 'X':
        Flags |= PREFIX_ZERO;
        Width = sizeof (UINT64) * 2;
        //
        // break skiped on purpose
        //
      case 'x':
        if ((Flags & LONG_TYPE) == LONG_TYPE) {
          Value = VA_ARG (Marker, UINT64);
        } else {
          Value = VA_ARG (Marker, UINTN);
        }

        EfiValueToHexStr (Buffer+Index, Value, Flags, Width);
        
        for ( ; Buffer[Index] != L'\0'; Index ++) {
        }

        break;

      default:
        //
        // if the type is unknown print it to the screen
        //
        Buffer[Index++] = *Format;
      }
    } 
  }
  Buffer[Index++] = '\0'; 
   
  VA_END (Marker);
  return Index;
}

STATIC
VOID
DumpExceptionDataVgaOut (
  IN EFI_EXCEPTION_TYPE   InterruptType,
  IN EFI_SYSTEM_CONTEXT   SystemContext
  )
{
  UINTN         COLUMN_MAX;
  UINTN         ROW_MAX;
  UINT32        ErrorCodeFlag;
  CHAR16        *VideoBufferBase;
  CHAR16        *VideoBuffer;
  UINTN         Index;

  COLUMN_MAX      = 80;
  ROW_MAX         = 25;
  ErrorCodeFlag   = 0x00027d00;
  VideoBufferBase = (CHAR16 *) (UINTN) 0xb8000;
  VideoBuffer     = (CHAR16 *) (UINTN) 0xb8000;

#ifdef EFI32
  SPrint (
    VideoBuffer, 
    L"!!!! IA32 Exception Type - %08x !!!!",
    InterruptType
    );
  VideoBuffer += COLUMN_MAX;
  SPrint (
    VideoBuffer, 
    L"EIP - %08x, CS - %08x, EFLAGS - %08x",
    SystemContext.SystemContextIa32->Eip,
    SystemContext.SystemContextIa32->Cs,
    SystemContext.SystemContextIa32->Eflags
    );
  VideoBuffer += COLUMN_MAX;
  if (ErrorCodeFlag & (1 << InterruptType)) {
    SPrint (
      VideoBuffer,
      L"ExceptionData - %08x",
      SystemContext.SystemContextIa32->ExceptionData
    );
    VideoBuffer += COLUMN_MAX;
  }
  SPrint (
    VideoBuffer,
    L"EAX - %08x, ECX - %08x, EDX - %08x, EBX - %08x",
    SystemContext.SystemContextIa32->Eax,
    SystemContext.SystemContextIa32->Ecx,
    SystemContext.SystemContextIa32->Edx,
    SystemContext.SystemContextIa32->Ebx
    );
  VideoBuffer += COLUMN_MAX;

  SPrint (
    VideoBuffer,
    L"ESP - %08x, EBP - %08x, ESI - %08x, EDI - %08x",
    SystemContext.SystemContextIa32->Esp,
    SystemContext.SystemContextIa32->Ebp,
    SystemContext.SystemContextIa32->Esi,
    SystemContext.SystemContextIa32->Edi
    );
  VideoBuffer += COLUMN_MAX;

  SPrint (
    VideoBuffer,
    L"DS - %08x, ES - %08x, FS - %08x, GS - %08x, SS - %08x",
    SystemContext.SystemContextIa32->Ds,
    SystemContext.SystemContextIa32->Es,
    SystemContext.SystemContextIa32->Fs,
    SystemContext.SystemContextIa32->Gs,
    SystemContext.SystemContextIa32->Ss
    );
  VideoBuffer += COLUMN_MAX;

  SPrint (
    VideoBuffer,
    L"GDTR - %08x %08x, IDTR - %08x %08x",
    SystemContext.SystemContextIa32->Gdtr[0],
    SystemContext.SystemContextIa32->Gdtr[1],
    SystemContext.SystemContextIa32->Idtr[0],
    SystemContext.SystemContextIa32->Idtr[1]
    );
  VideoBuffer += COLUMN_MAX;

  SPrint (
    VideoBuffer,
    L"LDTR - %08x, TR - %08x",
    SystemContext.SystemContextIa32->Ldtr,
    SystemContext.SystemContextIa32->Tr
    );
  VideoBuffer += COLUMN_MAX;

  SPrint (
    VideoBuffer,
    L"CR0 - %08x, CR2 - %08x, CR3 - %08x, CR4 - %08x",
    SystemContext.SystemContextIa32->Cr0,
    SystemContext.SystemContextIa32->Cr2,
    SystemContext.SystemContextIa32->Cr3,
    SystemContext.SystemContextIa32->Cr4
    );
  VideoBuffer += COLUMN_MAX;

  SPrint (
    VideoBuffer,
    L"DR0 - %08x, DR1 - %08x, DR2 - %08x, DR3 - %08x",
    SystemContext.SystemContextIa32->Dr0,
    SystemContext.SystemContextIa32->Dr1,
    SystemContext.SystemContextIa32->Dr2,
    SystemContext.SystemContextIa32->Dr3
    );
  VideoBuffer += COLUMN_MAX;

  SPrint (
    VideoBuffer,
    L"DR6 - %08x, DR7 - %08x",
    SystemContext.SystemContextIa32->Dr6,
    SystemContext.SystemContextIa32->Dr7
    );
  VideoBuffer += COLUMN_MAX;
#else
  SPrint (
    VideoBuffer,
    L"!!!! X64 Exception Type - %016lx !!!!",
    (UINT64)InterruptType
    );
  VideoBuffer += COLUMN_MAX;

  SPrint (
    VideoBuffer,
    L"RIP - %016lx, CS - %016lx, RFLAGS - %016lx",
    SystemContext.SystemContextX64->Rip,
    SystemContext.SystemContextX64->Cs,
    SystemContext.SystemContextX64->Rflags
    );
  VideoBuffer += COLUMN_MAX;

  if (ErrorCodeFlag & (1 << InterruptType)) {
    SPrint (
      VideoBuffer,
      L"ExceptionData - %016lx",
      SystemContext.SystemContextX64->ExceptionData
      );
    VideoBuffer += COLUMN_MAX;
  }

  SPrint (
    VideoBuffer,
    L"RAX - %016lx, RCX - %016lx, RDX - %016lx",
    SystemContext.SystemContextX64->Rax,
    SystemContext.SystemContextX64->Rcx,
    SystemContext.SystemContextX64->Rdx
    );
  VideoBuffer += COLUMN_MAX;

  SPrint (
    VideoBuffer,
    L"RBX - %016lx, RSP - %016lx, RBP - %016lx",
    SystemContext.SystemContextX64->Rbx,
    SystemContext.SystemContextX64->Rsp,
    SystemContext.SystemContextX64->Rbp
    );
  VideoBuffer += COLUMN_MAX;

  SPrint (
    VideoBuffer,
    L"RSI - %016lx, RDI - %016lx",
    SystemContext.SystemContextX64->Rsi,
    SystemContext.SystemContextX64->Rdi
    );
  VideoBuffer += COLUMN_MAX;

  SPrint (
    VideoBuffer,
    L"R8 - %016lx, R9 - %016lx, R10 - %016lx",
    SystemContext.SystemContextX64->R8,
    SystemContext.SystemContextX64->R9,
    SystemContext.SystemContextX64->R10
    );
  VideoBuffer += COLUMN_MAX;

  SPrint (
    VideoBuffer,
    L"R11 - %016lx, R12 - %016lx, R13 - %016lx",
    SystemContext.SystemContextX64->R11,
    SystemContext.SystemContextX64->R12,
    SystemContext.SystemContextX64->R13
    );
  VideoBuffer += COLUMN_MAX;

  SPrint (
    VideoBuffer,
    L"R14 - %016lx, R15 - %016lx",
    SystemContext.SystemContextX64->R14,
    SystemContext.SystemContextX64->R15
    );
  VideoBuffer += COLUMN_MAX;

  SPrint (
    VideoBuffer,
    L"DS - %016lx, ES - %016lx, FS - %016lx",
    SystemContext.SystemContextX64->Ds,
    SystemContext.SystemContextX64->Es,
    SystemContext.SystemContextX64->Fs
    );
  VideoBuffer += COLUMN_MAX;

  SPrint (
    VideoBuffer,
    L"GS - %016lx, SS - %016lx",
    SystemContext.SystemContextX64->Gs,
    SystemContext.SystemContextX64->Ss
    );
  VideoBuffer += COLUMN_MAX;

  SPrint (
    VideoBuffer,
    L"GDTR - %016lx %016lx, LDTR - %016lx",
    SystemContext.SystemContextX64->Gdtr[0],
    SystemContext.SystemContextX64->Gdtr[1],
    SystemContext.SystemContextX64->Ldtr
    );
  VideoBuffer += COLUMN_MAX;

  SPrint (
    VideoBuffer,
    L"IDTR - %016lx %016lx, TR - %016lx",
    SystemContext.SystemContextX64->Idtr[0],
    SystemContext.SystemContextX64->Idtr[1],
    SystemContext.SystemContextX64->Tr
    );
  VideoBuffer += COLUMN_MAX;

  SPrint (
    VideoBuffer,
    L"CR0 - %016lx, CR2 - %016lx, CR3 - %016lx",
    SystemContext.SystemContextX64->Cr0,
    SystemContext.SystemContextX64->Cr2,
    SystemContext.SystemContextX64->Cr3
    );
  VideoBuffer += COLUMN_MAX;

  SPrint (
    VideoBuffer,
    L"CR4 - %016lx, CR8 - %016lx",
    SystemContext.SystemContextX64->Cr4,
    SystemContext.SystemContextX64->Cr8
    );
  VideoBuffer += COLUMN_MAX;

  SPrint (
    VideoBuffer,
    L"DR0 - %016lx, DR1 - %016lx, DR2 - %016lx",
    SystemContext.SystemContextX64->Dr0,
    SystemContext.SystemContextX64->Dr1,
    SystemContext.SystemContextX64->Dr2
    );
  VideoBuffer += COLUMN_MAX;

  SPrint (
    VideoBuffer,
    L"DR3 - %016lx, DR6 - %016lx, DR7 - %016lx",
    SystemContext.SystemContextX64->Dr3,
    SystemContext.SystemContextX64->Dr6,
    SystemContext.SystemContextX64->Dr7
    );
  VideoBuffer += COLUMN_MAX;
#endif

  for (Index = 0; Index < COLUMN_MAX * ROW_MAX; Index ++) {
    if (Index > (UINTN)(VideoBuffer - VideoBufferBase)) {
      VideoBufferBase[Index] = 0x0c20;
    } else {
      VideoBufferBase[Index] |= 0x0c00;
    }
  }

  return ;
}

#if CPU_EXCEPTION_VGA_SWITCH
STATIC
UINT16
SwitchVideoMode (
  UINT16    NewVideoMode
  )
/*++
Description
  Switch Video Mode from current mode to new mode, and return the old mode.
  Use Thuink

Arguments
  NewVideoMode - new video mode want to set

Return
  UINT16       - (UINT16) -1 indicates failure
                 Other value indicates the old mode, which can be used for restore later
 
--*/
{
  return 0;
#if 0
  EFI_STATUS                      Status;
  EFI_LEGACY_BIOS_THUNK_PROTOCOL  *LegacyBios;
  EFI_IA32_REGISTER_SET           Regs;
  UINT16                          OriginalVideoMode = (UINT16) -1;
  
  //
  // See if the Legacy BIOS Protocol is available
  //
  Status = gBS->LocateProtocol (&gEfiLegacyBiosThunkProtocolGuid, NULL, (VOID **) &LegacyBios);
  if (EFI_ERROR (Status)) {
    return OriginalVideoMode;
  }

  //
  // VESA SuperVGA BIOS - GET CURRENT VIDEO MODE
  // AX = 4F03h
  // Return:AL = 4Fh if function supported
  // AH = status 00h successful
  // BX = video mode (see #0082,#0083)
  //
  gBS->SetMem (&Regs, sizeof (Regs), 0);
  Regs.X.AX = 0x4F03;
  LegacyBios->Int86 (LegacyBios, 0x10, &Regs);
  if (Regs.X.AX == 0x004F) {
    OriginalVideoMode = Regs.X.BX;
  } else {
    //
    // VIDEO - GET CURRENT VIDEO MODE
    // AH = 0Fh
    // Return:AH = number of character columns
    // AL = display mode (see #0009 at AH=00h)
    // BH = active page (see AH=05h)
    //
    gBS->SetMem (&Regs, sizeof (Regs), 0);
    Regs.H.AH = 0x0F;
    LegacyBios->Int86 (LegacyBios, 0x10, &Regs);
    OriginalVideoMode = Regs.H.AL;
  }

  //
  // Set new video mode
  //
  if (NewVideoMode < 0x100) {
    //
    // Set the 80x25 Text VGA Mode: Assume successful always
    //
    // VIDEO - SET VIDEO MODE
    // AH = 00h
    // AL = desired video mode (see #0009)
    // Return:AL = video mode flag (Phoenix, AMI BIOS)
    // 20h mode > 7
    // 30h modes 0-5 and 7
    // 3Fh mode 6
    // AL = CRT controller mode byte (Phoenix 386 BIOS v1.10)
    //
    gBS->SetMem (&Regs, sizeof (Regs), 0);
    Regs.H.AH = 0x00;
    Regs.H.AL = (UINT8) NewVideoMode;
    LegacyBios->Int86 (LegacyBios, 0x10, &Regs);

    //
    // VIDEO - TEXT-MODE CHARGEN - LOAD ROM 8x16 CHARACTER SET (VGA)
    // AX = 1114h
    // BL = block to load
    // Return:Nothing
    //
    gBS->SetMem (&Regs, sizeof (Regs), 0);
    Regs.H.AH = 0x11;
    Regs.H.AL = 0x14;
    Regs.H.BL = 0;
    LegacyBios->Int86 (LegacyBios, 0x10, &Regs);
  } else {
    //
    //    VESA SuperVGA BIOS - SET SuperVGA VIDEO MODE
    //    AX = 4F02h
    //    BX = mode (see #0082,#0083)
    //    bit 15 set means don't clear video memory
    //    bit 14 set means enable linear framebuffer mode (VBE v2.0+)
    //    Return:AL = 4Fh if function supported
    //    AH = status
    //    00h successful
    //    01h failed
    //
    gBS->SetMem (&Regs, sizeof (Regs), 0);
    Regs.X.AX = 0x4F02;
    Regs.X.BX = NewVideoMode;
    LegacyBios->Int86 (LegacyBios, 0x10, &Regs);
    if (Regs.X.AX != 0x004F) {
      DEBUG ((EFI_D_ERROR, "SORRY: Cannot set to video mode: 0x%04X!\n", NewVideoMode));
      return (UINT16) -1;
    }
  }

  return OriginalVideoMode;
#endif
}
#endif

VOID
EFIAPI
InstallException (
  VOID
  )
{
  UINT32 Cp0Status;
  
  EfiCommonLibCopyMem(GEN_EXC_VEC,Exception, ExceptionEnd - Exception);
  EfiCommonLibCopyMem(TLB_MISS_EXC_VEC,ExceptionTlbMiss, ExceptionTlbMissEnd - ExceptionTlbMiss);
  CpuFlushCpuDataCache(NULL, KSEG0_BASE,0x400, 0);

  Cp0Status = CP0_GetCause();
  CP0_SetCause(Cp0Status & CP0_STATUS_BEV);
  
}

VOID
EFIAPI
InitException (
  VOID
  )
{
  EfiCommonLibCopyMem(GEN_EXC_VEC,Exception, ExceptionEnd - Exception);
  EfiCommonLibCopyMem(TLB_MISS_EXC_VEC,ExceptionTlbMiss, ExceptionTlbMissEnd - ExceptionTlbMiss);
  CpuFlushCpuDataCache(NULL, KSEG0_BASE,0x400, 0);
}

VOID
ExceptionHandler (
  IN EFI_EXCEPTION_TYPE    InterruptType,
  IN EFI_SYSTEM_CONTEXT    SystemContext
  )
{
#if CPU_EXCEPTION_VGA_SWITCH
  UINT16                          VideoMode;
#endif

#if CPU_EXCEPTION_DEBUG_OUTPUT
  DumpExceptionDataDebugOut (InterruptType, SystemContext);
#endif

#if CPU_EXCEPTION_VGA_SWITCH
  //
  // Switch to text mode for RED-SCREEN output
  //
  VideoMode = SwitchVideoMode (0x83);
  if (VideoMode == (UINT16) -1) {
    DEBUG ((EFI_D_ERROR, "Video Mode Unknown!\n"));
  }
#endif

  //DumpExceptionDataVgaOut (InterruptType, SystemContext);

  //
  // Use this macro to hang so that the compiler does not optimize out
  // the following RET instructions. This allows us to return if we
  // have a debugger attached.
  //
  EFI_DEADLOOP ();

#if CPU_EXCEPTION_VGA_SWITCH
  //
  // Switch back to the old video mode
  //
  if (VideoMode != (UINT16)-1) {
    SwitchVideoMode (VideoMode);
  }
#endif

  return ;
}

VOID
InterruptHandler (
  IN EFI_EXCEPTION_TYPE    InterruptType,
  IN EFI_SYSTEM_CONTEXT    SystemContext
  )
{
  ;
}

VOID
InterruptDispatcher (
  IN EFI_EXCEPTION_TYPE    InterruptType,
  IN EFI_SYSTEM_CONTEXT    SystemContext
  )
{
  EFI_STATUS       Status;
  UINT32           Cp0Cause;
  UINT32           Cp0Status;
  UINT32           Pending;
  UINT32           InterruptVector;
  EFI_MIPS32_INTR  Intr;

  //
  // Clear soft interrupt 0/1
  //
  Status = gMips32Intr->ClearPendingIntr (gMips32Intr, EfiMips32Intr0);
  Status = gMips32Intr->ClearPendingIntr (gMips32Intr, EfiMips32Intr1);

  Cp0Cause  = SystemContext.SystemContextMips32->cause & CP0_CAUSE_IP;
  Cp0Status = SystemContext.SystemContextMips32->status & CP0_STATUS_IM;

  Pending = Cp0Cause & Cp0Status;

  Pending >>= CP0_CAUSE_IP_SHIFT;

  for (Intr = EfiMips32Intr0; Intr < EfiMips32IntrMax; Intr ++ ) {
    if (Pending & (1<<Intr)) {
      Status = gMips32Intr->GetVector (gMips32Intr, Intr, &InterruptVector);
      ASSERT_EFI_ERROR(Status);
      mExcVecTable[InterruptVector].Handle(InterruptVector, SystemContext);
    }
  }
}

VOID
ExceptionDispatcher (
  IN EFI_EXCEPTION_TYPE    InterruptType,
  IN EFI_SYSTEM_CONTEXT    SystemContext
  )
{
  mExcVecTable[InterruptType].Handle(InterruptType, SystemContext);
}

EFI_DRIVER_ENTRY_POINT (InitializeCpu)

EFI_STATUS
EFIAPI
InitializeCpu (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
/*++

Routine Description:
  Initialize the state information for the CPU Architectural Protocol

Arguments:
  ImageHandle of the loaded driver
  Pointer to the System Table

Returns:
  EFI_SUCCESS           - thread can be successfully created
  EFI_OUT_OF_RESOURCES  - cannot allocate protocol data structure
  EFI_DEVICE_ERROR      - cannot create the thread

--*/
{
  EFI_STATUS                  Status;
  EFI_MIPS32_INTR             Intr;
  UINT32                      InterruptVector;

  EfiInitializeDriverLib (ImageHandle, SystemTable);

  //
  // Find the Legacy8259 protocol.
  //
  Status = gBS->LocateProtocol (&gEfiMips32InterruptProtocolGuid, NULL, (VOID **) &gMips32Intr);
  ASSERT_EFI_ERROR (Status);

  //
  // Install Exception Handler (0 ~ 31)
  //
  for (InterruptVector = 0; InterruptVector < 32; InterruptVector++) {
    CpuRegisterInterruptHandler (NULL, InterruptVector, ExceptionHandler);
  }

  //
  // Unaligned Access Handler
  //
  CpuRegisterInterruptHandler (NULL, EXCEPT_MIPS32_ADEL, ExceptionHandlerADESL);
  CpuRegisterInterruptHandler (NULL, EXCEPT_MIPS32_ADES, ExceptionHandlerADESL);

  //
  // Install interrupt vector ( 32 ~ 39)
  //
  for (Intr = EfiMips32Intr0; Intr < EfiMips32IntrMax; Intr++) {
    InterruptVector = 0;
    Status = gMips32Intr->GetVector (gMips32Intr, Intr, &InterruptVector);
    ASSERT_EFI_ERROR (Status);
    CpuRegisterInterruptHandler (NULL, InterruptVector, InterruptHandler);
  }

  //
  // Install TLB miss Exception Handler (40 ~ 41)
  //
  CpuRegisterInterruptHandler (NULL, EXCEPT_MIPS32_TLB_MISS, ExceptionHandler);
  CpuRegisterInterruptHandler (NULL, EXCEPT_MIPS32_XTLB_MISS, ExceptionHandler);


  //
  // Install Mips RAM Common Excepiton Code
  //
  InstallException ();

  //
  // Enable Global Interrupt
  //
  CpuEnableInterrupt(NULL);
  
  //
  // Install CPU Architectural Protocol and the thunk protocol
  //
  mHandle = NULL;
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &mHandle,
                  &gEfiCpuArchProtocolGuid,
                  &mCpu,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);

  return EFI_SUCCESS;
}

