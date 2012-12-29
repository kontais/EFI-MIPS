/*++

Copyright (c) 2012, kontais                                                                   
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  Inode.c

Abstract:


--*/

#include "Ext2.h"

EFI_STATUS
EFIAPI
Ext2GetInodeEntry (
  IN  EXT2_VOLUME    *Volume,
  IN  UINT32         InodeNum,
  OUT EXT2_INODE     *Inode
  )
/*++

Routine Description:

  Open an inode entry by the inode num

Arguments:

  InodeNum              - 

Returns:

  EFI_VOLUME_CORRUPTED  - The EXT2 type is error.
  EFI_SUCCESS           - Open the volume successfully.

--*/
{
  EFI_STATUS          Status;
  UINT32              InodeGrp;       // The Group that hold the inode
  UINT32              InodeOff;       // Index offset in the group
  EXT2_GRP_DESC       GrpDesc;
  UINT32              GrpDescSize;
  UINT64              Offset;
  UINT32              InodeSize;

  GrpDescSize = sizeof(EXT2_GRP_DESC);
  InodeSize   = Volume->InodeSize;

  //
  // Read Group Descriptor
  //
  InodeGrp = (InodeNum - 1) / Volume->InodesPerGroup;
  // The 1 is the Supper Block
  Offset   = (1 + Volume->FirstDataBlock) * Volume->BlockSize;
  Offset   = Offset + InodeGrp * GrpDescSize;
  Status   = Ext2DiskIo (Volume, READ_DATA, Offset, GrpDescSize, &GrpDesc);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  //
  // Read Inode Entry
  //
  InodeOff = (InodeNum - 1) % Volume->InodesPerGroup;
  Offset   = GrpDesc.bg_inode_table * Volume->BlockSize;
  Offset   = Offset + InodeOff * Volume->InodeSize;
  Status   = Ext2DiskIo (Volume, READ_DATA, Offset, InodeSize, Inode);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  return Status;
}


EFI_STATUS
EFIAPI
Ext2FindOpenedInode (
  IN  EXT2_VOLUME    *Volume,
  IN  UINT32         InodeNum,
  OUT EXT2_OFILE     **OpenedInode
  )
/*++

  Routine Description:

  Arguments:

  Returns:
    EFI_SUCCESS         - StreamHandle was found and *OpenedInode contains
                          the stream node.
    EFI_NOT_FOUND       - SearchHandle was not found in the stream database.

--*/
{
  EXT2_OFILE          *OFile;
  
  if (!IsListEmpty (&(Volume->OFiles))) {
    OFile = OFILE_FROM_OPENEDLINK (GetFirstNode (&(Volume->OFiles)));
    for (;;) {
      if (OFile->InodeNum == InodeNum) {
        *OpenedInode = OFile;
        return EFI_SUCCESS;
      } else if (IsNodeAtEnd (&(Volume->OFiles), &OFile->OpenedLink)) {
        break;
      } else {
        OFile = OFILE_FROM_OPENEDLINK (GetNextNode (&(Volume->OFiles), &OFile->OpenedLink));
      }
    }
  }
  
  return EFI_NOT_FOUND;
}

EFI_STATUS
EFIAPI
Ext2OpenInode (
  IN  UINT32         InodeNum,
  IN  EXT2_OFILE     *Parent OPTIONAL,
  IN  CHAR16         *OpenName OPTIONAL,
  OUT EXT2_OFILE     **NewInode
  )
