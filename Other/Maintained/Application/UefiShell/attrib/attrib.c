/*++

Copyright (c) 2005 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  attrib.c
  
Abstract:

  EFI shell command "attrib" - set file attributes

Revision History

--*/

#include "EfiShellLib.h"
#include "attrib.h"

extern UINT8  STRING_ARRAY_NAME[];

//
// This is the generated header file which includes whatever needs to be exported (strings + IFR)
//
#include STRING_DEFINES_FILE

//
// Function declarations
//
EFI_STATUS
EFIAPI
InitializeAttrib (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
AttribSet (
  IN CHAR16               *Str,
  IN OUT UINT64           *Attr
  );

EFI_STATUS
AttribFile (
  IN SHELL_FILE_ARG       *Arg,
  IN UINT64               Remove,
  IN UINT64               Add
  );

//
// Global Variables
//
EFI_HII_HANDLE  HiiHandle;
EFI_GUID        EfiAttribGuid = EFI_ATTRIB_GUID;
SHELL_VAR_CHECK_ITEM    AttribCheckList[] = {
  {
    L"+a",
    0x01,
    0x02,
    FlagTypeSingle
  },
  {
    L"-a",
    0x02,
    0x01,
    FlagTypeSingle
  },
  {
    L"+s",
    0x04,
    0x08,
    FlagTypeSingle
  },
  {
    L"-s",
    0x08,
    0x04,
    FlagTypeSingle
  },
  {
    L"+h",
    0x10,
    0x20,
    FlagTypeSingle
  },
  {
    L"-h",
    0x20,
    0x10,
    FlagTypeSingle
  },
  {
    L"+r",
    0x40,
    0x80,
    FlagTypeSingle
  },
  {
    L"-r",
    0x80,
    0x40,
    FlagTypeSingle
  },
  {
    L"-b",
    0x100,
    0,
    FlagTypeSingle
  },
  {
    L"-?",
    0x200,
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

//
// Entry Point
//
EFI_BOOTSHELL_CODE(
  EFI_APPLICATION_ENTRY_POINT(InitializeAttrib)
)

EFI_STATUS
EFIAPI
InitializeAttrib (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
/*++

Routine Description:

  Command entry point. Parses command line arguments and calls internal
  function to perform actual action.

Arguments:

  ImageHandle    The image handle. 
  SystemTable    The system table.

Returns:

  EFI_SUCCESS             - Command completed successfully
  EFI_INVALID_PARAMETER   - Command usage error
  Other value             - Unknown error
  
--*/
{
  UINTN                   Index;
  EFI_LIST_ENTRY          FileList;
  EFI_LIST_ENTRY          *Link;
  SHELL_FILE_ARG          *Arg;
  UINT64                  Remove;
  UINT64                  Add;
  EFI_STATUS              Status;

  SHELL_VAR_CHECK_CODE    RetCode;
  CHAR16                  *Useful;
  SHELL_ARG_LIST          *Item;
  SHELL_VAR_CHECK_PACKAGE ChkPck;

  ZeroMem (&ChkPck, sizeof (SHELL_VAR_CHECK_PACKAGE));

  //
  // We are no being installed as an internal command driver, initialize
  // as an nshell app and run
  //
  EFI_SHELL_APP_INIT (ImageHandle, SystemTable);

  //
  // Enable tab key which can pause the output
  //
  EnableOutputTabPause();
  
  Status = LibFilterNullArgs ();
  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Register our string package with HII and return the handle to it.
  // If previously registered we will simply receive the handle
  //
  EFI_SHELL_STR_INIT (HiiHandle, STRING_ARRAY_NAME, EfiAttribGuid);

  if (!EFI_PROPER_VERSION (0, 99)) {
    PrintToken (
      STRING_TOKEN (STR_SHELLENV_GNC_COMMAND_NOT_SUPPORT),
      HiiHandle,
      L"attrib",
      EFI_VERSION_0_99 
      );
    Status = EFI_UNSUPPORTED;
    goto Quit;
  }

  RetCode = LibCheckVariables (SI, AttribCheckList, &ChkPck, &Useful);
  if (VarCheckOk != RetCode) {
    switch (RetCode) {
    case VarCheckConflict:

      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_FLAG_CONFLICT), HiiHandle, L"attrib", Useful);
      break;

    case VarCheckDuplicate:

      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_DUP_FLAG), HiiHandle, L"attrib", Useful);
      break;

    case VarCheckUnknown:

      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_UNKNOWN_FLAG), HiiHandle, L"attrib", Useful);
      break;

    default:
      break;
    }

    Status = EFI_INVALID_PARAMETER;
    goto Quit;
  }
  //
  // Out put help.
  //
  if (LibCheckVarGetFlag (&ChkPck, L"-b") != NULL) {
    EnablePageBreak (DEFAULT_INIT_ROW, DEFAULT_AUTO_LF);
  }

  if (LibCheckVarGetFlag (&ChkPck, L"-?") != NULL) {
    if (IS_OLD_SHELL) {
      PrintToken (STRING_TOKEN (STR_NO_HELP), HiiHandle);
      goto Quit;
    }

    if (ChkPck.ValueCount > 0 ||
        ChkPck.FlagCount > 2 ||
        (2 == ChkPck.FlagCount && !LibCheckVarGetFlag (&ChkPck, L"-b"))
        ) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiHandle, L"attrib");
      Status = EFI_INVALID_PARAMETER;
    } else {
      PrintToken (STRING_TOKEN (STR_ATTRIB_VERBOSE_HELP), HiiHandle);
      Status = EFI_SUCCESS;
    }

    goto Quit;
  }
  //
  // Local Variable Initializations
  //
  InitializeListHead (&FileList);
  Link    = NULL;
  Arg     = NULL;
  Remove  = 0;
  Add     = 0;

  //
  // Parse command line arguments
  //
  Item = GetFirstFlag (&ChkPck);
  for (Index = 0; Index < ChkPck.FlagCount; Index += 1) {
    if (Item->FlagStr[0] == '-') {
      //
      // Attributes to remove
      //
      Status = AttribSet (&Item->FlagStr[1], &Remove);
      if (EFI_ERROR (Status)) {
        PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_ARG), HiiHandle, L"attrib", Item->FlagStr);
        goto Done;
      }
    } else if (Item->FlagStr[0] == '+') {
      //
      // Attributes to Add
      //
      Status = AttribSet (&Item->FlagStr[1], &Add);
      if (EFI_ERROR (Status)) {
        PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_ARG), HiiHandle, L"attrib", Item->FlagStr);
        goto Done;
      }
    } else {
      //
      // we should never get here
      //
      ASSERT (FALSE);
    }

    Item = GetNextArg (Item);
  }

  Item = GetFirstArg (&ChkPck);
  for (Index = 0; Index < ChkPck.ValueCount; Index += 1) {
    Status = ShellFileMetaArg (Item->VarStr, &FileList);
    if (EFI_ERROR (Status)) {
      PrintToken (STRING_TOKEN (STR_ATTRIB_CANNOT_OPEN), HiiHandle, L"attrib", Item->VarStr, Status);
      goto Done;
    }

    Item = GetNextArg (Item);
  }
  //
  // if no file is specified, get the whole directory
  //
  if (IsListEmpty (&FileList)) {
    Status = ShellFileMetaArg (L"*", &FileList);
    if (EFI_ERROR (Status)) {
      PrintToken (STRING_TOKEN (STR_ATTRIB_CANNOT_OPEN_DIR), HiiHandle, L"attrib", Status);
      goto Done;
    }
  }

  ShellDelDupFileArg (&FileList);
  //
  // Attrib each file
  //
  for (Link = FileList.Flink; Link != &FileList; Link = Link->Flink) {
    //
    // Break the execution?
    //
    if (GetExecutionBreak ()) {
      goto Done;
    }

    Arg     = CR (Link, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);
    Status  = AttribFile (Arg, Remove, Add);
  }

