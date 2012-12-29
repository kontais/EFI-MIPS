/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  exec.c 
  
Abstract:

Revision History

--*/

#include "shelle.h"
#include "shellenvguid.h"
#include "parsecmd.h"

extern EFI_LIST_ENTRY SEnvCurMapping;

//
// Define the level for the cmdline parsing
//
#define MAX_ALIAS_NESTING_LEVEL       5
#define MAX_RECURSE_NESTING_LEVEL     5
#define MAX_SUBSTISTUE_NESTING_LEVEL  0
  //
  // (0x01 | 0x02 | 0x04 | 0x08 | 0x40 | 0x80 | 0x100 | 0x200)
  //
#define STDOUT_MASK 0x3cf
  //
  // (0x10 | 0x20)
  //
#define STDERR_MASK 0x30

SHELL_VAR_CHECK_ITEM    RedirCheckList[] = {
  {
    L">",
    0x01,
    STDOUT_MASK ^ 0x01,
    FlagTypeNeedVar
  },
  {
    L">a",
    0x02,
    STDOUT_MASK ^ 0x02,
    FlagTypeNeedVar
  },
  {
    L"1>",
    0x04,
    STDOUT_MASK ^ 0x04,
    FlagTypeNeedVar
  },
  {
    L"1>a",
    0x08,
    STDOUT_MASK ^ 0x08,
    FlagTypeNeedVar
  },
  {
    L"2>",
    0x10,
    STDERR_MASK ^ 0x10,
    FlagTypeNeedVar
  },
  {
    L"2>a",
    0x20,
    STDERR_MASK ^ 0x20,
    FlagTypeNeedVar
  },
  {
    L">>",
    0x40,
    STDOUT_MASK ^ 0x40,
    FlagTypeNeedVar
  },
  {
    L">>a",
    0x80,
    STDOUT_MASK ^ 0x80,
    FlagTypeNeedVar
  },
  {
    L"1>>",
    0x100,
    STDOUT_MASK ^ 0x100,
    FlagTypeNeedVar
  },
  {
    L"1>>a",
    0x200,
    STDOUT_MASK ^ 0x200,
    FlagTypeNeedVar
  },
  {
    NULL,
    0,
    0,
    0
  }
};

