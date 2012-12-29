/*
 * Copyright (c) 1999, 2000
 * Intel Corporation.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 
 * 3. All advertising materials mentioning features or use of this software must
 *    display the following acknowledgement:
 * 
 *    This product includes software developed by Intel Corporation and its
 *    contributors.
 * 
 * 4. Neither the name of Intel Corporation or its contributors may be used to
 *    endorse or promote products derived from this software without specific
 *    prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY INTEL CORPORATION AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL INTEL CORPORATION OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */
#include <efi.h>
#include <efilib.h>
#include "./ramdisk.h"

#define DEFAULT_DISK_SIZE	3		/* in MBs */
#define MIN(x, y)	(x < y ? x : y)
#define MAX(x, y)	(x > y ? x : y)

UINT32		GetDiskSize( EFI_HANDLE ImageHandle );

/* Embedded version string for VERS utility */
static char v[] = "version_number=1.00 ";

/* EFI device path definition */
static RAM_DISK_DEVICE_PATH RamDiskDevicePath =
{
	MESSAGING_DEVICE_PATH,
	MSG_VENDOR_DP,
	sizeof(RAM_DISK_DEVICE_PATH) - END_DEVICE_PATH_LENGTH,
	0,
	// {06ED4DD0-FF78-11d3-BDC4-00A0C94053D1}
	0x6ed4dd0, 0xff78, 0x11d3, 0xbd, 0xc4, 0x0, 0xa0, 0xc9, 0x40, 0x53, 0xd1,
	0,0,0,0,0,0,0,0,	// ID assigned below
	END_DEVICE_PATH_TYPE,
	END_ENTIRE_DEVICE_PATH_SUBTYPE,
	END_DEVICE_PATH_LENGTH
};

/* Lookup table of total sectors vs. cluster size.
 * Ramdisk sizes between 0x20D0 (4.1MB) and 0x100000 (512MB) sectors are valid FAT16 drive sizes.
 */
/* #define MIN_DISK_SIZE	5 */
#define MIN_DISK_SIZE	1
#define	MAX_DISK_SIZE	512
static FAT16TABLE fat16tbl[] =
{
	/* {0x000020D0, 0}, */
	{0x00000800, 1},	/* 800 sectors * 1 sec/cluster * 512 bytes = 1 M */
	{0x00001000, 1},	/* 1000 sectors * 1 sec/cluster * 512 bytes = 2 M */
	{0x00001800, 1},	/* 1800 sectors * 1 sec/cluster * 512 bytes = 3 M */
	{0x00007FA8, 2},
	{0x00040000, 4},
	{0x00080000, 8},
	{0x00100000,16},
	{0xFFFFFFFF, 0}
};

/* RAM pseudo-boot sector.  No code.
 * Needs BS_Sig, BPB_SecPerClus, BPB_TotSec32, g_bs.BPB_TotSec16,
 * and BPB_FATSz16 filled out properly by FormatRamdisk().
 */
static BOOTSEC g_bs =
{
	/* BS_jmpBoot     */ {0xeb,0x0,0x90},
	/* BS_OEMName     */ {'E','F','I','R','D','I','S','K'},
	/* BPB_BytsPerSec */ 512,
	/* BPB_SecPerClus */ 0,
	/* BPB_RsvdSecCnt */ 1,
	/* BPB_NumFATs    */ 2,
	/* BPB_RootEntCnt */ 512,
	/* BPB_TotSec16   */ 0,
	/* BPB_Media      */ 0xF8,
	/* BPB_FATSz16    */ 0,
	/* BPB_SecPerTrk  */ 0,
	/* BPB_NumHeads   */ 0,
	/* BPB_HiddSec    */ 0,
	/* BPB_TotSec32   */ 0,
	/* BS_DrvNum      */ 0,
	/* BS_Reserved1   */ 0,
	/* BS_BootSig     */ 0x29,
	/* BS_VolID       */ 0,
	/* BS_VolLab      */ {'N','O',' ','N','A','M','E',' ',' ',' '},
	/* BS_FilSysType  */ {'F','A','T','1','6',' ',' ',' '}
};


/* Helper function to compute cluster size
 * vs. total sectors on drive.
 */
STATIC UINT8 size2spc(UINT32 ts)
{
	int i = 0;
	
	while(fat16tbl[i].size != 0xFFFFFFFF)
	{
		if(ts <= fat16tbl[i].size)
			return fat16tbl[i].spc;
		++i;
	}
	
	return 0;
}

EFI_SYSTEM_TABLE	BackupSystemTable;

/*
 * Entry point for RamDisk driver.
 */
EFI_DRIVER_ENTRY_POINT(InitializeRamDiskDriver)

