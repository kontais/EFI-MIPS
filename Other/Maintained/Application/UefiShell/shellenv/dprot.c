/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  dprot.c

Abstract:

  Shell environment - dump protocol functions for the "dh" command

Revision History

--*/

#include "shelle.h"
#include "shellenvguid.h"
#include "Acpi.h"

STATIC CHAR16 *SEnvDP_IlleagalStr[] = { L"Illegal" };

STATIC CHAR16 *SEnvDP_HardwareStr[] = {
  L"Illegal",
  L"PCI",
  L"PCMCIA",
  L"Memory Mapped",
  L"Vendor-Defined",
  L"Controller"
};

STATIC CHAR16 *SEnvDP_ACPI_Str[] = { L"Illegal", L"Acpi", L"AcpiEx", L"AcpiAdr" };

STATIC CHAR16 *SEnvDP_MessageStr[] = {
  L"Illegal",
  L"ATAPI",
  L"SCSI",
  L"Fibre Channel",
  L"1394",
  L"USB",
  L"I2O",
  L"Illegal",
  L"Illegal",
  L"InfiniBand",
  L"Vendor-Defined",
  L"MAC",
  L"IPv4",
  L"IPv6",
  L"UART",
  L"USB Class"
};

STATIC CHAR16 *SEnvDP_MediaStr[] = {
  L"Illegal",
  L"Hard Drive",
  L"CD-ROM",
  L"Vender-Defined",
  L"File Path",
  L"Media Protocol"
};

STATIC CHAR16 *SEnvDP_BBS_Str[] = { L"Illegal", L"BIOS Boot Spec" };

struct DevicePathTypes {
  UINT8   Type;
  UINT8   MaxSubType;
  CHAR16  *TypeString;
  CHAR16  **SubTypeStr;
  VOID (*Function) (EFI_DEVICE_PATH_PROTOCOL *);
};

VOID
SEnvHardwareDevicePathEntry (
  IN EFI_DEVICE_PATH_PROTOCOL  *DevicePath
  )
/*++

Routine Description:

Arguments:

  DevicePath - The device path

Returns:

--*/
{
  PCI_DEVICE_PATH         *PciDevicePath;
  PCCARD_DEVICE_PATH      *PcCardDevicePath;
  MEMMAP_DEVICE_PATH      *MemMapDevicePath;
  CONTROLLER_DEVICE_PATH  *ControllerDevicePath;
  VENDOR_DEVICE_PATH      *VendorDevicePath;

  if (DevicePathType (DevicePath) != HW_PCI_DP) {
    return ;
  }
  //
  // Process hardware device path entry
  //
  switch (DevicePathSubType (DevicePath)) {
  case HW_PCI_DP:
    PciDevicePath = (PCI_DEVICE_PATH *) DevicePath;
    PrintToken (
      STRING_TOKEN (STR_SHELLENV_DPROT_FUNCTION_DEVICE),
      HiiEnvHandle,
      PciDevicePath->Function,
      PciDevicePath->Device
      );
    break;

  case HW_PCCARD_DP:
    PcCardDevicePath = (PCCARD_DEVICE_PATH *) DevicePath;
    PrintToken (
      STRING_TOKEN (STR_SHELLENV_DPROT_FUNCTION_NUMBER),
      HiiEnvHandle,
      PcCardDevicePath->FunctionNumber
      );
    break;

  case HW_MEMMAP_DP:
    MemMapDevicePath = (MEMMAP_DEVICE_PATH *) DevicePath;
    PrintToken (
      STRING_TOKEN (STR_SHELLENV_DPROT_MEMORY_TYPE),
      HiiEnvHandle,
      MemMapDevicePath->MemoryType,
      MemMapDevicePath->StartingAddress,
      MemMapDevicePath->EndingAddress
      );
    break;

  case HW_CONTROLLER_DP:
    ControllerDevicePath = (CONTROLLER_DEVICE_PATH *) DevicePath;
    PrintToken (STRING_TOKEN (STR_SHELLENV_DPROT_CONTROLLER_NUMBER), HiiEnvHandle, ControllerDevicePath->Controller);
    break;

  case HW_VENDOR_DP:
    VendorDevicePath = (VENDOR_DEVICE_PATH *) DevicePath;
    PrintToken (
      STRING_TOKEN (STR_SHELLENV_DPROT_GUID),
      HiiEnvHandle,
      &VendorDevicePath->Guid
      );
    if (DevicePathNodeLength (DevicePath) > sizeof (VENDOR_DEVICE_PATH)) {
      Print (L"\n");
      DumpHex (
        7,
        0,
        DevicePathNodeLength (DevicePath) - sizeof (VENDOR_DEVICE_PATH),
        VendorDevicePath + 1
        );
    }
    break;
  }
  //
  // End processing
  //
}

VOID
SEnvAcpiDevicePathEntry (
  IN EFI_DEVICE_PATH_PROTOCOL      *DevicePath
  )
