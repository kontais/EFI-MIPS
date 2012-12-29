/*++

Copyright (c) 2012, kontais                                                                   
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  DirectoryManage.c.c

Abstract:


--*/

#include "Ext2.h"


EFI_STATUS
Ext2GetDirEntryInfo (
  IN EXT2_VOLUME         *Volume,
  IN CHAR16              *Name,
  IN UINT32              InodeNum,
  IN OUT UINTN           *BufferSize,
  OUT VOID               *Buffer
  )
/*++

Routine Description:

  Get the directory entry's info into Buffer.

Arguments:

  Volume                - EXT2 file system volume.
  DirEnt                - The corresponding directory entry.
  BufferSize            - Size of Buffer.
  Buffer                - Buffer containing file info.

Returns:

  EFI_SUCCESS           - Get the file info successfully.
  EFI_BUFFER_TOO_SMALL  - The buffer is too small.

--*/
{
  UINTN               Size;
  UINTN               NameSize;
  UINTN               ResultSize;
  EFI_STATUS          Status;
  EFI_FILE_INFO       *Info;
  UINT32              Len;
  EXT2_INODE          Inode;

  ASSERT_VOLUME_LOCKED (Volume);

  Size        = SIZE_OF_EFI_FILE_INFO;
  NameSize    = EfiStrSize(Name);
  ResultSize  = Size + NameSize;

  Status      = EFI_BUFFER_TOO_SMALL;
  if (*BufferSize >= ResultSize) {
    Status      = EFI_SUCCESS;
    Info        = Buffer;
    Info->Size  = ResultSize;

    EfiAsciiStrToUnicodeStr (Name, (CHAR16*)((CHAR8 *) Buffer + Size));

    //
    // Get more info from inode entry
    //
    Status = Ext2GetInodeEntry (Volume, InodeNum, &Inode);
    if (!EFI_ERROR(Status)) {
      Info->FileSize     = (UINT64) Inode.i_size;
      Info->PhysicalSize = (UINT64) Inode.i_blocks * 512; // See Understanding the Linux Kernel 3rd 18.2.3
      Ext2TimeToEfiTime (Inode.i_ctime, &(Info->CreateTime));
      Ext2TimeToEfiTime (Inode.i_atime, &(Info->LastAccessTime));
      Ext2TimeToEfiTime (Inode.i_mtime, &(Info->ModificationTime));
      EfiCopyMem ((CHAR8 *) Buffer + Size, Name, NameSize);
      Info->Attribute = 0;
      if (EXT2_ISDIR(Inode.i_mode)) {
        Info->Attribute |= EFI_FILE_DIRECTORY;
      }
      if ((Inode.i_mode & EXT2_IWRITE) == 0) {
        Info->Attribute |= EFI_FILE_READ_ONLY;
      }
    }
  }

  *BufferSize = ResultSize;
  return Status;
}

EFI_STATUS
Ext2SearchDir (
  IN  EXT2_OFILE     *OFile,
  IN  CHAR16         *FileNameString,
  OUT ino_t          *InodeNum
  )
/*++

Routine Description:

  Search the directory for the directory entry whose filename is FileNameString.

Arguments:

  OFile                 - The parent OFile whose directory is to be searched.
  FileNameString        - The filename to be searched.
  PtrDirEnt             - pointer to the directory entry if found.

Returns:

  EFI_SUCCESS           - Find the directory entry or not found.
  other                 - An error occurred when reading the directory entries.

--*/
{
  EFI_STATUS       Status;
  EXT2_DIR_ENTRY   DirEntry;
  UINTN            SavePos;
  CHAR8            Name[EXT2_NAME_LEN];
  
  
  EfiUnicodeStrToAsciiStr (FileNameString, Name);

  SavePos         = OFile->Position;
  OFile->Position = 0;
  for (;;) {
    Status = Ext2GetNextDirEntry (OFile, &DirEntry);
    if (EFI_ERROR(Status)) {
      return Status;
    }
    if (DirEntry.inode == 0) {
      goto NotFound;
    }
    if (0 == EfiAsciiStrnCmp (Name, DirEntry.name, DirEntry.name_len)) {
      *InodeNum = DirEntry.inode;
      OFile->Position = SavePos;
      return EFI_SUCCESS;
    }      
  }

NotFound:
  OFile->Position = SavePos;
  return EFI_NOT_FOUND;
}

EFI_STATUS
Ext2GetNextDirEntry (
  IN  EXT2_OFILE     *OFile,
  OUT EXT2_DIR_ENTRY *DirEntry
  )
