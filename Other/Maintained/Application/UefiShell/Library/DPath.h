/*++

Copyright (c) 2005 - 2007, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

    DPath.c

Abstract:
    Infomation about device path functions



Revision History

--*/
#ifndef _D_PATH_H
#define _D_PATH_H

#define NextStrA(a) ((UINT8 *) (((UINT8 *) (a)) + strlena (a) + 1))

#include EFI_GUID_DEFINITION (PcAnsi)

extern EFI_GUID mEfiDevicePathMessagingUartFlowControlGuid;

typedef struct {
  EFI_DEVICE_PATH_PROTOCOL  Header;
  UINT32                    HID;
  UINT32                    UID;
  UINT32                    CID;
  CHAR8                     HidUidCidStr[3];
} ACPI_EXTENDED_HID_DEVICE_PATH_WITH_STR;

typedef struct {
  EFI_DEVICE_PATH_PROTOCOL  Header;
  EFI_GUID                  Guid;
  UINT8                     VendorDefinedData[1];
} VENDOR_DEVICE_PATH_WITH_DATA;

#if (EFI_SPECIFICATION_VERSION >= 0x00020000)

extern EFI_GUID mEfiDevicePathMessagingSASGuid;

typedef struct {
  EFI_DEVICE_PATH_PROTOCOL  Header;
  UINT16                    NetworkProtocol;
  UINT16                    LoginOption;
  UINT16                    Reserved;
  UINT16                    TargetPortalGroupTag;
  UINT64                    Lun;
  CHAR16                    iSCSITargetName[1];
} ISCSI_DEVICE_PATH_WITH_NAME;

#endif

EFI_DEVICE_PATH_PROTOCOL  *
DevicePathFromHandle (
  IN EFI_HANDLE           Handle
  );

EFI_DEVICE_PATH_PROTOCOL  *
DevicePathInstance (
  IN OUT EFI_DEVICE_PATH_PROTOCOL  **DevicePath,
  OUT UINTN                        *Size
  );

EFI_DEVICE_PATH_PROTOCOL  *
AppendDevicePath (
  IN EFI_DEVICE_PATH_PROTOCOL      *Src1,
  IN EFI_DEVICE_PATH_PROTOCOL      *Src2
  );

UINTN
DevicePathSize (
  IN EFI_DEVICE_PATH_PROTOCOL      *DevPath
  );

EFI_DEVICE_PATH_PROTOCOL  *
UnpackDevicePath (
  IN EFI_DEVICE_PATH_PROTOCOL      *DevPath
  );

BOOLEAN
LibMatchDevicePaths (
  IN  EFI_DEVICE_PATH_PROTOCOL      *Multi,
  IN  EFI_DEVICE_PATH_PROTOCOL      *Single
  );

EFI_DEVICE_PATH_PROTOCOL          *
LibDuplicateDevicePathInstance (
  IN EFI_DEVICE_PATH_PROTOCOL       *DevPath
  );

UINTN
DevicePathInstanceCount (
  IN EFI_DEVICE_PATH_PROTOCOL       *DevicePath
  );

EFI_DEVICE_PATH_PROTOCOL          *
AppendDevicePathNode (
  IN EFI_DEVICE_PATH_PROTOCOL       *Src1,
  IN EFI_DEVICE_PATH_PROTOCOL       *Src2
  );

EFI_DEVICE_PATH_PROTOCOL          *
AppendDevicePathInstance (
  IN EFI_DEVICE_PATH_PROTOCOL       *Src,
  IN EFI_DEVICE_PATH_PROTOCOL       *Instance
  );

EFI_DEVICE_PATH_PROTOCOL          *
FileDevicePath (
  IN EFI_HANDLE                               Device  OPTIONAL,
  IN CHAR16                                   *FileName
  );

EFI_DEVICE_PATH_PROTOCOL          *
DuplicateDevicePath (
  IN EFI_DEVICE_PATH_PROTOCOL       *DevPath
  );

EFI_STATUS
LibDevicePathToInterface (
  IN EFI_GUID                       *Protocol,
  IN EFI_DEVICE_PATH_PROTOCOL       *FilePath,
  OUT VOID                          **Interface
  );

CHAR16                                *
LibDevicePathToStr (
  IN EFI_DEVICE_PATH_PROTOCOL         *DevPath
  );

INTN
DevicePathCompare (
  IN  EFI_DEVICE_PATH_PROTOCOL *DevicePath1,
  IN  EFI_DEVICE_PATH_PROTOCOL *DevicePath2
  );

VOID
EFIAPI
InitializeFwVolDevicepathNode (
  IN  MEDIA_FW_VOL_FILEPATH_DEVICE_PATH     *FvDevicePathNode,
  IN EFI_GUID                               *NameGuid
  );

EFI_GUID *
EFIAPI
GetNameGuidFromFwVolDevicePathNode (
  IN  MEDIA_FW_VOL_FILEPATH_DEVICE_PATH   *FvDevicePathNode
  );

#endif