/*++

Routine Description:

Arguments:

  DevicePath - The device path

Returns:

--*/
{
  ACPI_HID_DEVICE_PATH                   *AcpiDevicePath;
  ACPI_EXTENDED_HID_DEVICE_PATH_WITH_STR *AcpiExDevicepath;
  ACPI_ADR_DEVICE_PATH                   *AcpiAdrDevicePath;
  UINT16                                 Index;
  UINT16                                 Length;
  UINT16                                 AdditionalAdrCount;
  CHAR8                                  *HIDString;
  CHAR8                                  *UIDString;
  CHAR8                                  *CIDString;
  CHAR8                                  NullString[5];

  if (DevicePathType (DevicePath) != ACPI_DEVICE_PATH) {
    return ;
  }
  //
  // Process ACPI device path entry
  //
  switch (DevicePathSubType (DevicePath)) {
  case ACPI_DP:
    AcpiDevicePath = (ACPI_HID_DEVICE_PATH *) DevicePath;
    if ((AcpiDevicePath->HID & PNP_EISA_ID_MASK) == PNP_EISA_ID_CONST) {
      PrintToken (
        STRING_TOKEN (STR_SHELLENV_DPROT_HIDPNP_UID),
        HiiEnvHandle,
        EISA_ID_TO_NUM (AcpiDevicePath->HID),
        AcpiDevicePath->UID
        );
    } else {
       PrintToken (
        STRING_TOKEN (STR_SHELLENV_DPROT_HID_UID),
        HiiEnvHandle,
        AcpiDevicePath->HID,
        AcpiDevicePath->UID
        );
    }
    break;
  case ACPI_EXTENDED_DP:
    AcpiExDevicepath = (ACPI_EXTENDED_HID_DEVICE_PATH_WITH_STR *) DevicePath;
    HIDString        = AcpiExDevicepath->HidUidCidStr;
    UIDString        = NextStrA (HIDString);
    CIDString        = NextStrA (UIDString);
    CopyMem (NullString, "NULL", sizeof (NullString));
    if (*HIDString == '\0') {
      HIDString = NullString;
    }
    if (*UIDString == '\0') {
      UIDString = NullString;
    }
    if (*CIDString == '\0') {
      CIDString = NullString;
    }
    if ((AcpiExDevicepath->HID & PNP_EISA_ID_MASK) == PNP_EISA_ID_CONST) {
      if ((AcpiExDevicepath->CID & PNP_EISA_ID_MASK) == PNP_EISA_ID_CONST) {
        PrintToken (
          STRING_TOKEN (STR_SHELLENV_DPROT_HIDPNP_CIDPNP_UID),
          HiiEnvHandle,
          EISA_ID_TO_NUM (AcpiExDevicepath->HID),
          EISA_ID_TO_NUM (AcpiExDevicepath->CID),
          AcpiExDevicepath->UID,
          HIDString,
          CIDString,
          UIDString
          );
      } else {
        PrintToken (
          STRING_TOKEN (STR_SHELLENV_DPROT_HIDPNP_CID_UID),
          HiiEnvHandle,
          EISA_ID_TO_NUM (AcpiExDevicepath->HID),
          AcpiExDevicepath->CID,
          AcpiExDevicepath->UID,
          HIDString,
          CIDString,
          UIDString
          );
      }
    } else {
      if ((AcpiExDevicepath->CID & PNP_EISA_ID_MASK) == PNP_EISA_ID_CONST) {
        PrintToken (
          STRING_TOKEN (STR_SHELLENV_DPROT_HID_CIDPNP_UID),
          HiiEnvHandle,
          AcpiExDevicepath->HID,
          EISA_ID_TO_NUM (AcpiExDevicepath->CID),
          AcpiExDevicepath->UID,
          HIDString,
          CIDString,
          UIDString
          );
      } else {
        PrintToken (
          STRING_TOKEN (STR_SHELLENV_DPROT_HID_CID_UID),
          HiiEnvHandle,
          AcpiExDevicepath->HID,
          AcpiExDevicepath->CID,
          AcpiExDevicepath->UID,
          HIDString,
          CIDString,
          UIDString
          );
      }
    }
    break;
  case ACPI_ADR_DP:
    AcpiAdrDevicePath  = (ACPI_ADR_DEVICE_PATH *) DevicePath;
    Length             = DevicePathNodeLength (DevicePath);
    AdditionalAdrCount = (Length - 8) / 4;

    PrintToken (STRING_TOKEN (STR_SHELLENV_DPROT_ADR), HiiEnvHandle);
    Print (L"%hx", AcpiAdrDevicePath->ADR);
    for (Index = 0; Index < AdditionalAdrCount; Index++) {
      Print (L",%hx", *(UINT32 *) ((UINT8 *) AcpiAdrDevicePath + 8 + Index * 4));
    }
    break;
  default:
    break;
  }
}

VOID
SEnvMessagingDevicePathEntry (
  IN EFI_DEVICE_PATH_PROTOCOL      *DevicePath
  )