EFI_STATUS InitializeRamDiskDriver(
	IN EFI_HANDLE       ImageHandle,
	IN EFI_SYSTEM_TABLE *SystemTable)
{
	EFI_STATUS           Status;
	RAM_DISK_DEV         *RamDiskDev;
	UINT32               RamDiskSize;
	UINT32               NumPages;
	UINT32               BlockSize;
	UINT64               DiskId;

	/*
	 * Make a copy of the system table to workaround load command bug
	 */
	CopyMem(&BackupSystemTable,SystemTable,sizeof(BackupSystemTable));

	/*
	 * Initialize EFI library
	 */
	InitializeLib(ImageHandle,&BackupSystemTable);

	/* IA64 compiler is removing version string (unused?) so I use it */
	v[0] = 'v';

	/*
	 *  Set the disk size
	 */
	RamDiskSize = GetDiskSize(ImageHandle);
	BlockSize   = 512;

	/* Allocate storage for ramdisk device info on the heap.
	 */
	RamDiskDev = AllocateZeroPool(sizeof(RAM_DISK_DEV));
	if(RamDiskDev == NULL)
		return EFI_OUT_OF_RESOURCES;

	/*
	 * Compute the number of 4KB pages needed by the ramdisk and allocate the memory.
	 */
	NumPages = RamDiskSize / EFI_PAGE_SIZE;
	if(NumPages % RamDiskSize)
		NumPages++;

	Status = BS->AllocatePages(AllocateAnyPages,EfiBootServicesData,NumPages,&RamDiskDev->Start);
	if(EFI_ERROR(Status)) {
		FreePool(RamDiskDev);
		return Status;
	}

	/*
	 * Initialize the ramdisk's device info.
	 */
	(void)BS->GetNextMonotonicCount(&DiskId);
	CopyMem(&RamDiskDevicePath.DiskId, &DiskId, sizeof(DiskId));

	RamDiskDev->Signature            = PBLOCK_DEVICE_SIGNATURE;
	RamDiskDev->BlkIo.Revision       = EFI_BLOCK_IO_INTERFACE_REVISION;
	RamDiskDev->BlkIo.Media          = &RamDiskDev->Media;
	RamDiskDev->Media.RemovableMedia = FALSE;
	RamDiskDev->Media.MediaPresent   = TRUE;
	
	RamDiskDev->Media.LastBlock        = RamDiskSize/BlockSize - 1;
	RamDiskDev->Media.BlockSize        = BlockSize;
	RamDiskDev->Media.LogicalPartition = TRUE;
	RamDiskDev->Media.ReadOnly         = FALSE;
	RamDiskDev->Media.WriteCaching     = TRUE;
	
	RamDiskDev->BlkIo.ReadBlocks  = RamDiskReadBlocks;
	RamDiskDev->BlkIo.WriteBlocks = RamDiskWriteBlocks;
	RamDiskDev->BlkIo.FlushBlocks = RamDiskFlushBlocks;
	
	RamDiskDev->DevicePath = DuplicateDevicePath((EFI_DEVICE_PATH*)&RamDiskDevicePath);
	
	/*
	 * Build a FAT16 file system on the ramdisk.
	 */
	FormatRamdisk((VOID*)RamDiskDev->Start,RamDiskSize);
	
	/*
	 * Install the device.
	 */
	Status = LibInstallProtocolInterfaces(
		&ImageHandle,
		&BlockIoProtocol,
		&RamDiskDev->BlkIo,
		&DevicePathProtocol,
		RamDiskDev->DevicePath,
		NULL);
	
	return Status;
}

UINT32
GetDiskSize( EFI_HANDLE ImageHandle )
{
	EFI_STATUS			Status;
	EFI_LOADED_IMAGE	*Image;
	UINT32				DiskSize = DEFAULT_DISK_SIZE;

	/*
	 * Check load options to see if they want to specify disk size in MBs
	 */
	Status = BS->HandleProtocol(ImageHandle, &LoadedImageProtocol, (void**)&Image);
	if (!EFI_ERROR(Status)) {
		if (Image->LoadOptions && Image->LoadOptionsSize) {
#define MAX_ARG_SIZE		32
			CHAR16	Size[ MAX_ARG_SIZE ];
			CHAR16	*CmdLine = Image->LoadOptions;
			INT32	CmdLen   = (INT32)Image->LoadOptionsSize;

			/*
			 * Get past program name
			 */
			while( CmdLen > 0 && *CmdLine != L' ' ) {
				CmdLen -= sizeof(CHAR16);
				CmdLine++;
			}

			if ( CmdLen > 0 ) {
				/*
				 * Make sure we're null terminated
				 */
				CopyMem( Size, CmdLine, MIN(CmdLen, sizeof(Size)));
				Size[MAX_ARG_SIZE - 1] = 0;

				/*
				 *  Atoi() will skip any leading white space
				 */
				DiskSize = (UINT32)Atoi(Size);
				if (DiskSize == 0)
					DiskSize = DEFAULT_DISK_SIZE;
				DiskSize = MAX(DiskSize, MIN_DISK_SIZE);
				DiskSize = MIN(DiskSize, MAX_DISK_SIZE);
			}
		}
	}

	return (DiskSize * 1024 * 1024);
}

/* Given a block of memory representing a ramdisk, build a pseudo-boot sector
 * and initialize the drive.
 *
 * Assumes the global boot sector structure g_bs has been filled out with the
 * static information the boot sector requires.  Also assumes the ramdisk size
 * is between 4.1MB and 512MB as appropriate for FAT16 file system.
 */
