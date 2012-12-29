/*++

Copyright (c) 2005 - 2007, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  err.c
  
Abstract:

  Shell command "err".

Revision History

--*/

#include "EfiShellLib.h"
#include "err.h"
#include EFI_PROTOCOL_DEFINITION (DebugMask)

extern UINT8  STRING_ARRAY_NAME[];
extern UINTN  EFIDebug;
//
// This is the generated header file which includes whatever needs to be exported (strings + IFR)
//
#include STRING_DEFINES_FILE

//
// Global Variables
//
EFI_HII_HANDLE  HiiHandle;
EFI_GUID        EfiErrGuid = EFI_ERR_GUID;
SHELL_VAR_CHECK_ITEM    ErrCheckList[] = {
  {
    L"-dump",
    0x01,
    0x04,
    FlagTypeSingle
  },
  {
    L"-b",
    0x02,
    0,
    FlagTypeSingle
  },
  {
    L"-?",
    0x04,
    0x01,
    FlagTypeSingle
  },
  {
    NULL,
    0,
    0,
    0
  }
};

EFI_STATUS
InitializeError (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  );

UINTN
PrintErrMsg (
  IN UINTN EFIDebug
  );

EFI_STATUS
_SetAllDriverMask (
  IN     UINTN          Msk
  );

VOID
_DumpGlobalMask (
  VOID
  )
{
  VOID  *Variable; 
  Variable = LibGetVariable (L"EFIDebug", &gEfiGenericVariableGuid); 
  if (NULL == Variable) { 
    EFIDebug = EFI_D_ERROR; 
  } else { 
    EFIDebug = *(UINTN *) Variable; 
  } 
  PrintErrMsg (EFIDebug);
}

EFI_STATUS
_SetGlobalMask (
  IN     CHAR16         *Mask
  )
{
  UINTN       DebugLevel;
  EFI_STATUS  Status;
  CHAR16      In;
  UINTN       BufSize;

  DebugLevel = (UINTN) StrToUIntegerBase (Mask, 16, &Status);
  if (EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_ARG), HiiHandle, L"err", Mask);
    return Status;
  }

  for (;;) {
    PrintToken (STRING_TOKEN (STR_ERR_WRITE_TO_NV), HiiHandle);
    Input (L"", &In, sizeof (CHAR16));
    Print (L"\n");

    if (In == 'Y' || In == 'y') {
      //
      // Tell the core to use this value on the next boot
      //
      BufSize = sizeof (EFIDebug);
      Status = RT->SetVariable (
                    L"EFIDebug",
                    &gEfiGenericVariableGuid,
                    (EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS),
                    BufSize,
                    &DebugLevel
                    );
      EFIDebug = DebugLevel;
      _SetAllDriverMask(DebugLevel);
      break;
    } else if (In == 'N' || In == 'n') {
      break;
    }
  }

  return Status;
}

EFI_STATUS
_DumpHandleMask (
  IN     EFI_HANDLE         Handle,
  IN     UINTN              DrvHandle
  )
{
  EFI_STATUS              Status;
  EFI_DEBUG_MASK_PROTOCOL *dmp;
  UINTN                   Mask;

  Status = BS->HandleProtocol (Handle, &gEfiDebugMaskProtocolGuid, &dmp);
  if (!EFI_ERROR (Status)) {
    Status = dmp->GetDebugMask (dmp, &Mask);
    if (!EFI_ERROR (Status)) {
      PrintToken (STRING_TOKEN (STR_ERR_DUMP_DRV_DEBUG_MASK), HiiHandle, DrvHandle, Mask);
    }
  }

  return Status;
}

