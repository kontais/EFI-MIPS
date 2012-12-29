/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  shelle.h

Abstract:
 
Revision History

--*/

#ifndef _SHELLE_H_
#define _SHELLE_H_

#include "EfiShellLib.h"
#include "shellenv.h"

//
// Internal defines
//
#define COMMON_CMDLINE_LENGTH 32
#define COMMON_ARG_COUNT      32
#define COMMON_ARG_LENGTH     32

//
// Keep these 2 macros because conio.c and map.c(maybe more other files) uses
// them!!
//
#define MAX_CMDLINE     256
#define MAX_ARG_LENGTH  256

//
// The definitions of VARIABLE_ID'Flags
//
#define VAR_ID_NON_VOL            0x01
#define VAR_ID_INVALID            0x02
#define VAR_ID_ACCESSED           0x04
#define VAR_ID_CONSIST            0x08

#define IsWhiteSpace(c)           (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == 0)
#define IsValidChar(c)            (c >= ' ')
#define IsDigit(c)                (c >= '0' && c <= '9')
#define IsAlpha(c)                ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))

#define GOTO_TARGET_FOUND         (1)
#define GOTO_TARGET_NOT_FOUND     (2)
#define GOTO_TARGET_DOESNT_EXIST  (3)

//
//  Protocol guid
//
#define SHELL_GLOBALVAR_PROTOCOL \
  { \
    0x47c7b222, 0xc42a, 0x11d2, 0x8e, 0x57, 0x0, 0xa0, 0xc9, 0x69, 0x72, 0x3b \
  }

//
// Internal structures
//
//
// IDs of different variables stored by the shell environment
//
#define ENVIRONMENT_VARIABLE_ID \
  { \
    0x47c7b224, 0xc42a, 0x11d2, 0x8e, 0x57, 0x0, 0xa0, 0xc9, 0x69, 0x72, 0x3b \
  }

#define DEVICE_PATH_MAPPING_ID \
  { \
    0x47c7b225, 0xc42a, 0x11d2, 0x8e, 0x57, 0x0, 0xa0, 0xc9, 0x69, 0x72, 0x3b \
  }

#define PROTOCOL_ID_ID \
  { \
    0x47c7b226, 0xc42a, 0x11d2, 0x8e, 0x57, 0x0, 0xa0, 0xc9, 0x69, 0x72, 0x3b \
  }

#define ALIAS_ID \
  { \
    0x47c7b227, 0xc42a, 0x11d2, 0x8e, 0x57, 0x0, 0xa0, 0xc9, 0x69, 0x72, 0x3b \
  }

//
//
//
#define ENV_REDIR_SIGNATURE EFI_SIGNATURE_32 ('r', 'i', 'd', 's')
typedef struct {
  UINTN                         Signature;
  BOOLEAN                       Ascii;
  EFI_STATUS                    WriteError;
  EFI_FILE_HANDLE               File;
  EFI_DEVICE_PATH_PROTOCOL      *FilePath;
  EFI_HANDLE                    Handle;
  EFI_SIMPLE_TEXT_OUT_PROTOCOL  Out;
  EFI_SIMPLE_TEXT_IN_PROTOCOL   In;
} ENV_SHELL_REDIR_FILE;

typedef struct {
  EFI_SHELL_INTERFACE   ShellInt;
  EFI_SYSTEM_TABLE      *SystemTable;

  ENV_SHELL_REDIR_FILE  StdIn;
  ENV_SHELL_REDIR_FILE  StdOut;
  ENV_SHELL_REDIR_FILE  StdErr;

} ENV_SHELL_INTERFACE;

typedef struct _ENV_SHELL_INTERFACE_ITEM {
  EFI_SHELL_ENVIRONMENT             *Interface;
  EFI_HANDLE                        Handle;
  struct _ENV_SHELL_INTERFACE_ITEM  *Next;
} ENV_SHELL_INTERFACE_ITEM;

//
// Internal prototypes from init.c
//
EFI_SHELL_INTERFACE       *
EFIAPI
SEnvNewShell (
  IN EFI_HANDLE                   ImageHandle
  );

//
// Internal prototypes from cmddisp.c
//
VOID
SEnvInitCommandTable (
  VOID
  );

EFI_STATUS
EFIAPI
SEnvAddCommand (
  IN SHELLENV_INTERNAL_COMMAND    Handler,
  IN CHAR16                       *CmdStr,
  IN SHELLCMD_GET_LINE_HELP       GetLineHelp
  );

EFI_STATUS
SEnvFreeCommandTable (
  VOID
  );

BOOLEAN
IsValidCommand (
  IN CHAR16                   *Cmd
  );

SHELLENV_INTERNAL_COMMAND
SEnvGetCmdDispath (
  IN CHAR16                   *CmdName
  );

//
// From exec.c
//
VOID
SEnvFreeArgument (
  IN EFI_SHELL_INTERFACE    *ShellInt
  );

VOID
SEnvFreeArgInfoArray (
  IN EFI_SHELL_INTERFACE      *ShellInt
  );

VOID
SEnvFreeRedirArgument (
  IN EFI_SHELL_INTERFACE   *ShellInt
  );

EFI_STATUS
EFIAPI
SEnvExecute (
  IN EFI_HANDLE           *ParentImageHandle,
  IN CHAR16               *CommandLine,
  IN BOOLEAN              DebugOutput
  );