/*++

Routine Description:

Arguments:

  DevicePath - The device path

Returns:

--*/
{
  ATAPI_DEVICE_PATH         *AtapiDevicePath;
  SCSI_DEVICE_PATH          *ScsiDevicePath;
  FIBRECHANNEL_DEVICE_PATH  *FibreChannelDevicePath;
  F1394_DEVICE_PATH         *F1394DevicePath;
  USB_DEVICE_PATH           *UsbDevicePath;
  USB_CLASS_DEVICE_PATH     *UsbClassDevicePath;
  I2O_DEVICE_PATH           *I2ODevicePath;
  MAC_ADDR_DEVICE_PATH      *MacAddrDevicePath;
  IPv4_DEVICE_PATH          *IPv4DevicePath;
  IPv6_DEVICE_PATH          *IPv6DevicePath;
  INFINIBAND_DEVICE_PATH    *InfinibandDevicePath;
  UART_DEVICE_PATH          *UartDevicePath;
  VENDOR_DEVICE_PATH        *VendorDevicePath;

  UINTN                     HwAddressSize;
  UINTN                     Index;
  CHAR8                     Parity;

  if (DevicePathType (DevicePath) != MESSAGING_DEVICE_PATH) {
    return ;
  }
  //
  // Process messaging device path entry
  //
  switch (DevicePathSubType (DevicePath)) {
  case MSG_ATAPI_DP:
    AtapiDevicePath = (ATAPI_DEVICE_PATH *) DevicePath;
    PrintToken (
      STRING_TOKEN (STR_SHELLENV_DPROT_ATAPI),
      HiiEnvHandle,
      AtapiDevicePath->PrimarySecondary ? L"Secondary" : L"Primary",
      AtapiDevicePath->SlaveMaster ? L"Slave" : L"Master",
      AtapiDevicePath->Lun
      );
    break;

  case MSG_SCSI_DP:
    ScsiDevicePath = (SCSI_DEVICE_PATH *) DevicePath;
    PrintToken (
      STRING_TOKEN (STR_SHELLENV_DPROT_PUN_LUN),
      HiiEnvHandle,
      ScsiDevicePath->Pun,
      ScsiDevicePath->Lun
      );
    break;

  case MSG_FIBRECHANNEL_DP:
    FibreChannelDevicePath = (FIBRECHANNEL_DEVICE_PATH *) DevicePath;
    PrintToken (
      STRING_TOKEN (STR_SHELLENV_DPROT_FIBRE_CHANNEL),
      HiiEnvHandle,
      FibreChannelDevicePath->WWN,
      FibreChannelDevicePath->Lun
      );
    break;

  case MSG_1394_DP:
    F1394DevicePath = (F1394_DEVICE_PATH *) DevicePath;
    PrintToken (STRING_TOKEN (STR_SHELLENV_DPROT_GUID_2), HiiEnvHandle, F1394DevicePath->Guid);
    break;

  case MSG_USB_DP:
    UsbDevicePath = (USB_DEVICE_PATH *) DevicePath;
    PrintToken (
      STRING_TOKEN (STR_SHELLENV_DPROT_USB),
      HiiEnvHandle,
      UsbDevicePath->ParentPortNumber,
      UsbDevicePath->InterfaceNumber
      );
    break;

  case MSG_USB_CLASS_DP:
    UsbClassDevicePath = (USB_CLASS_DEVICE_PATH *) DevicePath;
    PrintToken (
      STRING_TOKEN (STR_SHELLENV_DPROT_USB_CLASS),
      HiiEnvHandle,
      UsbClassDevicePath->VendorId,
      UsbClassDevicePath->ProductId
      );
    PrintToken (
      STRING_TOKEN (STR_SHELLENV_DPROT_DEVICE),
      HiiEnvHandle,
      UsbClassDevicePath->DeviceClass,
      UsbClassDevicePath->DeviceSubClass,
      UsbClassDevicePath->DeviceProtocol
      );
    break;

  case MSG_I2O_DP:
    I2ODevicePath = (I2O_DEVICE_PATH *) DevicePath;
    PrintToken (
      STRING_TOKEN (STR_SHELLENV_DPROT_I20),
      HiiEnvHandle,
      I2ODevicePath->Tid
      );
    break;

  case MSG_MAC_ADDR_DP:
    MacAddrDevicePath = (MAC_ADDR_DEVICE_PATH *) DevicePath;
    HwAddressSize     = sizeof (EFI_MAC_ADDRESS);
    if (MacAddrDevicePath->IfType == 0x01 || MacAddrDevicePath->IfType == 0x00) {
      HwAddressSize = 6;
    }

    PrintToken (STRING_TOKEN (STR_SHELLENV_DPROT_MAC), HiiEnvHandle);
    for (Index = 0; Index < HwAddressSize; Index++) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_DPROT_ONE_VAR_X), HiiEnvHandle, MacAddrDevicePath->MacAddress.Addr[Index]);
    }

    Print (L")");
    break;

  case MSG_IPv4_DP:
    IPv4DevicePath = (IPv4_DEVICE_PATH *) DevicePath;
    PrintToken (
      STRING_TOKEN (STR_SHELLENV_DPROT_LOCAL),
      HiiEnvHandle,
      IPv4DevicePath->LocalIpAddress.Addr[0],
      IPv4DevicePath->LocalIpAddress.Addr[1],
      IPv4DevicePath->LocalIpAddress.Addr[2],
      IPv4DevicePath->LocalIpAddress.Addr[3],
      IPv4DevicePath->LocalPort
      );
    PrintToken (
      STRING_TOKEN (STR_SHELLENV_DPROT_REMOVE_IP),
      HiiEnvHandle,
      IPv4DevicePath->RemoteIpAddress.Addr[0],
      IPv4DevicePath->RemoteIpAddress.Addr[1],
      IPv4DevicePath->RemoteIpAddress.Addr[2],
      IPv4DevicePath->RemoteIpAddress.Addr[3],
      IPv4DevicePath->RemotePort
      );
    PrintToken (
      STRING_TOKEN (STR_SHELLENV_DPROT_PROTOCOL),
      HiiEnvHandle,
      IPv4DevicePath->Protocol
      );
    PrintToken (
      STRING_TOKEN (STR_SHELLENV_DPROT_SOURCE),
      HiiEnvHandle,
      IPv4DevicePath->StaticIpAddress ? L"Static" : L"DHCP"
      );
    break;

  case MSG_IPv6_DP:
    IPv6DevicePath = (IPv6_DEVICE_PATH *) DevicePath;
    PrintToken (STRING_TOKEN (STR_SHELLENV_DPROT_NOT_AVAIL), HiiEnvHandle);
    break;

  case MSG_INFINIBAND_DP:
    InfinibandDevicePath = (INFINIBAND_DEVICE_PATH *) DevicePath;
    PrintToken (
      STRING_TOKEN (STR_SHELLENV_DPROT_MODE_GUID),
      HiiEnvHandle,
      InfinibandDevicePath->ServiceId,
      InfinibandDevicePath->TargetPortId
      );
    PrintToken (
      STRING_TOKEN (STR_SHELLENV_DPROT_DEVICE_ID),
      HiiEnvHandle,
      InfinibandDevicePath->DeviceId
      );
    break;

  case MSG_UART_DP:
    UartDevicePath = (UART_DEVICE_PATH *) DevicePath;
    switch (UartDevicePath->Parity) {
    case 0:
      Parity = 'D';
      break;

    case 1:
      Parity = 'N';
      break;

    case 2:
      Parity = 'E';
      break;

    case 3:
      Parity = 'O';
      break;

    case 4:
      Parity = 'M';
      break;

    case 5:
      Parity = 'S';
      break;

    default:
      Parity = 'x';
      break;
    }

    if (UartDevicePath->BaudRate == 0) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_DPROT_UART_DEFAULT), HiiEnvHandle, Parity);
    } else {
      PrintToken (STRING_TOKEN (STR_SHELLENV_DPROT_UART_D_C), HiiEnvHandle, UartDevicePath->BaudRate, Parity);
    }

    if (UartDevicePath->DataBits == 0) {
      Print (L" D");
    } else {
      PrintToken (STRING_TOKEN (STR_SHELLENV_DPROT_ONE_VAR_D), HiiEnvHandle, UartDevicePath->DataBits);
    }

    switch (UartDevicePath->StopBits) {
    case 0:
      Print (L" D)");
      break;

    case 1:
      Print (L" 1)");
      break;

    case 2:
      Print (L" 1.5)");
      break;

    case 3:
      Print (L" 2)");
      break;

    default:
      Print (L" x)");
      break;
    }
    break;

  case MSG_VENDOR_DP:
    VendorDevicePath = (VENDOR_DEVICE_PATH *) DevicePath;
    PrintToken (
      STRING_TOKEN (STR_SHELLENV_DPROT_GUID_G),
      HiiEnvHandle,
      &VendorDevicePath->Guid
      );
    if (DevicePathNodeLength (DevicePath) > sizeof (VENDOR_DEVICE_PATH)) {
      Print (L"\n");
      DumpHex (
        7,
        0,
        DevicePathNodeLength (DevicePath) - sizeof (VENDOR_DEVICE_PATH),
        VendorDevicePath + 1
        );
    }
    break;
  }
  //
  // End processing
  //
}

