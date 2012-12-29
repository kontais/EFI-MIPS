/*++

Copyright (c) 2005 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  ls.c

Abstract:

  EFI shell command "ls"

Revision History

--*/

#include "ls.h"

extern UINT8    STRING_ARRAY_NAME[];

//
// Global Variables
//
EFI_HII_HANDLE  HiiLsHandle;
EFI_GUID        EfiLsGuid = EFI_LS_GUID;
SHELL_VAR_CHECK_ITEM    LsCheckList[] = {
  {
    L"-b",
    0x01,
    0,
    FlagTypeSingle
  },
  {
    L"-r",
    0x02,
    0,
    FlagTypeSingle
  },
  {
    L"-a",
    0x04,
    0,
    FlagTypeNeedSet
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

BOOLEAN         mIgnoreAttribute;
BOOLEAN         mAttributes;
BOOLEAN         mAttribA;
BOOLEAN         mAttribH;
BOOLEAN         mAttribS;
BOOLEAN         mAttribR;
BOOLEAN         mAttribD;
BOOLEAN         mRecursive;
BOOLEAN         mPageBreak;

BOOLEAN         mMatchStratage;

UINT64          mTotalSize;
UINT64          mTotalFiles;
UINT64          mTotalDirs;

//
// Function declarations
//
VOID
LsDumpFileInfo (
  IN EFI_FILE_INFO *Info
  );

SHELL_FILE_TYPE
GetFileType (
  IN EFI_FILE_INFO *Info
  );

EFI_STATUS
EFIAPI
InitializeLS (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

//
// Entry Point
//
EFI_BOOTSHELL_CODE(
  EFI_APPLICATION_ENTRY_POINT(InitializeLS)
)

EFI_STATUS
EFIAPI
InitializeLS (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
/*++

Routine Description:
  Command entry point. Parses command line arguments and calls internal
  function to perform actual action.

Arguments:
  ImageHandle     The image handle.
  SystemTable     The system table.

Returns:
  EFI_SUCCESS             - Command completed successfully
  EFI_INVALID_PARAMETER   - Command usage error
  EFI_UNSUPPORTED         - Protocols unsupported
  EFI_OUT_OF_RESOURCES    - Out of memory
  Other value             - Unknown error

--*/
{
  EFI_STATUS  Status;

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
  Status = LibInitializeStrings (&HiiLsHandle, STRING_ARRAY_NAME, &EfiLsGuid);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (!EFI_PROPER_VERSION (0, 99)) {
    PrintToken (
      STRING_TOKEN (STR_SHELLENV_GNC_COMMAND_NOT_SUPPORT),
      HiiLsHandle,
      L"ls/dir",
      EFI_VERSION_0_99 
      );
    Status = EFI_UNSUPPORTED;
    goto Done;
  }

  Status = MainProc (ImageHandle, SystemTable, STRING_TOKEN (STR_LS_VERBOSE_HELP));
Done:
  LibUnInitializeStrings ();
  return Status;
}

EFI_STATUS
MainProc (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable,
  IN UINT16               VHlpToken
  )
/*++

Routine Description:

Arguments:
  ImageHandle     The image handle.
  SystemTable     The system table.
  VHlpToken       The help token.

Returns:
  EFI_SUCCESS             - Command completed successfully
  EFI_INVALID_PARAMETER   - Command usage error
  EFI_UNSUPPORTED         - Protocols unsupported
  EFI_OUT_OF_RESOURCES    - Out of memory
  Other value             - Unknown error

--*/
{
  EFI_STATUS              Status;
  CHAR16                  **Argv;
  CHAR16                  *PtrTwo;
  CHAR16                  *Path;
  EFI_LIST_ENTRY          DirList;
  EFI_LIST_ENTRY          *Link;
  SHELL_FILE_ARG          *Arg;
  BOOLEAN                 WildcardsEncountered;
  CHAR16                  *Pattern;
  SHELL_VAR_CHECK_CODE    RetCode;
  SHELL_VAR_CHECK_PACKAGE ChkPck;
  SHELL_ARG_LIST          *Item;
  CHAR16                  *Useful;
  Argv    = SI->Argv;
  PtrTwo  = NULL;
  InitializeListHead (&DirList);
  Link                = NULL;
  Arg                 = NULL;
  Pattern             = NULL;
  Status              = EFI_SUCCESS;
  ZeroMem (&ChkPck, sizeof (SHELL_VAR_CHECK_PACKAGE));

  //
  // Global variable initializations
  //
  mIgnoreAttribute  = FALSE;
  mAttributes       = FALSE;
  mRecursive        = FALSE;
  mAttribA          = FALSE;
  mAttribH          = FALSE;
  mAttribS          = FALSE;
  mAttribR          = FALSE;
  mAttribD          = FALSE;
  mTotalSize = 0;
  mTotalFiles = 0;
  mTotalDirs = 0;

  //
  // Check flags
  //
  RetCode = LibCheckVariables (SI, LsCheckList, &ChkPck, &Useful);
  if (VarCheckOk != RetCode) {
    switch (RetCode) {
    case VarCheckDuplicate:

      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_DUP_FLAG), HiiLsHandle, L"ls/dir", Useful);
      break;

    case VarCheckUnknown:

      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_UNKNOWN_FLAG), HiiLsHandle, L"ls/dir", Useful);
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
    if (IS_OLD_SHELL) {
      PrintToken (STRING_TOKEN (STR_NO_HELP), HiiLsHandle);
    } else if (ChkPck.ValueCount > 0 ||
             ChkPck.FlagCount > 2 ||
             (2 == ChkPck.FlagCount && !LibCheckVarGetFlag (&ChkPck, L"-b"))
            ) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiLsHandle, L"ls/dir");
      Status = EFI_INVALID_PARAMETER;
    } else {
      PrintToken (VHlpToken, HiiLsHandle);
      Status = EFI_SUCCESS;
    }

    LibCheckVarFreeVarList (&ChkPck);
    return Status;
  }

  Item = LibCheckVarGetFlag (&ChkPck, L"-r");
  if (Item) {
    mRecursive = TRUE;
  }

  Item = LibCheckVarGetFlag (&ChkPck, L"-a");
  if (Item) {
    mAttributes = TRUE;
    Useful      = Item->VarStr;
    if (0 == *Useful) {
      mIgnoreAttribute = TRUE;
    } else {
      while (*Useful) {
        switch (*Useful) {
        case 'A':
        case 'a':
          if (!mAttribA) {
            mAttribA = TRUE;
          } else {
            PrintToken (STRING_TOKEN (STR_LS_DUP_ATTRIBUTE), HiiLsHandle, L"ls/dir", *Useful);
            Status = EFI_INVALID_PARAMETER;
            goto Done;
          }
          break;

        case 'H':
        case 'h':
          if (!mAttribH) {
            mAttribH = TRUE;
          } else {
            PrintToken (STRING_TOKEN (STR_LS_DUP_ATTRIBUTE), HiiLsHandle, L"ls/dir", *Useful);
            Status = EFI_INVALID_PARAMETER;
            goto Done;
          }
          break;

        case 'S':
        case 's':
          if (!mAttribS) {
            mAttribS = TRUE;
          } else {
            PrintToken (STRING_TOKEN (STR_LS_DUP_ATTRIBUTE), HiiLsHandle, L"ls/dir", *Useful);
            Status = EFI_INVALID_PARAMETER;
            goto Done;
          }
          break;

        case 'R':
        case 'r':

          if (!mAttribR) {
            mAttribR = TRUE;
          } else {
            PrintToken (STRING_TOKEN (STR_LS_DUP_ATTRIBUTE), HiiLsHandle, L"ls/dir", *Useful);
            Status = EFI_INVALID_PARAMETER;
            goto Done;
          }
          break;

        case 'D':
        case 'd':

          if (!mAttribD) {
            mAttribD = TRUE;
          } else {
            PrintToken (STRING_TOKEN (STR_LS_DUP_ATTRIBUTE), HiiLsHandle, L"ls/dir", *Useful);
            Status = EFI_INVALID_PARAMETER;
            goto Done;
          }
          break;

        default:
          PrintToken (STRING_TOKEN (STR_LS_FILE_ATTR), HiiLsHandle, L"ls/dir", *Useful);
          Status = EFI_INVALID_PARAMETER;
          goto Done;
        }

        Useful++;
      }
      //
      // end of while (*Useful)
      //
    }
  }
  //
  // end of if (item)
  //
  if (ChkPck.ValueCount > 1) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiLsHandle, L"ls/dir");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  } else if (1 == ChkPck.ValueCount) {
    Path    = ChkPck.VarList->VarStr;
    PtrTwo  = GetPattern (Path);
    Pattern = StrDuplicate (PtrTwo);

    Status  = ShellFileMetaArg (ChkPck.VarList->VarStr, &DirList);
    if (EFI_ERROR (Status)) {
      PrintToken (STRING_TOKEN (STR_LS_CANNOT_OPEN), HiiLsHandle, L"ls/dir", Argv[ChkPck.ValueCount], Status);
      goto Done;
    }
    //
    // Even file not found, can't be an empty list
    //
    if (IsListEmpty (&DirList)) {
      Status = EFI_NOT_FOUND;
      PrintToken (STRING_TOKEN (STR_LS_CANNOT_OPEN), HiiLsHandle, L"ls/dir", Argv[ChkPck.ValueCount], Status);
      goto Done;
    }
  } else {
    //
    // ChkPck.ValueCount = 0
    //
    Pattern = StrDuplicate (L"");
    Status  = ShellFileMetaArg (L".", &DirList);
    if (EFI_ERROR (Status)) {
      PrintToken (STRING_TOKEN (STR_LS_CANNOT_OPEN_CURRENT_DIR), HiiLsHandle, L"ls/dir", Status);
      goto Done;
    }

    if (IsListEmpty (&DirList)) {
      Status = EFI_NOT_FOUND;
      PrintToken (STRING_TOKEN (STR_LS_CANNOT_OPEN_CURRENT_DIR), HiiLsHandle, L"ls/dir", Status);
      goto Done;
    }

    Arg = CR (DirList.Flink, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);
    if (EFI_ERROR (Arg->Status)) {
      PrintToken (STRING_TOKEN (STR_LS_CANNOT_OPEN_CURRENT_DIR), HiiLsHandle, L"ls/dir", Arg->Status);
      Status = Arg->Status;
      goto Done;
    }
  }

  WildcardsEncountered = HasWildcards (Pattern);
  //
  // no wildcards and is a directory
  //
  if (DirList.Flink->Flink == &DirList) {
    Arg = CR (DirList.Flink, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);

    if (Arg->Status == EFI_SUCCESS && Arg->Info && (Arg->Info->Attribute & EFI_FILE_DIRECTORY)) {
      if (!WildcardsEncountered) {
        Status = Listing (Arg->Handle, Arg->FullName, L"*");
        goto Final;
      }
    }
  }
  //
  // Call LsList
  //
  Arg = CR (
          DirList.Flink,
          SHELL_FILE_ARG,
          Link,
          SHELL_FILE_ARG_SIGNATURE
          );

  //
  // if (Arg->Parent && Arg->Status == EFI_SUCCESS) {
  //
  if (Arg->Parent) {
    Status = Listing (Arg->Parent, Arg->ParentName, CleanupAsFatLfn (Pattern));
  } else {
    PrintToken (
      STRING_TOKEN (STR_LS_CANNOT_OPEN_DIR),
      HiiLsHandle,
      L"ls/dir",
      Arg->FullName,
      Arg->Status
      );
    Status = Arg->Status;
    goto Final;
  }

