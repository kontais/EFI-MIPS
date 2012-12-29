/*++

Copyright (c) 2012, kontais                                                                   
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  Misc.c

Abstract:

  Ext2 file system init.

--*/

#include "Ext2.h"


EFI_STATUS
Ext2DiskIo (
  IN     EXT2_VOLUME       *Volume,
  IN     IO_MODE          IoMode,
  IN     UINT64           Offset,
  IN     UINTN            BufferSize,
  IN OUT VOID             *Buffer
  )
/*++

Routine Description:

  General disk access function

Arguments:

  Volume                - EXT2 file system volume.
  IoMode                - The access mode (disk read/write or cache access).
  Offset                - The starting byte offset to read from.
  BufferSize            - Size of Buffer.
  Buffer                - Buffer containing read data.

Returns:

  EFI_SUCCESS           - The operation is performed successfully.
  EFI_VOLUME_CORRUPTED  - The accesss is
  Others                - The status of read/write the disk

--*/
{
  EFI_STATUS            Status;
  EFI_DISK_IO_PROTOCOL  *DiskIo;
  EFI_DISK_READ         IoFunction;

  //
  // Verify the IO is in devices range
  //
  Status = EFI_VOLUME_CORRUPTED;
  if (Offset + BufferSize <= Volume->VolumeSize) {
    //
    // Access disk directly
    //
    DiskIo      = Volume->DiskIo;
    IoFunction  = (IoMode == READ_DATA) ? DiskIo->ReadDisk : DiskIo->WriteDisk;
    Status      = IoFunction (DiskIo, Volume->MediaId, Offset, BufferSize, Buffer);
  }

  if (EFI_ERROR (Status)) {
    Volume->DiskError = TRUE;
  }

  return Status;
}

VOID
Ext2AcquireLock (
  VOID
  )
/*++

Routine Description:

  Lock the volume.

Arguments:

  None.

Returns:

  None.

--*/
{
  EfiAcquireLock (&Ext2FsLock);
}

EFI_STATUS
Ext2AcquireLockOrFail (
  VOID
  )
/*++

Routine Description:

  Lock the volume.
  If the lock is already in the acquired state, then EFI_ACCESS_DENIED is returned.
  Otherwise, EFI_SUCCESS is returned.

Arguments:

  None.

Returns:

  EFI_SUCCESS           - The volume is locked.
  EFI_ACCESS_DENIED     - The volume could not be locked because it is already locked.

--*/
{
  return EfiAcquireLockOrFail (&Ext2FsLock);
}

VOID
Ext2ReleaseLock (
  VOID
  )
/*++

Routine Description:

  Unlock the volume.

Arguments:

  Null.

Returns:

  None.

--*/
{
  EfiReleaseLock (&Ext2FsLock);
}

VOID
Ext2FreeVolume (
  IN EXT2_VOLUME       *Volume
  )
/*++

Routine Description:

  Free volume structure (including the contents of directory cache and disk cache).

Arguments:

  Volume                - The volume structure to be freed.

Returns:

  None.

--*/
{
  gBS->FreePool (Volume);
}

UINT32 DayPerMonth[MONSPERYEAR] = {31, 0, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

UINT32 
GetPassDays (
  UINT32 Year
  )
{
  UINT32 PassYear = Year - 1;
  return PASS_LEAP_YEARS(PassYear) * 366 + (PassYear - PASS_LEAP_YEARS(PassYear)) * 365;
}

VOID 
Ext2TimeToEfiTime (
  IN  UINT32        SecTime,
  OUT EFI_TIME      *ETime
  )
{
  UINT32  Year;
  UINT32  Month;
  UINT32  Day;
  UINT32  Hour;
  UINT32  Minute;
  UINT32  Second;
  UINT32  PassDays;

  Day     = SecTime / SECSPERDAY;
  SecTime = SecTime - SECSPERDAY * Day;
  Hour    = SecTime / SECSPERHOUR;
  SecTime = SecTime - SECSPERHOUR * Hour;
  Minute  = SecTime / SECSPERMIN;
  SecTime = SecTime - SECSPERMIN * Minute;
  Second  = SecTime;

  PassDays = GetPassDays(EPOCH_YEAR);
  Year     = (PassDays + Day) / 365;
  while (GetPassDays(Year) > (PassDays + Day)) {
    Year--;
  };

  Day = (PassDays + Day) - GetPassDays(Year);

  DayPerMonth[1] = isleap (Year) ? 29 : 28;
  for (Month = 0; Day >= DayPerMonth[Month]; Month++) {
    Day = Day - DayPerMonth[Month];
  }
  
  Month++;
  Day++;

  ETime->Year       = (UINT16)Year;
  ETime->Month      = (UINT8) Month;
  ETime->Day        = (UINT8) Day;
  ETime->Hour       = (UINT8) Hour;
  ETime->Minute     = (UINT8) Minute;
  ETime->Second     = (UINT8) Second;
  ETime->Nanosecond = 0;
  ETime->TimeZone   = EFI_UNSPECIFIED_TIMEZONE;
  ETime->Daylight   = 0;

}