EFI_STATUS
SEnvDoExecute (
  IN EFI_HANDLE           *ParentImageHandle,
  IN CHAR16               *CommandLine,
  IN ENV_SHELL_INTERFACE  *Shell,
  IN BOOLEAN              Output
  );

EFI_STATUS
EFIAPI
SEnvExitGetLineHelp (
  OUT CHAR16                  **Str
  )
/*++

Routine Description:

  Get this command's line help

Arguments:

  Str - The line help

Returns:

  EFI_SUCCESS   - Success

--*/
;

EFI_STATUS
EFIAPI
SEnvExit (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  );

EFI_STATUS
MemLeakTest (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
SEnvStringToArg (
  IN CHAR16                   *Str,
  IN BOOLEAN                  Output,
  OUT EFI_SHELL_INTERFACE     *ShellInt
  );

VOID
EFIAPI
SEnvIncrementShellNestingLevel (
  IN VOID
  );

VOID
EFIAPI
SEnvDecrementShellNestingLevel (
  IN VOID
  );

BOOLEAN
EFIAPI
SEnvIsRootShell (
  IN VOID
  );

VOID
EFIAPI
SEnvEnablePageBreak (
  IN INT32      StartRow,
  IN BOOLEAN    AutoWrap
  );

VOID
EFIAPI
SEnvDisablePageBreak (
  IN VOID
  );

BOOLEAN
EFIAPI
SEnvGetPageBreak (
  IN VOID
  );

VOID
SEnvEnableExecutionBreak (
  IN VOID
  );

VOID
SEnvDisableExecutionBreak (
  IN VOID
  );

BOOLEAN
EFIAPI
SEnvGetExecutionBreak (
  IN VOID
  );

//
// Internal prototypes from protid.c
//
VOID
SEnvInitProtocolInfo (
  VOID
  );

EFI_STATUS
SEnvLoadDefaultsGetLineHelp (
  IN OUT CHAR16                     **Str
  )
/*++

Routine Description:

  Get this command's line help

Arguments:

  Str - The line help

Returns:

  EFI_SUCCESS   - Success

--*/
;

EFI_STATUS
EFIAPI
SEnvLoadDefaults (
  IN EFI_HANDLE           Parent,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

VOID
SEnvLoadInternalProtInfo (
  VOID
  );

VOID
SEnvFreeHandleProtocolInfo (
  VOID
  );

//
// functions declaration for enumerate protoocl info database
//
VOID
SEnvInitProtocolInfoEnumerator (
  VOID
  );

VOID
SEnvCloseProtocolInfoEnumerator (
  VOID
  );

EFI_STATUS
SEnvNextProtocolInfo (
  IN OUT   PROTOCOL_INFO            **ProtocolInfo
  );

EFI_STATUS
SEnvSkipProtocolInfo (
  IN UINTN                  SkipNum
  );

VOID
SEnvResetProtocolInfoEnumerator (
  VOID
  );

VOID
EFIAPI
SEnvAddProtocol (
  IN EFI_GUID                                 *Protocol,
  IN SHELLENV_DUMP_PROTOCOL_INFO              DumpToken OPTIONAL,
  IN SHELLENV_DUMP_PROTOCOL_INFO              DumpInfo OPTIONAL,
  IN CHAR16                                   *IdString
  );

VOID
SEnvIAddProtocol (
  IN BOOLEAN                                  SaveId,
  IN EFI_GUID                                 *Protocol,
  IN SHELLENV_DUMP_PROTOCOL_INFO              DumpToken OPTIONAL,
  IN SHELLENV_DUMP_PROTOCOL_INFO              DumpInfo OPTIONAL,
  IN CHAR16                                   *IdString
  );

VOID
SEnvLoadHandleProtocolInfo (
  IN EFI_GUID                     *Skip
  );

CHAR16                    *
EFIAPI
SEnvGetProtocol (
  IN EFI_GUID                     *ProtocolId,
  IN BOOLEAN                      GenId
  );

PROTOCOL_INFO             *
SEnvGetProtByStr (
  IN CHAR16           *Str
  );

EFI_STATUS
SEnvGetDeviceName (
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

EFI_STATUS
SEnvCmdGetStringByToken (
  IN     UINT16         Token,
  IN OUT CHAR16         **Str
  );

EFI_STATUS
SEnvDHProt (
  IN BOOLEAN                      Verbose,
  IN BOOLEAN                      DriverModel,
  IN UINTN                        HandleNo,
  IN EFI_HANDLE                   Handle,
  IN CHAR8                        *Language
  );

EFI_STATUS
EFIAPI
SEnvHelpGetLineHelp (
  IN CHAR16         **Str
  )
/*++

Routine Description:

  Get this command's line help

Arguments:

  Str - The line help

Returns:

  EFI_SUCCESS   - Success

--*/
;

EFI_STATUS
EFIAPI
SEnvHelp (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
EFIAPI
SEnvCmdDHGetLineHelp (
  OUT CHAR16                   **Str
  )
/*++

Routine Description:

  Get this command's line help

Arguments:

  Str - The line help

Returns:

  EFI_SUCCESS   - Success

--*/
;

EFI_STATUS
EFIAPI
SEnvCmdDH (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  );

EFI_STATUS
SEnvIGetProtID (
  IN CHAR16           *Str,
  OUT EFI_GUID        *ProtId
  );

EFI_STATUS
SEnvCmdUnloadGetLineHelp (
  OUT CHAR16                   **Str
  )
/*++

Routine Description:

  Get this command's line help

Arguments:

  Str - The line help

Returns:

  EFI_SUCCESS   - Success

--*/
;

EFI_STATUS
SEnvCmdUnload (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  );

//
// Handle.c
//
VOID
SEnvInitHandleGlobals (
  VOID
  );

VOID
SEnvLoadHandleTable (
  VOID
  );

VOID
SEnvFreeHandleTable (
  VOID
  );

UINTN
SEnvHandleNoFromStr (
  IN CHAR16       *Str
  );

EFI_HANDLE
SEnvHandleFromStr (
  IN CHAR16       *Str
  );

UINTN
SEnvHandleNoFromUINT (
  IN UINTN       Value
  );

EFI_HANDLE
SEnvHandleFromUINT (
  IN UINTN       Value
  );

UINTN
SEnvHandleToNumber (
  IN  EFI_HANDLE  Handle
  );

//
// functions declaration for enumerate handle database
//
EFI_STATUS
SEnvNextHandle (
  IN OUT   EFI_HANDLE             **Handle
  );

EFI_STATUS
SEnvSkipHandle (
  IN UINTN             NumSkip
  );

UINTN
SEnvResetHandleEnumerator (
  IN UINTN             EnumIndex
  );

VOID
SEnvInitHandleEnumerator (
  IN              VOID
  );

VOID
SEnvCloseHandleEnumerator (
  IN              VOID
  );

UINTN
SEnvGetHandleNum (
  IN               VOID
  );
//
// Internal prototypes from var.c
//
VOID
SEnvInitVariables (
  VOID
  );

EFI_STATUS
EFIAPI
SEnvCmdSetGetLineHelp (
  IN OUT CHAR16                             **Str
  )
/*++

Routine Description:

  Get this command's line help

Arguments:

  Str - The line help

Returns:

  EFI_SUCCESS   - Success

--*/
;

EFI_STATUS
EFIAPI
SEnvCmdSet (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  );

EFI_STATUS
EFIAPI
SEnvCmdAliasGetLineHelp (
  IN OUT CHAR16         **Str
  )
/*++

Routine Description:

  Get this command's line help

Arguments:

  Str - The line help

Returns:

  EFI_SUCCESS   - Success

--*/
;

EFI_STATUS
EFIAPI
SEnvCmdAlias (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  );

CHAR16                    *
EFIAPI
SEnvGetMap (
  IN CHAR16           *Name
  );

CHAR16                    *
EFIAPI
SEnvGetEnv (
  IN CHAR16           *Name
  );

CHAR16                    *
SEnvGetAlias (
  IN CHAR16           *Name
  );

VARIABLE_ID               *
SEnvAddVarToList (
  IN EFI_LIST_ENTRY           *Head,
  IN CHAR16                   *Name,
  IN UINT8                    *Value,
  IN UINTN                    ValueSize,
  BOOLEAN                     Volatile
  );

EFI_STATUS
SEnvAddVar (
  IN EFI_LIST_ENTRY           *Head,
  IN EFI_GUID                 *Guid,
  IN CHAR16                   *Name,
  IN UINT8                    *Value,
  IN UINTN                    ValueSize,
  BOOLEAN                     Volatile
  );

VARIABLE_ID               *
SEnvFindVar (
  IN EFI_LIST_ENTRY           *Head,
  IN CHAR16                   *Name
  );

//
// Prototypes from conio.c
//
VOID
SEnvConIoInitDosKey (
  VOID
  );

EFI_STATUS
EFIAPI
SEnvConIoOpen (
  IN EFI_FILE                   *File,
  OUT EFI_FILE                  **NewHandle,
  IN CHAR16                     *FileName,
  IN UINT64                     OpenMode,
  IN UINT64                     Attributes
  );

EFI_STATUS
EFIAPI
SEnvConIoNop (
  IN EFI_FILE  *File
  );

EFI_STATUS
EFIAPI
SEnvConIoGetPosition (
  IN EFI_FILE                   *File,
  OUT UINT64                    *Position
  );

EFI_STATUS
EFIAPI
SEnvConIoSetPosition (
  IN EFI_FILE                   *File,
  OUT UINT64                    Position
  );

EFI_STATUS
EFIAPI
SEnvConIoGetInfo (
  IN EFI_FILE                   *File,
  IN EFI_GUID                   *InformationType,
  IN OUT UINTN                  *BufferSize,
  OUT VOID                      *Buffer
  );

EFI_STATUS
EFIAPI
SEnvConIoSetInfo (
  IN EFI_FILE                   *File,
  IN EFI_GUID                   *InformationType,
  IN UINTN                      BufferSize,
  OUT VOID                      *Buffer
  );

EFI_STATUS
EFIAPI
SEnvConIoWrite (
  IN EFI_FILE                   *File,
  IN OUT UINTN                  *BufferSize,
  IN VOID                       *Buffer
  );

EFI_STATUS
EFIAPI
SEnvConIoRead (
  IN EFI_FILE                   *File,
  IN OUT UINTN                  *BufferSize,
  IN VOID                       *Buffer
  );

EFI_STATUS
EFIAPI
SEnvErrIoWrite (
  IN EFI_FILE                   *File,
  IN OUT UINTN                  *BufferSize,
  IN VOID                       *Buffer
  );

EFI_STATUS
EFIAPI
SEnvErrIoRead (
  IN EFI_FILE                   *File,
  IN OUT UINTN                  *BufferSize,
  IN VOID                       *Buffer
  );

EFI_STATUS
EFIAPI
SEnvReset (
  IN EFI_SIMPLE_TEXT_OUT_PROTOCOL     *This,
  IN BOOLEAN                          ExtendedVerification
  );

EFI_STATUS
EFIAPI
SEnvOutputString (
  IN EFI_SIMPLE_TEXT_OUT_PROTOCOL *This,
  IN CHAR16                       *String
  );

EFI_STATUS
EFIAPI
SEnvTestString (
  IN EFI_SIMPLE_TEXT_OUT_PROTOCOL *This,
  IN CHAR16                       *String
  );

EFI_STATUS
EFIAPI
SEnvQueryMode (
  IN EFI_SIMPLE_TEXT_OUT_PROTOCOL *This,
  IN UINTN                        ModeNumber,
  OUT UINTN                       *Columns,
  OUT UINTN                       *Rows
  );

EFI_STATUS
EFIAPI
SEnvSetMode (
  IN EFI_SIMPLE_TEXT_OUT_PROTOCOL *This,
  IN UINTN                        ModeNumber
  );

EFI_STATUS
EFIAPI
SEnvSetAttribute (
  IN EFI_SIMPLE_TEXT_OUT_PROTOCOL     *This,
  IN UINTN                            Attribute
  );

EFI_STATUS
EFIAPI
SEnvClearScreen (
  IN EFI_SIMPLE_TEXT_OUT_PROTOCOL     *This
  );

EFI_STATUS
EFIAPI
SEnvSetCursorPosition (
  IN EFI_SIMPLE_TEXT_OUT_PROTOCOL *This,
  IN UINTN                        Column,
  IN UINTN                        Row
  );

EFI_STATUS
EFIAPI
SEnvEnableCursor (
  IN EFI_SIMPLE_TEXT_OUT_PROTOCOL *This,
  IN BOOLEAN                      Enable
  );

EFI_STATUS
EFIAPI
SEnvDummyReset (
  IN EFI_SIMPLE_TEXT_OUT_PROTOCOL     *This,
  IN BOOLEAN                          ExtendedVerification
  );

EFI_STATUS
EFIAPI
SEnvDummyOutputString (
  IN EFI_SIMPLE_TEXT_OUT_PROTOCOL *This,
  IN CHAR16                       *String
  );

EFI_STATUS
EFIAPI
SEnvDummyTestString (
  IN EFI_SIMPLE_TEXT_OUT_PROTOCOL *This,
  IN CHAR16                       *String
  );

EFI_STATUS
EFIAPI
SEnvDummyQueryMode (
  IN EFI_SIMPLE_TEXT_OUT_PROTOCOL *This,
  IN UINTN                        ModeNumber,
  OUT UINTN                       *Columns,
  OUT UINTN                       *Rows
  );

EFI_STATUS
EFIAPI
SEnvDummySetMode (
  IN EFI_SIMPLE_TEXT_OUT_PROTOCOL *This,
  IN UINTN                        ModeNumber
  );

EFI_STATUS
EFIAPI
SEnvDummySetAttribute (
  IN EFI_SIMPLE_TEXT_OUT_PROTOCOL     *This,
  IN UINTN                            Attribute
  );

EFI_STATUS
EFIAPI
SEnvDummyClearScreen (
  IN EFI_SIMPLE_TEXT_OUT_PROTOCOL *This
  );

EFI_STATUS
EFIAPI
SEnvDummySetCursorPosition (
  IN EFI_SIMPLE_TEXT_OUT_PROTOCOL *This,
  IN UINTN                        Column,
  IN UINTN                        Row
  );

EFI_STATUS
EFIAPI
SEnvDummyEnableCursor (
  IN EFI_SIMPLE_TEXT_OUT_PROTOCOL *This,
  IN BOOLEAN                      Enable
  );

//
// Prototypes from ConsoleProxy.c
//
VOID
SEnvOpenConsoleProxy (
  IN     EFI_HANDLE                       ConInHandle,
  IN OUT EFI_SIMPLE_TEXT_IN_PROTOCOL      **ConIn,
  IN     EFI_HANDLE                       ConOutHandle,
  IN OUT EFI_SIMPLE_TEXT_OUT_PROTOCOL     **ConOut
  );

VOID
EFIAPI
SEnvCloseConsoleProxy (
  IN     EFI_HANDLE                       ConInHandle,
  IN OUT EFI_SIMPLE_TEXT_IN_PROTOCOL      **ConIn,
  IN     EFI_HANDLE                       ConOutHandle,
  IN OUT EFI_SIMPLE_TEXT_OUT_PROTOCOL     **ConOut
  );

EFI_STATUS
SEnvCheckConsoleProxy (
  IN     EFI_HANDLE                       ConInHandle,
  IN OUT EFI_SIMPLE_TEXT_IN_PROTOCOL      **ConIn,
  IN     EFI_HANDLE                       ConOutHandle,
  IN OUT EFI_SIMPLE_TEXT_OUT_PROTOCOL     **ConOut
  );

EFI_STATUS
SEnvConOutHistoryEnable (
  IN BOOLEAN      Enable
  );

EFI_STATUS
SEnvConOutHistoryScrollBack (
  IN UINTN      *ScrollRows OPTIONAL
  );

EFI_STATUS
SEnvConOutHistoryScrollForward (
  IN UINTN      *ScrollRows OPTIONAL
  );

EFI_STATUS
SEnvConOutHistoryQuitScroll (
  IN VOID
  );

VOID
SEnvConOutEnablePageBreak (
  IN INT32      StartRow,
  IN BOOLEAN    AutoWrap
  );

VOID
SEnvConOutDisablePageBreak (
  IN VOID
  );

BOOLEAN
SEnvConOutGetPageBreak (
  IN VOID
  );

VOID
EFIAPI
SEnvSetKeyFilter (
  IN UINT32      KeyFilter
  );

UINT32
EFIAPI
SEnvGetKeyFilter (
  IN VOID
  );

//
// Prototypes from dprot.c
//
CHAR8                     *
GetPdbPath (
  VOID *ImageBase
  );
VOID
PrintShortPdbFileName (
  CHAR8  *PdbFileName,
  UINTN  Length
  );

VOID
EFIAPI
SEnvBlkIo (
  EFI_HANDLE,
  VOID *
  );
VOID
SEnvComponentName (
  EFI_HANDLE,
  VOID *
  );
VOID
EFIAPI
SEnvDPath (
  EFI_HANDLE,
  VOID *
  );
VOID
EFIAPI
SEnvDPathTok (
  EFI_HANDLE,
  VOID *
  );
VOID
EFIAPI
SEnvDebugSupport (
  EFI_HANDLE,
  VOID *
  );
VOID
EFIAPI
SEnvImage (
  EFI_HANDLE,
  VOID *
  );
VOID
EFIAPI
SEnvImageTok (
  EFI_HANDLE,
  VOID *
  );
VOID
EFIAPI
SEnvIsaIo (
  EFI_HANDLE,
  VOID *
  );
VOID
EFIAPI
SEnvPciIo (
  EFI_HANDLE,
  VOID *
  );
VOID
EFIAPI
SEnvPciRootBridgeIo (
  EFI_HANDLE,
  VOID *
  );
VOID
EFIAPI
SEnvTextOut (
  EFI_HANDLE,
  VOID *
  );
VOID
EFIAPI
SEnvUsbIo (
  EFI_HANDLE,
  VOID *
  );
VOID
EFIAPI
SEnvBusSpecificDriverOverride (
  EFI_HANDLE,
  VOID *
  );

//
// Prototypes from map.c
//
VOID
SEnvInitMap (
  VOID
  );

EFI_STATUS
EFIAPI
SEnvCmdMapGetLineHelp (
  OUT CHAR16                  **Str
  )
/*++

Routine Description:

  Get this command's line help

Arguments:

  Str - The line help

Returns:

  EFI_SUCCESS   - Success

--*/
;

EFI_STATUS
EFIAPI
SEnvCmdMap (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  );

EFI_STATUS
EFIAPI
SEnvCmdConnectGetLineHelp (
  OUT CHAR16                  **Str
  )
/*++

Routine Description:

  Get this command's line help

Arguments:

  Str - The line help

Returns:

  EFI_SUCCESS   - Success

--*/
;

EFI_STATUS
EFIAPI
SEnvCmdConnect (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  );

EFI_STATUS
EFIAPI
SEnvCmdDisconnectGetLineHelp (
  OUT CHAR16                  **Str
  )
/*++

Routine Description:

  Get this command's line help

Arguments:

  Str - The line help

Returns:

  EFI_SUCCESS   - Success

--*/
;

EFI_STATUS
EFIAPI
SEnvCmdDisconnect (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  );

EFI_STATUS
EFIAPI
SEnvCmdReconnectGetLineHelp (
  OUT CHAR16                  **Str
  )
/*++

Routine Description:

  Get this command's line help

Arguments:

  Str - The line help

Returns:

  EFI_SUCCESS   - Success

--*/
;

EFI_STATUS
EFIAPI
SEnvCmdReconnect (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  );

VARIABLE_ID               *
SEnvMapDeviceFromName (
  IN OUT CHAR16   **pPath
  );

EFI_DEVICE_PATH_PROTOCOL  *
SEnvIFileNameToPath (
  IN CHAR16               *Path
  );

EFI_DEVICE_PATH_PROTOCOL  *
SEnvFileNameToPath (
  IN CHAR16               *Path
  );

EFI_DEVICE_PATH_PROTOCOL  *
EFIAPI
SEnvNameToPath (
  IN CHAR16                   *PathName
  );

EFI_STATUS
SEnvSetCurrentDevice (
  IN CHAR16       *Name
  );

CHAR16                    *
EFIAPI
SEnvGetCurDir (
  IN CHAR16       *DeviceName OPTIONAL
  );

EFI_STATUS
EFIAPI
SEnvCmdCdGetLineHelp (
  OUT CHAR16      **Str
  )
/*++

Routine Description:

  Get this command's line help

Arguments:

  Str - The line help

Returns:

  EFI_SUCCESS   - Success

--*/
;

EFI_STATUS
EFIAPI
SEnvCmdCd (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
SEnvGetFsName (
  IN EFI_DEVICE_PATH_PROTOCOL *DevPath,
  IN BOOLEAN                  ConsistMapping,
  OUT CHAR16                  **Name
  );

EFI_STATUS
SEnvGetFsDevicePath (
  IN CHAR16                    *Name,
  OUT EFI_DEVICE_PATH_PROTOCOL **DevicePath
  );

//
// Prototypes from echo.c
//
EFI_STATUS
EFIAPI
SEnvCmdEchoGetLineHelp (
  OUT CHAR16              **Str
  )
/*++

Routine Description:

  Get this command's line help

Arguments:

  Str - The line help

Returns:

  EFI_SUCCESS   - Success

--*/
;

EFI_STATUS
EFIAPI
SEnvCmdEcho (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

//
// Prototypes from if.c
//
EFI_STATUS
EFIAPI
SEnvCmdIfGetLineHelp (
  OUT CHAR16              **Str
  )
/*++

Routine Description:

  Get this command's line help

Arguments:

  Str - The line help

Returns:

  EFI_SUCCESS   - Success

--*/
;

EFI_STATUS
EFIAPI
SEnvCmdIf (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
EFIAPI
SEnvCmdElseGetLineHelp (
  OUT CHAR16              **Str
  )
/*++

Routine Description:

  Get this command's line help

Arguments:

  Str - The line help

Returns:

  EFI_SUCCESS   - Success

--*/
;

EFI_STATUS
EFIAPI
SEnvCmdElse (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
EFIAPI
SEnvCmdEndifGetLineHelp (
  OUT CHAR16              **Str
  )
/*++

Routine Description:

  Get this command's line help

Arguments:

  Str - The line help

Returns:

  EFI_SUCCESS   - Success

--*/
;

EFI_STATUS
EFIAPI
SEnvCmdEndif (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
EFIAPI
SEnvCmdShiftGetLineHelp (
  OUT CHAR16              **Str
  )
/*++

Routine Description:

  Get this command's line help

Arguments:

  Str - The line help

Returns:

  EFI_SUCCESS   - Success

--*/
;

EFI_STATUS
EFIAPI
SEnvCmdShift (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );
//
// Prototypes from wait.c
//
EFI_STATUS
EFIAPI
SEnvNoUse (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

//
// Prototypes from goto.c
//
EFI_STATUS
EFIAPI
SEnvCmdGotoGetLineHelp (
  OUT CHAR16              **Str
  )
/*++

Routine Description:

  Get this command's line help

Arguments:

  Str - The line help

Returns:

  EFI_SUCCESS   - Success

--*/
;

EFI_STATUS
EFIAPI
SEnvCmdGoto (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
SEnvCheckForGotoTarget (
  IN  CHAR16 *Candidate,
  IN  UINT64 GotoFilePos,
  IN  UINT64 FilePosition,
  OUT UINTN  *GotoTargetStatus
  );

VOID
SEnvPrintLabelNotFound (
  VOID
  );

VOID
SEnvInitTargetLabel (
  VOID
  );

VOID
SEnvFreeTargetLabel (
  VOID
  );

//
// Prototypes from for.c
//
VOID
SEnvInitForLoopInfo (
  VOID
  );

EFI_STATUS
SEnvSubstituteForLoopIndex (
  IN CHAR16  *Str,
  OUT CHAR16 **Val
  );

EFI_STATUS
EFIAPI
SEnvCmdForGetLineHelp (
  OUT CHAR16              **Str
  )
/*++

Routine Description:

  Get this command's line help

Arguments:

  Str - The line help

Returns:

  EFI_SUCCESS   - Success

--*/
;

EFI_STATUS
EFIAPI
SEnvCmdFor (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
EFIAPI
SEnvCmdEndforGetLineHelp (
  OUT CHAR16              **Str
  )
/*++

Routine Description:

  Get this command's line help

Arguments:

  Str - The line help

Returns:

  EFI_SUCCESS   - Success

--*/
;

EFI_STATUS
EFIAPI
SEnvCmdEndfor (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

//
// Prototypes from pause.c
//
EFI_STATUS
EFIAPI
SEnvCmdPauseGetLineHelp (
  OUT CHAR16              **Str
  )
/*++

Routine Description:

  Get this command's line help

Arguments:

  Str - The line help

Returns:

  EFI_SUCCESS   - Success

--*/
;

EFI_STATUS
EFIAPI
SEnvCmdPause (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

//
// Prototypes from marg.c
//
CHAR16                    *
SEnvFileHandleToFileName (
  IN EFI_FILE_HANDLE      Handle
  );

EFI_STATUS
EFIAPI
SEnvDelDupFileArg (
  IN EFI_LIST_ENTRY   *ListHead
  );

VOID
SEnvFreeFileArg (
  IN SHELL_FILE_ARG       *Arg
  );

EFI_STATUS
EFIAPI
SEnvFreeFileList (
  IN OUT EFI_LIST_ENTRY   *ListHead
  );

EFI_STATUS
EFIAPI
SEnvFileMetaArg (
  IN CHAR16               *Arg,
  IN OUT EFI_LIST_ENTRY   *ListHead
  );

EFI_STATUS
SEnvIFileMetaArg (
  IN CHAR16               *Arg,
  IN OUT EFI_LIST_ENTRY   *ListHead
  );

EFI_STATUS
EFIAPI
SEnvIFileMetaArgNoWildCard (
  IN CHAR16               *Arg,
  IN OUT EFI_LIST_ENTRY   *ListHead
  );

EFI_STATUS
CheckValidFileName (
  IN OUT CHAR16               *Path
  );

VOID
EFIStructsPrint (
  IN  VOID                     *Buffer,
  IN  UINTN                    BlockSize,
  IN  UINT64                   BlockAddress,
  IN  EFI_BLOCK_IO_PROTOCOL    *BlkIo
  );

EFI_STATUS
DumpBlockDev (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  )
/*++

Routine Description:

  Dump block device

Arguments:

  ImageHandle - The image handle
  SystemTable - The system table

Returns:

  EFI_SUCCESS - Success

--*/
;

//
// Device path info
//
#define DEVICEPATH_INFO_SIGNATURE EFI_SIGNATURE_32 ('d', 'p', 'i', 's')
typedef struct {
  UINTN                     Signature;
  EFI_LIST_ENTRY            Link;
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath;
  BOOLEAN                   Found;
} DEVICEPATH_INFO;

VOID
SEnvGetCurMappings (
  VOID
  );
VOID
SEnvCheckValidMappings (
  VARIABLE_ID *Var
  );
VOID
SEnvGetDevicePathList (
  EFI_LIST_ENTRY    *ListHead,
  EFI_GUID          *Protocol
  );
BOOLEAN
SEnvCompareDevicePathList (
  EFI_LIST_ENTRY    *ListHead1,
  EFI_LIST_ENTRY    *ListHead2
  );

//
// Global data
//
extern EFI_GUID                     SEnvEnvId;
extern EFI_GUID                     SEnvMapId;
extern EFI_GUID                     SEnvProtId;
extern EFI_GUID                     SEnvAliasId;
extern EFI_SHELL_ENVIRONMENT        SEnvInterface;
extern EFI_SHELL_ENVIRONMENT2       SEnvInterface2;

extern EFI_FILE                     SEnvIOFromCon;
extern EFI_FILE                     SEnvErrIOFromCon;
extern FLOCK                        SEnvLock;
extern FLOCK                        SEnvGuidLock;
extern UINTN                        SEnvNoHandles;
extern EFI_HANDLE                   *SEnvHandles;
extern EFI_SIMPLE_TEXT_OUT_PROTOCOL SEnvConToIo;
extern EFI_SIMPLE_TEXT_OUT_PROTOCOL SEnvDummyConToIo;

extern EFI_LIST_ENTRY               SEnvProtocolInfo;
extern EFI_LIST_ENTRY               SEnvEnv;
extern EFI_LIST_ENTRY               SEnvMap;
extern EFI_LIST_ENTRY               SEnvCmds;
extern EFI_LIST_ENTRY               SEnvCurMapping;
extern UINTN                        SEnvNoHandles;
extern EFI_HANDLE                   *SEnvHandles;
extern EFI_LIST_ENTRY               SEnvAlias;
//
// Data structures and Prototypes from batch.c
//
//
// Goto target searching status
//
#define GOTO_TARGET_FOUND         (1)
#define GOTO_TARGET_NOT_FOUND     (2)
#define GOTO_TARGET_DOESNT_EXIST  (3)

//
// Data structure for batch script processing
//
//
//  Definitions for the statement stack
//
//  To allow checking if/endif or for/endif matching, and to prevent from
//  jumpting into another loop using goto, for each script, a statement
//  stack is maintained. It contain the nesting information of the if/for
//  statements. For the if statement, the true or false of the condition
//  is recorded; for the for statement, the variable name and a list of
//  variable values are recorded.
//
#define EFI_BATCH_VAR_SIGNATURE EFI_SIGNATURE_32 ('b', 'v', 'a', 'r')
typedef struct {
  UINTN           Signature;
  EFI_LIST_ENTRY  Link;

  CHAR16          *Value;
} EFI_BATCH_VAR_VALUE;

typedef enum {
  StmtFor,
  StmtIf,
  StmtUndefined
} EFI_BATCH_STMT_TYPE;

#define EFI_BATCH_STMT_SIGNATURE  EFI_SIGNATURE_32 ('b', 's', 'm', 't')
typedef struct {
  CHAR16          VariableName[1];
  EFI_LIST_ENTRY  ValueList;
  UINTN           BeginLineNum;
} EFI_BATCH_FOR_INFO;

typedef struct {
  BOOLEAN Condition;
  BOOLEAN FoundElse;
} EFI_BATCH_IF_INFO;

typedef union {
  EFI_BATCH_FOR_INFO  ForInfo;
  EFI_BATCH_IF_INFO   IfInfo;
} EFI_BATCH_STMT_INFO;

typedef struct {
  UINTN               Signature;
  EFI_LIST_ENTRY      Link;

  EFI_BATCH_STMT_TYPE StmtType;
  UINT64              BeginFilePos;
  EFI_BATCH_STMT_INFO StmtInfo;
} EFI_BATCH_STATEMENT;

typedef struct {
  UINTN           NestLevel;
  EFI_LIST_ENTRY  StmtList;

} EFI_BATCH_STMT_STACK;

//
//  Definitions for the script stack
//
//  In order to support nested scripts (script calling script calling
//  script...). There is an script stack "ScriptStack". It is a list of
//  scripts, each script contain a argument list, a statement stack, and
//  the current file position. The argument list contains Argv[0] - Argv[n]
//  of the script, this allows positional argument substitution to be done
//  when each line is read and scanned.
//
#define EFI_BATCH_ARG_SIGNATURE EFI_SIGNATURE_32 ('b', 'a', 'r', 'g')
typedef struct {
  UINTN               Signature;
  EFI_LIST_ENTRY      Link;

  CHAR16              *ArgValue;
  EFI_SHELL_ARG_INFO  *ArgInfo;
} EFI_BATCH_ARGUMENT;

#define EFI_BATCH_SCRIPT_SIGNATURE  EFI_SIGNATURE_32 ('b', 's', 'p', 't')
typedef struct {
  UINTN                 Signature;
  EFI_LIST_ENTRY        Link;

  EFI_LIST_ENTRY        ArgListHead;  // Head of argument list
  EFI_BATCH_STMT_STACK  StmtStack;
  UINT64                FilePosition; // Current file position
  BOOLEAN               BatchAbort;
  UINTN                 LineNumber;
  UINTN                 ShiftIndex;
} EFI_BATCH_SCRIPT;

typedef struct {
  UINTN           NestLevel;
  EFI_LIST_ENTRY  ScriptList;
} EFI_BATCH_SCRIPT_STACK;

#define EFI_SHELL_BATCH_MODE_SIGNATURE  EFI_SIGNATURE_32 ('s', 'b', 'm', 's')
typedef struct _EFI_SHELL_BATCH_MODE_STACK {
  UINTN           Signature;
  CHAR16          ModeName[64];
  EFI_LIST_ENTRY  Link;
} EFI_SHELL_BATCH_MODE_STACK;
//
// Prototype declaration
//
VOID
SEnvInitBatch (
  VOID
  );

EFI_STATUS
SEnvBatchSetFilePosition (
  IN UINT64 NewPos
  );

EFI_STATUS
SEnvBatchFindVariable (
  IN  CHAR16                   *VariableName,
  OUT CHAR16                   **Value
  );

EFI_BATCH_STATEMENT *
SEnvBatchExtraStackTop (
  VOID
  );

EFI_BATCH_STATEMENT *
SEnvBatchStmtStackTop (
  VOID
  );

/*
BOOLEAN
SEnvBatchIsStmtStackEmpty (
  VOID
  );
*/
EFI_BATCH_STATEMENT *
SEnvGetJumpStmt (
  VOID
  );

EFI_STATUS
SEnvBatchFindArgument (
  IN  UINTN                    ArgumentNum,
  OUT CHAR16                   **Value,
  OUT EFI_SHELL_ARG_INFO       **ArgInfo
  );

EFI_STATUS
SEnvFindBatchFileName (
  OUT CHAR16                   **Value
  );

EFI_STATUS
SEnvBatchShiftArgument (
  VOID
  );

VOID
SEnvBatchSetGotoActive (
  VOID
  );

BOOLEAN
SEnvBatchGetGotoActive (
  VOID
  );

UINTN
SEnvGetLineNumber (
  VOID
  );

VOID
SEnvSetLineNumber (
  UINTN                        LineNumber
  );

BOOLEAN
SEnvBatchGetRewind (
  VOID
  );

EFI_STATUS
SEnvTryMoveUpJumpStmt (
  IN  EFI_BATCH_STMT_TYPE      StmtType,
  OUT BOOLEAN                  *Success
  );

EFI_STATUS
SEnvMoveDownJumpStmt (
  IN  EFI_BATCH_STMT_TYPE      StmtType
  ) ;

EFI_STATUS
SEnvBatchResetJumpStmt (
  VOID
  );

BOOLEAN
SEnvBatchExtraStackEmpty (
  VOID
  );

BOOLEAN
EFIAPI
SEnvBatchIsActive (
  VOID
  );

VOID
EFIAPI
SEnvFreeResources (
  VOID
  );

VOID
SEnvSetBatchAbort (
  VOID
  );

CHAR16              *
SEnvBatchGetLastError (
  VOID
  );

EFI_STATUS
SEnvBatchEchoCommand (
  IN  ENV_SHELL_INTERFACE      *Shell
  );

BOOLEAN
SEnvBatchGetEcho (
  VOID
  );

VOID
SEnvBatchSetEcho (
  IN  BOOLEAN                  Val
  );

EFI_STATUS
SEnvExecuteScript (
  IN  ENV_SHELL_INTERFACE      *Shell,
  IN  EFI_FILE_HANDLE          File
  );

BOOLEAN
SEnvBatchVarIsLastError (
  IN  CHAR16                   *Name
  );

VOID
SEnvBatchGetConsole (
  OUT EFI_SIMPLE_TEXT_IN_PROTOCOL  **ConIn,
  OUT EFI_SIMPLE_TEXT_OUT_PROTOCOL **ConOut
  );

EFI_STATUS
SEnvBatchPushStmtStack (
  IN  EFI_BATCH_STMT_TYPE      StmtType,
  IN  BOOLEAN                  PushExtraStack
  );

EFI_STATUS
SEnvBatchPushFor2Stack (
  IN EFI_BATCH_STMT_TYPE                    StmtType,
  IN BOOLEAN                                PushExtraStack
  );

EFI_STATUS
SEnvBatchPopStmtStack (
  IN  UINTN                    PopCount,
  IN  BOOLEAN                  PushExtraStack
  );

EFI_STATUS
SEnvBatchSetCondition (
  IN  BOOLEAN                  Val
  );

BOOLEAN
SEnvBatchGetCondition (
  VOID
  );

//
//  Shell mode switch support
//
EFI_STATUS
SEnvGetShellMode (
  OUT CHAR16       **Mode
  );

VOID
EFIAPI
SEnvGraphicsOutput (
  IN EFI_HANDLE      h,
  IN VOID            *Interface
  );

#endif // _SHELLE_H_
