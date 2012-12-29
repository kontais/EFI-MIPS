/*++

Copyright (c) 2005 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  ConsistMapping.c
  
Abstract:

  Library function support consist mapping.

Revision History

--*/

#include "EfiShellLib.h"

MTD_NAME  mMTDName[] = {
  {
    MTDTypeUnknown,
    L"f"
  },
  {
    MTDTypeFloppy,
    L"fp"
  },
  {
    MTDTypeHardDisk,
    L"hd"
  },
  {
    MTDTypeCDRom,
    L"cd"
  },
  {
    MTDTypeEnd,
    NULL
  }
};

VOID
AppendCSDNum2 (
  IN OUT POOL_PRINT       *Str,
  IN UINT64               Num
  )
{
  UINT64  Result;
  UINTN   Rem;
  
  ASSERT(Str != NULL);
  
  Result = DivU64x32 (Num, 25, &Rem);
  if (Result > 0) {
    AppendCSDNum2 (Str, Result);
  }

  CatPrint (Str, L"%c", Rem + 'a');
}

VOID
AppendCSDNum (
  DEVICE_CONSIST_MAPPING_INFO            *MappingItem,
  UINT64                                 Num
  )
{
  ASSERT(MappingItem != NULL);

  if (MappingItem->Digital) {
    CatPrint (&MappingItem->CSD, L"%ld", Num);
  } else {
    AppendCSDNum2 (&MappingItem->CSD, Num);
  }

  MappingItem->Digital = (BOOLEAN)!(MappingItem->Digital);
}

VOID
AppendCSDStr (
  DEVICE_CONSIST_MAPPING_INFO            *MappingItem,
  CHAR16                                 *Str
  )
{
  CHAR16  *Index;
  
  ASSERT(Str != NULL);
  ASSERT(MappingItem != NULL);

  if (MappingItem->Digital) {
    //
    // To aVOID mult-meaning, the mapping is:
    //  0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
    //  0  16 2  3  4  5  6  7  8  9  10 11 12 13 14 15
    //
    for (Index = Str; *Index != 0; Index++) {
      switch (*Index) {
      case '0':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        CatPrint (&MappingItem->CSD, L"%c", *Index);
        break;

      case '1':
        CatPrint (&MappingItem->CSD, L"16");
        break;

      case 'a':
      case 'b':
      case 'c':
      case 'd':
      case 'e':
      case 'f':
        CatPrint (&MappingItem->CSD, L"1%c", *Index - 'a' + '0');
        break;

      case 'A':
      case 'B':
      case 'C':
      case 'D':
      case 'E':
      case 'F':
        CatPrint (&MappingItem->CSD, L"1%c", *Index - 'A' + '0');
        break;
      }
    }
  } else {
    for (Index = Str; *Index != 0; Index++) {
      //
      //  The mapping is:
      //  0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
      //  a  b  c  d  e  f  g  h  i  j  k  l  m  n  o  p
      //
      if (*Index >= '0' && *Index <= '9') {
        CatPrint (&MappingItem->CSD, L"%c", *Index - '0' + 'a');
      } else if (*Index >= 'a' && *Index <= 'f') {
        CatPrint (&MappingItem->CSD, L"%c", *Index - 'a' + 'k');
      } else if (*Index >= 'A' && *Index <= 'F') {
        CatPrint (&MappingItem->CSD, L"%c", *Index - 'A' + 'k');
      }
    }
  }

  MappingItem->Digital = (BOOLEAN)!(MappingItem->Digital);
}

VOID
AppendCSDGuid (
  DEVICE_CONSIST_MAPPING_INFO            *MappingItem,
  EFI_GUID                               *Guid
  )
{
  CHAR16  Buffer[64];
  ASSERT(Guid != NULL);
  ASSERT(MappingItem != NULL);

  SPrint (
    Buffer,
    0,
    L"%08x%04x%04x%02x%02x%02x%02x%02x%02x%02x%02x",
    (UINTN) Guid->Data1,
    (UINTN) Guid->Data2,
    (UINTN) Guid->Data3,
    (UINTN) Guid->Data4[0],
    (UINTN) Guid->Data4[1],
    (UINTN) Guid->Data4[2],
    (UINTN) Guid->Data4[3],
    (UINTN) Guid->Data4[4],
    (UINTN) Guid->Data4[5],
    (UINTN) Guid->Data4[6],
    (UINTN) Guid->Data4[7]
    );
  StrLwr (Buffer);
  AppendCSDStr (MappingItem, Buffer);
}

