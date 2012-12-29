/*++

Copyright (c) 2005 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  protid.c
 
Abstract:

  Shell environment protocol id information management



Revision History

--*/

#include "shelle.h"
#include "shellenvguid.h"

#if (PLATFORM == NT32)
#define LOCAL_EFI_WIN_NT_THUNK_PROTOCOL_GUID \
  { \
    0x58c518b1, 0x76f3, 0x11d4, 0xbc, 0xea, 0x0, 0x80, 0xc7, 0x3c, 0x88, 0x81 \
  }

#define LOCAL_EFI_WIN_NT_BUS_DRIVER_IO_PROTOCOL_GUID \
  { \
    0x96eb4ad6, 0xa32a, 0x11d4, 0xbc, 0xfd, 0x0, 0x80, 0xc7, 0x3c, 0x88, 0x81 \
  }

#define LOCAL_EFI_WIN_NT_SERIAL_PORT_GUID \
  { \
    0xc95a93d, 0xa006, 0x11d4, 0xbc, 0xfa, 0x0, 0x80, 0xc7, 0x3c, 0x88, 0x81 \
  }
#endif

struct {
  CHAR16                      *IdString;
  SHELLENV_DUMP_PROTOCOL_INFO DumpInfo;
  SHELLENV_DUMP_PROTOCOL_INFO DumpToken;
  EFI_GUID                    ProtocolId;
}
SEnvInternalProtocolInfo[] = {
  L"DevIo",
  NULL,
  NULL,
  EFI_DEVICE_IO_PROTOCOL_GUID,
  L"Fs",
  NULL,
  NULL,
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID,
  L"DiskIo",
  NULL,
  NULL,
  EFI_DISK_IO_PROTOCOL_GUID,
  L"BlkIo",
  SEnvBlkIo,
  NULL,
  EFI_BLOCK_IO_PROTOCOL_GUID,
  L"Txtin",
  NULL,
  NULL,
  EFI_SIMPLE_TEXT_IN_PROTOCOL_GUID,
  L"Txtout",
  SEnvTextOut,
  NULL,
  EFI_SIMPLE_TEXT_OUT_PROTOCOL_GUID,
  L"Load",
  NULL,
  NULL,
  LOAD_FILE_PROTOCOL_GUID,
  L"Image",
  SEnvImage,
  SEnvImageTok,
  EFI_LOADED_IMAGE_PROTOCOL_GUID,
  L"UnicodeCollation",
  NULL,
  NULL,
  EFI_UNICODE_COLLATION_PROTOCOL_GUID,
  //
  //  L"LegacyBoot",          NULL,                 NULL,           EFI_LEGACY_BOOT_PROTOCOL_GUID,
  //
  L"SerialIo",
  NULL,
  NULL,
  EFI_SERIAL_IO_PROTOCOL_GUID,
  L"Pxebc",
  NULL,
  NULL,
  EFI_PXE_BASE_CODE_PROTOCOL_GUID,
  L"Tcp",
  NULL,
  NULL,
  EFI_TCP_PROTOCOL_GUID,
  L"Net",
  NULL,
  NULL,
  EFI_SIMPLE_NETWORK_PROTOCOL_GUID,
  //
  //  L"TxtOutSplit",         NULL,                 NULL,           TEXT_OUT_SPLITER_PROTOCOL,
  //  L"ErrOutSplit",         NULL,                 NULL,           ERROR_OUT_SPLITER_PROTOCOL,
  //  L"TxtInSplit",          NULL,                 NULL,           TEXT_IN_SPLITER_PROTOCOL,
  //
  L"Nii",
  NULL,
  NULL,
  EFI_NETWORK_INTERFACE_IDENTIFIER_PROTOCOL_GUID,
  L"UgaDraw",
  NULL,
  NULL,
  EFI_UGA_DRAW_PROTOCOL_GUID,
  L"UgaIo",
  NULL,
  NULL,
  EFI_UGA_IO_PROTOCOL_GUID,
  L"GraphicsOutput",
  SEnvGraphicsOutput,
  NULL,
  EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID,
  L"EdidDiscovered",
  NULL,
  NULL,
  EFI_EDID_DISCOVERED_PROTOCOL_GUID,
  L"EdidActive",
  NULL,
  NULL,
  EFI_EDID_ACTIVE_PROTOCOL_GUID,
  L"EdidOverride",
  NULL,
  NULL,
  EFI_EDID_OVERRIDE_PROTOCOL_GUID,
  //
  // just plain old protocol ids
  //
  L"ShellInt",
  NULL,
  NULL,
  SHELL_INTERFACE_PROTOCOL,
  L"SEnv",
  NULL,
  NULL,
  ENVIRONMENT_VARIABLE_ID,
  L"ShellProtId",
  NULL,
  NULL,
  PROTOCOL_ID_ID,
  L"ShellDevPathMap",
  NULL,
  NULL,
  DEVICE_PATH_MAPPING_ID,
  L"ShellAlias",
  NULL,
  NULL,
  ALIAS_ID,
  //
  // ID guids
  //
  L"G0",
  NULL,
  NULL,
  {
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0
  },
  L"Efi",
  NULL,
  NULL,
  EFI_GLOBAL_VARIABLE_GUID,
  L"GenFileInfo",
  NULL,
  NULL,
  EFI_FILE_INFO_ID,
  L"FileSysInfo",
  NULL,
  NULL,
  EFI_FILE_SYSTEM_INFO_ID_GUID,
  L"PcAnsi",
  NULL,
  NULL,
  DEVICE_PATH_MESSAGING_PC_ANSI,
  L"Vt100",
  NULL,
  NULL,
  DEVICE_PATH_MESSAGING_VT_100,
  L"Vt100+",
  NULL,
  NULL,
  DEVICE_PATH_MESSAGING_VT_100_PLUS,
  L"VtUtf8",
  NULL,
  NULL,
  DEVICE_PATH_MESSAGING_VT_UTF8,
  L"ESP",
  NULL,
  NULL,
  EFI_PART_TYPE_EFI_SYSTEM_PART_GUID,
  L"GPT MBR",
  NULL,
  NULL,
  EFI_PART_TYPE_LEGACY_MBR_GUID,
  L"DriverBinding",
  NULL,
  NULL,
  EFI_DRIVER_BINDING_PROTOCOL_GUID,
  L"ComponentName",
  NULL,
  NULL,
  EFI_COMPONENT_NAME_PROTOCOL_GUID,
  L"Configuration",
  NULL,
  NULL,
  EFI_DRIVER_CONFIGURATION_PROTOCOL_GUID,
  L"Diagnostics",
  NULL,
  NULL,
  EFI_DRIVER_DIAGNOSTICS_PROTOCOL_GUID,
#if (PLATFORM == NT32)
  L"WinNtThunk",
  NULL,
  NULL,
  LOCAL_EFI_WIN_NT_THUNK_PROTOCOL_GUID,
  L"WinNtDriverIo",
  NULL,
  NULL,
  LOCAL_EFI_WIN_NT_BUS_DRIVER_IO_PROTOCOL_GUID,
  L"SerialPrivate",
  NULL,
  NULL,
  LOCAL_EFI_WIN_NT_SERIAL_PORT_GUID,
#endif
  L"PciRootBridgeIo",
  SEnvPciRootBridgeIo,
  NULL,
  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_GUID,
  L"PciIo",
  SEnvPciIo,
  NULL,
  EFI_PCI_IO_PROTOCOL_GUID,
  L"IsaIo",
  SEnvIsaIo,
  NULL,
  EFI_ISA_IO_PROTOCOL_GUID,
  L"UsbIo",
  SEnvUsbIo,
  NULL,
  EFI_USB_IO_PROTOCOL_GUID,
  L"IsaAcpi",
  NULL,
  NULL,
  EFI_ISA_ACPI_PROTOCOL_GUID,
  L"SimplePointer",
  NULL,
  NULL,
  EFI_SIMPLE_POINTER_PROTOCOL_GUID,
  L"ConIn",
  NULL,
  NULL,
  EFI_CONSOLE_IN_DEVICE_GUID,
  L"ConOut",
  NULL,
  NULL,
  EFI_CONSOLE_OUT_DEVICE_GUID,
  L"StdErr",
  NULL,
  NULL,
  EFI_STANDARD_ERROR_DEVICE_GUID,
  L"Decompress",
  NULL,
  NULL,
  EFI_DECOMPRESS_PROTOCOL_GUID,
  L"DebugPort",
  NULL,
  NULL,
  EFI_DEBUGPORT_PROTOCOL_GUID,
  L"DebugSupport",
  SEnvDebugSupport,
  NULL,
  EFI_DEBUG_SUPPORT_PROTOCOL_GUID,
  L"ScsiPassThru",
  NULL,
  NULL,
  EFI_SCSI_PASS_THRU_PROTOCOL_GUID,
  L"ScsiIo",
  NULL,
  NULL,
  EFI_SCSI_IO_PROTOCOL_GUID,
  L"BusSpecificDriverOverride",
  SEnvBusSpecificDriverOverride,
  NULL,
  EFI_BUS_SPECIFIC_DRIVER_OVERRIDE_PROTOCOL_GUID,

  L"UsbHc",
  NULL,
  NULL,
  EFI_USB_HC_PROTOCOL_GUID,

  L"UsbHc2",
  NULL,
  NULL,
  EFI_USB2_HC_PROTOCOL_GUID,

  L"ExtScsiPassThru",
  NULL,
  NULL,
  EFI_EXT_SCSI_PASS_THRU_PROTOCOL_GUID,

  L"MNPSb",
  NULL,
  NULL,
  EFI_MANAGED_NETWORK_SERVICE_BINDING_PROTOCOL_GUID,

  L"MNP",
  NULL,
  NULL,
  EFI_MANAGED_NETWORK_PROTOCOL_GUID,

  L"ARPSb",
  NULL,
  NULL,
  EFI_ARP_SERVICE_BINDING_PROTOCOL_GUID,

  L"ARP",
  NULL,
  NULL,
  EFI_ARP_PROTOCOL_GUID,

  L"DHCPv4Sb",
  NULL,
  NULL,
  EFI_DHCP4_SERVICE_BINDING_PROTOCOL_GUID,

  L"DHCPv4",
  NULL,
  NULL,
  EFI_DHCP4_PROTOCOL_GUID,

  L"TCPv4Sb",
  NULL,
  NULL,
  EFI_TCP4_SERVICE_BINDING_PROTOCOL_GUID,

  L"TCPv4",
  NULL,
  NULL,
  EFI_TCP4_PROTOCOL_GUID,

  L"IPv4Sb",
  NULL,
  NULL,
  EFI_IP4_SERVICE_BINDING_PROTOCOL_GUID,

  L"IPv4",
  NULL,
  NULL,
  EFI_IP4_PROTOCOL_GUID,

  L"IPv4Config",
  NULL,
  NULL,
  EFI_IP4_CONFIG_PROTOCOL_GUID,

  L"UDPv4Sb",
  NULL,
  NULL,
  EFI_UDP4_SERVICE_BINDING_PROTOCOL_GUID,

  L"UDPv4",
  NULL,
  NULL,
  EFI_UDP4_PROTOCOL_GUID,

  L"MTFTPv4Sb",
  NULL,
  NULL,
  EFI_MTFTP4_SERVICE_BINDING_PROTOCOL_GUID,

  L"MTFTPv4",
  NULL,
  NULL,
  EFI_MTFTP4_PROTOCOL_GUID,

  L"Dpath",
  SEnvDPath,
  SEnvDPathTok,
  EFI_DEVICE_PATH_PROTOCOL_GUID,

  L"Unknown Device",
  NULL,
  NULL,
  UNKNOWN_DEVICE_GUID,
  NULL
};

