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
    Device Path functions

Revision History

--*/

#include "EfiShellLib.h"

EFI_GUID mEfiDevicePathMessagingUartFlowControlGuid = DEVICE_PATH_MESSAGING_UART_FLOW_CONTROL;

#if (EFI_SPECIFICATION_VERSION >= 0x00020000)
EFI_GUID mEfiDevicePathMessagingSASGuid = DEVICE_PATH_MESSAGING_SAS;
#endif

EFI_DEVICE_PATH_PROTOCOL *
DevicePathInstance (
  IN OUT EFI_DEVICE_PATH_PROTOCOL             **DevicePath,
  OUT UINTN                                   *Size
  )
/*++

Routine Description:
  Function retrieves the next device path instance from a device path data structure.

Arguments:
  DevicePath           - A pointer to a device path data structure.

  Size                 - A pointer to the size of a device path instance in bytes.

Returns:

  This function returns a pointer to the current device path instance.  
  In addition, it returns the size in bytes of the current device path instance in Size, 
  and a pointer to the next device path instance in DevicePath.  
  If there are no more device path instances in DevicePath, then DevicePath will be set to NULL.

--*/
{
  EFI_DEVICE_PATH_PROTOCOL  *Start;
  EFI_DEVICE_PATH_PROTOCOL  *Next;
  EFI_DEVICE_PATH_PROTOCOL  *DevPath;
  UINTN                     Count;

  ASSERT (Size);

  DevPath = *DevicePath;
  Start   = DevPath;

  if (!DevPath) {
    return NULL;
  }
  //
  // Check for end of device path type
  //
  for (Count = 0;; Count++) {
    Next = (EFI_DEVICE_PATH_PROTOCOL *) NextDevicePathNode (DevPath);

    if (IsDevicePathEndType (DevPath)) {
      break;
    }

    if (Count > 01000) {
      DEBUG (
        (
        EFI_D_ERROR, "DevicePathInstance Too Long : DevicePath %x Size %d", *DevicePath, ((UINT8 *) DevPath) -
        ((UINT8 *) Start)
        )
        );
      DumpHex (0, 0, ((UINT8 *) DevPath) - ((UINT8 *) Start), Start);
      break;
    }

    DevPath = Next;
  }

  ASSERT (DevicePathSubType (DevPath) == END_ENTIRE_DEVICE_PATH_SUBTYPE || DevicePathSubType (DevPath) == END_INSTANCE_DEVICE_PATH_SUBTYPE);

  //
  // Set next position
  //
  if (DevicePathSubType (DevPath) == END_ENTIRE_DEVICE_PATH_SUBTYPE) {
    Next = NULL;
  }

  *DevicePath = Next;

  //
  // Return size and start of device path instance
  //
  *Size = ((UINT8 *) DevPath) - ((UINT8 *) Start) + sizeof (EFI_DEVICE_PATH_PROTOCOL);
  return Start;
}

UINTN
DevicePathInstanceCount (
  IN EFI_DEVICE_PATH_PROTOCOL      *DevicePath
  )
/*++

Routine Description:
  Function is used to determine the number of device path instances that exist in a device path.

Arguments:
  DevicePath           - A pointer to a device path data structure.

Returns:

  This function counts and returns the number of device path instances in DevicePath.

--*/
{
  UINTN Count;
  UINTN Size;

  Count = 0;
  while (DevicePathInstance (&DevicePath, &Size)) {
    Count += 1;
  }

  return Count;
}

EFI_DEVICE_PATH_PROTOCOL *
AppendDevicePath (
  IN EFI_DEVICE_PATH_PROTOCOL  *Src1,
  IN EFI_DEVICE_PATH_PROTOCOL  *Src2
  )
/*++

Routine Description:
  Function is used to append a device path to all the instances in another device path.

Arguments:
  Src1           - A pointer to a device path data structure.

  Src2           - A pointer to a device path data structure.

Returns:

  A pointer to the new device path is returned.  
  NULL is returned if space for the new device path could not be allocated from pool. 
  It is up to the caller to free the memory used by Src1 and Src2 if they are no longer needed.

  Src1 may have multiple "instances" and each instance is appended
  Src2 is appended to each instance is Src1.  (E.g., it's possible
  to append a new instance to the complete device path by passing 
  it in Src2)

--*/
{
  UINTN                     Src1Size;
  UINTN                     Src1Inst;
  UINTN                     Src2Size;
  UINTN                     Size;
  EFI_DEVICE_PATH_PROTOCOL  *Dst;
  EFI_DEVICE_PATH_PROTOCOL  *Inst;
  UINT8                     *DstPos;

  //
  // If there's only 1 path, just duplicate it
  //
  if (!Src1) {
    ASSERT (!IsDevicePathUnpacked (Src2));
    return DuplicateDevicePath (Src2);
  }

  if (!Src2) {
    ASSERT (!IsDevicePathUnpacked (Src1));
    return DuplicateDevicePath (Src1);
  }
  //
  // Verify we're not working with unpacked paths
  //
  //
  // Append Src2 to every instance in Src1
  //
  Src1Size  = DevicePathSize (Src1);
  Src1Inst  = DevicePathInstanceCount (Src1);
  Src2Size  = DevicePathSize (Src2);
  Size      = Src2Size * Src1Inst + Src1Size;
  Size -= Src1Inst * sizeof (EFI_DEVICE_PATH_PROTOCOL);

  Dst = AllocatePool (Size);
  if (Dst) {
    DstPos = (UINT8 *) Dst;

    //
    // Copy all device path instances
    //
    Inst = DevicePathInstance (&Src1, &Size);
    while (Inst) {

      CopyMem (DstPos, Inst, Size);
      DstPos += Size - sizeof (EFI_DEVICE_PATH_PROTOCOL);

      CopyMem (DstPos, Src2, Src2Size);
      DstPos += Src2Size - sizeof (EFI_DEVICE_PATH_PROTOCOL);

      SetDevicePathEndNode ((EFI_DEVICE_PATH_PROTOCOL *)DstPos);
      ((EFI_DEVICE_PATH_PROTOCOL *)DstPos)->SubType = END_INSTANCE_DEVICE_PATH_SUBTYPE; 
      DstPos += sizeof (EFI_DEVICE_PATH_PROTOCOL);

      Inst = DevicePathInstance (&Src1, &Size);
    }
    //
    // Change last end marker
    //
    DstPos -= sizeof (EFI_DEVICE_PATH_PROTOCOL);
    SetDevicePathEndNode ((EFI_DEVICE_PATH_PROTOCOL *)DstPos);
  }

  return Dst;
}

UINTN
DevicePathSize (
  IN EFI_DEVICE_PATH_PROTOCOL  *DevPath
  )
/*++

Routine Description:
  Function returns the size of a device path in bytes.

Arguments:
  DevPath        - A pointer to a device path data structure

Returns:

  Size is returned.

--*/
{
  EFI_DEVICE_PATH_PROTOCOL  *Start;

  ASSERT (DevPath != NULL);

  //
  // Search for the end of the device path structure
  //
  Start = DevPath;
  while (!IsDevicePathEnd (DevPath)) {
    DevPath = (EFI_DEVICE_PATH_PROTOCOL *) NextDevicePathNode (DevPath);
  }
  //
  // Compute the size
  //
  return ((UINTN) DevPath - (UINTN) Start) + sizeof (EFI_DEVICE_PATH_PROTOCOL);
}

