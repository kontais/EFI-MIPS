/*++

Copyright (c) 2005 - 2007, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  compressMain.c
  
Abstract:

  EFI shell command "EfiCompress" - compress a file

Revision History

--*/

#include "EfiShellLib.h"
#include "Compress.h"

extern UINT8    STRING_ARRAY_NAME[];

//
// This is the generated header file which includes whatever needs to be exported (strings + IFR)
//
#include STRING_DEFINES_FILE

EFI_HII_HANDLE  HiiCompressHandle;
EFI_GUID        EfiCompressGuid = EFI_COMPRESS_GUID;
SHELL_VAR_CHECK_ITEM    CompressCheckList[] = {
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
InitializeCompress (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

//
// Entry Point
//
EFI_BOOTSHELL_CODE(
  EFI_APPLICATION_ENTRY_POINT(InitializeCompress)
)

EFI_STATUS
InitializeCompress (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
/*++

Routine Description:
  Command entry point. Compress the contents of a file.

Arguments:
  ImageHandle     The image handle. 
  SystemTable     The system table.

Returns:
  EFI_SUCCESS             - The command completed successfully
  EFI_INVALID_PARAMETER   - Command usage error
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
  UINT8                   *File1Buffer;
  UINT8                   *File2Buffer;
  EFI_STATUS              Status;
  INT32                   Ratio;
  SHELL_VAR_CHECK_CODE    RetCode;
  CHAR16                  *Useful;
  SHELL_ARG_LIST          *Item;
  SHELL_VAR_CHECK_PACKAGE ChkPck;
  BOOLEAN                 DstFileExist;

  //
  // Local variable initializations
  //
  File1Buffer             = NULL;
  File2Buffer             = NULL;
  DstFileExist            = FALSE;
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
  EFI_SHELL_STR_INIT (HiiCompressHandle, STRING_ARRAY_NAME, EfiCompressGuid);

  if (!EFI_PROPER_VERSION (0, 99)) {
    PrintToken (
      STRING_TOKEN (STR_SHELLENV_GNC_COMMAND_NOT_SUPPORT),
      HiiCompressHandle,
      L"eficompress",
      EFI_VERSION_0_99 
      );
    Status = EFI_UNSUPPORTED;
    goto Done;
  }

  LibFilterNullArgs ();
  //
  // Parse command line arguments
  //
  RetCode = LibCheckVariables (SI, CompressCheckList, &ChkPck, &Useful);
  if (VarCheckOk != RetCode) {
    switch (RetCode) {
    case VarCheckUnknown:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_UNKNOWN_FLAG), HiiCompressHandle, L"eficompress", Useful);
      break;

    default:
      break;
    }

    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  if (LibCheckVarGetFlag (&ChkPck, L"-b")) {
    EnablePageBreak (DEFAULT_INIT_ROW, DEFAULT_AUTO_LF);
  }

  if (LibCheckVarGetFlag (&ChkPck, L"-?")) {
    if (ChkPck.ValueCount > 0 ||
        ChkPck.FlagCount > 2 ||
        (2 == ChkPck.FlagCount && !LibCheckVarGetFlag (&ChkPck, L"-b"))
        ) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiCompressHandle, L"eficompress");
      Status = EFI_INVALID_PARAMETER;
    } else {
      PrintToken (STRING_TOKEN (STR_EFICOMPRESS_VERBOSEHELP), HiiCompressHandle);
      Status = EFI_SUCCESS;
    }

    goto Done;
  }
  //
  // verify number of arguments
  //
  if (ChkPck.ValueCount > 2) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiCompressHandle, L"eficompress");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  if (ChkPck.ValueCount < 2) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_FEW), HiiCompressHandle, L"eficompress");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }
  //
  // validate first file
  //
  Item    = ChkPck.VarList;
  Status  = ShellFileMetaArg (Item->VarStr, &File1List);
  if (EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_COMPRESS_CANNOT_OPEN), HiiCompressHandle, L"eficompress", Item->VarStr, Status);
    goto Done;
  }
  //
  // empty list
  //
  if (IsListEmpty (&File1List)) {
    Status = EFI_NOT_FOUND;
    PrintToken (STRING_TOKEN (STR_COMPRESS_CANNOT_OPEN), HiiCompressHandle, L"eficompress", Item->VarStr, Status);
    goto Done;
  }
  //
  // multiple files
  //
  if (File1List.Flink->Flink != &File1List) {
    PrintToken (STRING_TOKEN (STR_COMPRESS_FIRST_MULT_FILES), HiiCompressHandle, L"eficompress");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  File1Arg = CR (File1List.Flink, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);

  //
  // Open error
  //
  if (EFI_ERROR (File1Arg->Status) || !File1Arg->Handle) {
    PrintToken (
      STRING_TOKEN (STR_COMPRESS_CANNOT_OPEN),
      HiiCompressHandle,
      L"eficompress",
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
    PrintToken (STRING_TOKEN (STR_COMPRESS_FIRST_DIR), HiiCompressHandle, L"eficompress");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }
  //
  // Validate second file
  //
  Item    = Item->Next;
  Status  = ShellFileMetaArg (Item->VarStr, &File2List);
  if (EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_COMPRESS_CANNOT_OPEN), HiiCompressHandle, L"eficompress", Item->VarStr, Status);
    goto Done;
  }
  //
  // multiple files
  //
  if (File2List.Flink->Flink != &File2List) {
    PrintToken (STRING_TOKEN (STR_COMPRESS_SECOND_MULT_FILES), HiiCompressHandle, L"eficompress");
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
      STRING_TOKEN (STR_COMPRESS_CANNOT_OPEN),
      HiiCompressHandle,
      L"eficompress",
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
    PrintToken (STRING_TOKEN (STR_COMPRESS_SECOND_DIR), HiiCompressHandle, L"eficompress");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }
  //
  //
  //
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
    if (EFI_ERROR (Status)) {
      PrintToken (
        STRING_TOKEN (STR_COMPRESS_WRITE_ERROR),
        HiiCompressHandle,
        L"eficompress",
        File2Arg->FullName,
        Status
        );
      Status = EFI_ACCESS_DENIED;
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
        STRING_TOKEN (STR_COMPRESS_CREATE_ERROR),
        HiiCompressHandle,
        L"eficompress",
        File2Arg->FullName,
        Status
        );
      goto Done;
    }
  }
  //
  // Allocate buffers for both files
  //
  SourceSize  = (UINTN) File1Arg->Info->FileSize;

  File1Buffer = AllocatePool (SourceSize);
  if (File1Buffer == NULL) {
    PrintToken (STRING_TOKEN (STR_COMPRESS_OUT_OF_MEM), HiiCompressHandle, L"eficompress");
    Status = EFI_OUT_OF_RESOURCES;
    goto Done;
  }

  Status = File1Arg->Handle->Read (File1Arg->Handle, &SourceSize, File1Buffer);
  if (EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_COMPRESS_READ_ERROR), HiiCompressHandle, L"eficompress", File1Arg->FullName, Status);
    goto Done;
  }

  DestinationSize = SourceSize;
  File2Buffer     = AllocatePool (SourceSize);
  if (File2Buffer == NULL) {
    PrintToken (STRING_TOKEN (STR_COMPRESS_OUT_OF_MEM), HiiCompressHandle, L"eficompress");
    Status = EFI_OUT_OF_RESOURCES;
    goto Done;
  }

  Status = Compress (File1Buffer, (UINT32) SourceSize, File2Buffer, (UINT32 *) &DestinationSize);

  if (Status == EFI_BUFFER_TOO_SMALL) {
    FreePool (File2Buffer);
    File2Buffer = AllocatePool (DestinationSize);
    if (File2Buffer == NULL) {
      PrintToken (STRING_TOKEN (STR_COMPRESS_OUT_OF_MEM), HiiCompressHandle, L"eficompress");
      Status = EFI_OUT_OF_RESOURCES;
      goto Done;
    }

    Status = Compress (File1Buffer, (UINT32) SourceSize, File2Buffer, (UINT32 *) &DestinationSize);
  }

  if (EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_COMPRESS_COMPRESS_ERROR), HiiCompressHandle, L"eficompress", Status);
    goto Done;
  }

  if (SourceSize) {
    Ratio = ((INT32) SourceSize * 100 - (INT32) DestinationSize * 100) / (INT32) SourceSize;
    if (Ratio >= 0) {
      PrintToken (
        STRING_TOKEN (STR_COMPRESS_ORIG_SIZE),
        HiiCompressHandle,
        L"eficompress",
        SourceSize,
        DestinationSize,
        Ratio
        );
    } else {
      PrintToken (
        STRING_TOKEN (STR_COMPRESS_ORIG_SIZE),
        HiiCompressHandle,
        L"eficompress",
        SourceSize,
        DestinationSize,
        -Ratio
        );
    }
  } else {
    PrintToken (
      STRING_TOKEN (STR_COMPRESS_ORIG_SIZE_2),
      HiiCompressHandle,
      L"eficompress",
      SourceSize,
      DestinationSize
      );
  }

  Status = File2Handle->Write (File2Handle, &DestinationSize, File2Buffer);
  if (EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_COMPRESS_WRITE_ERROR), HiiCompressHandle, L"eficompress", File2Arg->FullName, Status);
    goto Done;
  }

  if (!DstFileExist) {
    File2Handle->Close (File2Handle);
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

  //
  // Shell command always succeeds
  //
  LibUnInitializeStrings ();
  LibCheckVarFreeVarList (&ChkPck);
  return Status;
}

EFI_STATUS
InitializeCompressGetLineHelp (
  OUT CHAR16                     **Str
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
  return LibCmdGetStringByToken (STRING_ARRAY_NAME, &EfiCompressGuid, STRING_TOKEN (STR_EFICOMPRESS_LINEHELP), Str);
}
