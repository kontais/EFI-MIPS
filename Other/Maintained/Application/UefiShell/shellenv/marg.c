/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  marg.c
  
Abstract:
 
Revision History

--*/

#include "shelle.h"
#include "EfiShellLib.h"
#include "shellenvguid.h"

typedef struct _CWD {
  struct _CWD *Next;
  CHAR16      Name[1];
} SENV_CWD;

VOID
_AppendList (
  EFI_LIST_ENTRY   *ListHead,
  EFI_LIST_ENTRY   *AppendHead
  );

CHAR16 *
SEnvFileHandleToFileName (
  IN EFI_FILE_HANDLE      Handle
  )
/*++

Routine Description:

Arguments:

  Handle  - The file handle

Returns:

--*/
{
  UINTN           BufferSize;
  UINTN           BufferSize1;
  SENV_CWD        *CwdHead;
  SENV_CWD        *Cwd;
  POOL_PRINT      Str;
  EFI_FILE_INFO   *Info;
  EFI_STATUS      Status;
  EFI_FILE_HANDLE NextDir;

  ASSERT_LOCKED (&SEnvLock);

  Info    = NULL;

  Status  = EFI_SUCCESS;
  CwdHead = NULL;
  ZeroMem (&Str, sizeof (Str));

  //
  //
  //
  Status = Handle->Open (Handle, &Handle, L".", EFI_FILE_MODE_READ, 0);
  if (EFI_ERROR (Status)) {
    Handle = NULL;
    goto Done;
  }

  BufferSize  = SIZE_OF_EFI_FILE_INFO + 1024;
  Info        = AllocatePool (BufferSize);
  if (!Info) {
    goto Done;
  }
  //
  // Reverse out the current directory on the device
  //
  for (;;) {
    BufferSize1 = BufferSize;
    Status      = Handle->GetInfo (Handle, &gEfiFileInfoGuid, &BufferSize1, Info);
    if (EFI_ERROR (Status)) {
      goto Done;
    }
    //
    // Allocate & chain in a new name node
    //
    Cwd = AllocatePool (sizeof (SENV_CWD) + StrSize (Info->FileName));
    if (!Cwd) {
      goto Done;
    }

    StrCpy (Cwd->Name, Info->FileName);

    Cwd->Next = CwdHead;
    CwdHead   = Cwd;

    if (StrLen (Info->FileName) == 0) {
      break;
    }
    //
    // Move to the parent directory
    //
    Status = Handle->Open (Handle, &NextDir, L"..", EFI_FILE_MODE_READ, 0);
    if (EFI_ERROR (Status)) {
      break;
    }

    Handle->Close (Handle);
    Handle = NextDir;
  }
  //
  // Build the name string of the current path
  //
  if (CwdHead->Next) {
    for (Cwd = CwdHead->Next; Cwd; Cwd = Cwd->Next) {
      CatPrint (&Str, L"\\%s", Cwd->Name);
    }
  } else {
    //
    // Must be in the root
    //
    Str.str = StrDuplicate (L"\\");
  }

Done:
  while (CwdHead) {
    Cwd     = CwdHead;
    CwdHead = CwdHead->Next;
    FreePool (Cwd);
  }

  if (Info) {
    FreePool (Info);
  }

  if (Handle) {
    Handle->Close (Handle);
  }

  return Str.str;
}

VOID
SEnvFreeFileArg (
  IN SHELL_FILE_ARG   *Arg
  )
