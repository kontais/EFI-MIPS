/*++
 
Copyright (c) 2005 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  cp.c

Abstract:

  EFI shell command "cp"

Revision History

--*/

#include "cp.h"

extern UINT8              STRING_ARRAY_NAME[];

//
// Global Variables
//
BOOLEAN                   mCpRecursive;
EFI_DEVICE_PATH_PROTOCOL  *mCpDstDevice;
EFI_DEVICE_PATH_PROTOCOL  *mCpSrcDevice;
BOOLEAN                   mCpReplace;
BOOLEAN                   mCpPrompt;
VOID                      *mCpBuffer;

EFI_HII_HANDLE            HiiCpyHandle;
EFI_GUID                  EfiCpyGuid = EFI_CPY_GUID;
SHELL_VAR_CHECK_ITEM      CpCheckList[] = {
  {
    L"-r",
    0x01,
    0,
    FlagTypeSingle
  },
  {
    L"-q",
    0x02,
    0,
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
// Function Declarations
//
EFI_STATUS
EFIAPI
InitializeCP (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_BOOTSHELL_CODE(
  EFI_APPLICATION_ENTRY_POINT(InitializeCP)
)

EFI_STATUS
EFIAPI
InitializeCP (
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
  EFI_SUCCESS             - The command completed successfully
  EFI_INVALID_PARAMETER   - Command usage error
  EFI_ACCESS_DENIED       - Read-only files/directories can't be modified
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
  Status = LibInitializeStrings (&HiiCpyHandle, STRING_ARRAY_NAME, &EfiCpyGuid);

  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (!EFI_PROPER_VERSION (0, 99)) {
    PrintToken (
      STRING_TOKEN (STR_SHELLENV_GNC_COMMAND_NOT_SUPPORT),
      HiiCpyHandle,
      L"cp/copy",
      EFI_VERSION_0_99 
      );
    Status = EFI_UNSUPPORTED;
    goto Done;
  }

  if (IS_OLD_SHELL) {
    Status = CPMainProcOld (ImageHandle, SystemTable);
  } else {
    Status = CPMainProcNew (ImageHandle, SystemTable, STRING_TOKEN (STR_CP_VERBOSE_HELP));
  }

Done:
  LibUnInitializeStrings ();
  return Status;
}

EFI_STATUS
EFIAPI
InitializeCPGetLineHelp (
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
  return LibCmdGetStringByToken (STRING_ARRAY_NAME, &EfiCpyGuid, STRING_TOKEN (STR_CP_LINE_HELP), Str);
}

EFI_STATUS
CPMainProcNew (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable,
  IN UINT16               VHlpToken
  )
/*++

Routine Description:
  Command entry point. Parses command line arguments and calls internal
  function to perform actual action.

Arguments:
  ImageHandle     The image handle. 
  SystemTable     The system table.
  VHlpToken       The help token.
Returns:
  EFI_SUCCESS             - The command completed successfully
  EFI_INVALID_PARAMETER   - Command usage error
  EFI_ACCESS_DENIED       - Read-only files/directories can't be modified
  EFI_OUT_OF_RESOURCES    - Out of memory
  Other value             - Unknown error
  
--*/
{
  EFI_STATUS              Status;
  CHAR16                  **Argv;
  UINTN                   DstIndex;
  UINTN                   BufferSize;
  UINTN                   NumValidSrc;
  EFI_LIST_ENTRY          SrcList;
  EFI_LIST_ENTRY          DstList;
  EFI_LIST_ENTRY          *Link;
  SHELL_FILE_ARG          *SrcArg;
  SHELL_FILE_ARG          *DstArg;
  EFI_FILE_HANDLE         DstHandle;
  EFI_FILE_HANDLE         OrigDstHandle;
  EFI_FILE_INFO           *FileInfo;
  BOOLEAN                 DstMustDir;
  BOOLEAN                 MutiSrc;
  BOOLEAN                 IsSubPath;
  CHAR16                  *SrcFilePath;
  CHAR16                  *DstFilePath;
  CHAR16                  *DstDev;
  CHAR16                  *SrcDev;
  CHAR16                  *DstFullName;

  SHELL_VAR_CHECK_CODE    RetCode;
  SHELL_ARG_LIST          *Item;
  CHAR16                  *Useful;
  SHELL_VAR_CHECK_PACKAGE ChkPck;


  ZeroMem (&ChkPck, sizeof (SHELL_VAR_CHECK_PACKAGE));
  //
  // Local variable initializations
  //
  Status      = EFI_SUCCESS;
  Argv        = SI->Argv;
  NumValidSrc = 0;
  InitializeListHead (&SrcList);
  InitializeListHead (&DstList);
  Link          = NULL;
  SrcArg        = NULL;
  DstArg        = NULL;
  DstHandle     = NULL;
  DstMustDir    = FALSE;
  MutiSrc       = FALSE;
  SrcFilePath   = NULL;
  DstFilePath   = NULL;
  DstDev        = NULL;
  SrcDev        = NULL;
  DstFullName   = NULL;
  FileInfo      = NULL;

  //
  // Global variable initializations
  //
  mCpRecursive  = FALSE;
  mCpDstDevice  = NULL;
  mCpSrcDevice  = NULL;
  mCpReplace    = FALSE;
  mCpPrompt     = TRUE;
  mCpBuffer     = NULL;

  //
  // verify number of arguments
  //
  RetCode = LibCheckVariables (SI, CpCheckList, &ChkPck, &Useful);
  if (VarCheckOk != RetCode) {
    switch (RetCode) {
    case VarCheckDuplicate:

      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_DUP_FLAG), HiiCpyHandle, L"cp/copy", Useful);
      break;

    case VarCheckUnknown:

      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_UNKNOWN_FLAG), HiiCpyHandle, L"cp/copy", Useful);
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
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiCpyHandle, L"cp/copy");
      Status = EFI_INVALID_PARAMETER;
    } else {
      PrintToken (VHlpToken, HiiCpyHandle);
      Status = EFI_SUCCESS;
    }

    LibCheckVarFreeVarList (&ChkPck);
    return Status;
  }

  if (ChkPck.ValueCount < 1) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_FEW), HiiCpyHandle, L"cp/copy");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }
  //
  // if in script execution, everything is quiet
  //
  if (ShellBatchIsActive ()) {
    mCpReplace  = TRUE;
    mCpPrompt   = FALSE;
  }
  //
  // locate the last file argument as dst, while finding -r & -q
  //
  Item = LibCheckVarGetFlag (&ChkPck, L"-r");
  if (Item) {
    mCpRecursive = TRUE;
  }

  Item = LibCheckVarGetFlag (&ChkPck, L"-q");
  if (Item) {
    mCpReplace  = TRUE;
    mCpPrompt   = FALSE;
  }

  Item = ChkPck.VarList;
  while (Item->Next) {
    Item = Item->Next;
  }

  DstIndex = Item->Index;

  //
  // expand source list but excluding the last file argument
  //
  Item = ChkPck.VarList;
  while (Item && DstIndex != 1) {
    if (DstIndex != Item->Index) {
      Status = ShellFileMetaArg (Item->VarStr, &SrcList);
      if (EFI_ERROR (Status) || IsListEmpty (&SrcList)) {
        PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_CANNOT_OPEN_FILE), HiiCpyHandle, L"cp/copy", Item->VarStr);
        goto Done;
      }
    }

    Item = Item->Next;
  }
  //
  // if no file is expanded to be source
  //
  if (IsListEmpty (&SrcList)) {
    if (DstIndex == 0) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_FEW), HiiCpyHandle, L"cp/copy");
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    } else {
      //
      // only one file argument is specified,
      // Current directory is assumed to be dst
      // using the only file argument as src
      //
      Status = ShellFileMetaArg (Argv[DstIndex], &SrcList);
      if (EFI_ERROR (Status) || IsListEmpty (&SrcList)) {
        PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_CANNOT_OPEN_FILE), HiiCpyHandle, L"cp/copy", Argv[DstIndex]);
        goto Done;
      }
      //
      // using the current dir as the destination directory
      //
      Status = ShellFileMetaArg (L".", &DstList);
      if (EFI_ERROR (Status) || IsListEmpty (&DstList)) {
        PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_CANNOT_OPEN_DIR), HiiCpyHandle, L"cp/copy", L".");
        goto Done;
      }

      if (DstList.Flink->Flink != &DstList) {
        PrintToken (STRING_TOKEN (STR_CP_MULT_DEST), HiiCpyHandle, L"cp/copy");
        Status = EFI_INVALID_PARAMETER;
        goto Done;
      }
    }
  } else {
    //
    // Some file(s) are expanded as source, Destination dir
    // should be only one file or directory
    //
    Useful = Argv[DstIndex];
    while (*Useful) {
      if ('*' == *Useful || '?' == *Useful) {
        PrintToken (STRING_TOKEN (STR_CP_MULT_DEST), HiiCpyHandle, L"cp/copy");
        Status = EFI_INVALID_PARAMETER;
        goto Done;
      }

      Useful++;
    }

    //
    // The last argument is treated as destination dir
    // and it does not uspport wildcard
    //
    Status = ShellFileMetaArgNoWildCard (Argv[DstIndex], &DstList);
    if (EFI_ERROR (Status) || IsListEmpty (&DstList)) {
      PrintToken (STRING_TOKEN (STR_CP_CANNOT_OPEN_DEST), HiiCpyHandle, L"cp/copy", Argv[DstIndex]);
      goto Done;
    }

    if (DstList.Flink->Flink != &DstList) {
      PrintToken (STRING_TOKEN (STR_CP_MULT_DEST), HiiCpyHandle, L"cp/copy");
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }
  }
  //
  // delete duplicated file in SrcList
  //
  ShellDelDupFileArg (&SrcList);

  //
  // At this stage, make sure there is at least one valid src
  //
  for (Link = SrcList.Flink; Link != &SrcList; Link = Link->Flink) {
    SrcArg = CR (Link, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);
    if (SrcArg->Status == EFI_SUCCESS) {
      NumValidSrc++;
    }
  }

  if (0 == NumValidSrc) {
    //
    // can not find any valid source file&directory in source
    //
    PrintToken (STRING_TOKEN (STR_CP_SOURCE_NOT_FOUND), HiiCpyHandle, L"cp/copy");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }
  //
  // Determine if the dst should be a file or a directory
  //
  if (NumValidSrc > 1) {
    //
    // multiple sources, so destination file should be a directory
    //
    MutiSrc     = TRUE;
    DstMustDir  = TRUE;
    //
    // only one source and one destination, if source is a directory,
    // destination must be a directory
    //
  } else {
    //
    // single source becasue NumValidSrc = 1, at this point, src list can't be empty
    //
    SrcArg = CR (SrcList.Flink, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);
    if (SrcArg->Status == EFI_SUCCESS && SrcArg->Info && SrcArg->Info->Attribute & EFI_FILE_DIRECTORY) {
      DstMustDir = TRUE;
    }
  }

  //
  // Open or create the destination if it is not opened in read/write mode
  //
  DstArg = CR (DstList.Flink, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);

  if (EFI_ERROR (DstArg->Status) && MutiSrc) {
    PrintToken (STRING_TOKEN (STR_CP_MULT_SOURCE), HiiCpyHandle, L"cp/copy");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }
  //
  // can not copy a directory without -r
  //
  if (!EFI_ERROR (SrcArg->Status) && SrcArg->Info->Attribute & EFI_FILE_DIRECTORY && !mCpRecursive && !MutiSrc) {
    PrintToken (STRING_TOKEN (STR_CP_CANNOT_DIR_WITHOUT_R), HiiCpyHandle, L"cp/copy", SrcArg->FullName);
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  if (DstArg->Status == EFI_SUCCESS && DstArg->OpenMode & EFI_FILE_MODE_READ && DstArg->OpenMode & EFI_FILE_MODE_WRITE) {
    //
    // Check if we are copying multiple sources to single existing file
    //
    if (DstArg->Info && !(DstArg->Info->Attribute & EFI_FILE_DIRECTORY)) {
      if (MutiSrc) {
        PrintToken (STRING_TOKEN (STR_CP_MULT_SOURCE), HiiCpyHandle, L"cp/copy");
        Status = EFI_INVALID_PARAMETER;
        goto Done;
      }
    }
  } else if (DstArg->Status == EFI_NOT_FOUND) {
    //
    // we try to open the dst
    // the parent of the dst should have been opened once
    // ShellFileMetaArgNoWildCard returns EFI_SUCCESS
    // we don't use library for better performance
    //
    Status = DstArg->Parent->Open (
                              DstArg->Parent,
                              &DstHandle,
                              DstArg->FileName,
                              EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE,
                              DstMustDir ? EFI_FILE_DIRECTORY : 0
                              );
    if (EFI_ERROR (Status)) {
      PrintToken (STRING_TOKEN (STR_CP_CANNOT_CREATE_DEST), HiiCpyHandle, L"cp/copy", DstArg->FullName);
      goto Done;
    }
  } else if (DstArg->OpenMode & EFI_FILE_MODE_READ) {
    PrintToken (STRING_TOKEN (STR_CP_DEST_READ_ONLY), HiiCpyHandle, L"cp/copy");
    Status = EFI_ACCESS_DENIED;
    goto Done;
  } else {
    //
    // something error
    //
    PrintToken (STRING_TOKEN (STR_CP_CANNOT_OPEN_DEST), HiiCpyHandle, L"cp/copy", DstArg->FullName);
    Status = DstArg->Status;
    goto Done;
  }
  //
  // Find out the device path of the destination
  // Note whether or not we create new dst, ParentDevicePath is always valid
  //
  mCpDstDevice = ShellGetMap (DstArg->FullName);

  //
  // Make sure no source is being copied onto itself
  //
  StrTrimRight (DstArg->FullName, '\\');

  for (Link = SrcList.Flink; Link != &SrcList; Link = Link->Flink) {
    SrcArg = CR (Link, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);

    //
    // Find out the device path of the source
    //
    mCpSrcDevice = ShellGetMap (SrcArg->FullName);

    //
    // if devices are different, we need not continue
    //
    if (DevicePathCompare (mCpSrcDevice, mCpDstDevice) != 0) {
      continue;
    }
    //
    // Trim the last \ in srource file
    //
    StrTrimRight (SrcArg->FullName, '\\');

    //
    // skip device name for source and destination: fsX:\filepath --> filepath
    //
    for (SrcFilePath = SrcArg->FullName; *SrcFilePath && *SrcFilePath != ':'; SrcFilePath++) {
      ;
    }

    SrcFilePath++;

    for (DstFilePath = DstArg->FullName; *DstFilePath && *DstFilePath != ':'; DstFilePath++) {
      ;
    }

    DstFilePath++;
    //
    // directory can not copy to itself
    //
    if (SrcArg->Status == EFI_SUCCESS &&
        (SrcArg->Info->Attribute & EFI_FILE_DIRECTORY) &&
        (DstArg->Status == EFI_SUCCESS && (DstArg->Info->Attribute & EFI_FILE_DIRECTORY) || DstMustDir && DstHandle)
        ) {
      if (StrCmp (SrcFilePath, DstFilePath) == 0) {
        PrintToken (STRING_TOKEN (STR_CP_CANNOT_COPY_ITSELF), HiiCpyHandle, L"cp/copy", SrcArg->FullName);
        Status = EFI_INVALID_PARAMETER;
        goto Done;
      }
    }

    //
    // if target is a directory and source is a file,
    // we need to compose a full name of dir\file for the source
    //
    if (SrcArg->Status == EFI_SUCCESS &&
      (DstArg->Status == EFI_SUCCESS && (DstArg->Info->Attribute & EFI_FILE_DIRECTORY) || DstMustDir && DstHandle)) {
      if (DstFullName) {
        FreePool (DstFullName);
      }

      BufferSize  = StrSize (DstArg->FullName) + 2 + StrSize (SrcArg->FileName) + 2;
      DstFullName = AllocateZeroPool (BufferSize);
      if (!DstFullName) {
        PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_OUT_RESOURCE), HiiCpyHandle, L"cp/copy");
        Status = EFI_OUT_OF_RESOURCES;
        goto Done;
      }

      SPrint (DstFullName, BufferSize, L"%s\\%s", DstArg->FullName, SrcArg->FileName);

      //
      // skip device name fsX:
      //
      for (DstFilePath = DstFullName; *DstFilePath && *DstFilePath != ':'; DstFilePath++) {
        ;
      }
    }

    if (StrCmp (SrcFilePath, DstFilePath) == 0) {
      PrintToken (STRING_TOKEN (STR_CP_CANNOT_COPY_ITSELF), HiiCpyHandle, L"cp/copy", SrcArg->FullName);
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }
  }
  //
  // Make sure no source is being copied onto its sub if -r is specified
  //
  if (mCpRecursive) {
    for (Link = SrcList.Flink; Link != &SrcList; Link = Link->Flink) {
      SrcArg = CR (Link, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);

      //
      // Find out the device path of the source
      //
      mCpSrcDevice = ShellGetMap (SrcArg->FullName);

      for (SrcFilePath = SrcArg->FullName; *SrcFilePath && *SrcFilePath != ':'; SrcFilePath++) {
        ;
      }

      for (DstFilePath = DstArg->FullName; *DstFilePath && *DstFilePath != ':'; DstFilePath++) {
        ;
      }

      if (DevicePathCompare (mCpSrcDevice, mCpDstDevice) == 0) {
        Status = LibIsSubPath (SrcArg->FullName, DstArg->FullName, &IsSubPath);
        if (!EFI_ERROR (Status) && IsSubPath) {
          PrintToken (
            STRING_TOKEN (STR_CP_CANNOT_COPY_SUBDIR),
            HiiCpyHandle,
            L"cp/copy",
            SrcArg->FullName
            );
          Status = EFI_INVALID_PARAMETER;
          goto Done;
        }
      }
    }
  }
  //
  // ready to copy
  // allocate pool for following use
  //
  mCpBuffer = AllocatePool (CP_BLOCK_SIZE);
  if (!mCpBuffer) {
    PrintToken (STRING_TOKEN (STR_CP_OUT_OF_MEM), HiiCpyHandle, L"cp/copy");
    Status = EFI_OUT_OF_RESOURCES;
    goto Done;
  }

  OrigDstHandle = DstHandle;

  //
  // for each src, call CpCopy to copy it
  //
  for (Link = SrcList.Flink; Link != &SrcList; Link = Link->Flink) {
    //
    // Break the execution?
    //
    if (GetExecutionBreak ()) {
      goto Done;
    }

    if (mCpPrompt == FALSE && mCpReplace == FALSE) {
      goto Done;
    }

    SrcArg = CR (Link, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);

    //
    // Check if SrcHandle valid
    //
    if (SrcArg->Status != EFI_SUCCESS || !SrcArg->Handle) {
      PrintToken (STRING_TOKEN (STR_CP_CANNOT_ACCESS), HiiCpyHandle, L"cp/copy", SrcArg->FullName);
      Status = SrcArg->Status;
      goto Done;
    }

    if (SrcArg->Info->Attribute & EFI_FILE_DIRECTORY && !mCpRecursive) {
      PrintToken (STRING_TOKEN (STR_CP_CANNOT_DIR_WITHOUT_R), HiiCpyHandle, L"cp/copy", SrcArg->FullName);
      Status = EFI_INVALID_PARAMETER;
      if (MutiSrc) {
        continue;
      } else {
        goto Done;
      }
    }

    if (SrcArg->Info &&
        !EFI_ERROR (DstArg->Status) &&
        SrcArg->Info->Attribute & EFI_FILE_DIRECTORY &&
        DstArg->Info->Attribute & EFI_FILE_DIRECTORY
        ) {
      //
      // dir -> dir
      //
      BufferSize  = StrSize (DstArg->FullName) + 2 + StrSize (SrcArg->FileName) + 2;
      DstFullName = AllocateZeroPool (BufferSize);
      if (!DstFullName) {
        PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_OUT_RESOURCE), HiiCpyHandle, L"cp/copy");
        Status = EFI_OUT_OF_RESOURCES;
        goto Done;
      }

      SPrint (DstFullName, BufferSize, L"%s\\%s", DstArg->FullName, SrcArg->FileName);

      //
      // first check if the directory exists
      //
      Status = DstArg->Handle->Open (
                                DstArg->Handle,
                                &DstHandle,
                                SrcArg->FileName,
                                EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE,
                                0
                                );

      if (!EFI_ERROR (Status)) {
        FileInfo = LibGetFileInfo (DstHandle);
        if (!(FileInfo->Attribute & EFI_FILE_DIRECTORY)) {
          PrintToken (
            STRING_TOKEN (STR_CP_CANNOT_OVERWIRTE_FILE_WITH_DIR),
            HiiCpyHandle,
            L"cp/copy",
            SrcArg->FileName
            );
          FreePool (FileInfo);
          goto Done;
        }
      }

      Status = DstArg->Handle->Open (
                                DstArg->Handle,
                                &DstHandle,
                                SrcArg->FileName,
                                EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE,
                                EFI_FILE_DIRECTORY
                                );

    }
    //
    // CpCopy requires SrcHandle and DstHandle be valid
    //
    if (DstHandle) {
      //
      // we've created a new file/dir as dst
      //
      Status = CpCopy (
                SrcArg->Handle,
                DstHandle,
                SrcArg->FullName,
                (
                !(DstArg->Status & EFI_NOT_FOUND) &&
                SrcArg->Info->Attribute & EFI_FILE_DIRECTORY
                ) ? DstFullName : DstArg->FullName,
                FALSE
                );
      if (EFI_ERROR (Status)) {
        //
        // we don't care the return status
        //
        DstHandle->Delete (DstHandle);
        DstHandle = NULL;
        goto Done;
      }
    } else {
      //
      // dst already exists
      //
      Status = CpCopy (
                SrcArg->Handle,
                DstArg->Handle,
                SrcArg->FullName,
                DstArg->FullName,
                TRUE
                );
      if (EFI_ERROR (Status)) {
        //
        // Error message is supposed to be displayed in CpCopy
        //
        goto Done;
      }
    }
    //
    // restore back the orignal dst handle
    //
    DstHandle = OrigDstHandle;
  }