Final:
  if (EFI_ERROR (Status)) {
    goto Done;
  }
  //
  // Display Final Summary for recursive
  //
  if (mRecursive) {
    PrintToken (STRING_TOKEN (STR_LS_TOTAL_SUMMARY), HiiLsHandle);
    PrintToken (STRING_TOKEN (STR_LS_FILE_BYTES), HiiLsHandle, mTotalFiles, mTotalSize);
    PrintToken (STRING_TOKEN (STR_LS_DIR), HiiLsHandle, mTotalDirs);
  }

Done:
  //
  // Free resources
  //
  if (Pattern) {
    FreePool (Pattern);
  }

  LibCheckVarFreeVarList (&ChkPck);
  ShellFreeFileList (&DirList);

  return Status;
}

EFI_STATUS
Listing (
  IN EFI_FILE_HANDLE   ParentHandle,
  IN CHAR16            *ParentPath,
  IN CHAR16            *Pattern
  )
/*++

        Routine Description:

        Performs list operation on the directory indicated by ParentHandle.
        Use Pattern as the search string to find matches to be listed.
        Recursive search is performed if required

        Arguments:

        ParentHandle    The handle of the parent directory
        ParentPath      The path of the parent directory
        Pattern         Search string

        Returns:

        EFI_SUCCESS

        --*/
{
  EFI_STATUS      Status;
  EFI_FILE_INFO   *Info;
  UINTN           BufSize;
  EFI_FILE_HANDLE ThisHandle;
  UINT64          TotalSize;
  UINT64          TotalFiles;
  UINT64          TotalDirs;
  CHAR16          *FullPath;
  BOOLEAN         Something;

  //
  // local variable initializations
  //
  ThisHandle  = NULL;
  BufSize     = SIZE_OF_EFI_FILE_INFO + 1024;
  //
  // Large enough
  //
  TotalSize       = 0;
  TotalFiles      = 0;
  TotalDirs       = 0;
  FullPath        = NULL;
  Something       = FALSE;
  mMatchStratage  = FALSE;

  Info            = AllocatePool (BufSize);
  if (Info == NULL) {
    PrintToken (STRING_TOKEN (STR_LS_OUT_OF_MEM), HiiLsHandle, L"ls/dir");
    Status = EFI_OUT_OF_RESOURCES;
    goto ListDone;
  }
  //
  // get parent directory info
  //
  Status = ParentHandle->GetInfo (ParentHandle, &gEfiFileInfoGuid, &BufSize, Info);
  if (EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_LS_CANNOT_INFO_DIR), HiiLsHandle, L"ls/dir", ParentPath);
    goto ListDone;
  }
  //
  // is it a directory?
  //
  if (!(Info->Attribute & EFI_FILE_DIRECTORY)) {
    PrintToken (STRING_TOKEN (STR_LS_CANNOT_OPERATE_FILE), HiiLsHandle, L"ls/dir", ParentPath);
    Status = EFI_INVALID_PARAMETER;
    goto ListDone;
  }
  //
  // find all match items in the parent directory for the first time
  // to determine if this dir need to be displayed in Recursive case
  //
  Status = FindMatchItem (
            ParentHandle,
            ParentPath,
            Pattern,
            FALSE,
            &Something,
            Info
            );
  if (EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_LS_CANNOT_ACCESS_DIR), HiiLsHandle, L"ls/dir", ParentPath);
    goto ListDone;
  }

  if (!Something) {
    Status = FindMatchItem (
              ParentHandle,
              ParentPath,
              Pattern,
              TRUE,
              &Something,
              Info
              );
    if (EFI_ERROR (Status)) {
      PrintToken (STRING_TOKEN (STR_LS_CANNOT_ACCESS_DIR), HiiLsHandle, L"ls/dir", ParentPath);
      goto ListDone;
    }

    mMatchStratage = TRUE;
  }

  if (Something || !mRecursive) {
    PrintToken (STRING_TOKEN (STR_LS_DIR_OF), HiiLsHandle, ParentPath);
    Print (L"\n");

    //
    // list all match items in the parent directory
    //
    Status = ParentHandle->SetPosition (ParentHandle, 0);
    if (EFI_ERROR (Status)) {
      PrintToken (STRING_TOKEN (STR_LS_CANNOT_ACCESS_DIR), HiiLsHandle, L"ls/dir", ParentPath);
      goto ListDone;
    }
    //
    // loop
    //
    for (;;) {
      //
      // Break the execution?
      //
      if (GetExecutionBreak ()) {
        goto ListDone;
      }
      //
      // reuse BufSize and Info
      //
      BufSize = SIZE_OF_EFI_FILE_INFO + 1024;
      Status  = ParentHandle->Read (ParentHandle, &BufSize, Info);
      if (EFI_ERROR (Status)) {
        PrintToken (
          STRING_TOKEN (STR_LS_CANNOT_READ_DIR),
          HiiLsHandle,
          L"ls/dir",
          ParentPath,
          Status
          );
        goto ListDone;
      }
      //
      // no more entries
      //
      if (BufSize == 0) {
        break;
      }
      //
      // if file name matches, and attributes right, display it
      //
      if (IsMatch (Info, Pattern, mMatchStratage)) {
        if (Info->Attribute & EFI_FILE_DIRECTORY) {
          TotalDirs++;
          mTotalDirs++;
        } else {
          TotalFiles++;
          TotalSize += Info->FileSize;
          mTotalFiles++;
          mTotalSize += Info->FileSize;
        }

        LsDumpFileInfo (Info);
      }
    }
    //
    // Display subTotal
    //
    if (TotalFiles == 0 && TotalDirs == 0) {
      PrintToken (STRING_TOKEN (STR_LS_FILE_NOT_FOUND), HiiLsHandle);
      Status = EFI_NOT_FOUND;
    } else {
      PrintToken (STRING_TOKEN (STR_LS_FILE_BYTES), HiiLsHandle, TotalFiles, TotalSize);
      PrintToken (STRING_TOKEN (STR_LS_DIR), HiiLsHandle, TotalDirs);
      Print (L"\n");
    }

  }
  //
  // if recursive, for each dir within the parent directory,
  // call myself
  //
  if (mRecursive) {
    Status = ParentHandle->SetPosition (ParentHandle, 0);
    if (EFI_ERROR (Status)) {
      PrintToken (STRING_TOKEN (STR_LS_CANNOT_ACCESS_DIR), HiiLsHandle, L"ls/dir", ParentPath);
      goto ListDone;
    }
    //
    // loop
    //
    for (;;) {
      //
      // Break the execution?
      //
      if (GetExecutionBreak ()) {
        goto ListDone;
      }
      //
      // reuse BufSize and Info
      //
      BufSize = SIZE_OF_EFI_FILE_INFO + 1024;
      Status  = ParentHandle->Read (ParentHandle, &BufSize, Info);
      if (EFI_ERROR (Status)) {
        PrintToken (
          STRING_TOKEN (STR_LS_CANNOT_READ_DIR),
          HiiLsHandle,
          L"ls/dir",
          ParentPath,
          Status
          );
        goto ListDone;
      }
      //
      // no more entries
      //
      if (BufSize == 0) {
        break;
      }
      //
      // if it's a directory, open it and recursive
      //
      if (Info->Attribute & EFI_FILE_DIRECTORY &&
          StriCmp (Info->FileName, L".") != 0 &&
          StriCmp (Info->FileName, L"..") != 0
          ) {
        //
        // IsMatch(Info, Pattern))
        //
        // Compose a full path for this dir
        //
        if (FullPath) {
          FreePool (FullPath);
        }

        FullPath = AllocatePool (StrSize (ParentPath) + 1 + StrSize (Info->FileName) + 2);
        if (!FullPath) {
          PrintToken (STRING_TOKEN (STR_LS_OUT_OF_MEM), HiiLsHandle, L"ls/dir");
          Status = EFI_OUT_OF_RESOURCES;
          goto ListDone;
        }

        StrCpy (FullPath, ParentPath);
        if (FullPath[StrLen (FullPath) - 1] != '\\') {
          StrCat (FullPath, L"\\");
        }

        StrCat (FullPath, Info->FileName);

        Status = ParentHandle->Open (
                                ParentHandle,
                                &ThisHandle,
                                Info->FileName,
                                EFI_FILE_MODE_READ,
                                0
                                );
        if (EFI_ERROR (Status)) {
          PrintToken (
            STRING_TOKEN (STR_LS_CANNOT_OPEN_DIR),
            HiiLsHandle,
            L"ls/dir",
            FullPath,
            Status
            );
          goto ListDone;
        }

        Status = Listing (ThisHandle, FullPath, CleanupAsFatLfn (Pattern));
        if (Status == EFI_ABORTED) {
          goto ListDone;
        }

        ThisHandle->Close (ThisHandle);
      }
    }
  }
  //
  // end of mRecursive
  //