/*++

Routine Description:

Arguments:

  Arg    - The file arg.

Returns:

--*/
{
  //
  // Free memory for all elements belonging to Arg
  //
  if (Arg->Parent) {
    Arg->Parent->Close (Arg->Parent);
  }

  if (Arg->ParentName) {
    FreePool (Arg->ParentName);
  }

  if (Arg->ParentDevicePath) {
    FreePool (Arg->ParentDevicePath);
  }

  if (Arg->FullName) {
    FreePool (Arg->FullName);
  }

  if (Arg->FileName) {
    FreePool (Arg->FileName);
  }

  if (Arg->Handle) {
    Arg->Handle->Close (Arg->Handle);
  }

  if (Arg->Info) {
    FreePool (Arg->Info);
  }

  if (Arg->Link.Flink) {
    RemoveEntryList (&Arg->Link);
  }
  //
  // Free memory for Arg
  //
  FreePool (Arg);
}

EFI_STATUS
EFIAPI
SEnvDelDupFileArg (
  IN EFI_LIST_ENTRY   *ListHead
  )
{
#define EFI_DUP_FILE  ((EFI_STATUS) (EFI_SUCCESS - 10))
  EFI_STATUS      Status;
  EFI_LIST_ENTRY  *NextHead;
  EFI_LIST_ENTRY  *Link;
  EFI_LIST_ENTRY  *NextLink;
  EFI_LIST_ENTRY  *TmpLink;
  SHELL_FILE_ARG  *FileArg;
  SHELL_FILE_ARG  *NextFileArg;

  Status = EFI_SUCCESS;

  for (Link = ListHead->Flink; Link != ListHead; Link = Link->Flink) {
    FileArg   = CR (Link, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);
    NextHead  = Link;
    if (!EFI_ERROR (FileArg->Status)) {
      for (NextLink = NextHead->Flink; NextLink != ListHead; NextLink = NextLink->Flink) {
        NextFileArg = CR (NextLink, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);
        if (StriCmp (FileArg->FullName, NextFileArg->FullName) == 0) {
          NextFileArg->Status = EFI_DUP_FILE;
        }
      }
    }
  }
  //
  // delete these duplicated file
  //
  Link = ListHead->Flink;

  while (Link != ListHead) {
    FileArg = CR (Link, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);
    TmpLink = Link->Flink;
    if (FileArg->Status == EFI_DUP_FILE) {
      SEnvFreeFileArg (FileArg);
    }

    Link = TmpLink;
  }

  return Status;
}

EFI_STATUS
EFIAPI
SEnvFreeFileList (
  IN OUT EFI_LIST_ENTRY       *ListHead
  )
/*++

Routine Description:

Arguments:

  ListHead   - The list head

Returns:

  EFI_SUCCESS - Success

--*/
{
  SHELL_FILE_ARG  *Arg;

  while (!IsListEmpty (ListHead)) {
    Arg = CR (ListHead->Flink, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);
    SEnvFreeFileArg (Arg);
  }

  return EFI_SUCCESS;
}

SHELL_FILE_ARG *
SEnvNewFileArg (
  IN EFI_FILE_HANDLE            Parent,
  IN UINT64                     OpenMode,
  IN EFI_DEVICE_PATH_PROTOCOL   *ParentPath,
  IN CHAR16                     *ParentName,
  IN CHAR16                     *FileName
  )