Done:
  if (DstDev) {
    FreePool (DstDev);
  }

  if (SrcDev) {
    FreePool (SrcDev);
  }

  if (DstFullName) {
    FreePool (DstFullName);
  }

  if (DstHandle) {
    DstHandle->Close (DstHandle);
  }

  if (mCpBuffer) {
    FreePool (mCpBuffer);
  }

  ShellFreeFileList (&SrcList);
  ShellFreeFileList (&DstList);
  LibCheckVarFreeVarList (&ChkPck);

  return Status;
}

EFI_STATUS
CpCopy (
  IN EFI_FILE_HANDLE      SrcHandle,
  IN EFI_FILE_HANDLE      DstHandle,
  IN CHAR16               *SrcPath,
  IN CHAR16               *DstPath,
  IN BOOLEAN              DstExists
  )
/*++

Routine Description:

  Performs copy operation. 
  Four possibilities are treated separately:
  File->File, File->Directory, Directory->File, Directory->Directory

Arguments:

  SrcHandle       Handle of source file or directory 
  DstHandle       Handle of destination file or directory
  SrcPath         Path of source file or directory
  DstPath         Path of destination file or directory
  DstExists       Indicates if destination already exists

Returns:

  EFI_SUCCESS     The function completed successfully.
  Other value     The function failed due to some reason.
  
--*/
{
  UINTN                           Index;
  EFI_STATUS                      Status;
  EFI_FILE_INFO                   *SrcInfo;
  EFI_FILE_INFO                   *DstInfo;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *DstVol;
  EFI_FILE_SYSTEM_INFO            *DstFsInfo;
  CHAR16                          InputString[10];
  UINTN                           BufSize;
  UINTN                           WriteSize;
  UINTN                           NameSize;
  EFI_FILE_HANDLE                 SubSrcHandle;
  EFI_FILE_HANDLE                 SubDstHandle;
  CHAR16                          *SubSrcPath;
  CHAR16                          *SubDstPath;
  BOOLEAN                         SubDstExists;
  EFI_FILE_INFO                   *InfoBuffer;
  UINTN                           InfoBufSize;
  EFI_FILE_INFO                   *SubSrcInfo;
  EFI_FILE_INFO                   *FileInfo;

#define MAX_PROMPT_TIMES  1000
  //
  // Local variable initializations
  //
  Status            = EFI_SUCCESS;

  SrcInfo           = NULL;
  DstInfo           = NULL;
  FileInfo          = NULL;
  DstVol            = NULL;
  DstFsInfo         = NULL;
  InputString[0]    = 0;
  BufSize           = 0;
  WriteSize         = 0;
  SubSrcHandle      = NULL;
  SubDstHandle      = NULL;
  SubSrcPath        = NULL;
  SubDstPath        = NULL;
  SubDstExists      = FALSE;
  InfoBuffer        = NULL;
  InfoBufSize       = 0;
  SubSrcInfo        = NULL;

  //
  // Check if we are copying the same file/dir
  //
  if (StrCmp (SrcPath, DstPath) == 0) {
    PrintToken (STRING_TOKEN (STR_CP_CANNOT_COPY_ITSELF), HiiCpyHandle, L"cp/copy", SrcPath);
    Status = EFI_INVALID_PARAMETER;
    goto CopyDone;
  }
  //
  // Get info from both files
  //
  SrcInfo = LibGetFileInfo (SrcHandle);
  if (!SrcInfo) {
    Status = EFI_OUT_OF_RESOURCES;
    PrintToken (STRING_TOKEN (STR_CP_ACCESS_ERROR), HiiCpyHandle, L"cp/copy", SrcPath);
    goto CopyDone;
  }

  DstInfo = LibGetFileInfo (DstHandle);
  if (!DstInfo) {
    Status = EFI_OUT_OF_RESOURCES;
    PrintToken (STRING_TOKEN (STR_CP_ACCESS_ERROR), HiiCpyHandle, L"cp/copy", DstPath);
    goto CopyDone;
  }
  //
  // file -> file
  //
  if (!(SrcInfo->Attribute & EFI_FILE_DIRECTORY) && !(DstInfo->Attribute & EFI_FILE_DIRECTORY)) {
    //
    // if already exists, prompt
    //
    if (DstExists && mCpPrompt) {
      InputString[0] = 0;
      PrintToken (STRING_TOKEN (STR_CP_OVERWRITE), HiiCpyHandle, DstPath);

      Input (L"", InputString, 2);
      Print (L"\n");

      Index = 0;
      while (Index < MAX_PROMPT_TIMES) {
        if (InputString[0] == 'Y' || InputString[0] == 'y') {
          mCpPrompt   = TRUE;
          mCpReplace  = TRUE;
          break;
        } else if (InputString[0] == 'N' || InputString[0] == 'n') {
          mCpPrompt   = TRUE;
          mCpReplace  = FALSE;
          break;
        } else if (InputString[0] == 'A' || InputString[0] == 'a') {
          mCpPrompt   = FALSE;
          mCpReplace  = TRUE;
          break;
        } else if (InputString[0] == 'C' || InputString[0] == 'c') {
          mCpPrompt   = FALSE;
          mCpReplace  = FALSE;
          break;
        }

        InputString[0] = 0;
        PrintToken (STRING_TOKEN (STR_CP_OVERWRITE), HiiCpyHandle, DstPath);
        Input (L"", InputString, 2);
        Print (L"\n");
        Index++;
      }
    }

    if (!mCpReplace && DstExists) {
      goto CopyDone;
    }
    //
    // set both positions to beginning
    //
    PrintToken (STRING_TOKEN (STR_CP_COPYING), HiiCpyHandle, SrcPath, DstPath);

    Status = LibSetPosition (SrcHandle, 0);
    if (EFI_ERROR (Status)) {
      PrintToken (STRING_TOKEN (STR_CP_SET_POS_ERROR), HiiCpyHandle, L"cp/copy", SrcPath);
      goto CopyDone;
    }

    Status = LibSetPosition (DstHandle, 0);
    if (EFI_ERROR (Status)) {
      PrintToken (STRING_TOKEN (STR_CP_SET_POS_ERROR), HiiCpyHandle, L"cp/copy", DstPath);
      goto CopyDone;
    }
    //
    // Set destination file size to the same size as source file
    //
    DstInfo->FileSize     = SrcInfo->FileSize;
    DstInfo->PhysicalSize = SrcInfo->PhysicalSize;

    Status                = LibSetFileInfo (DstHandle, (UINTN) DstInfo->Size, DstInfo);
    if (EFI_ERROR (Status)) {
      PrintToken (STRING_TOKEN (STR_CP_ACCESS_ERROR_2), HiiCpyHandle, L"cp/copy", DstPath);
      goto CopyDone;
    }

    DstInfo->Attribute = EFI_FILE_ARCHIVE;
    //
    // Copy it
    //
    for (;;) {
      //
      // Break the execution?
      //
      if (GetExecutionBreak ()) {
        goto CopyDone;
      }

      BufSize = CP_BLOCK_SIZE;

      Status  = LibReadFile (SrcHandle, &BufSize, mCpBuffer);
      if (EFI_ERROR (Status)) {
        PrintToken (STRING_TOKEN (STR_CP_READ_ERROR), HiiCpyHandle, L"cp/copy");
        goto CopyDone;
      }
      //
      // end of file
      //
      if (!BufSize) {
        break;
      }

      WriteSize = BufSize;

      Status    = LibWriteFile (DstHandle, &WriteSize, mCpBuffer);
      if (EFI_ERROR (Status)) {
        PrintToken (STRING_TOKEN (STR_CP_WRITE_ERROR), HiiCpyHandle, L"cp/copy");
        goto CopyDone;
      }

      if (WriteSize != BufSize) {
        Status = EFI_OUT_OF_RESOURCES;
        PrintToken (STRING_TOKEN (STR_CP_WRITE_ERROR_DISK_FULL), HiiCpyHandle, L"cp/copy");
        goto CopyDone;
      }
    }
    //
    // Set destination modification time as the same as the source
    //
    FreePool (DstInfo);
    DstInfo = LibGetFileInfo (DstHandle);
    if (!DstInfo) {
      Status = EFI_OUT_OF_RESOURCES;
      PrintToken (STRING_TOKEN (STR_CP_CANNOT_ACCESS), HiiCpyHandle, L"cp/copy", DstPath);
      goto CopyDone;
    }

    DstInfo->ModificationTime = SrcInfo->ModificationTime;
    Status                    = LibSetFileInfo (DstHandle, (UINTN) DstInfo->Size, DstInfo);
    if (EFI_ERROR (Status)) {
      PrintToken (STRING_TOKEN (STR_CP_CANNOT_ACCESS), HiiCpyHandle, L"cp/copy", DstPath);
      goto CopyDone;
    }

    Status = LibFlushFile (DstHandle);

    if (EFI_ERROR (Status)) {
      PrintToken (STRING_TOKEN (STR_CP_WRITE_ERROR), HiiCpyHandle, L"cp/copy");
      goto CopyDone;
    } else {
      PrintToken (STRING_TOKEN (STR_CP_OK), HiiCpyHandle);
    }
  }
  //
  // file -> dir
  //
  else if (!(SrcInfo->Attribute & EFI_FILE_DIRECTORY) && (DstInfo->Attribute & EFI_FILE_DIRECTORY)) {

    PrintToken (STRING_TOKEN (STR_CP_COPYING), HiiCpyHandle, SrcPath, DstPath);
    //
    // see if the sub destination already exists
    //
    SubDstExists = TRUE;
    Status = DstHandle->Open (
                          DstHandle,
                          &SubDstHandle,
                          SrcInfo->FileName,
                          EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE,
                          0
                          );

    if (EFI_ERROR (Status)) {
      SubDstExists = FALSE;
      Status = DstHandle->Open (
                            DstHandle,
                            &SubDstHandle,
                            SrcInfo->FileName,
                            EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE,
                            0
                            );

      if (EFI_ERROR (Status)) {
        PrintToken (
          STRING_TOKEN (STR_CP_CANNOT_OPEN_CREATE),
          HiiCpyHandle,
          L"cp/copy",
          SrcInfo->FileName,
          DstPath
          );
        SubDstHandle = NULL;
        goto CopyDone;
      }
    } else {
      FileInfo = LibGetFileInfo (SubDstHandle);
      if (FileInfo->Attribute & EFI_FILE_DIRECTORY) {
        PrintToken (
          STRING_TOKEN (STR_CP_CANNOT_OVERWIRTE_DIR_WITH_FILE),
          HiiCpyHandle,
          L"cp/copy",
          FileInfo->FileName
          );
        goto CopyDone;
      }
    }
    //
    // Compose new path for sub dst
    //
    NameSize    = StrSize (DstPath) + 1 + StrSize (SrcInfo->FileName) + 2;
    SubDstPath  = AllocatePool (NameSize);
    if (!SubDstPath) {
      Status = EFI_OUT_OF_RESOURCES;
      PrintToken (STRING_TOKEN (STR_CP_OUT_OF_MEM), HiiCpyHandle, L"cp/copy");
      goto CopyDone;
    }

    SPrint (SubDstPath, NameSize, L"%s\\%s", DstPath, SrcInfo->FileName);

    Status = CpCopy (
              SrcHandle,
              SubDstHandle,
              SrcPath,
              SubDstPath,
              SubDstExists
              );
    if (EFI_ERROR (Status)) {
      if (!SubDstExists) {
        //
        // we don't care return status
        //
        SubDstHandle->Delete (SubDstHandle);
        SubDstHandle = NULL;
      }

      goto CopyDone;
    }
  }
  //
  // dir -> file
  //
  else if ((SrcInfo->Attribute & EFI_FILE_DIRECTORY) && !(DstInfo->Attribute & EFI_FILE_DIRECTORY)) {
    PrintToken (
      STRING_TOKEN (STR_CP_CANNOT_COPY_FROM),
      HiiCpyHandle,
      L"cp/copy",
      SrcPath,
      DstPath
      );
    Status = EFI_INVALID_PARAMETER;
    goto CopyDone;
  }
  //
  // dir -> dir
  //
  else if ((SrcInfo->Attribute & EFI_FILE_DIRECTORY) && (DstInfo->Attribute & EFI_FILE_DIRECTORY)) {
    if (!mCpRecursive) {
      PrintToken (STRING_TOKEN (STR_CP_CANNOT_DIR_WITHOUT_R), HiiCpyHandle, L"cp/copy", SrcPath);
      Status = EFI_SUCCESS;
      goto CopyDone;
    }
    //
    // Set position to 0 in source
    //
    PrintToken (STRING_TOKEN (STR_CP_COPYING), HiiCpyHandle, SrcPath, DstPath);
    Status = SrcHandle->SetPosition (SrcHandle, 0);
    if (EFI_ERROR (Status)) {
      PrintToken (
        STRING_TOKEN (STR_CP_ACCESS_ERROR_2),
        HiiCpyHandle,
        L"cp/copy",
        SrcPath
        );
      goto CopyDone;
    }
    //
    // loop
    //
    for (;;) {
      //
      // Break the execution?
      //
      if (GetExecutionBreak ()) {
        goto CopyDone;
      }
      //
      // reuse Info Buffer to store entries of source dir
      //
      InfoBufSize = SIZE_OF_EFI_FILE_INFO + 1024;

      if (!InfoBuffer) {
        InfoBuffer = AllocatePool (InfoBufSize);
        if (!InfoBuffer) {
          Status = EFI_OUT_OF_RESOURCES;
          PrintToken (STRING_TOKEN (STR_CP_OUT_OF_MEM), HiiCpyHandle, L"cp/copy");
          goto CopyDone;
        }
      }

      Status = SrcHandle->Read (SrcHandle, &InfoBufSize, InfoBuffer);
      if (EFI_ERROR (Status)) {
        PrintToken (
          STRING_TOKEN (STR_CP_READ_ERROR_2),
          HiiCpyHandle,
          L"cp/copy",
          SrcPath
          );
        goto CopyDone;
      }
      //
      // no more entries
      //
      if (InfoBufSize == 0) {
        break;
      }
      //
      // skip
      //
      if (StriCmp (InfoBuffer->FileName, L".") == 0 || StriCmp (InfoBuffer->FileName, L"..") == 0) {
        continue;
      }
      //
      // compose SubSrcPath
      //
      if (SubSrcPath) {
        FreePool (SubSrcPath);
      }

      NameSize    = StrSize (SrcPath) + 1 + StrSize (InfoBuffer->FileName) + 2;
      SubSrcPath  = AllocatePool (NameSize);

      if (!SubSrcPath) {
        Status = EFI_OUT_OF_RESOURCES;
        PrintToken (STRING_TOKEN (STR_CP_OUT_OF_MEM), HiiCpyHandle, L"cp/copy");
        goto CopyDone;
      }

      SPrint (SubSrcPath, NameSize, L"%s\\%s", SrcPath, InfoBuffer->FileName);

      //
      // compose SubDstPath
      //
      if (SubDstPath) {
        FreePool (SubDstPath);
      }

      SubDstPath = AllocateZeroPool (StrSize (DstPath) + 1 + StrSize (InfoBuffer->FileName) + 2);

      if (!SubDstPath) {
        Status = EFI_OUT_OF_RESOURCES;
        PrintToken (STRING_TOKEN (STR_CP_OUT_OF_MEM), HiiCpyHandle, L"cp/copy");
        goto CopyDone;
      }

      StrCpy (SubDstPath, DstPath);
      if (SubDstPath[StrLen (SubDstPath) - 1] != '\\') {
        StrCat (SubDstPath, L"\\");
      }

      StrCat (SubDstPath, InfoBuffer->FileName);

      //
      // Open SubSrcHandle
      //
      Status = SrcHandle->Open (
                            SrcHandle,
                            &SubSrcHandle,
                            InfoBuffer->FileName,
                            EFI_FILE_MODE_READ,
                            0
                            );
      if (EFI_ERROR (Status)) {
        PrintToken (
          STRING_TOKEN (STR_CP_CANNOT_ACCESS),
          HiiCpyHandle,
          L"cp/copy",
          SubSrcPath
          );
        SubSrcHandle = NULL;
        goto CopyDone;
      }
      //
      // Get info of sub source
      //
      SubSrcInfo = LibGetFileInfo (SubSrcHandle);
      if (!SubSrcInfo) {
        Status = EFI_OUT_OF_RESOURCES;
        PrintToken (STRING_TOKEN (STR_CP_ACCESS_ERROR), HiiCpyHandle, L"cp/copy", SubDstPath);
        goto CopyDone;
      }
      //
      // Copy sub source to sub destination
      //
      if (!mCpRecursive && (SubSrcInfo->Attribute & EFI_FILE_DIRECTORY)) {
        //
        // Can't copy Sub directory.
        //
        continue;
      } else {
        //
        // open sub destination, see if it already exists
        //
        SubDstExists = TRUE;
        Status = DstHandle->Open (
                              DstHandle,
                              &SubDstHandle,
                              InfoBuffer->FileName,
                              EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE,
                              0
                              );
        if (EFI_ERROR (Status)) {
          SubDstExists = FALSE;

          if (SubSrcInfo->Attribute & EFI_FILE_DIRECTORY) {
            PrintToken (STRING_TOKEN (STR_CP_MAKING_DIR), HiiCpyHandle, SubDstPath);
          }

          Status = DstHandle->Open (
                                DstHandle,
                                &SubDstHandle,
                                InfoBuffer->FileName,
                                EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE,
                                (SubSrcInfo->Attribute & EFI_FILE_DIRECTORY) ? EFI_FILE_DIRECTORY : 0
                                );

          if (EFI_ERROR (Status)) {
            PrintToken (
              STRING_TOKEN (STR_CP_CANNOT_OPEN_CREATE_2),
              HiiCpyHandle,
              L"cp/copy",
              InfoBuffer->FileName,
              DstPath
              );
            SubDstHandle = NULL;
            goto CopyDone;
          }
        }

        Status = CpCopy (
                  SubSrcHandle,
                  SubDstHandle,
                  SubSrcPath,
                  SubDstPath,
                  SubDstExists
                  );

        if (EFI_ERROR (Status)) {
          if (!SubDstExists) {
            //
            // we don't care return status
            //
            SubDstHandle->Delete (SubDstHandle);
            SubDstHandle = NULL;
          }

          goto CopyDone;
        } else {
          if (SubSrcHandle) {
            SubSrcHandle->Close (SubSrcHandle);
            SubSrcHandle = NULL;
          }

          if (SubDstHandle) {
            SubDstHandle->Close (SubDstHandle);
            SubDstHandle = NULL;
          }
        }
      }

      FreePool (SubSrcInfo);
      SubSrcInfo = NULL;
    }
  }

CopyDone:
  if (SrcInfo) {
    FreePool (SrcInfo);
  }

  if (DstInfo) {
    FreePool (DstInfo);
  }

  if (FileInfo) {
    FreePool (FileInfo);
  }

  if (DstFsInfo) {
    FreePool (DstFsInfo);
  }

  if (SubSrcPath) {
    FreePool (SubSrcPath);
  }

  if (SubDstPath) {
    FreePool (SubDstPath);
  }

  if (InfoBuffer) {
    FreePool (InfoBuffer);
  }

  if (SubDstHandle) {
    SubDstHandle->Close (SubDstHandle);
  }

  if (SubSrcHandle) {
    SubSrcHandle->Close (SubSrcHandle);
  }

  if (SubSrcInfo) {
    FreePool (SubSrcInfo);
  }

  return Status;
}