ListDone:
  if (FullPath) {
    FreePool (FullPath);
  }

  if (Info) {
    FreePool (Info);
  }

  return Status;
}

CHAR16 *
CleanupAsFatLfn (
  CHAR16  *Name
  )
{
  CHAR16  *p1;

  CHAR16  *p2;

  //
  // Strip off starting/trailing spaces and trailing periods, as FAT spec.
  //
  for (p1 = Name; *p1 && *p1 == ' '; p1++) {
    ;
  }

  p2 = Name;

  while (*p1) {
    *p2 = *p1;
    p1++;
    p2++;
  }

  *p2 = 0;

  for (p1 = Name + StrLen (Name) - 1; p1 >= Name && (*p1 == ' ' || *p1 == '.'); p1--) {
    ;
  }

  *(p1 + 1) = 0;

  return Name;
}

CHAR16 *
GetPattern (
  CHAR16 *Path
  )
{

  EFI_STATUS  Status;
  CHAR16      *PtrOne;
  CHAR16      *PtrTwo;
  UINTN       Index1;
  UINTN       Index2;
  UINTN       Pos;
  BOOLEAN     LeadingBlanks;

  LeadingBlanks = TRUE;
  Status        = EFI_SUCCESS;

  for (PtrOne = PtrTwo = Path; PtrOne < Path + StrLen (Path); PtrOne++) {
    if ((*PtrOne) == ' ' && LeadingBlanks) {
      PtrTwo = PtrOne + 1;
    } else {
      LeadingBlanks = FALSE;
      if ((*PtrOne) == '\\' || (*PtrOne) == ':') {
        PtrTwo = PtrOne + 1;
      }
    }
  }
  //
  // Filter redundant '*' in pattern
  //
  Index1  = 0;
  while (PtrTwo[Index1] != '\0') {
    Pos     = Index1;
    Index2  = Index1 + 1;
    if (PtrTwo[Pos] == '*') {
      Pos++;
      while (PtrTwo[Pos] == '*' && PtrTwo[Pos] != '\0') {
        Pos++;
      }

      PtrTwo[Index2] = PtrTwo[Pos];
      while (PtrTwo[Index2] != '\0') {
        Index2++;
        Pos++;
        PtrTwo[Index2] = PtrTwo[Pos];
      }
    }

    Index1++;
  }
  //
  // Strip off trailing blanks in pattern
  //
  for (Index1 = StrLen (PtrTwo) - 1; Index1 >= 0 && PtrTwo[Index1] == ' '; Index1--) {
    PtrTwo[Index1] = 0;
  }

  return PtrTwo;
}

