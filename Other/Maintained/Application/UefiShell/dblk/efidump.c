/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  EfiDump.c
  
Abstract: 

  Dump out info about EFI structures



Revision History

--*/

#include "dblk.h"
#include "EfiPart.h"
#include "EfiShellLib.h"

#include STRING_DEFINES_FILE

typedef union {
  FAT_BOOT_SECTOR     Fat;
  FAT_BOOT_SECTOR_EX  Fat32;
} EFI_FAT_BOOT_SECTOR;

FAT_VOLUME_TYPE
DblkValidBPB (
  IN  EFI_FAT_BOOT_SECTOR *Bpb,
  IN  UINTN               BlockSize
  );

VOID
DblkDumpBpb (
  IN  FAT_VOLUME_TYPE     FatType,
  IN  EFI_FAT_BOOT_SECTOR *Bpb,
  IN  UINTN               BlockSize
  );

typedef union {
  VOID                  *RawData;
  UINT8                 *PtrMath;
  EFI_TABLE_HEADER      *Hdr;
  EFI_PARTITION_HEADER  *Partition;
  EFI_SYSTEM_TABLE      *Sys;
  EFI_RUNTIME_SERVICES  *RT;
  EFI_BOOT_SERVICES     *BS;
} EFI_TABLE_HEADER_PTR;

VOID
EFIFileAttributePrint (
  IN UINT64 Attrib
  );

CHAR16  *
DblkEFIClassString (
  IN  UINT32  Class
  );

BOOLEAN
DblkIsValidEfiHeader (
  IN  UINTN                 BlockSize,
  IN  EFI_TABLE_HEADER_PTR  Hdr
  );

VOID
DblkTableHeaderPrint (
  IN  VOID      *Buffer,
  IN  UINTN     ByteSize,
  IN  UINT64    BlockAddress,
  IN  BOOLEAN   IsBlkDevice
  );

VOID
DblkDumpGenericHeader (
  IN  EFI_TABLE_HEADER_PTR  Tbl
  );

VOID
DblkDumpPartition (
  IN  EFI_TABLE_HEADER_PTR  Tbl
  );

VOID
DumpFileHeader (
  IN  EFI_TABLE_HEADER_PTR  Tbl
  );

VOID
DblkDumpSystemTable (
  IN  EFI_TABLE_HEADER_PTR  Tbl
  );

VOID
DblkStructsPrint (
  IN  VOID                    *Buffer,
  IN  UINTN                   BlockSize,
  IN  UINT64                  BlockAddress,
  IN  EFI_BLOCK_IO_PROTOCOL   *BlkIo
  )
{
  MASTER_BOOT_RECORD  *Mbr;
  INTN                Index;
  BOOLEAN             IsBlkDevice;
  FAT_VOLUME_TYPE     FatType;

  Print (L"\n");
  Mbr = NULL;
  if (BlockAddress == 0 && BlkIo != NULL) {
    Mbr = (MASTER_BOOT_RECORD *) Buffer;
    if (ValidMBR (Mbr, BlkIo)) {
      PrintToken (STRING_TOKEN (STR_DEBUG_VALID_MBR), HiiDblkHandle);
      for (Index = 0; Index < MAX_MBR_PARTITIONS; Index++) {

        PrintToken (
          STRING_TOKEN (STR_DEBUG_PARTITION_START),
          HiiDblkHandle,
          Index,
          Mbr->Partition[Index].OSIndicator,
          EXTRACT_UINT32 (Mbr->Partition[Index].StartingLBA),
          EXTRACT_UINT32 (Mbr->Partition[Index].SizeInLBA)
          );
      }
    } else {
      FatType = DblkValidBPB ((EFI_FAT_BOOT_SECTOR *) Buffer, BlockSize);
      if (FatType != FatUndefined) {
        DblkDumpBpb (FatType, (EFI_FAT_BOOT_SECTOR *) Buffer, BlockSize);
      }
    }
  }

  if (!Mbr) {
    IsBlkDevice = (BOOLEAN) (BlkIo != NULL);
    DblkTableHeaderPrint (Buffer, BlockSize, BlockAddress, IsBlkDevice);
  }
}

typedef
VOID
(*EFI_DUMP_HEADER) (
  IN  EFI_TABLE_HEADER_PTR  Tbl
  );

typedef struct {
  UINT64          Signature;
  CHAR16          *String;
  EFI_DUMP_HEADER Func;
} TABLE_HEADER_INFO;

TABLE_HEADER_INFO DblkTableHeader[] = {
  EFI_PARTITION_SIGNATURE,
  L"Partition",
  DblkDumpPartition,
  EFI_SYSTEM_TABLE_SIGNATURE,
  L"System",
  DblkDumpSystemTable,
  EFI_BOOT_SERVICES_SIGNATURE,
  L"Boot Services",
  DblkDumpGenericHeader,
  EFI_RUNTIME_SERVICES_SIGNATURE,
  L"Runtime Services",
  DblkDumpGenericHeader,
  0x00,
  L"",
  DblkDumpGenericHeader
};