EFI_STATUS
_DumpDriverMask (
  IN     CHAR16             *Handle
  )
{
  EFI_STATUS  Status;
  EFI_HANDLE  *HandleBuf;
  UINTN       HandleNum;
  UINTN       Index;
  UINTN       DrvHandle;
  BOOLEAN     Found;

  HandleBuf = NULL;
  HandleNum = 0;
  DrvHandle = 0;
  Status    = EFI_SUCCESS;
  Found     = FALSE;

  if (NULL != Handle) {
    DrvHandle = (UINTN) StrToUIntegerBase (Handle, 16, &Status) - 1;
    if (EFI_ERROR (Status)) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_ARG), HiiHandle, L"err", Handle);
      goto Done;
    }
  }

  ShellInitHandleEnumerator ();

  Status = LibLocateHandle (
            ByProtocol,
            &gEfiDebugMaskProtocolGuid,
            NULL,
            &HandleNum,
            &HandleBuf
            );
  if (EFI_ERROR (Status)) {
    goto Done;
  }

  if (0 == HandleNum) {
    PrintToken (STRING_TOKEN (STR_ERR_NO_HANDLE), HiiHandle);
    Status = EFI_NOT_FOUND;
    goto Done;
  }

  PrintToken (STRING_TOKEN (STR_ERR_TITLE), HiiHandle);
  for (Index = 0; Index < HandleNum; Index++) {
    BREAK_LOOP_ON_ESC ();
    if (NULL != Handle) {
      if (HandleBuf[Index] == ShellHandleFromIndex (DrvHandle)) {
        Status = _DumpHandleMask (HandleBuf[Index], DrvHandle + 1);
        if (EFI_ERROR (Status)) {
          PrintToken (STRING_TOKEN (STR_ERR_CAN_NOT_DUMP_HANDLE), HiiHandle, DrvHandle + 1);
          goto Done;
        }
        break;
      }

      continue;
    }

    Status = _DumpHandleMask (HandleBuf[Index], ShellHandleToIndex (HandleBuf[Index]));
    if (EFI_ERROR (Status)) {
      PrintToken (STRING_TOKEN (STR_ERR_CAN_NOT_DUMP_HANDLE), HiiHandle, DrvHandle + 1);
    }
  }

  if (NULL != Handle && Index == HandleNum) {
    PrintToken (STRING_TOKEN (STR_ERR_HANDLE_NOT_FOUND), HiiHandle, DrvHandle + 1);
    Status = EFI_NOT_FOUND;
  }

Done:
  if (NULL != HandleBuf) {
    FreePool (HandleBuf);
  }
  ShellCloseHandleEnumerator ();

  return Status;
}

EFI_STATUS
_SetDriverMask (
  IN     CHAR16         *Handle,
  IN     CHAR16         *Mask
  )
{
  EFI_STATUS              Status;
  EFI_HANDLE              RealHandle;
  UINTN                   DrvHandle;
  UINTN                   Msk;
  EFI_DEBUG_MASK_PROTOCOL *dmp;

  DrvHandle = 0;
  Status    = EFI_SUCCESS;
  Msk       = 0;
  dmp       = NULL;

  DrvHandle = (UINTN) StrToUIntegerBase (Handle, 16, &Status) - 1;
  if (EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_ARG), HiiHandle, L"err", Handle);
    goto Done;
  }

  Msk = (UINTN) StrToUIntegerBase (Mask, 16, &Status);
  if (EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_ARG), HiiHandle, L"err", Mask);
    goto Done;
  }

  ShellInitHandleEnumerator ();
  RealHandle = ShellHandleFromIndex (DrvHandle);
  if (NULL == RealHandle) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_HANDLE_OUT_OF_RANGE), HiiHandle, L"err", Handle);
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  Status = BS->HandleProtocol (RealHandle, &gEfiDebugMaskProtocolGuid, &dmp);
  if (EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_ERR_HANDLE_NOT_FOUND), HiiHandle, DrvHandle + 1);
    goto Done;
  }

  Status = dmp->SetDebugMask (dmp, Msk);
  if (EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_ERR_SET_ERROR), HiiHandle, DrvHandle + 1, Msk);
  }

Done:
  ShellCloseHandleEnumerator ();
  return Status;
}

