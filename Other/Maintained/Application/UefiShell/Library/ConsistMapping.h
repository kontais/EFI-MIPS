/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  ConsistMapping.h

Abstract:

  Infomation about the library function support consist mapping.
 
Revision History

--*/
#ifndef _CONSIST_MAPPING_H
#define _CONSIST_MAPPING_H

typedef enum {
  MTDTypeUnknown,
  MTDTypeFloppy,
  MTDTypeHardDisk,
  MTDTypeCDRom,
  MTDTypeEnd
} MTD_TYPE;

typedef struct {
  UINTN       HI;
  MTD_TYPE    MTD;
  POOL_PRINT  CSD;
  BOOLEAN     Digital;
} DEVICE_CONSIST_MAPPING_INFO;

typedef struct {
  MTD_TYPE  MTDType;
  CHAR16    *Name;
} MTD_NAME;

typedef struct {
  UINT8 Type;
  UINT8 SubType;
  VOID (*SerialFun) (EFI_DEVICE_PATH_PROTOCOL *, DEVICE_CONSIST_MAPPING_INFO *);
  INTN (*CompareFun) (EFI_DEVICE_PATH_PROTOCOL *, EFI_DEVICE_PATH_PROTOCOL *);
}
DEV_PATH_CONSIST_MAPPING_TABLE;

EFI_STATUS
ConsistMappingCreateHIDevicePathTable (
  EFI_DEVICE_PATH_PROTOCOL           ***HIDevicePathTable
  );

EFI_STATUS
ConsistMappingFreeHIDevicePathTable (
  EFI_DEVICE_PATH_PROTOCOL **HIDevicePathTable
  );

CHAR16  *
ConsistMappingGenMappingName (
  EFI_DEVICE_PATH_PROTOCOL    *Devicepath,
  EFI_DEVICE_PATH_PROTOCOL    **HIDevicePathTable
  );

BOOLEAN
DevicePathIsChildDevice (
  IN  EFI_DEVICE_PATH_PROTOCOL *ParentDevicePath,
  IN  EFI_DEVICE_PATH_PROTOCOL *ChildDevicePath
  );

INTN
DevicePathConsistMappingCompare (
  IN  VOID *Buffer1,
  IN  VOID *Buffer2
  );

#endif