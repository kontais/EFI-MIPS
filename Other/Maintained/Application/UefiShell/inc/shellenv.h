/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  shellenv.h

Abstract:

  Defines for shell environment



Revision History

--*/

#ifndef _SHELLENV_H_
#define _SHELLENV_H_

#include "shelltypes.h"

#include EFI_PROTOCOL_DEFINITION (LoadedImage)

#if defined(__cplusplus)
extern "C"
{
#endif

#define EFI_SHELL_COMPATIBLE_MODE_VER L"1.1.1"
#define EFI_SHELL_ENHANCED_MODE_VER   L"1.1.2"

//
// Shell Interface - additional information (over image_info) provided
// to an application started by the shell.
//
// ConIo - provides a file style interface to the console.  Note that the
// ConOut & ConIn interfaces in the system table will work as well, and both
// all will be redirected to a file if needed on a command line
//
// The shell interface's and data (including ConIo) are only valid during
// the applications Entry Point.  Once the application returns from it's
// entry point the data is freed by the invoking shell.
//
#define SHELL_INTERFACE_PROTOCOL \
  { \
    0x47c7b223, 0xc42a, 0x11d2, 0x8e, 0x57, 0x0, 0xa0, 0xc9, 0x69, 0x72, 0x3b \
  }

typedef struct _EFI_SHELL_ARG_INFO {
  UINT32  Attributes;
} EFI_SHELL_ARG_INFO;

typedef struct _EFI_SHELL_INTERFACE {
  //
  // Handle back to original image handle & image info
  //
  EFI_HANDLE                ImageHandle;
  EFI_LOADED_IMAGE_PROTOCOL *Info;

  //
  // Parsed arg list
  //
  CHAR16                    **Argv;
  UINTN                     Argc;

  //
  // Storage for file redirection args after parsing
  //
  CHAR16                    **RedirArgv;
  UINTN                     RedirArgc;

  //
  // A file style handle for console io
  //
  EFI_FILE_HANDLE           StdIn;
  EFI_FILE_HANDLE           StdOut;
  EFI_FILE_HANDLE           StdErr;
  EFI_SHELL_ARG_INFO        *ArgInfo;
  BOOLEAN                   EchoOn;
} EFI_SHELL_INTERFACE;

//
// The shell environment is provided by a driver.  The shell links to the
// shell environment for services.  In addition, other drivers may connect
// to the shell environment and add new internal command handlers, or
// internal protocol handlers.
//
#define SHELL_ENVIRONMENT_INTERFACE_PROTOCOL \
  { \
    0x47c7b221, 0xc42a, 0x11d2, 0x8e, 0x57, 0x0, 0xa0, 0xc9, 0x69, 0x72, 0x3b \
  }

typedef
EFI_STATUS
(EFIAPI *SHELLENV_EXECUTE) (
  IN EFI_HANDLE   * ParentImageHandle,
  IN CHAR16       *CommandLine,
  IN BOOLEAN      DebugOutput
  );

typedef CHAR16 * (EFIAPI *SHELLENV_GET_ENV) (IN CHAR16 *Name);

typedef CHAR16 * (EFIAPI *SHELLENV_GET_MAP) (IN CHAR16 *Name);

//
// Add to shell's internal command list
//
typedef
EFI_STATUS
(EFIAPI *SHELLENV_ADD_CMD) (
  IN SHELLENV_INTERNAL_COMMAND    Handler,
  IN CHAR16                       *Cmd,
  IN SHELLCMD_GET_LINE_HELP       GetLineHelp
  );

//
// Add to shell environment protocol information & protocol information dump handlers
//
//typedef
//VOID
//(EFIAPI *SHELLENV_DUMP_PROTOCOL_INFO) (
//  IN EFI_HANDLE                   Handle,
//  IN VOID                         *Interface
//  );

typedef
VOID
(EFIAPI *SHELLENV_ADD_PROT) (
  IN EFI_GUID                                 * Protocol,
  IN SHELLENV_DUMP_PROTOCOL_INFO              DumpToken OPTIONAL,
  IN SHELLENV_DUMP_PROTOCOL_INFO              DumpInfo OPTIONAL,
  IN CHAR16                                   *IdString
  );

typedef CHAR16 * (EFIAPI *SHELLENV_GET_PROT) (IN EFI_GUID *Protocol, IN BOOLEAN GenId);

typedef
VOID
(EFIAPI *SHELLENV_ENABLE_PAGE_BREAK) (
  IN INT32      StartRow,
  IN BOOLEAN    AutoWrap
  );

typedef
VOID
(EFIAPI *SHELLENV_DISABLE_PAGE_BREAK) (
  IN VOID
  );

typedef
BOOLEAN
(EFIAPI *SHELLENV_GET_PAGE_BREAK) (
  IN VOID
  );

typedef
VOID
(EFIAPI *SHELLENV_SET_KEY_FILTER) (
  IN UINT32      KeyFilter
  );

typedef
UINT32
(EFIAPI *SHELLENV_GET_KEY_FILTER) (
  IN VOID
  );

typedef
BOOLEAN
(EFIAPI *SHELLENV_GET_EXECUTION_BREAK) (
  IN VOID
  );

typedef EFI_SHELL_INTERFACE * (EFIAPI *SHELLENV_NEW_SHELL) (IN EFI_HANDLE ImageHandle);

typedef
VOID
(EFIAPI *SHELLENV_INCREMENT_SHELL_NESTING_LEVEL) (
  IN VOID
  );

typedef
VOID
(EFIAPI *SHELLENV_DECREMENT_SHELL_NESTING_LEVEL) (
  IN VOID
  );

typedef
BOOLEAN
(EFIAPI *SHELLENV_IS_ROOT_SHELL) (
  IN VOID
  );

typedef CHAR16 * (EFIAPI *SHELLENV_CUR_DIR) (IN CHAR16 *DeviceName OPTIONAL);

typedef
BOOLEAN
(EFIAPI *SHELLENV_BATCH_IS_ACTIVE) (
  IN VOID
  );

typedef
EFI_STATUS
(EFIAPI *SHELLENV_FILE_META_ARG) (
  IN CHAR16               *Arg,
  IN OUT EFI_LIST_ENTRY   * ListHead
  );

typedef
EFI_STATUS
(EFIAPI *SHELLENV_FILE_META_ARG_NO_WILDCARD) (
  IN CHAR16               *Arg,
  IN OUT EFI_LIST_ENTRY   * ListHead
  );

typedef
EFI_STATUS
(EFIAPI *SHELLENV_DEL_DUP_FILE) (
  IN EFI_LIST_ENTRY   * ListHead
  );

typedef
EFI_STATUS
(EFIAPI *SHELLENV_FREE_FILE_LIST) (
  IN OUT EFI_LIST_ENTRY   * ListHead
  );

typedef
VOID
(EFIAPI *SHELLENV_FREE_RESOURCES) (
  VOID
  );

typedef
VOID
(EFIAPI *SHELLENV_CLOSE_CONSOLE_PROXY) (
  IN     EFI_HANDLE                       ConInHandle,
  IN OUT EFI_SIMPLE_TEXT_IN_PROTOCOL      **ConIn,
  IN     EFI_HANDLE                       ConOutHandle,
  IN OUT EFI_SIMPLE_TEXT_OUT_PROTOCOL     **ConOut
  );

typedef EFI_DEVICE_PATH_PROTOCOL * (EFIAPI *SHELLENV_NAME_TO_PATH) (IN CHAR16 *Path);
//
//
//
typedef struct {
  SHELLENV_EXECUTE          Execute;  // Execute a command line
  SHELLENV_GET_ENV          GetEnv;   // Get an environment variable
  SHELLENV_GET_MAP          GetMap;   // Get mapping tables
  SHELLENV_ADD_CMD          AddCmd;   // Add an internal command handler
  SHELLENV_ADD_PROT         AddProt;  // Add protocol info handler
  SHELLENV_GET_PROT         GetProt;  // Get the protocol ID
  SHELLENV_CUR_DIR          CurDir;
  SHELLENV_FILE_META_ARG    FileMetaArg;
  SHELLENV_FREE_FILE_LIST   FreeFileList;

  //
  // The following services are only used by the shell itself
  //
  SHELLENV_NEW_SHELL        NewShell;
  SHELLENV_BATCH_IS_ACTIVE  BatchIsActive;

  SHELLENV_FREE_RESOURCES   FreeResources;
} EFI_SHELL_ENVIRONMENT;

//
// declarations of handle enumerator
//
typedef
EFI_STATUS
(*NEXT_HANDLE) (
  IN OUT   EFI_HANDLE             **Handle
  );

typedef
EFI_STATUS
(*SKIP_HANDLE) (
  IN UINTN                   SkipNum
  );

typedef
UINTN
(*RESET_HANDLE_ENUMERATOR) (
  IN UINTN                  EnumIndex
  );

typedef
VOID
(*INIT_HANDLE_ENUMERATOR) (
  VOID
  );

typedef
VOID
(*CLOSE_HANDLE_ENUMERATOR) (
  VOID
  );

typedef
UINTN
(*GET_NUM) (
  VOID
  );

typedef struct {
  INIT_HANDLE_ENUMERATOR  Init;
  NEXT_HANDLE             Next;
  SKIP_HANDLE             Skip;
  RESET_HANDLE_ENUMERATOR Reset;
  CLOSE_HANDLE_ENUMERATOR Close;
  GET_NUM                 GetNum;
} HANDLE_ENUMERATOR;

//
// declarations of protocol info enumerator
//
typedef
EFI_STATUS
(*NEXT_PROTOCOL_INFO) (
  IN OUT   PROTOCOL_INFO            **ProtocolInfo
  );

typedef
EFI_STATUS
(*SKIP_PROTOCOL_INFO) (
  IN UINTN                         SkipNum
  );

typedef
VOID
(*RESET_PROTOCOL_INFO_ENUMERATOR) (
  VOID
  );

typedef
VOID
(*INIT_PROTOCOL_INFO_ENUMERATOR) (
  VOID
  );

typedef
VOID
(*CLOSE_PROTOCOL_INFO_ENUMERATOR) (
  VOID
  );

typedef struct {
  INIT_PROTOCOL_INFO_ENUMERATOR   Init;
  NEXT_PROTOCOL_INFO              Next;
  SKIP_PROTOCOL_INFO              Skip;
  RESET_PROTOCOL_INFO_ENUMERATOR  Reset;
  CLOSE_PROTOCOL_INFO_ENUMERATOR  Close;
} PROTOCOL_INFO_ENUMERATOR;

typedef
EFI_STATUS
(*GET_DEVICE_NAME) (
  EFI_HANDLE  DeviceHandle,
  BOOLEAN     UseComponentName,
  BOOLEAN     UseDevicePath,
  CHAR8       *Language,
  CHAR16      **BestDeviceName,
  EFI_STATUS  *ConfigurationStatus,
  EFI_STATUS  *DiagnosticsStatus,
  BOOLEAN     Display,
  UINTN       Indent
  );

typedef
EFI_STATUS
(*GET_SHELL_MODE) (
  OUT CHAR16     **Mode
  );

typedef
EFI_STATUS
(*SHELLENV_GET_FS_NAME) (
  IN EFI_DEVICE_PATH_PROTOCOL     * DevPath,
  IN BOOLEAN                      ConsistMapping,
  OUT CHAR16                      **Name
  );

typedef
EFI_STATUS
(*SHELLENV_GET_FS_DEVICE_PATH) (
  IN CHAR16                        *Name,
  OUT EFI_DEVICE_PATH_PROTOCOL     **DevPath
  );

#define EFI_SE_EXT_SIGNATURE_GUID \
  { \
    0xd2c18636, 0x40e5, 0x4eb5, \
    { \
      0xa3, 0x1b, 0x36, 0x69, 0x5f, 0xd4, 0x2c, 0x87 \
    } \
  }
#define EFI_SHELL_MAJOR_VER 0x00000001
#define EFI_SHELL_MINOR_VER 0x00000000

typedef struct {
  SHELLENV_EXECUTE                        Execute;  // Execute a command line
  SHELLENV_GET_ENV                        GetEnv;   // Get an environment variable
  SHELLENV_GET_MAP                        GetMap;   // Get mapping tables
  SHELLENV_ADD_CMD                        AddCmd;   // Add an internal command handler
  SHELLENV_ADD_PROT                       AddProt;  // Add protocol info handler
  SHELLENV_GET_PROT                       GetProt;  // Get the protocol ID
  SHELLENV_CUR_DIR                        CurDir;
  SHELLENV_FILE_META_ARG                  FileMetaArg;
  SHELLENV_FREE_FILE_LIST                 FreeFileList;

  //
  // The following services are only used by the shell itself
  //
  SHELLENV_NEW_SHELL                      NewShell;
  SHELLENV_BATCH_IS_ACTIVE                BatchIsActive;

  SHELLENV_FREE_RESOURCES                 FreeResources;

  //
  // Major Version grows if shell environment interface has been changes
  //
  EFI_GUID                                SESGuid;
  UINT32                                  MajorVersion;
  UINT32                                  MinorVersion;
  SHELLENV_ENABLE_PAGE_BREAK              EnablePageBreak;
  SHELLENV_DISABLE_PAGE_BREAK             DisablePageBreak;
  SHELLENV_GET_PAGE_BREAK                 GetPageBreak;

  SHELLENV_SET_KEY_FILTER                 SetKeyFilter;
  SHELLENV_GET_KEY_FILTER                 GetKeyFilter;

  SHELLENV_GET_EXECUTION_BREAK            GetExecutionBreak;
  SHELLENV_INCREMENT_SHELL_NESTING_LEVEL  IncrementShellNestingLevel;
  SHELLENV_DECREMENT_SHELL_NESTING_LEVEL  DecrementShellNestingLevel;
  SHELLENV_IS_ROOT_SHELL                  IsRootShell;

  SHELLENV_CLOSE_CONSOLE_PROXY            CloseConsoleProxy;
  HANDLE_ENUMERATOR                       HandleEnumerator;
  PROTOCOL_INFO_ENUMERATOR                ProtocolInfoEnumerator;
  GET_DEVICE_NAME                         GetDeviceName;
  GET_SHELL_MODE                          GetShellMode;
  SHELLENV_NAME_TO_PATH                   NameToPath;
  SHELLENV_GET_FS_NAME                    GetFsName;
  SHELLENV_FILE_META_ARG_NO_WILDCARD      FileMetaArgNoWildCard;
  SHELLENV_DEL_DUP_FILE                   DelDupFileArg;
  SHELLENV_GET_FS_DEVICE_PATH             GetFsDevicePath;
} EFI_SHELL_ENVIRONMENT2;

#ifdef EFI_MONOSHELL
EFI_STATUS
EFIAPI
InitializeShellEnvironment (
  IN  EFI_HANDLE          ImageHandle,
  IN  EFI_SYSTEM_TABLE    *SystemTable
  )
/*++

Routine Description:

  Shell entry point

Arguments:

  ImageHandle - The image handle
  SystemTable - The system table

Returns:

  EFI_SUCCESS - Success

--*/
;
EFI_STATUS
UnInstallShellEnvironment (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );
#endif

#if defined(__cplusplus)
} /* extern "C" */
#endif

#endif // _SHELLENV_H_