//
// put supported lanuage here.
//
struct {
  CHAR16  *IdString;
}
SLanguageTable[] = {
  L"eng",
  L"fra",
  NULL
};

//
// SEnvProtocolInfo - A list of all known protocol info structures
//
EFI_LIST_ENTRY  SEnvProtocolInfo;
SHELL_VAR_CHECK_ITEM    DHCheckList[] = {
  {
    L"-p",
    0x01,
    0,
    FlagTypeNeedVar
  },
  {
    L"-l",
    0x02,
    0,
    FlagTypeNeedVar
  },
  {
    L"-b",
    0x04,
    0,
    FlagTypeSingle
  },
  {
    L"-d",
    0x08,
    0,
    FlagTypeSingle
  },
  {
    L"-v",
    0x10,
    0,
    FlagTypeSingle
  },
  {
    L"-?",
    0x20,
    0,
    FlagTypeSingle
  },
  {
    NULL,
    0,
    0,
    FALSE
  }
};

//
//
//
VOID
SEnvInitProtocolInfo (
  VOID
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  //
  // Initialize SEnvProtocolInfo linked list
  //
  InitializeListHead (&SEnvProtocolInfo);
}

VOID
SEnvLoadInternalProtInfo (
  VOID
  )
/*++

Routine Description:

  Initialize internal protocol handlers.

Arguments:

Returns:

--*/
{
  UINTN Index;

  //
  // Walk through the SEnvInternalProtocolInfo[] array
  // add each protocol info to a linked list
  //
  for (Index = 0; SEnvInternalProtocolInfo[Index].IdString; Index += 1) {
    SEnvAddProtocol (
      &SEnvInternalProtocolInfo[Index].ProtocolId,
      SEnvInternalProtocolInfo[Index].DumpToken,
      SEnvInternalProtocolInfo[Index].DumpInfo,
      SEnvInternalProtocolInfo[Index].IdString
      );
  }
}

PROTOCOL_INFO *
SEnvGetProtById (
  IN EFI_GUID         *Protocol,
  IN BOOLEAN          GenId
  )
/*++

Routine Description:

  Locate a protocol handle by guid.

Arguments:

  Protocol - The protocol guid
  GenId    - Is Gen the ID

Returns:

--*/
{
  PROTOCOL_INFO   *Prot;
  EFI_LIST_ENTRY  *Link;
  UINTN           LastId;
  UINTN           Id;
  CHAR16          s[40];

  ASSERT_LOCKED (&SEnvGuidLock);

  //
  // Find the protocol entry for this id
  //
  LastId = 0;
  for (Link = SEnvProtocolInfo.Flink; Link != &SEnvProtocolInfo; Link = Link->Flink) {
    Prot = CR (Link, PROTOCOL_INFO, Link, PROTOCOL_INFO_SIGNATURE);
    if (CompareGuid (&Prot->ProtocolId, Protocol) == 0) {
      return Prot;
    }

    if (Prot->IdString[0] == 'g') {
      Id      = Atoi (Prot->IdString + 1);
      LastId  = Id > LastId ? Id : LastId;
    }
  }
  //
  // If the protocol id is not found, generate a string for it if needed
  //
  Prot = NULL;
  if (GenId) {
    SPrint (s, sizeof (s), L"g%d", LastId + 1);
    Prot = AllocateZeroPool (sizeof (PROTOCOL_INFO));
    if (Prot) {
      Prot->Signature = PROTOCOL_INFO_SIGNATURE;
      CopyMem (&Prot->ProtocolId, Protocol, sizeof (EFI_GUID));
      Prot->IdString = StrDuplicate (s);
      InsertTailList (&SEnvProtocolInfo, &Prot->Link);
    }
  }

  return Prot;
}

PROTOCOL_INFO *
SEnvGetProtByStr (
  IN CHAR16           *Str
  )
/*++

Routine Description:

Arguments:

  Str  - The string

Returns:

--*/
{
  PROTOCOL_INFO   *Prot;
  EFI_LIST_ENTRY  *Link;
  UINTN           Index;
  EFI_GUID        Guid;
  CHAR16          c;
  CHAR16          *Ptr;

  ASSERT_LOCKED (&SEnvGuidLock);

  //
  // Search for short name match
  //
  for (Link = SEnvProtocolInfo.Flink; Link != &SEnvProtocolInfo; Link = Link->Flink) {
    Prot = CR (Link, PROTOCOL_INFO, Link, PROTOCOL_INFO_SIGNATURE);
    if (StriCmp (Prot->IdString, Str) == 0) {
      return Prot;
    }
  }
  //
  // Convert Str to guid and then match
  //
  if (StrLen (Str) == 36 && Str[9] == '-' && Str[19] == '-' && Str[24] == '-') {
    Guid.Data1  = (UINT32) Xtoi (Str + 0);
    Guid.Data2  = (UINT16) Xtoi (Str + 10);
    Guid.Data3  = (UINT16) Xtoi (Str + 15);
    for (Index = 0; Index < 8; Index++) {
      Ptr               = Str + 25 + Index * 2;
      c                 = Ptr[3];
      Ptr[3]            = 0;
      Guid.Data4[Index] = (UINT8) Xtoi (Ptr);
      Ptr[3]            = c;
    }

    for (Link = SEnvProtocolInfo.Flink; Link != &SEnvProtocolInfo; Link = Link->Flink) {
      Prot = CR (Link, PROTOCOL_INFO, Link, PROTOCOL_INFO_SIGNATURE);
      if (CompareGuid (&Prot->ProtocolId, &Guid) == 0) {
        return Prot;
      }
    }
  }

  return NULL;
}

EFI_STATUS
SEnvIGetProtID (
  IN CHAR16           *Str,
  OUT EFI_GUID        *ProtId
  )
/*++

Routine Description:

Arguments:

  Str       - The string
  ProtID    - The protocol guid

Returns:

--*/
{
  PROTOCOL_INFO *Prot;
  EFI_STATUS    Status;

  AcquireLock (&SEnvGuidLock);

  Status = EFI_NOT_FOUND;
  CopyMem (ProtId, &NullGuid, sizeof (EFI_GUID));

  //
  // Get protocol id by string
  //
  Prot = SEnvGetProtByStr (Str);
  if (Prot) {
    CopyMem (ProtId, &Prot->ProtocolId, sizeof (EFI_GUID));
    Status = EFI_SUCCESS;
  }

  ReleaseLock (&SEnvGuidLock);

  return Status;
}

VOID
EFIAPI
SEnvAddProtocol (
  IN EFI_GUID                                 *Protocol,
  IN SHELLENV_DUMP_PROTOCOL_INFO              DumpToken OPTIONAL,
  IN SHELLENV_DUMP_PROTOCOL_INFO              DumpInfo OPTIONAL,
  IN CHAR16                                   *IdString
  )
/*++

Routine Description:

  Published interface to add protocol handlers.

Arguments:

  Protocol  - The protocol
  DumpToken - The dump token
  DumpInfo  - The dump infomation
  IdString  - The ID string

Returns:

--*/
{
  SEnvIAddProtocol (TRUE, Protocol, DumpToken, DumpInfo, IdString);
}

VOID
SEnvIAddProtocol (
  IN BOOLEAN                                  SaveId,
  IN EFI_GUID                                 *Protocol,
  IN SHELLENV_DUMP_PROTOCOL_INFO              DumpToken OPTIONAL,
  IN SHELLENV_DUMP_PROTOCOL_INFO              DumpInfo OPTIONAL,
  IN CHAR16                                   *IdString
  )
