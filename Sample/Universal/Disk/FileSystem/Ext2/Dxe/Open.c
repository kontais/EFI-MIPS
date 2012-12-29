/*++

Copyright (c) 2012, kontais                                                                   
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  Open.c

Abstract:

  Open() function of Simple File System Protocol

Revision History

--*/

#include "Ext2.h"

EFI_STATUS
Ext2AllocateIFile (
  IN EXT2_OFILE    *OFile,
  OUT EXT2_IFILE   **PtrIFile
  )
/*++

Routine Description:

  Create an Open instance for the existing OFile.
  The IFile of the newly opened file is passed out.

Arguments:

  OFile                 - The file that serves as a starting reference point.
  PtrIFile              - The newly generated IFile instance.

Returns:

  EFI_OUT_OF_RESOURCES  - Can not allocate the memory for the IFile
  EFI_SUCCESS           - Create the new IFile for the OFile successfully

--*/
{
  EXT2_IFILE *IFile;

  ASSERT_VOLUME_LOCKED (OFile->Volume);

  //
  // Allocate a new open instance
  //
  IFile = AllocateZeroPool (sizeof (EXT2_IFILE));
  if (IFile == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  IFile->Signature = EXT2_IFILE_SIGNATURE;

  EfiCopyMem (&(IFile->Handle), &Ext2FileInterface, sizeof (EFI_FILE));

  IFile->OFile = OFile;
  InsertTailList (&OFile->Opens, &IFile->Link);

  EfiStrCpy (IFile->Name, OFile->OpenName);

  *PtrIFile = IFile;
  return EFI_SUCCESS;
}

EFI_STATUS
Ext2OFileOpen (
  IN  EXT2_OFILE           *OFile,
  OUT EXT2_IFILE           **NewIFile,
  IN  CHAR16               *FileName,
  IN  UINT64               OpenMode,
  IN  UINT8                Attributes
  )
/*++

Routine Description:

  Open a file for a file name relative to an existing OFile.
  The IFile of the newly opened file is passed out.

Arguments:

  OFile                 - The file that serves as a starting reference point.
  NewIFile              - The newly generated IFile instance.
  FileName              - The file name relative to the OFile.
  OpenMode              - Open mode.
  Attributes            - Attributes to set if the file is created.

Returns:

  EFI_SUCCESS           - Open the file successfully.
  EFI_INVALID_PARAMETER - The open mode is conflict with the attributes
                          or the file name is not valid.
  EFI_NOT_FOUND         - Conficts between dir intention and attribute.
  EFI_WRITE_PROTECTED   - Can't open for write if the volume is read only.
  EFI_ACCESS_DENIED     - If the file's attribute is read only, and the
                          open is for read-write fail it.
  EFI_OUT_OF_RESOURCES  - Can not allocate the memory.

--*/
{
  EXT2_VOLUME  *Volume;
  EFI_STATUS  Status;
  CHAR16      NewFileName[EXT2_NAME_LEN];
  //EXT2_DIR_ENTRY  *DirEntry;
  UINT16      Mode;
  BOOLEAN     WriteMode;

  Volume = OFile->Volume;
  ASSERT_VOLUME_LOCKED (Volume);
  WriteMode = (BOOLEAN) (OpenMode & EFI_FILE_MODE_WRITE);
  if (Volume->ReadOnly && WriteMode) {
    return EFI_WRITE_PROTECTED;
  }
  //
  // Verify the source file handle isn't in an error state
  //
  Status = OFile->Error;
  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Get new OFile for the file
  //
  Status = Ext2LocateOFile (&OFile, FileName, Attributes, NewFileName);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (*NewFileName != 0) {
    return EFI_UNSUPPORTED;
#if 0

    //
    // If there's a remaining part of the name, then we had
    // better be creating the file in the directory
    //
    if ((OpenMode & EFI_FILE_MODE_CREATE) == 0) {
      return EFI_NOT_FOUND;
    }

    Status = FatCreateDirEnt (OFile, NewFileName, Attributes, &DirEnt);
    if (EFI_ERROR (Status)) {
      return Status;
    }

    Status = FatOpenDirEnt (OFile, DirEnt);
    if (EFI_ERROR (Status)) {
      return Status;
    }

    OFile = DirEnt->OFile;
    if (OFile->ODir != NULL) {
      //
      // If we just created a directory, we need to create "." and ".."
      //
      Status = FatCreateDotDirEnts (OFile);
      if (EFI_ERROR (Status)) {
        return Status;
      }
    }
#endif
  }
  //
  // If the file's attribute is read only, and the open is for
  // read-write, then the access is denied.
  //
  Mode = OFile->Mode;
  //if ((Mode & EFI_FILE_READ_ONLY) != 0 && (Mode & FAT_ATTRIBUTE_DIRECTORY) == 0 && WriteMode) {
  //  return EFI_ACCESS_DENIED;
  //}
  //
  // Create an open instance of the OFile
  //
  Status = Ext2AllocateIFile (OFile, NewIFile);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  (*NewIFile)->ReadOnly = (BOOLEAN)!WriteMode;

  return Ext2OFileFlush (OFile);
}

EFI_STATUS
EFIAPI
Ext2Open (
  IN  EFI_FILE            *FHand,
  OUT EFI_FILE            **NewHandle,
  IN  CHAR16              *FileName,
  IN  UINT64              OpenMode,
  IN  UINT64              Attributes
  )
/*++
Routine Description:

  Implements Open() of Simple File System Protocol.

Arguments:

  FHand                 - File handle of the file serves as a starting reference point.
  NewHandle             - Handle of the file that is newly opened.
  FileName              - File name relative to FHand.
  OpenMode              - Open mode.
  Attributes            - Attributes to set if the file is created.

Returns:

  EFI_INVALID_PARAMETER - The FileName is NULL or the file string is empty.
                          The OpenMode is not supported.
                          The Attributes is not the valid attributes.
  EFI_OUT_OF_RESOURCES  - Can not allocate the memory for file string.
  EFI_SUCCESS           - Open the file successfully.
  Others                - The status of open file.

--*/
{
  EXT2_IFILE   *IFile;
  EXT2_IFILE   *NewIFile;
  EXT2_OFILE   *OFile;
  EFI_STATUS   Status;

  //
  // Perform some parameter checking
  //
  if ((FileName == NULL) || (NewHandle == NULL)) {
    return EFI_INVALID_PARAMETER;
  }
  //
  // Check for a valid mode
  //
  switch (OpenMode) {
  case EFI_FILE_MODE_READ:
  case EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE:
  case EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE:
    break;

  default:
    return EFI_INVALID_PARAMETER;
  }
  
  //
  // Check for valid Attributes for file creation case. 
  //
  if (((OpenMode & EFI_FILE_MODE_CREATE) != 0) && (Attributes & (EFI_FILE_READ_ONLY | (~EFI_FILE_VALID_ATTR))) != 0) {
	return EFI_INVALID_PARAMETER;
  }
  
  IFile = EXT2_IFILE_FROM_FILE_HANDLE (FHand);
  OFile = IFile->OFile;

  //
  // Lock
  //
  Ext2AcquireLock ();

  //
  // Open the file
  //
  Status = Ext2OFileOpen (OFile, &NewIFile, FileName, OpenMode, (UINT8) Attributes);

  //
  // If the file was opened, return the handle to the caller
  //
  if (!EFI_ERROR (Status)) {
    *NewHandle = &NewIFile->Handle;
  }
  //
  // Unlock
  //
  Status = Ext2CleanupVolume (OFile->Volume, NULL, Status);
  Ext2ReleaseLock ();

  return Status;
}