INTN
_DevPathCompareAcpi (
  IN EFI_DEVICE_PATH_PROTOCOL       *DevicePath1,
  IN EFI_DEVICE_PATH_PROTOCOL       *DevicePath2
  )
{
  ACPI_HID_DEVICE_PATH  *Acpi1;
  ACPI_HID_DEVICE_PATH  *Acpi2;

  ASSERT(DevicePath1 != NULL);
  ASSERT(DevicePath2 != NULL);

  Acpi1 = (ACPI_HID_DEVICE_PATH *) DevicePath1;
  Acpi2 = (ACPI_HID_DEVICE_PATH *) DevicePath2;
  if (Acpi1->HID > Acpi2->HID || (Acpi1->HID == Acpi2->HID && Acpi1->UID > Acpi2->UID)) {
    return 1;
  }

  if (Acpi1->HID == Acpi2->HID && Acpi1->UID == Acpi2->UID) {
    return 0;
  }

  return -1;
}

INTN
_DevPathComparePci (
  IN EFI_DEVICE_PATH_PROTOCOL       *DevicePath1,
  IN EFI_DEVICE_PATH_PROTOCOL       *DevicePath2
  )
{
  PCI_DEVICE_PATH *Pci1;
  PCI_DEVICE_PATH *Pci2;

  ASSERT(DevicePath1 != NULL);
  ASSERT(DevicePath2 != NULL);

  Pci1  = (PCI_DEVICE_PATH *) DevicePath1;
  Pci2  = (PCI_DEVICE_PATH *) DevicePath2;
  if (Pci1->Device > Pci2->Device || (Pci1->Device == Pci2->Device && Pci1->Function > Pci2->Function)) {
    return 1;
  }

  if (Pci1->Device == Pci2->Device && Pci1->Function == Pci2->Function) {
    return 0;
  }

  return -1;
}

INTN
_DevPathCompareDefault (
  IN EFI_DEVICE_PATH_PROTOCOL       *DevicePath1,
  IN EFI_DEVICE_PATH_PROTOCOL       *DevicePath2
  )
{
  UINTN DevPathSize1;
  UINTN DevPathSize2;

  ASSERT(DevicePath1 != NULL);
  ASSERT(DevicePath2 != NULL);

  DevPathSize1  = DevicePathNodeLength (DevicePath1);
  DevPathSize2  = DevicePathNodeLength (DevicePath2);
  if (DevPathSize1 > DevPathSize2) {
    return 1;
  } else if (DevPathSize1 < DevPathSize2) {
    return -1;
  } else {
    return CompareMem (DevicePath1, DevicePath2, DevPathSize1);
  }
}

VOID
_DevPathSerialHardDrive (
  IN EFI_DEVICE_PATH_PROTOCOL     *DevicePathNode,
  IN DEVICE_CONSIST_MAPPING_INFO  *MappingItem
  )
{
  HARDDRIVE_DEVICE_PATH *Hd;
  
  ASSERT(DevicePathNode != NULL);
  ASSERT(MappingItem != NULL);

  Hd = (HARDDRIVE_DEVICE_PATH *) DevicePathNode;
  if (MappingItem->MTD == MTDTypeUnknown) {
    MappingItem->MTD = MTDTypeHardDisk;
  }

  AppendCSDNum (MappingItem, Hd->PartitionNumber);
}

VOID
_DevPathSerialAtapi (
  IN EFI_DEVICE_PATH_PROTOCOL     *DevicePathNode,
  IN DEVICE_CONSIST_MAPPING_INFO  *MappingItem
  )
{
  ATAPI_DEVICE_PATH *Atapi;

  ASSERT(DevicePathNode != NULL);
  ASSERT(MappingItem != NULL);

  Atapi = (ATAPI_DEVICE_PATH *) DevicePathNode;
  AppendCSDNum (MappingItem, (Atapi->PrimarySecondary * 2 + Atapi->SlaveMaster));
}

VOID
_DevPathSerialCDROM (
  IN EFI_DEVICE_PATH_PROTOCOL     *DevicePathNode,
  IN DEVICE_CONSIST_MAPPING_INFO  *MappingItem
  )
{
  CDROM_DEVICE_PATH *Cd;

  ASSERT(DevicePathNode != NULL);
  ASSERT(MappingItem != NULL);

  Cd                = (CDROM_DEVICE_PATH *) DevicePathNode;
  MappingItem->MTD  = MTDTypeCDRom;
  AppendCSDNum (MappingItem, Cd->BootEntry);
}