/*++

Routine Description:

  Internal interface to add protocol handlers.

Arguments:

  SaveId    - Is save the ID
  Protocol  - The protocol
  DumpToken - The dump token
  DumpInfo  - The dump infomation
  IdString  - The ID string

Returns:

--*/
{
  PROTOCOL_INFO *Prot;
  BOOLEAN       StoreInfo;
  CHAR16        *ObsoleteName;

  ObsoleteName  = NULL;
  StoreInfo     = FALSE;

  AcquireLock (&SEnvGuidLock);

  //
  // Get the current protocol info
  //
  Prot = SEnvGetProtById (Protocol, FALSE);

  if (Prot) {
    //
    // If the name has changed, delete the old variable
    //
    if (StriCmp (Prot->IdString, IdString)) {
      ObsoleteName  = Prot->IdString;
      StoreInfo     = TRUE;
    } else {
      FreePool (Prot->IdString);
    }

    Prot->IdString = NULL;
  } else {
    //
    // Allocate new protocol info
    //
    Prot            = AllocateZeroPool (sizeof (PROTOCOL_INFO));
    Prot->Signature = PROTOCOL_INFO_SIGNATURE;
    StoreInfo       = TRUE;
  }
  //
  // Apply any updates to the protocol info
  //
  if (Prot) {
    CopyMem (&Prot->ProtocolId, Protocol, sizeof (EFI_GUID));
    Prot->IdString  = StrDuplicate (IdString);
    Prot->DumpToken = DumpToken;
    Prot->DumpInfo  = DumpInfo;

    if (Prot->Link.Flink) {
      RemoveEntryList (&Prot->Link);
    }

    InsertTailList (&SEnvProtocolInfo, &Prot->Link);
  }

  ReleaseLock (&SEnvGuidLock);

  //
  // If the name changed, delete the old name
  //
  if (ObsoleteName) {
    RT->SetVariable (ObsoleteName, &SEnvProtId, 0, 0, NULL);
    FreePool (ObsoleteName);
  }
  //
  // Store the protocol idstring to a variable
  //
  if (Prot && StoreInfo && SaveId) {
    RT->SetVariable (
          Prot->IdString,
          &SEnvProtId,
          EFI_VARIABLE_BOOTSERVICE_ACCESS,  // | EFI_VARIABLE_NON_VOLATILE,
          sizeof (EFI_GUID),
          &Prot->ProtocolId
          );
  }
}

VOID
SEnvLoadHandleProtocolInfo (
  IN EFI_GUID         *SkipProtocol
  )
/*++

Routine Description:

  Code to load the internal handle cross-reference info for each protocol.

Arguments:

  SkipProtocol  - Skiped protocol

Returns:

--*/
{
  PROTOCOL_INFO   *Prot;
  EFI_LIST_ENTRY  *Link;

  AcquireLock (&SEnvGuidLock);

  for (Link = SEnvProtocolInfo.Flink; Link != &SEnvProtocolInfo; Link = Link->Flink) {
    Prot = CR (Link, PROTOCOL_INFO, Link, PROTOCOL_INFO_SIGNATURE);
    //
    // Load internal handle cross-reference info for each protocol
    //
    if (!SkipProtocol || CompareGuid (SkipProtocol, &Prot->ProtocolId) != 0) {
      LibLocateHandle (
        ByProtocol,
        &Prot->ProtocolId,
        NULL,
        &Prot->NoHandles,
        &Prot->Handles
        );
    }
  }

  ReleaseLock (&SEnvGuidLock);
}

VOID
SEnvFreeHandleProtocolInfo (
  VOID
  )
/*++

Routine Description:

  Free the internal handle cross-reference protocol info.

Arguments:

Returns:

--*/
{
  PROTOCOL_INFO   *Prot;
  EFI_LIST_ENTRY  *Link;

  AcquireLock (&SEnvGuidLock);

  //
  // Free all protocol handle info
  //
  for (Link = SEnvProtocolInfo.Flink; Link != &SEnvProtocolInfo; Link = Link->Flink) {
    Prot = CR (Link, PROTOCOL_INFO, Link, PROTOCOL_INFO_SIGNATURE);

    if (Prot->NoHandles) {
      FreePool (Prot->Handles);
      Prot->Handles   = NULL;
      Prot->NoHandles = 0;
    }
  }

  ReleaseLock (&SEnvGuidLock);
}

CHAR16 *
SEnvIGetProtocol (
  IN EFI_GUID     *ProtocolId,
  IN BOOLEAN      GenId
  )
/*++

Routine Description:

  Published interface to lookup a protocol id string.

Arguments:

  ProtocolId - The protocol ID
  GenId      - Is generate the ID

Returns:

--*/
{
  PROTOCOL_INFO *Prot;
  CHAR16        *Id;

  ASSERT_LOCKED (&SEnvGuidLock);
  Prot  = SEnvGetProtById (ProtocolId, GenId);
  Id    = Prot ? Prot->IdString : NULL;
  return Id;
}

CHAR16 *
EFIAPI
SEnvGetProtocol (
  IN EFI_GUID     *ProtocolId,
  IN BOOLEAN      GenId
  )
/*++

Routine Description:

  Published interface to lookup a protocol id string.

Arguments:

  ProtocolId - Protocol ID
  GenId      - Is generate the ID

Returns:

--*/
{
  CHAR16  *Id;

  AcquireLock (&SEnvGuidLock);
  Id = SEnvIGetProtocol (ProtocolId, GenId);
  ReleaseLock (&SEnvGuidLock);
  return Id;
}

UINTN
SEnvGetHandleNumber (
  IN  EFI_HANDLE  Handle
  )
/*++

Routine Description:

Arguments:

  Handle - The handle

Returns:

--*/
{
  UINTN HandleNumber;

  for (HandleNumber = 0; HandleNumber < SEnvNoHandles; HandleNumber++) {
    if (SEnvHandles[HandleNumber] == Handle) {
      break;
    }
  }

  if (HandleNumber >= SEnvNoHandles) {
    return 0;
  }

  return HandleNumber + 1;
}

EFI_STATUS
GetDriverName (
  EFI_HANDLE  DriverBindingHandle,
  UINT8       *Language,
  BOOLEAN     ImageName,
  CHAR16      **DriverName
  )

