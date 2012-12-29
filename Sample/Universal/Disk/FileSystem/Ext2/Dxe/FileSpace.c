/*++

Copyright (c) 2012, kontais                                                                   
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  FileSpace.c

Abstract:

  Routines dealing with inode

Revision History

--*/

#include "Ext2.h"

EFI_STATUS
Ext2GrowEof (
  IN EXT2_OFILE            *OFile,
  IN UINT64               NewSizeInBytes
  )
/*++

Routine Description:

  Grow the end of the open file base on the NewSizeInBytes.

Arguments:

  OFile                 - The open file.
  NewSizeInBytes        - The new size in bytes of the open file.

Returns:

  EFI_SUCCESS           - The file is grown sucessfully.
  EFI_UNSUPPORTED       - The file size is larger than 4GB.
  EFI_VOLUME_CORRUPTED  - There are errors in the files' clusters.
  EFI_VOLUME_FULL       - The volume is full and can not grow the file.

--*/
{
  return EFI_SUCCESS;
}

EFI_STATUS
Ext2OFilePosition (
  IN EXT2_OFILE       *OFile,
  IN UINTN            Position,
  IN UINTN            PosLimit
  )
/*++

Routine Description:

  Seek OFile to requested position, and calculate the number of
  consecutive clusters from the position in the file

Arguments:

  OFile                 - The open file.
  Position              - The file's position which will be accessed.
  PosLimit              - The maximum length current reading/writing may access

Returns:

  EFI_SUCCESS           - Set the info successfully.
  EFI_VOLUME_CORRUPTED  - Cluster chain corrupt.

--*/
{
  EFI_STATUS   Status;
  EXT2_VOLUME  *Volume;
  UINTN        BlockSize;
  UINT32       FileBlock;
  UINT32       VolumeBlock;

  Volume    = OFile->Volume;
  BlockSize = Volume->BlockSize;

  ASSERT_VOLUME_LOCKED (Volume);

  FileBlock = Position / BlockSize;

  Status = Ext2FileBlockToVolumeBlock (OFile, FileBlock, &VolumeBlock);
  if (EFI_ERROR(Status)) {
    return Status;
  }
  
  OFile->PosDisk  = VolumeBlock * BlockSize + Position % BlockSize;
  OFile->PosRem   = BlockSize - Position % BlockSize;

  return EFI_SUCCESS;
}

