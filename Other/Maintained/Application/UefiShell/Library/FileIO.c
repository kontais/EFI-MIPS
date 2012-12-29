/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  FileIo.c

Abstract:

  Implements File I/O function

Revision History

--*/

#include "EfiShellLib.h"

typedef struct _PATH_COMPONENTS {
  CHAR16                  *Name;
  struct _PATH_COMPONENTS *Previous;
  struct _PATH_COMPONENTS *Next;
} PATH_COMPONENTS;

EFI_FILE_HANDLE
LibOpenRoot (
  IN EFI_HANDLE                   DeviceHandle
  )
/*++

Routine Description:

  Function opens and returns a file handle to the root directory of a volume.

Arguments:

  DeviceHandle         - A handle for a device

Returns:
  
  A valid file handle or NULL is returned

--*/
{
  EFI_STATUS                      Status;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *Volume;
  EFI_FILE_HANDLE                 File;

  File = NULL;

  //
  // Handle the file system interface to the device
  //
  Status = BS->HandleProtocol (
                DeviceHandle,
                &gEfiSimpleFileSystemProtocolGuid,
                (VOID *) &Volume
                );

  //
  // Open the root directory of the volume
  //
  if (!EFI_ERROR (Status)) {
    Status = Volume->OpenVolume (
                      Volume,
                      &File
                      );
  }
  //
  // Done
  //
  return EFI_ERROR (Status) ? NULL : File;
}

EFI_FILE_INFO *
LibGetFileInfo (
  IN EFI_FILE_HANDLE      FileHandle
  )
/*++

Routine Description:

  Function gets the file information from an open file descriptor, and stores it 
  in a buffer allocated from pool.

Arguments:

  FileHandle         - A file handle

Returns:
  
  A pointer to a buffer with file information or NULL is returned

--*/
{
  EFI_STATUS    Status;
  EFI_FILE_INFO *Buffer;
  UINTN         BufferSize;

  ASSERT (FileHandle != NULL);
  //
  // Initialize for GrowBuffer loop
  //
  Buffer      = NULL;
  BufferSize  = SIZE_OF_EFI_FILE_INFO + 200;

  //
  // Call the real function
  //
  while (GrowBuffer (&Status, (VOID **) &Buffer, BufferSize)) {
    Status = FileHandle->GetInfo (
                          FileHandle,
                          &gEfiFileInfoGuid,
                          &BufferSize,
                          Buffer
                          );
  }

  if (EFI_ERROR (Status)) {
    return NULL;
  }

  return Buffer;
}

EFI_STATUS
LibSetFileInfo (
  IN EFI_FILE_HANDLE  FileHandle,
  IN UINTN            BufferSize,
  IN VOID             *Buffer
  )
{
  ASSERT (FileHandle != NULL);
  ASSERT (Buffer != NULL);
  return FileHandle->SetInfo (
                        FileHandle,
                        &gEfiFileInfoGuid,
                        BufferSize,
                        Buffer
                        );
}

EFI_STATUS
LibSetPosition (
  IN EFI_FILE_HANDLE  FileHandle,
  IN UINT64           Position
  )
{
  ASSERT (FileHandle != NULL);
  return FileHandle->SetPosition (FileHandle, Position);
}

EFI_STATUS
LibGetPosition (
  IN EFI_FILE_HANDLE  FileHandle,
  OUT UINT64          *Position
  )
{
  ASSERT (FileHandle != NULL);
  ASSERT (Position != NULL);
  //
  // very simple wrapper
  //
  return FileHandle->GetPosition (FileHandle, Position);
}

EFI_STATUS
LibFlushFile (
  IN EFI_FILE_HANDLE FileHandle
  )
{
  ASSERT (FileHandle != NULL);
  //
  // very simple wrapper
  //
  return FileHandle->Flush (FileHandle);
}

EFI_STATUS
LibDeleteFile (
  IN EFI_FILE_HANDLE FileHandle
  )
{
  //
  // very simple wrapper
  //
  ASSERT (FileHandle != NULL);
  return FileHandle->Delete (FileHandle);
}