{
  EFI_STATUS                  Status;
  EFI_DRIVER_BINDING_PROTOCOL *DriverBinding;
  EFI_LOADED_IMAGE_PROTOCOL   *Image;
  EFI_COMPONENT_NAME_PROTOCOL *ComponentName;

  *DriverName = NULL;

  Status = BS->OpenProtocol (
                DriverBindingHandle,
                &gEfiDriverBindingProtocolGuid,
                (VOID **) &DriverBinding,
                NULL,
                NULL,
                EFI_OPEN_PROTOCOL_GET_PROTOCOL
                );
  if (EFI_ERROR (Status)) {
    return EFI_SUCCESS;
  }

  if (ImageName) {
    Status = BS->OpenProtocol (
                  DriverBinding->ImageHandle,
                  &gEfiLoadedImageProtocolGuid,
                  (VOID **) &Image,
                  NULL,
                  NULL,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
    if (!EFI_ERROR (Status)) {
      *DriverName = LibDevicePathToStr (Image->FilePath);
    }
  } else {
    Status = BS->OpenProtocol (
                  DriverBindingHandle,
                  &gEfiComponentNameProtocolGuid,
                  (VOID **) &ComponentName,
                  NULL,
                  NULL,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
    if (!EFI_ERROR (Status)) {
      //
      // Make sure the interface has been implemented
      //
      if (ComponentName->GetDriverName != NULL) {
        Status = ComponentName->GetDriverName (
                                  ComponentName,
                                  Language,
                                  DriverName
                                  );
      }
    }
  }

  return EFI_SUCCESS;
}

EFI_STATUS
SEnvGetDeviceName (
  EFI_HANDLE  DeviceHandle,
  BOOLEAN     UseComponentName,
  BOOLEAN     UseDevicePath,
  CHAR8       *Language,
  CHAR16      **BestDeviceName,
  EFI_STATUS  *ConfigurationStatus,
  EFI_STATUS  *DiagnosticsStatus,
  BOOLEAN     Display,
  UINTN       Indent
  )

{
  EFI_STATUS                  Status;
  UINTN                       HandleIndex;
  UINTN                       ParentDriverIndex;
  UINTN                       ChildIndex;
  UINTN                       DriverBindingHandleCount;
  EFI_HANDLE                  *DriverBindingHandleBuffer;
  UINTN                       ParentControllerHandleCount;
  EFI_HANDLE                  *ParentControllerHandleBuffer;
  UINTN                       ParentDriverBindingHandleCount;
  EFI_HANDLE                  *ParentDriverBindingHandleBuffer;
  UINTN                       ChildControllerHandleCount;
  EFI_HANDLE                  *ChildControllerHandleBuffer;
  UINTN                       ChildHandleCount;
  EFI_HANDLE                  *ChildHandleBuffer;
  EFI_DEVICE_PATH_PROTOCOL    *DevicePath;
  CHAR16                      *ControllerName;
  EFI_COMPONENT_NAME_PROTOCOL *ComponentName;
  UINTN                       Index;
  BOOLEAN                     First;

  *BestDeviceName       = NULL;
  ControllerName        = NULL;
  *ConfigurationStatus  = EFI_NOT_FOUND;
  *DiagnosticsStatus    = EFI_NOT_FOUND;
  First                 = TRUE;

  Status = LibGetManagingDriverBindingHandles (
            DeviceHandle,
            &DriverBindingHandleCount,
            &DriverBindingHandleBuffer
            );

  Status = LibGetParentControllerHandles (
            DeviceHandle,
            &ParentControllerHandleCount,
            &ParentControllerHandleBuffer
            );

  Status = LibGetChildControllerHandles (
            DeviceHandle,
            &ChildControllerHandleCount,
            &ChildControllerHandleBuffer
            );

  for (HandleIndex = 0; HandleIndex < DriverBindingHandleCount; HandleIndex++) {

    Status = BS->OpenProtocol (
                  DriverBindingHandleBuffer[HandleIndex],
                  &gEfiDriverConfigurationProtocolGuid,
                  NULL,
                  NULL,
                  NULL,
                  EFI_OPEN_PROTOCOL_TEST_PROTOCOL
                  );
    if (!EFI_ERROR (Status)) {
      *ConfigurationStatus = EFI_SUCCESS;
    }

    Status = BS->OpenProtocol (
                  DriverBindingHandleBuffer[HandleIndex],
                  &gEfiDriverDiagnosticsProtocolGuid,
                  NULL,
                  NULL,
                  NULL,
                  EFI_OPEN_PROTOCOL_TEST_PROTOCOL
                  );
    if (!EFI_ERROR (Status)) {
      *DiagnosticsStatus = EFI_SUCCESS;
    }

    Status = BS->OpenProtocol (
                  DriverBindingHandleBuffer[HandleIndex],
                  &gEfiComponentNameProtocolGuid,
                  (VOID **) &ComponentName,
                  NULL,
                  NULL,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
    if (EFI_ERROR (Status)) {
      continue;
    }
    //
    // Make sure this interface has been implemented
    //
    if (ComponentName->GetControllerName != NULL) {
      Status = ComponentName->GetControllerName (
                                ComponentName,
                                DeviceHandle,
                                NULL,
                                Language,
                                &ControllerName
                                );
    } else {
      Status = EFI_UNSUPPORTED;
    }

    if (EFI_ERROR (Status)) {
      continue;
    }

    if (Display) {
      if (!First) {
        Print (L"\n");
        for (Index = 0; Index < Indent; Index++) {
          Print (L" ");
        }
      }

      PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_ONE_VAR_HS_NO_SPACES), HiiEnvHandle, ControllerName);
      First = FALSE;
    }

    if (UseComponentName) {
      if (*BestDeviceName == NULL) {
        *BestDeviceName = StrDuplicate (ControllerName);
      }
    }
  }

  for (HandleIndex = 0; HandleIndex < ParentControllerHandleCount; HandleIndex++) {

    Status = LibGetManagingDriverBindingHandles (
              ParentControllerHandleBuffer[HandleIndex],
              &ParentDriverBindingHandleCount,
              &ParentDriverBindingHandleBuffer
              );
    if (EFI_ERROR (Status)) {
      continue;
    }

    for (ParentDriverIndex = 0; ParentDriverIndex < ParentDriverBindingHandleCount; ParentDriverIndex++) {

      Status = LibGetManagedChildControllerHandles (
                ParentDriverBindingHandleBuffer[ParentDriverIndex],
                ParentControllerHandleBuffer[HandleIndex],
                &ChildHandleCount,
                &ChildHandleBuffer
                );
      if (EFI_ERROR (Status)) {
        continue;
      }

      for (ChildIndex = 0; ChildIndex < ChildHandleCount; ChildIndex++) {
        if (ChildHandleBuffer[ChildIndex] == DeviceHandle) {

          Status = BS->OpenProtocol (
                        ParentDriverBindingHandleBuffer[ParentDriverIndex],
                        &gEfiDriverConfigurationProtocolGuid,
                        NULL,
                        NULL,
                        NULL,
                        EFI_OPEN_PROTOCOL_TEST_PROTOCOL
                        );
          if (!EFI_ERROR (Status)) {
            *ConfigurationStatus = EFI_SUCCESS;
          }

          Status = BS->OpenProtocol (
                        ParentDriverBindingHandleBuffer[ParentDriverIndex],
                        &gEfiDriverDiagnosticsProtocolGuid,
                        NULL,
                        NULL,
                        NULL,
                        EFI_OPEN_PROTOCOL_TEST_PROTOCOL
                        );
          if (!EFI_ERROR (Status)) {
            *DiagnosticsStatus = EFI_SUCCESS;
          }

          Status = BS->OpenProtocol (
                        ParentDriverBindingHandleBuffer[ParentDriverIndex],
                        &gEfiComponentNameProtocolGuid,
                        (VOID **) &ComponentName,
                        NULL,
                        NULL,
                        EFI_OPEN_PROTOCOL_GET_PROTOCOL
                        );
          if (EFI_ERROR (Status)) {
            continue;
          }
          //
          // Make sure this interface has been implemented
          //
          if (ComponentName->GetControllerName != NULL) {
            Status = ComponentName->GetControllerName (
                                      ComponentName,
                                      ParentControllerHandleBuffer[HandleIndex],
                                      DeviceHandle,
                                      Language,
                                      &ControllerName
                                      );
          } else {
            Status = EFI_UNSUPPORTED;
          }

          if (EFI_ERROR (Status)) {
            continue;
          }

          if (Display) {
            if (!First) {
              Print (L"\n");
              for (Index = 0; Index < Indent; Index++) {
                Print (L" ");
              }
            }

            PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_ONE_VAR_HS_NO_SPACES), HiiEnvHandle, ControllerName);
            First = FALSE;
          }

          if (UseComponentName) {
            if (*BestDeviceName == NULL) {
              *BestDeviceName = StrDuplicate (ControllerName);
            }
          }
        }
      }

      FreePool (ChildHandleBuffer);
    }

    FreePool (ParentDriverBindingHandleBuffer);
  }

  if (UseDevicePath) {
    if (*BestDeviceName == NULL) {
      Status = BS->OpenProtocol (
                    DeviceHandle,
                    &gEfiDevicePathProtocolGuid,
                    (VOID **) &DevicePath,
                    NULL,
                    NULL,
                    EFI_OPEN_PROTOCOL_GET_PROTOCOL
                    );

      if (!EFI_ERROR (Status)) {
        *BestDeviceName = LibDevicePathToStr (DevicePath);
      }
    }
  }

  if (DriverBindingHandleCount != 0) {
    FreePool (DriverBindingHandleBuffer);
  }

  if (ParentControllerHandleCount != 0) {
    FreePool (ParentControllerHandleBuffer);
  }

  if (ChildControllerHandleCount != 0) {
    FreePool (ChildControllerHandleBuffer);

  }

  return EFI_SUCCESS;
}

EFI_STATUS
DisplayDriverModelHandle (
  IN EFI_HANDLE  Handle,
  IN BOOLEAN     BestName,
  IN CHAR8       *Language
  )