SHELL_VAR_CHECK_ITEM    ExitCheckList[] = {
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

// Structure to do with substistution
//
typedef struct _SUBSTISTUE_LIST {
  struct _SUBSTISTUE_LIST *Parent;
  CHAR16                  *RetPtr;
} SUBSTISTUE_LIST;

typedef struct {
  CHAR16          **Arg;
  UINTN           ArgIndex;
  UINTN           ArgCount;

  BOOLEAN         Output;
  BOOLEAN         Quote;
  UINTN           AliasLevel;
  UINTN           RecurseLevel;

  CHAR16          *Buffer;
  UINTN           BufferIndex;
  UINTN           BufLength;

  UINTN           SubstituteLevel;
  SUBSTISTUE_LIST *Substitute;

} PARSE_STATE;

typedef struct _SENV_OPEN_DIR {
  struct _SENV_OPEN_DIR *Next;
  EFI_FILE_HANDLE       Handle;
} SENV_OPEN_DIR;

//
// The nesting level of the command executing
//
STATIC UINT32   mCmdNestingLevel = 0;

//
// The nesting level of the shell prompt
//
STATIC UINT32   mShellNestingLevel = 0;

//
// The command execution break flag
//
STATIC BOOLEAN  mExecutionBreak = FALSE;

//
// Internal prototypes
//

/*
EFI_STATUS
ShellParseStr (
  IN  CHAR16                   *Str,
  IN  OUT PARSE_STATE          *ParseState
  );
*/
EFI_STATUS
ShellParseStrOld (
  IN  CHAR16                   *Str,
  IN  OUT PARSE_STATE          *ParseState
  );

EFI_STATUS
ShellAppendBuffer (
  IN  PARSE_STATE              *ParseState,
  IN  UINTN                    Length,
  IN  CHAR16                   *String
  );

EFI_STATUS
SEnvDoExecute (
  IN  EFI_HANDLE               *ParentImageHandle,
  IN  CHAR16                   *CommandLine,
  IN  ENV_SHELL_INTERFACE      *Shell,
  IN  BOOLEAN                  Output
  );

VOID
SEnvLoadImage (
  IN  EFI_HANDLE               ParentImage,
  IN  CHAR16                   *IName,
  OUT EFI_HANDLE               *pImageHandle,
  OUT EFI_FILE_HANDLE          *pScriptsHandle,
  OUT UINT16                   *ImageMachineType,
  OUT BOOLEAN                  *MachineTypeMismatch
  );

//
//  Parser driver function
//
EFI_STATUS
SEnvStringToArg (
  IN  CHAR16                    *Str,
  IN  BOOLEAN                   Output,
  OUT EFI_SHELL_INTERFACE       *ShellInt
  )
/*++

Routine Description:
  Initialize the PARSE_STATE structure and call ShellParseStr to parse a 
  command line into several arguments.
  
Arguments:
  Str                          The command line string to be parsed
  Output                       If error message should be outputed to screen
  ShellInt                     The shell interface
  
Returns:
  EFI_STATUS                   The function finished successfully
  
--*/
{
  PARSE_STATE       ParseState;
  EFI_STATUS        Status;
  SHELL_PARSED_ARGS args;
  SHELL_PARSED_ARGS FinalArgs;
  SHELL_SINGLE_ARG  *arg;
  UINTN             Index;
  UINTN             Round;

  Round = 0;
  //
  // Initialize a new state
  //
  //
  // Parse the string
  //
  ShellInt->EchoOn = TRUE;

  while (*Str == '@' || *Str == ' ') {
    if (*Str == '@') {
      ShellInt->EchoOn = FALSE;
    }

    Str++;
  }

  ZeroMem (&ParseState, sizeof (ParseState));
  Status = EFI_SUCCESS;
  if (IS_OLD_SHELL) {
    ParseState.Output   = Output;
    ParseState.ArgCount = COMMON_ARG_COUNT;
    ParseState.Arg      = AllocateZeroPool (COMMON_ARG_COUNT * sizeof (CHAR16 *));
    if (ParseState.Arg == NULL) {
      Status = EFI_OUT_OF_RESOURCES;
      goto Done;
    }

    ParseState.BufLength  = COMMON_ARG_LENGTH;
    ParseState.Buffer     = AllocateZeroPool (COMMON_ARG_LENGTH * sizeof (CHAR16 *));
    if (ParseState.Buffer == NULL) {
      Status = EFI_OUT_OF_RESOURCES;
      goto Done;
    }

    ShellInt->ArgInfo = AllocateZeroPool (COMMON_ARG_COUNT * sizeof (EFI_SHELL_ARG_INFO));
    if (NULL == ShellInt->ArgInfo) {
      Status = EFI_OUT_OF_RESOURCES;
      goto Done;
    }

    if (!EFI_ERROR (ShellParseStrOld (Str, &ParseState))) {
      ShellInt->Argv  = ParseState.Arg;
      ShellInt->Argc  = ParseState.ArgIndex;
    }
  } else {
    FinalArgs.ArgCount  = 0;
    FinalArgs.ArgList   = NULL;
    do {
      DeleteHeadArg (&FinalArgs);
      Status = ProcessQuotationMarks (Str, &args);
      if (EFI_ERROR (Status)) {
        break;
      }

      Status = ShellVarSubstitution (&args);
      if (EFI_ERROR (Status)) {
        break;
      }

      MoveArgsToHeadOfAnother (&args, &FinalArgs);
      Str = SEnvGetAlias (FinalArgs.ArgList->Result);
      //
      // Round decides the recursive level of alias substitution
      // Current implementation doesn't support recursive
      // substitution for alias.
      //
      Round++;
    } while (Str && Round < 2);

    if (!EFI_ERROR (Status) && FinalArgs.ArgList->Result[0]) {
      arg             = FinalArgs.ArgList;
      ShellInt->Argc  = FinalArgs.ArgCount;
      ShellInt->Argv  = AllocateZeroPool (FinalArgs.ArgCount * sizeof (CHAR16 *));
      if (NULL == ShellInt->Argv) {
        Status = EFI_OUT_OF_RESOURCES;
        goto Done;
      }

      ShellInt->ArgInfo = AllocateZeroPool (FinalArgs.ArgCount * sizeof (EFI_SHELL_ARG_INFO));
      if (NULL == ShellInt->ArgInfo) {
        Status = EFI_OUT_OF_RESOURCES;
        goto Done;
      }

      Index = 0;
      while (!EFI_ERROR (Status) && arg) {
        ShellInt->Argv[Index]               = StrDuplicate (arg->Result);
        ShellInt->ArgInfo[Index].Attributes = arg->Attributes;
        Index++;
        arg = arg->Next;
      }
    }
  }

Done:
  if (IS_OLD_SHELL) {
    if (ParseState.Buffer) {
      FreePool (ParseState.Buffer);
    }

    if (EFI_ERROR (Status)) {
      if (ParseState.Arg) {
        FreePool (ParseState.Arg);
      }
    }
  } else {
    FreeParsedArgs (&args);
    FreeParsedArgs (&FinalArgs);
  }

  return Status;
}

EFI_STATUS
ShellAppendBuffer (
  IN  PARSE_STATE              *ParseState,
  IN  UINTN                    Length,
  IN  CHAR16                   *String
  )
/*++

Routine Description:
  This function appends String to the end of buffer in ParseState. If the
  buffer is not large enough to hold the string, it will be expanded.
  
Arguments:
  ParseState - The parse state
  Length     - The length
  String     - The string

Returns:
  EFI_OUT_OF_RESOURCES  - Out of resources
  EFI_SUCCESS           - Success

--*/
{
  UINTN Index;
  UINTN ExtendLength;

  Index = ParseState->BufferIndex;

  //
  // Check if current buffer can hold the string
  //
  if (Index + Length - 1 >= ParseState->BufLength - 1) {
    ExtendLength = Length > COMMON_ARG_LENGTH ? Length : COMMON_ARG_LENGTH;

    //
    // If cann't hold, allocate more memory
    //
    ParseState->Buffer = ReallocatePool (
                          ParseState->Buffer,
                          ParseState->BufLength * sizeof (CHAR16 *),
                          (ParseState->BufLength + ExtendLength) * sizeof (CHAR16 *)
                          );
    if (ParseState->Buffer == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }

    ZeroMem (
      ParseState->Buffer + ParseState->BufLength,
      ExtendLength * sizeof (CHAR16 *)
      );
    ParseState->BufLength += ExtendLength;
  }
  //
  // Copy the string into the end of the buffer
  //
  CopyMem (&ParseState->Buffer[Index], String, Length * sizeof (CHAR16));
  ParseState->BufferIndex += Length;

  return EFI_SUCCESS;
}

VOID
SEnvFreeArgInfoArray (
  IN EFI_SHELL_INTERFACE      *ShellInt
  )
{
  UINTN Index;

  ASSERT (ShellInt);
  if (ShellInt->ArgInfo != NULL) {
    //
    // Free arguments according to the argument count
    //
    for (Index = 0; Index < ShellInt->Argc; Index++) {
      LibFreeArgInfo (&ShellInt->ArgInfo[Index]);
    }

    FreePool (ShellInt->ArgInfo);
    ShellInt->ArgInfo = NULL;
  }
}

VOID
SEnvFreeArgument (
  IN EFI_SHELL_INTERFACE      *ShellInt
  )
/*++
Routine Description:
  This function frees the parsed arguments.
  
Arguments:
  
  ShellInt    The shell interface
    
Returns:

--*/
{
  UINT32  Index;

  ASSERT (ShellInt);

  if (ShellInt->Argv != NULL) {
    //
    // Free arguments according to the argument count
    //
    for (Index = 0; Index < ShellInt->Argc; Index++) {
      if (ShellInt->Argv[Index] != NULL) {
        FreePool (ShellInt->Argv[Index]);
        ShellInt->Argv[Index] = NULL;
      }
    }

    FreePool (ShellInt->Argv);
    ShellInt->Argv = NULL;
  }

  ShellInt->Argc = 0;
}

VOID
SEnvFreeRedirArgument (
  IN EFI_SHELL_INTERFACE         *ShellInt
  )
{
  UINT32  Index;

  if (ShellInt->RedirArgv != NULL) {
    //
    // Free arguments according to the argument count
    //
    for (Index = 0; Index < ShellInt->RedirArgc; Index++) {
      if (ShellInt->RedirArgv[Index] != NULL) {
        FreePool (ShellInt->RedirArgv[Index]);
        ShellInt->RedirArgv[Index] = NULL;
      }
    }

    FreePool (ShellInt->RedirArgv);
    ShellInt->RedirArgv = NULL;
  }

  ShellInt->RedirArgc = 0;
}

EFI_STATUS
SEnvRedirOutput (
  IN OUT ENV_SHELL_INTERFACE  *Shell,
  IN BOOLEAN                  Ascii,
  IN BOOLEAN                  Append,
  IN OUT UINTN                *Index,
  OUT ENV_SHELL_REDIR_FILE    *Redir
  )
/*++

Routine Description:

Arguments:

  Shell  - The shell interface
  Ascii  - Use the ASCII
  Append - Append
  Index  - The index
  Redir  - The redir file

Returns:
  EFI_INVALID_PARAMETER - Invalid parameter
  EFI_SUCCESS - Success

--*/
{
  CHAR16        *FileName;
  EFI_STATUS    Status;
  EFI_FILE_INFO *Info;
  UINTN         Size;
  CHAR16        UnicodeMarker;
  UINT64        FileMode;
  UINTN         FIndex;

  UnicodeMarker = EFI_UNICODE_BYTE_ORDER_MARK;
  Info          = NULL;
  FIndex        = *Index + 1;

  if (FIndex >= Shell->ShellInt.Argc) {
    return EFI_INVALID_PARAMETER;
  }

  if (Redir->Handle) {
    return EFI_INVALID_PARAMETER;
  }
  //
  // Open the output file
  //
  Redir->Ascii      = Ascii;
  Redir->WriteError = EFI_SUCCESS;
  FileName          = Shell->ShellInt.Argv[FIndex];
  Redir->FilePath   = SEnvNameToPath (FileName);

  if (StriCmp (FileName, L"NUL") != 0) {
    if (Redir->FilePath) {
      FileMode    = EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | ((Append) ? 0 : EFI_FILE_MODE_CREATE);
      Redir->File = LibOpenFilePath (Redir->FilePath, FileMode);
      if (Append && !Redir->File) {
        //
        // If file does not exist make a new one. And send us down the other path
        //
        FileMode |= EFI_FILE_MODE_CREATE;
        Redir->File = LibOpenFilePath (Redir->FilePath, FileMode);
        Append      = FALSE;
      }
    }

    if (Redir->File) {
      Info = LibGetFileInfo (Redir->File);
      ASSERT (Info);
      if (Info->Attribute & EFI_FILE_DIRECTORY) {
        PrintToken (STRING_TOKEN (STR_SHELLENV_EXEC_DIRNOTALLOWED), HiiEnvHandle, FileName);
        goto Done;
      }
    }

    if (!Redir->File) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_EXEC_COULD_NOT_OPEN_OUTPUT), HiiEnvHandle, FileName);
      goto Done;
    }

    if (Append) {
      if (Info) {
        FreePool (Info);
      }

      Size = sizeof (UnicodeMarker);
      Redir->File->Read (Redir->File, &Size, &UnicodeMarker);
      if ((UnicodeMarker == EFI_UNICODE_BYTE_ORDER_MARK) && Ascii) {
        PrintToken (STRING_TOKEN (STR_SHELLENV_EXEC_COULD_NOT_OPEN_ASCII), HiiEnvHandle, FileName);
        if (Redir->FilePath) {
          FreePool (Redir->FilePath);
        }

        Redir->FilePath = NULL;
        return EFI_INVALID_PARAMETER;
      } else if ((UnicodeMarker != EFI_UNICODE_BYTE_ORDER_MARK) && !Ascii) {
        PrintToken (STRING_TOKEN (STR_SHELLENV_EXEC_COULD_NOT_APPEND_UNICODE), HiiEnvHandle, FileName);
        if (Redir->FilePath) {
          FreePool (Redir->FilePath);
        }

        Redir->FilePath = NULL;
        return EFI_INVALID_PARAMETER;
      }
      //
      // Seek to end of the file
      //
      Redir->File->SetPosition (Redir->File, (UINT64) -1);
    } else {
      //
      // Truncate the file
      //
      Info->FileSize  = 0;
      Size            = SIZE_OF_EFI_FILE_INFO + StrSize (Info->FileName);
      if (Redir->File->SetInfo) {
        Redir->File->SetInfo (Redir->File, &gEfiFileInfoGuid, Size, Info);
      } else {
        DEBUG (
          (EFI_D_ERROR,
          "SEnvRedirOutput: SetInfo in file system driver not complete\n")
          );
      }

      FreePool (Info);

      if (!Ascii) {
        Size = sizeof (UnicodeMarker);
        Redir->File->Write (Redir->File, &Size, &UnicodeMarker);
      }
    }
    //
    // Allocate a new handle
    //
    CopyMem (&Redir->Out, &SEnvConToIo, sizeof (EFI_SIMPLE_TEXT_OUT_PROTOCOL));

  } else {
    CopyMem (
      &Redir->Out,
      &SEnvDummyConToIo,
      sizeof (EFI_SIMPLE_TEXT_OUT_PROTOCOL)
      );
    Redir->FilePath = LibEndDevicePath;
  }

  Status = LibInstallProtocolInterfaces (
            &Redir->Handle,
            &gEfiSimpleTextOutProtocolGuid,
            &Redir->Out,
            &gEfiDevicePathProtocolGuid,
            Redir->FilePath,
            NULL
            );
  Redir->Signature = ENV_REDIR_SIGNATURE;
  ASSERT (!EFI_ERROR (Status));

  return EFI_SUCCESS;

Done:
  if (Redir->FilePath) {
    FreePool (Redir->FilePath);
  }

  Redir->FilePath = NULL;
  return EFI_INVALID_PARAMETER;
}

SHELL_VAR_CHECK_CODE
_CheckRedirecSyntax (
  IN VOID                            *si,
  IN SHELL_VAR_CHECK_ITEM            *CheckList,
  OUT SHELL_VAR_CHECK_PACKAGE        *ChkPck,
  OUT CHAR16                         **Useful
  )
