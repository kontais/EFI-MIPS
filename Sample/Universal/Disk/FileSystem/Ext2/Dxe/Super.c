/*++

Copyright (c) 2012, kontais                                                                   
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  Super.c

Abstract:

  Ext2 File System driver routines that support EFI driver model

--*/

#include "Ext2.h"

EFI_STATUS
EFIAPI
Ext2CheckSuperBlock (
  EXT2_SUPER_BLOCK    *SuperBlock
  )
/*++

Routine Description:

  check super block

Arguments:

  SuperBlock     - 

Returns:

  EFI_SUCCESS    - The device or bus controller has been started.

  EFI_UNSUPPORT  - The request could not be completed due to lack of resources.

--*/
{
  if (SuperBlock->s_magic != E2FS_MAGIC) {
    return EFI_UNSUPPORTED;
  }

  if (SuperBlock->s_rev_level > E2FS_REV1) {
    return EFI_UNSUPPORTED;
  }

  if (SuperBlock->s_log_block_size > 2) {
    return EFI_UNSUPPORTED;
  }

  if (SuperBlock->s_rev_level > E2FS_REV0) {
		if (SuperBlock->s_first_ino != EXT2_FIRSTINO) {
			return EFI_UNSUPPORTED;
		}
		if (SuperBlock->s_feature_incompat & (~EXT2F_INCOMPAT_SUPP)) {
			return EFI_UNSUPPORTED;
		}
	}

#ifdef EXT2_DEBUG
  DEBUG((EFI_D_ERROR, "s_magic             0x%x\n", SuperBlock->s_magic));
  DEBUG((EFI_D_ERROR, "s_rev_level         0x%x\n", SuperBlock->s_rev_level));
  DEBUG((EFI_D_ERROR, "s_log_block_size    0x%x\n", SuperBlock->s_log_block_size));
  DEBUG((EFI_D_ERROR, "s_first_ino         0x%x\n", SuperBlock->s_first_ino));
  DEBUG((EFI_D_ERROR, "s_feature_incompat  0x%x\n", SuperBlock->s_feature_incompat));
  DEBUG((EFI_D_ERROR, "s_inode_size        0x%x\n", SuperBlock->s_inode_size));

  DEBUG((EFI_D_ERROR, "s_inodes_count      0x%x\n", SuperBlock->s_inodes_count));
  DEBUG((EFI_D_ERROR, "s_free_inodes_count 0x%x\n", SuperBlock->s_free_inodes_count));
  DEBUG((EFI_D_ERROR, "s_inodes_per_group  0x%x\n", SuperBlock->s_inodes_per_group));

  DEBUG((EFI_D_ERROR, "s_blocks_count      0x%x\n", SuperBlock->s_blocks_count));
  DEBUG((EFI_D_ERROR, "s_r_blocks_count    0x%x\n", SuperBlock->s_r_blocks_count));
  DEBUG((EFI_D_ERROR, "s_free_blocks_count 0x%x\n", SuperBlock->s_free_blocks_count));
  DEBUG((EFI_D_ERROR, "s_blocks_per_group  0x%x\n", SuperBlock->s_blocks_per_group));
#endif

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
Ext2ReadSuperBlock (
  IN OUT EXT2_VOLUME     *Volume
  )
/*++

Routine Description:

  Read the super block of the disk.

Arguments:

  Volume                - Return ext2 volume.

Returns:

  EFI_SUCCESS           - The device or bus controller has been started.

  EFI_OUT_OF_RESOURCES  - The request could not be completed due to lack of resources.

--*/
{
  EFI_STATUS          Status;
  EXT2_SUPER_BLOCK    *SuperBlock = NULL;

  SuperBlock = EfiLibAllocatePool(sizeof(EXT2_SUPER_BLOCK));
  
  if (SuperBlock == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Status = Volume->DiskIo->ReadDisk (
                     Volume->DiskIo,
                     Volume->MediaId,
                     SBSIZE,
                     sizeof(EXT2_SUPER_BLOCK),
                     (VOID*)SuperBlock
                     );
  
  if (EFI_ERROR(Status)) {
    goto Exit;
  }

  Status = Ext2CheckSuperBlock (SuperBlock);
  if (EFI_ERROR(Status)) {
    goto Exit;
  }

  Volume->BlockSize      = 1024 << SuperBlock->s_log_block_size;
  Volume->BlockCount     = SuperBlock->s_blocks_count;
  Volume->FreeBlockCount = SuperBlock->s_free_blocks_count;
  Volume->InodeSize      = SuperBlock->s_inode_size;
  Volume->FirstDataBlock = SuperBlock->s_first_data_block;
  Volume->InodesPerGroup = SuperBlock->s_inodes_per_group;
  Volume->RootInode      = NULL;

  Volume->VolumeSize     = (UINT64) ((UINT64)Volume->BlockCount * Volume->BlockSize);

  if (SuperBlock->s_rev_level == E2FS_REV0) {
    Volume->InodeSize    = E2FS_REV0_INODE_SIZE;
  }

  EfiAsciiStrToUnicodeStr (SuperBlock->s_volume_name, Volume->Label);

Exit:

  if (SuperBlock != NULL) {
    gBS->FreePool (SuperBlock);
  }
  
  return Status;
}