EFI_STATUS
_SetAllDriverMask (
  IN     UINTN         Msk
  )
{
  EFI_STATUS              Status;
  EFI_HANDLE              *Buffer;
  UINTN                   BufferSize;
  UINTN                   Index;
  EFI_DEBUG_MASK_PROTOCOL *dmp;
  
  dmp       = NULL;
  Buffer    = NULL;
  BufferSize = 0;

  Status = BS->LocateHandle (
                 ByProtocol,
                 &gEfiDebugMaskProtocolGuid,
                 NULL,
                 &BufferSize,
                 Buffer
                 );
  if (Status != EFI_BUFFER_TOO_SMALL) {
    PrintToken (STRING_TOKEN (STR_ERR_NO_HANDLE), HiiHandle);
    goto Done;
  }
  
  Buffer = AllocatePool (BufferSize);
  
  Status = BS->LocateHandle (
                 ByProtocol,
                 &gEfiDebugMaskProtocolGuid,
                 NULL,
                 &BufferSize,
                 Buffer
                 );
  if (EFI_ERROR(Status)) {
    PrintToken (STRING_TOKEN (STR_ERR_NO_HANDLE), HiiHandle);
    goto Done;
  }
  
  for (Index = 0; Index < BufferSize / sizeof(EFI_HANDLE); Index ++) {
    Status = BS->HandleProtocol (Buffer[Index], &gEfiDebugMaskProtocolGuid, &dmp);
    Status = dmp->SetDebugMask (dmp, Msk);
  }

Done: 
  if (Buffer != NULL) {
    FreePool (Buffer);
  }
  return Status;
}

EFI_BOOTSHELL_CODE(
  EFI_APPLICATION_ENTRY_POINT(InitializeError)
)

EFI_STATUS
InitializeError (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
/*++
Routine Description:
  Displays the contents of a file on the standard output device.

Arguments:
  ImageHandle     The image handle. 
  SystemTable     The system table.

Returns:
  EFI_SUCCESS             - Command completed successfully
  EFI_INVALID_PARAMETER   - Command usage error
  EFI_NOT_FOUND           - Files not found
--*/
{
  EFI_STATUS              Status;
  SHELL_VAR_CHECK_CODE    RetCode;
  CHAR16                  *Useful;
  SHELL_VAR_CHECK_PACKAGE ChkPck;
  BOOLEAN                 PageBreak;
  SHELL_ARG_LIST          *Item;

  PageBreak = FALSE;
  ZeroMem (&ChkPck, sizeof (SHELL_VAR_CHECK_PACKAGE));

  //
  //  We are no being installed as an internal command driver, initialize
  //  as an nshell app and run
  //
  EFI_SHELL_APP_INIT (ImageHandle, SystemTable);
  
  //
  // Enable tab key which can pause the output
  //
  EnableOutputTabPause();
  
  //
  // Register our string package with HII and return the handle to it.
  // If previously registered we will simply receive the handle
  //
  Status = LibInitializeStrings (&HiiHandle, STRING_ARRAY_NAME, &EfiErrGuid);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (!EFI_PROPER_VERSION (0, 99)) {
    PrintToken (
      STRING_TOKEN (STR_SHELLENV_GNC_COMMAND_NOT_SUPPORT),
      HiiHandle,
      L"err",
      EFI_VERSION_0_99 
      );
    Status = EFI_UNSUPPORTED;
    goto Done;
  }

  LibFilterNullArgs ();

  Status  = EFI_SUCCESS;
  RetCode = LibCheckVariables (SI, ErrCheckList, &ChkPck, &Useful);
  if (VarCheckOk != RetCode) {
    switch (RetCode) {
    case VarCheckUnknown:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_UNKNOWN_FLAG), HiiHandle, L"err", Useful);
      break;

    case VarCheckDuplicate:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_DUP_FLAG), HiiHandle, L"err", Useful);
      break;

    case VarCheckConflict:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_FLAG_CONFLICT), HiiHandle, L"err", Useful);
      break;

    default:
      break;
    }

    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  if (LibCheckVarGetFlag (&ChkPck, L"-b")) {
    EnablePageBreak (DEFAULT_INIT_ROW, DEFAULT_AUTO_LF);
    PageBreak = TRUE;
  }

  if (LibCheckVarGetFlag (&ChkPck, L"-?")) {
    PrintToken (STRING_TOKEN (STR_ERR_VERBOSEHELP), HiiHandle);
    goto Done;
  }

  Item = LibCheckVarGetFlag (&ChkPck, L"-dump");

  if (0 == ChkPck.ValueCount && (0 == ChkPck.FlagCount || (PageBreak && 1 == ChkPck.FlagCount))) {
    //
    // err
    //
    _DumpGlobalMask ();
  } else if (1 == ChkPck.ValueCount && (0 == ChkPck.FlagCount || (PageBreak && 1 == ChkPck.FlagCount))) {
    //
    // err mask
    //
    Status = _SetGlobalMask (ChkPck.VarList->VarStr);
  } else if (Item && ChkPck.ValueCount < 2) {
    if (0 == ChkPck.ValueCount) {
      //
      // err -dump
      //
      Status = _DumpDriverMask (NULL);
    } else {
      if (ChkPck.VarList->Index != Item->Index + 1) {
        PrintToken (STRING_TOKEN (STR_ERR_SYNTAX_ERROR_HELP), HiiHandle);
        Status = EFI_INVALID_PARAMETER;
        goto Done;
      }
      //
      // err -dump drvhandle
      //
      Status = _DumpDriverMask (ChkPck.VarList->VarStr);
    }
  } else if (2 == ChkPck.ValueCount) {
    if (ChkPck.FlagCount > 1 || (1 == ChkPck.FlagCount && !PageBreak)) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiHandle, L"err");
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }
    //
    // err drvhandle mask
    //
    Status = _SetDriverMask (ChkPck.VarList->VarStr, ChkPck.VarList->Next->VarStr);
  } else {
    PrintToken (STRING_TOKEN (STR_ERR_SYNTAX_ERROR_HELP), HiiHandle);
    Status = EFI_INVALID_PARAMETER;
  }