/*++
Routine Description:

Arguments:

  si - The shell interface
  CheckList - The checklist
  ChkPck - Check package
  Useful - The useful infomation
  
Returns:

--*/
{
  UINTN               Index;
  UINTN               Item;
  UINT32              Mask;
  SHELL_ARG_LIST      **p;
  BOOLEAN             QuoteVar;
  EFI_SHELL_INTERFACE *ShellInt;
  EFI_SHELL_ARG_INFO  *ArgInfo;

  ShellInt            = (EFI_SHELL_INTERFACE *) si;
  Mask                = 0;
  ChkPck->FlagCount   = 0;
  ChkPck->ValueCount  = 0;
  QuoteVar            = FALSE;
  p                   = NULL;
  //
  // ArgInfo array contains associated properties of each argument
  //
  ArgInfo = ShellInt->ArgInfo;
  for (Index = 1; Index < ShellInt->Argc; Index++) {
    if (ArgInfo) {
      //
      // If an argument was totally quoted on the command line,
      // we don't think it is possible to be a flag
      //
      QuoteVar = (BOOLEAN) ((ArgInfo[Index].Attributes & ARG_IS_QUOTED) && !(ArgInfo[Index].Attributes & ARG_PARTIALLY_QUOTED));
    }

    if (QuoteVar ||
    //
    // If an argument has the attribute of ARG_FIRST_CHAR_IS_ESC,
    // we don't think it is possible to be a flag.
    //
    (ArgInfo && (ArgInfo[Index].Attributes & ARG_FIRST_CHAR_IS_ESC)) ||
    //
    // If an argument's first-half was in quotation,
    // we don't think it is possible to be a flag.
    //
    (ArgInfo && (ArgInfo[Index].Attributes & ARG_FIRST_HALF_QUOTED)) ||
    //
    // If there is '>' in this argument, it's supposed to be a redirection
    // operator.
    //
    !StrChr (ShellInt->Argv[Index], '>')) {
      //
      // Find the last element in the VarList, then allocate a new element
      // to store the current argument.
      //
      p = &ChkPck->VarList;
      while (*p) {
        p = &((*p)->Next);
      }

      *p = (SHELL_ARG_LIST *) AllocateZeroPool (sizeof (SHELL_ARG_LIST));
      if (NULL == *p) {
        return VarCheckOutOfMem;
      }
      (*p)->VarStr  = ShellInt->Argv[Index];
      (*p)->Index   = Index;
      ChkPck->ValueCount++;
      continue;
    }
    //
    // We got here, the current argument will be taken as a flag.
    //
    for (Item = 0; CheckList[Item].FlagStr; Item++) {
      ASSERT (FlagTypeNeedVar == CheckList[Item].FlagType);
      if (StriCmp (ShellInt->Argv[Index], CheckList[Item].FlagStr)) {
        continue;
      }
      //
      // check conflict
      //
      if (Mask & CheckList[Item].ConflictMask) {
        *Useful = ShellInt->Argv[Index];
        return VarCheckConflict;
      }
      //
      // check duplicate
      //
      p = &ChkPck->FlagList;
      while (*p) {
        if (0 == StriCmp (ShellInt->Argv[Index], (*p)->FlagStr)) {
          *Useful = ShellInt->Argv[Index];
          return VarCheckDuplicate;
        }

        p = &(*p)->Next;
      }

      *p = AllocateZeroPool (sizeof (SHELL_ARG_LIST));
      if (NULL == *p) {
        return VarCheckOutOfMem;
      }
      (*p)->Index = Index;

      switch (CheckList[Item].FlagType) {
      case FlagTypeNeedVar:
        if (Index < ShellInt->Argc - 1) {
          (*p)->FlagStr = ShellInt->Argv[Index];
          (*p)->VarStr  = ShellInt->Argv[Index + 1];
          Index++;
        } else {
          *Useful = ShellInt->Argv[Index];
          return VarCheckLackValue;
        }
        break;

      default:
        ASSERT (FALSE);
      }

      Mask |= CheckList[Item].FlagID;
      ChkPck->FlagCount++;
      break;
    }

    if (NULL == CheckList[Item].FlagStr) {
      *Useful = ShellInt->Argv[Index];
      return VarCheckUnknown;
    }
  }

  return VarCheckOk;
}

