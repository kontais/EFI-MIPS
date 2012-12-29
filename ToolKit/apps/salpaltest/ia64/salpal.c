/*++
 
Copyright (c) 1998  Intel Corporation

Module Name:

    salpal.c
    
Abstract:
    
    Tests some sample sal/pal procs using library functions


Revision History

--*/
 
#include "efi.h"
#include "efilib.h"

#define PAL_DEBUG_INFO        0x0B
#define SAL_PCI_CONFIG_READ   0x01000010

EFI_STATUS
InitializeSalPalApplication (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    )
{
    EFI_INPUT_KEY       Key;
    rArg                ReturnValue;

    /* 
     *  Initialize the Library. Set BS, RT, &ST globals
     *   BS = Boot Services RT = RunTime Services
     *   ST = System Table
     */
    InitializeLib (ImageHandle, SystemTable);

    Print(L"SAL & PAL Proc test application started\n");

    LibPalProc(PAL_DEBUG_INFO,0,0,0, &ReturnValue);
    if (!ReturnValue.p0) {
        Print(L"PAL_PROC returned: Instruction debug regs supported is  %x\n",ReturnValue.p1);
        Print(L"PAL_PROC returned: Data debug regs supported is  %x\n",ReturnValue.p2);
        Print(L"\n");
    }
    else
        Print(L"PAL Proc returned Error %x\n",ReturnValue.p0);


    LibSalProc(SAL_PCI_CONFIG_READ,0,4,0,0,0,0,0, &ReturnValue);
    if (!ReturnValue.p0) {
//        Print(L"SAL_PROC returned: Freq in ticks per sec is  %x\n",ReturnValue.p1);
//        Print(L"SAL_PROC returned: Drift in ppm is  %x\n",ReturnValue.p2);
        Print(L"SAL_PROC returned: PCI read is  %x\n",ReturnValue.p1);
        Print(L"\n");
    }
    else
        Print(L"SAL Proc returned Error %08x\n",ReturnValue.p0);


    Print(L"\n%EHit any key to exit this image%N");
    WaitForSingleEvent (ST->ConIn->WaitForKey, 0);

    /* 
     *  flush the KB buffer once wait is satisfied
     */

    ST->ConIn->ReadKeyStroke(ST->ConIn,&Key);

    Print(L"\n");

    return EFI_SUCCESS;
}