EFI_STATUS
LibOpenFileByName (
  IN  CHAR16                            *FileName,
  OUT EFI_FILE_HANDLE                   *FileHandle,
  IN UINT64                             OpenMode,
  IN UINT64                             Attributes
  )
{
  EFI_DEVICE_PATH_PROTOCOL  *FilePath;
  EFI_HANDLE                DeviceHandle;
  
  ASSERT (FileName != NULL);
  ASSERT (FileHandle != NULL);

  FilePath = SE2->NameToPath (FileName);
  if (FilePath != NULL) {
    return LibOpenFile (
            &FilePath,
            &DeviceHandle,
            FileHandle,
            OpenMode,
            Attributes
            );
  }

  return EFI_DEVICE_ERROR;

}

EFI_STATUS
LibOpenFile (
  IN OUT EFI_DEVICE_PATH_PROTOCOL       **FilePath,
  OUT EFI_HANDLE                        *DeviceHandle,
  OUT EFI_FILE_HANDLE                   *FileHandle,
  IN UINT64                             OpenMode,
  IN UINT64                             Attributes
  )
/*++

Routine Description:

    Opens a file for (simple) reading.  The simple read abstraction
    will access the file access from a file system interface.

Arguments:

    FilePath     - File path
    DeviceHandle - Device handle
    FileHandle   - File Handle 
    OpenMode     - Open Mode
    Attributes   - The attributes

Returns:

    A handle to access the file

--*/
{
  EFI_STATUS            Status;
  EFI_FILE_HANDLE       LastHandle;
  FILEPATH_DEVICE_PATH  *FilePathNode;

  ASSERT (FilePath != NULL);
  ASSERT (DeviceHandle != NULL);
  ASSERT (FileHandle != NULL);
  
  //
  // File the file system for this file path
  //
  Status = BS->LocateDevicePath (&gEfiSimpleFileSystemProtocolGuid, FilePath, DeviceHandle);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Attempt to access the file via a file system interface
  //
  *FileHandle = LibOpenRoot (*DeviceHandle);
  Status      = *FileHandle ? EFI_SUCCESS : EFI_UNSUPPORTED;

  //
  // To access as a file system, the file path should only
  // contain file path components.  Follow the file path nodes
  // and find the target file
  //
  FilePathNode = (FILEPATH_DEVICE_PATH *) *FilePath;
  while (!IsDevicePathEnd (&FilePathNode->Header)) {
    //
    // For file system access each node should be a file path component
    //
    if (DevicePathType (&FilePathNode->Header) != MEDIA_DEVICE_PATH ||
        DevicePathSubType (&FilePathNode->Header) != MEDIA_FILEPATH_DP
        ) {
      Status = EFI_UNSUPPORTED;
    }
    //
    // If there's been an error, stop
    //
    if (EFI_ERROR (Status)) {
      break;
    }
    //
    // Open this file path node
    //
    LastHandle  = *FileHandle;
    *FileHandle = NULL;

    Status = LastHandle->Open (
                          LastHandle,
                          FileHandle,
                          FilePathNode->PathName,
                          OpenMode &~EFI_FILE_MODE_CREATE,
                          0
                          );

    if ((EFI_ERROR (Status)) && (OpenMode != (OpenMode &~EFI_FILE_MODE_CREATE))) {
      Status = LastHandle->Open (
                            LastHandle,
                            FileHandle,
                            FilePathNode->PathName,
                            OpenMode,
                            Attributes
                            );
    }
    //
    // Close the last node
    //
    LastHandle->Close (LastHandle);

    //
    // Get the next node
    //
    FilePathNode = (FILEPATH_DEVICE_PATH *) NextDevicePathNode (&FilePathNode->Header);
  }

  if (EFI_ERROR (Status)) {
    *FileHandle = NULL;
  }

  return Status;
}

EFI_STATUS
LibReadFile (
  IN EFI_FILE_HANDLE      FileHandle,
  IN OUT UINTN            *ReadSize,
  OUT VOID                *Buffer
  )
{
  //
  // very simple wrapper
  //
  return FileHandle->Read (FileHandle, ReadSize, Buffer);
}

EFI_STATUS
LibWriteFile (
  IN EFI_FILE_HANDLE    FileHandle,
  IN OUT UINTN          *BufferSize,
  OUT VOID              *Buffer
  )
{
  return FileHandle->Write (
                      FileHandle,
                      BufferSize,
                      Buffer
                      );

}

EFI_STATUS
LibCloseFile (
  IN EFI_FILE_HANDLE     FileHandle
  )
{
  //
  // very simple wrapper
  //
  return FileHandle->Close (FileHandle);
}

EFI_DEVICE_PATH_PROTOCOL *
LibFileDevicePath (
  IN EFI_HANDLE                 Device  OPTIONAL,
  IN CHAR16                     *FileName
  )