/*++

Routine Description:

Arguments:

  Handle   - The handle
  BestName - The best name
  Language - The language

Returns:

--*/
{
  EFI_STATUS                  Status;
  EFI_STATUS                  ConfigurationStatus;
  EFI_STATUS                  DiagnosticsStatus;
  UINTN                       DriverBindingHandleCount;
  EFI_HANDLE                  *DriverBindingHandleBuffer;
  UINTN                       ParentControllerHandleCount;
  EFI_HANDLE                  *ParentControllerHandleBuffer;
  UINTN                       ChildControllerHandleCount;
  EFI_HANDLE                  *ChildControllerHandleBuffer;
  CHAR16                      *BestDeviceName;
  EFI_DEVICE_PATH_PROTOCOL    *DevicePath;
  UINTN                       Index;
  CHAR16                      *DriverName;
  EFI_DRIVER_BINDING_PROTOCOL *DriverBinding;
  EFI_COMPONENT_NAME_PROTOCOL *ComponentName;
  UINTN                       NumberOfChildren;
  UINTN                       HandleIndex;
  UINTN                       ControllerHandleCount;
  EFI_HANDLE                  *ControllerHandleBuffer;
  UINTN                       ChildIndex;
  BOOLEAN                     Image;

  //
  // See if Handle is a device handle and display its details.
  //
  DriverBindingHandleBuffer = NULL;
  Status = LibGetManagingDriverBindingHandles (
            Handle,
            &DriverBindingHandleCount,
            &DriverBindingHandleBuffer
            );

  ParentControllerHandleBuffer = NULL;
  Status = LibGetParentControllerHandles (
            Handle,
            &ParentControllerHandleCount,
            &ParentControllerHandleBuffer
            );

  ChildControllerHandleBuffer = NULL;
  Status = LibGetChildControllerHandles (
            Handle,
            &ChildControllerHandleCount,
            &ChildControllerHandleBuffer
            );

  if (DriverBindingHandleCount > 0 || ParentControllerHandleCount > 0 || ChildControllerHandleCount > 0) {

    DevicePath      = NULL;
    BestDeviceName  = NULL;
    Status          = BS->HandleProtocol (Handle, &gEfiDevicePathProtocolGuid, &DevicePath);

    Print (L"\n");
    PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_CONTROLLER_NAME), HiiEnvHandle);

    Status = SEnvGetDeviceName (
              Handle,
              TRUE,
              FALSE,
              Language,
              &BestDeviceName,
              &ConfigurationStatus,
              &DiagnosticsStatus,
              TRUE,
              26
              );

    if (BestDeviceName == NULL) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_UNKNOWN_2), HiiEnvHandle);
    } else {
      FreePool (BestDeviceName);
      BestDeviceName = NULL;
    }

    Print (L"\n");

    if (DevicePath != NULL) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_DEV_PAATH), HiiEnvHandle, LibDevicePathToStr (DevicePath));
    } else {
      PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_DEV_PATH_2), HiiEnvHandle);
    }

    PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_CONTROLLER_TYPE), HiiEnvHandle);
    if (ParentControllerHandleCount == 0) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_ROOT), HiiEnvHandle);
    } else if (ChildControllerHandleCount > 0) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_HBUS), HiiEnvHandle);
    } else {
      PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_HDEVICE), HiiEnvHandle);
    }

    if (!EFI_ERROR (ConfigurationStatus)) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_CONFIG_HYES), HiiEnvHandle);
    } else {
      PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_CONFIG_HNO), HiiEnvHandle);
    }

    if (!EFI_ERROR (DiagnosticsStatus)) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_DIAG_HYES), HiiEnvHandle);
    } else {
      PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_DIAG_HNO), HiiEnvHandle);
    }

    if (DriverBindingHandleCount != 0) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_MANAGED_BY), HiiEnvHandle);
      for (Index = 0; Index < DriverBindingHandleCount; Index++) {
        Image = FALSE;
        Status = GetDriverName (
                  DriverBindingHandleBuffer[Index],
                  Language,
                  FALSE,
                  &DriverName
                  );
        if (DriverName == NULL) {
          Status = GetDriverName (
                    DriverBindingHandleBuffer[Index],
                    Language,
                    TRUE,
                    &DriverName
                    );
          if (!EFI_ERROR (Status)) {
            Image = TRUE;
          }
        }

        if (DriverName == NULL) {
          DriverName = L"<UNKNOWN>";
        }

        if (Image) {
          PrintToken (
            STRING_TOKEN (STR_SHELLENV_PROTID_DRV_IMAGE),
            HiiEnvHandle,
            SEnvGetHandleNumber (DriverBindingHandleBuffer[Index]),
            DriverName
            );
        } else {
          PrintToken (
            STRING_TOKEN (STR_SHELLENV_PROTID_DRV_HS),
            HiiEnvHandle,
            SEnvGetHandleNumber (DriverBindingHandleBuffer[Index]),
            DriverName
            );
        }
      }
    } else {
      PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_MANAGED_BY_NONE), HiiEnvHandle);
    }

    if (ParentControllerHandleCount != 0) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_PARENT_CONTROLLERS), HiiEnvHandle);
      for (Index = 0; Index < ParentControllerHandleCount; Index++) {
        Status = SEnvGetDeviceName (
                  ParentControllerHandleBuffer[Index],
                  BestName,
                  TRUE,
                  Language,
                  &BestDeviceName,
                  &ConfigurationStatus,
                  &DiagnosticsStatus,
                  FALSE,
                  0
                  );
        if (BestDeviceName == NULL) {
          BestDeviceName = StrDuplicate (L"<UNKNOWN>");
        }

        PrintToken (
          STRING_TOKEN (STR_SHELLENV_PROTID_PARENT),
          HiiEnvHandle,
          SEnvGetHandleNumber (ParentControllerHandleBuffer[Index]),
          BestDeviceName
          );
        FreePool (BestDeviceName);
        BestDeviceName = NULL;
      }
    } else {
      PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_PARENT_CONT_NONE), HiiEnvHandle);
    }

    if (ChildControllerHandleCount != 0) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_CHILD_CONTROL), HiiEnvHandle);
      for (Index = 0; Index < ChildControllerHandleCount; Index++) {
        Status = SEnvGetDeviceName (
                  ChildControllerHandleBuffer[Index],
                  BestName,
                  TRUE,
                  Language,
                  &BestDeviceName,
                  &ConfigurationStatus,
                  &DiagnosticsStatus,
                  FALSE,
                  0
                  );
        if (BestDeviceName == NULL) {
          BestDeviceName = StrDuplicate (L"<UNKNOWN>");
        }

        PrintToken (
          STRING_TOKEN (STR_SHELLENV_PROTID_CHILD),
          HiiEnvHandle,
          SEnvGetHandleNumber (ChildControllerHandleBuffer[Index]),
          BestDeviceName
          );
        FreePool (BestDeviceName);
        BestDeviceName = NULL;
      }
    } else {
      PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_CHILD_CONTROL_NONE), HiiEnvHandle);
    }
  }

  Status = EFI_SUCCESS;

  if (DriverBindingHandleBuffer) {
    FreePool (DriverBindingHandleBuffer);
  }

  if (ParentControllerHandleBuffer) {
    FreePool (ParentControllerHandleBuffer);
  }

  if (ChildControllerHandleBuffer) {
    FreePool (ChildControllerHandleBuffer);
  }

  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // See if Handle is a driver binding handle and display its details.
  //
  Status = BS->OpenProtocol (
                Handle,
                &gEfiDriverBindingProtocolGuid,
                (VOID **) &DriverBinding,
                NULL,
                NULL,
                EFI_OPEN_PROTOCOL_GET_PROTOCOL
                );
  if (EFI_ERROR (Status)) {
    return EFI_SUCCESS;
  }

  ComponentName = NULL;
  Status = BS->OpenProtocol (
                Handle,
                &gEfiComponentNameProtocolGuid,
                (VOID **) &ComponentName,
                NULL,
                NULL,
                EFI_OPEN_PROTOCOL_GET_PROTOCOL
                );

  DiagnosticsStatus = BS->OpenProtocol (
                            Handle,
                            &gEfiDriverDiagnosticsProtocolGuid,
                            NULL,
                            NULL,
                            NULL,
                            EFI_OPEN_PROTOCOL_TEST_PROTOCOL
                            );

  ConfigurationStatus = BS->OpenProtocol (
                              Handle,
                              &gEfiDriverConfigurationProtocolGuid,
                              NULL,
                              NULL,
                              NULL,
                              EFI_OPEN_PROTOCOL_TEST_PROTOCOL
                              );

  NumberOfChildren        = 0;
  ControllerHandleBuffer  = NULL;
  Status = LibGetManagedControllerHandles (
            Handle,
            &ControllerHandleCount,
            &ControllerHandleBuffer
            );
  if (ControllerHandleCount > 0) {
    for (HandleIndex = 0; HandleIndex < ControllerHandleCount; HandleIndex++) {
      Status = LibGetManagedChildControllerHandles (
                Handle,
                ControllerHandleBuffer[HandleIndex],
                &ChildControllerHandleCount,
                NULL
                );
      NumberOfChildren += ChildControllerHandleCount;
    }
  }

  Status = GetDriverName (
            Handle,
            Language,
            FALSE,
            &DriverName
            );
  Print (L"\n");

  if (DriverName != NULL) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_DRIVER_NAME), HiiEnvHandle, DriverName);
  } else {
    PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_DRIVER_NAME_NONE), HiiEnvHandle);
  }

  Status = GetDriverName (
            Handle,
            Language,
            TRUE,
            &DriverName
            );
  if (DriverName != NULL) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_IAMGE_NAME), HiiEnvHandle, DriverName);
  } else {
    PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_IMAGE_NAME_NONE), HiiEnvHandle);
  }

  PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_DRIVER_VERSION), HiiEnvHandle, DriverBinding->Version);

  if (NumberOfChildren > 0) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_DRIVER_TYPE_HBUS), HiiEnvHandle);
  } else if (ControllerHandleCount > 0) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_DRIVER_TYPE_HDEVICE), HiiEnvHandle);
  } else {
    PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_DRIVER_TYPE_UNKNOWN), HiiEnvHandle);
  }

  if (!EFI_ERROR (ConfigurationStatus)) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_CONFIG_HYES_2), HiiEnvHandle);
  } else {
    PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_CONFIG_HNO_2), HiiEnvHandle);
  }

  if (!EFI_ERROR (DiagnosticsStatus)) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_DIAGNOSTICS_HYES), HiiEnvHandle);
  } else {
    PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_DIAGNOSTICS_HNO), HiiEnvHandle);
  }

  if (ControllerHandleCount == 0) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_MANAGING_NONE), HiiEnvHandle);
  } else {
    PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_MANAGING), HiiEnvHandle);
    for (HandleIndex = 0; HandleIndex < ControllerHandleCount; HandleIndex++) {
      Status = SEnvGetDeviceName (
                ControllerHandleBuffer[HandleIndex],
                BestName,
                TRUE,
                Language,
                &BestDeviceName,
                &ConfigurationStatus,
                &DiagnosticsStatus,
                FALSE,
                0
                );
      if (BestDeviceName == NULL) {
        BestDeviceName = StrDuplicate (L"<UNKNOWN>");
      }

      PrintToken (
        STRING_TOKEN (STR_SHELLENV_PROTID_CTRL_HS),
        HiiEnvHandle,
        SEnvGetHandleNumber (ControllerHandleBuffer[HandleIndex]),
        BestDeviceName
        );
      FreePool (BestDeviceName);
      BestDeviceName = NULL;

      Status = LibGetManagedChildControllerHandles (
                Handle,
                ControllerHandleBuffer[HandleIndex],
                &ChildControllerHandleCount,
                &ChildControllerHandleBuffer
                );
      if (!EFI_ERROR (Status)) {
        for (ChildIndex = 0; ChildIndex < ChildControllerHandleCount; ChildIndex++) {
          Status = SEnvGetDeviceName (
                    ChildControllerHandleBuffer[ChildIndex],
                    BestName,
                    TRUE,
                    Language,
                    &BestDeviceName,
                    &ConfigurationStatus,
                    &DiagnosticsStatus,
                    FALSE,
                    0
                    );
          if (BestDeviceName == NULL) {
            BestDeviceName = StrDuplicate (L"<UNKNOWN>");
          }

          PrintToken (
            STRING_TOKEN (STR_SHELLENV_PROTID_CHILD_HS),
            HiiEnvHandle,
            SEnvGetHandleNumber (ChildControllerHandleBuffer[ChildIndex]),
            BestDeviceName
            );
          FreePool (BestDeviceName);
          BestDeviceName = NULL;
        }

        FreePool (ChildControllerHandleBuffer);
      }
    }

    FreePool (ControllerHandleBuffer);
  }

  return EFI_SUCCESS;
}