EFI_STATUS
_IsTheSameFile (
  IN     CHAR16             *StdFileName,
  IN     CHAR16             *ErrFileName,
  OUT BOOLEAN               *TheSame
  )
{
  EFI_LIST_ENTRY  StdFileList;
  EFI_LIST_ENTRY  ErrFileList;
  EFI_STATUS      Status;
  SHELL_FILE_ARG  *StdFile;
  SHELL_FILE_ARG  *ErrFile;
  CHAR16          *StdFs;
  CHAR16          *ErrFs;
  CHAR16          *StdPath;
  CHAR16          *ErrPath;

  StdFile   = NULL;
  ErrFile   = NULL;
  StdFs     = NULL;
  ErrFs     = NULL;
  StdPath   = NULL;
  ErrPath   = NULL;
  *TheSame  = TRUE;

  InitializeListHead (&StdFileList);
  InitializeListHead (&ErrFileList);

  Status = ShellFileMetaArg (StdFileName, &StdFileList);
  if (EFI_ERROR (Status)) {
    goto Done;
  }

  if (IsListEmpty (&StdFileList) || StdFileList.Flink->Flink != &StdFileList) {
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  StdFile = CR (StdFileList.Flink, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);
  if (EFI_ERROR (StdFile->Status) && NULL == StdFile->Parent) {
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  Status = ShellFileMetaArg (ErrFileName, &ErrFileList);
  if (EFI_ERROR (Status)) {
    goto Done;
  }

  if (IsListEmpty (&ErrFileList) || ErrFileList.Flink->Flink != &ErrFileList) {
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  ErrFile = CR (ErrFileList.Flink, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);
  if (EFI_ERROR (ErrFile->Status) && NULL == ErrFile->Parent) {
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  Status = LibSplitFsAndPath (
            StdFile->FullName,
            &StdFs,
            &StdPath
            );
  if (EFI_ERROR (Status)) {
    goto Done;
  }

  Status = LibSplitFsAndPath (
            ErrFile->FullName,
            &ErrFs,
            &ErrPath
            );
  if (EFI_ERROR (Status)) {
    goto Done;
  }

  if (!CompareFsDevice (StdFs, ErrFs)) {
    *TheSame = FALSE;
    goto Done;
  }

  if (StriCmp (StdPath, ErrPath)) {
    *TheSame = FALSE;
    goto Done;
  }

Done:
  ShellFreeFileList (&StdFileList);
  ShellFreeFileList (&ErrFileList);
  FreePool (StdFs);
  FreePool (ErrFs);
  FreePool (StdPath);
  FreePool (ErrPath);

  return Status;
}

EFI_STATUS
SEnvExecRedir (
  IN OUT ENV_SHELL_INTERFACE  *Shell
  )
/*++

Routine Description:

Arguments:

  Shell - Shell interface

Returns:

  EFI_INVALID_PARAMETER  - Invalid Parameter
  
--*/
{
  CHAR16                **Argv;
  UINTN                 ErrIndex;
  UINTN                 StdIndex;
  UINTN                 Index;
  UINTN                 RedirIndex;
  EFI_STATUS            Status;
  BOOLEAN               ErrAscii;
  BOOLEAN               StdAscii;
  BOOLEAN               ErrAppend;
  BOOLEAN               StdAppend;
  EFI_SYSTEM_TABLE      *SysTable;
  CHAR16                *Useful;
  SHELL_ARG_LIST        *Item;
  SHELL_ARG_LIST        *tmpItem;
  SHELL_VAR_CHECK_CODE  RetCode;
  EFI_SHELL_ARG_INFO    *ArgInfo;
  CHAR16                *ErrRedirFile;
  CHAR16                *StdRedirFile;
  BOOLEAN               TheSame;
  SHELL_VAR_CHECK_PACKAGE ChkPck;

  TheSame                   = FALSE;
  ErrRedirFile              = NULL;
  StdRedirFile              = NULL;
  Status                    = EFI_SUCCESS;
  ErrIndex                  = 0;
  StdIndex                  = 0;
  ErrAscii                  = FALSE;
  StdAscii                  = FALSE;
  ErrAppend                 = FALSE;
  StdAppend                 = FALSE;
  Useful                    = NULL;
  Argv                      = NULL;
  ArgInfo                   = NULL;
  Shell->ShellInt.RedirArgv = NULL;
  ZeroMem (&ChkPck, sizeof (SHELL_VAR_CHECK_PACKAGE));
  SysTable = Shell->SystemTable;

  //
  // Initializing Append to aVOID a level 4 warning
  //
  RetCode = _CheckRedirecSyntax (&Shell->ShellInt, RedirCheckList, &ChkPck, &Useful);
  if (VarCheckOk != RetCode) {
    switch (RetCode) {
    case VarCheckConflict:
      PrintToken (STRING_TOKEN (STR_SHELLENV_REDIR_CONLICT), HiiEnvHandle, Useful);
      break;

    case VarCheckDuplicate:
      PrintToken (STRING_TOKEN (STR_SHELLENV_REDIR_MULTIPLE), HiiEnvHandle, Useful);
      break;

    case VarCheckLackValue:
      Shell->ShellInt.RedirArgc = 0;
      Shell->ShellInt.RedirArgv = NULL;
      PrintToken (STRING_TOKEN (STR_SHELLENV_REDIR_LACK_VALUE), HiiEnvHandle, Useful);
      break;

    case VarCheckUnknown:
      PrintToken (STRING_TOKEN (STR_SHELLENV_REDIR_INCORRECT_SYNTAX), HiiEnvHandle, Useful);
      break;

    default:
      break;
    }

    LibCheckVarFreeVarList (&ChkPck);
    return EFI_INVALID_PARAMETER;
  }

  Item = GetFirstFlag (&ChkPck);
  //
  // Make sure that there is only one argument following redirection operator
  //
  tmpItem = GetFirstArg (&ChkPck);
  while (tmpItem && Item) {
    if (tmpItem->Index > Item->Index) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiEnvHandle, L"Shell", tmpItem->VarStr);
      LibCheckVarFreeVarList (&ChkPck);
      return EFI_INVALID_PARAMETER;
    }

    tmpItem = GetNextArg (tmpItem);
  }
  //
  // Check if the std output redirection file is the same as err output redirection file
  //
  while (Item) {
    if (StrSubCmp (L"2>", Item->FlagStr, 2)) {
      ErrRedirFile  = Item->VarStr;
      ErrIndex      = Item->Index;

      if (StrSubCmp (L">a", Item->FlagStr, 2)) {
        ErrAscii = TRUE;
      }

      if (StrSubCmp (L">>", Item->FlagStr, 2)) {
        ErrAppend = TRUE;
      }
    } else {
      StdRedirFile  = Item->VarStr;
      StdIndex      = Item->Index;

      if (StrSubCmp (L">a", Item->FlagStr, 2)) {
        StdAscii = TRUE;
      }

      if (StrSubCmp (L">>", Item->FlagStr, 2)) {
        StdAppend = TRUE;
      }
    }

    Item = GetNextArg (Item);
  }

  if (NULL != ErrRedirFile && NULL != StdRedirFile) {
    Status = _IsTheSameFile (StdRedirFile, ErrRedirFile, &TheSame);
    if (EFI_ERROR (Status)) {
      goto Done;
    }

    if (TheSame) {
      PrintToken (
        STRING_TOKEN (STR_SHELLENV_REDIR_CANNOT_BE_SAME),
        HiiEnvHandle,
        StdRedirFile,
        ErrRedirFile
        );
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }
  }

  if (NULL != StdRedirFile) {
    Status = SEnvRedirOutput (
              Shell,
              StdAscii,
              StdAppend,
              &StdIndex,
              &Shell->StdOut
              );
    if (EFI_ERROR (Status)) {
      goto Done;
    }

    SysTable->ConOut            = &Shell->StdOut.Out;
    SysTable->ConsoleOutHandle  = Shell->StdOut.Handle;
    Shell->ShellInt.StdOut      = Shell->StdOut.File;
  }

  if (NULL != ErrRedirFile) {
    Status = SEnvRedirOutput (
              Shell,
              ErrAscii,
              ErrAppend,
              &ErrIndex,
              &Shell->StdErr
              );
    if (EFI_ERROR (Status)) {
      goto Done;
    }

    SysTable->StdErr              = &Shell->StdErr.Out;
    SysTable->StandardErrorHandle = Shell->StdErr.Handle;
    Shell->ShellInt.StdErr        = Shell->StdErr.File;
  }
  //
  //  Strip redirection args from arglist, saving in RedirArgv so they can be
  //  echoed in batch scripts.
  //
  if (ErrIndex || StdIndex) {
    Shell->ShellInt.RedirArgc = (ErrIndex) ? 2 : 0;
    Shell->ShellInt.RedirArgc += ((StdIndex) ? 2 : 0);
    Shell->ShellInt.RedirArgv = AllocateZeroPool (Shell->ShellInt.RedirArgc * sizeof (CHAR16 *));
    if (!Shell->ShellInt.RedirArgv) {
      Status = EFI_OUT_OF_RESOURCES;
      goto Done;
    }

    RedirIndex = 0;
    if (StdIndex) {
      Shell->ShellInt.RedirArgv[RedirIndex] = Shell->ShellInt.Argv[StdIndex];
      Shell->ShellInt.Argv[StdIndex]        = NULL;
      RedirIndex++;
      Shell->ShellInt.RedirArgv[RedirIndex] = Shell->ShellInt.Argv[StdIndex + 1];
      Shell->ShellInt.Argv[StdIndex + 1]    = NULL;
      RedirIndex++;
    }

    if (ErrIndex) {
      Shell->ShellInt.RedirArgv[RedirIndex] = Shell->ShellInt.Argv[ErrIndex];
      Shell->ShellInt.Argv[ErrIndex]        = NULL;
      RedirIndex++;
      Shell->ShellInt.RedirArgv[RedirIndex] = Shell->ShellInt.Argv[ErrIndex + 1];
      Shell->ShellInt.Argv[ErrIndex + 1]    = NULL;
      RedirIndex++;
    }

    Argv = AllocateZeroPool ((Shell->ShellInt.Argc - Shell->ShellInt.RedirArgc) * sizeof (CHAR16 *));
    if (NULL == Argv) {
      Status = EFI_OUT_OF_RESOURCES;
      goto Done;
    }

    ArgInfo = AllocateZeroPool ((Shell->ShellInt.Argc - Shell->ShellInt.RedirArgc) * sizeof (EFI_SHELL_ARG_INFO));
    if (NULL == ArgInfo) {
      Status = EFI_OUT_OF_RESOURCES;
      goto Done;
    }

    StdIndex = 0;
    for (Index = 0; Index < Shell->ShellInt.Argc; Index++) {
      if (NULL == Shell->ShellInt.Argv[Index]) {
        continue;
      }

      Argv[StdIndex] = Shell->ShellInt.Argv[Index];
      CopyMem (&ArgInfo[StdIndex], &Shell->ShellInt.ArgInfo[Index], sizeof (EFI_SHELL_ARG_INFO));
      StdIndex++;
    }

    FreePool (Shell->ShellInt.Argv);
    FreePool (Shell->ShellInt.ArgInfo);
    Shell->ShellInt.Argv    = Argv;
    Shell->ShellInt.ArgInfo = ArgInfo;
    Shell->ShellInt.Argc -= Shell->ShellInt.RedirArgc;
    Argv    = NULL;
    ArgInfo = NULL;
  } else {
    Shell->ShellInt.RedirArgc = 0;
    Shell->ShellInt.RedirArgv = NULL;
  }

Done:
  LibCheckVarFreeVarList (&ChkPck);

  if (EFI_ERROR (Status) && Shell->ShellInt.RedirArgv) {
    FreePool (Shell->ShellInt.RedirArgv);
  }

  if (Argv) {
    FreePool (Argv);
  }

  if (ArgInfo) {
    FreePool (ArgInfo);
  }

  return Status;
}

VOID
SEnvCloseRedir (
  IN OUT ENV_SHELL_REDIR_FILE    *Redir
  )
/*++

Routine Description:

Arguments:

  Redir - The redirect file

Returns:

--*/
{
  //
  // Close redirection file
  //
  if (Redir->File) {
    Redir->File->Close (Redir->File);
    Redir->File = NULL;
  }
  //
  // Uninstall related protocol interfaces
  //
  if (Redir->Handle) {
    BS->UninstallProtocolInterface (
          Redir->Handle,
          &gEfiSimpleTextOutProtocolGuid,
          &Redir->Out
          );
    BS->UninstallProtocolInterface (
          Redir->Handle,
          &gEfiSimpleTextInProtocolGuid,
          &Redir->In
          );
    BS->UninstallProtocolInterface (
          Redir->Handle,
          &gEfiDevicePathProtocolGuid,
          Redir->FilePath
          );
    if (Redir->FilePath != LibEndDevicePath) {
      FreePool (Redir->FilePath);
    }

    Redir->Handle = NULL;
  }
}

EFI_STATUS
SEnvDoExecute (
  IN EFI_HANDLE           *ParentImageHandle,
  IN CHAR16               *CommandLine,
  IN ENV_SHELL_INTERFACE  *Shell,
  IN BOOLEAN              Output
  )
/*++

Routine Description:

Arguments:

  ParentImageHandle - The parent image handle
  CommandLine       - The command line
  Shell             - The shell interface
  Output            - The output

Returns:

--*/
{
  EFI_SHELL_INTERFACE           *ParentShell;
  EFI_SYSTEM_TABLE              *ParentSystemTable;
  EFI_STATUS                    Status;
  UINTN                         Index;
  SHELLENV_INTERNAL_COMMAND     InternalCommand;
  EFI_HANDLE                    NewImage;
  EFI_FILE_HANDLE               Script;
  BOOLEAN                       ShowHelp;
  BOOLEAN                       ConsoleContextSaved;
  EFI_HANDLE                    SavedConsoleInHandle;
  EFI_HANDLE                    SavedConsoleOutHandle;
  EFI_HANDLE                    SavedStandardErrorHandle;
  EFI_SIMPLE_TEXT_IN_PROTOCOL   *SavedConIn;
  EFI_SIMPLE_TEXT_OUT_PROTOCOL  *SavedConOut;
  EFI_SIMPLE_TEXT_OUT_PROTOCOL  *SavedStdErr;
  EFI_HANDLE                    SavedRedirConsoleInHandle;
  EFI_HANDLE                    SavedRedirConsoleOutHandle;
  EFI_HANDLE                    SavedRedirStandardErrorHandle;
  CHAR16                        *CurrentDir;
  CHAR16                        *OptionsBuffer;
  UINT32                        OptionsSize;
  UINT16                        MachineType;
  BOOLEAN                       MachineTypeMismatch;

  //
  // Initializing Status to aVOID a level 4 warning
  //
  Status                        = EFI_SUCCESS;
  ParentShell                   = NULL;
  ShowHelp                      = FALSE;
  ConsoleContextSaved           = FALSE;
  SavedConIn                    = NULL;
  SavedConOut                   = NULL;
  SavedStdErr                   = NULL;
  SavedConsoleInHandle          = NULL;
  SavedConsoleOutHandle         = NULL;
  SavedStandardErrorHandle      = NULL;
  SavedRedirConsoleInHandle     = NULL;
  SavedRedirConsoleOutHandle    = NULL;
  SavedRedirStandardErrorHandle = NULL;

  //
  // Switch output attribute to normal
  //
  PrintToken (STRING_TOKEN (STR_SHELLENV_EXEC_ONE_VAR_N), HiiEnvHandle);

  //
  //  Check that there is something to do
  //
  if (Shell->ShellInt.Argc < 1) {
    goto Done;
  }
  //
  // Assume some defaults
  //
  BS->HandleProtocol (
        ParentImageHandle,
        &gEfiLoadedImageProtocolGuid,
        (VOID *) &Shell->ShellInt.Info
        );
  Shell->ShellInt.ImageHandle = ParentImageHandle;
  Shell->ShellInt.StdIn       = &SEnvIOFromCon;
  Shell->ShellInt.StdOut      = &SEnvIOFromCon;
  Shell->ShellInt.StdErr      = &SEnvErrIOFromCon;

  //
  // Get parent's image stdout & stdin
  //
  Status = BS->HandleProtocol (
                ParentImageHandle,
                &ShellInterfaceProtocol,
                (VOID *) &ParentShell
                );
  if (EFI_ERROR (Status)) {
    goto Done;
  }

  ParentSystemTable       = ParentShell->Info->SystemTable;
  Shell->ShellInt.StdIn   = ParentShell->StdIn;
  Shell->ShellInt.StdOut  = ParentShell->StdOut;
  Shell->ShellInt.StdErr  = ParentShell->StdErr;

  Shell->SystemTable      = Shell->ShellInt.Info->SystemTable;

  //
  // If the SystemTable is not NULL, then save the current console
  // into local variables
  //
  if (Shell->SystemTable) {
    SavedConIn                = Shell->SystemTable->ConIn;
    SavedConOut               = Shell->SystemTable->ConOut;
    SavedStdErr               = Shell->SystemTable->StdErr;
    SavedConsoleInHandle      = Shell->SystemTable->ConsoleInHandle;
    SavedConsoleOutHandle     = Shell->SystemTable->ConsoleOutHandle;
    SavedStandardErrorHandle  = Shell->SystemTable->StandardErrorHandle;
    ConsoleContextSaved       = TRUE;
  }

  Status = SEnvBatchEchoCommand (Shell);
  if (EFI_ERROR (Status)) {
    goto Done;
  }

  Status = SEnvExecRedir (Shell);
  SetCrc (&Shell->SystemTable->Hdr);

  SavedRedirConsoleInHandle     = Shell->SystemTable->ConsoleInHandle;
  SavedRedirConsoleOutHandle    = Shell->SystemTable->ConsoleOutHandle;
  SavedRedirStandardErrorHandle = Shell->SystemTable->StandardErrorHandle;

  if (EFI_ERROR (Status)) {
    goto Done;
  }
  //
  // Handle special case of the internal "set default device command"
  // Is it one argument that ends with a ":"?
  //
  Index = StrLen (Shell->ShellInt.Argv[0]);
  if (Shell->ShellInt.Argc == 1 && Shell->ShellInt.Argv[0][0] != ':' && Shell->ShellInt.Argv[0][Index - 1] == ':') {
    Status = SEnvSetCurrentDevice (Shell->ShellInt.Argv[0]);
    if (EFI_ERROR (Status)) {
      PrintToken (
        STRING_TOKEN (STR_SHELLENV_EXEC_INVLAID_MAPPING_NAME),
        HiiEnvHandle,
        Shell->ShellInt.Argv[0]
        );
    }

    goto Done;
  }
  //
  // Attempt to dispatch it as an internal command
  //
  InternalCommand = SEnvGetCmdDispath (Shell->ShellInt.Argv[0]);
  if (InternalCommand) {
    //
    // Push & replace the current shell info on the parent image handle.
    // (note we are using the parent image's loaded image information structure)
    //
    BS->ReinstallProtocolInterface (
          ParentImageHandle,
          &ShellInterfaceProtocol,
          ParentShell,
          &Shell->ShellInt
          );
    ParentShell->Info->SystemTable = Shell->SystemTable;

    EFI_SHELL_APP_INIT (ParentImageHandle, Shell->SystemTable);
    //
    // Dispatch the command
    //
    Status = InternalCommand (
              ParentImageHandle,
              Shell->ShellInt.Info->SystemTable
              );

    //
    // Restore the parent's image handle shell info
    //
    BS->ReinstallProtocolInterface (
          ParentImageHandle,
          &ShellInterfaceProtocol,
          &Shell->ShellInt,
          ParentShell
          );
    ParentShell->Info->SystemTable = ParentSystemTable;
    EFI_SHELL_APP_INIT (ParentImageHandle, ParentSystemTable);
    goto Done;
  }
  //
  // Load the app, or open the script
  //
  SEnvLoadImage (ParentImageHandle, Shell->ShellInt.Argv[0], &NewImage, &Script, &MachineType, &MachineTypeMismatch);
  if (!NewImage && !Script) {
    if (Output) {
      if (MachineTypeMismatch == FALSE) {
        PrintToken (STRING_TOKEN (STR_SHELLENV_EXEC_NOT_FOUND), HiiEnvHandle, Shell->ShellInt.Argv[0]);
      } else {
        //
        // Image is not loaded successfully because of the machine type mismatch.
        // Print the error info.
        //
        PrintToken (
          STRING_TOKEN (STR_SHELLENV_EXEC_IMAGE_TYPE_UNSUPPORTED),
          HiiEnvHandle,
          LibGetMachineTypeString (MachineType),
          LibGetMachineTypeString (EFI_IMAGE_MACHINE_TYPE)
          );
      }
    }
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  if (NewImage) {
    //
    // Put the shell info on the handle
    //
    BS->HandleProtocol (
          NewImage,
          &gEfiLoadedImageProtocolGuid,
          (VOID *) &Shell->ShellInt.Info
          );

    LibInstallProtocolInterfaces (
      &NewImage,
      &ShellInterfaceProtocol,
      &Shell->ShellInt,
      NULL
      );

    //
    // Create load options which may include command line and current
    // working directory
    //
    CurrentDir = SEnvGetCurDir (NULL);
    for (Index = OptionsSize = 0; Index < Shell->ShellInt.Argc; Index++) {
      //
      // StrSize includes NULL which we need for space between args
      //
      OptionsSize += (UINT32) StrSize (Shell->ShellInt.Argv[Index]);
    }

    OptionsSize += sizeof (CHAR16);

    if (CurrentDir) {
      OptionsSize += (UINT32) StrSize (CurrentDir);
      //
      // StrSize includes NULL
      //
    }

    OptionsBuffer = AllocateZeroPool (OptionsSize);

    if (OptionsBuffer) {
      //
      // Set the buffer before we manipulate it.
      //
      Shell->ShellInt.Info->LoadOptions     = OptionsBuffer;
      Shell->ShellInt.Info->LoadOptionsSize = OptionsSize;

      //
      // Copy the command line and current working directory
      //
      for (Index = 0; Index < Shell->ShellInt.Argc; Index++) {
        StrCat (OptionsBuffer, Shell->ShellInt.Argv[Index]);
        StrCat (OptionsBuffer, L" ");
      }

      if (CurrentDir) {
        StrCpy (&OptionsBuffer[StrLen (OptionsBuffer) + 1], CurrentDir);
      }
    } else {
      //
      // Fail semi-gracefully (i.e. no command line expansion)
      //
      Shell->ShellInt.Info->LoadOptions     = CommandLine;
      Shell->ShellInt.Info->LoadOptionsSize = (UINT32) StrSize (CommandLine);
    }
    //
    // Pass a copy of the system table with new input & outputs
    //
    Shell->ShellInt.Info->SystemTable = Shell->SystemTable;

    //
    // If the image is an app start it, else abort it
    //
    if (Shell->ShellInt.Info->ImageCodeType == EfiLoaderCode) {
      EFI_SHELL_APP_INIT (ParentImageHandle, Shell->SystemTable);
      Status = BS->StartImage (NewImage, NULL, NULL);
    } else {
      PrintToken (STRING_TOKEN (STR_SHELLENV_EXEC_IMAGE_NOT_APP), HiiEnvHandle);
      BS->Exit (NewImage, EFI_INVALID_PARAMETER, 0, NULL);
      Status = EFI_INVALID_PARAMETER;
    }
    //
    // App has exited, remove our data from the image handle
    //
    if (CurrentDir) {
      FreePool (CurrentDir);
    }

    if (OptionsBuffer) {
      FreePool (OptionsBuffer);
    }

    BS->UninstallProtocolInterface (
          NewImage,
          &ShellInterfaceProtocol,
          &Shell->ShellInt
          );

    EFI_SHELL_APP_INIT (ParentImageHandle, ParentSystemTable);
    if (-2 == Status) {
      //
      // if status = -2 we assume that a nested shell has just exited.
      //
      SEnvDisableExecutionBreak ();
      if (IS_OLD_SHELL) {
        SEnvExecute (ParentImageHandle, L"@map -r", FALSE);
      } else {
        SEnvExecute (ParentImageHandle, L"@map -r -f", FALSE);
      }

      Status = EFI_SUCCESS;
      EFI_NT_EMULATOR_CODE (
        //
        // In NT32, all the individual images share one single DLL,
        // the cleanup of nested child might damage the parent's
        // global data.
        //
        // here we assign a special value to indicate this situation
        // only for NT32
        //
        Status = (EFI_STATUS) -2;
      )
    }

  } else if (Script) {
    //
    // Push & replace the current shell info on the parent image handle.
    // (note we are using the parent image's loaded image information structure)
    //
    BS->ReinstallProtocolInterface (
          ParentImageHandle,
          &ShellInterfaceProtocol,
          ParentShell,
          &Shell->ShellInt
          );
    ParentShell->Info->SystemTable  = Shell->SystemTable;

    Status                          = SEnvExecuteScript (Shell, Script);

    //
    // Restore the parent's image handle shell info
    //
    BS->ReinstallProtocolInterface (
          ParentImageHandle,
          &ShellInterfaceProtocol,
          &Shell->ShellInt,
          ParentShell
          );
    ParentShell->Info->SystemTable = ParentSystemTable;
    EFI_SHELL_APP_INIT (ParentImageHandle, ParentSystemTable);
  }

Done:
  DEBUG_CODE (
    if (EFI_ERROR (Status) && Output) {
      if ((Status == -1) || (Status == -2) || (Status == EFI_REDIRECTION_NOT_ALLOWED)
          || (Status == EFI_REDIRECTION_SAME)) {
      } else {
        PrintToken (STRING_TOKEN (STR_SHELLENV_EXEC_EXIT_STATUS_CODE), HiiEnvHandle, Status);
      }
    }
  )
  //
  // Cleanup
  //
  if (Shell) {
    //
    // If there's an arg list, free it
    //
    SEnvFreeArgument (&Shell->ShellInt);
    SEnvFreeArgInfoArray (&Shell->ShellInt);

    //
    //  If any redirection arguments were saved, free them
    //
    SEnvFreeRedirArgument (&Shell->ShellInt);

    //
    // Close any file redirection
    //
    SEnvCloseRedir (&Shell->StdOut);
    SEnvCloseRedir (&Shell->StdErr);
    SEnvCloseRedir (&Shell->StdIn);

    //
    // If the SystemTable is not NULL, then restore the current console into
    // local variables
    //
    if (ConsoleContextSaved) {
      if (SavedRedirConsoleInHandle == Shell->SystemTable->ConsoleInHandle) {
        Shell->SystemTable->ConIn           = SavedConIn;
        Shell->SystemTable->ConsoleInHandle = SavedConsoleInHandle;
      }

      if (SavedRedirConsoleOutHandle == Shell->SystemTable->ConsoleOutHandle) {
        Shell->SystemTable->ConOut            = SavedConOut;
        Shell->SystemTable->ConsoleOutHandle  = SavedConsoleOutHandle;
      }

      if (SavedRedirStandardErrorHandle == Shell->SystemTable->StandardErrorHandle) {
        Shell->SystemTable->StdErr              = SavedStdErr;
        Shell->SystemTable->StandardErrorHandle = SavedStandardErrorHandle;
      }

      SetCrc (&Shell->SystemTable->Hdr);
    }
  }
  //
  // Switch output attribute to normal
  //
  if (Status == EFI_REDIRECTION_SAME) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_REDIR_SAME), HiiEnvHandle);
  } else if (EFI_REDIRECTION_NOT_ALLOWED == Status) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_REDIR_NOT_ALLOWED), HiiEnvHandle);
  }

  if (Status != -1) {
    //
    // Don't Print on a "Disconnect All" exit. The ConOut device may not exist
    //
    PrintToken (STRING_TOKEN (STR_SHELLENV_EXEC_ONE_VAR_N), HiiEnvHandle);
  }

  return Status;
}