EFI_DEVICE_PATH_PROTOCOL *
DevicePathFromHandle (
  IN EFI_HANDLE       Handle
  )
/*++

Routine Description:
  Function retrieves the device path for the specified handle.  

Arguments:
  Handle           - Handle of the device

Returns:

  If Handle is valid, then a pointer to the device path is returned.  
  If Handle is not valid, then NULL is returned.

--*/
{
  EFI_STATUS                Status;
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath;

  Status = BS->HandleProtocol (
                Handle,
                &gEfiDevicePathProtocolGuid,
                (VOID *) &DevicePath
                );

  if (EFI_ERROR (Status)) {
    DevicePath = NULL;
  }

  return DevicePath;
}

EFI_DEVICE_PATH_PROTOCOL *
DuplicateDevicePath (
  IN EFI_DEVICE_PATH_PROTOCOL  *DevPath
  )
/*++

Routine Description:
  Function creates a duplicate copy of an existing device path.

Arguments:
  DevPath        - A pointer to a device path data structure

Returns:

  If the memory is successfully allocated, then the contents of DevPath are copied 
  to the newly allocated buffer, and a pointer to that buffer is returned.  
  Otherwise, NULL is returned.

--*/
{
  EFI_DEVICE_PATH_PROTOCOL  *NewDevPath;
  UINTN                     Size;

  ASSERT (DevPath);

  //
  // Compute the size
  //
  Size = DevicePathSize (DevPath);

  //
  // Make a copy
  //
  NewDevPath = AllocatePool (Size);
  if (NewDevPath != NULL) {
    CopyMem (NewDevPath, DevPath, Size);
  }

  return NewDevPath;
}

EFI_DEVICE_PATH_PROTOCOL *
UnpackDevicePath (
  IN EFI_DEVICE_PATH_PROTOCOL  *DevPath
  )
/*++

Routine Description:
  Function unpacks a device path data structure so that all the nodes of a device path 
  are naturally aligned.

Arguments:
  DevPath        - A pointer to a device path data structure

Returns:

  If the memory for the device path is successfully allocated, then a pointer to the 
  new device path is returned.  Otherwise, NULL is returned.

--*/
{
  EFI_DEVICE_PATH_PROTOCOL  *Src;
  EFI_DEVICE_PATH_PROTOCOL  *Dest;
  EFI_DEVICE_PATH_PROTOCOL  *NewPath;
  UINTN                     Size;

  if (DevPath == NULL) {
    return NULL;
  }
  //
  // Walk device path and round sizes to valid boundries
  //
  Src   = DevPath;
  Size  = 0;
  for (;;) {
    Size += DevicePathNodeLength (Src);
    Size += ALIGN_SIZE (Size);

    if (IsDevicePathEnd (Src)) {
      break;
    }

    Src = (EFI_DEVICE_PATH_PROTOCOL *) NextDevicePathNode (Src);
  }
  //
  // Allocate space for the unpacked path
  //
  NewPath = AllocateZeroPool (Size);
  if (NewPath != NULL) {

    ASSERT (((UINTN) NewPath) % MIN_ALIGNMENT_SIZE == 0);

    //
    // Copy each node
    //
    Src   = DevPath;
    Dest  = NewPath;
    for (;;) {
      Size = DevicePathNodeLength (Src);
      CopyMem (Dest, Src, Size);
      Size += ALIGN_SIZE (Size);
      SetDevicePathNodeLength (Dest, Size);
      Dest->Type |= EFI_DP_TYPE_UNPACKED;
      Dest = (EFI_DEVICE_PATH_PROTOCOL *) (((UINT8 *) Dest) + Size);

      if (IsDevicePathEnd (Src)) {
        break;
      }

      Src = (EFI_DEVICE_PATH_PROTOCOL *) NextDevicePathNode (Src);
    }
  }

  return NewPath;
}

VOID
_DevPathPci (
  IN OUT POOL_PRINT       *Str,
  IN VOID                 *DevPath
  )
{
  PCI_DEVICE_PATH *Pci;

  ASSERT (Str != NULL);
  ASSERT (DevPath != NULL);

  Pci = DevPath;
  CatPrint (Str, L"Pci(%x|%x)", (UINTN) Pci->Device, (UINTN) Pci->Function);
}

VOID
_DevPathPccard (
  IN OUT POOL_PRINT       *Str,
  IN VOID                 *DevPath
  )
{
  PCCARD_DEVICE_PATH  *Pccard;

  ASSERT (Str != NULL);
  ASSERT (DevPath != NULL);

  Pccard = DevPath;
  CatPrint (Str, L"Pccard(Function%x)", (UINTN) Pccard->FunctionNumber);
}

VOID
_DevPathMemMap (
  IN OUT POOL_PRINT       *Str,
  IN VOID                 *DevPath
  )
{
  MEMMAP_DEVICE_PATH  *MemMap;

  ASSERT (Str != NULL);
  ASSERT (DevPath != NULL);

  MemMap = DevPath;
  CatPrint (
    Str,
    L"MemMap(%d:%lx-%lx)",
    (UINTN) MemMap->MemoryType,
    MemMap->StartingAddress,
    MemMap->EndingAddress
    );
}

VOID
_DevPathController (
  IN OUT POOL_PRINT       *Str,
  IN VOID                 *DevPath
  )
{
  CONTROLLER_DEVICE_PATH  *Controller;

  ASSERT (Str != NULL);
  ASSERT (DevPath != NULL);

  Controller = DevPath;
  CatPrint (
    Str,
    L"Ctrl(%d)",
    (UINTN) Controller->Controller
    );
}