VOID
SEnvMediaDevicePathEntry (
  IN EFI_DEVICE_PATH_PROTOCOL  *DevicePath
  )
/*++

Routine Description:

Arguments:

  DevicePath - The device path

Returns:

--*/
{
  HARDDRIVE_DEVICE_PATH       *HardDriveDevicePath;
  CDROM_DEVICE_PATH           *CDDevicePath;
  VENDOR_DEVICE_PATH          *VendorDevicePath;
  FILEPATH_DEVICE_PATH        *FilePath;
  MEDIA_PROTOCOL_DEVICE_PATH  *MediaProtocol;

  if (DevicePathType (DevicePath) != MEDIA_DEVICE_PATH) {
    return ;
  }
  //
  // Process media device path entry
  //
  switch (DevicePathSubType (DevicePath)) {
  case MEDIA_HARDDRIVE_DP:
    HardDriveDevicePath = (HARDDRIVE_DEVICE_PATH *) DevicePath;
    PrintToken (
      STRING_TOKEN (STR_SHELLENV_DPROT_PARTITION_START_SIZE),
      HiiEnvHandle,
      HardDriveDevicePath->PartitionNumber,
      HardDriveDevicePath->PartitionStart,
      HardDriveDevicePath->PartitionSize
      );
    break;

  case MEDIA_CDROM_DP:
    CDDevicePath = (CDROM_DEVICE_PATH *) DevicePath;
    PrintToken (
      STRING_TOKEN (STR_SHELLENV_DPROT_BOOTENTRY_START_SIZE),
      HiiEnvHandle,
      CDDevicePath->BootEntry,
      CDDevicePath->PartitionStart,
      CDDevicePath->PartitionSize
      );
    break;

  case MEDIA_VENDOR_DP:
    VendorDevicePath = (VENDOR_DEVICE_PATH *) DevicePath;
    PrintToken (
      STRING_TOKEN (STR_SHELLENV_DPROT_GUID_G),
      HiiEnvHandle,
      &VendorDevicePath->Guid
      );
    if (DevicePathNodeLength (DevicePath) > sizeof (VENDOR_DEVICE_PATH)) {
      Print (L"\n");
      DumpHex (
        7,
        0,
        DevicePathNodeLength (DevicePath) - sizeof (VENDOR_DEVICE_PATH),
        VendorDevicePath + 1
        );
    }
    break;

  case MEDIA_FILEPATH_DP:
    FilePath = (FILEPATH_DEVICE_PATH *) DevicePath;
    PrintToken (STRING_TOKEN (STR_SHELLENV_DPROT_FILE), HiiEnvHandle, FilePath->PathName);
    break;

  case MEDIA_PROTOCOL_DP:
    MediaProtocol = (MEDIA_PROTOCOL_DEVICE_PATH *) DevicePath;
    PrintToken (STRING_TOKEN (STR_SHELLENV_DPROT_PROTOCOL_2), HiiEnvHandle, &MediaProtocol->Protocol);
    break;
  };

  //
  // End processing
  //
}

struct DevicePathTypes  SEnvDP_Strings[] = {
  0x00,
  0x01,
  L"Illegal",
  SEnvDP_IlleagalStr,
  NULL,
  0x01,
  0x05,
  L"Hardware",
  SEnvDP_HardwareStr,
  SEnvHardwareDevicePathEntry,
  0x02,
  0x03,
  L"ACPI",
  SEnvDP_ACPI_Str,
  SEnvAcpiDevicePathEntry,
  0x03,
  0x0f,
  L"Messaging",
  SEnvDP_MessageStr,
  SEnvMessagingDevicePathEntry,
  0x04,
  0x05,
  L"Media",
  SEnvDP_MediaStr,
  SEnvMediaDevicePathEntry,
  0x05,
  0x01,
  L"BIOS Boot Spec",
  SEnvDP_BBS_Str,
  NULL,

  END_DEVICE_PATH_TYPE,
  0x01,
  L"End",
  SEnvDP_IlleagalStr,
  NULL
};

VOID
SEnvPrintDevicePathEntry (
  IN EFI_DEVICE_PATH_PROTOCOL     *DevicePath,
  IN BOOLEAN                      Verbose
  )
/*++

Routine Description:

Arguments:

  DevicePath - The device path
  Verbose    - Verbose
Returns:

--*/
{
  UINT8 Type;
  UINT8 SubType;
  INTN  Index;

  //
  // Process print device path entry
  //
  Type    = (UINT8) DevicePathType (DevicePath);
  SubType = DevicePathSubType (DevicePath);

  for (Index = 0; SEnvDP_Strings[Index].Type != END_DEVICE_PATH_TYPE; Index++) {
    if (Type == SEnvDP_Strings[Index].Type) {
      if (SubType > SEnvDP_Strings[Index].MaxSubType) {
        SubType = 0;
      }

      PrintToken (
        STRING_TOKEN (STR_SHELLENV_DPROT_DEVICE_PATH_FOR),
        HiiEnvHandle,
        SEnvDP_Strings[Index].TypeString,
        SEnvDP_Strings[Index].SubTypeStr[SubType]
        );
      if (Verbose) {
        if (SEnvDP_Strings[Index].Function != NULL) {
          SEnvDP_Strings[Index].Function (DevicePath);
        }
      }

      return ;
    }
  }

  PrintToken (STRING_TOKEN (STR_SHELLENV_DPROT_DEVICE_PATH_ERROR), HiiEnvHandle);
}

VOID
EFIAPI
SEnvDPath (
  IN EFI_HANDLE           h,
  IN VOID                 *Interface
  )