VOID
_DevPathSerialFibre (
  IN EFI_DEVICE_PATH_PROTOCOL     *DevicePathNode,
  IN DEVICE_CONSIST_MAPPING_INFO  *MappingItem
  )
{
  FIBRECHANNEL_DEVICE_PATH  *Fibre;

  ASSERT(DevicePathNode != NULL);
  ASSERT(MappingItem != NULL);

  Fibre = (FIBRECHANNEL_DEVICE_PATH *) DevicePathNode;
  AppendCSDNum (MappingItem, Fibre->WWN);
  AppendCSDNum (MappingItem, Fibre->Lun);
}

VOID
_DevPathSerialUart (
  IN EFI_DEVICE_PATH_PROTOCOL     *DevicePathNode,
  IN DEVICE_CONSIST_MAPPING_INFO  *MappingItem
  )
{
  UART_DEVICE_PATH  *Uart;

  ASSERT(DevicePathNode != NULL);
  ASSERT(MappingItem != NULL);

  Uart = (UART_DEVICE_PATH *) DevicePathNode;
  AppendCSDNum (MappingItem, Uart->BaudRate);
  AppendCSDNum (MappingItem, Uart->DataBits);
  AppendCSDNum (MappingItem, Uart->Parity);
  AppendCSDNum (MappingItem, Uart->StopBits);
}

VOID
_DevPathSerialUsb (
  IN EFI_DEVICE_PATH_PROTOCOL     *DevicePathNode,
  IN DEVICE_CONSIST_MAPPING_INFO  *MappingItem
  )
{
  USB_DEVICE_PATH *Usb;

  ASSERT(DevicePathNode != NULL);
  ASSERT(MappingItem != NULL);

  Usb = (USB_DEVICE_PATH *) DevicePathNode;
  AppendCSDNum (MappingItem, Usb->ParentPortNumber);
  AppendCSDNum (MappingItem, Usb->InterfaceNumber);
}

VOID
_DevPathSerialVendor (
  IN EFI_DEVICE_PATH_PROTOCOL     *DevicePathNode,
  IN DEVICE_CONSIST_MAPPING_INFO  *MappingItem
  )
{
  VENDOR_DEVICE_PATH  *Vendor;

  ASSERT(DevicePathNode != NULL);
  ASSERT(MappingItem != NULL);

  Vendor = (VENDOR_DEVICE_PATH *) DevicePathNode;
  AppendCSDGuid (MappingItem, &Vendor->Guid);
}

VOID
_DevPathSerialI2O (
  IN EFI_DEVICE_PATH_PROTOCOL     *DevicePathNode,
  IN DEVICE_CONSIST_MAPPING_INFO  *MappingItem
  )
{
  I2O_DEVICE_PATH *I2O;

  ASSERT(DevicePathNode != NULL);
  ASSERT(MappingItem != NULL);

  I2O = (I2O_DEVICE_PATH *) DevicePathNode;
  AppendCSDNum (MappingItem, I2O->Tid);
}

VOID
_DevPathSerialMacAddr (
  IN EFI_DEVICE_PATH_PROTOCOL     *DevicePathNode,
  IN DEVICE_CONSIST_MAPPING_INFO  *MappingItem
  )
{
  MAC_ADDR_DEVICE_PATH  *MAC;
  UINTN                 HwAddressSize;
  UINTN                 Index;
  CHAR16                Buffer[64];
  CHAR16                *PBuffer;

  ASSERT(DevicePathNode != NULL);
  ASSERT(MappingItem != NULL);

  MAC           = (MAC_ADDR_DEVICE_PATH *) DevicePathNode;

  HwAddressSize = sizeof (EFI_MAC_ADDRESS);
  if (MAC->IfType == 0x01 || MAC->IfType == 0x00) {
    HwAddressSize = 6;
  }

  for (Index = 0, PBuffer = Buffer; Index < HwAddressSize; Index++, PBuffer += 2) {
    SPrint (PBuffer, 0, L"%02x", (UINTN) MAC->MacAddress.Addr[Index]);
  }

  AppendCSDStr (MappingItem, Buffer);
}

