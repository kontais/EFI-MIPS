/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  debug.h

Abstract:

  declares interface functions

Revision History

--*/

#ifndef _DEBUG_H_
#define _DEBUG_H_
#include "EfiShellLib.h"
#include EFI_PROTOCOL_DEFINITION (BlockIo)

#define EFI_PARTITION_SIGNATURE       0x5053595320494249
typedef struct _EFI_PARTITION_HEADER {
  EFI_TABLE_HEADER  Hdr;
  UINT32            DirectoryAllocationNumber;
  UINT32            BlockSize;
  EFI_LBA           FirstUsableLba;
  EFI_LBA           LastUsableLba;
  EFI_LBA           UnusableSpace;
  EFI_LBA           FreeSpace;
  EFI_LBA           RootFile;
  EFI_LBA           SecutiryFile;
} EFI_PARTITION_HEADER;

typedef enum {
  FAT12,
  FAT16,
  FAT32,
  FatUndefined
} FAT_VOLUME_TYPE;

//
// Used for fat12 & fat16
//
typedef struct {
  UINT8   Ia32Jump[3];
  CHAR8   OemId[8];

  UINT16  SectorSize;
  UINT8   SectorsPerCluster;
  UINT16  ReservedSectors;
  UINT8   NoFats;
  UINT16  RootEntries;          // < FAT32, root dir is fixed size
  UINT16  Sectors;
  UINT8   Media;                // (ignored)
  UINT16  SectorsPerFat;        // < FAT32
  UINT16  SectorsPerTrack;      // (ignored)
  UINT16  Heads;                // (ignored)
  UINT32  HiddenSectors;        // (ignored)
  UINT32  LargeSectors;         // => FAT32
  UINT8   PhysicalDriveNumber;  // (ignored)
  UINT8   CurrentHead;          // holds boot_sector_dirty bit
  UINT8   Signature;            // (ignored)
  CHAR8   Id[4];
  CHAR8   FatLabel[11];
  CHAR8   SystemId[8];

} FAT_BOOT_SECTOR;

typedef struct {
  UINT8   Ia32Jump[3];
  CHAR8   OemId[8];

  UINT16  SectorSize;
  UINT8   SectorsPerCluster;
  UINT16  ReservedSectors;
  UINT8   NoFats;
  UINT16  RootEntries;          // < FAT32, root dir is fixed size
  UINT16  Sectors;
  UINT8   Media;                // (ignored)
  UINT16  SectorsPerFat;        // < FAT32
  UINT16  SectorsPerTrack;      // (ignored)
  UINT16  Heads;                // (ignored)
  UINT32  HiddenSectors;        // (ignored)
  UINT32  LargeSectors;         // Used if Sectors==0
  UINT32  LargeSectorsPerFat;   // FAT32
  UINT16  ExtendedFlags;        // FAT32 (ignored)
  UINT16  FsVersion;            // FAT32 (ignored)
  UINT32  RootDirFirstCluster;  // FAT32
  UINT16  FsInfoSector;         // FAT32
  UINT16  BackupBootSector;     // FAT32
  UINT8   Reserved[12];         // FAT32 (ignored)
  UINT8   PhysicalDriveNumber;  // (ignored)
  UINT8   CurrentHead;          // holds boot_sector_dirty bit
  UINT8   Signature;            // (ignored)
  CHAR8   Id[4];
  CHAR8   FatLabel[11];
  CHAR8   SystemId[8];
} FAT_BOOT_SECTOR_EX;

//
// ignored - means not used in the EFI driver
//
// For <Fat32 RootEntries indicates the number of entries in the root directory,
// and SectorsPerFat is used to size the fact table
//
typedef struct {
  UINT16  Day : 5;
  UINT16  Month : 4;
  UINT16  Year : 7;                 // From 1980
} FAT_DATE;

typedef struct {
  UINT16  DoubleSecond : 5;
  UINT16  Minute : 6;
  UINT16  Hour : 5;
} FAT_TIME;

typedef struct {
  FAT_TIME  Time;
  FAT_DATE  Date;
} FAT_DATE_TIME;

typedef struct {
  CHAR8         FileName[11];       // 8.3 filename
  UINT8         Attributes;
  UINT8         CaseFlag;
  UINT8         CreateMillisecond;  // (creation milliseconds - ignored)
  FAT_DATE_TIME FileCreateTime;
  FAT_DATE      FileLastAccess;
  UINT16        FileClusterHigh;    // >= FAT32
  FAT_DATE_TIME FileModificationTime;
  UINT16        FileCluster;
  UINT32        FileSize;
} FAT_DIRECTORY_ENTRY;

#define EFI_FILE_CLASS_FREE_SPACE 1
#define EFI_FILE_CLASS_EMPTY      2
#define EFI_FILE_CLASS_NORMAL     3

VOID
EFIStructsPrint (
  IN  VOID                    *Buffer,
  IN  UINTN                   BlockSize,
  IN  UINT64                  BlockAddress,
  IN  EFI_BLOCK_IO_PROTOCOL   *BlkIo
  );

extern EFI_HII_HANDLE HiiDmemHandle;

#endif
