/*++

Copyright (c) 2005 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  type.c 
  
Abstract:

  EFI  Shell command "type"

Revision History

--*/

#include "EfiShellLib.h"
#include "type.h"

extern UINT8  STRING_ARRAY_NAME[];

//
// This is the generated header file which includes whatever needs to be exported (strings + IFR)
//
#include STRING_DEFINES_FILE

//
//
//
EFI_STATUS
EFIAPI
InitializeType (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  );

EFI_STATUS
TypeFile (
  IN SHELL_FILE_ARG     *Arg
  );

BOOLEAN
iCompare2File (
  IN CHAR16                      *DstFile,
  IN CHAR16                      *SrcFile
  );

//
// Global Variables
//
BOOLEAN         TypeAscii;
BOOLEAN         TypeUnicode;
BOOLEAN         TypeAuto;

EFI_HII_HANDLE  HiiHandle;
EFI_GUID        EfiTypeGuid = EFI_TYPE_GUID;
SHELL_VAR_CHECK_ITEM    TypeCheckList[] = {
  {
    L"-a",
    0x01,
    0x02,
    FlagTypeSingle
  },
  {
    L"-u",
    0x02,
    0x01,
    FlagTypeSingle
  },
  {
    L"-b",
    0x04,
    0,
    FlagTypeSingle
  },
  {
    L"-?",
    0x08,
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
EFI_BOOTSHELL_CODE(
  EFI_APPLICATION_ENTRY_POINT(InitializeType)
)

EFI_STATUS
EFIAPI
InitializeType (
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
  EFI_LIST_ENTRY          FileList;
  EFI_LIST_ENTRY          *Link;
  SHELL_FILE_ARG          *Arg;
  UINTN                   Index;
  BOOLEAN                 IsSameFile;

  SHELL_VAR_CHECK_CODE    RetCode;
  CHAR16                  *Useful;
  SHELL_VAR_CHECK_PACKAGE ChkPck;
  SHELL_ARG_LIST          *Item;

  InitializeListHead (&FileList);
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

  //
  // Register our string package with HII and return the handle to it.
  // If previously registered we will simply receive the handle
  //
  Status = LibInitializeStrings (&HiiHandle, STRING_ARRAY_NAME, &EfiTypeGuid);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (!EFI_PROPER_VERSION (0, 99)) {
    PrintToken (
      STRING_TOKEN (STR_SHELLENV_GNC_COMMAND_NOT_SUPPORT),
      HiiHandle,
      L"type",
      EFI_VERSION_0_99 
      );
    Status = EFI_UNSUPPORTED;
    goto Done;
  }

  RetCode = LibCheckVariables (SI, TypeCheckList, &ChkPck, &Useful);
  if (VarCheckOk != RetCode) {
    switch (RetCode) {
    case VarCheckUnknown:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_UNKNOWN_FLAG), HiiHandle, L"type", Useful);
      break;

    case VarCheckDuplicate:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_DUP_FLAG), HiiHandle, L"type", Useful);
      break;

    case VarCheckConflict:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_FLAG_CONFLICT), HiiHandle, L"type", Useful);
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
  //
  // Out put help.
  //
  if (LibCheckVarGetFlag (&ChkPck, L"-?") != NULL) {
    if (ChkPck.ValueCount > 0 ||
        ChkPck.FlagCount > 2 ||
        (2 == ChkPck.FlagCount && !LibCheckVarGetFlag (&ChkPck, L"-b"))
        ) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiHandle, L"type");
      Status = EFI_INVALID_PARAMETER;
    } else {
      if (IS_OLD_SHELL) {
        PrintToken (STRING_TOKEN (STR_NO_HELP), HiiHandle);
      } else {
        PrintToken (STRING_TOKEN (STR_TYPE_VERBOSE_HELP), HiiHandle);
      }

      Status = EFI_SUCCESS;
    }

    goto Done;
  }

  if (ChkPck.ValueCount == 0) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_FEW), HiiHandle, L"type");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }
  //
  // Scan args for flags
  //
  TypeAscii   = FALSE;
  TypeUnicode = FALSE;
  TypeAuto    = TRUE;
  if (LibCheckVarGetFlag (&ChkPck, L"-a") != NULL) {
    TypeAscii = TRUE;
    TypeAuto  = FALSE;
  } else if (LibCheckVarGetFlag (&ChkPck, L"-u") != NULL) {
    TypeUnicode = TRUE;
    TypeAuto    = FALSE;
  }

  Item = ChkPck.VarList;
  while (NULL != Item) {
    Status = ShellFileMetaArg (Item->VarStr, &FileList);
    if (EFI_ERROR (Status) || IsListEmpty (&FileList)) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_CANNOT_OPEN_FILE), HiiHandle, L"type", Item->VarStr);
      goto Done;
    }

    Item = Item->Next;
  }

  LibCheckVarFreeVarList (&ChkPck);

  //
  // Expand each arg and type each file
  //
  IsSameFile = FALSE;
  for (Link = FileList.Flink; Link != &FileList && SI->RedirArgc; Link = Link->Flink) {
    BREAK_LOOP_ON_ESC ();
    Arg         = CR (Link, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);
    IsSameFile  = FALSE;
    for (Index = 1; Index < SI->RedirArgc; Index += 2) {
      if (!EFI_ERROR (LibCompareFile (Arg->FullName, SI->RedirArgv[Index], &IsSameFile)) && IsSameFile) {
        break;
      }
    }

    if (IsSameFile) {
      break;
    }
  }

  if (!IsSameFile) {
    for (Link = FileList.Flink; Link != &FileList; Link = Link->Flink) {
      BREAK_LOOP_ON_ESC ();
      Arg     = CR (Link, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);
      Status  = TypeFile (Arg);
    }
  } else {
    Status = EFI_REDIRECTION_SAME;
  }

