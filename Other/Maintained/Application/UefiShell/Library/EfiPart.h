/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  EfiPart.h
    
Abstract:   
  Info about disk partitions and Master Boot Records

Revision History

--*/
#ifndef _EFI_PART_H
#define _EFI_PART_H

#define EFI_PARTITION 0xef
#define MBR_SIZE      512

#pragma pack(1)

typedef struct {
  UINT8 BootIndicator;
  UINT8 StartHead;
  UINT8 StartSector;
  UINT8 StartTrack;
  UINT8 OSIndicator;
  UINT8 EndHead;
  UINT8 EndSector;
  UINT8 EndTrack;
  UINT8 StartingLBA[4];
  UINT8 SizeInLBA[4];
} MBR_PARTITION_RECORD;

#define EXTRACT_UINT32(D)   (UINT32) (D[0] | (D[1] << 8) | (D[2] << 16) | (D[3] << 24))

#define MBR_SIGNATURE       0xaa55
#define MIN_MBR_DEVICE_SIZE 0x80000
#define MBR_ERRATA_PAD      0x40000 // 128 MB
#define MAX_MBR_PARTITIONS  4
typedef struct {
  UINT8                 BootStrapCode[440];
  UINT8                 UniqueMbrSignature[4];
  UINT8                 Unknown[2];
  MBR_PARTITION_RECORD  Partition[MAX_MBR_PARTITIONS];
  UINT16                Signature;
} MASTER_BOOT_RECORD;
#pragma pack()

#endif