/*++

Routine Description:

Arguments:

  Parent     - The parent directory
  OpenMode   - Open mode
  ParentPath - The parent directory path
  ParentName - The parent directory name
  FileName   - The file name

Returns:

--*/
{
  SHELL_FILE_ARG  *Arg;
  CHAR16          *LPath;
  CHAR16          *Ptr;
  UINTN           Len;

  Arg = NULL;

  //
  // Allocate a new arg structure
  //
  Arg = AllocateZeroPool (sizeof (SHELL_FILE_ARG));
  if (!Arg) {
    goto Done;
  }

  Arg->Signature = SHELL_FILE_ARG_SIGNATURE;
  Parent->Open (Parent, &Arg->Parent, L".", OpenMode, 0);
  Arg->ParentDevicePath = DuplicateDevicePath (ParentPath);
  Arg->ParentName       = StrDuplicate (ParentName);
  if (!Arg->Parent || !Arg->ParentDevicePath || !Arg->ParentName) {
    Arg->Status = EFI_OUT_OF_RESOURCES;
    goto Done;
  }
  //
  // Open the target file
  //
  Arg->Status = Parent->Open (
                          Parent,
                          &Arg->Handle,
                          FileName,
                          OpenMode,
                          0
                          );

  if (Arg->Status == EFI_WRITE_PROTECTED || Arg->Status == EFI_ACCESS_DENIED) {
    OpenMode = OpenMode &~EFI_FILE_MODE_WRITE;
    Arg->Status = Parent->Open (
                            Parent,
                            &Arg->Handle,
                            FileName,
                            OpenMode,
                            0
                            );
  }

  Arg->OpenMode = OpenMode;
  if (Arg->Handle) {
    Arg->Info = LibGetFileInfo (Arg->Handle);
  }
  //
  // Compute the file's full name
  //
  Arg->FileName = StrDuplicate (FileName);
  if (StriCmp (FileName, L".") == 0) {
    //
    // It is the same as the parent
    //
    Arg->FullName = StrDuplicate (Arg->ParentName);
  } else if (StriCmp (FileName, L"..") == 0) {

    LPath = NULL;
    for (Ptr = Arg->ParentName; *Ptr; Ptr++) {
      if (*Ptr == L'\\') {
        LPath = Ptr;
      }
    }

    if (LPath) {
      Arg->FullName = PoolPrint (
                        L"%.*s",
                        (UINTN) (LPath - Arg->ParentName + 1),
                        Arg->ParentName
                        );
    }

    Len = StrLen (Arg->FullName);

    if (Len && Arg->FullName[Len - 2] != ':') {
      Arg->FullName[Len - 1] = 0;
    }
  }

  if (!Arg->FullName) {
    //
    // Append filename to parent's name to get the file's full name
    //
    Len = StrLen (Arg->ParentName);
    if (Len && Arg->ParentName[Len - 1] == '\\') {
      Len -= 1;
    }

    if (FileName[0] == '\\') {
      FileName += 1;
    }

    if (EFI_ERROR (Arg->Status)) {
      Arg->FullName = PoolPrint (L"%.*s\\%s", Len, Arg->ParentName, FileName);
    } else {
      Arg->FullName = PoolPrint (L"%.*s\\%s", Len, Arg->ParentName, Arg->Info->FileName);
    }
  }

  if (!Arg->FileName || !Arg->FullName) {
    Arg->Status = EFI_OUT_OF_RESOURCES;
  }

Done:
  if (Arg && Arg->Status == EFI_OUT_OF_RESOURCES) {
    SEnvFreeFileArg (Arg);
    Arg = NULL;
  }

  if (Arg && !EFI_ERROR (Arg->Status) && !Arg->Handle) {
    Arg->Status = EFI_NOT_FOUND;
  }

  return Arg;
}