Done:
  ShellFreeFileList (&FileList);
  LibUnInitializeStrings ();
  return Status;
}

EFI_STATUS
TypeFile (
  IN  SHELL_FILE_ARG  *Arg
  )
{
  EFI_STATUS      Status;
  CHAR16          *Buffer;
  CHAR8           *CharBuffer;
  CHAR16          *WideBuffer;
  EFI_FILE_HANDLE Handle;
  UINTN           BufferSize;
  UINTN           Size;
  BOOLEAN         FormatAscii;
  UINTN           Index;
  UINTN           BlockSize;
  UINTN           TotalSize;
  UINTN           PrintSize;

  Status = Arg->Status;
  if (!EFI_ERROR (Status)) {
    if (Arg->Info->Attribute & EFI_FILE_DIRECTORY) {
      PrintToken (STRING_TOKEN (STR_TYPE_TARGET_DIR), HiiHandle, L"type", Arg->FullName);
      return Status;
    }

    Handle = Arg->Handle;
    PrintToken (STRING_TOKEN (STR_TYPE_FILE_SIZE), HiiHandle, L"type", Arg->FullName, Arg->Info->FileSize);

    //
    // Allocate Buffer according to file size
    //
    BufferSize  = (UINTN) Arg->Info->FileSize;
    Buffer      = AllocatePool ((UINTN) Arg->Info->FileSize);

    //
    // if out of resource, allocate 64k buffer
    //
    if (Buffer == NULL) {
      Buffer = AllocatePool (64 * 1024);
      if (Buffer == NULL) {
        PrintToken (STRING_TOKEN (STR_TYPE_OUT_OF_MEM), HiiHandle, L"type");
        return EFI_OUT_OF_RESOURCES;
      }

      BufferSize = 64 * 1024;
    }
    //
    // Unicode files start with a marker of 0xff, 0xfe.  Skip it.
    //
    Size    = 2;
    Status  = Handle->Read (Handle, &Size, Buffer);
    if (Buffer[0] == EFI_UNICODE_BYTE_ORDER_MARK) {
      FormatAscii = FALSE;
      if (TypeAuto) {
        TypeAscii   = FALSE;
        TypeUnicode = TRUE;
      }
    } else {
      FormatAscii = TRUE;
      if (TypeAuto) {
        TypeAscii   = TRUE;
        TypeUnicode = FALSE;
      }

      if (TypeUnicode) {
        PrintToken (STRING_TOKEN (STR_TYPE_TWO_VARS_PTR_S), HiiHandle, Size / sizeof (CHAR16), Buffer);
      } else {
        PrintToken (STRING_TOKEN (STR_TYPE_TWO_VARS_PTR_A), HiiHandle, Size, Buffer);
      }
    }

    TotalSize = 0;
    for (;;) {
      //
      // Break the execution?
      //
      if (GetExecutionBreak ()) {
        Status = EFI_SUCCESS;
        break;
      }

      Size    = BufferSize;
      Status  = Handle->Read (Handle, &Size, Buffer);
      if (EFI_ERROR (Status) || !Size) {
        break;
      }

      TotalSize += Size;
      if (TotalSize > Arg->Info->FileSize) {
        //
        // Prevent inifinite loop when "type abc.txt > abc.txt"
        //
        break;
      }

      WideBuffer  = (CHAR16 *) Buffer;
      CharBuffer  = (CHAR8 *) Buffer;

      //
      // if type unicode file as ascii file, convert '0' to space character
      //
      if (TypeAscii && !FormatAscii) {
        for (Index = 0; Index < Size; Index++) {
          if (CharBuffer[Index] == 0x0d) {
            CharBuffer[Index + 1] = 0x0d;
            Index++;
            continue;
          }

          if (CharBuffer[Index] == 0x0a) {
            CharBuffer[Index + 1] = 0x0a;
            Index++;
            continue;
          }

          if (CharBuffer[Index] == 0) {
            CharBuffer[Index] = 0x20;
          }
        }
      }
      //
      // Type file according to type option, or file format
      //
      FormatAscii = (BOOLEAN) ((TypeAscii || TypeUnicode) ? TypeAscii : FormatAscii);
      if (FormatAscii) {
        Size = Size / sizeof (CHAR8);
      } else {
        Size = Size / sizeof (CHAR16);
      }

      BlockSize = 256;
      PrintSize = 0;
      while (Size) {

        if (GetExecutionBreak ()) {
          break;
        }

        if (Size > BlockSize) {
          Size -= BlockSize;
        } else {
          BlockSize = Size;
          Size      = 0;
        }

        if (FormatAscii) {
          if (*(CharBuffer + BlockSize - 1) == 0x0D && *(CharBuffer + BlockSize) == 0x0A) {
            PrintSize = BlockSize - 1;
            Size++;
          } else {
            PrintSize = BlockSize;
          }

          PrintToken (STRING_TOKEN (STR_TYPE_TWO_VARS_PTR_A), HiiHandle, PrintSize, CharBuffer);
          CharBuffer += PrintSize;
        } else {
          if (*(WideBuffer + BlockSize - 1) == 0x000D && *(WideBuffer + BlockSize) == 0x000A) {
            PrintSize = BlockSize - 1;
            Size++;
          } else {
            PrintSize = BlockSize;
          }

          PrintToken (STRING_TOKEN (STR_TYPE_TWO_VARS_PTR_S), HiiHandle, PrintSize, WideBuffer);
          WideBuffer += PrintSize;
        }
      }
    }

    FreePool (Buffer);
  }

  if (EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_TYPE_TYPE_ERROR), HiiHandle, L"type", Arg->FullName, Status);
  }

  PrintToken (STRING_TOKEN (STR_TYPE_RETURN), HiiHandle);
  return Status;
}

