/*++

Copyright (c) 2005 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  ShellEnvInt.h

Abstract:

  information shell enviorment interface functions

Revision History

--*/

#ifndef _SHELL_ENVIORMENT_INTERFACE_H
#define _SHELL_ENVIORMENT_INTERFACE_H

#define DEFAULT_INIT_ROW    1
#define DEFAULT_AUTO_LF     FALSE

#define BREAK_LOOP_ON_ESC() { \
    if (GetExecutionBreak ()) { \
      break; \
    } \
  }

BOOLEAN
GetExecutionBreak (
  IN VOID
  );

BOOLEAN
GetPageBreak (
  VOID
  );

VOID
EnablePageBreak (
  IN INT32      StartRow,
  IN BOOLEAN    AutoWrap
  );

VOID
DisablePageBreak (
  VOID
  );

VOID
EnableOutputTabPause (
  VOID
  );

VOID
DisableOutputTabPause (
  VOID
  );

BOOLEAN
GetOutputTabPause (
  VOID
  );

VOID
SetKeyFilter (
  IN UINT32      KeyFilter
  );

UINT32
GetKeyFilter (
  IN VOID
  );

CHAR16                    *
ShellGetEnv (
  IN CHAR16       *Name
  );

EFI_DEVICE_PATH_PROTOCOL  *
ShellGetMap (
  IN CHAR16       *Name
  );

CHAR16                    *
ShellCurDir (
  IN CHAR16               *DeviceName OPTIONAL
  );

BOOLEAN
ShellBatchIsActive (
  VOID
  );

EFI_STATUS
ShellExecute (
  IN EFI_HANDLE   ParentImageHandle,
  IN CHAR16       *CommandLine,
  IN BOOLEAN      Output
  );

EFI_STATUS
ShellFileMetaArg (
  IN CHAR16               *Arg,
  IN OUT EFI_LIST_ENTRY   *ListHead
  );

EFI_STATUS
ShellFileMetaArgNoWildCard (
  IN CHAR16               *Arg,
  IN OUT EFI_LIST_ENTRY   *ListHead
  );

EFI_STATUS
ShellFreeFileList (
  IN OUT EFI_LIST_ENTRY   *ListHead
  );

VOID
ShellInitHandleEnumerator (
  VOID
  );

EFI_STATUS
ShellNextHandle (
  IN OUT   EFI_HANDLE            **Handle
  );

EFI_STATUS
ShellSkipHandle (
  IN UINTN                  SkipNum
  );

VOID
ShellResetHandleEnumerator (
  VOID
  );

VOID
ShellCloseHandleEnumerator (
  VOID
  );

VOID
ShellFreeResources (
  VOID
  );

VOID
ShellInitProtocolInfoEnumerator (
  VOID
  );

EFI_STATUS
ShellNextProtocolInfo (
  IN OUT   PROTOCOL_INFO            **ProtocolInfo
  );

EFI_STATUS
ShellSkipProtocolInfo (
  IN UINTN                          SkipNum
  );

VOID
ShellResetProtocolInfoEnumerator (
  VOID
  );

VOID
ShellCloseProtocolInfoEnumerator (
  VOID
  );
  
VOID
ShellCloseConsoleProxy (
  IN     EFI_HANDLE                       ConInHandle,
  IN OUT EFI_SIMPLE_TEXT_IN_PROTOCOL      **ConIn,
  IN     EFI_HANDLE                       ConOutHandle,
  IN OUT EFI_SIMPLE_TEXT_OUT_PROTOCOL     **ConOut
  );
  
EFI_STATUS
ShellDelDupFileArg (
  IN EFI_LIST_ENTRY   *ListHead
  );
  
UINTN
ShellGetHandleNum (
  VOID
  );

#endif