/*++

Routine Description:
  Function allocates a device path for a file and appends it to an existing device path.

Arguments:
  Device         - A pointer to a device handle.

  FileName       - A pointer to a Null-terminated Unicode string.

Returns:

  If Device is not a valid device handle, then a device path for the file specified 
  by FileName is allocated and returned.

  Results are allocated from pool.  The caller must FreePool the resulting device path 
  structure

--*/
{
  UINTN                     Size;
  FILEPATH_DEVICE_PATH      *FilePath;
  EFI_DEVICE_PATH_PROTOCOL  *Eop;
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath;

  Size        = StrSize (FileName);
  FilePath    = AllocateZeroPool (Size + SIZE_OF_FILEPATH_DEVICE_PATH + sizeof (EFI_DEVICE_PATH_PROTOCOL));
  DevicePath  = NULL;

  if (FilePath != NULL) {
    //
    // Build a file path
    //
    FilePath->Header.Type     = MEDIA_DEVICE_PATH;
    FilePath->Header.SubType  = MEDIA_FILEPATH_DP;
    SetDevicePathNodeLength (&FilePath->Header, Size + SIZE_OF_FILEPATH_DEVICE_PATH);
    CopyMem (FilePath->PathName, FileName, Size);
    Eop = NextDevicePathNode (&FilePath->Header);
    SetDevicePathEndNode (Eop);

    //
    // Append file path to device's device path
    //
    DevicePath = (EFI_DEVICE_PATH_PROTOCOL *) FilePath;
    if (Device != NULL) {
      DevicePath = AppendDevicePath (
                    DevicePathFromHandle (Device),
                    DevicePath
                    );
      FreePool (FilePath);
      ASSERT (DevicePath);
    }
  }

  return DevicePath;
}

EFI_STATUS
LibFindFirstFile (
  IN  EFI_FILE_HANDLE                       DirHandle,
  OUT EFI_FILE_INFO                         *Buffer
  )
{
  EFI_STATUS    Status;
  EFI_FILE_INFO *DirInfo;
  UINTN         BufferSize;

  ASSERT (Buffer);
  
  BufferSize  = SIZE_OF_EFI_FILE_INFO + 1024;
  DirInfo     = LibGetFileInfo (DirHandle);
  if (DirInfo == NULL || !(DirInfo->Attribute & EFI_FILE_DIRECTORY)) {
    Status = EFI_NOT_FOUND;
    goto Done;
  }

  Status = LibSetPosition (DirHandle, 0);
  if (EFI_ERROR (Status)) {
    goto Done;
  }

  Status = LibReadFile (DirHandle, &BufferSize, Buffer);

Done:
  if (DirInfo != NULL) {
    FreePool (DirInfo);
  }
  return Status;
}