/*++

Routine Description:

Arguments:

  h         - An EFI handle
  Interface - The interface

Returns:

--*/
{
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath;
  EFI_DEVICE_PATH_PROTOCOL  *DevicePathNode;
  CHAR16                    *Str;

  DevicePath  = Interface;
  Str         = LibDevicePathToStr (DevicePath);
  DevicePath  = UnpackDevicePath (DevicePath);

  //
  // Print device path entry
  //
  DevicePathNode = DevicePath;
  while (!IsDevicePathEnd (DevicePathNode)) {
    SEnvPrintDevicePathEntry (DevicePathNode, TRUE);
    DevicePathNode = NextDevicePathNode (DevicePathNode);
  }

  PrintToken (STRING_TOKEN (STR_SHELLENV_DPROT_ASSTR), HiiEnvHandle, Str);
  FreePool (Str);
  FreePool (DevicePath);
}

VOID
EFIAPI
SEnvDPathTok (
  IN EFI_HANDLE   h,
  IN VOID         *Interface
  )
/*++

Routine Description:

Arguments:

  h         - An EFI handle
  Interface - The interface

Returns:

--*/
{
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath;
  CHAR16                    *Str;
  CHAR16                    *Disp;
  UINTN                     Len;

  DevicePath  = Interface;
  Str         = LibDevicePathToStr (DevicePath);
  Disp        = L"";

  //
  // Print device path token
  //
  if (Str) {
    Len   = StrLen (Str);
    Disp  = Str;
    if (Len > 30) {
      Disp    = Str + Len - 30;
      Disp[0] = '.';
      Disp[1] = '.';
    }
  }

  PrintToken (STRING_TOKEN (STR_SHELLENV_DPROT_DEVPATH), HiiEnvHandle, Disp);

  if (Str) {
    FreePool (Str);
  }
}

VOID
EFIAPI
SEnvTextOut (
  IN EFI_HANDLE   h,
  IN VOID         *Interface
  )
/*++

Routine Description:

Arguments:

  h         - An EFI handle
  Interface - The interface

Returns:

--*/
{
  EFI_SIMPLE_TEXT_OUT_PROTOCOL  *Dev;
  INTN                          Index;
  UINTN                         Col;
  UINTN                         Row;
  EFI_STATUS                    Status;

  Dev = Interface;

  PrintToken (
    STRING_TOKEN (STR_SHELLENV_DPROT_ATTRIB_X),
    HiiEnvHandle,
    Dev->Mode->Attribute
    );

  //
  // Dump TextOut Info
  //
  for (Index = 0; Index < Dev->Mode->MaxMode; Index++) {
    Status = Dev->QueryMode (Dev, Index, &Col, &Row);
    PrintToken (
      STRING_TOKEN (STR_SHELLENV_DPROT_MODE),
      HiiEnvHandle,
      Index == Dev->Mode->Mode ? '*' : ' ',
      Index
      );

    if (EFI_ERROR (Status)) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_DPROT_ERROR), HiiEnvHandle, Status);
    } else {
      PrintToken (STRING_TOKEN (STR_SHELLENV_DPROT_COL_ROW), HiiEnvHandle, Col, Row);
    }
  }
}

VOID
EFIAPI
SEnvBlkIo (
  IN EFI_HANDLE   h,
  IN VOID         *Interface
  )
/*++

Routine Description:

Arguments:

  h         - An EFI handle
  Interface - The interface

Returns:

--*/
{
  EFI_BLOCK_IO_PROTOCOL *BlkIo;
  EFI_BLOCK_IO_MEDIA    *BlkMedia;
  VOID                  *Buffer;

  BlkIo     = Interface;
  BlkMedia  = BlkIo->Media;

  //
  // Issue a dummy read to the device to check for media change
  //
  Buffer = AllocatePool (BlkMedia->BlockSize);
  if (Buffer) {
    BlkIo->ReadBlocks (
            BlkIo,
            BlkMedia->MediaId,
            0,
            BlkMedia->BlockSize,
            Buffer
            );
    FreePool (Buffer);
  }
  //
  // Dump BlkIo Info
  //
  PrintToken (
    STRING_TOKEN (STR_SHELLENV_DPROT_MID),
    HiiEnvHandle,
    BlkMedia->RemovableMedia ? L"Removable " : L"Fixed ",
    BlkMedia->MediaPresent ? L"" : L"not-present ",
    BlkMedia->MediaId
    );

  PrintToken (
    STRING_TOKEN (STR_SHELLENV_DPROT_BSIZE_LBLOCK),
    HiiEnvHandle,
    BlkMedia->BlockSize,
    BlkMedia->LastBlock,
    MultU64x32 (BlkMedia->LastBlock + 1, BlkMedia->BlockSize),
    BlkMedia->LogicalPartition ? L"partition" : L"raw",
    BlkMedia->ReadOnly ? L"ro" : L"rw",
    BlkMedia->WriteCaching ? L"cached" : L"!cached"
    );
}

CHAR8 *
GetPdbPath (
  VOID *ImageBase
  )
/*++

Routine Description:
  Located PDB path name in PE image

Arguments:
  ImageBase - base of PE to search

Returns:
  Pointer into image at offset of PDB file name if PDB file name is found,
  Otherwise a pointer to an empty string.
  
--*/
{
  return NULL;
}

VOID
PrintShortPdbFileName (
  CHAR8  *PdbFileName,
  UINTN  Length
  )
/*++

Routine Description:

Arguments:

  PdbFileName  - The pdb file name
  Length       - The length

Returns:

--*/
{
  UINTN Index;
  UINTN StartIndex;
  UINTN EndIndex;

  if (PdbFileName == NULL) {
    Print (L"NULL");
  } else {
    StartIndex = 0;
    for (EndIndex = 0; PdbFileName[EndIndex] != 0; EndIndex++)
      ;
    for (Index = 0; PdbFileName[Index] != 0; Index++) {
      if (PdbFileName[Index] == '\\') {
        StartIndex = Index + 1;
      }

      if (PdbFileName[Index] == '.') {
        EndIndex = Index;
      }
    }

    for (Index = StartIndex; Index < EndIndex && Length > 0; Index++, Length--) {
      Print (L"%c", PdbFileName[Index]);
    }
  }
}

