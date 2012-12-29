/*++

Copyright (c) 2005 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  mv.c

Abstract:
 
  EFI Shell command 'mv'

Revision History

--*/

#include "EfiShellLib.h"
#include "mv.h"
//
// #include "EfiFs.h"
//
extern UINT8  STRING_ARRAY_NAME[];

//
// This is the generated header file which includes whatever needs to be exported (strings + IFR)
//
#include STRING_DEFINES_FILE

//
// Global Variables
//
EFI_HII_HANDLE  HiiHandle;
EFI_GUID        EfiMvGuid = EFI_MV_GUID;
SHELL_VAR_CHECK_ITEM      MvCheckList[] = {
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
// Function Declarations
//
EFI_STATUS
EFIAPI
InitializeMv (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
MvFile (
  IN SHELL_FILE_ARG       *SrcArg,
  IN CHAR16               *NewName
  );

//
// Entry point
//
EFI_BOOTSHELL_CODE(
  EFI_APPLICATION_ENTRY_POINT(InitializeMv)
)

EFI_STATUS
EFIAPI
InitializeMv (
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
  EFI_UNSUPPORTED         - Unsupported function (mv between different file systems)
  EFI_NO_MAPPING          - Device not mapped
  EFI_OUT_OF_RESOURCES    - Out of memory or file handle
  Other value             - Unknown error
  
--*/
{
  EFI_STATUS                Status;
  CHAR16                    **Argv;
  UINTN                     Argc;
  UINTN                     Index;
  UINTN                     Index1;
  EFI_LIST_ENTRY            SrcList;
  EFI_LIST_ENTRY            DstList;
  EFI_LIST_ENTRY            *Link;
  SHELL_FILE_ARG            *SrcArg;
  SHELL_FILE_ARG            *DstArg;
  CHAR16                    *DestName;
  CHAR16                    *FullDestName;
  BOOLEAN                   DestWild;
  BOOLEAN                   MutiSrc;
  CHAR16                    *Ptr;
  CHAR16                    *PtrOne;
  CHAR16                    *PtrTwo;
  UINTN                     BufferSize;
  CHAR16                    *CurPath;
  CHAR16                    *CurDir;
  BOOLEAN                   DstExists;
  CHAR16                    DefaultDir[2];
  EFI_DEVICE_PATH_PROTOCOL  *DstDevicePath;
  EFI_DEVICE_PATH_PROTOCOL  *SrcDevicePath;
  EFI_DEVICE_PATH_PROTOCOL  *CurDevicePath;
  EFI_DEVICE_PATH_PROTOCOL  *TargetDevicePath;
  CHAR16                    *TargetDev;
  CHAR16                    *DstDevName;
  CHAR16                    *SrcDevName;
  CHAR16                    *CurDevName;
  BOOLEAN                   IsSubPath;
  BOOLEAN                   IsSameFile;

  SHELL_VAR_CHECK_CODE      RetCode;
  CHAR16                    *Useful;
  SHELL_VAR_CHECK_PACKAGE   ChkPck;

  //
  // Local variable initializations
  //
  Argv    = SI->Argv;
  Argc    = SI->Argc;

  InitializeListHead (&SrcList);
  InitializeListHead (&DstList);
  Link                          = NULL;
  SrcArg                        = NULL;
  DstArg                        = NULL;
  DestName                      = NULL;
  MutiSrc                       = FALSE;
  CurPath                       = NULL;
  CurDir                        = NULL;
  DefaultDir[0]                 = '.';
  DefaultDir[1]                 = 0;
  CurDevicePath                 = NULL;
  CurDevName                    = NULL;
  DstDevName                    = NULL;
  TargetDevicePath              = NULL;
  TargetDev                     = NULL;
  PtrTwo                        = NULL;
  DstDevicePath                 = NULL;
  SrcDevicePath                 = NULL;
  SrcDevName                    = NULL;
  FullDestName                  = NULL;
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
  Status = LibInitializeStrings (&HiiHandle, STRING_ARRAY_NAME, &EfiMvGuid);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (!EFI_PROPER_VERSION (0, 99)) {
    PrintToken (
      STRING_TOKEN (STR_SHELLENV_GNC_COMMAND_NOT_SUPPORT),
      HiiHandle,
      L"mv",
      EFI_VERSION_0_99 
      );
    Status = EFI_UNSUPPORTED;
    goto Done;
  }

  RetCode = LibCheckVariables (SI, MvCheckList, &ChkPck, &Useful);
  if (VarCheckOk != RetCode) {
    switch (RetCode) {
    case VarCheckDuplicate:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_DUP_FLAG), HiiHandle, L"mv", Useful);
      break;

    case VarCheckUnknown:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_UNKNOWN_FLAG), HiiHandle, L"mv", Useful);
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
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiHandle, L"mv");
      Status = EFI_INVALID_PARAMETER;
    } else {
      if (IS_OLD_SHELL) {
        PrintToken (STRING_TOKEN (STR_NO_HELP), HiiHandle);
      } else {
        PrintToken (STRING_TOKEN (STR_MV_VERBOSE_HELP), HiiHandle);
      }

      Status = EFI_SUCCESS;
    }

    goto Done;
  }
  if (ChkPck.ValueCount < 1) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_FEW), HiiHandle, L"mv");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }
  //
  // Parse command line arguments
  //
  for (Index = 1; Index < Argc; Index += 1) {
    if (StrLen (Argv[Index]) == 0) {
      PrintToken (STRING_TOKEN (STR_MV_ZERO_LENGTH_ARG), HiiHandle, L"mv");
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }
  }
  //
  // If the last arg has wild cards then report an error
  //
  DestWild = FALSE;

  if (Argc == 2) {
    //
    // Destination is not specified in command line
    //
    DestName = DefaultDir;
  } else {
    DestName = Argv[Argc - 1];
    if (StrLen (DestName) == 0) {
      PrintToken (STRING_TOKEN (STR_MV_DEST_NOT_SPEC), HiiHandle, L"mv");
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }
  }

  if (Argc > 3) {
    MutiSrc = TRUE;
  }

  for (PtrOne = DestName; *PtrOne; PtrOne += 1) {
    if (*PtrOne == '*' || *PtrOne == '?') {
      DestWild = TRUE;
    }
  }

  if (DestWild) {
    PrintToken (STRING_TOKEN (STR_MV_DEST_NAME_WILDCARDS), HiiHandle, L"mv");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }
  //
  // find out the current directory
  //
  CurDir = ShellCurDir (NULL);
  if (CurDir) {
    CurDevicePath = ShellGetMap (CurDir);
  }
  //
  // Verify destination and sources contain the same device mapping
  //
  //
  // Get Dst device
  //
  DstDevName  = StrDuplicate (DestName);
  Ptr         = StrChr (DstDevName, ':');

  if (Ptr == NULL) {
    if (CurDevicePath) {
      DstDevicePath = CurDevicePath;
    } else {
      PrintToken (STRING_TOKEN (STR_MV_CANNOT_GET_CURRENT_DIR), HiiHandle, L"mv");
      Status = EFI_ABORTED;
      goto Done;
    }
  } else {
    DstDevName[Index] = 0;
    DstDevicePath     = ShellGetMap (DstDevName);
    if (!DstDevicePath) {
      PrintToken (STRING_TOKEN (STR_MV_DEST_NOT_MAPPED), HiiHandle, L"mv");
      Status = EFI_NO_MAPPING;
      goto Done;
    }
  }
  //
  // Get Source devices and compare them with destination device
  //
  for (Index1 = 1; Index1 < Argc - 1 || Index1 == 1; Index1++) {
    if (SrcDevName) {
      FreePool (SrcDevName);
    }

    SrcDevName  = StrDuplicate (Argv[Index1]);
    Ptr         = StrChr (SrcDevName, ':');

    if (Ptr == NULL) {
      if (CurDevicePath) {
        SrcDevicePath = CurDevicePath;
      } else {
        PrintToken (STRING_TOKEN (STR_MV_CANNOT_GET_CURRENT_DIR), HiiHandle, L"mv");
        Status = EFI_ABORTED;
        goto Done;
      }
    } else {
      *Ptr          = 0;
      SrcDevicePath = ShellGetMap (SrcDevName);
      if (!SrcDevicePath) {
        PrintToken (STRING_TOKEN (STR_MV_CANNOT_FIND_DEV_MAPPIN), HiiHandle, L"mv", Argv[Index1]);
        Status = EFI_NO_MAPPING;
        goto Done;
      }
    }
    //
    // compare source device with dst device
    //
    if (DevicePathCompare (SrcDevicePath, DstDevicePath) != 0) {
      PrintToken (STRING_TOKEN (STR_MV_SOURCE_DEST), HiiHandle, L"mv");
      Status = EFI_UNSUPPORTED;
      goto Done;
    }
  }
  //
  // at this stage, sources and destination are on the same file system,
  // we let cur dir point to the cur dir of destination device
  //
  if (DstDevicePath != CurDevicePath) {
    CurDevicePath = DstDevicePath;

    if (CurDevName) {
      FreePool (CurDevName);
    }

    CurDevName = StrDuplicate (DstDevName);

    if (CurDir) {
      FreePool (CurDir);
    }

    CurDir = ShellCurDir (CurDevName);

    if (!CurDir) {
      PrintToken (STRING_TOKEN (STR_MV_CANNOT_GET_CURRENT_DIR), HiiHandle, L"mv", CurDevName);
      Status = EFI_ABORTED;
      goto Done;
    }
  }
  //
  // Check to see if the dst is an existing dir
  //
  DstExists = FALSE;
  Status    = ShellFileMetaArg (DestName, &DstList);

  if (EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_MV_CANNOT_OPEN), HiiHandle, L"mv", DestName, Status);
    goto Done;
  }

  if (Status == EFI_SUCCESS && !IsListEmpty (&DstList)) {
    DstArg = CR (DstList.Flink, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);
    if (DstArg->Status == EFI_SUCCESS && DstArg->Handle) {
      DstExists = TRUE;
    }
  }

  if ((!DstExists && MutiSrc) || (DstExists && !(DstArg->Info->Attribute & EFI_FILE_DIRECTORY) && MutiSrc)) {
    Status = EFI_INVALID_PARAMETER;
    PrintToken (STRING_TOKEN (STR_MV_MUST_DIR), HiiHandle, L"mv", Status);
    goto Done;
  }
  //
  // Move DestName after ':' if there is ':'
  //
  PtrOne    = StrChr (DestName, ':');
  DestName  = PtrOne ? PtrOne + 1 : DestName;

  //
  // Strip off the trailing '\'s of DestName
  //
  StrTrimRight (DestName, '\\');

  if (StrLen (DestName) == 0) {
    DestName = DefaultDir;
  }
  //
  // Expand each source arg
  //
  for (Index = 1; Index < Argc - 1 || Index == 1; Index += 1) {
    Status = ShellFileMetaArg (Argv[Index], &SrcList);
    if (EFI_ERROR (Status) || IsListEmpty (&SrcList)) {
      if (IsListEmpty (&SrcList)) {
        Status = EFI_NOT_FOUND;
      }

      PrintToken (STRING_TOKEN (STR_MV_CANNOT_OPEN), HiiHandle, L"mv", Argv[Index], Status);
      goto Done;
    }
  }
  //
  // Check to see if we are moving root directory,
  // or current dir or its's ancestor
  //
  for (Link = SrcList.Flink; Link != &SrcList; Link = Link->Flink) {

    SrcArg = CR (Link, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);
    if (SrcArg->Status != EFI_SUCCESS) {
      continue;
    }
    //
    // Check to see if we are moving a root dir
    //
    PtrTwo = StrChr (SrcArg->FullName, ':');
    if (*PtrTwo && *(PtrTwo + 1) == '\\' && *(PtrTwo + 2) == 0) {
      PrintToken (STRING_TOKEN (STR_MV_CANNOT_MOVE_ROOT_DIR), HiiHandle, L"mv");
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }
    //
    // Check to see if we are moving current dir or its ancestor
    //
    if (SrcArg->FileName && (StriCmp (SrcArg->FileName, L".") == 0 || StriCmp (SrcArg->FileName, L"..") == 0)) {
      PrintToken (STRING_TOKEN (STR_MV_CANNOT_MOVE_CURRENT_DIR), HiiHandle, L"mv");
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }
    //
    // Get target device path
    //
    TargetDev         = StrDuplicate (SrcArg->FullName);
    PtrOne            = StrChr (TargetDev, ':');
    *PtrOne           = 0;
    TargetDevicePath  = ShellGetMap (TargetDev);

    //
    // Check if we are moving current path or current path's ancestor
    //
    for (CurPath = CurDir; *CurPath && *CurPath != ':'; CurPath++) {
      ;
    }

    if (DevicePathCompare (CurDevicePath, TargetDevicePath) == 0) {
      Status = LibIsSubPath (SrcArg->FullName, CurDir, &IsSubPath);
      if (!EFI_ERROR (Status) && IsSubPath) {
        PrintToken (STRING_TOKEN (STR_MV_CANNOT_MOVE_CURRENT_DIR), HiiHandle, L"mv");
        Status = EFI_INVALID_PARAMETER;
        goto Done;
      }
    }
  }
  //
  // Strip off the trailing '\\' of curdir
  //
  StrTrimRight (CurDir, '\\');

  //
  // skip the device mapping name
  //
  for (CurPath = CurDir; (*CurPath) && (*CurPath) != '\\'; CurPath++) {
    ;
  }

  BufferSize    = StrSize (DestName) + EFI_FILE_STRING_SIZE;
  FullDestName  = AllocatePool (BufferSize);
  if (!FullDestName) {
    PrintToken (STRING_TOKEN (STR_MV_OUT_OF_MEM), HiiHandle, L"mv");
    Status = EFI_OUT_OF_RESOURCES;
    goto Done;
  }

  if (DstExists) {
    for (Index = 1; Index < SI->RedirArgc; Index++) {
      if (!EFI_ERROR (LibCompareFile (DstArg->FullName, SI->RedirArgv[Index], &IsSameFile)) && IsSameFile == TRUE) {
        Status = EFI_REDIRECTION_SAME;
        goto Done;
      }
    }
  }
  //
  // Perform move
  //
  for (Link = SrcList.Flink; Link != &SrcList; Link = Link->Flink) {

    SrcArg = CR (Link, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);

    for (Index = 1; Index < SI->RedirArgc; Index++) {
      if (!EFI_ERROR (LibCompareFile (SrcArg->FullName, SI->RedirArgv[Index], &IsSameFile)) && IsSameFile == TRUE) {
        Status = EFI_REDIRECTION_SAME;
        goto Done;
      }
    }

    if (DstExists) {
      //
      //  Check to see if we are moving a directory into itself or its subdirectories.
      //
      Status = LibIsSubPath (SrcArg->FullName, DstArg->FullName, &IsSubPath);
      if (!EFI_ERROR (Status) && IsSubPath) {
        PrintToken (STRING_TOKEN (STR_MV_CANNOT_MOVE_INTO_SUB), HiiHandle, L"mv", SrcArg->FileName);
        Status = EFI_INVALID_PARAMETER;
        goto Done;
      }
      //
      // The destination exists
      //
      SPrint (FullDestName, BufferSize, L"%s", DstArg->FullName);

      //
      // Strip off the trailing '\\' of Dest path
      //
      if (StrLen (FullDestName) > 0) {
        if (FullDestName[StrLen (FullDestName) - 1] == '\\') {
          FullDestName[StrLen (FullDestName) - 1] = 0;
        }
      }

      if (DstArg->Info->Attribute & EFI_FILE_DIRECTORY) {
        //
        // For folder, just append the file name
        //
        if (StriCmp (DstArg->FullName, SrcArg->FullName) != 0) {
          SPrint (FullDestName, BufferSize, L"%s\\%s", FullDestName, SrcArg->FileName);
        } else {
          SPrint (FullDestName, BufferSize, L"%s", FullDestName);
        }
      }
    } else {
      //
      // The destination doesn't exist
      //
      SPrint (FullDestName, BufferSize, L"%s", DestName);

      //
      // The destination doesn't exist
      //
      if (FullDestName[0] != '\\') {
        //
        // For relative path, insert current path before the file name
        //
        SPrint (FullDestName, BufferSize, L"%s\\%s", CurPath, FullDestName);
      }
    }

    Status = MvFile (SrcArg, FullDestName);
  }