/*++

Routine Description:

  Open an inode handle from inode num.

Arguments:

  InodeNum              - 

Returns:

  EFI_VOLUME_CORRUPTED  - The EXT2 type is error.
  EFI_SUCCESS           - Open the volume successfully.

--*/
{
  EFI_STATUS          Status;
  EXT2_OFILE          *OFile  = NULL;
  EXT2_VOLUME         *Volume;

  if (InodeNum == EXT2_ROOTINO) {
    Volume = EXT2_VOLUME_FROM_ROOT_INODE (NewInode);
  } else {
    Volume = Parent->Volume;
  }

  Status = Ext2FindOpenedInode (Volume, InodeNum, NewInode);
  if (!EFI_ERROR(Status)) {
    return EFI_SUCCESS;
  }

  OFile  = AllocateZeroPool(sizeof(EXT2_OFILE));
  if (OFile == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  
  Status = Ext2GetInodeEntry (Volume, InodeNum, &(OFile->Inode));
  if (EFI_ERROR(Status)) {
    gBS->FreePool (OFile);
    return Status;
  }

  OFile->Signature = EXT2_OFILE_SIGNATURE;
  OFile->Parent    = Parent;
  InitializeListHead (&OFile->Opens);
  InitializeListHead (&OFile->ChildHead);
  
  if (InodeNum == EXT2_ROOTINO) {
    Volume->RootInode  = OFile;
  } else {
    InsertTailList (&Parent->ChildHead, &OFile->ChildLink);
  }
  
  //
  // Fill OFile info
  //
  OFile->Volume    = Volume;
  OFile->InodeNum  = InodeNum;
  OFile->Size      = OFile->Inode.i_size;
  OFile->Mode      = OFile->Inode.i_mode;

  InsertHeadList (&Volume->CheckRef, &OFile->CheckLink);
  InsertHeadList (&Volume->OFiles, &OFile->OpenedLink);  

  if(OpenName != NULL) {
    EfiStrCpy (OFile->OpenName, OpenName);
  } else {
    OFile->OpenName[0] = L'\0';
  }

  *NewInode = OFile;

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
Ext2CloseInode (
  IN EXT2_OFILE   *OFile
  )
/*++

Routine Description:

  Open an inode handle from inode num.

Arguments:

  InodeNum              - 

Returns:

  EFI_VOLUME_CORRUPTED  - The EXT2 type is error.
  EFI_SUCCESS           - Open the volume successfully.

--*/
{
  EXT2_VOLUME  *Volume;
  UINT32       Index;

  Volume  = OFile->Volume;

  if (OFile->Parent == NULL) {
    Volume->RootInode = NULL;
  } else {
    RemoveEntryList (&OFile->ChildLink);
  }

  //
  // Remove form opened file list
  //
  RemoveEntryList (&OFile->OpenedLink);

  //
  // Free file block buffer
  //
  for (Index = 0; Index < 3; Index++) {
    if (OFile->BlockPtr[Index].Buffer != NULL) {
      gBS->FreePool (OFile->BlockPtr[Index].Buffer);
    }
  }
  
  gBS->FreePool (OFile);

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
Ext2VolumeBlockLookup (
  IN EXT2_OFILE   *OFile,
  IN     UINT32   FileBlockOffset,
  IN     UINT32   Deep,
  IN OUT UINT32   *VolueBlockIndex
  )
{
  EFI_STATUS   Status;
  EXT2_VOLUME  *Volume;
  UINT32       Level;
  UINT32       Index;
  UINT32       Mod = 1;
  UINT32       FileBlockTag;
  VOID         *Buffer;
  UINT32       BufferTag;
  UINT64       Offset;
  UINT32       BlockSize;

  if (!Deep) {
    return EFI_SUCCESS;
  }

  Volume    = OFile->Volume;
  BlockSize = Volume->BlockSize;

  for (Index = 1; Index < Deep; Index++) {
    Mod *= BlockSize / sizeof(UINT32);
  }

  Level     = 3 - Deep;
  Buffer    = OFile->BlockPtr[Level].Buffer;
  BufferTag = OFile->BlockPtr[Level].BufferTag;
  //
  // if no allocate, then allocate
  //
  if (Buffer == NULL) {
    Buffer = AllocateZeroPool (BlockSize);
    if (Buffer == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }
  }

  //
  // if never read, then read
  //
  if (BufferTag != *VolueBlockIndex) {
    Offset = *VolueBlockIndex * BlockSize;
    Status = Ext2DiskIo (Volume, READ_DATA, Offset, BlockSize, Buffer);
    if (EFI_ERROR(Status)) {
      return Status;
    }
  }

  //
  // Save the buffer
  //
  OFile->BlockPtr[Level].Buffer    = Buffer;
  OFile->BlockPtr[Level].BufferTag = *VolueBlockIndex;

  //
  // Get the volume block of the file block
  //
  FileBlockTag    = FileBlockOffset / Mod;
  *VolueBlockIndex = *((UINT32*)Buffer + FileBlockTag); 

  //
  // recursive lookup
  //
  FileBlockOffset = FileBlockOffset % Mod;
  return Ext2VolumeBlockLookup (OFile, FileBlockOffset, Deep - 1, VolueBlockIndex);  
}


EFI_STATUS
Ext2FileBlockToVolumeBlock (
  IN EXT2_OFILE   *OFile,
  IN UINT32       FileBlockIndex,
  OUT UINT32      *VolumeBlockIndex
  )
/*++

Routine Description:


Arguments:
  PtrOFile              - As input, the reference OFile; as output, the located OFile.
  FileName              - The file name relevant to the OFile.

Returns:

  EFI_NOT_FOUND         - The file name can't be opened and there is more than one
  EFI_SUCCESS           - Find the volume block

--*/
{
  EXT2_INODE          *Inode;
  EXT2_VOLUME         *Volume;
  UINT32              FileBlockOffset;
  UINT32              DirectCount;
  UINT32              IndirectCount;
  UINT32              DoubleCount;
  UINT32              TripleCount;

  Inode = &(OFile->Inode);
  Volume= OFile->Volume;

  if (FileBlockIndex > (Inode->i_size / Volume->BlockSize)) {
    return EFI_INVALID_PARAMETER;
  }

  DirectCount   = EXT2_NDIR_BLOCKS;
  IndirectCount = Volume->BlockSize / sizeof(UINT32);
  DoubleCount   = IndirectCount * IndirectCount;
  TripleCount   = DoubleCount * IndirectCount;

  FileBlockOffset = FileBlockIndex;
  if (FileBlockOffset < DirectCount) {
    *VolumeBlockIndex = Inode->i_block[FileBlockOffset];
    return EFI_SUCCESS;
  }

  FileBlockOffset -= DirectCount;
  if (FileBlockOffset < IndirectCount) {
    *VolumeBlockIndex = Inode->i_block[EXT2_IND_BLOCK];
    return Ext2VolumeBlockLookup (OFile, FileBlockOffset, 1, VolumeBlockIndex);
  }

  FileBlockOffset -= IndirectCount;
  if (FileBlockOffset < DoubleCount) {
    *VolumeBlockIndex = Inode->i_block[EXT2_DIND_BLOCK];
    return Ext2VolumeBlockLookup (OFile, FileBlockOffset, 2, VolumeBlockIndex);
  }

  FileBlockOffset -= DoubleCount;
  if (FileBlockOffset < TripleCount) {
    *VolumeBlockIndex = Inode->i_block[EXT2_TIND_BLOCK];
    return Ext2VolumeBlockLookup (OFile, FileBlockOffset, 3, VolumeBlockIndex);
  }

  return EFI_NOT_FOUND;
}