static UINT8   FatNumber[] = { 12, 16, 32, 0 };

VOID
DblkTableHeaderPrint (
  IN  VOID      *Buffer,
  IN  UINTN     ByteSize,
  IN  UINT64    BlockAddress,
  IN  BOOLEAN   IsBlkDevice
  )
{
  EFI_TABLE_HEADER_PTR  Hdr;
  INTN                  Index;

  Hdr.RawData = Buffer;
  if (DblkIsValidEfiHeader (ByteSize, Hdr)) {
    for (Index = 0; DblkTableHeader[Index].Signature != 0x00; Index++) {

      if (DblkTableHeader[Index].Signature == Hdr.Hdr->Signature) {
        PrintToken (
          STRING_TOKEN (STR_DEBUG_VALID_EFI_HEADER),
          HiiDblkHandle,
          (IsBlkDevice ? "LBA" : "Address"),
          BlockAddress,
          (IsBlkDevice ? "" : "----")
          );
        PrintToken (
          STRING_TOKEN (STR_DEBUG_TABLE_STRUCT),
          HiiDblkHandle,
          DblkTableHeader[Index].String,
          Hdr.Hdr->HeaderSize,
          Hdr.Hdr->Revision
          );
        DblkTableHeader[Index].Func (Hdr);
        return ;
      }
    }
    //
    // Dump the Generic Header, since we don't know the type
    //
    DblkTableHeader[Index].Func (Hdr);
  }
}

BOOLEAN
DblkIsValidEfiHeader (
  IN  UINTN                 BlockSize,
  IN  EFI_TABLE_HEADER_PTR  Hdr
  )
{
  if ((UINTN) Hdr.RawData > (UINTN) ((UINT8 *) Hdr.RawData + BlockSize)) {
    return FALSE;
  }

  if (Hdr.Hdr == NULL) {
    return FALSE;
  }
  //
  // Check buffer alignment to solve native alignment issue on IA-64 processor
  //
  if ((((UINTN) Hdr.RawData) & 0x07) != 0) {
    return FALSE;
  }

  if (Hdr.Hdr->HeaderSize == 0x00) {
    return FALSE;
  }

  if (Hdr.Hdr->HeaderSize > BlockSize) {
    return FALSE;
  }

  return TRUE;
}

VOID
DblkDumpGenericHeader (
  IN  EFI_TABLE_HEADER_PTR  Tbl
  )
{
  PrintToken (
    STRING_TOKEN (STR_DEBUG_HEADER_REV),
    HiiDblkHandle,
    Tbl.Hdr->Signature,
    Tbl.Hdr->Revision,
    Tbl.Hdr->HeaderSize,
    Tbl.Hdr->CRC32
    );
}

VOID
DblkDumpPartition (
  IN  EFI_TABLE_HEADER_PTR  Tbl
  )
{
  PrintToken (
    STRING_TOKEN (STR_DEBUG_LBA_UNUSABLE),
    HiiDblkHandle,
    Tbl.Partition->FirstUsableLba,
    Tbl.Partition->LastUsableLba,
    Tbl.Partition->UnusableSpace
    );
  PrintToken (
    STRING_TOKEN (STR_DEBUG_FREE_SPACE),
    HiiDblkHandle,
    Tbl.Partition->FreeSpace,
    Tbl.Partition->RootFile
    );
  PrintToken (
    STRING_TOKEN (STR_DEBUG_BLOCK_SIZE),
    HiiDblkHandle,
    Tbl.Partition->BlockSize,
    Tbl.Partition->DirectoryAllocationNumber
    );
}

CHAR16 *
DblkEFIClassString (
  IN  UINT32  Class
  )
{
  switch (Class) {
  case EFI_FILE_CLASS_FREE_SPACE:
    return L"Free Space";

  case EFI_FILE_CLASS_EMPTY:
    return L"Empty";

  case EFI_FILE_CLASS_NORMAL:
    return L"Normal";

  default:
    return L"Invalid";
  }
}

