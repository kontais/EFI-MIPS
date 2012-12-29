/*++

Copyright (c) 2005 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

    Data.c

Abstract:

    EFI library global data

Revision History

--*/

#include "EfiShellLib.h"

//
// ShellLibInitialized - TRUE once InitializeShellLib() is called for the first time
//
BOOLEAN                         ShellLibInitialized = FALSE;

//
// ST - pointer to the EFI system table
//
EFI_SYSTEM_TABLE                *ST;

//
// BS - pointer to the boot services table
//
EFI_BOOT_SERVICES               *BS;

EFI_SHELL_INTERFACE             *SI;

EFI_SHELL_ENVIRONMENT           *SE;

EFI_SHELL_ENVIRONMENT2          *SE2;

//
// Default pool allocation type
//
EFI_MEMORY_TYPE                 PoolAllocationType = EfiBootServicesData;

//
// Unicode collation functions that are in use
//
EFI_UNICODE_COLLATION_PROTOCOL  LibStubUnicodeInterface = {
  LibStubStriCmp,
  LibStubMetaiMatch,
  LibStubStrLwr,
  LibStubStrUpr,
  NULL, // FatToStr
  NULL, // StrToFat
  NULL  // SupportedLanguages
};

EFI_UNICODE_COLLATION_PROTOCOL  *UnicodeInterface = &LibStubUnicodeInterface;

EFI_GUID                        ShellEnvProtocol = SHELL_ENVIRONMENT_INTERFACE_PROTOCOL;

EFI_DEVICE_PATH_PROTOCOL        LibEndDevicePath[] = {
  END_DEVICE_PATH_TYPE,
  END_ENTIRE_DEVICE_PATH_SUBTYPE,
  END_DEVICE_PATH_LENGTH,
  0
};

//
// protocol GUIDs and other miscellaneous GUIDs
//
EFI_GUID                        NullGuid                  = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
EFI_GUID                        UnknownDeviceGuid         = UNKNOWN_DEVICE_GUID;