EFI_STATUS
EFIAPI
SEnvExecute (
  IN EFI_HANDLE           *ParentImageHandle,
  IN CHAR16               *CommandLine,
  IN BOOLEAN              Output
  )
/*++

Routine Description:

Arguments:

  ParentImageHandle - The parent image handle
  CommandLine       - The command line
  Output            - The output

Returns:

--*/
{
  ENV_SHELL_INTERFACE Shell;
  EFI_STATUS          Status;

  EFI_LIST_ENTRY      *Link;
  VARIABLE_ID         *Var;
  BOOLEAN             Invalid;
  BOOLEAN             PrintHead;

  Status  = EFI_SUCCESS;
  Invalid = FALSE;

  mCmdNestingLevel++;

  //
  // Before executing a command, disable the execution break flag as default.
  //
  SEnvDisableExecutionBreak ();

  //
  // Make all file systems as un-accessed
  //
  for (Link = SEnvMap.Flink; Link != &SEnvMap; Link = Link->Flink) {
    Var = CR (Link, VARIABLE_ID, Link, VARIABLE_SIGNATURE);
    Var->Flags &= ~VAR_ID_ACCESSED;
  }
  //
  // Convert the command line to an arg list
  //
  ZeroMem (&Shell, sizeof (Shell));
  Status = SEnvStringToArg (CommandLine, Output, &Shell.ShellInt);
  if (EFI_ERROR (Status)) {
    SEnvFreeArgument (&Shell.ShellInt);
    SEnvFreeArgInfoArray (&Shell.ShellInt);
    goto Done;
  }
  //
  // Execute the command
  //
  Status = SEnvDoExecute (ParentImageHandle, CommandLine, &Shell, Output);

  /*
  if (EFI_ERROR(Status)) {
    goto Done;
  }
*/
Done:

  mCmdNestingLevel--;

  //
  // If command is a file/dir related command,
  // display invalid file systems mappings used by this command
  //
  PrintHead = TRUE;
  for (Link = SEnvMap.Flink; Link != &SEnvMap; Link = Link->Flink) {
    Var = CR (Link, VARIABLE_ID, Link, VARIABLE_SIGNATURE);
    if ((Var->Flags & VAR_ID_ACCESSED) && (Var->Flags & VAR_ID_INVALID)) {
      Invalid = TRUE;
      if (PrintHead) {
        PrintToken (STRING_TOKEN (STR_SHELLENV_EXEC_INVALID_FILE_SYSTEM), HiiEnvHandle);
        PrintHead = FALSE;
      }

      PrintToken (STRING_TOKEN (STR_SHELLENV_EXEC_ONE_VAR_HS), HiiEnvHandle, Var->Name);
    }
  }

  if (Invalid) {
    Print (L"\n\n");
  }

  return Status;
}

