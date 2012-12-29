/*++

Copyright (c) 2012, kontais                                                                   
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  ReadWrite.c

Abstract:

  Functions that perform file read/write

Revision History

--*/

#include "Ext2.h"

EFI_STATUS
EFIAPI
Ext2GetPosition (
  IN  EFI_FILE          *FHand,
  OUT UINT64            *Position
  )
/*++

Routine Description:

  Get the file's position of the file.

Arguments:

  FHand                 - The handle of file.
  Position              - The file's position of the file.

Returns:

  EFI_SUCCESS           - Get the info successfully.
  EFI_DEVICE_ERROR      - Can not find the OFile for the file.
  EFI_UNSUPPORTED       - The open file is not a file.

--*/
{
  EXT2_IFILE *IFile;
  EXT2_OFILE *OFile;

  IFile = EXT2_IFILE_FROM_FILE_HANDLE (FHand);
  OFile = IFile->OFile;

  if (OFile->Error == EFI_NOT_FOUND) {
    return EFI_DEVICE_ERROR;
  }

  if (EXT2_ISDIR(OFile->Mode)) {
    //
    // Dir do not support GetPosition
    //
    return EFI_UNSUPPORTED;
  }

  *Position = IFile->Position;
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
Ext2SetPosition (
  IN EFI_FILE           *FHand,
  IN UINT64             Position
  )
/*++

Routine Description:

  Set the file's position of the file.

Arguments:

  FHand                 - The handle of file.
  Position              - The file's position of the file.

Returns:

  EFI_SUCCESS           - Set the info successfully.
  EFI_DEVICE_ERROR      - Can not find the OFile for the file.
  EFI_UNSUPPORTED       - Set a directory with a not-zero position.

--*/
{
  EXT2_IFILE *IFile;
  EXT2_OFILE *OFile;

  IFile = EXT2_IFILE_FROM_FILE_HANDLE (FHand);
  OFile = IFile->OFile;

  if (OFile->Error == EFI_NOT_FOUND) {
    return EFI_DEVICE_ERROR;
  }
  //
  // If this is a directory, we can only set back to position 0
  //
  if (EXT2_ISDIR(OFile->Mode)) {
    if (Position != 0) {
      //
      // Reset current directory cursor;
      //
      return EFI_UNSUPPORTED;
    }
  }
  //
  // Set the position
  //
  if (Position == (UINT64)-1) {
    Position = OFile->Size;
  }
  //
  // Set the position
  //
  IFile->Position = Position;
  return EFI_SUCCESS;
}

EFI_STATUS
Ext2IFileReadDir (
  IN EXT2_IFILE              *IFile,
  IN OUT UINTN               *BufferSize,
  OUT VOID                   *Buffer
  )
/*++

Routine Description:

  Get the file info from the open file of the IFile into Buffer.

Arguments:

  IFile                 - The instance of the open file.
  BufferSize            - Size of Buffer.
  Buffer                - Buffer containing read data.

Returns:

  EFI_SUCCESS           - Get the file info successfully.
  other                 - An error occurred when operation the disk.

--*/
{
  EFI_STATUS       Status;
  EXT2_OFILE       *OFile;
  UINT64           CurrentPos;
  CHAR16           DirName[EXT2_NAME_LEN + 1];
  EXT2_DIR_ENTRY   DirEntry;

  OFile       = IFile->OFile;
  CurrentPos  = IFile->Position;

  //
  // We need to relocate the directory
  //
  if (CurrentPos != OFile->Position) {
    //
    // The directory cursor has been modified by another IFile, we reset the cursor
    //
    OFile->Position = 0;
  }
  //
  // We seek the next directory entry's position
  //
  Status = Ext2GetNextDirEntry (OFile, &DirEntry);
  if (EFI_ERROR (Status) || DirEntry.inode == 0) {
    //
    // Something error occurred or reach the end of directory,
    // return 0 buffersize
    //
    *BufferSize = 0;
    goto Done;
  }
  
  DirEntry.name[DirEntry.name_len] = '\0';
  EfiAsciiStrToUnicodeStr (DirEntry.name, DirName);
  
  Status = Ext2GetDirEntryInfo (OFile->Volume, DirName, DirEntry.inode, BufferSize, Buffer);

Done:
  //
  // Update IFile's Position
  //
  if (!EFI_ERROR (Status)) {
    //
    // Update IFile->Position, if everything is all right
    //
    IFile->Position = OFile->Position;
  }

  return Status;
}

EFI_STATUS
Ext2IFileAccess (
  IN     EFI_FILE              *FHand,
  IN     IO_MODE               IoMode,
  IN OUT UINTN                 *BufferSize,
  IN OUT VOID                  *Buffer
  )
/*++

Routine Description:

  Get the file info from the open file of the IFile into Buffer.

Arguments:

  FHand                 - The file handle to access.
  IoMode                - Indicate whether the access mode is reading or writing.
  BufferSize            - Size of Buffer.
  Buffer                - Buffer containing read data.

Returns:

  EFI_SUCCESS           - Get the file info successfully.
  EFI_DEVICE_ERROR      - Can not find the OFile for the file.
  EFI_VOLUME_CORRUPTED  - The file type of open file is error.
  EFI_WRITE_PROTECTED   - The disk is write protect.
  EFI_ACCESS_DENIED     - The file is read-only.
  other                 - An error occurred when operating on the disk.

--*/
{
  EFI_STATUS  Status;
  EXT2_IFILE   *IFile;
  EXT2_OFILE   *OFile;
  EXT2_VOLUME  *Volume;
  UINT64      EndPosition;

  IFile  = EXT2_IFILE_FROM_FILE_HANDLE (FHand);
  OFile  = IFile->OFile;
  Volume = OFile->Volume;

  if (OFile->Error == EFI_NOT_FOUND) {
    return EFI_DEVICE_ERROR;
  }

  if (IoMode == READ_DATA) {
    //
    // If position is at EOF, then return device error
    //
    if (IFile->Position > OFile->Size) {
      return EFI_DEVICE_ERROR;
    }
  } else {
    //
    // Check if the we can write data
    //
    if (Volume->ReadOnly) {
      return EFI_WRITE_PROTECTED;
    }

    if (IFile->ReadOnly) {
      return EFI_ACCESS_DENIED;
    }
  }

  Ext2AcquireLock ();

  Status = OFile->Error;
  if (!EFI_ERROR (Status)) {
    if (EXT2_ISDIR(OFile->Mode)) {
      //
      // Access a directory
      //
      Status = EFI_UNSUPPORTED;
      if (IoMode == READ_DATA) {
        //
        // Read a directory is supported
        //
        Status = Ext2IFileReadDir (IFile, BufferSize, Buffer);
      }

      OFile = NULL;
    } else {
      //
      // Access a file
      //
      EndPosition = IFile->Position + *BufferSize;
      if (EndPosition > OFile->Size) {
        //
        // The position goes beyond the end of file
        //
        if (IoMode == READ_DATA) {
          //
          // Adjust the actual size read
          //
          *BufferSize -= (UINTN) (EndPosition - OFile->Size);
        } else {
          //
          // We expand the file size of OFile
          //
          Status = Ext2GrowEof (OFile, EndPosition);
          if (EFI_ERROR (Status)) {
            //
            // Must update the file's info into the file's Directory Entry
            // and then flush the dirty cache info into disk.
            //
            *BufferSize = 0;
            Ext2OFileFlush (OFile);
            OFile = NULL;
            goto Done;
          }
        }
      }

      Status = Ext2AccessOFile (OFile, IoMode, (UINTN) IFile->Position, BufferSize, Buffer);
      IFile->Position += *BufferSize;
    }
  }

Done:
  if (EFI_ERROR (Status)) {
    Status = Ext2CleanupVolume (Volume, OFile, Status);
  }
  //
  // On EFI_SUCCESS case, not calling FatCleanupVolume():
  // 1) The Cache flush operation is avoided to enhance
  // performance. Caller is responsible to call Flush() when necessary.
  // 2) The volume dirty bit is probably set already, and is expected to be
  // cleaned in subsequent Flush() or other operations.
  // 3) Write operation doesn't affect OFile/IFile structure, so
  // Reference checking is not necessary.
  //
  Ext2ReleaseLock ();
  return Status;
}

EFI_STATUS
EFIAPI
Ext2Read (
  IN EFI_FILE           *FHand,
  IN OUT UINTN          *BufferSize,
  OUT VOID              *Buffer
  )
/*++

Routine Description:

  Get the file info.

Arguments:

  FHand                 - The handle of the file.
  BufferSize            - Size of Buffer.
  Buffer                - Buffer containing read data.

Returns:

  EFI_SUCCESS           - Get the file info successfully.
  EFI_DEVICE_ERROR      - Can not find the OFile for the file.
  EFI_VOLUME_CORRUPTED  - The file type of open file is error.
  other                 - An error occurred when operation the disk.

--*/
{
  return Ext2IFileAccess (FHand, READ_DATA, BufferSize, Buffer);
}

EFI_STATUS
EFIAPI
Ext2Write (
  IN     EFI_FILE           *FHand,
  IN OUT UINTN              *BufferSize,
  IN     VOID               *Buffer
  )
/*++

Routine Description:

  Write the content of buffer into files.

Arguments:

  FHand                 - The handle of the file.
  BufferSize            - Size of Buffer.
  Buffer                - Buffer containing write data.

Returns:

  EFI_SUCCESS           - Set the file info successfully.
  EFI_WRITE_PROTECTED   - The disk is write protect.
  EFI_ACCESS_DENIED     - The file is read-only.
  EFI_DEVICE_ERROR      - The OFile is not valid.
  EFI_UNSUPPORTED       - The open file is not a file.
                        - The writing file size is larger than 4GB.
  other                 - An error occurred when operation the disk.

--*/
{
  //
  // BUGBUG, This driver not support ext2 write now
  //
  return EFI_UNSUPPORTED;
  //return Ext2IFileAccess (FHand, WRITE_DATA, BufferSize, Buffer);
}

EFI_STATUS
Ext2AccessOFile (
  IN     EXT2_OFILE      *OFile,
  IN     IO_MODE        IoMode,
  IN     UINTN          Position,
  IN OUT UINTN          *DataBufferSize,
  IN OUT UINT8          *UserBuffer
  )
/*++

Routine Description:

  This function reads data from a file or writes data to a file.
  It uses OFile->PosRem to determine how much data can be accessed in one time.

Arguments:

  OFile                 - The open file.
  IoMode                - Indicate whether the access mode is reading or writing.
  Position              - The position where data will be accessed.
  DataBufferSize        - Size of Buffer.
  UserBuffer            - Buffer containing data.

Returns:

  EFI_SUCCESS           - Access the data successfully.
  other                 - An error occurred when operating on the disk.

--*/
{
  EFI_STATUS  Status;
  EXT2_VOLUME  *Volume;
  UINTN       Len;
  UINTN       BufferSize;

  BufferSize  = *DataBufferSize;
  Volume      = OFile->Volume;
  ASSERT_VOLUME_LOCKED (Volume);

  Status = EFI_SUCCESS;
  while (BufferSize > 0) {
    //
    // Seek the OFile to the file position
    //
    Status = Ext2OFilePosition (OFile, Position, BufferSize);
    if (EFI_ERROR (Status)) {
      break;
    }
    //
    // Clip length to block run
    //
    Len = BufferSize > OFile->PosRem ? OFile->PosRem : BufferSize;

    //
    // Write the data
    //
    Status = Ext2DiskIo (Volume, IoMode, OFile->PosDisk, Len, UserBuffer);
    if (EFI_ERROR (Status)) {
      break;
    }
    //
    // Data was successfully accessed
    //
    Position   += Len;
    UserBuffer += Len;
    BufferSize -= Len;

#if 0
    if (IoMode == WRITE_DATA) {
      OFile->Dirty    = TRUE;
      OFile->Archive  = TRUE;
    }
#endif

     OFile->Position = Position;

    //
    // Make sure no outbound occurred
    //
    ASSERT (Position <= OFile->Size);
  }
  //
  // Update the number of bytes accessed
  //
  *DataBufferSize -= BufferSize;
  return Status;
}