VOID
DblkDumpSystemTable (
  IN  EFI_TABLE_HEADER_PTR  Tbl
  )
{
  EFI_STATUS                Status;
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath;
  VOID                      *AcpiTable;
  VOID                      *Acpi20Table;
  VOID                      *SMBIOSTable;
  VOID                      *SalSystemTable;
  VOID                      *MpsTable;

  AcpiTable       = NULL;
  Acpi20Table     = NULL;
  SMBIOSTable     = NULL;
  SalSystemTable  = NULL;
  MpsTable        = NULL;

  PrintToken (
    STRING_TOKEN (STR_DEBUG_CONIN_CONOUT),
    HiiDblkHandle,
    Tbl.Sys->ConsoleInHandle,
    Tbl.Sys->ConsoleOutHandle,
    Tbl.Sys->StandardErrorHandle
    );

  Status = BS->HandleProtocol (Tbl.Sys->ConsoleInHandle, &gEfiDevicePathProtocolGuid, (VOID *) &DevicePath);
  if (!EFI_ERROR (Status) && DevicePath) {
    PrintToken (STRING_TOKEN (STR_DEBUG_CONSOLE_IN_ON), HiiDblkHandle, LibDevicePathToStr (DevicePath));
  }

  Status = BS->HandleProtocol (Tbl.Sys->ConsoleOutHandle, &gEfiDevicePathProtocolGuid, (VOID *) &DevicePath);
  if (!EFI_ERROR (Status) && DevicePath) {
    PrintToken (STRING_TOKEN (STR_DEBUG_CONSOLE_OUT_ON), HiiDblkHandle, LibDevicePathToStr (DevicePath));
  }

  Status = BS->HandleProtocol (Tbl.Sys->StandardErrorHandle, &gEfiDevicePathProtocolGuid, (VOID *) &DevicePath);
  if (!EFI_ERROR (Status) && DevicePath) {
    PrintToken (STRING_TOKEN (STR_DEBUG_STD_ERROR_ON), HiiDblkHandle, LibDevicePathToStr (DevicePath));
  }

  PrintToken (STRING_TOKEN (STR_DEBUG_RUNTIME_SERVICES), HiiDblkHandle, (UINT64) Tbl.Sys->RuntimeServices);
  PrintToken (STRING_TOKEN (STR_DEBUG_BOOT_SERVICES), HiiDblkHandle, (UINT64) Tbl.Sys->BootServices);

  EFI64_CODE (
    Status = LibGetSystemConfigurationTable(&gEfiSalSystemTableGuid, &SalSystemTable);
    if (!EFI_ERROR(Status)) {
      PrintToken (STRING_TOKEN(STR_DEBUG_SAL_SYSTEM_TABLE), HiiDblkHandle, (UINT64)SalSystemTable);
    }
  )

  Status = LibGetSystemConfigurationTable (&gEfiAcpiTableGuid, &AcpiTable);
  if (!EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_DEBUG_ACPI_TABLE), HiiDblkHandle, (UINT64) AcpiTable);
  }

  Status = LibGetSystemConfigurationTable (&gEfiAcpi20TableGuid, &Acpi20Table);
  if (!EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_DEBUG_APCI_2_TABLE), HiiDblkHandle, (UINT64) Acpi20Table);
  }

  Status = LibGetSystemConfigurationTable (&gEfiMpsTableGuid, &MpsTable);
  if (!EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_DEBUG_MPS_TABLE), HiiDblkHandle, (UINT64) MpsTable);
  }

  Status = LibGetSystemConfigurationTable (&gEfiSmbiosTableGuid, &SMBIOSTable);
  if (!EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_DEBUG_SMBIOS_TABLE), HiiDblkHandle, (UINT64) SMBIOSTable);
  }
}