EFI_STATUS
LibFindNextFile (
  IN     EFI_FILE_HANDLE                    DirHandle,
  OUT EFI_FILE_INFO                         *Buffer,
  OUT BOOLEAN                               *NoFile
  )
{
  UINTN       BufferSize;
  EFI_STATUS  Status;

  ASSERT (DirHandle != NULL);
  ASSERT (Buffer != NULL);
  ASSERT (NoFile != NULL);

  *NoFile     = FALSE;
  BufferSize  = SIZE_OF_EFI_FILE_INFO + 1024;
  //
  // Large enough
  //
  Status = LibReadFile (DirHandle, &BufferSize, Buffer);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (BufferSize == 0) {
    *NoFile = TRUE;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
LibGetFileSize (
  IN     EFI_FILE_HANDLE                      DirHandle,
  OUT UINT64                                  *Size
  )
{
  EFI_FILE_INFO *Info;
  
  ASSERT (Size != NULL);
  
  Info = LibGetFileInfo (DirHandle);

  if (Info != NULL) {
    *Size = Info->FileSize;
    FreePool (Info);
    return EFI_SUCCESS;
  }

  return EFI_DEVICE_ERROR;
}

EFI_STATUS
LibCreateDirectory (
  IN CHAR16                             *DirName,
  OUT EFI_FILE_HANDLE                   *FileHandle
  )
{
  return LibOpenFileByName (
          DirName,
          FileHandle,
          EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE,
          EFI_FILE_DIRECTORY
          );

}

CHAR16 *
_ProcessDotInPath (
  IN CHAR16                       *PathFromRoot
  )
{
  CHAR16          *Result;
  CHAR16          *p;
  CHAR16          *q;
  PATH_COMPONENTS *root;
  PATH_COMPONENTS *cur;
  PATH_COMPONENTS *tmp;
  UINTN           Size;

  ASSERT ('\\' == *PathFromRoot);

  root    = NULL;
  cur     = NULL;
  Result  = NULL;
  Size    = 0;
  p       = PathFromRoot + 1;
  while (*p) {
    q = StrChr (p, '\\');
    if (q) {
      *q = 0;
    }

    tmp = (PATH_COMPONENTS *) AllocateZeroPool (sizeof (PATH_COMPONENTS));
    if (NULL == tmp) {
      goto Fail;
    }

    if (NULL == root) {
      root  = tmp;
      cur   = root;
    } else {
      tmp->Previous = cur;
      cur->Next     = tmp;
      cur           = tmp;
    }

    tmp->Name = StrDuplicate (p);
    Size += StrSize (tmp->Name);
    if (q) {
      p = q + 1;
    } else {
      *p = 0;
    }
  }

  cur = root;
  while (cur) {
    if (0 == StriCmp (cur->Name, L"..")) {
      tmp = cur->Previous;
      if (tmp) {
        if (tmp->Previous) {
          tmp->Previous->Next = cur->Next;
          if (cur->Next) {
            cur->Next->Previous = tmp->Previous;
          }
        } else {
          root = cur->Next;
          if (root) {
            cur->Next->Previous = NULL;
          }
        }

        cur = cur->Next;
        FreePool (tmp->Next->Name);
        FreePool (tmp->Next);
        FreePool (tmp->Name);
        FreePool (tmp);
      } else {
        root = cur->Next;
        if (root) {
          cur->Next->Previous = NULL;
        }

        FreePool (cur->Name);
        FreePool (cur);
        cur = root;
      }
    } else if (0 == StriCmp (cur->Name, L".")) {
      tmp = cur->Previous;
      if (tmp) {
        tmp->Next = cur->Next;
        if (cur->Next) {
          cur->Next->Previous = tmp;
        }

        FreePool (cur->Name);
        FreePool (cur);
        cur = tmp->Next;
      } else {
        root = cur->Next;
        if (root) {
          cur->Next->Previous = NULL;
        }

        FreePool (cur->Name);
        FreePool (cur);
        cur = root;
      }
    } else {
      cur = cur->Next;
    }
  }

  Result = AllocateZeroPool (Size + sizeof (UINT16) * 2);
  if (NULL == Result) {
    goto Fail;
  }

  *Result = '\\';
  tmp     = root;
  while (tmp) {
    StrCat (Result, tmp->Name);
    tmp = tmp->Next;
    if (tmp) {
      StrCat (Result, L"\\");
    }
  }

Fail:
  tmp = root;
  while (tmp) {
    root = tmp->Next;
    FreePool (tmp->Name);
    FreePool (tmp);
    tmp = root;
  }

  return Result;
}

EFI_STATUS
_CheckContinuousSlash (
  IN CHAR16                       *PathFromRoot
  )
{
  CHAR16      *p;
  EFI_STATUS  Status;

  ASSERT (PathFromRoot != NULL);

  p       = PathFromRoot;
  Status  = EFI_SUCCESS;
  while (*p) {
    if ('\\' == *p && '\\' == *(p + 1)) {
      Status = EFI_INVALID_PARAMETER;
      break;
    }

    p++;
  }

  return Status;
}

EFI_STATUS
LibSplitFsAndPath (
  IN     CHAR16                       *AbPath,
  IN OUT CHAR16                       **Fs,
  IN OUT CHAR16                       **Path
  )
{
  EFI_STATUS  Status;
  CHAR16      *p;
  UINTN       Size;

  ASSERT (Fs);
  ASSERT (Path);

  *Fs     = NULL;
  *Path   = NULL;
  Status  = EFI_SUCCESS;

  p       = AbPath;
  while (*p) {
    if (':' == *p && ('\\' == *(p + 1) || 0 == *(p + 1))) {
      break;
    }

    p++;
  }

  if (0 == *p) {
    return EFI_INVALID_PARAMETER;
  }

  Size  = sizeof (CHAR16) * (UINTN) (p - AbPath + 1);
  *Fs   = AllocateZeroPool (Size);
  if (NULL == Fs) {
    return EFI_OUT_OF_RESOURCES;
  }

  CopyMem (*Fs, AbPath, Size - sizeof (CHAR16));

  if (*(p + 1)) {
    *Path = StrDuplicate (p + 1);
  } else {
    *Path = StrDuplicate (L"\\");
  }

  if (NULL == *Path) {
    FreePool (*Fs);
    return EFI_OUT_OF_RESOURCES;
  }

  return EFI_SUCCESS;
}

BOOLEAN
CompareFsDevice (
  IN  CHAR16     *FsName1,
  IN  CHAR16     *FsName2
  )
{
  EFI_STATUS                Status;
  EFI_DEVICE_PATH_PROTOCOL  *DevPath1;
  EFI_DEVICE_PATH_PROTOCOL  *DevPath2;

  Status = SE2->GetFsDevicePath (FsName1, &DevPath1);
  if (EFI_ERROR (Status)) {
    return FALSE;
  }

  Status = SE2->GetFsDevicePath (FsName2, &DevPath2);
  if (EFI_ERROR (Status)) {
    return FALSE;
  }

  if (DevicePathCompare (DevPath1, DevPath2) == 0) {
    return TRUE;
  }

  return FALSE;
}

EFI_STATUS
LibIsSubPath (
  IN CHAR16                       *Parent,
  IN CHAR16                       *Child,
  OUT BOOLEAN                     *Result
  )
{
  EFI_STATUS  Status;
  CHAR16      *PathFromRoot1;
  CHAR16      *PathFromRoot2;
  CHAR16      *Fs1;
  CHAR16      *Fs2;
  CHAR16      *tmp;
  UINTN       Index;

  ASSERT (Parent);
  ASSERT (Child);
  ASSERT (Result);

  Index         = 0;
  tmp           = NULL;
  Fs1           = NULL;
  Fs2           = NULL;
  PathFromRoot1 = NULL;
  PathFromRoot2 = NULL;
  Status        = EFI_SUCCESS;
  *Result       = FALSE;

  Status        = _CheckContinuousSlash (Parent);
  if (EFI_ERROR (Status)) {
    goto Done;
  }

  Status = _CheckContinuousSlash (Child);
  if (EFI_ERROR (Status)) {
    goto Done;
  }

  Status = LibSplitFsAndPath (Parent, &Fs1, &PathFromRoot1);
  if (EFI_ERROR (Status)) {
    goto Done;
  }

  Status = LibSplitFsAndPath (Child, &Fs2, &PathFromRoot2);
  if (EFI_ERROR (Status)) {
    goto Done;
  }

  if (!CompareFsDevice (Fs1, Fs2)) {
    goto Done;
  }

  tmp           = PathFromRoot1;
  PathFromRoot1 = _ProcessDotInPath (tmp);
  FreePool (tmp);
  if (NULL == PathFromRoot1) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Done;
  }

  tmp           = PathFromRoot2;
  PathFromRoot2 = _ProcessDotInPath (tmp);
  FreePool (tmp);
  if (NULL == PathFromRoot2) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Done;
  }

  while (PathFromRoot1[Index] && PathFromRoot2[Index] && PathFromRoot1[Index] == PathFromRoot2[Index]) {
    Index++;
  }

  if (0 == PathFromRoot1[Index]) {
    if ('\\' == PathFromRoot1[Index - 1]) {
      *Result = TRUE;
    } else {
      if ('\\' == PathFromRoot2[Index] || 0 == PathFromRoot2[Index]) {
        *Result = TRUE;
      }
    }

    goto Done;
  }

Done:
  FreePool (Fs1);
  FreePool (Fs2);
  FreePool (PathFromRoot1);
  FreePool (PathFromRoot2);

  return Status;
}