VOID
_DevPathSerialInfiniBand (
  IN EFI_DEVICE_PATH_PROTOCOL     *DevicePathNode,
  IN DEVICE_CONSIST_MAPPING_INFO  *MappingItem
  )
{
  INFINIBAND_DEVICE_PATH  *InfiniBand;
  UINTN                   Index;
  CHAR16                  Buffer[64];
  CHAR16                  *PBuffer;

  ASSERT(DevicePathNode != NULL);
  ASSERT(MappingItem != NULL);

  InfiniBand = (INFINIBAND_DEVICE_PATH *) DevicePathNode;
  for (Index = 0, PBuffer = Buffer; Index < 16; Index++, PBuffer += 2) {
    SPrint (PBuffer, 0, L"%02x", (UINTN) InfiniBand->PortGid[Index]);
  }

  AppendCSDStr (MappingItem, Buffer);
  AppendCSDNum (MappingItem, InfiniBand->ServiceId);
  AppendCSDNum (MappingItem, InfiniBand->TargetPortId);
  AppendCSDNum (MappingItem, InfiniBand->DeviceId);
}

VOID
_DevPathSerialIPv4 (
  IN EFI_DEVICE_PATH_PROTOCOL     *DevicePathNode,
  IN DEVICE_CONSIST_MAPPING_INFO  *MappingItem
  )
{
  IPv4_DEVICE_PATH  *IP;
  CHAR16            Buffer[10];

  ASSERT(DevicePathNode != NULL);
  ASSERT(MappingItem != NULL);

  IP = (IPv4_DEVICE_PATH *) DevicePathNode;
  SPrint (
    Buffer,
    0,
    L"%02x%02x%02x%02x",
    (UINTN) IP->LocalIpAddress.Addr[0],
    (UINTN) IP->LocalIpAddress.Addr[1],
    (UINTN) IP->LocalIpAddress.Addr[2],
    (UINTN) IP->LocalIpAddress.Addr[3]
    );
  AppendCSDStr (MappingItem, Buffer);
  AppendCSDNum (MappingItem, IP->LocalPort);
  SPrint (
    Buffer,
    0,
    L"%02x%02x%02x%02x",
    (UINTN) IP->RemoteIpAddress.Addr[0],
    (UINTN) IP->RemoteIpAddress.Addr[1],
    (UINTN) IP->RemoteIpAddress.Addr[2],
    (UINTN) IP->RemoteIpAddress.Addr[3]
    );
  AppendCSDStr (MappingItem, Buffer);
  AppendCSDNum (MappingItem, IP->RemotePort);
}

VOID
_DevPathSerialIPv6 (
  IN EFI_DEVICE_PATH_PROTOCOL     *DevicePathNode,
  IN DEVICE_CONSIST_MAPPING_INFO  *MappingItem
  )
{
  IPv6_DEVICE_PATH  *IP;
  UINTN             Index;
  CHAR16            Buffer[64];
  CHAR16            *PBuffer;

  ASSERT(DevicePathNode != NULL);
  ASSERT(MappingItem != NULL);

  IP = (IPv6_DEVICE_PATH *) DevicePathNode;
  for (Index = 0, PBuffer = Buffer; Index < 16; Index++, PBuffer += 2) {
    SPrint (PBuffer, 0, L"%02x", (UINTN) IP->LocalIpAddress.Addr[Index]);
  }

  AppendCSDStr (MappingItem, Buffer);
  AppendCSDNum (MappingItem, IP->LocalPort);
  for (Index = 0, PBuffer = Buffer; Index < 16; Index++, PBuffer += 2) {
    SPrint (PBuffer, 0, L"%02x", (UINTN) IP->RemoteIpAddress.Addr[Index]);
  }

  AppendCSDStr (MappingItem, Buffer);
  AppendCSDNum (MappingItem, IP->RemotePort);
}

VOID
_DevPathSerialScsi (
  IN EFI_DEVICE_PATH_PROTOCOL     *DevicePathNode,
  IN DEVICE_CONSIST_MAPPING_INFO  *MappingItem
  )
{
  SCSI_DEVICE_PATH  *Scsi;

  ASSERT(DevicePathNode != NULL);
  ASSERT(MappingItem != NULL);

  Scsi = (SCSI_DEVICE_PATH *) DevicePathNode;
  AppendCSDNum (MappingItem, Scsi->Pun);
  AppendCSDNum (MappingItem, Scsi->Lun);
}

VOID
_DevPathSerial1394 (
  IN EFI_DEVICE_PATH_PROTOCOL     *DevicePathNode,
  IN DEVICE_CONSIST_MAPPING_INFO  *MappingItem
  )
{
  F1394_DEVICE_PATH *F1394;
  CHAR16            Buffer[20];
  
  ASSERT(DevicePathNode != NULL);
  ASSERT(MappingItem != NULL);

  F1394 = (F1394_DEVICE_PATH *) DevicePathNode;
  SPrint (Buffer, 0, L"%lx", F1394->Guid);
  AppendCSDStr (MappingItem, Buffer);
}