BOOLEAN
IsMatch (
  EFI_FILE_INFO  *Info,
  CHAR16         *Pattern,
  BOOLEAN        BracketLiteral
  )
{
  CHAR16  *NewPattern;
  CHAR16  *PtrPattern;
  CHAR16  *PtrNewPattern;
  UINTN   Size;
  BOOLEAN IsMatch;

  if (!BracketLiteral) {
    NewPattern = StrDuplicate (Pattern);
    if (!NewPattern) {
      IsMatch = FALSE;
      goto Done;
    }
  } else {
    Size        = StrSize (Pattern);
    NewPattern  = AllocateZeroPool (Size * 2);
    if (!NewPattern) {
      IsMatch = FALSE;
      goto Done;
    }
    //
    // composed the name
    //
    PtrPattern    = Pattern;
    PtrNewPattern = NewPattern;

    while (*PtrPattern) {
      if (*PtrPattern == '[' || *PtrPattern == ']') {
        *(PtrNewPattern++) = ESCAPE_CHAR;
      }

      *(PtrNewPattern++) = *(PtrPattern++);
    }

    *PtrNewPattern = 0;
  }

  IsMatch = MetaMatch (Info->FileName, NewPattern);

  if ((IsMatch && mIgnoreAttribute) ||
      (IsMatch && !mAttributes && !(Info->Attribute & EFI_FILE_HIDDEN) && !(Info->Attribute & EFI_FILE_SYSTEM)) ||
      IsMatch &&
      mAttributes &&
      (
        (mAttribA ? (Info->Attribute & EFI_FILE_ARCHIVE ? TRUE : FALSE) : TRUE) &&
      (mAttribH ? (Info->Attribute & EFI_FILE_HIDDEN ? TRUE : FALSE) : TRUE) &&
      (mAttribR ? (Info->Attribute & EFI_FILE_READ_ONLY ? TRUE : FALSE) : TRUE) &&
      (mAttribS ? (Info->Attribute & EFI_FILE_SYSTEM ? TRUE : FALSE) : TRUE) &&
      (mAttribD ? (Info->Attribute & EFI_FILE_DIRECTORY ? TRUE : FALSE) : TRUE)
    )
      ) {
    IsMatch = TRUE;
  } else {
    IsMatch = FALSE;
  }

Done:
  if (NewPattern) {
    FreePool (NewPattern);
  }

  return IsMatch;

}