CHAR16 *
LibFormAbsolutePath (
  IN CHAR16                       *FilePath,
  IN CHAR16                       *Ref OPTIONAL
  )
{
  CHAR16  *p;
  CHAR16  *OrgStr;
  CHAR16  ch;
  CHAR16  *fs;
  CHAR16  *PathFromRoot;
  UINTN   len;
  
  ASSERT (FilePath != NULL);

  fs      = NULL;
  OrgStr  = FilePath;
  p       = FilePath;
  while (*p) {
    if (':' == *p && '\\' == *(p + 1)) {
      break;
    }

    p++;
  }

  if (0 == *p && Ref) {
    OrgStr  = Ref;
    p       = Ref;
    while (*p) {
      if (':' == *p && '\\' == *(p + 1)) {
        break;
      }

      p++;
    }
  }

  if (0 == *p) {
    return NULL;
  }

  ch  = *p;
  *p  = 0;
  fs  = StrDuplicate (OrgStr);
  *p  = ch;
  p++;
  PathFromRoot = AllocateZeroPool (StrSize (p) + sizeof (UINT16));
  if (NULL == PathFromRoot) {
    FreePool (fs);
    return NULL;
  }

  StrCpy (PathFromRoot, p);
  if ('\\' != PathFromRoot[StrLen (p) - 1]) {
    StrCat (PathFromRoot, L"\\");
  }

  if (OrgStr == Ref) {
    if ('\\' == *FilePath) {
      FilePath++;
    }

    p             = PathFromRoot;
    PathFromRoot  = AllocateZeroPool (StrSize (p) + StrSize (FilePath));
    if (NULL == PathFromRoot) {
      FreePool (fs);
      FreePool (p);
      return NULL;
    }

    StrCpy (PathFromRoot, p);
    FreePool (p);
    StrCat (PathFromRoot, FilePath);
  }

  if (EFI_ERROR (_CheckContinuousSlash (PathFromRoot))) {
    FreePool (fs);
    FreePool (PathFromRoot);
    return NULL;
  }

  len = StrLen (PathFromRoot);
  if (len > 1 && '\\' == PathFromRoot[len - 1]) {
    PathFromRoot[len - 1] = 0;
  }

  if (StrLen (PathFromRoot) > 1) {
    p = _ProcessDotInPath (PathFromRoot);
  } else {
    p = StrDuplicate (PathFromRoot);
  }

  FreePool (PathFromRoot);

  len           = StrSize (fs) + StrSize (p) + sizeof (UINT16);
  PathFromRoot  = AllocateZeroPool (len);
  if (PathFromRoot) {
    SPrint (PathFromRoot, len, L"%s:%s", fs, p);
  }

  FreePool (fs);
  FreePool (p);

  return PathFromRoot;
}