EFI_STATUS
EFIAPI
SEnvFileMetaArg (
  IN CHAR16               *Path,
  IN OUT EFI_LIST_ENTRY   *ListHead
  )
{
  EFI_STATUS      Status;
  BOOLEAN         Found;
  EFI_LIST_ENTRY  *Link;
  EFI_LIST_ENTRY  ListHead1;
  EFI_LIST_ENTRY  ListHead2;
  SHELL_FILE_ARG  *Arg;
  CHAR16          *NewPath;
  CHAR16          *Ptr;
  CHAR16          *PtrPath;
  CHAR16          *PtrNewPath;
  UINTN           Size;

  InitializeListHead (&ListHead1);
  InitializeListHead (&ListHead2);

  Status  = SEnvIFileMetaArg (Path, &ListHead1);

  Found   = FALSE;
  for (Ptr = Path; *Ptr && !Found; Ptr++) {
    switch (*Ptr) {
    case '[':
    case ']':

      Found = TRUE;
      break;
    }
  }

  if (!Found) {
    _AppendList (ListHead, &ListHead1);
    return Status;
  }

  if (!EFI_ERROR (Status)) {
    Found = FALSE;
    for (Link = ListHead1.Flink; Link != &ListHead1; Link = Link->Flink) {
      Arg = CR (Link, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);
      if (!EFI_ERROR (Arg->Status)) {
        Found = TRUE;
      }
    }
  }

  if (Found) {
    _AppendList (ListHead, &ListHead1);
    return Status;
  }
  //
  // can not found any valid file, will try to interpert the [ to its literal meaning
  //
  NewPath = NULL;
  Size    = StrSize (Path);
  NewPath = AllocateZeroPool (Size * 2);
  if (!NewPath) {
    return Status;
  }
  //
  // composed the name
  //
  PtrPath     = Path;
  PtrNewPath  = NewPath;

  while (*PtrPath) {
    if (*PtrPath == '[' || *PtrPath == ']') {
      *(PtrNewPath++) = ESCAPE_CHAR;
    }

    *(PtrNewPath++) = *(PtrPath++);
  }

  *PtrNewPath = 0;

  Status      = SEnvIFileMetaArg (NewPath, &ListHead2);

  if (!EFI_ERROR (Status)) {
    Found = FALSE;
    for (Link = ListHead2.Flink; Link != &ListHead2; Link = Link->Flink) {
      Arg = CR (Link, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);
      if (!EFI_ERROR (Arg->Status)) {
        Found = TRUE;
      }
    }
  }

  if (Found) {
    _AppendList (ListHead, &ListHead2);
    SEnvFreeFileList (&ListHead1);
  } else {
    _AppendList (ListHead, &ListHead1);
    SEnvFreeFileList (&ListHead2);
  }

  FreePool (NewPath);
  return Status;
}

EFI_STATUS
SEnvIFileMetaArg (
  IN CHAR16               *Path,
  IN OUT EFI_LIST_ENTRY   *ListHead
  )
