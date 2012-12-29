/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  shelltypes.h

Abstract:

  Defines data structures for shell core and application



Revision History

--*/

#ifndef _SHELLTYPES_H_
#define _SHELLTYPES_H_

//
// This module is included by shell core and library
// it defines internal data structures used in shell
//
#if defined(__cplusplus)
extern "C"
{
#endif

#include "Tiano.h"
#include "LinkedList.h"

#include EFI_PROTOCOL_DEFINITION (FileInfo)
#include EFI_PROTOCOL_DEFINITION (SimpleFileSystem)

#define VARIABLE_SIGNATURE  EFI_SIGNATURE_32 ('v', 'i', 'd', ' ')

#ifdef EFI_BOOTSHELL
#define EFI_BOOTSHELL_CODE(code)    code
#define EFI_NO_BOOTSHELL_CODE(code)
#else
#define EFI_BOOTSHELL_CODE(code)
#define EFI_NO_BOOTSHELL_CODE(code) code
#endif

#ifdef EFI_MONOSHELL
#define EFI_MONOSHELL_CODE(code)    code
#define EFI_NO_MONOSHELL_CODE(code)
#else
#define EFI_MONOSHELL_CODE(code)
#define EFI_NO_MONOSHELL_CODE(code) code
#endif

#ifdef EFI64
#define EFI64_CODE(code)   code
#else
#define EFI64_CODE(code)
#endif

#ifdef EFI_NT_EMULATOR
#define EFI_NT_EMULATOR_CODE(code)    code
#define EFI_NO_NT_EMULATOR_CODE(code)
#else
#define EFI_NT_EMULATOR_CODE(code)
#define EFI_NO_NT_EMULATOR_CODE(code) code
#endif

#ifdef EFI_BUILD_VERSION
#define EFI_BUILD_VERSION_CODE(code)   code
#else
#define EFI_BUILD_VERSION_CODE(code)
#endif

typedef struct {
  UINTN           Signature;
  EFI_LIST_ENTRY  Link;
  CHAR16          *Name;

  UINTN           ValueSize;
  union {
    UINT8   *Value;
    CHAR16  *Str;
  } u;

  CHAR16  *CurDir;
  UINT8   Flags;
} VARIABLE_ID;

typedef
VOID
(EFIAPI *SHELLENV_DUMP_PROTOCOL_INFO) (
  IN EFI_HANDLE                   Handle,
  IN VOID                         *Interface
  );

#define PROTOCOL_INFO_SIGNATURE EFI_SIGNATURE_32 ('s', 'p', 'i', 'n')
typedef struct {
  UINTN                       Signature;
  EFI_LIST_ENTRY              Link;

  //
  // parsing info for the protocol
  //
  EFI_GUID                    ProtocolId;
  CHAR16                      *IdString;
  SHELLENV_DUMP_PROTOCOL_INFO DumpToken;
  SHELLENV_DUMP_PROTOCOL_INFO DumpInfo;

  //
  // database info on which handles are supporting this protocol
  //
  UINTN                       NoHandles;
  EFI_HANDLE                  *Handles;

} PROTOCOL_INFO;

#define DEFAULT_CMD_SIGNATURE EFI_SIGNATURE_32 ('d', 'f', 'c', 's')
typedef struct {
  UINTN           Signature;
  EFI_LIST_ENTRY  Link;
  CHAR16          *Line;
  CHAR16          Buffer[80];
} DEFAULT_CMD;

#define SHELL_FILE_ARG_SIGNATURE  EFI_SIGNATURE_32 ('g', 'r', 'a', 'f')
typedef struct {
  UINT32                    Signature;
  EFI_LIST_ENTRY            Link;
  EFI_STATUS                Status;

  EFI_FILE_HANDLE           Parent;
  UINT64                    OpenMode;
  CHAR16                    *ParentName;
  EFI_DEVICE_PATH_PROTOCOL  *ParentDevicePath;

  CHAR16                    *FullName;
  CHAR16                    *FileName;

  EFI_FILE_HANDLE           Handle;
  EFI_FILE_INFO             *Info;
} SHELL_FILE_ARG;

//typedef
//VOID
//(EFIAPI *SHELLENV_DUMP_PROTOCOL_INFO) (
//  IN EFI_HANDLE                   Handle,
//  IN VOID                         *Interface
//  );

typedef
EFI_STATUS
(EFIAPI *SHELLENV_INTERNAL_COMMAND) (
  IN EFI_HANDLE                   ImageHandle,
  IN EFI_SYSTEM_TABLE             * SystemTable
  );

typedef
EFI_STATUS
(EFIAPI *SHELLCMD_GET_LINE_HELP) (
  IN OUT CHAR16              **Str
  );

#define COMMAND_SIGNATURE EFI_SIGNATURE_32 ('c', 'm', 'd', 's')

typedef struct {
  UINTN                     Signature;
  EFI_LIST_ENTRY            Link;
  SHELLENV_INTERNAL_COMMAND Dispatch;
  CHAR16                    *Cmd;
  SHELLCMD_GET_LINE_HELP    GetLineHelp;
} COMMAND;

#if defined(__cplusplus)
} /* extern "C" */
#endif

#endif