Done:
  if (CurDir) {
    FreePool (CurDir);
  }

  if (SrcDevName) {
    FreePool (SrcDevName);
  }

  if (DstDevName) {
    FreePool (DstDevName);
  }

  if (CurDevName) {
    FreePool (CurDevName);
  }

  if (TargetDev) {
    FreePool (TargetDev);
  }

  if (FullDestName) {
    FreePool (FullDestName);
  }

  ShellFreeFileList (&DstList);
  ShellFreeFileList (&SrcList);
  LibCheckVarFreeVarList (&ChkPck);
  LibUnInitializeStrings ();
  return Status;
}

EFI_STATUS
MvFile (
  IN SHELL_FILE_ARG       *SrcArg,
  IN CHAR16               *NewName
  )
/*++

Routine Description:

  Move a file to new name

Arguments:

  SrcArg      The file to be moved.
  NewName     The newname.

Returns:

  EFI_SUCCESS - Success
  EFI_INVALID_PARAMETER - Invalid parameter

--*/
{
  EFI_STATUS      Status;
  EFI_FILE_INFO   *Info;
  UINTN           NameSize;
  CHAR16          *SrcName;
  CHAR16          *DstName;
  EFI_FILE_HANDLE DstHandle;

  EFI_LIST_ENTRY  SrcList;
  EFI_LIST_ENTRY  *Link;
  EFI_STATUS      OpenStatus;
  SHELL_FILE_ARG  *SrcArg1;


  //
  // skip the device mapping name
  //
  Status = SrcArg->Status;
  for (SrcName = SrcArg->FullName; (*SrcName) && (*SrcName) != '\\'; SrcName++) {
    ;
  }

  for (DstName = NewName; (*DstName) && (*DstName) != '\\'; DstName++) {
    ;
  }

  if (!EFI_ERROR (Status)) {
    //
    //  check if the file was already moved
    //
    InitializeListHead (&SrcList);
    OpenStatus = ShellFileMetaArg (SrcArg->FullName, &SrcList);

    if (EFI_ERROR (OpenStatus) || IsListEmpty (&SrcList)) {
      if (IsListEmpty (&SrcList)) {
        return EFI_SUCCESS;
      }
    }

    for (Link = SrcList.Flink; Link != &SrcList; Link = Link->Flink) {

      SrcArg1 = CR (Link, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);

      if (EFI_ERROR (SrcArg1->Status)) {
        ShellFreeFileList (&SrcList);
        return EFI_SUCCESS;
      }
    }

    ShellFreeFileList (&SrcList);

    //
    // If moving to itself, report error.
    //
    if (StriCmp (SrcName, DstName) == 0) {
      PrintToken (STRING_TOKEN (STR_MV_CANNOT_MOVE_INTO_SUB), HiiHandle, L"mv", SrcArg->FileName);
      return EFI_INVALID_PARAMETER;
    }
    //
    // Check if a duplicate destination already exists.
    //
    Status = SrcArg->Handle->Open (
                              SrcArg->Handle,
                              &DstHandle,
                              DstName,
                              EFI_FILE_MODE_READ,
                              0
                              );
    if (!EFI_ERROR (Status)) {
      DstHandle->Close (DstHandle);
      PrintToken (STRING_TOKEN (STR_MV_DUPLICATE_DEST), HiiHandle, L"mv", SrcArg->FullName, DstName);
      return EFI_INVALID_PARAMETER;
    }

    NameSize  = StrSize (DstName);
    Info      = AllocatePool (SIZE_OF_EFI_FILE_INFO + NameSize);
    Status    = EFI_OUT_OF_RESOURCES;

    if (Info) {
      CopyMem (Info, SrcArg->Info, SIZE_OF_EFI_FILE_INFO);
      CopyMem (Info->FileName, DstName, NameSize);
      Info->Size = SIZE_OF_EFI_FILE_INFO + NameSize;
      Status = SrcArg->Handle->SetInfo (
                                SrcArg->Handle,
                                &gEfiFileInfoGuid,
                                (UINTN) Info->Size,
                                Info
                                );

      FreePool (Info);
    }
  }

  if (EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_MV_MOVING_ERROR), HiiHandle, L"mv", SrcArg->FullName, DstName, Status);
  } else {
    PrintToken (STRING_TOKEN (STR_MV_MOVING_OK), HiiHandle, L"mv", SrcArg->FullName, DstName);
  }

  return Status;
}

EFI_STATUS
EFIAPI
InitializeMvGetLineHelp (
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
  return LibCmdGetStringByToken (STRING_ARRAY_NAME, &EfiMvGuid, STRING_TOKEN (STR_MV_LINE_HELP), Str);
}