VOID
_DevPathSerialAcpi (
  IN EFI_DEVICE_PATH_PROTOCOL     *DevicePathNode,
  IN DEVICE_CONSIST_MAPPING_INFO  *MappingItem
  )
{
  ACPI_HID_DEVICE_PATH  *Acpi;

  ASSERT(DevicePathNode != NULL);
  ASSERT(MappingItem != NULL);

  Acpi = (ACPI_HID_DEVICE_PATH *) DevicePathNode;
  if ((Acpi->HID & PNP_EISA_ID_MASK) == PNP_EISA_ID_CONST) {
    if (EISA_ID_TO_NUM (Acpi->HID) == 0x0604) {
      MappingItem->MTD = MTDTypeFloppy;
      AppendCSDNum (MappingItem, Acpi->UID);
    }
  }
}

VOID
_DevPathSerialDefault (
  IN EFI_DEVICE_PATH_PROTOCOL     *DevicePathNode,
  IN DEVICE_CONSIST_MAPPING_INFO  *MappingItem
  )
{
}

DEV_PATH_CONSIST_MAPPING_TABLE  DevPathConsistMappingTable[] = {
  HARDWARE_DEVICE_PATH,
  HW_PCI_DP,
  _DevPathSerialDefault,
  _DevPathComparePci,
  HARDWARE_DEVICE_PATH,
  HW_PCCARD_DP,
  _DevPathSerialDefault,
  _DevPathCompareDefault,
  HARDWARE_DEVICE_PATH,
  HW_MEMMAP_DP,
  _DevPathSerialDefault,
  _DevPathCompareDefault,
  HARDWARE_DEVICE_PATH,
  HW_VENDOR_DP,
  _DevPathSerialDefault,
  _DevPathCompareDefault,
  HARDWARE_DEVICE_PATH,
  HW_CONTROLLER_DP,
  _DevPathSerialDefault,
  _DevPathCompareDefault,
  ACPI_DEVICE_PATH,
  ACPI_DP,
  _DevPathSerialAcpi,
  _DevPathCompareAcpi,
  MESSAGING_DEVICE_PATH,
  MSG_ATAPI_DP,
  _DevPathSerialAtapi,
  _DevPathCompareDefault,
  MESSAGING_DEVICE_PATH,
  MSG_SCSI_DP,
  _DevPathSerialScsi,
  _DevPathCompareDefault,
  MESSAGING_DEVICE_PATH,
  MSG_FIBRECHANNEL_DP,
  _DevPathSerialFibre,
  _DevPathCompareDefault,
  MESSAGING_DEVICE_PATH,
  MSG_1394_DP,
  _DevPathSerial1394,
  _DevPathCompareDefault,
  MESSAGING_DEVICE_PATH,
  MSG_USB_DP,
  _DevPathSerialUsb,
  _DevPathCompareDefault,
  MESSAGING_DEVICE_PATH,
  MSG_USB_CLASS_DP,
  _DevPathSerialDefault,
  _DevPathCompareDefault,
  MESSAGING_DEVICE_PATH,
  MSG_I2O_DP,
  _DevPathSerialI2O,
  _DevPathCompareDefault,
  MESSAGING_DEVICE_PATH,
  MSG_MAC_ADDR_DP,
  _DevPathSerialMacAddr,
  _DevPathCompareDefault,
  MESSAGING_DEVICE_PATH,
  MSG_IPv4_DP,
  _DevPathSerialIPv4,
  _DevPathCompareDefault,
  MESSAGING_DEVICE_PATH,
  MSG_IPv6_DP,
  _DevPathSerialIPv6,
  _DevPathCompareDefault,
  MESSAGING_DEVICE_PATH,
  MSG_INFINIBAND_DP,
  _DevPathSerialInfiniBand,
  _DevPathCompareDefault,
  MESSAGING_DEVICE_PATH,
  MSG_UART_DP,
  _DevPathSerialUart,
  _DevPathCompareDefault,
  MESSAGING_DEVICE_PATH,
  MSG_VENDOR_DP,
  _DevPathSerialVendor,
  _DevPathCompareDefault,
  MEDIA_DEVICE_PATH,
  MEDIA_HARDDRIVE_DP,
  _DevPathSerialHardDrive,
  _DevPathCompareDefault,
  MEDIA_DEVICE_PATH,
  MEDIA_CDROM_DP,
  _DevPathSerialCDROM,
  _DevPathCompareDefault,
  MEDIA_DEVICE_PATH,
  MEDIA_VENDOR_DP,
  _DevPathSerialVendor,
  _DevPathCompareDefault,
  MEDIA_DEVICE_PATH,
  MEDIA_FILEPATH_DP,
  _DevPathSerialDefault,
  _DevPathCompareDefault,
  MEDIA_DEVICE_PATH,
  MEDIA_PROTOCOL_DP,
  _DevPathSerialDefault,
  _DevPathCompareDefault,
#if (EFI_SPECIFICATION_VERSION < 0x00020000)
  MEDIA_DEVICE_PATH,
  MEDIA_FV_FILEPATH_DP,
  _DevPathSerialDefault,
  _DevPathCompareDefault,
#endif
  BBS_DEVICE_PATH,
  BBS_BBS_DP,
  _DevPathSerialDefault,
  _DevPathCompareDefault,
  END_DEVICE_PATH_TYPE,
  END_INSTANCE_DEVICE_PATH_SUBTYPE,
  _DevPathSerialDefault,
  _DevPathCompareDefault,
  0,
  0,
  NULL,
  NULL
};