EFI_STATUS
FindMatchItem (
  IN EFI_FILE_HANDLE   ParentHandle,
  IN CHAR16            *ParentPath,
  IN CHAR16            *Pattern,
  IN BOOLEAN           MatchStratage,
  IN OUT BOOLEAN       *Something,
  IN EFI_FILE_INFO     *Info
  )
{
  EFI_STATUS  Status;
  UINTN       BufSize;

  *Something  = FALSE;
  Status      = ParentHandle->SetPosition (ParentHandle, 0);
  if (EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_LS_CANNOT_ACCESS_DIR), HiiLsHandle, L"ls/dir", ParentPath);
    return Status;
  }
  //
  // loop
  //
  for (;;) {
    //
    // reuse BufSize and Info
    //
    BufSize = SIZE_OF_EFI_FILE_INFO + 1024;
    Status  = ParentHandle->Read (ParentHandle, &BufSize, Info);
    if (EFI_ERROR (Status)) {
      PrintToken (
        STRING_TOKEN (STR_LS_CANNOT_READ_DIR),
        HiiLsHandle,
        L"ls/dir",
        ParentPath,
        Status
        );
      return Status;
    }
    //
    // no more entries
    //
    if (BufSize == 0) {
      break;
    }
    //
    // if file name matches, and attributes right
    //
    if (IsMatch (Info, Pattern, MatchStratage)) {
      *Something = TRUE;
      return EFI_SUCCESS;
    }
  }

  return EFI_SUCCESS;
}

