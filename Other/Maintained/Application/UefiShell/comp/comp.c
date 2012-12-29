/*++

Copyright (c) 2005 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  comp.c
  
Abstract:

  EFI shell command "comp" - compare two files

Revision History

--*/

#include "EfiShellLib.h"
#include "comp.h"

extern UINT8    STRING_ARRAY_NAME[];

//
// This is the generated header file which includes whatever needs to be exported (strings + IFR)
//
#include STRING_DEFINES_FILE

EFI_GUID        EfiCompGuid = EFI_COMP_GUID;
EFI_HII_HANDLE  HiiCompHandle;
SHELL_VAR_CHECK_ITEM    CompCheckList[] = {
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

//
//
//
#define BLOCK_SIZE  (64 * 1024)

//
// Function declarations
//
EFI_STATUS
InitializeComp (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

//
// Entry Point
//
EFI_BOOTSHELL_CODE(
  EFI_APPLICATION_ENTRY_POINT(InitializeComp)
)

EFI_STATUS
InitializeComp (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
/*++

Routine Description:
  Command entry point. Compares the contents of two files.

Arguments:
  ImageHandle     The image handle. 
  SystemTable     The system table.

Returns:
  EFI_SUCCESS            - The command completed successfully
  EFI_INVALID_PARAMETER  - Input command arguments error
  EFI_OUT_OF_RESOURCES   - Out of memory
  Other                  - Misc error

--*/
{
  CHAR16                  **Argv;
  UINTN                   Argc;
  EFI_LIST_ENTRY          File1List;
  EFI_LIST_ENTRY          File2List;
  SHELL_FILE_ARG          *File1Arg;
  SHELL_FILE_ARG          *File2Arg;
  UINTN                   Size;
  UINTN                   ReadSize;
  UINT8                   *File1Buffer;
  UINT8                   *File2Buffer;
  UINTN                   NotTheSameCount;
  EFI_STATUS              Status;
  UINTN                   Index;
  UINTN                   Count;
  UINTN                   Address;
  CHAR16                  *ScanFile;
  CHAR16                  *Useful;
  SHELL_ARG_LIST          *Item;
  SHELL_VAR_CHECK_CODE    RetCode;
  SHELL_VAR_CHECK_PACKAGE ChkPck;

  //
  // Local variable initializations
  //
  File1Arg                        = NULL;
  File2Arg                        = NULL;
  ReadSize                        = 0;
  File1Buffer                     = NULL;
  File2Buffer                     = NULL;
  Address                         = 0;
  InitializeListHead (&File1List);
  InitializeListHead (&File2List);

  ZeroMem (&ChkPck, sizeof (SHELL_VAR_CHECK_PACKAGE));
  //
  // We are not being installed as an internal command driver, initialize
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
  Status = LibInitializeStrings (&HiiCompHandle, STRING_ARRAY_NAME, &EfiCompGuid);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (!EFI_PROPER_VERSION (0, 99)) {
    PrintToken (
      STRING_TOKEN (STR_SHELLENV_GNC_COMMAND_NOT_SUPPORT),
      HiiCompHandle,
      L"comp",
      EFI_VERSION_0_99 
      );
    Status = EFI_UNSUPPORTED;
    goto Done;
  }

  Argv  = SI->Argv;
  Argc  = SI->Argc;
  RetCode = LibCheckVariables (SI, CompCheckList, &ChkPck, &Useful);
  if (VarCheckOk != RetCode) {
    switch (RetCode) {
    case VarCheckUnknown:

      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_UNKNOWN_FLAG), HiiCompHandle, L"comp", Useful);
      break;

    case VarCheckDuplicate:

      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_DUP_FLAG), HiiCompHandle, L"comp", Useful);
      break;

    default:
      break;
    }

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
    if (ChkPck.ValueCount > 0 ||
        ChkPck.FlagCount > 2 ||
        (2 == ChkPck.FlagCount && !LibCheckVarGetFlag (&ChkPck, L"-b"))
        ) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiCompHandle, L"comp");
      Status = EFI_INVALID_PARAMETER;
    } else {
      PrintToken (STRING_TOKEN (STR_HELPINFO_COMP_VERBOSEHELP), HiiCompHandle);
      Status = EFI_SUCCESS;
    }

    goto Done;
  }
  //
  // Parse command line arguments
  //
  for (Index = 1; Index < Argc; Index += 1) {
    if (StrLen (Argv[Index]) == 0) {
      PrintToken (STRING_TOKEN (STR_COMP_ZERO_LENGTH_ARG), HiiCompHandle);
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }
  }
  //
  // verify number of arguments
  //
  if (ChkPck.ValueCount < 2) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_FEW), HiiCompHandle, L"comp");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  } else if (ChkPck.ValueCount > 2) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiCompHandle, L"comp");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  //
  // multiple files
  //
  Item      = ChkPck.VarList;
  ScanFile  = Item->VarStr;
  while (*ScanFile) {
    if ('*' == *ScanFile || '?' == *ScanFile) {
      break;
    }

    ScanFile++;
  }

  if (*ScanFile) {
    PrintToken (STRING_TOKEN (STR_COMP_FIRST_MULT_FILES), HiiCompHandle);
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }
  //
  // validate first file
  //
  Status = ShellFileMetaArg (Item->VarStr, &File1List);

  if (EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_COMP_CANNOT_OPEN), HiiCompHandle, Item->VarStr, Status);
    goto Done;
  }
  //
  // empty list
  //
  if (IsListEmpty (&File1List)) {
    Status = EFI_NOT_FOUND;
    PrintToken (STRING_TOKEN (STR_COMP_CANNOT_OPEN), HiiCompHandle, Item->VarStr, Status);
    goto Done;
  }

  File1Arg = CR (
              File1List.Flink,
              SHELL_FILE_ARG,
              Link,
              SHELL_FILE_ARG_SIGNATURE
              );

  //
  // Open error
  //
  if (EFI_ERROR (File1Arg->Status) || !File1Arg->Handle) {
    PrintToken (
      STRING_TOKEN (STR_COMP_CANNOT_OPEN),
      HiiCompHandle,
      File1Arg->FullName,
      File1Arg->Status
      );
    Status = File1Arg->Status;
    goto Done;
  }
  //
  // directory
  //
  if (File1Arg->Info && (File1Arg->Info->Attribute & EFI_FILE_DIRECTORY)) {
    PrintToken (STRING_TOKEN (STR_COMP_FIRST_DIR), HiiCompHandle);
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  Item = Item->Next;
  //
  // multiple files
  //
  ScanFile = Item->VarStr;
  while (*ScanFile) {
    if ('*' == *ScanFile || '?' == *ScanFile) {
      break;
    }

    ScanFile++;
  }

  if (*ScanFile) {
    PrintToken (STRING_TOKEN (STR_COMP_SECOND_MULT_FILES), HiiCompHandle);
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }
  //
  // Validate second file
  //
  Status = ShellFileMetaArg (Item->VarStr, &File2List);

  if (EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_COMP_CANNOT_OPEN), HiiCompHandle, Item->VarStr, Status);
    goto Done;
  }
  //
  // empty list
  //
  if (IsListEmpty (&File2List)) {
    Status = EFI_NOT_FOUND;
    PrintToken (STRING_TOKEN (STR_COMP_CANNOT_OPEN), HiiCompHandle, Item->VarStr, Status);
    goto Done;
  }

  File2Arg = CR (
              File2List.Flink,
              SHELL_FILE_ARG,
              Link,
              SHELL_FILE_ARG_SIGNATURE
              );

  //
  // open error
  //
  if (EFI_ERROR (File2Arg->Status) || !File2Arg->Handle) {
    PrintToken (
      STRING_TOKEN (STR_COMP_CANNOT_OPEN),
      HiiCompHandle,
      File2Arg->FullName,
      File2Arg->Status
      );
    Status = File2Arg->Status;
    goto Done;
  }
  //
  // directory
  //
  if (File2Arg->Info && (File2Arg->Info->Attribute & EFI_FILE_DIRECTORY)) {
    PrintToken (STRING_TOKEN (STR_COMP_SECOND_DIR), HiiCompHandle);
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }
  //
  // Allocate buffers for both files
  //
  File1Buffer = AllocatePool (BLOCK_SIZE);
  File2Buffer = AllocatePool (BLOCK_SIZE);
  if (!File1Buffer || !File2Buffer) {
    PrintToken (STRING_TOKEN (STR_COMP_OUT_OF_MEM), HiiCompHandle);
    Status = EFI_OUT_OF_RESOURCES;
    goto Done;
  }
  //
  // Compare files
  //
  PrintToken (STRING_TOKEN (STR_COMP_COMPARE), HiiCompHandle, File1Arg->FullName, File2Arg->FullName);

  //
  // Set positions to head
  //
  Status = File1Arg->Handle->SetPosition (File1Arg->Handle, 0);
  if (EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_COMP_SET_FILE_ERROR), HiiCompHandle, File1Arg->FullName, Status);
    goto Done;
  }

  Status = File2Arg->Handle->SetPosition (File2Arg->Handle, 0);
  if (EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_COMP_SET_FILE_ERROR), HiiCompHandle, File2Arg->FullName, Status);
    goto Done;
  }
  //
  // Read blocks one by one from both files and compare each pair of blocks
  //
  Size            = BLOCK_SIZE;
  NotTheSameCount = 0;

  while (Size > 0 && NotTheSameCount < 10) {
    //
    // Break the execution?
    //
    if (GetExecutionBreak ()) {
      goto Done;
    }
    //
    // Read a block from first file
    //
    Size = BLOCK_SIZE;
    Status = File1Arg->Handle->Read (
                                File1Arg->Handle,
                                &Size,
                                File1Buffer
                                );
    if (EFI_ERROR (Status)) {
      PrintToken (
        STRING_TOKEN (STR_COMP_READ_FILE_ERROR),
        HiiCompHandle,
        File1Arg->FullName,
        Status
        );
      NotTheSameCount++;
      break;
    }
    //
    // Read a block from second file
    //
    ReadSize = BLOCK_SIZE;
    Status = File2Arg->Handle->Read (
                                File2Arg->Handle,
                                &ReadSize,
                                File2Buffer
                                );
    if (EFI_ERROR (Status)) {
      PrintToken (
        STRING_TOKEN (STR_COMP_READ_FILE_ERROR),
        HiiCompHandle,
        File2Arg->FullName,
        Status
        );
      NotTheSameCount++;
      break;
    }

    if (ReadSize != Size) {
      PrintToken (
        STRING_TOKEN (STR_COMP_MISMATCH),
        HiiCompHandle,
        NotTheSameCount + 1
        );
      NotTheSameCount++;
      break;
    }
    //
    // Diff the buffer
    //
    for (Index = 0; (Index < Size) && (NotTheSameCount < 10); Index++) {
      if (File1Buffer[Index] != File2Buffer[Index]) {
        for (Count = 1; Count < 0x20 && (Index + Count) < Size; Count++) {
          if (File1Buffer[Index + Count] == File2Buffer[Index + Count]) {
            break;
          }
        }

        PrintToken (STRING_TOKEN (STR_COMP_DIFFERENCE_FILE1), HiiCompHandle, NotTheSameCount + 1, File1Arg->FullName);
        DumpHex (1, Address + Index, Count, &File1Buffer[Index]);
        PrintToken (STRING_TOKEN (STR_COMP_FILE2), HiiCompHandle, File2Arg->FullName);
        DumpHex (1, Address + Index, Count, &File2Buffer[Index]);
        Print (L"\n");

        NotTheSameCount++;
        Index += Count - 1;
      }
    }

    Address += Size;
  }
  //
  // Results
  //
  if (!NotTheSameCount) {
    PrintToken (STRING_TOKEN (STR_COMP_NO_DIFFERENCE), HiiCompHandle);
  } else {
    PrintToken (STRING_TOKEN (STR_COMP_DIFFERENCE), HiiCompHandle);
    Status = EFI_ABORTED;
  }

Done:
  if (File1Buffer) {
    FreePool (File1Buffer);
  }

  if (File2Buffer) {
    FreePool (File2Buffer);
  }

  ShellFreeFileList (&File1List);
  ShellFreeFileList (&File2List);
  LibCheckVarFreeVarList (&ChkPck);
  LibUnInitializeStrings ();
  //
  // Shell command always succeeds
  //
  return Status;
}

EFI_STATUS
InitializeCompGetLineHelp (
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
  return LibCmdGetStringByToken (STRING_ARRAY_NAME, &EfiCompGuid, STRING_TOKEN (STR_HELPINFO_COMP_LINEHELP), Str);
}