INTN
DevicePathConsistMappingCompare (
  IN  VOID             *Buffer1,
  IN  VOID             *Buffer2
  )
{
  UINTN                     Index;
  INTN                      CompareResult;
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath1 = *(EFI_DEVICE_PATH_PROTOCOL**)Buffer1;
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath2 = *(EFI_DEVICE_PATH_PROTOCOL**)Buffer2;
  INTN                      (*CompareFun)(EFI_DEVICE_PATH_PROTOCOL *, EFI_DEVICE_PATH_PROTOCOL *);

  if (DevicePath1 == NULL) {
    if (DevicePath2 == NULL) {
      return 0;
    }

    return -1;
  }

  if (DevicePath2 == NULL) {
    return 1;
  }

  while (!(IsDevicePathEnd (DevicePath1) || IsDevicePathEnd (DevicePath2))) {
    CompareResult = DevicePathType (DevicePath1) - DevicePathType (DevicePath2);
    if (CompareResult != 0) {
      return CompareResult;
    }

    CompareResult = DevicePathSubType (DevicePath1) - DevicePathSubType (DevicePath2);
    if (CompareResult != 0) {
      return CompareResult;
    }

    CompareFun = NULL;
    for (Index = 0; DevPathConsistMappingTable[Index].CompareFun; Index += 1) {
      if (DevicePathType (DevicePath1) == DevPathConsistMappingTable[Index].Type &&
          DevicePathSubType (DevicePath1) == DevPathConsistMappingTable[Index].SubType
          ) {
        CompareFun = DevPathConsistMappingTable[Index].CompareFun;
        break;
      }
    }
    //
    // If not found, use a default function
    //
    if (!CompareFun) {
      CompareFun = _DevPathCompareDefault;
    }

    CompareResult = CompareFun (DevicePath1, DevicePath2);
    if (CompareResult != 0) {
      return CompareResult;
    }

    DevicePath1 = (EFI_DEVICE_PATH_PROTOCOL *) NextDevicePathNode (DevicePath1);
    DevicePath2 = (EFI_DEVICE_PATH_PROTOCOL *) NextDevicePathNode (DevicePath2);
  }

  if (IsDevicePathEnd (DevicePath1)) {
    if (IsDevicePathEnd (DevicePath2)) {
      return 0;
    }

    return -1;
  }

  return 1;
}

EFI_STATUS
DevicePathConsistMappingSort (
  IN  EFI_DEVICE_PATH_PROTOCOL **DevicePathBuffer,
  IN  UINTN                    DevicePathNum
  )
{
  QSort (DevicePathBuffer, DevicePathNum, sizeof (EFI_DEVICE_PATH_PROTOCOL *), DevicePathConsistMappingCompare);
  return EFI_SUCCESS;
}

BOOLEAN
IsHIDevicePathNode (
  EFI_DEVICE_PATH_PROTOCOL *DevicePathNode
  )
{
  ACPI_HID_DEVICE_PATH  *Acpi;

  ASSERT(DevicePathNode != NULL);

  if (DevicePathNode->Type == HARDWARE_DEVICE_PATH) {
    return TRUE;
  }

  if (DevicePathNode->Type == ACPI_DEVICE_PATH) {
    Acpi = (ACPI_HID_DEVICE_PATH *) DevicePathNode;
    switch (EISA_ID_TO_NUM (Acpi->HID)) {
    case 0x0301:
    case 0x0401:
    case 0x0501:
    case 0x0604:
      return FALSE;
    }

    return TRUE;
  }

  return FALSE;
}