VOID
_DevPathVendor (
  IN OUT POOL_PRINT       *Str,
  IN VOID                 *DevPath
  )
{
  VENDOR_DEVICE_PATH  *Vendor;
  CHAR16              *Type;
  UINTN               DataLength;
  UINTN               Index;
  UINT32              FlowControlMap;
#if (EFI_SPECIFICATION_VERSION >= 0x00020000)
  UINT16              Info;
#endif

  ASSERT (Str != NULL);
  ASSERT (DevPath != NULL);

  Vendor = DevPath;
  switch (DevicePathType (&Vendor->Header)) {
  case HARDWARE_DEVICE_PATH:
    Type = L"Hw";
    break;

  case MESSAGING_DEVICE_PATH:
    Type = L"Msg";
    if (CompareGuid (&Vendor->Guid, &gEfiPcAnsiGuid) == 0) {
      CatPrint (Str, L"VenPcAnsi()");
      return ;
    } else if (CompareGuid (&Vendor->Guid, &gEfiVT100Guid) == 0) {
      CatPrint (Str, L"VenVt100()");
      return ;
    } else if (CompareGuid (&Vendor->Guid, &gEfiVT100PlusGuid) == 0) {
      CatPrint (Str, L"VenVt100Plus()");
      return ;
    } else if (CompareGuid (&Vendor->Guid, &gEfiVTUTF8Guid) == 0) {
      CatPrint (Str, L"VenUft8()");
      return ;
    } else if (CompareGuid (&Vendor->Guid, &mEfiDevicePathMessagingUartFlowControlGuid) == 0) {
      FlowControlMap = (((UART_FLOW_CONTROL_DEVICE_PATH *) Vendor)->FlowControlMap);
      switch (FlowControlMap & 0x00000003) {
      case 0:
        CatPrint (Str, L"UartFlowCtrl(%s)", L"None");
        break;

      case 1:
        CatPrint (Str, L"UartFlowCtrl(%s)", L"Hardware");
        break;

      case 2:
        CatPrint (Str, L"UartFlowCtrl(%s)", L"XonXoff");
        break;

      default:
        break;
      }

      return ;
#if (EFI_SPECIFICATION_VERSION >= 0x00020000)
    } else if (CompareGuid (&Vendor->Guid, &mEfiDevicePathMessagingSASGuid) == 0) {
      CatPrint (
        Str,
        L"SAS(%lx,%lx,%x,",
        ((SAS_DEVICE_PATH *) Vendor)->SasAddress,
        ((SAS_DEVICE_PATH *) Vendor)->Lun,
        ((SAS_DEVICE_PATH *) Vendor)->RelativeTargetPort
        );
      Info = (((SAS_DEVICE_PATH *) Vendor)->DeviceTopology);
      if ((Info & 0x0f) == 0) {
        CatPrint (Str, L"NoTopology,0,0,0,");
      } else if (((Info & 0x0f) == 1) || ((Info & 0x0f) == 2)) {
        CatPrint (
          Str,
          L"%s,%s,%s,",
          (Info & (0x1 << 4)) ? L"SATA" : L"SAS",
          (Info & (0x1 << 5)) ? L"External" : L"Internal",
          (Info & (0x1 << 6)) ? L"Expanded" : L"Direct"
          );
        if ((Info & 0x0f) == 1) {
          CatPrint (Str, L"0,");
        } else {
          CatPrint (Str, L"%x,", (UINTN) ((Info >> 8) & 0xff));
        }
      } else {
        CatPrint (Str, L"0,0,0,0,");
      }

      CatPrint (Str, L"%x)", (UINTN) ((SAS_DEVICE_PATH *) Vendor)->Reserved);
      return ;
#endif
    } else if (CompareGuid (&Vendor->Guid, &gEfiDebugPortProtocolGuid) == 0) {
      CatPrint (Str, L"DebugPort()");
      return ;
    }
    break;

  case MEDIA_DEVICE_PATH:
    Type = L"Media";
    break;

  default:
    Type = L"?";
    break;
  }

  CatPrint (Str, L"Ven%s(%g", Type, &Vendor->Guid);
  DataLength = DevicePathNodeLength (&Vendor->Header) - sizeof (VENDOR_DEVICE_PATH);
  if (DataLength > 0) {
    CatPrint (Str, L",");
    for (Index = 0; Index < DataLength; Index++) {
      CatPrint (Str, L"%02x", (UINTN) ((VENDOR_DEVICE_PATH_WITH_DATA *) Vendor)->VendorDefinedData[Index]);
    }
  }
  CatPrint (Str, L")");
}

VOID
_DevPathAcpi (
  IN OUT POOL_PRINT       *Str,
  IN VOID                 *DevPath
  )
{
  ACPI_HID_DEVICE_PATH  *Acpi;

  ASSERT (Str != NULL);
  ASSERT (DevPath != NULL);

  Acpi = DevPath;
  if ((Acpi->HID & PNP_EISA_ID_MASK) == PNP_EISA_ID_CONST) {
    CatPrint (Str, L"Acpi(PNP%04x,%x)", (UINTN) EISA_ID_TO_NUM (Acpi->HID), (UINTN) Acpi->UID);
  } else {
    CatPrint (Str, L"Acpi(%08x,%x)", (UINTN) Acpi->HID, (UINTN) Acpi->UID);
  }
}

VOID
_DevPathExtendedAcpi (
  IN OUT POOL_PRINT       *Str,
  IN VOID                 *DevPath
  )
{
  ACPI_EXTENDED_HID_DEVICE_PATH_WITH_STR   *ExtendedAcpi;
  //
  // HID, UID and CID strings
  //
  CHAR8     *HIDString;
  CHAR8     *UIDString;
  CHAR8     *CIDString;


  ASSERT (Str != NULL);
  ASSERT (DevPath != NULL);
  ASSERT (DevicePathNodeLength ((EFI_DEVICE_PATH_PROTOCOL *) DevPath) >=
          sizeof (ACPI_EXTENDED_HID_DEVICE_PATH_WITH_STR));

  ExtendedAcpi = (ACPI_EXTENDED_HID_DEVICE_PATH_WITH_STR *) DevPath;

  HIDString = ExtendedAcpi->HidUidCidStr;
  UIDString = NextStrA (HIDString);
  CIDString = NextStrA (UIDString);

  CatPrint (Str, L"AcpiEx(");
  if ((ExtendedAcpi->HID & PNP_EISA_ID_MASK) == PNP_EISA_ID_CONST) {
    CatPrint (Str, L"PNP%04x,", (UINTN) EISA_ID_TO_NUM (ExtendedAcpi->HID));
  } else {
    CatPrint (Str, L"%08x,", (UINTN) ExtendedAcpi->HID);
  }
  if ((ExtendedAcpi->CID & PNP_EISA_ID_MASK) == PNP_EISA_ID_CONST) {
    CatPrint (Str, L"PNP%04x,", (UINTN) EISA_ID_TO_NUM (ExtendedAcpi->CID));
  } else {
    CatPrint (Str, L"%08x,", (UINTN) ExtendedAcpi->CID);
  }
  CatPrint (Str, L"%x,", (UINTN) ExtendedAcpi->UID);

  if (*HIDString != '\0') {
    CatPrint (Str, L"%a,", HIDString);
  } else {
    CatPrint (Str, L"NULL,");
  }
  if (*CIDString != '\0') {
    CatPrint (Str, L"%a,", CIDString);
  } else {
    CatPrint (Str, L"NULL,");
  }
  if (*UIDString != '\0') {
    CatPrint (Str, L"%a)", UIDString);
  } else {
    CatPrint (Str, L"NULL)");
  }
}

VOID
_DevPathAdrAcpi (
  IN OUT POOL_PRINT       *Str,
  IN VOID                 *DevPath
  )
{
  ACPI_ADR_DEVICE_PATH    *AcpiAdr;
  UINT16                  Index;
  UINT16                  Length;
  UINT16                  AdditionalAdrCount;

  AcpiAdr            = DevPath;
  Length             = DevicePathNodeLength ((EFI_DEVICE_PATH_PROTOCOL *) AcpiAdr);
  AdditionalAdrCount = (Length - 8) / 4;

  CatPrint (Str, L"AcpiAdr(%x", (UINTN) AcpiAdr->ADR);
  for (Index = 0; Index < AdditionalAdrCount; Index++) {
    CatPrint (Str, L",%x", (UINTN) *(UINT32 *) ((UINT8 *) AcpiAdr + 8 + Index * 4));
  }
  CatPrint (Str, L")");
}

VOID
_DevPathAtapi (
  IN OUT POOL_PRINT       *Str,
  IN VOID                 *DevPath
  )
{
  ATAPI_DEVICE_PATH *Atapi;

  ASSERT (Str != NULL);
  ASSERT (DevPath != NULL);

  Atapi = DevPath;
  CatPrint (
    Str,
    L"Ata(%s,%s)",
    Atapi->PrimarySecondary ? L"Secondary" : L"Primary",
    Atapi->SlaveMaster ? L"Slave" : L"Master"
    );
}