EFI_STATUS
LibCompareFile (
  IN CHAR16                      *DstFile,
  IN CHAR16                      *SrcFile,
  IN OUT BOOLEAN                 *IsSame
  )
/*++
Routine descriptions:

  Check if 2 files are the same, no wild card is suppoted

Arguments:

   DstFile - Dst File
   SrcFile - Src File

Return:         
   IsSame = FALSE:  dst is not the src  itself
   IsSame = TRUE    dst is the src itself

--*/
{
  EFI_LIST_ENTRY  SrcList;
  EFI_LIST_ENTRY  DstList;
  EFI_STATUS      Status;
  EFI_LIST_ENTRY  *Link;
  SHELL_FILE_ARG  *SrcArg;
  SHELL_FILE_ARG  *DstArg;
  
  ASSERT (IsSame != NULL);

  Link    = NULL;
  SrcArg  = NULL;
  DstArg  = NULL;
  InitializeListHead (&SrcList);
  InitializeListHead (&DstList);

  *IsSame = FALSE;

  Status  = ShellFileMetaArgNoWildCard (DstFile, &DstList);

  if (EFI_ERROR (Status) || IsListEmpty (&DstList)) {
    goto Done;
  }
  //
  // multi dst
  //
  if (DstList.Flink->Flink != &DstList) {
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  DstArg = CR (DstList.Flink, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);

  if (SrcFile) {
    Status = ShellFileMetaArg (SrcFile, &SrcList);

    if (EFI_ERROR (Status) || IsListEmpty (&DstList)) {
      goto Done;
    }
    //
    // multi dst
    //
    if (SrcList.Flink->Flink != &SrcList) {
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }

    SrcArg = CR (SrcList.Flink, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);

    //
    // At this stage, make sure there is at least one valid src
    //
    if (StrCmp (SrcArg->FullName, DstArg->FullName) == 0) {
      *IsSame = TRUE;
    }
  }

Done:
  ShellFreeFileList (&SrcList);
  ShellFreeFileList (&DstList);
  return Status;
}

EFI_FILE_HANDLE
LibOpenFilePath (
  IN EFI_DEVICE_PATH_PROTOCOL   *FilePath,
  IN UINT64                     FileMode
  )
/*++

Routine Description:
  
Arguments:
    FilePath - File path
    FileMode - File mode

Returns:
  
--*/
{
  EFI_FILE_HANDLE FileHandle;
  EFI_HANDLE      DeviceHandle;
  EFI_STATUS      Status;

  FileHandle    = NULL;
  DeviceHandle  = NULL;
  Status = LibOpenFile (
            &FilePath,
            &DeviceHandle,
            &FileHandle,
            FileMode,
            0
            );
  if (EFI_ERROR (Status)) {
    return NULL;
  }

  return FileHandle;
}

EFI_STATUS
OpenSimpleReadFile (
  IN BOOLEAN                        BootPolicy,
  IN VOID                           *SourceBuffer OPTIONAL,
  IN UINTN                          SourceSize,
  IN OUT EFI_DEVICE_PATH_PROTOCOL   **FilePath,
  OUT EFI_HANDLE                    *DeviceHandle,
  OUT SIMPLE_READ_FILE              *SimpleReadHandle
  )
/*++

Routine Description:

    Opens a file for (simple) reading.  The simple read abstraction
    will access the file either from a memory copy, from a file
    system interface, or from the load file interface. 

Arguments:

  BootPolicy       - The boot policy
  SourceBuffer     - The source buffer
  SourceSize       - The source size
  FilePath         - The file path
  DeviceHandle     - The device handle
  SimpleReadHandle - The simple read handle

Returns:

    A handle to access the file

--*/
{
  SIMPLE_READ_HANDLE        *FHand;
  EFI_DEVICE_PATH_PROTOCOL  *UserFilePath;
  EFI_DEVICE_PATH_PROTOCOL  *TempFilePath;
  EFI_DEVICE_PATH_PROTOCOL  *TempFilePathPtr;
  FILEPATH_DEVICE_PATH      *FilePathNode;
  EFI_FILE_HANDLE           FileHandle;
  EFI_FILE_HANDLE           LastHandle;
  EFI_STATUS                Status;
  EFI_LOAD_FILE_PROTOCOL    *LoadFile;

  ASSERT (DeviceHandle != NULL);
  ASSERT (SimpleReadHandle != NULL);
  
  FHand         = NULL;
  UserFilePath  = *FilePath;

  //
  // Allocate a new simple read handle structure
  //
  FHand = AllocateZeroPool (sizeof (SIMPLE_READ_HANDLE));
  if (!FHand) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Done;
  }

  FHand->Signature  = SIMPLE_READ_SIGNATURE;

  //
  // If the caller passed a copy of the file, then just use it
  //
  if (SourceBuffer) {
    FHand->Source     = SourceBuffer;
    FHand->SourceSize = SourceSize;
    *DeviceHandle     = NULL;
    Status            = EFI_SUCCESS;
    goto Done;
  }
  //
  // Attempt to access the file via a file system interface
  //
  FileHandle  = NULL;
  Status      = BS->LocateDevicePath (&gEfiSimpleFileSystemProtocolGuid, FilePath, DeviceHandle);
  if (!EFI_ERROR (Status)) {
    FileHandle = LibOpenRoot (*DeviceHandle);
  }

  Status = FileHandle ? EFI_SUCCESS : EFI_UNSUPPORTED;

  if (EFI_ERROR (Status)) {
    goto Done;
  }
  //
  // To access as a filesystem, the filepath should only
  // contain filepath components.  Follow the filepath nodes
  // and find the target file
  //
  FilePathNode = (FILEPATH_DEVICE_PATH *) *FilePath;
  while (!IsDevicePathEnd (&FilePathNode->Header)) {
    //
    // For filesystem access each node should be a filepath component
    //
    if (DevicePathType (&FilePathNode->Header) != MEDIA_DEVICE_PATH ||
        DevicePathSubType (&FilePathNode->Header) != MEDIA_FILEPATH_DP
        ) {
      Status = EFI_UNSUPPORTED;
    }
    //
    // If there's been an error, stop
    //
    if (EFI_ERROR (Status)) {
      break;
    }
    //
    // Open this file path node
    //
    LastHandle  = FileHandle;
    FileHandle  = NULL;

    Status = LastHandle->Open (
                          LastHandle,
                          &FileHandle,
                          FilePathNode->PathName,
                          EFI_FILE_MODE_READ,
                          0
                          );

    //
    // Close the last node
    //
    LastHandle->Close (LastHandle);

    //
    // Get the next node
    //
    FilePathNode = (FILEPATH_DEVICE_PATH *) NextDevicePathNode (&FilePathNode->Header);
  }
  //
  // If success, return the FHand
  //
  if (!EFI_ERROR (Status)) {
    ASSERT (FileHandle);
    FHand->FileHandle = FileHandle;
    goto Done;
  }
  //
  // Cleanup from filesystem access
  //
  if (FileHandle) {
    FileHandle->Close (FileHandle);
    FileHandle  = NULL;
    *FilePath   = UserFilePath;
  }
  //
  // If the error is something other then unsupported, return it
  //
  if (Status != EFI_UNSUPPORTED) {
    goto Done;
  }
  //
  // Attempt to access the file via the load file protocol
  //
  Status = LibDevicePathToInterface (&gEfiLoadFileProtocolGuid, *FilePath, (VOID *) &LoadFile);
  if (!EFI_ERROR (Status)) {

    TempFilePath    = DuplicateDevicePath (*FilePath);

    TempFilePathPtr = TempFilePath;

    Status          = BS->LocateDevicePath (&gEfiLoadFileProtocolGuid, &TempFilePath, DeviceHandle);

    FreePool (TempFilePathPtr);

    //
    // Determine the size of buffer needed to hold the file
    //
    SourceSize = 0;
    Status = LoadFile->LoadFile (
                        LoadFile,
                        *FilePath,
                        BootPolicy,
                        &SourceSize,
                        NULL
                        );

    //
    // We expect a buffer too small error to inform us
    // of the buffer size needed
    //
    if (Status == EFI_BUFFER_TOO_SMALL) {
      SourceBuffer = AllocatePool (SourceSize);

      if (SourceBuffer) {
        FHand->FreeBuffer = TRUE;
        FHand->Source     = SourceBuffer;
        FHand->SourceSize = SourceSize;

        Status = LoadFile->LoadFile (
                            LoadFile,
                            *FilePath,
                            BootPolicy,
                            &SourceSize,
                            SourceBuffer
                            );
      }
    }
    //
    // If success, return FHand
    //
    if (!EFI_ERROR (Status) || Status == EFI_ALREADY_STARTED) {
      goto Done;
    }
  }
  //
  // Nothing else to try
  //
  DEBUG ((EFI_D_LOAD | EFI_D_WARN, "OpenSimpleReadFile: Device did not support a known load protocol\n"));
  Status = EFI_UNSUPPORTED;

Done:
  //
  // If the file was not accessed, clean up
  //
  if (EFI_ERROR (Status) && (Status != EFI_ALREADY_STARTED)) {
    if (FHand) {
      if (FHand->FreeBuffer) {
        FreePool (FHand->Source);
      }

      FreePool (FHand);
    }
  }

  return Status;
}

