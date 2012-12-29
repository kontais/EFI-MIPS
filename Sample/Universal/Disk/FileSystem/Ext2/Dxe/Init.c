/*++

Copyright (c) 2012, kontais                                                                   
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  Init.c

Abstract:

  Ext2 file system init.

--*/

#include "Ext2.h"

EFI_STATUS
Ext2AllocateVolume (
  IN  EFI_HANDLE                Handle,
  IN  EFI_DISK_IO_PROTOCOL      *DiskIo,
  IN  EFI_BLOCK_IO_PROTOCOL     *BlockIo
  )
/*++

Routine Description:

  Allocates volume structure, detects Ext2 file system, installs protocol,
  and initialize cache.

Arguments:

  Handle                - The handle of parent device.
  DiskIo                - The DiskIo of parent device.
  BlockIo               - The BlockIo of parent devicel

Returns:

  EFI_SUCCESS           - Allocate a new volume successfully.
  EFI_OUT_OF_RESOURCES  - Can not allocate the memory.
  Others                - Allocating a new volume failed.

--*/
{
  EFI_STATUS   Status;
  EXT2_VOLUME  *Volume;

  //
  // Allocate a volume structure
  //
  Volume = AllocateZeroPool (sizeof (EXT2_VOLUME));
  if (Volume == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Initialize the structure
  //
  Volume->Signature                   = EXT2_VOLUME_SIGNATURE;
  Volume->Handle                      = Handle;
  Volume->DiskIo                      = DiskIo;
  Volume->BlockIo                     = BlockIo;
  Volume->MediaId                     = BlockIo->Media->MediaId;
  Volume->ReadOnly                    = BlockIo->Media->ReadOnly;
  Volume->VolumeInterface.Revision    = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_REVISION;
  Volume->VolumeInterface.OpenVolume  = Ext2OpenVolume;

  Status = Ext2ReadSuperBlock (Volume);

  if (EFI_ERROR (Status)) {
    goto Done;
  }

  InitializeListHead (&Volume->CheckRef);
  InitializeListHead (&Volume->OFiles);  


  
#if 0
  InitializeListHead (&Volume->DirCacheList);
  //
  // Initialize Root Directory entry
  //
  Volume->RootDirEnt.FileString       = Volume->RootFileString;
  Volume->RootDirEnt.Entry.Attributes = FAT_ATTRIBUTE_DIRECTORY;
  //
  // Check to see if there's a file system on the volume
  //
  Status = FatOpenDevice (Volume);
  if (EFI_ERROR (Status)) {
    goto Done;
  }
#endif

  //
  // Install our protocol interfaces on the device's handle
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Volume->Handle,
                  &gEfiSimpleFileSystemProtocolGuid,
                  &Volume->VolumeInterface,
                  NULL
                  );
  if (EFI_ERROR (Status)) {
    goto Done;
  }
  //
  // Volume installed
  //
  Volume->Valid = TRUE;

Done:
  if (EFI_ERROR (Status)) {
    Ext2FreeVolume (Volume);
  }

  return Status;
}

EFI_STATUS
Ext2AbandonVolume (
  IN EXT2_VOLUME    *Volume
  )
/*++

Routine Description:

  Called by FatDriverBindingStop(), Abandon the volume.

Arguments:

  Volume                - The volume to be abandoned.

Returns:

  EFI_SUCCESS           - Abandoned the volume successfully.
  Others                - Can not uninstall the protocol interfaces.

--*/
{
  EFI_STATUS  Status;
  BOOLEAN     LockedByMe;

  //
  // Uninstall the protocol interface.
  //
  if (Volume->Handle != NULL) {
    Status = gBS->UninstallMultipleProtocolInterfaces (
                    Volume->Handle,
                    &gEfiSimpleFileSystemProtocolGuid,
                    &Volume->VolumeInterface,
                    NULL
                    );
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  LockedByMe = FALSE;

  //
  // Acquire the lock.
  // If the caller has already acquired the lock (which
  // means we are in the process of some Fat operation),
  // we can not acquire again.
  //
  Status = Ext2AcquireLockOrFail ();
  if (!EFI_ERROR (Status)) {
    LockedByMe = TRUE;
  }
  //
  // The volume is still being used. Hence, set error flag for all OFiles still in
  // use. In two cases, we could get here. One is EFI_MEDIA_CHANGED, the other is
  // EFI_NO_MEDIA.
  //
  if (Volume->RootInode != NULL) {
    Ext2SetVolumeError (
      Volume->RootInode,
      Volume->BlockIo->Media->MediaPresent ? EFI_MEDIA_CHANGED : EFI_NO_MEDIA
      );
  }

  Volume->Valid = FALSE;

  //
  // Release the lock.
  // If locked by me, this means DriverBindingStop is NOT
  // called within an on-going Fat operation, so we should
  // take responsibility to cleanup and free the volume.
  // Otherwise, the DriverBindingStop is called within an on-going
  // Fat operation, we shouldn't check reference, so just let outer
  // FatCleanupVolume do the task.
  //
  if (LockedByMe) {
    Ext2CleanupVolume (Volume, NULL, EFI_SUCCESS);
    Ext2ReleaseLock ();
  }

  return EFI_SUCCESS;
}