FAT_VOLUME_TYPE
DblkValidBPB (
  IN  EFI_FAT_BOOT_SECTOR *Bpb,
  IN  UINTN               BlockSize
  )
{
  UINT32          BlockSize32;
  BOOLEAN         IsFat;
  UINT32          RootSize;
  UINT32          RootLba;
  UINT32          FirstClusterLba;
  UINT32          MaxCluster;
  UINT32          Sectors;
  FAT_VOLUME_TYPE FatType;

  BlockSize32 = (UINT32) BlockSize;

  if (Bpb->Fat.SectorsPerFat == 0) {
    FatType = FAT32;
  } else {
    FatType = FatUndefined;
  }

  IsFat = TRUE;
  if (Bpb->Fat.Ia32Jump[0] != 0xe9 && Bpb->Fat.Ia32Jump[0] != 0xeb && Bpb->Fat.Ia32Jump[0] != 0x49) {
    IsFat = FALSE;
  }

  Sectors = (Bpb->Fat.Sectors == 0) ? Bpb->Fat.LargeSectors : Bpb->Fat.Sectors;

  if (Bpb->Fat.SectorSize != BlockSize32 || Bpb->Fat.ReservedSectors == 0 || Bpb->Fat.NoFats == 0 || Sectors == 0) {
    IsFat = FALSE;
  }

  if (Bpb->Fat.SectorsPerCluster != 1 &&
      Bpb->Fat.SectorsPerCluster != 2 &&
      Bpb->Fat.SectorsPerCluster != 4 &&
      Bpb->Fat.SectorsPerCluster != 8 &&
      Bpb->Fat.SectorsPerCluster != 16 &&
      Bpb->Fat.SectorsPerCluster != 32 &&
      Bpb->Fat.SectorsPerCluster != 64 &&
      Bpb->Fat.SectorsPerCluster != 128
      ) {
    IsFat = FALSE;
  }

  if (FatType == FAT32 && (Bpb->Fat32.LargeSectorsPerFat == 0 || Bpb->Fat32.FsVersion != 0)) {
    IsFat = FALSE;
  }

  if (Bpb->Fat.Media != 0xf0 &&
      Bpb->Fat.Media != 0xf8 &&
      Bpb->Fat.Media != 0xf9 &&
      Bpb->Fat.Media != 0xfb &&
      Bpb->Fat.Media != 0xfc &&
      Bpb->Fat.Media != 0xfd &&
      Bpb->Fat.Media != 0xfe &&
      Bpb->Fat.Media != 0xff &&
      //
      // FujitsuFMR
      //
      Bpb->Fat.Media != 0x00 &&
      Bpb->Fat.Media != 0x01 &&
      Bpb->Fat.Media != 0xfa
      ) {
    IsFat = FALSE;
  }

  if (FatType != FAT32 && Bpb->Fat.RootEntries == 0) {
    IsFat = FALSE;
  }
  //
  // If this is fat32, refuse to mount mirror-disabled volumes
  //
  if (FatType == FAT32 && (Bpb->Fat32.ExtendedFlags & 0x80)) {
    IsFat = FALSE;
  }

  if (FatType != FAT32 && IsFat) {
    RootSize        = Bpb->Fat.RootEntries * sizeof (FAT_DIRECTORY_ENTRY);
    RootLba         = Bpb->Fat.NoFats * Bpb->Fat.SectorsPerFat + Bpb->Fat.ReservedSectors;
    FirstClusterLba = RootLba + (RootSize / Bpb->Fat.SectorSize);
    MaxCluster      = (Bpb->Fat.Sectors - FirstClusterLba) / Bpb->Fat.SectorsPerCluster;
    FatType         = (MaxCluster + 1) < 4087 ? FAT12 : FAT16;
  }

  return IsFat ? FatType : FatUndefined;
}

VOID
DblkDumpBpb (
  IN  FAT_VOLUME_TYPE     FatType,
  IN  EFI_FAT_BOOT_SECTOR *Bpb,
  IN  UINTN               BlockSize
  )
{
  UINT32  Sectors;

  Sectors             = (Bpb->Fat.Sectors == 0) ? Bpb->Fat.LargeSectors : Bpb->Fat.Sectors;

  PrintToken (STRING_TOKEN (STR_DEBUG_FAT_BPB), HiiDblkHandle, FatNumber[FatType]);
  if (FatType == FAT32) {
    PrintToken (
      STRING_TOKEN (STR_DEBUG_FATLABEL),
      HiiDblkHandle,
      sizeof (Bpb->Fat32.FatLabel),
      Bpb->Fat32.FatLabel,
      sizeof (Bpb->Fat32.SystemId),
      Bpb->Fat32.SystemId,
      sizeof (Bpb->Fat32.OemId),
      Bpb->Fat32.OemId
      );
  } else {
    PrintToken (
      STRING_TOKEN (STR_DEBUG_FATLABEL_SYSTEMID),
      HiiDblkHandle,
      sizeof (Bpb->Fat.FatLabel),
      Bpb->Fat.FatLabel,
      sizeof (Bpb->Fat.SystemId),
      Bpb->Fat.SystemId,
      sizeof (Bpb->Fat.OemId),
      Bpb->Fat.OemId
      );
  }

  PrintToken (STRING_TOKEN (STR_DEBUG_SECTORSIZE), HiiDblkHandle, Bpb->Fat.SectorSize, Bpb->Fat.SectorsPerCluster);
  PrintToken (
    STRING_TOKEN (STR_DEBUG_RES_SECTORS),
    HiiDblkHandle,
    Bpb->Fat.ReservedSectors,
    Bpb->Fat.NoFats,
    Bpb->Fat.RootEntries,
    Bpb->Fat.Media
    );
  if (FatType == FAT32) {
    PrintToken (STRING_TOKEN (STR_DEBUG_SECTORS), HiiDblkHandle, Sectors, Bpb->Fat32.LargeSectorsPerFat);
  } else {
    PrintToken (STRING_TOKEN (STR_DEBUG_SECTORS), HiiDblkHandle, Sectors, Bpb->Fat.SectorsPerFat);
  }

  PrintToken (STRING_TOKEN (STR_DEBUG_SECTORSPERTRACK), HiiDblkHandle, Bpb->Fat.SectorsPerTrack, Bpb->Fat.Heads);

}