EFI_STATUS
ReadSimpleReadFile (
  IN SIMPLE_READ_FILE     UserHandle,
  IN UINTN                Offset,
  IN OUT UINTN            *ReadSize,
  OUT VOID                *Buffer
  )
{
  UINTN               EndPos;
  SIMPLE_READ_HANDLE  *FHand;
  EFI_STATUS          Status;

  ASSERT (ReadSize);
  ASSERT (Buffer);

  FHand = UserHandle;
  ASSERT (FHand->Signature == SIMPLE_READ_SIGNATURE);
  if (FHand->Source) {
    //
    // Move data from our local copy of the file
    //
    EndPos = Offset +*ReadSize;
    if (EndPos > FHand->SourceSize) {
      *ReadSize = FHand->SourceSize - Offset;
      if (Offset >= FHand->SourceSize) {
        *ReadSize = 0;
      }
    }

    CopyMem (Buffer, (CHAR8 *) FHand->Source + Offset, *ReadSize);
    Status = EFI_SUCCESS;

  } else {
    //
    // Read data from the file
    //
    Status = FHand->FileHandle->SetPosition (FHand->FileHandle, Offset);

    if (!EFI_ERROR (Status)) {
      Status = FHand->FileHandle->Read (FHand->FileHandle, ReadSize, Buffer);
    }
  }

  return Status;
}

VOID
CloseSimpleReadFile (
  IN SIMPLE_READ_FILE     UserHandle
  )
{
  SIMPLE_READ_HANDLE  *FHand;

  FHand = UserHandle;
  ASSERT (FHand->Signature == SIMPLE_READ_SIGNATURE);

  //
  // Free any file handle we opened
  //
  if (FHand->FileHandle) {
    FHand->FileHandle->Close (FHand->FileHandle);
  }
  //
  // If we allocated the Source buffer, free it
  //
  if (FHand->FreeBuffer) {
    FreePool (FHand->Source);
  }
  //
  // Done with this simple read file handle
  //
  FreePool (FHand);
}
