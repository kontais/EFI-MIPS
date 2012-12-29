/*
 * Copyright (c) 2000
 * Intel Corporation.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 * 
 *    This product includes software developed by Intel Corporation and
 *    its contributors.
 * 
 * 4. Neither the name of Intel Corporation or its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY INTEL CORPORATION AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL INTEL CORPORATION OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */

#include <efi.h>

EFI_GUID NtEmulatorId = {
    0x71ce1411, 0xa993, 0x11d2, 0x8e, 0x4a, 0x0, 0xa0, 0xc9, 0x69, 0x72, 0x3b 
};

EFI_GUID LoadedImageProtocol = LOADED_IMAGE_PROTOCOL;

EFI_LOADED_IMAGE	*gLoadedImage;

#define Print( x ) pSystemTable->ConOut->OutputString( pSystemTable->ConOut, x )
#define VARNAME         L"InstallPhysicalFloppy" 

BOOLEAN
FindStr( CHAR16 *Str )
{
    CHAR16  *pStart;
    UINT32  OptionsSize;
    int     i;

    OptionsSize = gLoadedImage->LoadOptionsSize;
    pStart = gLoadedImage->LoadOptions;

    while (OptionsSize ) {
        for (i = 0; OptionsSize &&
                    Str[i] &&
                    pStart[i] == Str[i]; i++, OptionsSize-- )
            ;

        if ( Str[i] == 0 )
            return ( TRUE );
        pStart += (i + 1);
        OptionsSize--;
    }

    return ( FALSE );
}

EFI_STATUS
AppEntry(
    EFI_HANDLE          ImageHandle,
    EFI_SYSTEM_TABLE    *pSystemTable
	)
{
    EFI_STATUS              Status;
    EFI_BOOT_SERVICES       *BS;
    EFI_RUNTIME_SERVICES    *RT;

    BS = pSystemTable->BootServices;
    RT = pSystemTable->RuntimeServices;

    Status = BS->HandleProtocol (
                        ImageHandle,
                        &LoadedImageProtocol,
                        (VOID*)&gLoadedImage
                        );
    if ( ! EFI_ERROR( Status ) ) {
        if ( FindStr( L"on" ) ) {
            Status = RT->SetVariable(
                                 VARNAME,
                                 &NtEmulatorId,
                                 EFI_VARIABLE_NON_VOLATILE |
                                 EFI_VARIABLE_BOOTSERVICE_ACCESS,
                                 1,
                                 (VOID*)RT // just a valid pointer
                                 );
            if ( EFI_ERROR( Status ) ) {
                Print( L"Error setting variable\n\r");
            }
        } else if (FindStr( L"off" ) ) {
            Status = RT->SetVariable(
                                 VARNAME,
                                 &NtEmulatorId,
                                 EFI_VARIABLE_NON_VOLATILE |
                                 EFI_VARIABLE_BOOTSERVICE_ACCESS,
                                 0,
                                 NULL
                                 );
            if ( EFI_ERROR( Status ) ) {
                Print( L"Error deleting variable\n\r");
            }
        } else {
            Print( L"Usage: NtFloppy [on|off]\n\r");
        }
    } else {
        Print( L"We've got big trouble\n\r");
    }

    return 0;
}