VOID
EFIAPI
SEnvImageTok (
  IN EFI_HANDLE       h,
  IN VOID             *Interface
  )
/*++

Routine Description:

Arguments:

  h         - An EFI handle
  Interface - The interface

Returns:

--*/
{
  EFI_LOADED_IMAGE_PROTOCOL *Image;
  CHAR16                    *ImageName;

  CHAR16                    *Tok;

  Image     = Interface;
  ImageName = LibGetImageName (Image);
  if (ImageName != NULL) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_DPROT_IMAGE_2), HiiEnvHandle);
    Print (ImageName);
    Print (L") ");
    FreePool (ImageName);
  } else {
    Tok = LibDevicePathToStr (Image->FilePath);
    PrintToken (STRING_TOKEN (STR_SHELLENV_DPROT_IMAGE), HiiEnvHandle, Tok);
    FreePool (Tok);
  }
}

VOID
EFIAPI
SEnvImage (
  IN EFI_HANDLE       h,
  IN VOID             *Interface
  )
/*++

Routine Description:

Arguments:

  h         - An EFI handle
  Interface - The interface

Returns:

--*/
{
  EFI_LOADED_IMAGE_PROTOCOL *Image;
  CHAR16                    *FilePath;
  CHAR8                     *PdbFileName;
  CHAR16                    *ImageName;

  Image       = Interface;

  FilePath    = LibDevicePathToStr (Image->FilePath);
  PdbFileName = GetPdbPath (Image->ImageBase);
  ImageName   = LibGetImageName (Image);

  if (ImageName != NULL) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_DPROT_FILE_3), HiiEnvHandle);
    Print (ImageName);
    PrintToken (STRING_TOKEN (STR_SHELLENV_DPROT_ONE_VAR_N_NEW), HiiEnvHandle);
    FreePool (ImageName);
  } else {
    PrintToken (STRING_TOKEN (STR_SHELLENV_DPROT_FILE_2), HiiEnvHandle, FilePath);
  }
  //
  // Dump Image Info
  //
  if (!Image->ImageBase) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_DPROT_INTERNAL_IMAGE), HiiEnvHandle, FilePath);
    PrintToken (STRING_TOKEN (STR_SHELLENV_DPROT_CODETYPE), HiiEnvHandle, MemoryTypeStr (Image->ImageCodeType));
    PrintToken (STRING_TOKEN (STR_SHELLENV_DPROT_DATATYPE_NEWLINE), HiiEnvHandle, MemoryTypeStr (Image->ImageDataType));
  } else {
    PrintToken (STRING_TOKEN (STR_SHELLENV_DPROT_PARENTHANDLE), HiiEnvHandle, Image->ParentHandle);
    PrintToken (STRING_TOKEN (STR_SHELLENV_DPROT_SYSTEMTABLE), HiiEnvHandle, Image->SystemTable);
    PrintToken (STRING_TOKEN (STR_SHELLENV_DPROT_DEVICEHANDLE), HiiEnvHandle, Image->DeviceHandle);
    PrintToken (STRING_TOKEN (STR_SHELLENV_DPROT_FILEPATH), HiiEnvHandle, FilePath);
    if (PdbFileName != NULL) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_DPROT_PDBFILENAME), HiiEnvHandle, PdbFileName);
    }

    PrintToken (
      STRING_TOKEN (STR_SHELLENV_DPROT_IMAGEBASE),
      HiiEnvHandle,
      Image->ImageBase,
      (CHAR8 *) Image->ImageBase + Image->ImageSize
      );
    PrintToken (STRING_TOKEN (STR_SHELLENV_DPROT_IMAGESIZE), HiiEnvHandle, Image->ImageSize);
    PrintToken (STRING_TOKEN (STR_SHELLENV_DPROT_CODETYPE), HiiEnvHandle, MemoryTypeStr (Image->ImageCodeType));
    PrintToken (STRING_TOKEN (STR_SHELLENV_DPROT_DATATYPE), HiiEnvHandle, MemoryTypeStr (Image->ImageDataType));
  }

  if (FilePath) {
    FreePool (FilePath);
  }
}

VOID
EFIAPI
SEnvIsaIo (
  IN EFI_HANDLE       h,
  IN VOID             *Interface
  )
/*++

Routine Description:

Arguments:

  h         - An EFI handle
  Interface - The interface

Returns:

--*/
{
  EFI_ISA_IO_PROTOCOL *IsaIo;
  UINTN               Index;

  IsaIo = Interface;

  //
  // Dump IsaIo Info
  //
  Print (L"\n");
  PrintToken (STRING_TOKEN (STR_SHELLENV_DPROT_ROM_SIZE), HiiEnvHandle, IsaIo->RomSize);
  PrintToken (STRING_TOKEN (STR_SHELLENV_DPROT_ROM_LOCATION), HiiEnvHandle, IsaIo->RomImage);

  PrintToken (STRING_TOKEN (STR_SHELLENV_DPROT_ISA_RESOURCE_LIST), HiiEnvHandle);
  for (Index = 0; IsaIo->ResourceList->ResourceItem[Index].Type != EfiIsaAcpiResourceEndOfList; Index++) {
    switch (IsaIo->ResourceList->ResourceItem[Index].Type) {
    case EfiIsaAcpiResourceIo:
      PrintToken (STRING_TOKEN (STR_SHELLENV_DPROT_IO), HiiEnvHandle);
      break;

    case EfiIsaAcpiResourceMemory:
      PrintToken (STRING_TOKEN (STR_SHELLENV_DPROT_MEM), HiiEnvHandle);
      break;

    case EfiIsaAcpiResourceDma:
      PrintToken (STRING_TOKEN (STR_SHELLENV_DPROT_DMA), HiiEnvHandle);
      break;

    case EfiIsaAcpiResourceInterrupt:
      PrintToken (STRING_TOKEN (STR_SHELLENV_DPROT_INT), HiiEnvHandle);
      break;
    }

    if (IsaIo->ResourceList->ResourceItem[Index].StartRange == IsaIo->ResourceList->ResourceItem[Index].EndRange) {
      PrintToken (
        STRING_TOKEN (STR_SHELLENV_DPROT_ATTR),
        HiiEnvHandle,
        IsaIo->ResourceList->ResourceItem[Index].StartRange,
        IsaIo->ResourceList->ResourceItem[Index].EndRange,
        IsaIo->ResourceList->ResourceItem[Index].Attribute
        );
    } else {
      PrintToken (
        STRING_TOKEN (STR_SHELLENV_DPROT_ATTR_2),
        HiiEnvHandle,
        IsaIo->ResourceList->ResourceItem[Index].StartRange,
        IsaIo->ResourceList->ResourceItem[Index].EndRange,
        IsaIo->ResourceList->ResourceItem[Index].Attribute
        );
    }
  }
}

