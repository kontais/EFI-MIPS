/*++
 
Copyright (c) 2005 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  ver.c
  
Abstract:

  Shell app "ver"



Revision History

--*/

#include "EfiShellLib.h"
#include "ver.h"

extern UINT8  STRING_ARRAY_NAME[];

//
// This is the generated header file which includes whatever needs to be exported (strings + IFR)
//
#include STRING_DEFINES_FILE

EFI_GUID      EfiVerGuid = EFI_VER_GUID;
SHELL_VAR_CHECK_ITEM    VerCheckList[] = {
  {
    L"-s",
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

//
//
//
EFI_STATUS
EFIAPI
InitializeVer (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  );

//
//
//
EFI_BOOTSHELL_CODE(
  EFI_APPLICATION_ENTRY_POINT(InitializeVer)
)

EFI_STATUS
EFIAPI
InitializeVer (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
/*++

Routine Description:
  Displays version information.

Arguments:
  ImageHandle     The image handle. 
  SystemTable     The system table.

Returns:
  EFI_SUCCESS     - Command completed successfully

--*/
{
  EFI_HII_HANDLE          HiiHandle;
  EFI_STATUS              Status;
  SHELL_VAR_CHECK_CODE    RetCode;
  CHAR16                  *Useful;
  SHELL_VAR_CHECK_PACKAGE ChkPck;

  //
  // We are no being installed as an internal command driver, initialize
  // as an nshell app and run
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
  Status = LibInitializeStrings (&HiiHandle, STRING_ARRAY_NAME, &EfiVerGuid);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (!EFI_PROPER_VERSION (0, 99)) {
    PrintToken (
      STRING_TOKEN (STR_SHELLENV_GNC_COMMAND_NOT_SUPPORT),
      HiiHandle,
      L"ver",
      EFI_VERSION_0_99 
      );
    Status = EFI_UNSUPPORTED;
    goto Done;
  }

  ZeroMem (&ChkPck, sizeof (SHELL_VAR_CHECK_PACKAGE));
  Useful  = NULL;

  RetCode = LibCheckVariables (SI, VerCheckList, &ChkPck, &Useful);
  if (VarCheckOk != RetCode) {
    switch (RetCode) {
    case VarCheckDuplicate:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_DUP_FLAG), HiiHandle, L"ver", Useful);
      break;

    case VarCheckUnknown:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_UNKNOWN_FLAG), HiiHandle, L"ver", Useful);
      break;

    case VarCheckConflict:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_FLAG_CONFLICT), HiiHandle, L"ver", Useful);
      break;

    default:
      break;
    }

    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  if (ChkPck.ValueCount > 0) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiHandle, L"ver");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }
  //
  // Out put help.
  //
  if (LibCheckVarGetFlag (&ChkPck, L"-b") != NULL) {
    EnablePageBreak (DEFAULT_INIT_ROW, DEFAULT_AUTO_LF);
  }

  if (LibCheckVarGetFlag (&ChkPck, L"-?") != NULL) {
    if (ChkPck.FlagCount > 2 || (2 == ChkPck.FlagCount && !LibCheckVarGetFlag (&ChkPck, L"-b"))) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiHandle, L"ver");
      Status = EFI_INVALID_PARAMETER;
    } else {
      if (IS_OLD_SHELL) {
        PrintToken (STRING_TOKEN (STR_NO_HELP), HiiHandle);
      } else {
        PrintToken (STRING_TOKEN (STR_VER_VERBOSE_HELP), HiiHandle);
      }

      Status = EFI_SUCCESS;
    }

    goto Done;
  }

  if (ChkPck.ValueCount > 0) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiHandle, L"ver");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  if (LibCheckVarGetFlag (&ChkPck, L"-s")) {
    PrintToken (STRING_TOKEN (STR_VER_SHELL_VER), HiiHandle, SE2->MajorVersion, SE2->MinorVersion);
    PrintToken (STRING_TOKEN (STR_VER_SHELL_MACHINE_TYPE), HiiHandle, LibGetMachineTypeString (EFI_IMAGE_MACHINE_TYPE));
    PrintToken (STRING_TOKEN (STR_VER_SHELL_SIG), HiiHandle, &SE2->SESGuid);
  } else {
    //
    // Print version info
    //
    PrintToken (STRING_TOKEN (STR_VER_SPEC_REVISION), HiiHandle, ST->Hdr.Revision >> 16, ST->Hdr.Revision & 0xffff);
    PrintToken (STRING_TOKEN (STR_VER_EFI_VENDOR), HiiHandle, ST->FirmwareVendor);
    PrintToken (
      STRING_TOKEN (STR_VER_EFI_REVISION),
      HiiHandle,
      ST->FirmwareRevision >> 16,
      ST->FirmwareRevision & 0xffff
      );
    EFI_BUILD_VERSION_CODE (
      PrintToken (STRING_TOKEN (STR_VER_EFI_BUILD_REVISION), HiiHandle, EFI_BUILD_VERSION);
    )
    //
    // Display additional version info depending on processor type
    //
    DisplayExtendedVersionInfo (ImageHandle, SystemTable);
  }
  //
  // Done
  //
  Status = EFI_SUCCESS;
Done:
  LibCheckVarFreeVarList (&ChkPck);
  LibUnInitializeStrings ();
  return Status;
}

EFI_STATUS
EFIAPI
InitializeVerGetLineHelp (
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
  return LibCmdGetStringByToken (STRING_ARRAY_NAME, &EfiVerGuid, STRING_TOKEN (STR_VER_LINE_HELP), Str);
}