SHELL_FILE_TYPE
GetFileType (
  IN EFI_FILE_INFO *Info
  )
{
  CHAR16  *FileName;

  FileName = Info->FileName;
  if (Info->Attribute & EFI_FILE_DIRECTORY) {
    return TYPE_DIRECTORY;
  } else if (StriCmp (L".nsh", &(FileName[StrLen (FileName) - 4])) == 0) {
    return TYPE_EXECUTABLE;
  } else if (StriCmp (L".efi", &(FileName[StrLen (FileName) - 4])) == 0) {
    return TYPE_EXECUTABLE;
  } else {
    return TYPE_UNKNOWN;
  }
}

VOID
LsDumpFileInfo (
  IN EFI_FILE_INFO *Info
  )
{
  SHELL_FILE_TYPE FileType;

  PrintToken (
    STRING_TOKEN (STR_LS_FOUR_ARGS),
    HiiLsHandle,
    &Info->ModificationTime,
    Info->Attribute & EFI_FILE_DIRECTORY ? L"<DIR>" : L"     ",
    Info->Attribute & EFI_FILE_READ_ONLY ? 'r' : ' ',
    Info->FileSize
    );

  FileType = GetFileType (Info);
  if (!IS_OLD_SHELL) {
    switch (FileType) {
    case TYPE_DIRECTORY:

      if (StrCmp (Info->FileName, L".") && StrCmp (Info->FileName, L"..")) {
        PrintToken (STRING_TOKEN (STR_LS_ONE_DIR_ARG), HiiLsHandle, Info->FileName);
      } else {
        PrintToken (STRING_TOKEN (STR_LS_ONE_NORMALFILE_ARG), HiiLsHandle, Info->FileName);
      }
      break;

    case TYPE_EXECUTABLE:

      PrintToken (STRING_TOKEN (STR_LS_ONE_EXEFILE_ARG), HiiLsHandle, Info->FileName);
      break;

    case TYPE_UNKNOWN:

      PrintToken (STRING_TOKEN (STR_LS_ONE_NORMALFILE_ARG), HiiLsHandle, Info->FileName);
      break;

    }
  }
  //
  //  old shell does not have this feature
  //
  else {
    PrintToken (STRING_TOKEN (STR_LS_ONE_NORMALFILE_ARG), HiiLsHandle, Info->FileName);
  }
}


EFI_STATUS
EFIAPI
InitializeLSGetLineHelp (
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
  return LibCmdGetStringByToken (STRING_ARRAY_NAME, &EfiLsGuid, STRING_TOKEN (STR_LS_LINE_HELP), Str);
}