VOID
SEnvLoadImage (
  IN EFI_HANDLE           ParentImage,
  IN CHAR16               *IName,
  OUT EFI_HANDLE          *pImageHandle,
  OUT EFI_FILE_HANDLE     *pScriptHandle,
  OUT UINT16              *ImageMachineType,
  OUT BOOLEAN             *MachineTypeMismatch
  )
/*++

Routine Description:

Arguments:

  ParentImage   - The parent image
  IName         - The name
  pImageHandle  - The image handle
  pScriptHandle - The script handle

Returns:

--*/
{
  CHAR16                    *Path;
  BOOLEAN                   PathNeedFree;
  CHAR16                    *Ptr1;
  CHAR16                    *Ptr2;
  CHAR16                    *PathName;
  CHAR16                    *FileName;
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath;
  EFI_DEVICE_PATH_PROTOCOL  *TempDevicePath;
  EFI_DEVICE_PATH_PROTOCOL  *NewDPath;
  FILEPATH_DEVICE_PATH      *FilePath;
  CHAR16                    c;
  EFI_HANDLE                ImageHandle;
  EFI_STATUS                Status;
  SENV_OPEN_DIR             *OpenDir;
  SENV_OPEN_DIR             *OpenDirHead;
  EFI_FILE_HANDLE           ScriptHandle;
  INTN                      INameLen;
  INTN                      PathPos;
  UINTN                     Size;
  EFI_FILE_INFO             *Info;
  UINTN                     InfoBufSize;
  Elf32_Ehdr                ImageHeader;

  PathName               = NULL;
  FileName               = NULL;
  DevicePath             = NULL;
  TempDevicePath         = NULL;
  ImageHandle            = NULL;
  ScriptHandle           = NULL;
  OpenDir                = NULL;
  OpenDirHead            = NULL;
  *pImageHandle          = NULL;
  *pScriptHandle         = NULL;
  PathNeedFree           = FALSE;
  Info                   = NULL;
  *MachineTypeMismatch   = FALSE;


  //
  // Check if IName contains path info
  //
  INameLen = StrLen (IName);
  if (INameLen == 0) {
    return ;
  }

  Ptr1  = NULL;
  Ptr2  = NULL;

  for (PathPos = INameLen - 1; PathPos >= 0; PathPos--) {
    if (IName[PathPos] == ':' || IName[PathPos] == '\\') {
      if (PathPos != INameLen - 1) {
        Ptr1 = &IName[PathPos + 1];
      }
      break;
    }
  }
  //
  // Processing :foo
  //
  if (PathPos == 0 && IName[PathPos] == ':') {
    return ;
  }
  //
  // Processing foo: or foo:\
  //
  if (PathPos != -1 && Ptr1 == NULL) {
    return ;
  }

  if (PathPos >= 0) {
    Path = AllocateZeroPool ((PathPos + 1 + 1) * sizeof (CHAR16));
    if (Path == NULL) {
      return ;
    }

    PathNeedFree = TRUE;

    CopyMem (Path, IName, (PathPos + 1) * sizeof (CHAR16));
    //
    // Processing foo:bar
    //
    if (Path[PathPos] == ':') {
      Path[PathPos] = 0;
      Ptr2          = ShellCurDir (Path);
      FreePool (Path);
      PathNeedFree = FALSE;
      if (Ptr2) {
        Path          = Ptr2;
        PathNeedFree  = TRUE;
      } else {
        return ;
      }
    }

    IName = Ptr1;
    Ptr1  = StrDuplicate (Path);
    if (PathNeedFree) {
      FreePool (Path);
    }
  } else {
    //
    // Get the path variable
    //
    Path = SEnvGetEnv (L"path");
    if (!Path) {
      Path = StrDuplicate (L".");
    } else {
      Size  = StrSize (Path) + 2 * sizeof (CHAR16);
      Ptr1  = AllocateZeroPool (Size);
      if (NULL == Ptr1) {
        return ;
      }

      if (Path[0]) {
        SPrint (Ptr1, Size, L".;%s", Path);
      } else {
        SPrint (Ptr1, Size, L".");
      }

      Path = Ptr1;
    }

    PathNeedFree = TRUE;

    if (!Path) {
      return ;
    }

    Ptr1 = StrDuplicate (Path);
    if (PathNeedFree) {
      FreePool (Path);
    }
  }

  if (!Path) {
    return ;
  }

  Path = Ptr1;

  //
  // Search each path component
  // (using simple ';' as separator here - oh well)
  //
  c = *Path;
  for (Ptr1 = Path; *Ptr1 && c; Ptr1 = Ptr2 + 1) {
    for (Ptr2 = Ptr1; *Ptr2 && *Ptr2 != ';'; Ptr2++)
      ;

    if (Ptr1 != Ptr2) {
      c = *Ptr2;

      //
      // Null terminate the path
      //
      *Ptr2 = 0;
      //
      // Normally, a reference to a \FOO.EFI would end up
      // having a "." prepended to it because the environment
      // always has a "." as the first directory to check.
      // and you would end up executing .\FOO.EFI which is
      // not desired.
      // If the target starts with a "\", then we need to
      // progress the IName pointer past the "\" and seed the
      // *Ptr1 which is the environment pointer which has the "."
      // with a "\"
      //
      if (StrnCmp (L"\\", &IName[0], 1) == 0) {
        *Ptr1 = '\\';
        IName++;
      }
      //
      // Open the directory
      //
      DevicePath = SEnvNameToPath (Ptr1);
      if (!DevicePath) {
        continue;
      }

      if (OpenDir) {
        while (OpenDirHead) {
          if (OpenDirHead->Handle) {
            OpenDirHead->Handle->Close (OpenDirHead->Handle);
          }

          OpenDir = OpenDirHead->Next;
          FreePool (OpenDirHead);
          OpenDirHead = OpenDir;
        }
      }

      OpenDir = AllocateZeroPool (sizeof (SENV_OPEN_DIR));
      if (!OpenDir) {
        break;
      }

      OpenDir->Handle = LibOpenFilePath (DevicePath, EFI_FILE_MODE_READ);
      OpenDir->Next   = OpenDirHead;
      OpenDirHead     = OpenDir;
      FreePool (DevicePath);
      DevicePath = NULL;
      if (!OpenDir->Handle) {
        continue;
      }

      if (StriCmp (L".nsh", &(IName[StrLen (IName) - 4])) != 0) {
        //
        // Attempt to open it as an executable
        //
        PathName = (Ptr2[-1] == ':' || Ptr2[-1] == '\\') ? L"%s" : L"%s";

        if (StriCmp (L".efi", &(IName[StrLen (IName) - 4])) == 0) {
          FileName = L"%s";
        } else {
          FileName = L"%s.efi";
        }

        PathName  = PoolPrint (PathName, Ptr1);
        FileName  = PoolPrint (FileName, IName);
        if (!PathName || !FileName) {
          break;
        }

        DevicePath = SEnvNameToPath (PathName);
        if (PathName) {
          FreePool (PathName);
          PathName = NULL;
        }

        if (!DevicePath) {
          FreePool (FileName);
          FileName = NULL;
          continue;
        }

        TempDevicePath = FileDevicePath (NULL, FileName);
        FreePool (FileName);
        PathName = NULL;
        if (!TempDevicePath) {
          FreePool (DevicePath);
          DevicePath = NULL;
          continue;
        }

        NewDPath = AppendDevicePath (DevicePath, TempDevicePath);
        if (DevicePath != NULL) {
          FreePool (DevicePath);
          DevicePath = NULL;
        }

        if (TempDevicePath != NULL) {
          FreePool (TempDevicePath);
          TempDevicePath = NULL;
        }

        DevicePath = NewDPath;

        if (!DevicePath) {
          continue;
        }
        //
        // Attempt to load the image
        //
        Status = BS->LoadImage (
                      FALSE,
                      ParentImage,
                      DevicePath,
                      NULL,
                      0,
                      &ImageHandle
                      );
        if (!EFI_ERROR (Status)) {
          goto Done;
        } else {
          Status = LibGetImageHeader (
                     DevicePath,
                     &ImageHeader
                     );
          if (!EFI_ERROR (Status) &&
              !EFI_IMAGE_MACHINE_TYPE_SUPPORTED (ImageHeader.e_machine) &&
              *(UINT16*)(&ImageHeader.e_ident[EI_SUBSYSTEM]) == EFI_IMAGE_SUBSYSTEM_EFI_APPLICATION) {
            *MachineTypeMismatch = TRUE;
            *ImageMachineType = (UINT16) ImageHeader.e_machine;
            goto Done;
          }
        }
        //
        // Try as a ".nsh" file
        //
        FreePool (DevicePath);
        DevicePath = NULL;
      }

      if (StriCmp (L".nsh", &(IName[StrLen (IName) - 4])) == 0) {
        //
        //  User entered entire filename with .nsh extension
        //
        PathName = PoolPrint (L"%s", IName);
      } else {
        //
        //  User entered filename without .nsh extension
        //
        PathName = PoolPrint (L"%s.nsh", IName);
      }

      if (!PathName) {
        break;
      }

      DevicePath = SEnvFileNameToPath (PathName);

      if (DevicePath) {

        FilePath = (FILEPATH_DEVICE_PATH *) DevicePath;

        Status = OpenDir->Handle->Open (
                                    OpenDir->Handle,
                                    &ScriptHandle,
                                    FilePath->PathName,
                                    EFI_FILE_MODE_READ,
                                    0
                                    );

        FreePool (DevicePath);
        DevicePath = NULL;

        if (!EFI_ERROR (Status)) {
          InfoBufSize = SIZE_OF_EFI_FILE_INFO + 1024;
          Info        = AllocatePool (InfoBufSize);
          if (Info) {
            Status = ScriptHandle->GetInfo (ScriptHandle, &gEfiFileInfoGuid, &InfoBufSize, Info);
            if (!EFI_ERROR (Status) && !(Info->Attribute & EFI_FILE_DIRECTORY)) {
              FreePool (Info);
              goto Done;
            }

            FreePool (Info);
          }
        }
      }

      ScriptHandle = NULL;
    }

    if (DevicePath) {
      FreePool (DevicePath);
      DevicePath = NULL;
    }

    if (PathName) {
      FreePool (PathName);
      PathName = NULL;
    }
  }

Done:
  while (OpenDirHead) {
    if (OpenDirHead->Handle) {
      OpenDirHead->Handle->Close (OpenDirHead->Handle);
    }

    OpenDir = OpenDirHead->Next;
    FreePool (OpenDirHead);
    OpenDirHead = OpenDir;
  }

  FreePool (Path);

  if (DevicePath) {
    FreePool (DevicePath);
    DevicePath = NULL;
  }

  if (PathName) {
    FreePool (PathName);
    PathName = NULL;
  }

  if (ImageHandle) {
    ASSERT (!ScriptHandle);
    *pImageHandle = ImageHandle;
  }

  if (ScriptHandle) {
    ASSERT (!ImageHandle);
    *pScriptHandle = ScriptHandle;
  }
}