/*++

Routine Description:

Arguments:

  Path      - The file path
  ListHead  - The list head

Returns:

--*/
{
  VARIABLE_ID               *Var;
  EFI_STATUS                Status;
  EFI_DEVICE_PATH_PROTOCOL  *RPath;
  EFI_DEVICE_PATH_PROTOCOL  *TPath;
  EFI_DEVICE_PATH_PROTOCOL  *ParentPath;
  FILEPATH_DEVICE_PATH      *FilePath;
  EFI_FILE_INFO             *Info;
  UINTN                     BufferSize;
  UINTN                     BufferSize1;
  EFI_FILE_HANDLE           Parent;
  EFI_FILE_HANDLE           Parent1;
  SHELL_FILE_ARG            *Arg;
  CHAR16                    *ParentName;
  CHAR16                    *LPath;
  CHAR16                    *Ptr;
  UINT64                    OpenMode;
  BOOLEAN                   Found;

  CHAR16                    *Path1;
  CHAR16                    *Path11;

  RPath       = NULL;
  Parent      = NULL;
  Parent1     = NULL;
  ParentPath  = NULL;
  ParentName  = NULL;
  Path11      = NULL;
  Info        = NULL;
  TPath       = NULL;

  AcquireLock (&SEnvLock);

  Path1   = StrDuplicate (Path);
  Path11  = Path1;
  Status  = CheckValidFileName (Path1);

  if (EFI_ERROR (Status)) {
    goto Done;
  }

  BufferSize  = SIZE_OF_EFI_FILE_INFO + 1024;
  Info        = AllocatePool (BufferSize);
  if (!Info) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Done;
  }
  //
  // Get the device
  //
  Var = SEnvMapDeviceFromName (&Path1);
  if (!Var) {
    //
    // Can not find the device mapping for the path,
    // return EFI_NO_MAPPING
    //
    Arg             = AllocateZeroPool (sizeof (SHELL_FILE_ARG));
    Arg->Signature  = SHELL_FILE_ARG_SIGNATURE;
    Arg->Status     = EFI_NO_MAPPING;
    Arg->ParentName = StrDuplicate (Path1);
    Arg->FullName   = StrDuplicate (Path1);
    Arg->FileName   = StrDuplicate (Path1);
    InsertTailList (ListHead, &Arg->Link);
    Status = EFI_SUCCESS;
    goto Done;
  }
  //
  // device path of device
  //
  ParentPath = DuplicateDevicePath ((EFI_DEVICE_PATH_PROTOCOL *) Var->u.Value);

  //
  // If the path is relative, append the current dir of the device to the dpath
  //
  if (*Path1 != '\\') {
    RPath = SEnvIFileNameToPath (Var->CurDir ? Var->CurDir : L"\\");
    TPath = AppendDevicePath (ParentPath, RPath);
    if (!RPath || !TPath) {
      Status = EFI_OUT_OF_RESOURCES;
      goto Done;
    }

    FreePool (ParentPath);
    FreePool (RPath);
    RPath       = NULL;
    ParentPath  = TPath;
    TPath       = NULL;
  }
  //
  // If there is a path before the last node of the name, then
  // append it and strip path to the last node.
  //
  LPath = NULL;
  for (Ptr = Path1; *Ptr; Ptr++) {
    if (*Ptr == '\\' && *(Ptr + 1) != '\\') {
      LPath = Ptr;
    }
  }

  if (LPath) {
    *LPath  = 0;
    RPath   = SEnvIFileNameToPath (Path1);
    TPath   = AppendDevicePath (ParentPath, RPath);
    if (!RPath || !TPath) {
      Status = EFI_OUT_OF_RESOURCES;
      goto Done;
    }

    FreePool (ParentPath);
    FreePool (RPath);
    RPath       = NULL;
    ParentPath  = TPath;
    Path1       = LPath + 1;
  }

  if (StrLen (Path1) == 0) {
    Path1 = L".";
  }
  //
  // Open the parent dir.
  // First is ReadOnly open, second is RW open,
  // this order can not reversed, if we concern that
  // MediaChange could happen.
  //
  OpenMode  = EFI_FILE_MODE_READ;
  Parent    = LibOpenFilePath (ParentPath, OpenMode);
  if (Parent) {
    OpenMode  = EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE;
    Parent1   = LibOpenFilePath (ParentPath, OpenMode);
    if (Parent1) {
      Parent->Close (Parent);
      Parent = Parent1;
    } else {
      OpenMode = EFI_FILE_MODE_READ;
    }
  }

  if (Parent) {
    Ptr = SEnvFileHandleToFileName (Parent);
    if (Ptr) {
      ParentName = PoolPrint (L"%s:%s", Var->Name, Ptr);
      FreePool (Ptr);
    }
  }

  if (!Parent) {
    Status = EFI_NOT_FOUND;
    goto Done;
  }

  BufferSize1 = BufferSize;
  Status      = Parent->GetInfo (Parent, &gEfiFileInfoGuid, &BufferSize1, Info);
  if (EFI_ERROR (Status)) {
    goto Done;
  }
  //
  // Parent should be directory
  //
  if (!(Info->Attribute & EFI_FILE_DIRECTORY)) {
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }
  //
  // Parent - file handle to parent directory
  // ParentPath - device path of parent dir
  // ParentName - name string of parent directory
  // ParentGuid - last guid of parent path
  //
  // Path1 - remaining node name
  //
  Found = FALSE;
  for (Ptr = Path1; *Ptr && !Found; Ptr++) {
    switch (*Ptr) {
    case '*':
    case '?':
    case '[':
    case ']':

      Found = TRUE;
      break;
    }
  }

  if (!Found) {
    TPath = SEnvIFileNameToPath (Path1);
    ASSERT (DevicePathType (TPath) == MEDIA_DEVICE_PATH && DevicePathSubType (TPath) == MEDIA_FILEPATH_DP);
    FilePath = (FILEPATH_DEVICE_PATH *) TPath;

    Arg = SEnvNewFileArg (
            Parent,
            OpenMode,
            ParentPath,
            ParentName,
            FilePath->PathName
            );
    FreePool (TPath);

    if (!Arg) {
      Status = EFI_OUT_OF_RESOURCES;
      goto Done;
    }

    InsertTailList (ListHead, &Arg->Link);
  } else {
    //
    // Check all the files for matches
    //
    Parent->SetPosition (Parent, 0);

    Found = FALSE;
    for (;;) {
      //
      // Read each file entry
      //
      BufferSize1 = BufferSize;
      Status      = Parent->Read (Parent, &BufferSize1, Info);
      if (EFI_ERROR (Status) || BufferSize1 == 0) {
        break;
      }
      //
      // Skip "." and ".."
      //
      if (StriCmp (Info->FileName, L".") == 0 || StriCmp (Info->FileName, L"..") == 0) {
        continue;
      }
      //
      // See if this one matches
      //
      if (!MetaMatch (Info->FileName, Path1)) {
        continue;
      }

      Found = TRUE;
      Arg = SEnvNewFileArg (
              Parent,
              OpenMode,
              ParentPath,
              ParentName,
              Info->FileName
              );
      if (!Arg) {
        Status = EFI_OUT_OF_RESOURCES;
        goto Done;
      }

      InsertTailList (ListHead, &Arg->Link);

      //
      // check next file entry
      //
    }
    //
    // If no match was found, then add a not-found entry for this name
    //
    if (!Found) {
      Arg = SEnvNewFileArg (
              Parent,
              OpenMode,
              ParentPath,
              ParentName,
              Path1
              );
      if (!Arg) {
        Status = EFI_OUT_OF_RESOURCES;
        goto Done;
      }

      Arg->Status = EFI_NOT_FOUND;
      InsertTailList (ListHead, &Arg->Link);
    }
  }
  //
  // Done
  //