BOOLEAN
iCompare2File (
  IN CHAR16                      *DstFile,
  IN CHAR16                      *SrcFile
  )
/*++
Routine Description:

Arguments:
  DstFile - The dst file
  SrcFile - The source file

Returns:
  TRUE:  Same file
  FALSE: No same file

--*/
{
  EFI_LIST_ENTRY  SrcList;
  EFI_LIST_ENTRY  DstList;
  EFI_STATUS      Status;
  EFI_LIST_ENTRY  *Link;
  SHELL_FILE_ARG  *SrcArg;
  SHELL_FILE_ARG  *DstArg;
  BOOLEAN         CompareResult;

  Link    = NULL;
  SrcArg  = NULL;
  DstArg  = NULL;
  InitializeListHead (&SrcList);
  InitializeListHead (&DstList);

  CompareResult = FALSE;

  Status        = ShellFileMetaArg (DstFile, &DstList);
  if (EFI_ERROR (Status) || IsListEmpty (&DstList)) {
    //
    // STR_CP_CANNOT_OPEN_DEST
    //
    goto Done;
  }

  DstArg = CR (DstList.Flink, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);

  if (SrcFile) {
    Status = ShellFileMetaArg (SrcFile, &SrcList);

    //
    // At this stage, make sure there is at least one valid src
    //
    StrTrim (DstArg->FullName, '.');
    for (Link = SrcList.Flink; Link != &SrcList; Link = Link->Flink) {
      SrcArg = CR (Link, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);
      if (SrcArg->Status == EFI_SUCCESS) {
        StrTrim (SrcArg->FullName, '.');
        if (MetaiMatch (DstArg->FullName, SrcArg->FullName) || MetaiMatch (SrcArg->FullName, DstArg->FullName)) {
          CompareResult = TRUE;
          goto Done;
        }
      }
    }
  }

Done:
  ShellFreeFileList (&SrcList);
  ShellFreeFileList (&DstList);
  return CompareResult;
}

EFI_STATUS
EFIAPI
InitializeTypeGetLineHelp (
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
  return LibCmdGetStringByToken (STRING_ARRAY_NAME, &EfiTypeGuid, STRING_TOKEN (STR_TYPE_LINE_HELP), Str);
}
