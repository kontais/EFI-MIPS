/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  data.c
  
Abstract:

  Shell Environment driver global data

Revision History

--*/

#include "shelle.h"

//
// IDs of different variables stored by the shell environment
//
EFI_GUID                      SEnvEnvId   = ENVIRONMENT_VARIABLE_ID;
EFI_GUID                      SEnvMapId   = DEVICE_PATH_MAPPING_ID;
EFI_GUID                      SEnvProtId  = PROTOCOL_ID_ID;
EFI_GUID                      SEnvAliasId = ALIAS_ID;

EFI_SHELL_ENVIRONMENT         SEnvInterface = {
  SEnvExecute,
  SEnvGetEnv,
  SEnvGetMap,
  SEnvAddCommand,
  SEnvAddProtocol,
  SEnvGetProtocol,
  SEnvGetCurDir,
  SEnvFileMetaArg,
  SEnvFreeFileList,

  SEnvNewShell,
  SEnvBatchIsActive,
  SEnvFreeResources
};

EFI_SHELL_ENVIRONMENT2        SEnvInterface2 = {
  SEnvExecute,
  SEnvGetEnv,
  SEnvGetMap,
  SEnvAddCommand,
  SEnvAddProtocol,
  SEnvGetProtocol,
  SEnvGetCurDir,
  SEnvFileMetaArg,
  SEnvFreeFileList,

  SEnvNewShell,
  SEnvBatchIsActive,
  SEnvFreeResources,

  EFI_SE_EXT_SIGNATURE_GUID,
  EFI_SHELL_MAJOR_VER,
  EFI_SHELL_MINOR_VER,
  SEnvEnablePageBreak,
  SEnvDisablePageBreak,
  SEnvGetPageBreak,

  SEnvSetKeyFilter,
  SEnvGetKeyFilter,

  SEnvGetExecutionBreak,

  SEnvIncrementShellNestingLevel,
  SEnvDecrementShellNestingLevel,
  SEnvIsRootShell,

  SEnvCloseConsoleProxy,
  {
    SEnvInitHandleEnumerator,
    SEnvNextHandle,
    SEnvSkipHandle,
    SEnvResetHandleEnumerator,
    SEnvCloseHandleEnumerator,
    SEnvGetHandleNum
  },
  {
    SEnvInitProtocolInfoEnumerator,
    SEnvNextProtocolInfo,
    SEnvSkipProtocolInfo,
    SEnvResetProtocolInfoEnumerator,
    SEnvCloseProtocolInfoEnumerator
  },

  SEnvGetDeviceName,
  SEnvGetShellMode,
  SEnvNameToPath,
  SEnvGetFsName,
  SEnvIFileMetaArgNoWildCard,
  SEnvDelDupFileArg,
  SEnvGetFsDevicePath
};

//
// SEnvIoFromCon - used to access the console interface as a file handle
//
EFI_FILE                      SEnvIOFromCon = {
  EFI_FILE_HANDLE_REVISION,
  SEnvConIoOpen,
  SEnvConIoNop,
  SEnvConIoNop,
  SEnvConIoRead,
  SEnvConIoWrite,
  SEnvConIoGetPosition,
  SEnvConIoSetPosition,
  SEnvConIoGetInfo,
  SEnvConIoSetInfo,
  SEnvConIoNop
};

EFI_FILE                      SEnvErrIOFromCon = {
  EFI_FILE_HANDLE_REVISION,
  SEnvConIoOpen,
  SEnvConIoNop,
  SEnvConIoNop,
  SEnvErrIoRead,
  SEnvErrIoWrite,
  SEnvConIoGetPosition,
  SEnvConIoSetPosition,
  SEnvConIoGetInfo,
  SEnvConIoSetInfo,
  SEnvConIoNop
};

//
// SEnvConToIo - used to access the console interface as a file handle
//
EFI_SIMPLE_TEXT_OUTPUT_MODE   SEnvConToIoMode = {
  0,
  0,
  EFI_TEXT_ATTR(EFI_LIGHTGRAY, EFI_BLACK),
  0,
  0,
  TRUE
};

EFI_SIMPLE_TEXT_OUTPUT_MODE   SEnvDummyConToIoMode = {
  0,
  0,
  EFI_TEXT_ATTR(EFI_LIGHTGRAY, EFI_BLACK),
  0,
  0,
  TRUE
};

EFI_SIMPLE_TEXT_OUT_PROTOCOL  SEnvConToIo = {
  SEnvReset,
  SEnvOutputString,
  SEnvTestString,
  SEnvQueryMode,
  SEnvSetMode,
  SEnvSetAttribute,
  SEnvClearScreen,
  SEnvSetCursorPosition,
  SEnvEnableCursor,
  &SEnvConToIoMode
};

EFI_SIMPLE_TEXT_OUT_PROTOCOL  SEnvDummyConToIo = {
  SEnvDummyReset,
  SEnvDummyOutputString,
  SEnvDummyTestString,
  SEnvDummyQueryMode,
  SEnvDummySetMode,
  SEnvDummySetAttribute,
  SEnvDummyClearScreen,
  SEnvDummySetCursorPosition,
  SEnvDummyEnableCursor,
  &SEnvDummyConToIoMode
};

//
// SEnvLock - guards all shell data except the guid database
//
FLOCK                         SEnvLock;

//
// SEnvGuidLock - guards the guid data
//
FLOCK                         SEnvGuidLock;

//
// Hii handle for the strings used in shell environment
//
EFI_HII_HANDLE                HiiEnvHandle;

//
// SEnvCmds - a list of all internal commands
//
EFI_LIST_ENTRY                SEnvCmds;

