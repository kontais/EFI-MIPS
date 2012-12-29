/*++

Copyright (c) 2005 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name: 

  TelnetMgmt.c
  
Abstract:

  Shell command "TelnetMgmt".

Revision History

--*/

#include "TelnetMgmt.h"
#include "EfiShellLib.h"
#include "TelnetServer.h"

extern UINT8    STRING_ARRAY_NAME[];

//
// This is the generated header file which includes whatever needs to be exported (strings + IFR)
//
#include STRING_DEFINES_FILE

EFI_HII_HANDLE  HiiHandle;
EFI_GUID        EfiTelnetMgmtGuid = EFI_TELNETMGMT_GUID;
EFI_GUID        mEfiTelnetServerGuid = EFI_TELNET_SERVER_PROTOCOL_GUID;
SHELL_VAR_CHECK_ITEM        TelnetmgmtCheckList[] = {
  {
    L"-t",
    0x01,
    0,
    FlagTypeNeedVar
  },
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

EFI_STATUS
InitializeTelnetMgmt (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  );

VOID
PrintMyHelpInfo2 (
  VOID
  );

EFI_BOOTSHELL_CODE(
  EFI_APPLICATION_ENTRY_POINT(InitializeTelnetMgmt)
)

EFI_STATUS
InitializeTelnetMgmt (
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
  EFI_STATUS                  Status;
  EFI_TELNET_SERVER_PROTOCOL  *TelnetProtocol;
  UINT8                       TerminalType;

  SHELL_VAR_CHECK_CODE        RetCode;
  CHAR16                      *Useful;
  SHELL_VAR_CHECK_PACKAGE     ChkPck;
  SHELL_ARG_LIST              *Item;

  ZeroMem (&ChkPck, sizeof (SHELL_VAR_CHECK_PACKAGE));
  //
  //  We are no being installed as an internal command driver, initialize
  //  as an nshell app and run
  //
  Status = EFI_SUCCESS;
  EFI_SHELL_APP_INIT (ImageHandle, SystemTable);
  
  //
  // Enable tab key which can pause the output
  //
  EnableOutputTabPause();

  Status = LibInitializeStrings (&HiiHandle, STRING_ARRAY_NAME, &EfiTelnetMgmtGuid);

  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (!EFI_PROPER_VERSION (1, 10)) {
    PrintToken (
      STRING_TOKEN (STR_SHELLENV_GNC_COMMAND_NOT_SUPPORT),
      HiiHandle,
      L"telnetmgmt",
      EFI_VERSION_1_10
      );
    Status = EFI_UNSUPPORTED;
    goto Done;
  }

  RetCode = LibCheckVariables (SI, TelnetmgmtCheckList, &ChkPck, &Useful);
  if (VarCheckOk != RetCode) {
    switch (RetCode) {
    case VarCheckUnknown:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_UNKNOWN_FLAG), HiiHandle, L"telnetmgmt", Useful);
      break;

    case VarCheckDuplicate:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_DUP_FLAG), HiiHandle, L"telnetmgmt", Useful);
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

  if (LibCheckVarGetFlag (&ChkPck, L"-?") != NULL) {
    if (ChkPck.ValueCount > 0 ||
        ChkPck.FlagCount > 2 ||
        (2 == ChkPck.FlagCount && !LibCheckVarGetFlag (&ChkPck, L"-b"))
        ) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiHandle, L"telnetmgmt");
      Status = EFI_INVALID_PARAMETER;
    } else {
      PrintToken (STRING_TOKEN (STR_HELPINFO_TELNETMGMT_VERBOSEHELP), HiiHandle);
      Status = EFI_SUCCESS;
    }
    goto Done;
  }

  Item = LibCheckVarGetFlag (&ChkPck, L"-t");
  if (NULL == Item) {
    PrintToken (STRING_TOKEN (STR_HELPINFO_TELNETMGMT_VERBOSEHELP), HiiHandle);
    Status = EFI_SUCCESS;
    goto Done;
  }

  TerminalType = (UINT8) StrToUInteger (Item->VarStr, &Status);
  if (EFI_ERROR (Status)) {
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }
  //
  // Locate Telnet Server protocol
  //
  Status = BS->LocateProtocol (&mEfiTelnetServerGuid, NULL, (VOID **) &TelnetProtocol);
  if (EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_TELNETMGMT_LOCATE_PROTOCOL), HiiHandle, Status);
    goto Done;
  }

  Status = TelnetProtocol->SetOption (TelnetProtocol, TERMINAL_TYPE, TerminalType);
  if (EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_TELNETMGMT_SETOPTION), HiiHandle, Status);
  }

  switch (TelnetProtocol->TerminalType) {
  case PcAnsiType:
    PrintToken (STRING_TOKEN (STR_TELNETMGMT_ANSI_TYPE), HiiHandle);
    break;

  case VT100Type:
    PrintToken (STRING_TOKEN (STR_TELNETMGMT_VT100_TYPE), HiiHandle);
    break;

  case VT100PlusType:
    PrintToken (STRING_TOKEN (STR_TELNETMGMT_VT100PLUS_TYPE), HiiHandle);
    break;

  case VTUTF8Type:
    PrintToken (STRING_TOKEN (STR_TELNETMGMT_VTUTF8_TYPE), HiiHandle);
    break;

  default:
    PrintToken (STRING_TOKEN (STR_TELNETMGMT_UNKNOWN_TYPE), HiiHandle);
    break;
  }

Done:
  LibCheckVarFreeVarList (&ChkPck);
  LibUnInitializeStrings ();
  return Status;
}

EFI_STATUS
InitializeTelnetmgmtGetLineHelp (
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
  return LibCmdGetStringByToken (
          STRING_ARRAY_NAME,
          &EfiTelnetMgmtGuid,
          STRING_TOKEN (STR_HELPINFO_TELNETMGMT_LINEHELP),
          Str
          );
}
