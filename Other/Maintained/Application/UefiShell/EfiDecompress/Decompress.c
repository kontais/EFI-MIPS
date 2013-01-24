/*++

Copyright (c) 2005 - 2007, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  decompress.c
  
Abstract:

  EFI shell command "EfiDecompress" - decompress a file

Revision History

--*/

#include "EfiShellLib.h"
#include "Decompress.h"

extern UINT8    STRING_ARRAY_NAME[];

//
// This is the generated header file which includes whatever needs to be exported (strings + IFR)
//
#include STRING_DEFINES_FILE

#include EFI_PROTOCOL_DEFINITION (Decompress)

EFI_HII_HANDLE  HiiDecompressHandle;
EFI_GUID        EfiDecompressGuid = EFI_DECOMPRESS_GUID;
SHELL_VAR_CHECK_ITEM    DecompressCheckList[] = {
  {
    L"-b",
    0x1,
    0,
    FlagTypeSingle
  },
  {
    L"-?",
    0x2,
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
// Function declarations
//
EFI_STATUS
InitializeDecompress (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

//
// Entry Point
//
EFI_BOOTSHELL_CODE(
  EFI_APPLICATION_ENTRY_POINT(InitializeDecompress)
)

EFI_STATUS
InitializeDecompress (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
/*++

Routine Description:
  Command entry point. Decompress the contents of a file.

Arguments:
  ImageHandle     The image handle. 
  SystemTable     The system table.

Returns:
  EFI_SUCCESS             - The command completed successfully
  EFI_INVALID_PARAMETER   - Command usage error
  EFI_OUT_OF_RESOURCES    - Out of memory
  EFI_UNSUPPORTED         - Protocols unsupported
  Other value             - Unknown error
   
--*/
{
  EFI_LIST_ENTRY          File1List;
  EFI_LIST_ENTRY          File2List;
  SHELL_FILE_ARG          *File1Arg;
  SHELL_FILE_ARG          *File2Arg;
  EFI_FILE_HANDLE         File2Handle;
  UINTN                   SourceSize;
  UINTN                   DestinationSize;
  UINTN                   ScratchSize;
  UINT8                   *File1Buffer;
  UINT8                   *File2Buffer;
  UINT8                   *Scratch;
  EFI_STATUS              Status;
  EFI_DECOMPRESS_PROTOCOL *Decompress;
  SHELL_VAR_CHECK_CODE    RetCode;
  CHAR16                  *Useful;
  SHELL_ARG_LIST          *Item;
  BOOLEAN                 DstFileExist;
  SHELL_VAR_CHECK_PACKAGE ChkPck;
  
  //
  // Local variable initializations
  //
  DstFileExist  = FALSE;
  File1Buffer   = NULL;
  File2Buffer   = NULL;
  Scratch       = NULL;
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
  EFI_SHELL_STR_INIT (HiiDecompressHandle, STRING_ARRAY_NAME, EfiDecompressGuid);

  if (!EFI_PROPER_VERSION (0, 99)) {
    PrintToken (
      STRING_TOKEN (STR_SHELLENV_GNC_COMMAND_NOT_SUPPORT),
      HiiDecompressHandle,
      L"efidecompress",
      EFI_VERSION_0_99 
      );
    Status = EFI_UNSUPPORTED;
    goto Quit;
  }

  LibFilterNullArgs ();
  //
  // Parse command line arguments
  //
  RetCode = LibCheckVariables (SI, DecompressCheckList, &ChkPck, &Useful);
  if (VarCheckOk != RetCode) {
    switch (RetCode) {
    case VarCheckUnknown:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_UNKNOWN_FLAG), HiiDecompressHandle, L"efidecompress", Useful);
      break;

    default:
      break;
    }

    Status = EFI_INVALID_PARAMETER;
    goto Quit;
  }

  if (LibCheckVarGetFlag (&ChkPck, L"-b")) {
    EnablePageBreak (DEFAULT_INIT_ROW, DEFAULT_AUTO_LF);
  }

  if (LibCheckVarGetFlag (&ChkPck, L"-?")) {
    if (ChkPck.ValueCount > 0 ||
        ChkPck.FlagCount > 2 ||
        (2 == ChkPck.FlagCount && !LibCheckVarGetFlag (&ChkPck, L"-b"))
        ) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiDecompressHandle, L"efidecompress");
      Status = EFI_INVALID_PARAMETER;
    } else {
      PrintToken (STRING_TOKEN (STR_EFIDECOMPRESS_VERBOSEHELP), HiiDecompressHandle);
      Status = EFI_SUCCESS;
    }

    goto Quit;
  }
  //
  //
  //
  Status = LibLocateProtocol (&gEfiDecompressProtocolGuid, &Decompress);
  if (EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_DECOMPRESS_PROTOCOL_NOT_FOUND), HiiDecompressHandle, L"efidecompress");
    Status = EFI_UNSUPPORTED;
    goto Quit;
  }
  //
  // verify number of arguments
  //
  if (ChkPck.ValueCount > 2) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiDecompressHandle, L"efidecompress");
    Status = EFI_INVALID_PARAMETER;
    goto Quit;
  }

  if (ChkPck.ValueCount < 2) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_FEW), HiiDecompressHandle, L"efidecompress");
    Status = EFI_INVALID_PARAMETER;
    goto Quit;
  }
  //
  // validate first file
  //
  Item    = GetFirstArg (&ChkPck);
  Status  = ShellFileMetaArg (Item->VarStr, &File1List);
  if (EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_CANNOT_OPEN_FILE), HiiDecompressHandle, L"efidecompress", Item->VarStr);
    goto Done;
  }
  //
  // empty list
  //
  if (IsListEmpty (&File1List)) {
    Status = EFI_NOT_FOUND;
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_CANNOT_OPEN_FILE), HiiDecompressHandle, L"efidecompress", Item->VarStr);
    goto Done;
  }
  //
  // multiple files
  //
  if (File1List.Flink->Flink != &File1List) {
    PrintToken (STRING_TOKEN (STR_DECOMPRESS_FIRST_MULT_FILES), HiiDecompressHandle, L"efidecompress");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  File1Arg = CR (File1List.Flink, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);

  //
  // Open error
  //
  if (EFI_ERROR (File1Arg->Status) || !File1Arg->Handle) {
    PrintToken (
      STRING_TOKEN (STR_SHELLENV_GNC_CANNOT_OPEN_FILE),
      HiiDecompressHandle,
      L"efidecompress",
      File1Arg->FullName
      );
    Status = File1Arg->Status;
    goto Done;
  }
  //
  // directory
  //
  if (File1Arg->Info && (File1Arg->Info->Attribute & EFI_FILE_DIRECTORY)) {
    PrintToken (STRING_TOKEN (STR_DECOMPRESS_FIRST_ARG_DIR), HiiDecompressHandle, L"efidecompress");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }
  //
  // Validate second file
  //
  Item    = GetNextArg (Item);
  Status  = ShellFileMetaArg (Item->VarStr, &File2List);
  if (EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_CANNOT_OPEN_FILE), HiiDecompressHandle, L"efidecompress", Item->VarStr);
    goto Done;
  }
  //
  // empty list
  //
  if (IsListEmpty (&File2List)) {
    Status = EFI_NOT_FOUND;
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_CANNOT_OPEN_FILE), HiiDecompressHandle, L"efidecompress", Item->VarStr);
    goto Done;
  }
  //
  // multiple files
  //
  if (File2List.Flink->Flink != &File2List) {
    PrintToken (STRING_TOKEN (STR_DECOMPRESS_SECOND_MULT_FILES), HiiDecompressHandle, L"efidecompress");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  File2Arg = CR (
              File2List.Flink,
              SHELL_FILE_ARG,
              Link,
              SHELL_FILE_ARG_SIGNATURE
              );

  if (!File2Arg->Parent) {
    PrintToken (
      STRING_TOKEN (STR_SHELLENV_GNC_CANNOT_OPEN_FILE),
      HiiDecompressHandle,
      L"efidecompress",
      File2Arg->FullName
      );
    Status = File2Arg->Status;
    goto Done;
  }
  //
  // directory
  //
  if (File2Arg->Info && (File2Arg->Info->Attribute & EFI_FILE_DIRECTORY)) {
    PrintToken (STRING_TOKEN (STR_DECOMPRESS_SECOND_DIR), HiiDecompressHandle, L"efidecompress");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }
  //
  // Allocate buffers for both files
  //
  SourceSize  = (UINTN) File1Arg->Info->FileSize;
  File1Buffer = AllocatePool (SourceSize);
  if (File1Buffer == NULL) {
    PrintToken (STRING_TOKEN (STR_DECOMPRESS_OUT_OF_MEM), HiiDecompressHandle, L"efidecompress");
    Status = EFI_OUT_OF_RESOURCES;
    goto Done;
  }

  Status = File1Arg->Handle->Read (File1Arg->Handle, &SourceSize, File1Buffer);
  if (EFI_ERROR (Status)) {
    PrintToken (
      STRING_TOKEN (STR_DECOMPRESS_READ_ERROR),
      HiiDecompressHandle,
      L"efidecompress",
      File1Arg->FullName,
      Status
      );
    goto Done;
  }

  DestinationSize = 0;
  ScratchSize     = 0;
  Status = Decompress->GetInfo (
                        Decompress,
                        File1Buffer,
                        (UINT32) SourceSize,
                        (UINT32 *) &DestinationSize,
                        (UINT32 *) &ScratchSize
                        );
  if (EFI_ERROR (Status)) {
    PrintToken (
      STRING_TOKEN (STR_DECOMPRESS_COMPRESSED_DAMAGED),
      HiiDecompressHandle,
      L"eficompress",
      File1Arg->FullName,
      Status
      );
    goto Done;
  }

  File2Buffer = AllocatePool (DestinationSize);
  if (File2Buffer == NULL) {
    PrintToken (STRING_TOKEN (STR_DECOMPRESS_OUT_OF_MEM), HiiDecompressHandle, L"efidecompress");
    Status = EFI_OUT_OF_RESOURCES;
    goto Done;
  }

  Scratch = AllocatePool (ScratchSize);
  if (Scratch == NULL) {
    PrintToken (STRING_TOKEN (STR_DECOMPRESS_OUT_OF_MEM), HiiDecompressHandle, L"efidecompress");
    Status = EFI_OUT_OF_RESOURCES;
    goto Done;
  }

  Status = Decompress->Decompress (
                        Decompress,
                        File1Buffer,
                        (UINT32) SourceSize,
                        File2Buffer,
                        (UINT32) DestinationSize,
                        Scratch,
                        (UINT32) ScratchSize
                        );
  if (EFI_ERROR (Status)) {
    PrintToken (
      STRING_TOKEN (STR_DECOMPRESS_ERROR),
      HiiDecompressHandle,
      L"efidecompress",
      File1Arg->FullName,
      Status
      );
    goto Done;
  }

  if (File2Arg->Status == EFI_SUCCESS &&
      File2Arg->OpenMode & EFI_FILE_MODE_READ &&
      File2Arg->OpenMode & EFI_FILE_MODE_WRITE
      ) {
    File2Handle               = File2Arg->Handle;
    File2Arg->Info->FileSize  = 0;
    Status = File2Handle->SetInfo (
                            File2Handle,
                            &gEfiFileInfoGuid,
                            (UINTN) File2Arg->Info->Size,
                            File2Arg->Info
                            );
    Status = File2Handle->Write (File2Handle, &DestinationSize, File2Buffer);
    if (EFI_ERROR (Status)) {
      PrintToken (
        STRING_TOKEN (STR_DECOMPRESS_WRITE_ERROR),
        HiiDecompressHandle,
        L"efidecompress",
        File2Arg->FullName,
        Status
        );
      goto Done;
    }

    DstFileExist = TRUE;
  } else {
    Status = File2Arg->Parent->Open (
                                File2Arg->Parent,
                                &File2Handle,
                                File2Arg->FileName,
                                EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE,
                                0
                                );
    if (EFI_ERROR (Status)) {
      PrintToken (
        STRING_TOKEN (STR_DECOMPRESS_CREATE_ERROR),
        HiiDecompressHandle,
        L"efidecompress",
        File2Arg->FullName,
        Status
        );
      goto Done;
    }

    Status = File2Handle->Write (File2Handle, &DestinationSize, File2Buffer);
    if (EFI_ERROR (Status)) {
      PrintToken (
        STRING_TOKEN (STR_DECOMPRESS_WRITE_ERROR),
        HiiDecompressHandle,
        L"efidecompress",
        File2Arg->FullName,
        Status
        );
      File2Handle->Close (File2Handle);
      goto Done;
    }
  }

  if (!DstFileExist) {
    File2Handle->Close (File2Handle);
  }

  ASSERT (!EFI_ERROR (Status));
  PrintToken (
    STRING_TOKEN (STR_DECOMPRESS_SUCCESS),
    HiiDecompressHandle,
    L"efidecompress",
    File1Arg->FullName,
    File2Arg->FullName
    );

Done:
  if (File1Buffer) {
    FreePool (File1Buffer);
  }

  if (File2Buffer) {
    FreePool (File2Buffer);
  }

  if (Scratch) {
    FreePool (Scratch);
  }

  ShellFreeFileList (&File1List);
  ShellFreeFileList (&File2List);

Quit:
  //
  // Shell command always succeeds
  //
  LibUnInitializeStrings ();
  LibCheckVarFreeVarList (&ChkPck);
  return Status;
}

EFI_STATUS
InitializeDecompressGetLineHelp (
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
{
  return LibCmdGetStringByToken (STRING_ARRAY_NAME, &EfiDecompressGuid, STRING_TOKEN (STR_EFIDECOMPRESS_LINEHELP), Str);
}