/*++

Routine Description:

  Set the directory's cursor to the next and get the next directory entry.

Arguments:

  OFile                 - The parent OFile.
  PtrDirEnt             - The next directory entry.

Returns:

  EFI_SUCCESS           - We get the next directory entry successfully.
  other                 - An error occurred when get next directory entry.

--*/
{
  EFI_STATUS  Status;
  UINT32      ReadSize;
  UINT32      Rem;
  UINTN       SavePos;

  if (OFile->Position >= OFile->Size) {
    DirEntry->inode = 0;
    return EFI_SUCCESS;
  }

  SavePos  = OFile->Position;
  Rem      = OFile->Size - OFile->Position;
  ReadSize = Rem > sizeof(EXT2_DIR_ENTRY) ? sizeof(EXT2_DIR_ENTRY) : Rem;

  Status = Ext2AccessOFile (OFile, READ_DATA, (UINTN) OFile->Position, &ReadSize, (UINT8*)DirEntry);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  //
  // 8 = (sizeof(EXT2_DIR_ENTRY) - EXT2_NAME_LEN)
  //
  if ((ReadSize < 8) || (DirEntry->rec_len == 0)) {
    DirEntry->inode = 0;
    return EFI_SUCCESS;
  }
  
  OFile->Position = SavePos + DirEntry->rec_len;
  
  return EFI_SUCCESS;
}

EFI_STATUS
Ext2LocateOFile (
  IN OUT EXT2_OFILE       **PtrOFile,
  IN     CHAR16           *FileName,
  IN     UINT8            Attributes,
  OUT    CHAR16           *NewFileName
  )
/*++

Routine Description:

  Traverse filename and open all OFiles that can be opened.
  Update filename pointer to the component that can't be opened.
  If more than one name component remains, returns an error;
  otherwise, return the remaining name component so that the caller might choose to create it.

Arguments:
  PtrOFile              - As input, the reference OFile; as output, the located OFile.
  FileName              - The file name relevant to the OFile.
  Attributes            - The attribute of the destination OFile.
  NewFileName           - The remaining file name.

Returns:

  EFI_NOT_FOUND         - The file name can't be opened and there is more than one
                          components within the name left (this means the name can
                          not be created either).
  EFI_INVALID_PARAMETER - The parameter is not valid.
  EFI_SUCCESS           - Open the file successfully.
  other                 - An error occured when locating the OFile.

--*/
{
  EFI_STATUS   Status;
  EXT2_VOLUME  *Volume;
  CHAR16       ComponentName[EXT2_NAME_LEN];
  UINTN        FileNameLen;
  BOOLEAN      DirIntended;
  CHAR16       *Next;
  EXT2_OFILE   *OFile;
  EXT2_OFILE   *NewOFile;
  ino_t        InodeNum;

  FileNameLen = EfiStrLen (FileName);
  if (FileNameLen == 0) {
    return EFI_INVALID_PARAMETER;
  }

  OFile       = *PtrOFile;
  Volume      = OFile->Volume;

  DirIntended = FALSE;
  if (FileName[FileNameLen - 1] == PATH_NAME_SEPARATOR) {
    DirIntended = TRUE;
  }
  //
  // If name starts with path name separator, then move to root OFile
  //
  if (*FileName == PATH_NAME_SEPARATOR) {
    OFile = Volume->RootInode;
    FileName++;
    FileNameLen--;
  }
  //
  // Start at current location
  //
  Next = FileName;
  for (;;) {
    //
    // Get the next component name
    //
    FileName = Next;
    Next     = Ext2GetNextNameComponent (FileName, ComponentName);

    //
    // If end of the file name, we're done
    //
    if (ComponentName[0] == 0) {
      if (DirIntended && (!EXT2_ISDIR(OFile->Mode))) {
        return EFI_NOT_FOUND;
      }

      NewFileName[0] = 0;
      break;
    }
    //
    // If "dot", then current
    //
    if (EfiStrCmp (ComponentName, L".") == 0) {
      continue;
    }
    //
    // If "dot dot", then parent
    //
    if (EfiStrCmp (ComponentName, L"..") == 0) {
      if (OFile->Parent == NULL) {
        return EFI_INVALID_PARAMETER;
      }
      OFile = OFile->Parent;
      continue;
    }

    if (!Ext2FileNameIsValid (ComponentName, NewFileName)) {
      return EFI_INVALID_PARAMETER;
    }
    //
    // We have a component name, try to open it
    //
    if (!EXT2_ISDIR(OFile->Mode)) {
      //
      // This file isn't a directory, can't open it
      //
      return EFI_NOT_FOUND;
    }
    //
    // Search the compName in the directory
    //
    InodeNum = 0;
    Status = Ext2SearchDir (OFile, NewFileName, &InodeNum);

    if (InodeNum == 0) {
      //
      // component name is not found in the directory
      //
      if (*Next != 0) {
        return EFI_NOT_FOUND;
      }

      if (DirIntended && (Attributes & EFI_FILE_DIRECTORY) == 0) {
        return EFI_INVALID_PARAMETER;
      }
      //
      // It's the last component name - return with the open
      // path and the remaining name
      //
      break;
    }

    Status = Ext2OpenInode (InodeNum, OFile, NewFileName, &NewOFile);
    if (EFI_ERROR (Status)) {
      return Status;
    }

    

    OFile = NewOFile;
  }

  *PtrOFile = OFile;
  return EFI_SUCCESS;
}