VOID
_DevPathScsi (
  IN OUT POOL_PRINT       *Str,
  IN VOID                 *DevPath
  )
{
  SCSI_DEVICE_PATH  *Scsi;

  ASSERT (Str != NULL);
  ASSERT (DevPath != NULL);

  Scsi = DevPath;
  CatPrint (Str, L"Scsi(Pun%x,Lun%x)", (UINTN) Scsi->Pun, (UINTN) Scsi->Lun);
}

VOID
_DevPathFibre (
  IN OUT POOL_PRINT       *Str,
  IN VOID                 *DevPath
  )
{
  FIBRECHANNEL_DEVICE_PATH  *Fibre;

  ASSERT (Str != NULL);
  ASSERT (DevPath != NULL);

  Fibre = DevPath;
  CatPrint (Str, L"Fibre(Wwn%lx,Lun%lx)", Fibre->WWN, Fibre->Lun);
}

VOID
_DevPath1394 (
  IN OUT POOL_PRINT       *Str,
  IN VOID                 *DevPath
  )
{
  F1394_DEVICE_PATH *F1394;
  
  ASSERT (Str != NULL);
  ASSERT (DevPath != NULL);

  F1394 = DevPath;
  CatPrint (Str, L"1394(%lx)", &F1394->Guid);
}

VOID
_DevPathUsb (
  IN OUT POOL_PRINT       *Str,
  IN VOID                 *DevPath
  )
{
  USB_DEVICE_PATH *Usb;

  ASSERT (Str != NULL);
  ASSERT (DevPath != NULL);

  Usb = DevPath;
  CatPrint (Str, L"Usb(%x,%x)", (UINTN) Usb->ParentPortNumber, (UINTN) Usb->InterfaceNumber);
}

#if (EFI_SPECIFICATION_VERSION >= 0x00020000)
void
_DevPathUsbWWID (
  IN OUT POOL_PRINT       *Str,
  IN VOID                 *DevPath
  )
{
  USB_WWID_DEVICE_PATH  *UsbWWId;

  ASSERT (Str != NULL);
  ASSERT (DevPath != NULL);

  UsbWWId = DevPath;
  CatPrint (
    Str,
    L"UsbWwid(%x,%x,%x,\"WWID\")",
    (UINTN) UsbWWId->VendorId,
    (UINTN) UsbWWId->ProductId,
    (UINTN) UsbWWId->InterfaceNumber
    );
}

void
_DevPathLogicalUnit (
  IN OUT POOL_PRINT       *Str,
  IN VOID                 *DevPath
  )
{
  DEVICE_LOGICAL_UNIT_DEVICE_PATH *LogicalUnit;

  ASSERT (Str != NULL);
  ASSERT (DevPath != NULL);

  LogicalUnit = DevPath;
  CatPrint (Str, L"Unit(%x)", (UINTN) LogicalUnit->Lun);
}
#endif

VOID
_DevPathUsbClass (
  IN OUT POOL_PRINT       *Str,
  IN VOID                 *DevPath
  )
{
  //
  // tbd:
  //
  USB_CLASS_DEVICE_PATH *UsbClass;

  ASSERT (Str != NULL);
  ASSERT (DevPath != NULL);

  UsbClass = DevPath;
  CatPrint (
    Str,
    L"Usb Class(%x,%x,%x,%x,%x)",
    (UINTN) UsbClass->VendorId,
    (UINTN) UsbClass->ProductId,
    (UINTN) UsbClass->DeviceClass,
    (UINTN) UsbClass->DeviceSubClass,
    (UINTN) UsbClass->DeviceProtocol
    );
}

VOID
_DevPathI2O (
  IN OUT POOL_PRINT       *Str,
  IN VOID                 *DevPath
  )
{
  I2O_DEVICE_PATH *I2O;

  ASSERT (Str != NULL);
  ASSERT (DevPath != NULL);

  I2O = DevPath;
  CatPrint (Str, L"I2O(%x)", (UINTN) I2O->Tid);
}

VOID
_DevPathMacAddr (
  IN OUT POOL_PRINT       *Str,
  IN VOID                 *DevPath
  )
{
  MAC_ADDR_DEVICE_PATH  *MAC;
  UINTN                 HwAddressSize;
  UINTN                 Index;

  ASSERT (Str != NULL);
  ASSERT (DevPath != NULL);

  MAC           = DevPath;

  HwAddressSize = sizeof (EFI_MAC_ADDRESS);
  if (MAC->IfType == 0x01 || MAC->IfType == 0x00) {
    HwAddressSize = 6;
  }

  CatPrint (Str, L"Mac(");

  for (Index = 0; Index < HwAddressSize; Index++) {
    CatPrint (Str, L"%02x", (UINTN) MAC->MacAddress.Addr[Index]);
  }

  CatPrint (Str, L")");
}

VOID
_DevPathIPv4 (
  IN OUT POOL_PRINT       *Str,
  IN VOID                 *DevPath
  )
{
  IPv4_DEVICE_PATH  *IP;

  ASSERT (Str != NULL);
  ASSERT (DevPath != NULL);

  IP = DevPath;
  CatPrint (
    Str,
    L"IPv4(%d.%d.%d.%d:%d)",
    (UINTN) IP->RemoteIpAddress.Addr[0],
    (UINTN) IP->RemoteIpAddress.Addr[1],
    (UINTN) IP->RemoteIpAddress.Addr[2],
    (UINTN) IP->RemoteIpAddress.Addr[3],
    (UINTN) IP->RemotePort
    );
}

VOID
_DevPathIPv6 (
  IN OUT POOL_PRINT       *Str,
  IN VOID                 *DevPath
  )
{
  IPv6_DEVICE_PATH  *IP;

  ASSERT (Str != NULL);

  IP = DevPath;
  CatPrint (
    Str,
    L"IPv6(%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x)",
    (UINTN) IP->RemoteIpAddress.Addr[0],
    (UINTN) IP->RemoteIpAddress.Addr[1],
    (UINTN) IP->RemoteIpAddress.Addr[2],
    (UINTN) IP->RemoteIpAddress.Addr[3],
    (UINTN) IP->RemoteIpAddress.Addr[4],
    (UINTN) IP->RemoteIpAddress.Addr[5],
    (UINTN) IP->RemoteIpAddress.Addr[6],
    (UINTN) IP->RemoteIpAddress.Addr[7],
    (UINTN) IP->RemoteIpAddress.Addr[8],
    (UINTN) IP->RemoteIpAddress.Addr[9],
    (UINTN) IP->RemoteIpAddress.Addr[10],
    (UINTN) IP->RemoteIpAddress.Addr[11],
    (UINTN) IP->RemoteIpAddress.Addr[12],
    (UINTN) IP->RemoteIpAddress.Addr[13],
    (UINTN) IP->RemoteIpAddress.Addr[14],
    (UINTN) IP->RemoteIpAddress.Addr[15]
    );
}