EFI_DEVICE_PATH_PROTOCOL *
GetHIDevicePath (
  EFI_DEVICE_PATH_PROTOCOL        *DevicePath
  )
{
  UINTN                     NonHIDevicePathNodeCount;
  UINTN                     Index;
  EFI_DEV_PATH              Node;
  EFI_DEVICE_PATH_PROTOCOL  *HIDevicePath;
  EFI_DEVICE_PATH_PROTOCOL  *TempDevicePath;

  ASSERT(DevicePath != NULL);

  NonHIDevicePathNodeCount  = 0;

  HIDevicePath              = AllocatePool (sizeof (EFI_DEVICE_PATH_PROTOCOL));
  SetDevicePathEndNode (HIDevicePath);

  Node.DevPath.Type       = END_DEVICE_PATH_TYPE;
  Node.DevPath.SubType    = END_INSTANCE_DEVICE_PATH_SUBTYPE;
  Node.DevPath.Length[0]  = sizeof (EFI_DEVICE_PATH_PROTOCOL);
  Node.DevPath.Length[1]  = 0;

  while (!IsDevicePathEnd (DevicePath)) {
    if (IsHIDevicePathNode (DevicePath)) {
      for (Index = 0; Index < NonHIDevicePathNodeCount; Index++) {
        TempDevicePath = AppendDevicePathNode (HIDevicePath, &Node.DevPath);
        FreePool (HIDevicePath);
        HIDevicePath = TempDevicePath;
      }

      TempDevicePath = AppendDevicePathNode (HIDevicePath, DevicePath);
      FreePool (HIDevicePath);
      HIDevicePath = TempDevicePath;
    } else {
      NonHIDevicePathNodeCount++;
    }
    //
    // Next device path node
    //
    DevicePath = (EFI_DEVICE_PATH_PROTOCOL *) NextDevicePathNode (DevicePath);
  }

  return HIDevicePath;
}

EFI_STATUS
GetDeviceConsistMappingInfo (
  DEVICE_CONSIST_MAPPING_INFO    *MappingItem,
  EFI_DEVICE_PATH_PROTOCOL       *DevicePath
  )
{
  VOID (*SerialFun) (EFI_DEVICE_PATH_PROTOCOL *, DEVICE_CONSIST_MAPPING_INFO *);

  UINTN Index;

  ASSERT(DevicePath != NULL);
  ASSERT(MappingItem != NULL);

  SetMem (&MappingItem->CSD, sizeof (POOL_PRINT), 0);

  while (!IsDevicePathEnd (DevicePath)) {
    //
    // Find the handler to dump this device path node
    //
    SerialFun = NULL;
    for (Index = 0; DevPathConsistMappingTable[Index].SerialFun; Index += 1) {

      if (DevicePathType (DevicePath) == DevPathConsistMappingTable[Index].Type &&
          DevicePathSubType (DevicePath) == DevPathConsistMappingTable[Index].SubType
          ) {
        SerialFun = DevPathConsistMappingTable[Index].SerialFun;
        break;
      }
    }
    //
    // If not found, use a generic function
    //
    if (!SerialFun) {
      SerialFun = _DevPathSerialDefault;
    }

    SerialFun (DevicePath, MappingItem);

    //
    // Next device path node
    //
    DevicePath = (EFI_DEVICE_PATH_PROTOCOL *) NextDevicePathNode (DevicePath);
  }

  return EFI_SUCCESS;
}

EFI_STATUS
ConsistMappingCreateHIDevicePathTable (
  EFI_DEVICE_PATH_PROTOCOL           ***HIDevicePathTable
  )