EFI_STATUS
EFIAPI
SEnvExit (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  )
/*++

Routine Description:

Arguments:

  ImageHandle - The image handle
  SystemTable - The system table

Returns:

--*/
{
  //
  // All allocated resources for Shell will be freed in SEnvFreeResources (),
  // it will be called when Shell exits (In newshell\init.c).
  //
  UINTN                   Background;
  EFI_STATUS              Status;
  SHELL_VAR_CHECK_CODE    RetCode;
  CHAR16                  *Useful;
  SHELL_VAR_CHECK_PACKAGE ChkPck;

  Useful      = NULL;
  Background  = EFI_BLACK;
  ZeroMem (&ChkPck, sizeof (SHELL_VAR_CHECK_PACKAGE));

  EFI_SHELL_APP_INIT (ImageHandle, SystemTable);

  RetCode = LibCheckVariables (SI, ExitCheckList, &ChkPck, &Useful);
  if (VarCheckOk != RetCode) {
    switch (RetCode) {
    case VarCheckUnknown:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_UNKNOWN_FLAG), HiiEnvHandle, L"exit", Useful);
      break;

    case VarCheckDuplicate:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_DUP_FLAG), HiiEnvHandle, L"exit", Useful);
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
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiEnvHandle, L"exit");
      Status = EFI_INVALID_PARAMETER;
    } else {
      if (IS_OLD_SHELL) {
        PrintToken (STRING_TOKEN (STR_NO_HELP), HiiEnvHandle);
      } else {
        PrintToken (STRING_TOKEN (STR_SHELLENV_EXIT_VERBOSE_HELP), HiiEnvHandle);
      }

      Status = EFI_SUCCESS;
    }

    goto Done;
  }

  if (ChkPck.ValueCount > 0) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiEnvHandle, L"exit");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  ST->ConOut->SetAttribute (ST->ConOut, (ST->ConOut->Mode->Attribute & 0x0f) | (Background << 4));
  ST->ConOut->ClearScreen (ST->ConOut);
  Status = (EFI_STATUS) -1;

Done:
  LibCheckVarFreeVarList (&ChkPck);
  return Status;
}

VOID
EFIAPI
SEnvIncrementShellNestingLevel (
  IN VOID
  )
/*++

Routine Description:
  Increment the nesting level of the shell prompt.
  
Arguments:

  None
  
Returns:

--*/
{
  mShellNestingLevel++;
}

VOID
EFIAPI
SEnvDecrementShellNestingLevel (
  IN VOID
  )
/*++

Routine Description:
  Decrement the nesting level of the shell prompt.
  
Arguments:
  
  None
  
Returns:

--*/
{
  if (mShellNestingLevel != 0) {
    mShellNestingLevel--;
  }
}

BOOLEAN
EFIAPI
SEnvIsRootShell (
  IN VOID
  )
/*++

Routine Description:
  Decrement the nesting level of the shell prompt.
  
Arguments:
  
  None
  
Returns:

--*/
{
  if (mShellNestingLevel == 0) {
    return TRUE;
  } else {
    return FALSE;
  }
}

VOID
EFIAPI
SEnvEnablePageBreak (
  IN INT32      StartRow,
  IN BOOLEAN    AutoWrap
  )
/*++

Routine Description:
  Enable the page break output mode.
  
Arguments:
  StartRow    - The start row number
  AutoWrap    - Indicate if automatically wrap the line
  
Returns:

--*/
{
  //
  // One command can leverage other commands to implement itself. As a result,
  // the command executing is nested. The descendant commands should inherit the
  // setting of the page break output mode. So the page break output mode only can
  // be enabled/disabled by the root command.
  //
  if (mCmdNestingLevel == mShellNestingLevel) {
    SEnvConOutEnablePageBreak (StartRow, AutoWrap);
  }
}

VOID
EFIAPI
SEnvDisablePageBreak (
  IN VOID
  )
/*++

Routine Description:
  Disable the page break output mode.
  
Arguments:
  
  None
  
Returns:

--*/
{
  //
  // One command can leverage other commands to implement itself. As a result,
  // the command executing is nested. The descendant commands should inherit the
  // setting of the page break output mode. So the page break output mode only can
  // be enabled/disabled by the root command.
  //
  if (mCmdNestingLevel == mShellNestingLevel) {
    SEnvConOutDisablePageBreak ();
  }
}

BOOLEAN
EFIAPI
SEnvGetPageBreak (
  IN VOID
  )
/*++

Routine Description:
  Get the enable status of the page break output mode.
  
Arguments:

  None
  
Returns:

--*/
{
  return SEnvConOutGetPageBreak ();
}

VOID
SEnvEnableExecutionBreak (
  IN VOID
  )
/*++

Routine Description:
  Enable the execution break flag.
  
Arguments:

  None
  
Returns:

--*/
{
  mExecutionBreak = TRUE;
}

VOID
SEnvDisableExecutionBreak (
  IN VOID
  )
/*++

Routine Description:
  Disable the execution break flag.
  
Arguments:
  
  None
  
Returns:

--*/
{
  //
  // Once the execution break is issued by the user in whatever the root command
  // or the descendant command, the whole command execution should be break. So
  // the execution break flag only can be disabled by the root command.
  //
  if (mCmdNestingLevel == mShellNestingLevel) {
    mExecutionBreak = FALSE;
  }
}

BOOLEAN
EFIAPI
SEnvGetExecutionBreak (
  IN VOID
  )
/*++

Routine Description:
  Get the enable status of the execution break flag.
  
Arguments:

  None
  
Returns:

--*/
{
  return mExecutionBreak;
}

VOID
SEnvFreeArgumentOld (
  IN  UINTN                   *Argc,
  IN  CHAR16                  ***Argv
  )
/*++

Routine Description:
  This function frees the parsed arguments.
  
Arguments:
  Argc             Argument count
  Argv             The parsed arguments

Returns:

--*/
{
  UINT32  Index;

  ASSERT (Argv);
  ASSERT (Argc);

  if (*Argv != NULL) {
    //
    // Free arguments according to the argument count
    //
    for (Index = 0; Index < *Argc; Index++) {
      if ((*Argv)[Index] != NULL) {
        FreePool ((*Argv)[Index]);
        (*Argv)[Index] = NULL;
      }
    }

    FreePool (*Argv);
    *Argv = NULL;
  }

  *Argc = 0;
}