EFI_STATUS
CPMainProcOld (
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
  EFI_SUCCESS             - The command completed successfully
  EFI_INVALID_PARAMETER   - Command usage error
  EFI_ACCESS_DENIED       - Read-only files/directories can't be modified
  EFI_OUT_OF_RESOURCES    - Out of memory
  Other value             - Unknown error
  
--*/
{
  EFI_STATUS      Status;
  CHAR16          **Argv;
  UINTN           Argc;
  UINTN           Index;
  UINTN           DstIndex;
  EFI_LIST_ENTRY  SrcList;
  EFI_LIST_ENTRY  DstList;
  EFI_LIST_ENTRY  *Link;
  SHELL_FILE_ARG  *SrcArg;
  SHELL_FILE_ARG  *DstArg;
  EFI_FILE_HANDLE DstHandle;
  BOOLEAN         DstDir;
  CHAR16          *SrcFilePath;
  CHAR16          *DstFilePath;
  UINTN           SrcDevPathLen;
  UINTN           DstDevPathLen;
  CHAR16          *DstDev;
  CHAR16          *SrcDev;
  CHAR16          *DstFullName;
  CHAR16          TempChar;

  //
  // Local variable initializations
  //
  Status    = EFI_SUCCESS;
  Argv      = SI->Argv;
  Argc      = SI->Argc;
  Index     = 0;
  DstIndex  = 0;
  InitializeListHead (&SrcList);
  InitializeListHead (&DstList);
  Link          = NULL;
  SrcArg        = NULL;
  DstArg        = NULL;
  DstHandle     = NULL;
  DstDir        = FALSE;
  SrcFilePath   = NULL;
  DstFilePath   = NULL;
  SrcDevPathLen = 0;
  DstDevPathLen = 0;
  DstDev        = NULL;
  SrcDev        = NULL;
  DstFullName   = NULL;

  //
  // Global variable initializations
  //
  mCpRecursive  = FALSE;
  mCpDstDevice  = NULL;
  mCpSrcDevice  = NULL;
  mCpReplace    = FALSE;
  mCpPrompt     = TRUE;
  mCpBuffer     = NULL;

  //
  // verify number of arguments
  //
  if (Argc < 2) {
    Print (L"cp: Too few arguments\n");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }
  //
  // Parse command line arguments
  //
  for (Index = 1; Index < Argc; Index += 1) {
    if (StrLen (Argv[Index]) == 0) {
      Print (L"cp: Argument with zero length is not allowed\n");
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }
  }
  //
  // if in script execution, everything is quiet
  //
  if (ShellBatchIsActive ()) {
    mCpReplace  = TRUE;
    mCpPrompt   = FALSE;
  }
  //
  // locate the last file argument as dst, while finding -r & -q
  //
  for (Index = 1; Index < Argc; Index++) {
    if (Argv[Index][0] == '-' && (Argv[Index][1] == 'r' || Argv[Index][1] == 'R')) {
      mCpRecursive = TRUE;
    } else if (Argv[Index][0] == '-' && (Argv[Index][1] == 'q' || Argv[Index][1] == 'Q')) {
      mCpReplace  = TRUE;
      mCpPrompt   = FALSE;
    } else if (Argv[Index][0] == '-' && (Argv[Index][1] == '?')) {
      PrintToken (STRING_TOKEN (STR_NO_HELP), HiiCpyHandle);
      goto Done;
    } else {
      DstIndex = Index;
    }
  }
  //
  // expand source list but excluding the last file argument
  //
  for (Index = 1; Index < Argc; Index++) {
    if (Argv[Index][0] == '-' &&
        (Argv[Index][1] == 'r' || Argv[Index][1] == 'R' || Argv[Index][1] == 'q' || Argv[Index][1] == 'Q')
        ) {
      ;
    } else if (Index != DstIndex) {
      Status = ShellFileMetaArg (Argv[Index], &SrcList);
      if (EFI_ERROR (Status) || IsListEmpty (&SrcList)) {
        Print (L"cp: Cannot open %hs - %r\n", Argv[Index], Status);
        goto Done;
      }
    }
  }
  //
  // if no file is expanded to be source
  //
  if (IsListEmpty (&SrcList)) {
    if (DstIndex <= 0) {
      Print (L"cp: Too few arguments\n");
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    } else {
      //
      // only one file argument is specified,
      // 'current dir' is assumed to be dst
      //
      //
      // using the only file argument as src
      //
      Status = ShellFileMetaArg (Argv[DstIndex], &SrcList);
      if (EFI_ERROR (Status) || IsListEmpty (&SrcList)) {
        Print (L"cp: Cannot open %hs - %r\n", Argv[DstIndex], Status);
        goto Done;
      }
      //
      // using the current dir as the dst
      //
      Status = ShellFileMetaArg (L".", &DstList);
      if (EFI_ERROR (Status) || IsListEmpty (&DstList)) {
        Print (
          L"cp: Cannot open current directory as destination - %r\n",
          Status
          );
        goto Done;
      }

      if (DstList.Flink->Flink != &DstList) {
        Print (L"cp: Multiple destinations are not allowed\n");
        Status = EFI_INVALID_PARAMETER;
        goto Done;
      }
    }
  } else {
    //
    // Some file(s) are expanded as source
    //
    //
    // The last argument is treated as dst
    //
    Status = ShellFileMetaArg (Argv[DstIndex], &DstList);
    if (EFI_ERROR (Status) || IsListEmpty (&DstList)) {
      Print (
        L"cp: Cannot open destination - %r\n",
        Status
        );
      goto Done;
    }

    if (DstList.Flink->Flink != &DstList) {
      Print (L"cp: Multiple destinations are not allowed\n");
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }
  }
  //
  // At this stage, make sure there is at least one valid src
  //
  for (Link = SrcList.Flink; Link != &SrcList; Link = Link->Flink) {
    SrcArg = CR (Link, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);
    if (SrcArg->Status == EFI_SUCCESS) {
      break;
    }
  }

  if (Link == &SrcList) {
    Print (L"cp: Source file not found\n");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }
  //
  // Determine if the dst should be a file or a directory
  //
  if (SrcList.Flink->Flink != &SrcList) {
    //
    // multiple sources
    //
    DstDir = TRUE;
  } else {
    //
    // single source, at this point, src list can't be empty
    //
    SrcArg = CR (
              SrcList.Flink,
              SHELL_FILE_ARG,
              Link,
              SHELL_FILE_ARG_SIGNATURE
              );
    if (SrcArg->Status == EFI_SUCCESS && SrcArg->Info && SrcArg->Info->Attribute & EFI_FILE_DIRECTORY) {
      DstDir = TRUE;
    }
  }
  //
  // Open or create the destination if it is not opened in read/write mode
  //
  if (DstList.Flink->Flink != &DstList) {
    Print (L"cp: Multiple destinations are not allowed\n");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  DstArg = CR (DstList.Flink, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);

  if (DstArg->Status == EFI_SUCCESS && DstArg->OpenMode & EFI_FILE_MODE_READ && DstArg->OpenMode & EFI_FILE_MODE_WRITE) {
    //
    // Check if we are copying multiple sources to single existing file
    //
    if (DstArg->Info && !(DstArg->Info->Attribute & EFI_FILE_DIRECTORY)) {
      if (SrcList.Flink->Flink != &SrcList) {
        Print (L"cp: Cannot copy multiple sources to single existing file\n");
        Status = EFI_INVALID_PARAMETER;
        goto Done;
      }
    }
  } else if (DstArg->Status == EFI_NOT_FOUND) {
    //
    // check if destination name contains wildcards which is forbidden
    //
    for (Index = 0; Index < StrLen (DstArg->FileName); Index++) {
      if (DstArg->FileName[Index] == '*' || DstArg->FileName[Index] == '?' || DstArg->FileName[Index] == '[') {
        Print (L"cp: Multiple destinations are not allowed\n");
        Status = EFI_INVALID_PARAMETER;
        goto Done;
      }
    }
    //
    // we try to open the dst
    // the parent of the dst should have been opened once ShellFileMetaArg
    // returns EFI_SUCCESS
    //
    Status = DstArg->Parent->Open (
                              DstArg->Parent,
                              &DstHandle,
                              DstArg->FileName,
                              EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE,
                              DstDir ? EFI_FILE_DIRECTORY : 0
                              );
    if (EFI_ERROR (Status)) {
      Print (
        L"cp: Cannot create destination %hs - %r\n",
        DstArg->FullName,
        Status
        );
      goto Done;
    }
  } else if (DstArg->OpenMode & EFI_FILE_MODE_READ) {
    Print (L"cp: Destination is read only or write protected\n");
    Status = EFI_ACCESS_DENIED;
    goto Done;
  } else {
    //
    // something error
    //
    Print (
      L"cp: Cannot open %hs - %r\n",
      DstArg->FullName,
      DstArg->Status
      );
    Status = DstArg->Status;
    goto Done;
  }
  //
  // Find out the device path of the destination
  // Note whether or not we create new dst, ParentDevicePath is always valid
  //
  DstDev = StrDuplicate (DstArg->FullName);
  for (Index = 0; Index < StrLen (DstDev); Index++) {
    if (DstDev[Index] == ':') {
      break;
    }
  }

  DstDev[Index] = 0;

  mCpDstDevice  = (EFI_DEVICE_PATH_PROTOCOL *) ShellGetMap (DstDev);

  //
  // Make sure no source is being copied onto itself
  //
  for (Link = SrcList.Flink; Link != &SrcList; Link = Link->Flink) {
    SrcArg = CR (Link, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);

    //
    // Find out the device path of the source
    //
    if (SrcDev) {
      FreePool (SrcDev);
    }

    SrcDev = StrDuplicate (SrcArg->FullName);
    for (Index = 0; Index < StrLen (SrcDev); Index++) {
      if (SrcDev[Index] == ':') {
        break;
      }
    }

    SrcDev[Index] = 0;

    mCpSrcDevice  = (EFI_DEVICE_PATH_PROTOCOL *) ShellGetMap (SrcDev);

    for (SrcFilePath = SrcArg->FullName; *SrcFilePath && *SrcFilePath != ':'; SrcFilePath++) {
      ;
    }

    for (DstFilePath = DstArg->FullName; *DstFilePath && *DstFilePath != ':'; DstFilePath++) {
      ;
    }
    //
    // if target is a directory and source is a file,
    // we need to compose a full name
    //
    if (SrcArg->Status == EFI_SUCCESS &&
        !(SrcArg->Info->Attribute & EFI_FILE_DIRECTORY) &&
        (DstArg->Status == EFI_SUCCESS && (DstArg->Info->Attribute & EFI_FILE_DIRECTORY) || DstDir && DstHandle)
        ) {
      if (DstFullName) {
        FreePool (DstFullName);
      }

      DstFullName = AllocatePool (StrSize (DstArg->FullName) + 2 + StrSize (SrcArg->FileName) + 2);
      if (!DstFullName) {
        Print (L"Cp: Out of resources\n");
        Status = EFI_OUT_OF_RESOURCES;
        goto Done;
      }

      StrCpy (DstFullName, DstArg->FullName);
      if (DstFullName[StrLen (DstFullName) - 1] != '\\') {
        StrCat (DstFullName, L"\\");
      }

      StrCat (DstFullName, SrcArg->FileName);

      for (DstFilePath = DstFullName; *DstFilePath && *DstFilePath != ':'; DstFilePath++) {
        ;
      }
    }

    if (StriCmp (SrcFilePath, DstFilePath) == 0) {
      SrcDevPathLen = DevicePathSize (mCpSrcDevice);
      DstDevPathLen = DevicePathSize (mCpDstDevice);
      if (SrcDevPathLen == DstDevPathLen) {
        if (CompareMem (
              mCpSrcDevice,
              mCpDstDevice,
              SrcDevPathLen
              ) == 0) {
          Print (
            L"cp: Cannot copy %hs to itself\n",
            SrcArg->FullName
            );
          Status = EFI_INVALID_PARAMETER;
          goto Done;
        }
      }
    }
  }
  //
  // Make sure no source is being copied onto its sub if -r is specified
  //
  if (mCpRecursive) {
    for (Link = SrcList.Flink; Link != &SrcList; Link = Link->Flink) {
      SrcArg = CR (Link, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);

      //
      // Find out the device path of the source
      //
      if (SrcDev) {
        FreePool (SrcDev);
      }

      SrcDev = StrDuplicate (SrcArg->FullName);
      if (!SrcDev) {
        Print (L"Cp: Out of resources\n");
        Status = EFI_OUT_OF_RESOURCES;
        goto Done;
      }

      for (Index = 0; Index < StrLen (SrcDev); Index++) {
        if (SrcDev[Index] == ':') {
          break;
        }
      }

      SrcDev[Index] = 0;

      mCpSrcDevice  = (EFI_DEVICE_PATH_PROTOCOL *) ShellGetMap (SrcDev);

      for (SrcFilePath = SrcArg->FullName; *SrcFilePath && *SrcFilePath != ':'; SrcFilePath++) {
        ;
      }

      for (DstFilePath = DstArg->FullName; *DstFilePath && *DstFilePath != ':'; DstFilePath++) {
        ;
      }

      if (DstFullName) {
        FreePool (DstFullName);
      }

      DstFullName = StrDuplicate (DstFilePath);
      if (!DstFullName) {
        Print (L"Cp: Out of resources\n");
        Status = EFI_OUT_OF_RESOURCES;
        goto Done;
      }

      SrcDevPathLen = DevicePathSize (mCpSrcDevice);
      DstDevPathLen = DevicePathSize (mCpDstDevice);

      if (SrcDevPathLen == DstDevPathLen) {
        if (CompareMem (
              mCpSrcDevice,
              mCpDstDevice,
              SrcDevPathLen
              ) == 0) {
          for (Index = StrLen (DstFullName); Index > 0; Index--) {
            TempChar            = DstFullName[Index];
            DstFullName[Index]  = 0;
            if (StriCmp (SrcFilePath, DstFullName) == 0) {
              if (SrcFilePath[Index - 1] == '\\' || TempChar == '\\' || TempChar == 0) {
                Print (
                  L"cp: Cannot copy %hs to its subdirectory\n",
                  SrcArg->FullName
                  );
                Status = EFI_INVALID_PARAMETER;
                goto Done;
              }
            }
          }
        }
      }
    }
  }
  //
  // ready to copy
  // allocate pool for following use
  //
  mCpBuffer = AllocatePool (CP_BLOCK_SIZE);
  if (!mCpBuffer) {
    Print (L"cp: Out of memory\n");
    Status = EFI_OUT_OF_RESOURCES;
    goto Done;
  }
  //
  // for each src, call CpCopy to copy it
  //
  for (Link = SrcList.Flink; Link != &SrcList; Link = Link->Flink) {
    SrcArg = CR (Link, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);

    if (SrcArg->Status != EFI_SUCCESS || !SrcArg->Handle) {
      Print (
        L"cp: Cannot open %hs - %r\n",
        SrcArg->FullName,
        SrcArg->Status
        );
      Status = SrcArg->Status;
      goto Done;
    } else {
      //
      // CpCopy requires SrcHandle and DstHandle be valid
      //
      if (DstHandle) {
        //
        // we created a new file/dir as dst
        //
        Status = CpCopy (
                  SrcArg->Handle,
                  DstHandle,
                  SrcArg->FullName,
                  DstArg->FullName,
                  FALSE
                  );
        if (EFI_ERROR (Status)) {
          //
          // we don't care the return status
          //
          DstHandle->Delete (DstHandle);
          DstHandle = NULL;
          goto Done;
        }
      } else {
        //
        // dst already exists
        //
        Status = CpCopy (
                  SrcArg->Handle,
                  DstArg->Handle,
                  SrcArg->FullName,
                  DstArg->FullName,
                  TRUE
                  );
        if (EFI_ERROR (Status)) {
          //
          // Error message is supposed to be displayed in CpCopy
          //
          goto Done;
        }
      }
    }
  }

Done:
  if (DstDev) {
    FreePool (DstDev);
  }

  if (SrcDev) {
    FreePool (SrcDev);
  }

  if (DstFullName) {
    FreePool (DstFullName);
  }

  if (DstHandle) {
    DstHandle->Close (DstHandle);
  }

  if (mCpBuffer) {
    FreePool (mCpBuffer);
  }

  ShellFreeFileList (&SrcList);
  ShellFreeFileList (&DstList);

  return Status;
}