EFI_STATUS
SEnvDHProt (
  IN BOOLEAN          Verbose,
  IN BOOLEAN          DriverModel,
  IN UINTN            HandleNo,
  IN EFI_HANDLE       Handle,
  IN CHAR8            *Language
  )
/*++

Routine Description:

Arguments:

  Verbose     - Verbose
  DriverModel - Driver model
  HandleNo    - Handle number
  Handle      - The handle
  Language    - The language

Returns:

--*/
{
  PROTOCOL_INFO               *Prot;
  EFI_LIST_ENTRY              *Link;
  VOID                        *Interface;
  UINTN                       Index;
  UINTN                       Index1;
  EFI_STATUS                  Status;
  SHELLENV_DUMP_PROTOCOL_INFO Dump;
  EFI_GUID                    **ProtocolBuffer;
  UINTN                       ProtocolBufferCount;

  if (!HandleNo) {
    HandleNo = SEnvGetHandleNumber (Handle);
  }
  //
  // Get protocol info by handle
  //
  ProtocolBuffer = NULL;
  Status = BS->ProtocolsPerHandle (
                Handle,
                &ProtocolBuffer,
                &ProtocolBufferCount
                );
  if (EFI_ERROR (Status)) {
    return EFI_SUCCESS;
  }

  if (Verbose) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_NHANDLE), HiiEnvHandle, HandleNo, Handle);
  } else {
    PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_TWO_VARS_N), HiiEnvHandle, HandleNo, Handle);
  }

  for (Link = SEnvProtocolInfo.Flink; Link != &SEnvProtocolInfo; Link = Link->Flink) {
    Prot = CR (Link, PROTOCOL_INFO, Link, PROTOCOL_INFO_SIGNATURE);
    for (Index = 0; Index < Prot->NoHandles; Index++) {
      //
      // If this handle supports this protocol, dump it
      //
      if (GetExecutionBreak ()) {
        Status = EFI_ABORTED;
        goto Done;
      }

      if (Prot->Handles[Index] == Handle) {
        Dump    = Verbose ? Prot->DumpInfo : Prot->DumpToken;
        Status  = BS->HandleProtocol (Handle, &Prot->ProtocolId, &Interface);
        if (!EFI_ERROR (Status)) {
          if (Verbose) {
            for (Index1 = 0; Index1 < ProtocolBufferCount; Index1++) {
              if (ProtocolBuffer[Index1] != NULL) {
                if (CompareGuid (ProtocolBuffer[Index1], &Prot->ProtocolId) == 0) {
                  ProtocolBuffer[Index1] = NULL;
                }
              }
            }
            //
            // Dump verbose info
            //
            PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_TWO_VARS_HS), HiiEnvHandle, Prot->IdString, Interface);
            if (Dump != NULL) {
              Dump (Handle, Interface);
            }

            Print (L"\n");
          } else {
            if (Dump != NULL) {
              Dump (Handle, Interface);
            } else {
              PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_ONE_VAR_HS), HiiEnvHandle, Prot->IdString);
            }
          }
        }
      }
    }
  }

  if (DriverModel) {
    Status = DisplayDriverModelHandle (Handle, TRUE, Language);
    if (EFI_ERROR (Status)) {
      goto Done;
    }
  }
  //
  // Dump verbose info
  //
  if (Verbose) {
    for (Index1 = 0; Index1 < ProtocolBufferCount; Index1++) {
      if (ProtocolBuffer[Index1] != NULL) {
        if (GetExecutionBreak ()) {
          Status = EFI_ABORTED;
          goto Done;
        }

        Status = BS->HandleProtocol (Handle, ProtocolBuffer[Index1], &Interface);
        if (!EFI_ERROR (Status)) {
          PrintToken (
            STRING_TOKEN (STR_SHELLENV_PROTID_TWO_VARS_HG_NEW),
            HiiEnvHandle,
            ProtocolBuffer[Index1],
            Interface
            );
        }
      }
    }

    PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_ONE_VAR_N), HiiEnvHandle);
  } else {
    PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_ONE_VAR_N_NEWLINE), HiiEnvHandle);
  }

  Status = EFI_SUCCESS;

Done:
  if (ProtocolBuffer != NULL) {
    FreePool (ProtocolBuffer);
  }

  return Status;
}

EFI_STATUS
EFIAPI
SEnvCmdDH (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  )
/*++

Routine Description:

  Code for internal "DH" command

Arguments:

  ImageHandle - The image handle
  SystemTable - The system table

Returns:

--*/
{
  BOOLEAN                 ByProtocol;
  CHAR16                  *Arg;
  UINTN                   ArgVar;
  EFI_STATUS              Status;
  UINTN                   Index;
  PROTOCOL_INFO           *Prot;
  BOOLEAN                 Verbose;
  BOOLEAN                 DriverModel;
  UINTN                   StringIndex;
  CHAR8                   *Language;
  SHELL_VAR_CHECK_CODE    RetCode;
  CHAR16                  *Useful;
  SHELL_ARG_LIST          *Item;
  SHELL_VAR_CHECK_PACKAGE ChkPck;

  //
  // Initialize
  //
  EFI_SHELL_APP_INIT (ImageHandle, SystemTable);

  Arg         = NULL;
  ArgVar      = 0;
  ByProtocol  = FALSE;
  Status      = EFI_SUCCESS;
  Language    = LibGetVariable (VarLanguage, &gEfiGlobalVariableGuid);
  ZeroMem (&ChkPck, sizeof (SHELL_VAR_CHECK_PACKAGE));

  if (!EFI_PROPER_VERSION (1, 10)) {
    PrintToken (
      STRING_TOKEN (STR_SHELLENV_GNC_COMMAND_NOT_SUPPORT),
      HiiEnvHandle,
      L"dh",
      EFI_VERSION_1_10
      );
    Status = EFI_UNSUPPORTED;
    goto Done;
  }

  if (Language == NULL) {
    Language    = AllocatePool (4);
    Language[0] = 'e';
    Language[1] = 'n';
    Language[2] = 'g';
    Language[3] = 0;
  }

  RetCode = LibCheckVariables (SI, DHCheckList, &ChkPck, &Useful);
  if (VarCheckOk != RetCode) {
    switch (RetCode) {
    case VarCheckConflict:

      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_FLAG_CONFLICT), HiiEnvHandle, L"dh", Useful);
      break;

    case VarCheckDuplicate:

      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_DUP_FLAG), HiiEnvHandle, L"dh", Useful);
      break;

    case VarCheckLackValue:

      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_LACK_ARG), HiiEnvHandle, L"dh", Useful);
      break;

    case VarCheckUnknown:

      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_UNKNOWN_FLAG), HiiEnvHandle, L"dh", Useful);
      break;

    default:
      break;
    }
    //
    // end of switch(RetCode)
    //
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }
  //
  // Crack args
  //
  Item = LibCheckVarGetFlag (&ChkPck, L"-b");
  if (Item) {
    EnablePageBreak (DEFAULT_INIT_ROW, DEFAULT_AUTO_LF);
  }

  if (LibCheckVarGetFlag (&ChkPck, L"-?") != NULL) {
    if (IS_OLD_SHELL) {
      PrintToken (STRING_TOKEN (STR_NO_HELP), HiiEnvHandle);
      goto Done;
    }

    if (ChkPck.ValueCount > 0 ||
        ChkPck.FlagCount > 2 ||
        (2 == ChkPck.FlagCount && !LibCheckVarGetFlag (&ChkPck, L"-b"))
        ) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiEnvHandle, L"dh");
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }

    PrintToken (STRING_TOKEN (STR_SHELLENV_DH_VERBOSE_HELP), HiiEnvHandle);
    Status = EFI_SUCCESS;
    goto Done;
  }

  Verbose     = FALSE;
  DriverModel = FALSE;
  if (ChkPck.ValueCount > 1) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiEnvHandle, L"dh");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  Item = LibCheckVarGetFlag (&ChkPck, L"-l");
  if (Item) {
    for (Index = 0; SLanguageTable[Index].IdString; Index++) {
      if (StriCmp (Item->VarStr, SLanguageTable[Index].IdString) == 0) {
        break;
      }
    }

    if (SLanguageTable[Index].IdString == NULL) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_DH_BAD_LANG), HiiEnvHandle, L"dh", Item->VarStr);
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    } else {
      for (StringIndex = 0; StringIndex < 3; StringIndex++) {
        Language[StringIndex] = (CHAR8) Item->VarStr[StringIndex];
      }
    }
  }

  Item = LibCheckVarGetFlag (&ChkPck, L"-d");
  if (Item) {
    DriverModel = TRUE;
  }

  Item = LibCheckVarGetFlag (&ChkPck, L"-v");
  if (Item) {
    Verbose = TRUE;
  }

  Item = LibCheckVarGetFlag (&ChkPck, L"-p");
  if (Item) {
    Arg         = Item->VarStr;
    ByProtocol  = TRUE;
    if (ChkPck.ValueCount > 0) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiEnvHandle, L"dh");
      LibCheckVarFreeVarList (&ChkPck);
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }
  } else {
    Item = ChkPck.VarList;
    if (Item) {
      ArgVar = (UINTN) StrToUIntegerBase (Item->VarStr, 16, &Status);
      if (EFI_ERROR (Status)) {
        PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_ARG), HiiEnvHandle, L"dh", Item->VarStr);
        Status = EFI_INVALID_PARAMETER;
        goto Done;
      }

      Arg = Item->VarStr;
    }
  }
  //
  //
  // Load handle & protocol info tables
  //
  SEnvLoadHandleTable ();
  SEnvLoadHandleProtocolInfo (NULL);

  if (Arg) {
    if (ByProtocol) {
      AcquireLock (&SEnvGuidLock);
      Prot = SEnvGetProtByStr (Arg);
      ReleaseLock (&SEnvGuidLock);

      if (Prot) {
        //
        // Dump the handles on this protocol
        //
        PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_NHANDLE_DUMP), HiiEnvHandle, Prot->IdString);
        for (Index = 0; Index < Prot->NoHandles; Index++) {
          Status = SEnvDHProt (Verbose, DriverModel, 0, Prot->Handles[Index], Language);
          if (EFI_ERROR (Status)) {
            goto Done;
          }

        }
      } else {
        PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_PROTOCOL_NOT_FOUND), HiiEnvHandle, L"dh", Arg);
      }
    } else {
      //
      // Dump 1 handle
      //
      Index = SEnvHandleNoFromUINT (ArgVar) - 1;
      if (Index > SEnvNoHandles) {
        PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_HANDLE_OUT_OF_RANGE), HiiEnvHandle, L"dh", Arg);
        Status = EFI_INVALID_PARAMETER;
        goto Done;
      } else {
        if (DriverModel) {
          Status = SEnvDHProt (
                    Verbose,
                    DriverModel,
                    Index + 1,
                    SEnvHandles[Index],
                    Language
                    );

        } else {
          Status = SEnvDHProt (
                    TRUE,
                    DriverModel,
                    Index + 1,
                    SEnvHandles[Index],
                    Language
                    );
        }

        if (EFI_ERROR (Status)) {
          goto Done;
        }
      }
    }
  } else {
    //
    // Dump all handles
    //
    PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_HANDLE_DUMP), HiiEnvHandle);
    for (Index = 0; Index < SEnvNoHandles; Index++) {
      Status = SEnvDHProt (
                Verbose,
                DriverModel,
                Index + 1,
                SEnvHandles[Index],
                Language
                );
      if (EFI_ERROR (Status)) {
        goto Done;
      }
    }
  }

  Status = EFI_SUCCESS;

