/*++

Copyright (c) 2012, kontais                                                                   
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  OpenVolume.c

Abstract:

  OpenVolume() function of Simple File System Protocol

Revision History

--*/

#include "Ext2.h"

EFI_STATUS
EFIAPI
Ext2OpenVolume (
  IN  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL  *This,
  OUT EFI_FILE                         **Root
  )
/*++

Routine Description:

  Implements Simple File System Protocol interface function OpenVolume().

Arguments:

  This                  - Calling context.
  File                  - the Root Directory of the volume.

Returns:

  EFI_OUT_OF_RESOURCES  - Can not allocate the memory.
  EFI_VOLUME_CORRUPTED  - The EXT2 type is error.
  EFI_SUCCESS           - Open the volume successfully.

--*/
{
  EFI_STATUS   Status;
  EXT2_VOLUME  *Volume;
  EXT2_IFILE   *IFile;

  Volume = EXT2_VOLUME_FROM_FILE_SYSTEM(This);
  Ext2AcquireLock ();

  //
  // Open Root Inode
  //
  Status = Ext2OpenInode (EXT2_ROOTINO, NULL, L"\\", &Volume->RootInode);
  if (EFI_ERROR (Status)) {
    goto Done;
  }
  //
  // Open a new instance to the root
  //
  Status = Ext2AllocateIFile (Volume->RootInode, &IFile);
  if (!EFI_ERROR (Status)) {
    *Root = &IFile->Handle;
  }

Done:

  Status = Ext2CleanupVolume (Volume, Volume->RootInode, Status);
  Ext2ReleaseLock ();

  return Status;
}