STATIC VOID FormatRamdisk(
	IN VOID*  pStart,
	IN UINT32 Size)
{
	UINT32 TotalSectors,RootDirSectors,FatSz,tmp1,tmp2;
	UINT8 *Fat1,*Fat2;

	/* The boot signature needs to be filled out */
	g_bs.BS_Sig = 0xAA55;

	/* Compute the total sectors and appropriate cluster size */
	TotalSectors = Size / g_bs.BPB_BytsPerSec;
	g_bs.BPB_SecPerClus = size2spc(TotalSectors);
	ASSERT(g_bs.BPB_SecPerClus != 0);

	/* Compute how many root directory sectors are needed */
	RootDirSectors = (g_bs.BPB_RootEntCnt * 32 + g_bs.BPB_BytsPerSec - 1) / g_bs.BPB_BytsPerSec;

	/* Compute how many sectors are required per FAT */
	tmp1 = TotalSectors - (g_bs.BPB_RsvdSecCnt + RootDirSectors);
	tmp2 = 256 * g_bs.BPB_SecPerClus + g_bs.BPB_NumFATs;
	FatSz = (tmp1 + tmp2 - 1) / tmp2;
	ASSERT(FatSz <= 0xFFFF);

	/* Store the total sectors and fat size values */
	if(TotalSectors > 0xFFFF)
		g_bs.BPB_TotSec32 = TotalSectors;
	else
		g_bs.BPB_TotSec16 = (UINT16)TotalSectors;

	g_bs.BPB_FATSz16 = (UINT16)FatSz;

	/* The FAT table and root directory need to be all zeroes.
	 * We'll zero the whole drive.
	 */
	ZeroMem(pStart,Size);
	
	/* Write the completed boot sector to the ramdisk */
	CopyMem(pStart,&g_bs,512);

	/* Compute the starting offsets of the two FATs */
	Fat1 = (UINT8*)pStart + g_bs.BPB_RsvdSecCnt * 512;
	Fat2 = (UINT8*)pStart + (g_bs.BPB_RsvdSecCnt + FatSz) * 512;

	/* Initialize FAT1 */
	Fat1[0] = g_bs.BPB_Media;
	Fat1[1] = 0xFF;
	Fat1[2] = 0xFF;
	Fat1[3] = 0xFF;

	/* Initialize FAT2 */
	Fat2[0] = g_bs.BPB_Media;
	Fat2[1] = 0xFF;
	Fat2[2] = 0xFF;
	Fat2[3] = 0xFF;
}

/* Implementation of block I/O read */
STATIC EFI_STATUS RamDiskReadBlocks(
	IN EFI_BLOCK_IO *This,
	IN UINT32       MediaId,
	IN EFI_LBA      LBA,
	IN UINTN        BufferSize,
	OUT VOID        *Buffer)
{
	EFI_BLOCK_IO_MEDIA   *Media;
	RAM_DISK_DEV         *RamDiskDev;
	EFI_PHYSICAL_ADDRESS RamDiskLBA;

	Media = This->Media;

	if(BufferSize % Media->BlockSize != 0)
		return EFI_BAD_BUFFER_SIZE;

	if(LBA > Media->LastBlock)
		return EFI_DEVICE_ERROR;

	if(LBA + BufferSize / Media->BlockSize - 1 > Media->LastBlock)
		return EFI_DEVICE_ERROR;

	RamDiskDev = RAM_DISK_FROM_THIS(This);
	RamDiskLBA = RamDiskDev->Start + MultU64x32(LBA,Media->BlockSize);
	CopyMem(Buffer,(VOID*)RamDiskLBA,BufferSize);

	return EFI_SUCCESS;
}


/* Implementation of block I/O write */
STATIC EFI_STATUS RamDiskWriteBlocks(
	IN EFI_BLOCK_IO *This,
	IN UINT32       MediaId,
	IN EFI_LBA      LBA,
	IN UINTN        BufferSize,
	IN VOID         *Buffer)
{
	EFI_BLOCK_IO_MEDIA   *Media;
	RAM_DISK_DEV         *RamDiskDev;
	EFI_PHYSICAL_ADDRESS RamDiskLBA;

	Media = This->Media;
	if(Media->ReadOnly)
		return EFI_WRITE_PROTECTED;

	if(BufferSize % Media->BlockSize != 0)
		return EFI_BAD_BUFFER_SIZE;

	if(LBA > Media->LastBlock)
		return EFI_DEVICE_ERROR;

	if(LBA + BufferSize / Media->BlockSize - 1 > Media->LastBlock)
		return EFI_DEVICE_ERROR;

	RamDiskDev = RAM_DISK_FROM_THIS(This);
	RamDiskLBA = RamDiskDev->Start + MultU64x32(LBA,Media->BlockSize);
	CopyMem((VOID*)RamDiskLBA,Buffer,BufferSize);

	return EFI_SUCCESS;
}

/* Implementation of block I/O flush */
STATIC EFI_STATUS RamDiskFlushBlocks(
	IN EFI_BLOCK_IO *This)
{
	return EFI_SUCCESS;
}