Done:
  LibUnInitializeStrings ();
  LibCheckVarFreeVarList (&ChkPck);
  return Status;
}

UINTN
PrintErrMsg (
  IN UINTN EFIDebug
  )
/*++

Routine Description:
  
Arguments:

  EFIDebug - Debug mask

Returns:
  
--*/
{
  //
  // if a debug level is not set,
  //    print it normally
  // if a debug level is now set,
  //    print it highlight
  //
  PrintToken (STRING_TOKEN (STR_ERR_EFI_ERROR), HiiHandle, EFIDebug);

  if (EFIDebug & EFI_D_INIT) {
    PrintToken (STRING_TOKEN (STR_ERR_EFI_D_INIT), HiiHandle, EFI_D_INIT);
  } else {
    PrintToken (STRING_TOKEN (STR_ERR_EFI_D_INIT_2), HiiHandle, EFI_D_INIT);
  }

  if (EFIDebug & EFI_D_WARN) {
    PrintToken (STRING_TOKEN (STR_ERR_EFI_D_WARN), HiiHandle, EFI_D_WARN);
  } else {
    PrintToken (STRING_TOKEN (STR_ERR_EFI_D_WARN_2), HiiHandle, EFI_D_WARN);
  }

  if (EFIDebug & EFI_D_LOAD) {
    PrintToken (STRING_TOKEN (STR_ERR_EFI_D_LOAD), HiiHandle, EFI_D_LOAD);
  } else {
    PrintToken (STRING_TOKEN (STR_ERR_EFI_D_LOAD_2), HiiHandle, EFI_D_LOAD);
  }

  if (EFIDebug & EFI_D_FS) {
    PrintToken (STRING_TOKEN (STR_ERR_EFI_D_FS), HiiHandle, EFI_D_FS);
  } else {
    PrintToken (STRING_TOKEN (STR_ERR_EFI_D_FS_2), HiiHandle, EFI_D_FS);
  }

  if (EFIDebug & EFI_D_POOL) {
    PrintToken (STRING_TOKEN (STR_ERR_EFI_D_POOL), HiiHandle, EFI_D_POOL);
  } else {
    PrintToken (STRING_TOKEN (STR_ERR_EFI_D_POOL_2), HiiHandle, EFI_D_POOL);
  }

  if (EFIDebug & EFI_D_PAGE) {
    PrintToken (STRING_TOKEN (STR_ERR_EFI_D_PAGE), HiiHandle, EFI_D_PAGE);
  } else {
    PrintToken (STRING_TOKEN (STR_ERR_EFI_D_PAGE_2), HiiHandle, EFI_D_PAGE);
  }

  if (EFIDebug & EFI_D_INFO) {
    PrintToken (STRING_TOKEN (STR_ERR_EFI_D_INFO), HiiHandle, EFI_D_INFO);
  } else {
    PrintToken (STRING_TOKEN (STR_ERR_EFI_D_INFO_2), HiiHandle, EFI_D_INFO);
  }

  if (EFIDebug & EFI_D_VARIABLE) {
    PrintToken (STRING_TOKEN (STR_ERR_EFI_D_VARIABLE), HiiHandle, EFI_D_VARIABLE);
  } else {
    PrintToken (STRING_TOKEN (STR_ERR_EFI_D_VARIABLE_2), HiiHandle, EFI_D_VARIABLE);
  }

  if (EFIDebug & EFI_D_BM) {
    PrintToken (STRING_TOKEN (STR_ERR_EFI_D_BM), HiiHandle, EFI_D_BM);
  } else {
    PrintToken (STRING_TOKEN (STR_ERR_EFI_D_BM_2), HiiHandle, EFI_D_BM);
  }

  if (EFIDebug & EFI_D_BLKIO) {
    PrintToken (STRING_TOKEN (STR_ERR_EFI_D_BLKIO), HiiHandle, EFI_D_BLKIO);
  } else {
    PrintToken (STRING_TOKEN (STR_ERR_EFI_D_BLKIO_2), HiiHandle, EFI_D_BLKIO);
  }

  if (EFIDebug & EFI_D_NET) {
    PrintToken (STRING_TOKEN (STR_ERR_EFI_D_NET), HiiHandle, EFI_D_NET);
  } else {
    PrintToken (STRING_TOKEN (STR_ERR_EFI_D_NET_2), HiiHandle, EFI_D_NET);
  }

  if (EFIDebug & EFI_D_UNDI) {
    PrintToken (STRING_TOKEN (STR_ERR_EFI_D_UNDI), HiiHandle, EFI_D_UNDI);
  } else {
    PrintToken (STRING_TOKEN (STR_ERR_EFI_D_UNDI_2), HiiHandle, EFI_D_UNDI);
  }

  if (EFIDebug & EFI_D_LOADFILE) {
    PrintToken (STRING_TOKEN (STR_ERR_EFI_D_LOADFILE), HiiHandle, EFI_D_LOADFILE);
  } else {
    PrintToken (STRING_TOKEN (STR_ERR_EFI_D_LOADFILE_2), HiiHandle, EFI_D_LOADFILE);
  }

  if (EFIDebug & EFI_D_EVENT) {
    PrintToken (STRING_TOKEN (STR_ERR_EFI_D_EVENT), HiiHandle, EFI_D_EVENT);
  } else {
    PrintToken (STRING_TOKEN (STR_ERR_EFI_D_EVENT_2), HiiHandle, EFI_D_EVENT);
  }

  if (EFIDebug & EFI_D_ERROR) {
    PrintToken (STRING_TOKEN (STR_ERR_EFI_D_ERROR), HiiHandle, EFI_D_ERROR);
  } else {
    PrintToken (STRING_TOKEN (STR_ERR_EFI_D_ERROR_2), HiiHandle, EFI_D_ERROR);
  }
  //
  //
  //
  return EFIDebug;
}

EFI_STATUS
InitializeErrorGetLineHelp (
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
{
  return LibCmdGetStringByToken (STRING_ARRAY_NAME, &EfiErrGuid, STRING_TOKEN (STR_ERR_LINEHELP), Str);
}