Done:
  ReleaseLock (&SEnvLock);

  if (Parent) {
    Parent->Close (Parent);
  }

  if (RPath) {
    FreePool (RPath);
  }

  if (ParentPath) {
    FreePool (ParentPath);
  }

  if (ParentName) {
    FreePool (ParentName);
  }

  if (Info) {
    FreePool (Info);
  }

  if (Path11) {
    FreePool (Path11);
  }

  return Status;
}

EFI_STATUS
EFIAPI
SEnvIFileMetaArgNoWildCard (
  IN CHAR16               *Path,
  IN OUT EFI_LIST_ENTRY   *ListHead
  )
/*++

Routine Description:

Arguments:

  Path     - The file path
  ListHead - The list head

Returns:

--*/
{
  VARIABLE_ID               *Var;
  EFI_STATUS                Status;
  EFI_DEVICE_PATH_PROTOCOL  *RPath;
  EFI_DEVICE_PATH_PROTOCOL  *TPath;
  EFI_DEVICE_PATH_PROTOCOL  *ParentPath;
  FILEPATH_DEVICE_PATH      *FilePath;
  EFI_FILE_INFO             *Info;
  UINTN                     BufferSize;
  UINTN                     BufferSize1;
  EFI_FILE_HANDLE           Parent;
  EFI_FILE_HANDLE           Parent1;
  SHELL_FILE_ARG            *Arg;
  CHAR16                    *ParentName;
  CHAR16                    *LPath;
  CHAR16                    *Ptr;
  UINT64                    OpenMode;
  BOOLEAN                   Found;

  CHAR16                    *Path1;
  CHAR16                    *Path11;

  RPath       = NULL;
  Parent      = NULL;
  Parent1     = NULL;
  ParentPath  = NULL;
  ParentName  = NULL;
  Path11      = NULL;
  Info        = NULL;
  TPath       = NULL;

  AcquireLock (&SEnvLock);

  Path1   = StrDuplicate (Path);

  Path11  = Path1;

  Status  = CheckValidFileName (Path1);

  if (EFI_ERROR (Status)) {
    goto Done;
  }

  BufferSize  = SIZE_OF_EFI_FILE_INFO + 1024;
  Info        = AllocatePool (BufferSize);
  if (!Info) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Done;
  }
  //
  // Get the device
  //
  Var = SEnvMapDeviceFromName (&Path1);
  if (!Var) {
    //
    // Can not find the device mapping for the path,
    // return EFI_NO_MAPPING
    //
    Arg = AllocateZeroPool (sizeof (SHELL_FILE_ARG));
    if (NULL == Arg) {
      Status = EFI_OUT_OF_RESOURCES;
      goto Done;
    }

    Arg->Signature  = SHELL_FILE_ARG_SIGNATURE;
    Arg->Status     = EFI_NO_MAPPING;
    Arg->ParentName = StrDuplicate (Path1);
    Arg->FullName   = StrDuplicate (Path1);
    Arg->FileName   = StrDuplicate (Path1);
    InsertTailList (ListHead, &Arg->Link);
    Status = EFI_SUCCESS;
    goto Done;
  }

  ParentPath = DuplicateDevicePath ((EFI_DEVICE_PATH_PROTOCOL *) Var->u.Value);

  //
  // If the path is relative, append the current dir of the device to the dpath
  //
  if (*Path1 != '\\') {
    RPath = SEnvIFileNameToPath (Var->CurDir ? Var->CurDir : L"\\");
    TPath = AppendDevicePath (ParentPath, RPath);
    if (!RPath || !TPath) {
      Status = EFI_OUT_OF_RESOURCES;
      goto Done;
    }

    FreePool (ParentPath);
    FreePool (RPath);
    RPath       = NULL;
    ParentPath  = TPath;
  }
  //
  // If there is a path before the last node of the name, then
  // append it and strip path to the last node.
  //
  LPath = NULL;
  for (Ptr = Path1; *Ptr; Ptr++) {
    if (*Ptr == '\\' && *(Ptr + 1) != '\\') {
      LPath = Ptr;
    }
  }

  if (LPath) {
    *LPath  = 0;
    RPath   = SEnvIFileNameToPath (Path1);
    TPath   = AppendDevicePath (ParentPath, RPath);
    if (!RPath || !TPath) {
      Status = EFI_OUT_OF_RESOURCES;
      goto Done;
    }

    FreePool (ParentPath);
    FreePool (RPath);
    RPath       = NULL;
    ParentPath  = TPath;
    Path1       = LPath + 1;
  }

  if (StrLen (Path1) == 0) {
    Path1 = L".";
  }
  //
  // Open the parent dir.
  // First is ReadOnly open, second is RW open,
  // this order can not reversed, if we concern that
  // MediaChange could happen.
  //
  OpenMode  = EFI_FILE_MODE_READ;
  Parent    = LibOpenFilePath (ParentPath, OpenMode);
  if (Parent) {
    OpenMode  = EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE;
    Parent1   = LibOpenFilePath (ParentPath, OpenMode);
    if (Parent1) {
      Parent->Close (Parent);
      Parent = Parent1;
    } else {
      OpenMode = EFI_FILE_MODE_READ;
    }
  }

  if (Parent) {
    Ptr = SEnvFileHandleToFileName (Parent);
    if (Ptr) {
      ParentName = PoolPrint (L"%s:%s", Var->Name, Ptr);
      FreePool (Ptr);
    }
  }

  if (!Parent) {
    Status = EFI_NOT_FOUND;
    goto Done;
  }

  BufferSize1 = BufferSize;
  Status      = Parent->GetInfo (Parent, &gEfiFileInfoGuid, &BufferSize1, Info);
  if (EFI_ERROR (Status)) {
    goto Done;
  }
  //
  // Parent should be directory
  //
  if (!(Info->Attribute & EFI_FILE_DIRECTORY)) {
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }
  //
  // Parent - file handle to parent directory
  // ParentPath - device path of parent dir
  // ParentName - name string of parent directory
  // ParentGuid - last guid of parent path
  //
  // Path1 - remaining node name
  //
  Found = FALSE;
  for (Ptr = Path1; *Ptr && !Found; Ptr++) {
    switch (*Ptr) {
    case '*':
    case '?':

      Found = TRUE;
      break;
    }
  }

  if (!Found) {
    TPath = SEnvIFileNameToPath (Path1);
    ASSERT (DevicePathType (TPath) == MEDIA_DEVICE_PATH && DevicePathSubType (TPath) == MEDIA_FILEPATH_DP);
    FilePath = (FILEPATH_DEVICE_PATH *) TPath;

    Arg = SEnvNewFileArg (
            Parent,
            OpenMode,
            ParentPath,
            ParentName,
            FilePath->PathName
            );
    FreePool (TPath);

    if (!Arg) {
      Status = EFI_OUT_OF_RESOURCES;
      goto Done;
    }

    InsertTailList (ListHead, &Arg->Link);
  } else {
    Status = EFI_INVALID_PARAMETER;
  }
  //
  // Done
  //