VOID
EFIAPI
SEnvPciRootBridgeIo (
  IN EFI_HANDLE       h,
  IN VOID             *Interface
  )
/*++
Routine Description:

Arguments:

  h         - An EFI handle
  Interface - The interface

Returns:

--*/
{
  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL   *PciRootBridgeIo;
  EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR *Configuration;
  UINT64                            Supports;
  UINT64                            Attributes;

  PciRootBridgeIo = Interface;

  Configuration   = NULL;
  PciRootBridgeIo->Configuration (PciRootBridgeIo, (VOID **) &Configuration);
  PciRootBridgeIo->GetAttributes (PciRootBridgeIo, &Supports, &Attributes);

  //
  // Dump PciRootBridgeIo Info
  //
  Print (L"\n");
  PrintToken (STRING_TOKEN (STR_SHELLENV_DPROT_PARENTHANDLE), HiiEnvHandle, PciRootBridgeIo->ParentHandle);
  PrintToken (STRING_TOKEN (STR_SHELLENV_DPROT_SEGMENT), HiiEnvHandle, PciRootBridgeIo->SegmentNumber);
  PrintToken (STRING_TOKEN (STR_SHELLENV_DPROT_ATTRIBUTES), HiiEnvHandle, Attributes);
  PrintToken (STRING_TOKEN (STR_SHELLENV_DPROT_SUPPORTS), HiiEnvHandle, Supports);

  if (Configuration != NULL) {
    Print (L"\n");
    PrintToken (STRING_TOKEN (STR_SHELLENV_DPROT_TYPE_FLAG), HiiEnvHandle);
    Print (L"      ====  ====  ================  ================  ====");
    while (Configuration->Desc == ACPI_ADDRESS_SPACE_DESCRIPTOR) {
      Print (L"\n");
      switch (Configuration->ResType) {
      case ACPI_ADDRESS_SPACE_TYPE_MEM:
        Print (L"      MEM : ");
        break;

      case ACPI_ADDRESS_SPACE_TYPE_IO:
        Print (L"      I/O : ");
        break;

      case ACPI_ADDRESS_SPACE_TYPE_BUS:
        PrintToken (STRING_TOKEN (STR_SHELLENV_DPROT_BUS), HiiEnvHandle);
        break;
      }

      PrintToken (
        STRING_TOKEN (STR_SHELLENV_DPROT_FOUR_VARS),
        HiiEnvHandle,
        Configuration->SpecificFlag,
        Configuration->AddrRangeMin,
        Configuration->AddrRangeMax,
        Configuration->AddrSpaceGranularity
        );
      Configuration++;
    }
  }
}

VOID
EFIAPI
SEnvPciIo (
  IN EFI_HANDLE       h,
  IN VOID             *Interface
  )
/*++

Routine Description:

Arguments:

  h         - An EFI handle
  Interface - The interface

Returns:

--*/
{
  EFI_PCI_IO_PROTOCOL *PciIo;
  PCI_TYPE00          Pci;
  UINTN               Segment;
  UINTN               Bus;
  UINTN               Device;
  UINTN               Function;
  UINTN               Index;

  PciIo = Interface;

  PciIo->Pci.Read (PciIo, EfiPciIoWidthUint8, 0, sizeof (Pci), &Pci);

  PciIo->GetLocation (PciIo, &Segment, &Bus, &Device, &Function);

  //
  // Dump PciIo Info
  //
  Print (L"\n");
  PrintToken (STRING_TOKEN (STR_SHELLENV_DPROT_SEGMENT_2), HiiEnvHandle, Segment);
  PrintToken (STRING_TOKEN (STR_SHELLENV_DPROT_BUS_2), HiiEnvHandle, Bus);
  PrintToken (STRING_TOKEN (STR_SHELLENV_DPROT_DEVICE_NUMBER), HiiEnvHandle, Device);
  PrintToken (STRING_TOKEN (STR_SHELLENV_DPROT_FUNCTION_NUMBER_2), HiiEnvHandle, Function);
  PrintToken (STRING_TOKEN (STR_SHELLENV_DPROT_ROM_SIZE_2), HiiEnvHandle, PciIo->RomSize);
  PrintToken (STRING_TOKEN (STR_SHELLENV_DPROT_ROM_LOCATION), HiiEnvHandle, PciIo->RomImage);
  PrintToken (STRING_TOKEN (STR_SHELLENV_DPROT_VENDOR_ID), HiiEnvHandle, Pci.Hdr.VendorId);
  PrintToken (STRING_TOKEN (STR_SHELLENV_DPROT_DEVICE_ID_2), HiiEnvHandle, Pci.Hdr.DeviceId);
  PrintToken (
    STRING_TOKEN (STR_SHELLENV_DPROT_CLASS_CODE),
    HiiEnvHandle,
    Pci.Hdr.ClassCode[0],
    Pci.Hdr.ClassCode[1],
    Pci.Hdr.ClassCode[2]
    );
  PrintToken (STRING_TOKEN (STR_SHELLENV_DPROT_CONFIG_HEADER), HiiEnvHandle);
  for (Index = 0; Index < sizeof (Pci); Index++) {
    if ((Index % 0x10) == 0) {
      Print (L"\n       ");
    }

    PrintToken (STRING_TOKEN (STR_SHELLENV_DPROT_ONE_VAR_X), HiiEnvHandle, *((UINT8 *) (&Pci) + Index));
  }

  Print (L"\n");
}

VOID
EFIAPI
SEnvUsbIo (
  IN EFI_HANDLE       h,
  IN VOID             *Interface
  )
