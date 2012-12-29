/*++
 
Copyright (c) 2005 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  guid.c
  
Abstract:

  Shell command "guid"



Revision History

--*/

#include "EfiShellLib.h"
#include "guid.h"

extern UINT8  STRING_ARRAY_NAME[];

//
// This is the generated header file which includes whatever needs to be exported (strings + IFR)
//
#include STRING_DEFINES_FILE

//
// Global Variables
//
EFI_HII_HANDLE  HiiHandle;
EFI_GUID        EfiGuidGuid = EFI_GUID_GUID;
SHELL_VAR_CHECK_ITEM    GuidCheckList[] = {
  {
    L"-b",
    0x01,
    0,
    FlagTypeSingle
  },
  {
    L"-?",
    0x02,
    0,
    FlagTypeSingle
  },
  {
    NULL,
    0,
    0,
    0
  }
};

EFI_BOOTSHELL_CODE (
  EFI_APPLICATION_ENTRY_POINT(GuidMain)
)
//
//
//
EFI_STATUS
GuidMain (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  );

//
//
//
EFI_STATUS
GuidMain (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  )
/*++

Routine Description:

  Code for external shell "guid" command.

Arguments:

  ImageHandle - The image handle
  SystemTable - The system table

Returns:

--*/
{
  PROTOCOL_INFO           *Prot;
  UINTN                   Len;
  UINTN                   SLen;
  EFI_STATUS              Status;

  SHELL_VAR_CHECK_CODE    RetCode;
  CHAR16                  *Useful;
  SHELL_VAR_CHECK_PACKAGE ChkPck;

  ZeroMem (&ChkPck, sizeof (SHELL_VAR_CHECK_PACKAGE));
  EFI_SHELL_APP_INIT (ImageHandle, SystemTable);
  
  //
  // Enable tab key which can pause the output
  //
  EnableOutputTabPause();

  Status = LibInitializeStrings (&HiiHandle, STRING_ARRAY_NAME, &EfiGuidGuid);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (!EFI_PROPER_VERSION (0, 99)) {
    PrintToken (
      STRING_TOKEN (STR_SHELLENV_GNC_COMMAND_NOT_SUPPORT),
      HiiHandle,
      L"guid",
      EFI_VERSION_0_99 
      );
    Status = EFI_UNSUPPORTED;
    goto Done;
  }
  //
  // Initializing variable to aVOID level 4 warning
  //
  RetCode = LibCheckVariables (SI, GuidCheckList, &ChkPck, &Useful);

  if (VarCheckOk != RetCode) {
    switch (RetCode) {
    case VarCheckUnknown:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_UNKNOWN_FLAG), HiiHandle, L"guid", Useful);
      break;

    case VarCheckDuplicate:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_DUP_FLAG), HiiHandle, L"guid", Useful);
      break;

    default:
      break;
    }

    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  if (LibCheckVarGetFlag (&ChkPck, L"-b") != NULL) {
    EnablePageBreak (DEFAULT_INIT_ROW, DEFAULT_AUTO_LF);
  }

  if (LibCheckVarGetFlag (&ChkPck, L"-?")) {
    if (ChkPck.ValueCount > 0 ||
        ChkPck.FlagCount > 2 ||
        (2 == ChkPck.FlagCount && !LibCheckVarGetFlag (&ChkPck, L"-b"))
        ) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiHandle, L"guid");
      Status = EFI_INVALID_PARAMETER;
    } else {
      PrintToken (STRING_TOKEN (STR_GUID_VERBOSEHELP), HiiHandle);
      Status = EFI_SUCCESS;
    }

    goto Done;
  }

  if (ChkPck.ValueCount > 0) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiHandle, L"guid");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }
  //
  // Find the protocol entry for this id
  //
  Status  = EFI_SUCCESS;
  SLen    = 0;
  Prot    = NULL;
  ShellInitProtocolInfoEnumerator ();
  do {
    Status = ShellNextProtocolInfo (&Prot);
    if (Prot == NULL && Status == EFI_SUCCESS) {
      break;
    }

    Len = StrLen (Prot->IdString);
    if (StrLen (Prot->IdString) > SLen) {
      SLen = Len;
    }
  } while (Prot != NULL);

  ShellResetProtocolInfoEnumerator ();
  do {
    Status = ShellNextProtocolInfo (&Prot);
    if (Prot == NULL && Status == EFI_SUCCESS) {
      break;
    }
    //
    // Can't use Lib function to dump the guid as it may lookup the
    // "short name" for it
    //
    if (GetExecutionBreak ()) {
      Status = EFI_ABORTED;
      break;
    }

    PrintToken (
      STRING_TOKEN (STR_SHELLENV_PROTID_FOURTEEN_VARS),
      HiiHandle,
      SLen,
      Prot->IdString,
      Prot->ProtocolId.Data1,
      Prot->ProtocolId.Data2,
      Prot->ProtocolId.Data3,
      Prot->ProtocolId.Data4[0],
      Prot->ProtocolId.Data4[1],
      Prot->ProtocolId.Data4[2],
      Prot->ProtocolId.Data4[3],
      Prot->ProtocolId.Data4[4],
      Prot->ProtocolId.Data4[5],
      Prot->ProtocolId.Data4[6],
      Prot->ProtocolId.Data4[7],
      (Prot->DumpToken || Prot->DumpInfo) ? L'*' : L' '
      );
  } while (Prot != NULL);

  ShellCloseProtocolInfoEnumerator ();

Done:
  LibCheckVarFreeVarList (&ChkPck);
  LibUnInitializeStrings ();
  return Status;
}

EFI_STATUS
GuidMainGetLineHelp (
  OUT CHAR16          **Str
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
  return LibCmdGetStringByToken (STRING_ARRAY_NAME, &EfiGuidGuid, STRING_TOKEN (STR_GUID_LINEHELP), Str);
}