Done:
  ShellFreeFileList (&FileList);
Quit:
  LibCheckVarFreeVarList (&ChkPck);
  LibUnInitializeStrings ();
  return Status;
}

EFI_STATUS
AttribSet (
  IN CHAR16       *Str,
  IN OUT UINT64   *Attr
  )
{
  //
  // Convert to Lower, lest case/break not equal
  //
  StrLwr (Str);

  while (*Str) {
    //
    // Check one by one
    //
    switch (*Str) {
    case 'b':
      EnablePageBreak (DEFAULT_INIT_ROW, DEFAULT_AUTO_LF);
      break;

    case 'a':
      *Attr |= EFI_FILE_ARCHIVE;
      break;

    case 's':
      *Attr |= EFI_FILE_SYSTEM;
      break;

    case 'h':
      *Attr |= EFI_FILE_HIDDEN;
      break;

    case 'r':
      *Attr |= EFI_FILE_READ_ONLY;
      break;

    default:
      return EFI_INVALID_PARAMETER;
    }

    Str += 1;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
AttribFile (
  IN SHELL_FILE_ARG           *Arg,
  IN UINT64                   Remove,
  IN UINT64                   Add
  )
/*++

Routine Description:

  Set/remove attributes of a file

Arguments:

  Arg             The file whose attributes are to be modified. 
  Remove          Bitmap of the attributes to be removed.
  Add             Bitmap of the attributes to be added.

Returns:

  EFI_SUCCESS     Set attribute of file successfully
  EFI_ABORTED     Press 'q' to abort
  
--*/
{
  UINT64        Attr;
  EFI_STATUS    Status;
  EFI_FILE_INFO *Info;

  //
  // Local variable initializations
  //
  Attr    = 0;
  Status  = EFI_SUCCESS;
  Info    = NULL;

  //
  // Validate the file
  //
  Status = Arg->Status;
  if (!EFI_ERROR (Status)) {
    //
    // Attrib it
    //
    Info = Arg->Info;
    if (Add || Remove) {
      Info->Attribute = Info->Attribute & (~Remove) | Add;
      Status = Arg->Handle->SetInfo (
                              Arg->Handle,
                              &gEfiFileInfoGuid,
                              (UINTN) Info->Size,
                              Info
                              );
    }
  }
  //
  // Output result
  //
  if (EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_ATTRIB_CANNOT_SET), HiiHandle, L"attrib", Arg->FullName, Status);
    return Status;
  } else {
    Attr = Info->Attribute;
    PrintToken (
      STRING_TOKEN (STR_ATTRIB_EFI_DASHR),
      HiiHandle,
      L"attrib",
      Attr & EFI_FILE_DIRECTORY ? 'D' : ' ',
      Attr & EFI_FILE_ARCHIVE ? 'A' : ' ',
      Attr & EFI_FILE_SYSTEM ? 'S' : ' ',
      Attr & EFI_FILE_HIDDEN ? 'H' : ' ',
      Attr & EFI_FILE_READ_ONLY ? 'R' : ' ',
      Arg->FullName
      );
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
InitializeAttribGetLineHelp (
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
  return LibCmdGetStringByToken (STRING_ARRAY_NAME, &EfiAttribGuid, STRING_TOKEN (STR_ATTRIB_LINE_HELP), Str);
}