VOID
ShellCopyStrSkipQuote (
  OUT CHAR16                   *Dst,
  IN  CHAR16                   *Src,
  IN  UINTN                    Length
  )
/*++

Routine Description:
  Copy a string from source to destination, all '"'s are skipped

Arguments:
  Dst              Destination string
  Src              Source string
  Length           Length of destination string

Returns:

--*/
{
  UINTN SrcIndex;
  UINTN DstIndex;

  SrcIndex = DstIndex = 0;
  while (DstIndex < Length) {
    //
    // In shell, '"' is used to delimit quotation so we skip all '"'s to get
    // the variable name.
    //
    if (Src[SrcIndex] != '"') {
      Dst[DstIndex] = Src[SrcIndex];
      DstIndex++;
    }

    SrcIndex++;
  }
}

EFI_STATUS
ShellParseStrOld (
  IN  CHAR16                   *Str,
  IN  OUT PARSE_STATE          *ParseState
  )
/*++

Routine Description:
  Parse a command line string into several arguments

Arguments:
  Str              Position to begin parse
  ParseState       The PARSE_STATE structure holding the current parse state

Returns:
  EFI_SUCCESS      The function finished successfully
  
--*/
{
  EFI_STATUS          Status;
  CHAR16              *Alias;
  CHAR16              *SubstituteStr;
  CHAR16              *OldSubstituteStr;
  BOOLEAN             Literal;
  BOOLEAN             Comment;
  BOOLEAN             IsVariable;
  UINTN               ArgNo;
  CHAR16              *VarName;
  UINTN               VarNameLen;
  UINTN               QuoteCount;
  EFI_SHELL_ARG_INFO  *ArgInfo;

  ParseState->RecurseLevel += 1;
  if (ParseState->RecurseLevel > 5) {
    DEBUG ((EFI_D_VARIABLE, "shell: Recursive alias or macro\n"));
    if (ParseState->Output) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_EXEC_RECURSIVE_ALIAS), HiiEnvHandle);
    }

    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  SubstituteStr = NULL;
  VarNameLen    = 0;

  while (*Str) {
    //
    // Skip leading white space
    //
    if (IsWhiteSpace (*Str)) {
      Str += 1;
      continue;
    }
    //
    // Pull this arg out of the string
    //
    ParseState->BufferIndex = 0;
    Literal                 = FALSE;
    Comment                 = FALSE;
    while (*Str) {
      //
      // If we have white space (including ',') and we are not in a quote,
      // move to the next word
      //
      if ((IsWhiteSpace (*Str)) && !ParseState->Quote) {
        break;
      }
      //
      // Check char
      //
      switch (*Str) {
      case '#':
        if (SEnvBatchIsActive ()) {
          //
          //  Comment, discard the rest of the characters in the line
          //
          Comment = TRUE;
          while (*Str) {
            Str++;
          }

          Str--;
        } else {
          ShellAppendBuffer (ParseState, 1, Str);
        }
        break;

      case '%':
        //
        // if in a script, try to find argument
        //
        IsVariable = FALSE;
        if (SEnvBatchIsActive () && IsDigit (Str[1])) {
          //
          // Found a script argument - substitute
          //
          ArgNo   = Str[1] - '0';
          Status  = SEnvBatchFindArgument (ArgNo, &SubstituteStr, &ArgInfo);
          if (EFI_ERROR (Status) && Status != EFI_NOT_FOUND) {
            PrintToken (STRING_TOKEN (STR_SHELLENV_EXEC_RECURSIVE_ALIAS), HiiEnvHandle, Status);
            SEnvSetBatchAbort ();
            goto Done;
          }

          if (!EFI_ERROR (Status)) {
            ShellAppendBuffer (
              ParseState,
              StrLen (SubstituteStr),
              SubstituteStr
              );
            Str += 1;
            break;
          }
        }
        //
        // Try to find a shell enviroment variable
        //
        QuoteCount        = 0;
        OldSubstituteStr  = SubstituteStr;
        SubstituteStr     = Str + 1;
        while (*SubstituteStr != '%' && *SubstituteStr != 0 && (!IsWhiteSpace (*SubstituteStr) || ParseState->Quote)) {
          if (*SubstituteStr == '"') {
            ParseState->Quote = (BOOLEAN) (!ParseState->Quote);
            QuoteCount++;
          }

          SubstituteStr++;
        }

        if (*SubstituteStr == '%') {
          IsVariable  = TRUE;
          VarNameLen  = SubstituteStr - (Str + 1) - QuoteCount;
          VarName     = AllocateZeroPool ((VarNameLen + 1) * sizeof (CHAR16));
          if (!VarName) {
            Status = EFI_OUT_OF_RESOURCES;
            goto Done;
          }

          ShellCopyStrSkipQuote (VarName, Str + 1, VarNameLen);
          VarName[VarNameLen] = (CHAR16) 0x0000;
          //
          //  Check for special case "lasterror" variable
          //  Otherwise just get the matching environment variable
          //
          if (SEnvBatchIsActive () && SEnvBatchVarIsLastError (VarName)) {
            SubstituteStr = SEnvBatchGetLastError ();
          } else {
            SubstituteStr = SEnvGetEnv (VarName);
          }

          FreePool (VarName);

          if (SubstituteStr) {
            //
            //  Insert the variable's value in the new arg -
            //  the arg may include more than just the variable
            //
            ShellAppendBuffer (
              ParseState,
              StrLen (SubstituteStr),
              SubstituteStr
              );
            Str += VarNameLen + 1 + QuoteCount;
            break;
          }
        }
        //
        // Try to find a for statement defined variable
        //
        if (SEnvBatchIsActive () && IsAlpha (Str[1])) {
          //
          //  For loop index
          //
          Status = SEnvBatchFindVariable (Str, &SubstituteStr);
          if (EFI_ERROR (Status) && Status != EFI_NOT_FOUND) {
            if (ParseState->Output) {
              PrintToken (STRING_TOKEN (STR_SHELLENV_EXEC_FAIL_FIND_VAR), HiiEnvHandle);
            }

            goto Done;
          }

          if (!EFI_ERROR (Status) && SubstituteStr) {
            //
            //  Found a match
            //
            ShellAppendBuffer (
              ParseState,
              StrLen (SubstituteStr),
              SubstituteStr
              );

            //
            // only advance one char - standard processing will get the 2nd char
            //
            Str += 1;
            break;
          }
        }

        if (SEnvBatchIsActive () && IsDigit (Str[1])) {
          Str += 1;
          break;
        }

        if (IsVariable) {
          Str += VarNameLen + QuoteCount + 1;
        }
        break;

      case '^':
        //
        //  Literal, don't process aliases on this arg
        //
        if (Str[1]) {
          ShellAppendBuffer (ParseState, 1, &Str[1]);
          Str += 1;
          Literal = TRUE;
        }
        break;

      case '"':
        //
        //  Quoted string entry and exit
        //
        ParseState->Quote = (BOOLEAN) (!ParseState->Quote);
        break;

      default:
        if (!IsValidChar (*Str)) {
          if (ParseState->Output) {
            PrintToken (STRING_TOKEN (STR_SHELLENV_EXEC_INVALID_CHAR), HiiEnvHandle, *Str);
          }

          Status = EFI_INVALID_PARAMETER;
          goto Done;
        }

        ShellAppendBuffer (ParseState, 1, Str);
        break;
      }
      //
      // Next char
      //
      Str += 1;
    }
    //
    // If the new argument string is empty and we have encountered a
    // comment, then skip it.  Otherwise we have a new arg
    //
    if (Comment && ParseState->BufferIndex == 0) {
      break;
    }
    //
    // If it was composed with a literal, do not check to see if the arg has an
    // alias
    //
    Alias = NULL;
    ParseState->Buffer[ParseState->BufferIndex] = 0;
    if (!Literal && !ParseState->AliasLevel && ParseState->ArgIndex == 0) {
      Alias = SEnvGetAlias (ParseState->Buffer);
    }
    //
    // If there's an alias, parse it
    //
    if (Alias) {

      ParseState->AliasLevel += 1;
      Status = ShellParseStrOld (Alias, ParseState);
      ParseState->AliasLevel -= 1;

      if (EFI_ERROR (Status)) {
        goto Done;
      }

    } else {
      if (ParseState->ArgIndex == ParseState->ArgCount) {
        ParseState->Arg = ReallocatePool (
                            ParseState->Arg,
                            ParseState->ArgCount * sizeof (CHAR16 *),
                            (ParseState->ArgCount + COMMON_ARG_COUNT) * sizeof (CHAR16 *)
                            );
        if (ParseState->Arg == NULL) {
          Status = EFI_OUT_OF_RESOURCES;
          break;
        }

        ParseState->ArgCount += COMMON_ARG_COUNT;
      }
      //
      // (Prevent NULL Arg from being recorded into array) <- Old comments
      // Removed the 'if' statement to compy with EFI1.1 implementation on this feature - allow NULL arg
      //

      /*
      if (*ParseState->Buffer) {
*/

      //
      // Otherwise, copy the word to the arg array
      //
      ParseState->Arg[ParseState->ArgIndex] = StrDuplicate (ParseState->Buffer);
      if (!ParseState->Arg[ParseState->ArgIndex]) {
        Status = EFI_OUT_OF_RESOURCES;
        break;
      }

      ParseState->ArgIndex += 1;

      /*
      }
*/
    }
  }

  Status = EFI_SUCCESS;

Done:
  ParseState->RecurseLevel -= 1;
  if (EFI_ERROR (Status)) {
    //
    // Free all the args allocated
    //
    SEnvFreeArgumentOld (&ParseState->ArgIndex, &ParseState->Arg);
  }

  return Status;
}

EFI_STATUS
EFIAPI
SEnvExitGetLineHelp (
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
  return SEnvCmdGetStringByToken (STRING_TOKEN (STR_SHELLENV_EXIT_LINE_HELP), Str);
}