Done:
  if (Language != NULL) {
    FreePool (Language);
  }

  SEnvFreeHandleTable ();
  LibCheckVarFreeVarList (&ChkPck);
  return Status;
}

EFI_STATUS
SEnvDeviceTree (
  IN EFI_HANDLE       ControllerHandle,
  IN UINTN            Level,
  IN BOOLEAN          RootOnly,
  IN BOOLEAN          BestName,
  IN CHAR8            *Language
  )
/*++

Routine Description:

Arguments:

  ControllerHandle - The Controller handle
  Level            - The level
  RootOnly         - Root only
  BestName         - The best Name
  Language         - Language

Returns:

--*/
{
  EFI_STATUS                  Status;
  EFI_LOADED_IMAGE_PROTOCOL   *Image;
  EFI_DRIVER_BINDING_PROTOCOL *DriverBinding;
  UINT32                      ControllerHandleIndex;
  UINTN                       HandleCount;
  EFI_HANDLE                  *HandleBuffer;
  UINT32                      *HandleType;
  UINTN                       HandleIndex;
  UINTN                       ChildIndex;
  EFI_DEVICE_PATH_PROTOCOL    *DevicePath;
  UINTN                       ChildHandleCount;
  EFI_HANDLE                  *ChildHandleBuffer;
  UINT32                      *ChildHandleType;
  UINTN                       Index;
  BOOLEAN                     Root;
  EFI_STATUS                  ConfigurationStatus;
  EFI_STATUS                  DiagnosticsStatus;
  CHAR16                      *DeviceName;

  Status = BS->OpenProtocol (
                ControllerHandle,
                &gEfiDriverBindingProtocolGuid,
                (VOID **) &DriverBinding,
                NULL,
                NULL,
                EFI_OPEN_PROTOCOL_GET_PROTOCOL
                );
  if (!EFI_ERROR (Status)) {
    return EFI_SUCCESS;
  }

  Status = BS->OpenProtocol (
                ControllerHandle,
                &gEfiLoadedImageProtocolGuid,
                (VOID **) &Image,
                NULL,
                NULL,
                EFI_OPEN_PROTOCOL_GET_PROTOCOL
                );
  if (!EFI_ERROR (Status)) {
    return EFI_SUCCESS;
  }

  Status = LibScanHandleDatabase (
            NULL,
            NULL,
            ControllerHandle,
            &ControllerHandleIndex,
            &HandleCount,
            &HandleBuffer,
            &HandleType
            );
  if (EFI_ERROR (Status)) {
    return EFI_SUCCESS;
  }

  if (RootOnly) {
    Root = TRUE;
    for (HandleIndex = 0; HandleIndex < HandleCount; HandleIndex++) {
      if (HandleType[HandleIndex] & EFI_HANDLE_TYPE_PARENT_HANDLE) {
        Root = FALSE;
      }
    }

    Status = BS->OpenProtocol (
                  ControllerHandle,
                  &gEfiDevicePathProtocolGuid,
                  (VOID **) &DevicePath,
                  NULL,
                  NULL,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
    if (EFI_ERROR (Status)) {
      Root = FALSE;
    }

    if (!Root) {
      return EFI_SUCCESS;
    }
  }
  //
  // Display the handle specified by ControllerHandle
  //
  for (Index = 0; Index < Level; Index++) {
    Print (L"  ");
  }

  PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_CTRL), HiiEnvHandle, SEnvHandleToNumber (ControllerHandle));

  Status = SEnvGetDeviceName (
            ControllerHandle,
            BestName,
            TRUE,
            Language,
            &DeviceName,
            &ConfigurationStatus,
            &DiagnosticsStatus,
            FALSE,
            0
            );
  if (DeviceName != NULL) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_ONE_VAR_S), HiiEnvHandle, DeviceName);
  } else {
    PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_UNKNOWN_2), HiiEnvHandle);
  }
  //
  // Print the list of drivers that are managing this controller
  //
  for (HandleIndex = 0; HandleIndex < HandleCount; HandleIndex++) {
    if (HandleType[HandleIndex] & EFI_HANDLE_TYPE_DRIVER_BINDING_HANDLE &&
        HandleType[HandleIndex] & EFI_HANDLE_TYPE_DEVICE_DRIVER
        ) {

      DriverBinding = NULL;
      Status = BS->OpenProtocol (
                    HandleBuffer[HandleIndex],
                    &gEfiDriverBindingProtocolGuid,
                    (VOID **) &DriverBinding,
                    NULL,
                    NULL,
                    EFI_OPEN_PROTOCOL_GET_PROTOCOL
                    );

      Status = LibScanHandleDatabase (
                HandleBuffer[HandleIndex],
                NULL,
                ControllerHandle,
                &ControllerHandleIndex,
                &ChildHandleCount,
                &ChildHandleBuffer,
                &ChildHandleType
                );

      if (!EFI_ERROR (Status)) {
        for (ChildIndex = 0; ChildIndex < ChildHandleCount; ChildIndex++) {
          if (ChildHandleType[ChildIndex] & EFI_HANDLE_TYPE_CHILD_HANDLE &&
              ChildHandleType[ChildIndex] & EFI_HANDLE_TYPE_DEVICE_HANDLE
              ) {

            Status = SEnvDeviceTree (
                      ChildHandleBuffer[ChildIndex],
                      Level + 1,
                      FALSE,
                      BestName,
                      Language
                      );
            if (EFI_ERROR (Status)) {
              return Status;
            }
          }
        }

        FreePool (ChildHandleBuffer);
        FreePool (ChildHandleType);
      }
    }
  }

  FreePool (HandleBuffer);
  FreePool (HandleType);

  return EFI_SUCCESS;
}

EFI_STATUS
SEnvCmdDriverConfigurationProcessActionRequired (
  EFI_HANDLE                                DriverImageHandle,
  EFI_HANDLE                                ControllerHandle,
  EFI_HANDLE                                ChildHandle,
  EFI_DRIVER_CONFIGURATION_ACTION_REQUIRED  ActionRequired
  )

{
  CHAR16      ReturnStr[2];
  EFI_HANDLE  ContextOverride[2];

  switch (ActionRequired) {
  case EfiDriverConfigurationActionNone:
    PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_NONE), HiiEnvHandle);
    break;

  case EfiDriverConfigurationActionStopController:
    PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_STOP_CONTROLLER), HiiEnvHandle);
    PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_ENTER_STOP_CONTROL), HiiEnvHandle);
    Input (L"", ReturnStr, sizeof (ReturnStr) / sizeof (CHAR16));
    BS->DisconnectController (ControllerHandle, DriverImageHandle, ChildHandle);
    PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_CONTROLLER_STOPPED), HiiEnvHandle);
    break;

  case EfiDriverConfigurationActionRestartController:
    PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_RESTART_CONTROLLER), HiiEnvHandle);
    PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_ENTER_RESTART_CONTROLLER), HiiEnvHandle);
    Input (L"", ReturnStr, sizeof (ReturnStr) / sizeof (CHAR16));
    BS->DisconnectController (ControllerHandle, DriverImageHandle, ChildHandle);
    ContextOverride[0]  = DriverImageHandle;
    ContextOverride[1]  = NULL;
    BS->ConnectController (ControllerHandle, ContextOverride, NULL, TRUE);
    PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_CONTROLLER_RESTARTED), HiiEnvHandle);
    break;

  case EfiDriverConfigurationActionRestartPlatform:
    PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_RESTART_PLATFORM), HiiEnvHandle);
    PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_ENTER_RESTART_PLATFORM), HiiEnvHandle);
    Input (L"", ReturnStr, sizeof (ReturnStr) / sizeof (CHAR16));
    RT->ResetSystem (EfiResetCold, EFI_SUCCESS, 0, NULL);
    break;

  default:
    PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_UNKNOWN_3), HiiEnvHandle);
    break;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