/*++

Routine Description:

Arguments:

  h         - An EFI handle
  Interface - The interface

Returns:

--*/
{
  EFI_USB_IO_PROTOCOL           *UsbIo;
  EFI_USB_INTERFACE_DESCRIPTOR  InterfaceDesc;

  UsbIo = Interface;

  UsbIo->UsbGetInterfaceDescriptor (UsbIo, &InterfaceDesc);

  //
  // Dump UsbIo Info
  //
  Print (L"\n");
  PrintToken (
    STRING_TOKEN (STR_SHELLENV_DPROT_INTERFACE_NUMBER),
    HiiEnvHandle,
    InterfaceDesc.InterfaceNumber
    );
  PrintToken (
    STRING_TOKEN (STR_SHELLENV_DPROT_INTERFACE_CLASS),
    HiiEnvHandle,
    InterfaceDesc.InterfaceClass
    );
  PrintToken (
    STRING_TOKEN (STR_SHELLENV_DPROT_INTERFACE_SUBCLASS),
    HiiEnvHandle,
    InterfaceDesc.InterfaceSubClass
    );
  PrintToken (
    STRING_TOKEN (STR_SHELLENV_DPROT_INTERFACE_PROTOCOL),
    HiiEnvHandle,
    InterfaceDesc.InterfaceProtocol
    );
}

VOID
EFIAPI
SEnvDebugSupport (
  IN EFI_HANDLE       h,
  IN VOID             *Interface
  )
/*++
Routine Description:

Arguments:

  h         - An EFI handle
  Interface - The interface

Returns:

--*/
{
  EFI_DEBUG_SUPPORT_PROTOCOL  *DebugSupport;

  DebugSupport = Interface;

  //
  // Dump Debug support info
  //
  Print (L"\n");
  Print (L"Isa = ");
  switch (DebugSupport->Isa) {
  case (IsaIa32):
    Print (L"IA-32");
    break;

  case (IsaIpf):
    Print (L"IPF");
    break;

  case (IsaEbc):
    Print (L"EBC");
    break;

  default:
    PrintToken (STRING_TOKEN (STR_SHELLENV_DPROT_UNKNOWN), HiiEnvHandle, DebugSupport->Isa);
    break;
  }
}

VOID
EFIAPI
SEnvBusSpecificDriverOverride (
  IN EFI_HANDLE       h,
  IN VOID             *Interface
  )
/*++

Routine Description:

Arguments:

  h         - An EFI handle
  Interface - The interface

Returns:

--*/
{
  EFI_STATUS                                Status;
  EFI_BUS_SPECIFIC_DRIVER_OVERRIDE_PROTOCOL *BusSpecificDriverOverride;
  EFI_HANDLE                                ImageHandle;
  EFI_LOADED_IMAGE_PROTOCOL                 *Image;

  Print (L"\n");
  BusSpecificDriverOverride = Interface;
  do {
    Status = BusSpecificDriverOverride->GetDriver (
                                          BusSpecificDriverOverride,
                                          &ImageHandle
                                          );
    if (!EFI_ERROR (Status)) {
      Status = BS->HandleProtocol (
                    ImageHandle,
                    &gEfiLoadedImageProtocolGuid,
                    (VOID **) &Image
                    );
      if (!EFI_ERROR (Status)) {
        PrintToken (
          STRING_TOKEN (STR_SHELLENV_DPROT_DRV_FILE),
          HiiEnvHandle,
          SEnvHandleToNumber (ImageHandle),
          LibDevicePathToStr (Image->FilePath)
          );
      }
    }
  } while (!EFI_ERROR (Status));
}

VOID
EFIAPI
SEnvGraphicsOutput (
  IN EFI_HANDLE      h,
  IN VOID            *Interface
  )
{
  EFI_GRAPHICS_OUTPUT_PROTOCOL           *GraphicsOutput;
  EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE      *Mode;
  EFI_GRAPHICS_OUTPUT_MODE_INFORMATION   *Info;

  GraphicsOutput = (EFI_GRAPHICS_OUTPUT_PROTOCOL *) Interface;
  Mode           = GraphicsOutput->Mode;
  Info           = Mode->Info;

  //
  // Dump GraphicsOutput Info:
  // HorizontalResolution
  // VerticalResolution
  // PixelFormat
  // PixelInformation
  // PixelPerScanLine
  // FrameBufferBase
  // FrameBufferSize
  //
  PrintToken (
    STRING_TOKEN (STR_SHELLENV_DPROT_HRESOL),
    HiiEnvHandle,
    Info->HorizontalResolution
    );
  PrintToken (
    STRING_TOKEN (STR_SHELLENV_DPROT_VRESOL),
    HiiEnvHandle,
    Info->VerticalResolution
    );
  PrintToken (
    STRING_TOKEN (STR_SHELLENV_DPROT_PIXELFORMAT),
    HiiEnvHandle
    );
  switch (Info->PixelFormat) {
  case PixelRedGreenBlueReserved8BitPerColor:
    Print (L"PixelRedGreenBlueReserved8BitPerColor");
    break;
  case PixelBlueGreenRedReserved8BitPerColor:
    Print (L"PixelBlueGreenRedReserved8BitPerColor");
    break;
  case PixelBitMask:
    Print (L"PixelBitMask");
    PrintToken (
      STRING_TOKEN (STR_SHELLENV_DPROT_PIXELINFORMATION),
      HiiEnvHandle,
      Info->PixelInformation.RedMask,
      Info->PixelInformation.GreenMask,
      Info->PixelInformation.BlueMask,
      Info->PixelInformation.ReservedMask
      );
    break;
  case PixelBltOnly:
    Print (L"PixelBltOnly");
    break;
  default:
    Print (L"Unknown");
    break;
  }
  PrintToken (
    STRING_TOKEN (STR_SHELLENV_DPROT_PIXELSPERSCANLINE),
    HiiEnvHandle,
    Info->PixelsPerScanLine
    );
  PrintToken (
    STRING_TOKEN (STR_SHELLENV_DPROT_FRAMEBUFFERBASE),
    HiiEnvHandle,
    Mode->FrameBufferBase
    );
  PrintToken (
    STRING_TOKEN (STR_SHELLENV_DPROT_FRAMEBUFFERSIZE),
    HiiEnvHandle,
    Mode->FrameBufferSize
    );
}