Done:
  ReleaseLock (&SEnvLock);

  if (Parent) {
    Parent->Close (Parent);
  }

  if (RPath) {
    FreePool (RPath);
  }

  if (ParentPath) {
    FreePool (ParentPath);
  }

  if (ParentName) {
    FreePool (ParentName);
  }

  if (Info) {
    FreePool (Info);
  }

  if (Path11) {
    FreePool (Path11);
  }

  return Status;
}

EFI_STATUS
CheckValidFileName (
  IN OUT CHAR16               *Path
  )
{
  EFI_STATUS  Status;
  INTN        Index;
  INTN        Index1;
  INTN        Pos;
  INTN        Len;

  Status = EFI_SUCCESS;
  //
  // Remove redundant continuous '*' from path
  //
  Index = 0;
  Pos   = 0;
  while (Path[Index] != '\0') {
    Pos     = Index;
    Index1  = Index + 1;
    if (Path[Pos] == '*') {
      //
      // Skip redundant continuous '*'
      //
      while (Path[Pos] == '*') {
        Pos++;
      }
      //
      // Copy remaining characters forward
      //
      while (Path[Index1] != '\0') {
        Path[Index1++] = Path[Pos++];
      }
    }

    Index++;
  }

  Len = StrLen (Path);
  //
  // check duplicated '\'s in Path
  //
  for (Index = 0; Index < Len - 1; Index++) {
    if (Path[Index] == '\\' && Path[Index + 1] == '\\') {
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }
  }
  //
  // check triple '.'s in Path
  //

  /*
  if (Len < 2) goto Done;
  for (Index = 0; Index < Len - 2; Index++) {
    if (Path[Index] == '.' && Path[Index+1] == '.' && Path[Index+2] == '.') {
      Status = EFI_INVALID_PARAMETER;
      break;
    } 
  }
  */
Done:
  return Status;
}

VOID
_AppendList (
  EFI_LIST_ENTRY   *ListHead,
  EFI_LIST_ENTRY   *AppendHead
  )

{
  EFI_LIST_ENTRY  *End;
  EFI_LIST_ENTRY  *Entry1;
  EFI_LIST_ENTRY  *Entry2;

  if (IsListEmpty (AppendHead)) {
    return ;
  }

  End             = ListHead->Blink;

  Entry1          = AppendHead->Flink;
  Entry2          = AppendHead->Blink;

  End->Flink      = Entry1;
  Entry1->Blink   = End;

  ListHead->Blink = Entry2;
  Entry2->Flink   = ListHead;
}