SEnvCmdDriverRunDiagnostics (
  EFI_DRIVER_DIAGNOSTICS_PROTOCOL   *DriverDiagnostics,
  EFI_HANDLE                        ControllerHandle,
  EFI_HANDLE                        ChildHandle,
  EFI_DRIVER_DIAGNOSTIC_TYPE        DiagnosticType,
  CHAR8                             *Language
  )

{
  EFI_STATUS  Status;
  EFI_GUID    *ErrorType;
  UINTN       BufferSize;
  CHAR16      *Buffer;

  ErrorType   = NULL;
  BufferSize  = 0;
  Buffer      = NULL;
  Status = DriverDiagnostics->RunDiagnostics (
                                DriverDiagnostics,
                                ControllerHandle,
                                ChildHandle,
                                DiagnosticType,
                                Language,
                                &ErrorType,
                                &BufferSize,
                                &Buffer
                                );

  if (!EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_HPASSED), HiiEnvHandle);
  } else {
    PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_HFAILED), HiiEnvHandle, Status);
  }

  if (ErrorType != NULL) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_ERRORTYPE), HiiEnvHandle, ErrorType);
  }

  if (BufferSize > 0) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_BUFFERSIZE), HiiEnvHandle, BufferSize);
    PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_BUFFER_ONE_VAR), HiiEnvHandle, Buffer);
    DumpHex (2, 0, BufferSize, Buffer);
    FreePool (Buffer);
  }

  return EFI_SUCCESS;
}

extern EFI_LIST_ENTRY SEnvMap;
extern EFI_LIST_ENTRY SEnvEnv;
extern EFI_LIST_ENTRY SEnvAlias;

EFI_STATUS
EFIAPI
SEnvLoadDefaults (
  IN EFI_HANDLE           Image,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
/*++

Routine Description:

Arguments:

  Image       - The image handle
  SystemTable - The system table

Returns:

--*/
{
  //
  // These commands are guaranteed to be internal commands
  //
  SEnvExecute (Image, L"@alias copy cp", TRUE);
  SEnvExecute (Image, L"@alias del rm", TRUE);
  SEnvExecute (Image, L"@alias dir ls", TRUE);
  SEnvExecute (Image, L"@alias md mkdir", TRUE);
  SEnvExecute (Image, L"@alias rd rm", TRUE);
  SEnvExecute (Image, L"@set -v efishellmode "EFI_SHELL_ENHANCED_MODE_VER, TRUE);
  SEnvExecute (Image, L"@alias cr \"echo Current Running Mode: ^%efishellmode^%\"", TRUE);

  return EFI_SUCCESS;
}

VOID
SEnvGetDevicePathList (
  EFI_LIST_ENTRY    *ListHead,
  EFI_GUID          *Protocol
  )
/*++

Routine Description:
  Get current device paths by specified protocol

Arguments:

  ListHead - The list head
  Protocol - The protocol

Returns:

--*/
{
  EFI_STATUS                Status;

  DEVICEPATH_INFO           *DevicePathInfo;

  UINTN                     Index;
  UINTN                     NoHandles;
  EFI_HANDLE                *Handles;

  EFI_DEVICE_PATH_PROTOCOL  *DevicePath;
  UINTN                     DevPathSize;

  //
  // Remove previous device path info
  //
  while (!IsListEmpty (ListHead)) {
    DevicePathInfo = (DEVICEPATH_INFO *) (CR (
                                            ListHead->Flink,
                                            DEVICEPATH_INFO,
                                            Link,
                                            DEVICEPATH_INFO_SIGNATURE
                                            ));
    FreePool (DevicePathInfo->DevicePath);
    RemoveEntryList (&DevicePathInfo->Link);
    FreePool (DevicePathInfo);
  }
  //
  // Establish current device path info list
  //
  Status = LibLocateHandle (
            ByProtocol,
            Protocol,
            NULL,
            &NoHandles,
            &Handles
            );

  if (NoHandles) {
    for (Index = 0; Index < NoHandles; Index++) {
      DevicePath = DevicePathFromHandle (Handles[Index]);
      if (!DevicePath) {
        continue;
      }

      DevPathSize = DevicePathSize (DevicePath);

      //
      // Copy device path info to the device path list
      //
      DevicePathInfo              = AllocateZeroPool (sizeof (DEVICEPATH_INFO));
      DevicePathInfo->DevicePath  = AllocateZeroPool (DevPathSize);
      CopyMem (DevicePathInfo->DevicePath, DevicePath, DevPathSize);
      DevicePathInfo->Found     = FALSE;
      DevicePathInfo->Signature = DEVICEPATH_INFO_SIGNATURE;
      InsertTailList (ListHead, &DevicePathInfo->Link);
    }
  }
}

BOOLEAN
SEnvCompareDevicePathList (
  EFI_LIST_ENTRY    *ListHead1,
  EFI_LIST_ENTRY    *ListHead2
  )
/*++

Routine Description:
  Compare devicepath list

Arguments:

  ListHead1 - The first list
  ListHead2 - Thes second list

Returns:

--*/
{
  EFI_LIST_ENTRY  *Link1;
  EFI_LIST_ENTRY  *Link2;

  DEVICEPATH_INFO *DevicePathInfo1;
  DEVICEPATH_INFO *DevicePathInfo2;

  if (IsListEmpty (ListHead1) || IsListEmpty (ListHead2)) {
    return FALSE;
  }
  //
  // Compare two device path lists
  //
  for (Link1 = ListHead1->Flink; Link1 != ListHead1; Link1 = Link1->Flink) {
    DevicePathInfo1 = CR (Link1, DEVICEPATH_INFO, Link, DEVICEPATH_INFO_SIGNATURE);
    for (Link2 = ListHead2->Flink; Link2 != ListHead2; Link2 = Link2->Flink) {
      DevicePathInfo2 = CR (Link2, DEVICEPATH_INFO, Link, DEVICEPATH_INFO_SIGNATURE);
      if (DevicePathCompare (DevicePathInfo1->DevicePath, DevicePathInfo2->DevicePath) == 0) {
        DevicePathInfo1->Found  = TRUE;
        DevicePathInfo2->Found  = TRUE;
        break;
      }
    }

    if (!DevicePathInfo1->Found) {
      //
      // Devicepath node in devicepath1 is not in devicepath2 list,
      // don't need to continue comparing
      //
      return FALSE;
    }
  }
  //
  // All devicepath nodes in devicepath1 list are in devicepath2 list,
  // so we just need to find if there are new device path in devicepath2 list
  //
  for (Link2 = ListHead2->Flink; Link2 != ListHead2; Link2 = Link2->Flink) {
    DevicePathInfo2 = CR (
                        Link2,
                        DEVICEPATH_INFO,
                        Link,
                        DEVICEPATH_INFO_SIGNATURE
                        );
    if (!DevicePathInfo2->Found) {
      return FALSE;
    }
  }

  return TRUE;
}
//
// functions to enumerate protocol database
//
EFI_LIST_ENTRY  *ProtocolInfoEnum;

VOID
SEnvInitProtocolInfoEnumerator (
  VOID
  )
{
  SEnvLoadHandleTable ();
  SEnvLoadHandleProtocolInfo (NULL);
  ProtocolInfoEnum = &SEnvProtocolInfo;
}

EFI_STATUS
SEnvNextProtocolInfo (
  IN OUT   PROTOCOL_INFO            **ProtocolInfo
  )
{
  ProtocolInfoEnum = ProtocolInfoEnum->Flink;
  if (ProtocolInfoEnum == &SEnvProtocolInfo) {
    *ProtocolInfo = NULL;
    return EFI_SUCCESS;
  }

  if (ProtocolInfoEnum != &SEnvProtocolInfo) {
    *ProtocolInfo = CR (ProtocolInfoEnum, PROTOCOL_INFO, Link, PROTOCOL_INFO_SIGNATURE);
  }

  return EFI_SUCCESS;
}

EFI_STATUS
SEnvSkipProtocolInfo (
  IN UINTN                  SkipNum
  )
{

  return EFI_SUCCESS;
}

VOID
SEnvResetProtocolInfoEnumerator (
  VOID
  )
{
  ProtocolInfoEnum = &SEnvProtocolInfo;
}

VOID
SEnvCloseProtocolInfoEnumerator (
  VOID
  )
{
  SEnvFreeHandleTable ();
  ProtocolInfoEnum = NULL;
}

EFI_STATUS
EFIAPI
SEnvCmdDHGetLineHelp (
  IN CHAR16                   **Str
  )
/*++

Routine Description:

  Get this command's line help

Arguments:

  Str - The line help

Returns:

  EFI_SUCCESS   - Success

--*/
{
  return SEnvCmdGetStringByToken (STRING_TOKEN (STR_SHELLENV_DH_LINE_HELP), Str);
}