VOID
_DevPathInfiniBand (
  IN OUT POOL_PRINT       *Str,
  IN VOID                 *DevPath
  )
{
  INFINIBAND_DEVICE_PATH  *InfiniBand;

  ASSERT (Str != NULL);

  InfiniBand = DevPath;
  CatPrint (
    Str,
    L"Infiniband(%x,%g,%lx,%lx,%lx)",
    (UINTN) InfiniBand->ResourceFlags,
    InfiniBand->PortGid,
    InfiniBand->ServiceId,
    InfiniBand->TargetPortId,
    InfiniBand->DeviceId
    );
}

VOID
_DevPathUart (
  IN OUT POOL_PRINT       *Str,
  IN VOID                 *DevPath
  )
{
  UART_DEVICE_PATH  *Uart;
  CHAR8             Parity;

  ASSERT (Str != NULL);
  ASSERT (DevPath != NULL);

  Uart = DevPath;
  switch (Uart->Parity) {
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

  if (Uart->BaudRate == 0) {
    CatPrint (Str, L"Uart(DEFAULT,%c,", Parity);
  } else {
    CatPrint (Str, L"Uart(%ld,%c,", Uart->BaudRate, Parity);
  }

  if (Uart->DataBits == 0) {
    CatPrint (Str, L"D,");
  } else {
    CatPrint (Str, L"%d,", (UINTN) Uart->DataBits);
  }

  switch (Uart->StopBits) {
  case 0:
    CatPrint (Str, L"D)");
    break;

  case 1:
    CatPrint (Str, L"1)");
    break;

  case 2:
    CatPrint (Str, L"1.5)");
    break;

  case 3:
    CatPrint (Str, L"2)");
    break;

  default:
    CatPrint (Str, L"x)");
    break;
  }
}

#if (EFI_SPECIFICATION_VERSION >= 0x00020000)
VOID
_DevPathiSCSI (
  IN OUT POOL_PRINT       *Str,
  IN VOID                 *DevPath
  )
{
  ISCSI_DEVICE_PATH_WITH_NAME *iSCSI;
  UINT16                      Options;

  ASSERT (Str != NULL);
  ASSERT (DevPath != NULL);

  iSCSI = DevPath;
  CatPrint (
    Str,
    L"iSCSI(%s,%x,%lx,",
    iSCSI->iSCSITargetName,
    (UINTN) iSCSI->TargetPortalGroupTag,
    iSCSI->Lun
    );

  Options = iSCSI->LoginOption;
  CatPrint (Str, L"%s,", ((Options >> 1) & 0x0001) ? L"CRC32C" : L"None");
  CatPrint (Str, L"%s,", ((Options >> 3) & 0x0001) ? L"CRC32C" : L"None");
  if ((Options >> 11) & 0x0001) {
    CatPrint (Str, L"%s,", L"None");
  } else if ((Options >> 12) & 0x0001) {
    CatPrint (Str, L"%s,", L"CHAP_UNI");
  } else {
    CatPrint (Str, L"%s,", L"CHAP_BI");

  }

  CatPrint (Str, L"%s)", (iSCSI->NetworkProtocol == 0) ? L"TCP" : L"reserved");
}
#endif

VOID
_DevPathHardDrive (
  IN OUT POOL_PRINT       *Str,
  IN VOID                 *DevPath
  )
{
  HARDDRIVE_DEVICE_PATH *Hd;

  ASSERT (Str != NULL);
  ASSERT (DevPath != NULL);

  Hd = DevPath;
  switch (Hd->SignatureType) {
  case SIGNATURE_TYPE_MBR:
    CatPrint (
      Str,
      L"HD(Part%d,Sig%08x)",
      (UINTN) Hd->PartitionNumber,
      (UINTN) *((UINT32 *) (&(Hd->Signature[0])))
      );
    break;

  case SIGNATURE_TYPE_GUID:
    CatPrint (
      Str,
      L"HD(Part%d,Sig%g)",
      (UINTN) Hd->PartitionNumber,
      (EFI_GUID *) &(Hd->Signature[0])
      );
    break;

  default:
    CatPrint (
      Str,
      L"HD(Part%d,MBRType=%02x,SigType=%02x)",
      (UINTN) Hd->PartitionNumber,
      (UINTN) Hd->MBRType,
      (UINTN) Hd->SignatureType
      );
    break;
  }
}

VOID
_DevPathCDROM (
  IN OUT POOL_PRINT       *Str,
  IN VOID                 *DevPath
  )
{
  CDROM_DEVICE_PATH *Cd;

  ASSERT (Str != NULL);
  ASSERT (DevPath != NULL);

  Cd = DevPath;
  CatPrint (Str, L"CDROM(Entry%x)", (UINTN) Cd->BootEntry);
}

VOID
_DevPathFilePath (
  IN OUT POOL_PRINT       *Str,
  IN VOID                 *DevPath
  )
{
  FILEPATH_DEVICE_PATH  *Fp;
  UINTN                 Length;
  CHAR16                *NewPath;

  ASSERT (Str != NULL);
  ASSERT (DevPath != NULL);

  Fp      = DevPath;
  Length  = EfiDevicePathNodeLength (((EFI_DEVICE_PATH_PROTOCOL *) DevPath)) - 4;
  NewPath = AllocateZeroPool (Length + sizeof (CHAR16));
  CopyMem (NewPath, Fp->PathName, Length);
  StrTrim (NewPath, L' ');
  CatPrint (Str, L"%s", NewPath);
  FreePool (NewPath);
}

VOID
_DevPathMediaProtocol (
  IN OUT POOL_PRINT       *Str,
  IN VOID                 *DevPath
  )
{
  MEDIA_PROTOCOL_DEVICE_PATH  *MediaProt;

  ASSERT (Str != NULL);
  ASSERT (DevPath != NULL);

  MediaProt = DevPath;
  CatPrint (Str, L"%g", &MediaProt->Protocol);
}

VOID
_DevPathFvFilePath (
  IN OUT POOL_PRINT       *Str,
  IN VOID                 *DevPath
  )
{
  MEDIA_FW_VOL_FILEPATH_DEVICE_PATH *FvFilePath;

  ASSERT (Str != NULL);
  ASSERT (DevPath != NULL);

  FvFilePath = DevPath;
  CatPrint (Str, L"%g", &FvFilePath->NameGuid);
}

VOID
_DevPathBssBss (
  IN OUT POOL_PRINT       *Str,
  IN VOID                 *DevPath
  )
{
  BBS_BBS_DEVICE_PATH *BBS;
  CHAR16              *Type;

  ASSERT (Str != NULL);
  ASSERT (DevPath != NULL);

  BBS = DevPath;
  switch (BBS->DeviceType) {
  case BBS_TYPE_FLOPPY:
    Type = L"Floppy";
    break;

  case BBS_TYPE_HARDDRIVE:
    Type = L"Harddrive";
    break;

  case BBS_TYPE_CDROM:
    Type = L"CDROM";
    break;

  case BBS_TYPE_PCMCIA:
    Type = L"PCMCIA";
    break;

  case BBS_TYPE_USB:
    Type = L"Usb";
    break;

  case BBS_TYPE_EMBEDDED_NETWORK:
    Type = L"Net";
    break;

  default:
    Type = L"?";
    break;
  }

  CatPrint (Str, L"BBS-%s(%a)", Type, BBS->String);
}

VOID
_DevPathEndInstance (
  IN OUT POOL_PRINT       *Str,
  IN VOID                 *DevPath
  )
{
  ASSERT (Str != NULL);

  CatPrint (Str, L",");
}

VOID
_DevPathNodeUnknown (
  IN OUT POOL_PRINT       *Str,
  IN VOID                 *DevPath
  )
{
  ASSERT (Str != NULL);

  CatPrint (Str, L"?");
}

struct {
  UINT8 Type;
  UINT8 SubType;
  VOID (*Function) (POOL_PRINT *, VOID *);
} 
DevPathTable[] = {
  HARDWARE_DEVICE_PATH,
  HW_PCI_DP,
  _DevPathPci,
  HARDWARE_DEVICE_PATH,
  HW_PCCARD_DP,
  _DevPathPccard,
  HARDWARE_DEVICE_PATH,
  HW_MEMMAP_DP,
  _DevPathMemMap,
  HARDWARE_DEVICE_PATH,
  HW_VENDOR_DP,
  _DevPathVendor,
  HARDWARE_DEVICE_PATH,
  HW_CONTROLLER_DP,
  _DevPathController,
  ACPI_DEVICE_PATH,
  ACPI_DP,
  _DevPathAcpi,
  ACPI_DEVICE_PATH,
  ACPI_EXTENDED_DP,
  _DevPathExtendedAcpi,
  ACPI_DEVICE_PATH,
  ACPI_ADR_DP,
  _DevPathAdrAcpi,
  MESSAGING_DEVICE_PATH,
  MSG_ATAPI_DP,
  _DevPathAtapi,
  MESSAGING_DEVICE_PATH,
  MSG_SCSI_DP,
  _DevPathScsi,
  MESSAGING_DEVICE_PATH,
  MSG_FIBRECHANNEL_DP,
  _DevPathFibre,
  MESSAGING_DEVICE_PATH,
  MSG_1394_DP,
  _DevPath1394,
  MESSAGING_DEVICE_PATH,
  MSG_USB_DP,
  _DevPathUsb,
#if (EFI_SPECIFICATION_VERSION >= 0x00020000)
  MESSAGING_DEVICE_PATH,
  MSG_USB_WWID_DP,
  _DevPathUsbWWID,
  MESSAGING_DEVICE_PATH,
  MSG_DEVICE_LOGICAL_UNIT_DP,
  _DevPathLogicalUnit,
#endif
  MESSAGING_DEVICE_PATH,
  MSG_USB_CLASS_DP,
  _DevPathUsbClass,
  MESSAGING_DEVICE_PATH,
  MSG_I2O_DP,
  _DevPathI2O,
  MESSAGING_DEVICE_PATH,
  MSG_MAC_ADDR_DP,
  _DevPathMacAddr,
  MESSAGING_DEVICE_PATH,
  MSG_IPv4_DP,
  _DevPathIPv4,
  MESSAGING_DEVICE_PATH,
  MSG_IPv6_DP,
  _DevPathIPv6,
  MESSAGING_DEVICE_PATH,
  MSG_INFINIBAND_DP,
  _DevPathInfiniBand,
  MESSAGING_DEVICE_PATH,
  MSG_UART_DP,
  _DevPathUart,
  MESSAGING_DEVICE_PATH,
  MSG_VENDOR_DP,
  _DevPathVendor,
#if (EFI_SPECIFICATION_VERSION >= 0x00020000)
  MESSAGING_DEVICE_PATH,
  MSG_ISCSI_DP,
  _DevPathiSCSI,
#endif
  MEDIA_DEVICE_PATH,
  MEDIA_HARDDRIVE_DP,
  _DevPathHardDrive,
  MEDIA_DEVICE_PATH,
  MEDIA_CDROM_DP,
  _DevPathCDROM,
  MEDIA_DEVICE_PATH,
  MEDIA_VENDOR_DP,
  _DevPathVendor,
  MEDIA_DEVICE_PATH,
  MEDIA_FILEPATH_DP,
  _DevPathFilePath,
  MEDIA_DEVICE_PATH,
  MEDIA_PROTOCOL_DP,
  _DevPathMediaProtocol,
#if (EFI_SPECIFICATION_VERSION < 0x00020000)
  MEDIA_DEVICE_PATH,
  MEDIA_FV_FILEPATH_DP,
  _DevPathFvFilePath,
#endif
  BBS_DEVICE_PATH,
  BBS_BBS_DP,
  _DevPathBssBss,
  END_DEVICE_PATH_TYPE,
  END_INSTANCE_DEVICE_PATH_SUBTYPE,
  _DevPathEndInstance,
  0,
  0,
  NULL
};

CHAR16 *
LibDevicePathToStr (
  IN EFI_DEVICE_PATH_PROTOCOL     *DevPath
  )
/*++
Routine Description:
    Turns the Device Path into a printable string.  Allcoates
    the string from pool.  The caller must FreePool the returned
    string.
    
Arguments:
    Devpath      -  The devices
    
Returns:
    The name of the devpath

--*/
{
  POOL_PRINT                Str;
  EFI_DEVICE_PATH_PROTOCOL  *DevPathNode;
  VOID (*DumpNode) (POOL_PRINT *, VOID *);

  UINTN Index;
  UINTN NewSize;

  ZeroMem (&Str, sizeof (Str));

  if (DevPath == NULL) {
    goto Done;
  }
  //
  // Unpacked the device path
  //
  DevPath = UnpackDevicePath (DevPath);
  ASSERT (DevPath);

  //
  // Process each device path node
  //
  DevPathNode = DevPath;
  while (!IsDevicePathEnd (DevPathNode)) {
    //
    // Find the handler to dump this device path node
    //
    DumpNode = NULL;
    for (Index = 0; DevPathTable[Index].Function; Index += 1) {

      if (DevicePathType (DevPathNode) == DevPathTable[Index].Type &&
          DevicePathSubType (DevPathNode) == DevPathTable[Index].SubType
          ) {
        DumpNode = DevPathTable[Index].Function;
        break;
      }
    }
    //
    // If not found, use a generic function
    //
    if (!DumpNode) {
      DumpNode = _DevPathNodeUnknown;
    }
    //
    //  Put a path seperator in if needed
    //
    if (Str.len && DumpNode != _DevPathEndInstance) {
      CatPrint (&Str, L"/");
    }
    //
    // Print this node of the device path
    //
    DumpNode (&Str, DevPathNode);

    //
    // Next device path node
    //
    DevPathNode = (EFI_DEVICE_PATH_PROTOCOL *) NextDevicePathNode (DevPathNode);
  }
  //
  // Shrink pool used for string allocation
  //
  FreePool (DevPath);
Done:
  NewSize           = (Str.len + 1) * sizeof (CHAR16);
  Str.str           = ReallocatePool (Str.str, NewSize, NewSize);
  ASSERT(Str.str != NULL);
  Str.str[Str.len]  = 0;
  return Str.str;
}

EFI_DEVICE_PATH_PROTOCOL *
AppendDevicePathNode (
  IN EFI_DEVICE_PATH_PROTOCOL  *Src1,
  IN EFI_DEVICE_PATH_PROTOCOL  *Src2
  )
/*++

Routine Description:
  Function is used to append a device path node to all the instances in another device path.

Arguments:
  Src1           - A pointer to a device path data structure.

  Src2           - A pointer to a device path data structure.

Returns:

  This function returns a pointer to the new device path.  
  If there is not enough temporary pool memory available to complete this function, 
  then NULL is returned.

  Src1 may have multiple "instances" and each instance is appended
  Src2 is a signal device path node (without a terminator) that is
  appended to each instance is Src1.

--*/
{
  EFI_DEVICE_PATH_PROTOCOL  *Temp;

  EFI_DEVICE_PATH_PROTOCOL  *Eop;
  UINTN                     Length;
  
  ASSERT (Src1);
  ASSERT (Src2);
  //
  // Build a Src2 that has a terminator on it
  //
  Length  = DevicePathNodeLength (Src2);
  Temp    = AllocatePool (Length + sizeof (EFI_DEVICE_PATH_PROTOCOL));
  if (!Temp) {
    return NULL;
  }

  CopyMem (Temp, Src2, Length);
  Eop = NextDevicePathNode (Temp);
  SetDevicePathEndNode (Eop);

  //
  // Append device paths
  //
  Src1 = AppendDevicePath (Src1, Temp);
  FreePool (Temp);
  return Src1;
}

EFI_DEVICE_PATH_PROTOCOL *
FileDevicePath (
  IN EFI_HANDLE                 Device  OPTIONAL,
  IN CHAR16                     *FileName
  )
/*++

Routine Description:
  Function allocates a device path for a file and appends it to an existing device path.

Arguments:
  Device         - A pointer to a device handle.

  FileName       - A pointer to a Null-terminated Unicode string.

Returns:

  If Device is not a valid device handle, then a device path for the file specified 
  by FileName is allocated and returned.

  Results are allocated from pool.  The caller must FreePool the resulting device path 
  structure

--*/
{
  UINTN                     Size;
  FILEPATH_DEVICE_PATH      *FilePath;
  EFI_DEVICE_PATH_PROTOCOL  *Eop;
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath;

  ASSERT (FileName);

  Size        = StrSize (FileName);
  FilePath    = AllocateZeroPool (Size + SIZE_OF_FILEPATH_DEVICE_PATH + sizeof (EFI_DEVICE_PATH_PROTOCOL));
  DevicePath  = NULL;

  if (FilePath != NULL) {
    //
    // Build a file path
    //
    FilePath->Header.Type     = MEDIA_DEVICE_PATH;
    FilePath->Header.SubType  = MEDIA_FILEPATH_DP;
    SetDevicePathNodeLength (&FilePath->Header, Size + SIZE_OF_FILEPATH_DEVICE_PATH);
    CopyMem (FilePath->PathName, FileName, Size);
    Eop = NextDevicePathNode (&FilePath->Header);
    SetDevicePathEndNode (Eop);

    //
    // Append file path to device's device path
    //
    DevicePath = (EFI_DEVICE_PATH_PROTOCOL *) FilePath;
    if (Device) {
      DevicePath = AppendDevicePath (
                    DevicePathFromHandle (Device),
                    DevicePath
                    );
      FreePool (FilePath);

      ASSERT (DevicePath);
    }
  }

  return DevicePath;
}

EFI_DEVICE_PATH_PROTOCOL *
AppendDevicePathInstance (
  IN EFI_DEVICE_PATH_PROTOCOL  *Src,
  IN EFI_DEVICE_PATH_PROTOCOL  *Instance
  )
/*++

Routine Description:
  Function is used to add a device path instance to a device path.

Arguments:
  Src          - A pointer to a device path data structure

  Instance     - A pointer to a device path instance.

Returns:

  This function returns a pointer to the new device path. 
  If there is not enough temporary pool memory available to complete this function, 
  then NULL is returned. It is up to the caller to free the memory used by Src and 
  Instance if they are no longer needed.

--*/
{
  UINT8                     *Ptr;
  EFI_DEVICE_PATH_PROTOCOL  *DevPath;
  UINTN                     SrcSize;
  UINTN                     InstanceSize;

  ASSERT (Instance);

  if (Src == NULL) {
    return DuplicateDevicePath (Instance);
  }

  SrcSize       = DevicePathSize (Src);
  InstanceSize  = DevicePathSize (Instance);
  Ptr           = AllocatePool (SrcSize + InstanceSize);
  DevPath       = (EFI_DEVICE_PATH_PROTOCOL *) Ptr;
  ASSERT (DevPath);

  CopyMem (Ptr, Src, SrcSize);
  while (!IsDevicePathEnd (DevPath)) {
    DevPath = NextDevicePathNode (DevPath);
  }
  //
  // Convert the End to an End Instance, since we are
  //  appending another instacne after this one its a good
  //  idea.
  //
  DevPath->SubType  = END_INSTANCE_DEVICE_PATH_SUBTYPE;

  DevPath           = NextDevicePathNode (DevPath);
  CopyMem (DevPath, Instance, InstanceSize);
  return (EFI_DEVICE_PATH_PROTOCOL *) Ptr;
}

EFI_STATUS
LibDevicePathToInterface (
  IN EFI_GUID                   *Protocol,
  IN EFI_DEVICE_PATH_PROTOCOL   *FilePath,
  OUT VOID                      **Interface
  )
/*++

Routine Description:
  Function retrieves a protocol interface for a device.

Arguments:
  Protocol     - The published unique identifier of the protocol.

  FilePath     - A pointer to a device path data structure.

  Interface    - Supplies and address where a pointer to the requested 
                 Protocol interface is returned.

Returns:

  If a match is found, then the protocol interface of that device is 
  returned in Interface.  Otherwise, Interface is set to NULL.

--*/
{
  EFI_STATUS  Status;
  EFI_HANDLE  Device;
  
  ASSERT (FilePath != NULL);
  ASSERT (Interface != NULL);

  Status = BS->LocateDevicePath (
                Protocol,
                &FilePath,
                &Device
                );

  if (!EFI_ERROR (Status)) {
    //
    // If we didn't get a direct match return not found
    //
    Status = EFI_NOT_FOUND;

    if (IsDevicePathEnd (FilePath)) {
      //
      // It was a direct match, lookup the protocol interface
      //
      Status = BS->HandleProtocol (
                    Device,
                    Protocol,
                    Interface
                    );
    }
  }
  //
  // If there was an error, do not return an interface
  //
  if (EFI_ERROR (Status)) {
    *Interface = NULL;
  }

  return Status;
}

BOOLEAN
LibMatchDevicePaths (
  IN  EFI_DEVICE_PATH_PROTOCOL *Multi,
  IN  EFI_DEVICE_PATH_PROTOCOL *Single
  )
/*++

Routine Description:
  Function compares a device path data structure to that of all the nodes of a 
  second device path instance.

Arguments:
  Multi        - A pointer to a multi-instance device path data structure.

  Single       - A pointer to a single-instance device path data structure.

Returns:

  The function returns TRUE if the Single is contained within Multi.  
  Otherwise, FALSE is returned.

--*/
{
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath;

  EFI_DEVICE_PATH_PROTOCOL  *DevicePathInst;
  UINTN                     Size;

  if (!Multi || !Single) {
    return FALSE;
  }

  DevicePath      = Multi;
  DevicePathInst  = DevicePathInstance (&DevicePath, &Size);
  while (DevicePathInst) {
    if (CompareMem (Single, DevicePathInst, Size - sizeof (EFI_DEVICE_PATH_PROTOCOL)) == 0) {
      return TRUE;
    }

    DevicePathInst = DevicePathInstance (&DevicePath, &Size);
  }

  return FALSE;
}

EFI_DEVICE_PATH_PROTOCOL *
LibDuplicateDevicePathInstance (
  IN EFI_DEVICE_PATH_PROTOCOL  *DevPath
  )
/*++

Routine Description:
  Function creates a device path data structure that identically matches the 
  device path passed in.

Arguments:
  DevPath      - A pointer to a device path data structure.

Returns:

  The new copy of DevPath is created to identically match the input.  
  Otherwise, NULL is returned.

--*/
{
  EFI_DEVICE_PATH_PROTOCOL  *NewDevPath;

  EFI_DEVICE_PATH_PROTOCOL  *DevicePathInst;

  EFI_DEVICE_PATH_PROTOCOL  *Temp;
  UINTN                     Size;

  ASSERT (DevPath != NULL);
  //
  // get the size of an instance from the input
  //
  Temp            = DevPath;
  DevicePathInst  = DevicePathInstance (&Temp, &Size);

  //
  // Make a copy
  //
  NewDevPath = NULL;
  if (Size != 0) {
    NewDevPath = AllocatePool (Size);
  }

  if (NewDevPath != NULL) {
    CopyMem (NewDevPath, DevicePathInst, Size);
  }

  return NewDevPath;
}

INTN
DevicePathCompare (
  IN  EFI_DEVICE_PATH_PROTOCOL *DevicePath1,
  IN  EFI_DEVICE_PATH_PROTOCOL *DevicePath2
  )
/*++

Routine Description:
  Function compares two device pathes.

Arguments:
  DevicePath1  - A pointer to a device path data structure.

  DevicePath2  - A pointer to a device path data structure.

Returns:

  The function returns 0 if the two device paths are equal.
  Otherwise, other value is returned.

--*/
{
  UINTN DevPathSize1;
  UINTN DevPathSize2;
  
  ASSERT (DevicePath1);
  ASSERT (DevicePath2);

  DevPathSize1  = DevicePathSize (DevicePath1);
  DevPathSize2  = DevicePathSize (DevicePath2);

  if (DevPathSize1 > DevPathSize2) {
    return 1;
  } else if (DevPathSize1 < DevPathSize2) {
    return -1;
  } else {
    return CompareMem (DevicePath1, DevicePath2, DevPathSize1);
  }
}

VOID
EFIAPI
InitializeFwVolDevicepathNode (
  IN  MEDIA_FW_VOL_FILEPATH_DEVICE_PATH     *FvDevicePathNode,
  IN EFI_GUID                               *NameGuid
  )
/*++

Routine Description:
  The function returns 0 if the two device paths are equal.
  Otherwise, other value is returned.

  Initialize a Firmware Volume (FV) Media Device Path node.
  
  Tiano extended the EFI 1.10 device path nodes. Tiano does not own this enum
  so as we move to UEFI 2.0 support we must use a mechanism that conforms with
  the UEFI 2.0 specification to define the FV device path. An UEFI GUIDed 
  device path is defined for PIWG extensions of device path. If the code 
  is compiled to conform with the UEFI 2.0 specification use the new device path
  else use the old form for backwards compatability.

Arguments:
  FvDevicePathNode  - Pointer to a FV device path node to initialize

  NameGuid          - FV file name to use in FvDevicePathNode

Returns:

--*/
{
#if (EFI_SPECIFICATION_VERSION < 0x00020000) 
  //
  // Use old Device Path that conflicts with UEFI
  //
  FvDevicePathNode->Header.Type     = MEDIA_DEVICE_PATH;
  FvDevicePathNode->Header.SubType  = MEDIA_FV_FILEPATH_DP;
  SetDevicePathNodeLength (&FvDevicePathNode->Header, sizeof (MEDIA_FW_VOL_FILEPATH_DEVICE_PATH));
  
#else
  //
  // Use the new Device path that does not conflict with the UEFI
  //
  FvDevicePathNode->Piwg.Header.Type     = MEDIA_DEVICE_PATH;
  FvDevicePathNode->Piwg.Header.SubType  = MEDIA_VENDOR_DP;
  SetDevicePathNodeLength (&FvDevicePathNode->Piwg.Header, sizeof (MEDIA_FW_VOL_FILEPATH_DEVICE_PATH));

  //
  // Add the GUID for generic PIWG device paths
  //
  CopyMem (&FvDevicePathNode->Piwg.PiwgSpecificDevicePath, &gEfiFrameworkDevicePathGuid, sizeof(EFI_GUID));

  //
  // Add in the FW Vol File Path PIWG defined inforation
  //
  FvDevicePathNode->Piwg.Type = PIWG_MEDIA_FW_VOL_FILEPATH_DEVICE_PATH_TYPE;

#endif
  CopyMem (&FvDevicePathNode->NameGuid, NameGuid, sizeof(EFI_GUID));
}

EFI_GUID *
EFIAPI
GetNameGuidFromFwVolDevicePathNode (
  IN  MEDIA_FW_VOL_FILEPATH_DEVICE_PATH   *FvDevicePathNode
  )
/*++

Routine Description:
  The function returns 0 if the two device paths are equal.
  Otherwise, other value is returned.

  Check to see if the Firmware Volume (FV) Media Device Path is valid.
  
  Tiano extended the EFI 1.10 device path nodes. Tiano does not own this enum
  so as we move to UEFI 2.0 support we must use a mechanism that conforms with
  the UEFI 2.0 specification to define the FV device path. An UEFI GUIDed 
  device path is defined for PIWG extensions of device path. If the code 
  is compiled to conform with the UEFI 2.0 specification use the new device path
  else use the old form for backwards compatability. The return value to this
  function points to a location in FvDevicePathNode and it does not allocate
  new memory for the GUID pointer that is returned.

Arguments:
  FvDevicePathNode  - Pointer to FV device path to check

Returns:
  NULL  -  FvDevicePathNode is not valid.
  Other -  FvDevicePathNode is valid and pointer to NameGuid was returned.

--*/
{
#if (EFI_SPECIFICATION_VERSION < 0x00020000) 
  //
  // Use old Device Path that conflicts with UEFI
  //
  if (DevicePathType (&FvDevicePathNode->Header) == MEDIA_DEVICE_PATH &&
      DevicePathSubType (&FvDevicePathNode->Header) == MEDIA_FV_FILEPATH_DP) {
    return &FvDevicePathNode->NameGuid;
  }

#else
  //
  // Use the new Device path that does not conflict with the UEFI
  //
  if (DevicePathType (&FvDevicePathNode->Piwg.Header) == MEDIA_DEVICE_PATH &&
      DevicePathSubType (&FvDevicePathNode->Piwg.Header) == MEDIA_VENDOR_DP) {
    if (CompareMem (&gEfiFrameworkDevicePathGuid, &FvDevicePathNode->Piwg.PiwgSpecificDevicePath, sizeof(EFI_GUID)) == 0) {
      if (FvDevicePathNode->Piwg.Type == PIWG_MEDIA_FW_VOL_FILEPATH_DEVICE_PATH_TYPE) {
        return &FvDevicePathNode->NameGuid;
      }
    }
  }
#endif  
  return NULL;
}