/*++

Routine Description:

  Create the empty mapping table.
  
Arguments:
  
  HIDevicePathTable - The device table
  
Returns: 
  EFI_OUT_OF_RESOURCES - Out of resources
  EFI_SUCCESS          - Success

--*/
{
  EFI_HANDLE                *Handle;
  UINTN                     HandleNum;
  EFI_DEVICE_PATH_PROTOCOL  **Table;
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath;
  EFI_DEVICE_PATH_PROTOCOL  *HIDevicePath;
  UINTN                     Index;

  Handle = NULL;

  ShellInitHandleEnumerator ();
  HandleNum = ShellGetHandleNum ();
  Table     = AllocateZeroPool ((HandleNum + 1) * sizeof (EFI_DEVICE_PATH_PROTOCOL *));
  if (HIDevicePathTable == NULL) {
    ShellCloseHandleEnumerator ();
    return EFI_OUT_OF_RESOURCES;
  }

  for (ShellNextHandle (&Handle); Handle != NULL; ShellNextHandle (&Handle)) {
    DevicePath = DevicePathFromHandle (*Handle);
    if (DevicePath == NULL) {
      continue;
    }

    HIDevicePath = GetHIDevicePath (DevicePath);
    if (HIDevicePath == NULL) {
      continue;
    }

    for (Index = 0; Table[Index] != NULL; Index++) {
      if (DevicePathCompare (Table[Index], HIDevicePath) == 0) {
        FreePool (HIDevicePath);
        break;
      }
    }

    if (Table[Index] == NULL) {
      Table[Index] = HIDevicePath;
    }
  }

  ShellCloseHandleEnumerator ();
  for (Index = 0; Table[Index] != NULL; Index++)
    ;
  DevicePathConsistMappingSort (Table, Index);
  *HIDevicePathTable = Table;
  return EFI_SUCCESS;
}

EFI_STATUS
ConsistMappingFreeHIDevicePathTable (
  EFI_DEVICE_PATH_PROTOCOL **HIDevicePathTable
  )
{
  UINTN Index;
  for (Index = 0; HIDevicePathTable[Index] != NULL; Index++) {
    FreePool (HIDevicePathTable[Index]);
  }

  FreePool (HIDevicePathTable);
  return EFI_SUCCESS;
}

CHAR16 *
ConsistMappingGenMappingName (
  EFI_DEVICE_PATH_PROTOCOL    *DevicePath,
  EFI_DEVICE_PATH_PROTOCOL    **HIDevicePathTable
  )
{
  POOL_PRINT                  Str;
  DEVICE_CONSIST_MAPPING_INFO MappingInfo;
  EFI_DEVICE_PATH_PROTOCOL    *HIDevicePath;
  UINTN                       Index;
  UINTN                       NewSize;
  HIDevicePath = GetHIDevicePath (DevicePath);
  if (HIDevicePath == NULL) {
    return NULL;
  }

  for (Index = 0; HIDevicePathTable[Index] != NULL; Index++) {
    if (DevicePathCompare (HIDevicePathTable[Index], HIDevicePath) == 0) {
      break;
    }
  }

  FreePool (HIDevicePath);
  if (HIDevicePathTable[Index] == NULL) {
    return NULL;
  }

  MappingInfo.HI      = Index;
  MappingInfo.MTD     = MTDTypeUnknown;
  MappingInfo.Digital = FALSE;

  GetDeviceConsistMappingInfo (&MappingInfo, DevicePath);

  SetMem (&Str, sizeof (Str), 0);
  for (Index = 0; mMTDName[Index].MTDType != MTDTypeEnd; Index++) {
    if (MappingInfo.MTD == mMTDName[Index].MTDType) {
      break;
    }
  }

  if (mMTDName[Index].MTDType != MTDTypeEnd) {
    CatPrint (&Str, L"%s", mMTDName[Index].Name);
  }

  CatPrint (&Str, L"%d", (UINTN) MappingInfo.HI);
  if (MappingInfo.CSD.str != NULL) {
    CatPrint (&Str, L"%s", MappingInfo.CSD.str);
  }

  NewSize           = (Str.len + 1) * sizeof (CHAR16);
  Str.str           = ReallocatePool (Str.str, NewSize, NewSize);
  Str.str[Str.len]  = 0;
  return Str.str;
}

BOOLEAN
DevicePathIsChildDevice (
  IN  EFI_DEVICE_PATH_PROTOCOL *ParentDevicePath,
  IN  EFI_DEVICE_PATH_PROTOCOL *ChildDevicePath
  )
{
  if (ParentDevicePath == NULL || ParentDevicePath == NULL) {
    return FALSE;
  }

  while (!(IsDevicePathEnd (ParentDevicePath) || IsDevicePathEnd (ChildDevicePath))) {
    if (_DevPathCompareDefault (ParentDevicePath, ChildDevicePath) != 0) {
      return FALSE;
    }

    ParentDevicePath  = (EFI_DEVICE_PATH_PROTOCOL *) NextDevicePathNode (ParentDevicePath);
    ChildDevicePath   = (EFI_DEVICE_PATH_PROTOCOL *) NextDevicePathNode (ChildDevicePath);
  }

  if (IsDevicePathEnd (ParentDevicePath)) {
    return TRUE;
  }

  return FALSE;
}
