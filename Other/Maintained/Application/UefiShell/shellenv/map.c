/*++

Copyright (c) 2005 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  map.c
  
Abstract:

  Shell environment short device name mapping information management

Revision History

--*/

#include "shelle.h"
#include "shellenvguid.h"
#define DEFAULT_MAPPING_LINK_HEADER_SIGNATURE EFI_SIGNATURE_32 ('d', 'm', 'l', 'h')

typedef struct {
  UINT32          Signature;
  BOOLEAN         Dirty;
  EFI_LIST_ENTRY  BlkMappingHead;
  EFI_LIST_ENTRY  FsntMappingHead;
} DEFAULT_MAPPING_LINK_HEADER;

#define DEFAULT_MAPPING_LINK_NODE_SIGNATURE EFI_SIGNATURE_32 ('d', 'm', 'l', 'n')

typedef struct {
  UINT32                    Signature;
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath;
  EFI_LIST_ENTRY            Link;
} DEFAULT_MAPPING_LINK_NODE;

#define DEFAULT_MAPPING_TABLE_REVISION  0x00010000

typedef struct {
  UINT32  Revision;
  UINTN   BlkMappingTableLen;
  UINTN   FsntMappingTableLen;
} DEFAULT_MAPPING_TABLE_HEADER;

#define DEFAULT_MAPPING_TABLE_GUID \
  { \
    0xFC60F54F, 0x2624, 0x4834, 0xBE, 0x6E, 0xEA, 0x1C, 0xC9, 0x9D, 0x13, 0x92 \
  }

typedef struct {
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath;
  CHAR16                    *ConsistMappingName;
  CHAR16                    *FSName;
  CHAR16                    *BlkName;
  EFI_HANDLE                Handle;
  UINTN                     MediaType;
  BOOLEAN                   Removeable;
  BOOLEAN                   BlkDevice;
  BOOLEAN                   FSDevice;
} MAPPING_NAME_INFO;

#define MediaFloppy         0x01
#define MediaHardDisk       0x02
#define MediaCDRom          0x04
#define MediaUnknown        0x08
#define MediaTypeAll        (MediaFloppy | MediaHardDisk | MediaCDRom | MediaUnknown)

#define ShowNull            0
#define ShowConsistMapping  0x01
#define ShowNormal          0x02
#define ShowAll             (ShowConsistMapping | ShowNormal)

typedef struct {
  UINTN   MediaType;
  CHAR16  *Name;
  CHAR16  *ShowName;
} MEDIA_NAME;

MEDIA_NAME            mMediaName[] = {
  {
    MediaFloppy,
    L"fp",
    L"Floppy"
  },
  {
    MediaHardDisk,
    L"hd",
    L"HardDisk"
  },
  {
    MediaCDRom,
    L"cd",
    L"CDRom"
  },
  {
    MediaUnknown,
    L"f",
    L"BlockDevice"
  },
  {
    0,
    NULL
  }
};

SHELL_VAR_CHECK_ITEM    MapCheckList[] = {
  {
    L"-r",
    0x01,
    0x4,
    FlagTypeSingle
  },
  {
    L"-v",
    0x02,
    0x4,
    FlagTypeSingle
  },
  {
    L"-d",
    0x04,
    0x3b,
    FlagTypeNeedVar
  },
  {
    L"-c",
    0x08,
    0x4,
    FlagTypeSingle
  },
  {
    L"-f",
    0x10,
    0x4,
    FlagTypeSingle
  },
  {
    L"-t",
    0x20,
    0x4,
    FlagTypeNeedVar
  },
  {
    L"-b",
    0x40,
    0,
    FlagTypeSingle
  },
  {
    L"-?",
    0x80,
    0,
    FlagTypeSingle
  },
  {
    NULL,
    0,
    0,
    0
  }
};

SHELL_VAR_CHECK_ITEM      CdCheckList[] = {
  {
    L"-b",
    0x01,
    0,
    FlagTypeSingle
  },
  {
    L"-?",
    0x02,
    0,
    FlagTypeSingle
  },
  {
    NULL,
    0,
    0,
    0
  }
};

EFI_GUID              SEnvDefaultMappingTableGuid = DEFAULT_MAPPING_TABLE_GUID;

//
// Remember current mapping
//
extern EFI_LIST_ENTRY SEnvCurMapping;

extern EFI_LIST_ENTRY SEnvMap;

STATIC CHAR16         *SEnvCurDevice;

//
// Private worker function prototypes
//
STATIC
BOOLEAN
IsValidName (
  IN CHAR16           *Name
  );

EFI_STATUS
ParseMediaTypeStr (
  IN  CHAR16           *Str,
  OUT UINTN            *MediaType
  );

VOID
AddDefaultMappingLinkNode (
  IN OUT EFI_LIST_ENTRY          *Head,
  IN EFI_DEVICE_PATH_PROTOCOL    *DevicePath
  );

BOOLEAN
FindDefaultMappingIndex (
  IN EFI_LIST_ENTRY              *Head,
  IN EFI_DEVICE_PATH_PROTOCOL    *DevicePath,
  OUT UINTN                      *Index
  );

VOID
FreeDefaultMappingList (
  IN OUT EFI_LIST_ENTRY   *Head
  );

EFI_STATUS
CopyToDefaultMappingTable (
  IN EFI_LIST_ENTRY   * Head,
  IN OUT UINTN        *BufferSize,
  OUT UINT8           *Buffer OPTIONAL
  );

EFI_STATUS
Remapping (
  VOID
  );

EFI_STATUS
SEnvAddMappingName (
  IN  EFI_HANDLE           Handle,
  IN  CHAR16               *Name,
  IN  BOOLEAN              Consist
  );

UINTN
GetDeviceMediaType (
  IN  EFI_DEVICE_PATH_PROTOCOL     *DevicePath
  );

EFI_STATUS
SEnvCmdMapOld (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  );

//
// Function implementation
//
VOID
SEnvInitMap (
  VOID
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  //
  // Init the default map device
  //
  SEnvCurDevice = StrDuplicate (L"none");
}

BOOLEAN
IsRemoveableDevice (
  IN EFI_DEVICE_PATH_PROTOCOL      *DevicePath
  )
/*++

Routine Description:

  Judge if DevicePath is describing a removable device, such as USB or SCSI

Arguments:

  DevicePath  - Device path instance for the device

Returns:

  TRUE  - if the device is a removable device
  FALSE - if the device is not a removable device
--*/
{
  if (NULL == DevicePath) {
    return FALSE;
  }

  while (!IsDevicePathEndType (DevicePath)) {
    if (DevicePathType (DevicePath) == MESSAGING_DEVICE_PATH) {
      switch (DevicePathSubType (DevicePath)) {
      case MSG_USB_DP:
      case MSG_SCSI_DP:
        return TRUE;

      default:
        return FALSE;
      }
    }

    DevicePath = NextDevicePathNode (DevicePath);
  }

  return FALSE;
}

VOID
GetMappingRelatedName (
  IN  VARIABLE_ID           *MappingItem,
  IN  CHAR16                **ConsistName,
  IN  CHAR16                **OtherName
  )
{
  VARIABLE_ID     *Var;
  POOL_PRINT      NameList;
  EFI_LIST_ENTRY  *Link;

  *ConsistName  = NULL;
  *OtherName    = NULL;
  ZeroMem (&NameList, sizeof (NameList));
  for (Link = SEnvMap.Flink; Link != &SEnvMap; Link = Link->Flink) {
    Var = CR (Link, VARIABLE_ID, Link, VARIABLE_SIGNATURE);
    if (Var == MappingItem) {
      continue;
    }

    if (DevicePathCompare ((VOID *) Var->u.Str, (VOID *) MappingItem->u.Str) == 0) {
      if (Var->Flags & VAR_ID_CONSIST) {
        *ConsistName = StrDuplicate (Var->Name);
      } else {
        CatPrint (&NameList, L"%s ", Var->Name);
      }
    }
  }

  *OtherName = NameList.str;
}

VOID
DumpMappingItem (
  IN UINTN            SLen,
  IN BOOLEAN          Verbose,
  IN VARIABLE_ID      *Var
  )
{
  CHAR16                    *Ptr;
  CHAR16                    *ConsistName;
  CHAR16                    *MediaName;
  BOOLEAN                   Removeable;
  EFI_DEVICE_PATH_PROTOCOL  *DPath;
  EFI_STATUS                Status;
  EFI_HANDLE                DeviceHandle;
  UINTN                     MediaType;
  UINTN                     Index;

  if (SLen == 0) {
    SLen = StrLen (Var->Name);
  }

  MediaName = L"";
  DPath     = (EFI_DEVICE_PATH_PROTOCOL *) Var->u.Value;
  MediaType = GetDeviceMediaType (DPath);
  for (Index = 0; mMediaName[Index].MediaType != 0; Index++) {
    if (MediaType == mMediaName[Index].MediaType) {
      MediaName = mMediaName[Index].ShowName;
      break;
    }
  }

  GetMappingRelatedName (Var, &ConsistName, &Ptr);
  Removeable = IsRemoveableDevice (DPath);
  if (!Verbose) {
    if (ConsistName == NULL && Ptr != NULL) {
      ConsistName = StrDuplicate (Ptr);
    } else if (Ptr != NULL) {
      ConsistName = ReallocatePool (
                      ConsistName,
                      (StrLen (ConsistName) + 1) * sizeof (CHAR16),
                      (StrLen (ConsistName) + StrLen (Ptr) + 2) * sizeof (CHAR16)
                      );
      StrCat (ConsistName, L" ");
      StrCat (ConsistName, Ptr);
    }

    PrintToken (
      STRING_TOKEN (STR_SHELLENV_MAP_SHORT_INFO),
      HiiEnvHandle,
      SLen,
      Var->Name,
      Removeable ? L"Removable " : L"",
      MediaName,
      ConsistName
      );
  } else {
    PrintToken (
      STRING_TOKEN (STR_SHELLENV_MAP_CONSIST_NAME),
      HiiEnvHandle,
      SLen,
      Var->Name,
      (Var->Flags & VAR_ID_CONSIST) ? Var->Name : ConsistName
      );
    PrintToken (STRING_TOKEN (STR_SHELLENV_MAP_OTHER_NAME), HiiEnvHandle, SLen, L"", Ptr);

    //
    // lookup handle for this mapping
    //
    Status = BS->LocateDevicePath (
                  &gEfiDevicePathProtocolGuid,
                  &DPath,
                  &DeviceHandle
                  );
    if (EFI_ERROR (Status)) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_MAP_HANDLE_NOT_FOUND), HiiEnvHandle, SLen + 3);
    } else {
      PrintToken (STRING_TOKEN (STR_SHELLENV_MAP_HANDLE), HiiEnvHandle, SLen + 3, L"");
      SEnvDHProt (FALSE, FALSE, 0, DeviceHandle, "eng");
    }

    PrintToken (STRING_TOKEN (STR_SHELLENV_MAP_MEDIA_TYPE), HiiEnvHandle, SLen, L"", MediaName);
    PrintToken (STRING_TOKEN (STR_SHELLENV_MAP_REMOVEABLE), HiiEnvHandle, SLen, L"", Removeable ? L"YES" : L"NO");
    //
    // print current directory for this mapping
    //
    PrintToken (STRING_TOKEN (STR_SHELLENV_MAP_CUR_DIR), HiiEnvHandle, SLen, L"", Var->CurDir ? Var->CurDir : L"\\");
  }

  if (Ptr) {
    FreePool (Ptr);
  }

  Ptr = LibDevicePathToStr ((EFI_DEVICE_PATH_PROTOCOL *) Var->u.Value);
  PrintToken (STRING_TOKEN (STR_SHELLENV_MAP_DEVICE_PATH), HiiEnvHandle, SLen, L"", Ptr);

  if (ConsistName) {
    FreePool (ConsistName);
  }

  if (Ptr) {
    FreePool (Ptr);
  }
}

BOOLEAN
MappingNameExist (
  IN BOOLEAN            Verbose,
  IN UINTN              ShowType,
  IN UINTN              MediaType,
  IN CHAR16             *MappingName
  )
/*++

Routine Description:

Arguments:

  Verbose     - The verbose
  ShowType    - The show type
  MediaType   - The media type
  MappingName - The mapping name

Returns:

--*/
{
  EFI_LIST_ENTRY  *Link;
  UINTN           Type;
  VARIABLE_ID     *Var;

  if (ShowType == ShowNull) {
    ShowType = ShowAll;
  }

  for (Link = SEnvMap.Flink; Link != &SEnvMap; Link = Link->Flink) {
    Var = CR (Link, VARIABLE_ID, Link, VARIABLE_SIGNATURE);
    //
    // if is consist map and not show consist mapping name, skip.
    //
    if ((Var->Flags & VAR_ID_CONSIST) && !(ShowType & ShowConsistMapping)) {
      continue;
    }
    //
    // if is normal name and not show normal name, skip.
    //
    if ((!(Var->Flags & VAR_ID_CONSIST)) && !(ShowType & ShowNormal)) {
      continue;
    }
    //
    // if the device is not required, skip.
    //
    Type = GetDeviceMediaType ((VOID *) Var->u.Str);
    if (!(MediaType & Type)) {
      continue;
    }

    if ((MappingName != NULL) && !MetaMatch (Var->Name, MappingName)) {
      continue;
    }

    return TRUE;
  }

  return FALSE;
}

EFI_STATUS
SEnvDumpMapping (
  IN BOOLEAN            Verbose,
  IN UINTN              ShowType,
  IN UINTN              MediaType,
  IN CHAR16             *MappingName
  )
/*++

Routine Description:

Arguments:

  Verbose     - The verbose
  ShowType    - The show type
  MediaType   - The media type
  MappingName - The mapping name

Returns:
  EFI_SUCCESS - Success
  EFI_NOT_FOUND - Not found

--*/
{
  UINTN           SLen;
  UINTN           Len;
  EFI_LIST_ENTRY  *Link;
  UINTN           Type;
  VARIABLE_ID     *Var;
  BOOLEAN         Found;

  if (ShowType == ShowNull) {
    ShowType = ShowAll;
  }
  //
  //  get the max mapping name length, for the output format.
  //
  SLen = 0;
  for (Link = SEnvMap.Flink; Link != &SEnvMap; Link = Link->Flink) {
    Var = CR (Link, VARIABLE_ID, Link, VARIABLE_SIGNATURE);
    Len = StrLen (Var->Name);
    if (Len > SLen && Len < 8) {
      SLen = Len;
    }
  }

  Found = FALSE;
  for (Link = SEnvMap.Flink; Link != &SEnvMap; Link = Link->Flink) {
    if (GetExecutionBreak ()) {
      return EFI_SUCCESS;
    }

    Var = CR (Link, VARIABLE_ID, Link, VARIABLE_SIGNATURE);
    //
    // if is consist map and not show consist mapping name, skip.
    //
    if ((Var->Flags & VAR_ID_CONSIST) && !(ShowType & ShowConsistMapping)) {
      continue;
    }
    //
    // if is normal name and not show normal name, skip.
    //
    if ((!(Var->Flags & VAR_ID_CONSIST)) && !(ShowType & ShowNormal)) {
      continue;
    }
    //
    // if the device is not required, skip.
    //
    Type = GetDeviceMediaType ((VOID *) Var->u.Str);
    if (!(MediaType & Type)) {
      continue;
    }

    if ((MappingName != NULL) && !MetaMatch (Var->Name, MappingName)) {
      continue;
    }

    Len = StrLen (Var->Name);
    DumpMappingItem (SLen < Len ? Len : SLen, Verbose, Var);
    Found = TRUE;
  }

  if (!Found) {
    return EFI_NOT_FOUND;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
SEnvCmdMap (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  )
/*++

Routine Description:

  Code for internal "map" command

Arguments:

  ImageHandle - The image handle
  SystemTable - The system table

Returns:
  EFI_INVALID_PARAMETER       - The invalid parameter
  EFI_REDIRECTION_NOT_ALLOWED - Can't redirection
  EFI_NOT_FOUND               - Not found
  
--*/
{
  EFI_LIST_ENTRY          *Link;
  EFI_LIST_ENTRY          *Head;
  VARIABLE_ID             *Var;
  VARIABLE_ID             *Found;
  CHAR16                  *Name;
  CHAR16                  *Value;
  BOOLEAN                 Delete;
  BOOLEAN                 Verbose;
  BOOLEAN                 Remap;
  EFI_STATUS              Status;
  EFI_HANDLE              Handle;
  UINTN                   ShowType;
  UINTN                   MediaType;

  UINTN                   ItemValue;
  SHELL_VAR_CHECK_CODE    RetCode;
  CHAR16                  *Useful;
  SHELL_ARG_LIST          *Item;
  SHELL_VAR_CHECK_PACKAGE ChkPck;

  ZeroMem (&ChkPck, sizeof (SHELL_VAR_CHECK_PACKAGE));
  if (IS_OLD_SHELL) {
    return SEnvCmdMapOld (ImageHandle, SystemTable);
  }

  EFI_SHELL_APP_INIT (ImageHandle, SystemTable);

  if (!EFI_PROPER_VERSION (0, 99)) {
    PrintToken (
      STRING_TOKEN (STR_SHELLENV_GNC_COMMAND_NOT_SUPPORT),
      HiiEnvHandle,
      L"map",
      EFI_VERSION_0_99 
      );
    return EFI_UNSUPPORTED;
  }

  SEnvLoadHandleTable ();

  Head = &SEnvMap;

  //
  // Initialize variables to aVOID a level 4 warning
  //
  Var       = NULL;

  Name      = NULL;
  Value     = NULL;
  Delete    = FALSE;
  Verbose   = FALSE;
  Remap     = FALSE;
  Status    = EFI_SUCCESS;
  Found     = NULL;
  ShowType  = ShowNull;
  MediaType = MediaTypeAll;
  ItemValue = 0;
  Handle    = NULL;

  //
  // Crack arguments
  //
  RetCode = LibCheckVariables (SI, MapCheckList, &ChkPck, &Useful);
  if (VarCheckOk != RetCode) {
    switch (RetCode) {
    case VarCheckUnknown:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_UNKNOWN_FLAG), HiiEnvHandle, L"map", Useful);
      break;

    case VarCheckDuplicate:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_DUP_FLAG), HiiEnvHandle, L"map", Useful);
      break;

    case VarCheckConflict:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_FLAG_CONFLICT), HiiEnvHandle, L"map", Useful);
      break;

    case VarCheckLackValue:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_LACK_ARG), HiiEnvHandle, L"map", Useful);
      break;

    default:
      break;
    }

    LibCheckVarFreeVarList (&ChkPck);
    return EFI_INVALID_PARAMETER;
  }
  //
  // Out put help.
  //
  if (LibCheckVarGetFlag (&ChkPck, L"-b") != NULL) {
    EnablePageBreak (DEFAULT_INIT_ROW, DEFAULT_AUTO_LF);
  }

  if (LibCheckVarGetFlag (&ChkPck, L"-?") != NULL) {
    if (ChkPck.ValueCount > 0 ||
        ChkPck.FlagCount > 2 ||
        (2 == ChkPck.FlagCount && !LibCheckVarGetFlag (&ChkPck, L"-b"))
        ) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiEnvHandle, L"map");
      Status = EFI_INVALID_PARAMETER;
    } else {
      PrintToken (STRING_TOKEN (STR_SHELLENV_MAP_VERBOSE_HELP), HiiEnvHandle);
      Status = EFI_SUCCESS;
    }
    LibCheckVarFreeVarList (&ChkPck);
    return Status;
  }

  if (ChkPck.ValueCount > 2) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiEnvHandle, L"map");
    LibCheckVarFreeVarList (&ChkPck);
    return EFI_INVALID_PARAMETER;
  }

  Item = LibCheckVarGetFlag (&ChkPck, L"-d");
  if (Item != NULL) {
    if (ChkPck.ValueCount != 0) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiEnvHandle, L"map");
      LibCheckVarFreeVarList (&ChkPck);
      return EFI_INVALID_PARAMETER;
    }

    Delete  = TRUE;
    Name    = Item->VarStr;
  }

  if (LibCheckVarGetFlag (&ChkPck, L"-v") != NULL) {
    Verbose = TRUE;
  }

  if (LibCheckVarGetFlag (&ChkPck, L"-r") != NULL) {
    if (SI->RedirArgc != 0) {
      LibCheckVarFreeVarList (&ChkPck);
      return EFI_REDIRECTION_NOT_ALLOWED;
    }

    Remap = TRUE;
  }

  if (LibCheckVarGetFlag (&ChkPck, L"-c") != NULL) {
    ShowType |= ShowConsistMapping;
  }

  if (LibCheckVarGetFlag (&ChkPck, L"-f") != NULL) {
    ShowType |= ShowNormal;
  }

  Item = LibCheckVarGetFlag (&ChkPck, L"-t");
  if (Item != NULL) {
    Status = ParseMediaTypeStr (Item->VarStr, &MediaType);
    if (EFI_ERROR (Status)) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_ARG), HiiEnvHandle, L"map", Item->VarStr);
      LibCheckVarFreeVarList (&ChkPck);
      return EFI_INVALID_PARAMETER;
    }
  }

  Item = ChkPck.VarList;
  if (NULL != Item) {
    Name  = Item->VarStr;
    Item  = Item->Next;
  }

  if (NULL != Item) {

    Value = Item->VarStr;
    //
    // Find the handle
    //
    if (Value[StrLen (Value) - 1] == L':') {
      Value[StrLen (Value) - 1] = 0;
      Var                       = SEnvFindVar (Head, Value);
      Value[StrLen (Value)]     = L':';
      if (Var == NULL) {
        PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_ARG), HiiEnvHandle, L"map", Value);
        LibCheckVarFreeVarList (&ChkPck);
        return EFI_INVALID_PARAMETER;
      }
    } else {
      ItemValue = (UINTN) StrToUInt (Value, 16, &Status);
      if (EFI_ERROR (Status)) {
        PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_ARG), HiiEnvHandle, L"map", Value);
        LibCheckVarFreeVarList (&ChkPck);
        return EFI_INVALID_PARAMETER;
      }

      Handle = SEnvHandleFromUINT (ItemValue);
      if (Handle == NULL) {
        PrintToken (STRING_TOKEN (STR_SHELLENV_MAP_HANDLE_NOT_FOUND_2), HiiEnvHandle, ItemValue);
        LibCheckVarFreeVarList (&ChkPck);
        return EFI_NOT_FOUND;
      }
    }
  }

  LibCheckVarFreeVarList (&ChkPck);

  if (Delete && Value) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiEnvHandle, L"map");
    return EFI_INVALID_PARAMETER;
  }
  //
  // Process
  //
  if (Remap && !Value && !Delete) {
    Status  = Remapping ();
    Remap   = FALSE;
  }

  if (Verbose) {
    SEnvLoadHandleProtocolInfo (&gEfiDevicePathProtocolGuid);
  }

  AcquireLock (&SEnvLock);

  if (Delete) {
    Found = NULL;
    for (Link = Head->Flink; Link != Head;) {
      Var   = CR (Link, VARIABLE_ID, Link, VARIABLE_SIGNATURE);
      Link  = Link->Flink;
      if (MetaMatch (Var->Name, Name)) {
        Found = Var;
        Status = RT->SetVariable (
                      Found->Name,
                      &SEnvMapId,
                      0,
                      0,
                      NULL
                      );
        if (EFI_ERROR (Status)) {
          goto Done;
        }

        RemoveEntryList (&Found->Link);
        FreePool (Found);
      }
    }

    if (Found == NULL) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_MAP_CANNOT_FIND_SNAME), HiiEnvHandle, Name);
      Status = EFI_NOT_FOUND;
    }

    goto Done;
  }

  if (Value) {
    //
    // Check the name
    //
    if (!IsValidName (Name)) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_MAP_INVALID_NAME), HiiEnvHandle, Name);
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }

    Found = SEnvFindVar (Head, Name);
    if (Found != NULL) {
      RT->SetVariable (
            Name,
            &SEnvMapId,
            0,
            0,
            NULL
            );
    }

    if (Handle != NULL) {
      Status = SEnvAddMappingName (Handle, Name, FALSE);
      if (EFI_ERROR (Status)) {
        goto Done;
      }
    } else {
      SEnvAddVarToList (
        &SEnvMap,
        Name,
        (UINT8 *) Var->u.Str,
        DevicePathSize ((VOID *) Var->u.Str),
        TRUE
        );
      RT->SetVariable (
            Name,
            &SEnvMapId,
            EFI_VARIABLE_BOOTSERVICE_ACCESS,
            DevicePathSize ((VOID *) Var->u.Str),
            Var->u.Str
            );
    }

    if (Found != NULL) {
      RemoveEntryList (&Found->Link);
      FreePool (Found);
    }
  }

  if (MappingNameExist (Verbose, ShowType, MediaType, Name)) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_DEVICE_MAPPING_TABLE), HiiEnvHandle);
    Status = SEnvDumpMapping (Verbose, ShowType, MediaType, Name);
  } else {
    PrintToken (STRING_TOKEN (STR_SHELLENV_MAP_NOT_EXIST), HiiEnvHandle);
  }

Done:
  ReleaseLock (&SEnvLock);
  SEnvFreeHandleTable ();

  return Status;
}

VARIABLE_ID *
SEnvMapDeviceFromName (
  IN OUT CHAR16   **pPath
  )
/*++

Routine Description:

  Check the Path for a device name, and updates the path to point after
  the device name.  If no device name is found, the current default is used.

Arguments:

  pPath  - The pointer to the path
  
Returns:

--*/
{
  CHAR16          *Path;
  CHAR16          *Ptr;
  CHAR16          *MappedName;
  CHAR16          c;
  VARIABLE_ID     *Var;
  EFI_LIST_ENTRY  *Link;

  ASSERT_LOCKED (&SEnvLock);

  Var   = NULL;
  Path  = *pPath;

  //
  // Check for a device name terminator
  //
  for (Ptr = Path; *Ptr && *Ptr != ':' && *Ptr != '\\'; Ptr++) {
    ;
  }
  //
  // Move Ptr to the last ':' in contiguous ':'s
  //
  while (*Ptr == ':' && *(Ptr + 1) == ':') {
    Ptr++;
  }
  //
  // Use either the passed in name or the current device name setting
  //
  MappedName = *Ptr == ':' ? Path : SEnvCurDevice;

  //
  // Null terminate the string in Path just in case that is the one we
  // are using
  //
  c     = *Ptr;
  *Ptr  = 0;

  //
  // Find the mapping for the device
  //
  for (Link = SEnvMap.Flink; Link != &SEnvMap; Link = Link->Flink) {
    Var = CR (Link, VARIABLE_ID, Link, VARIABLE_SIGNATURE);
    if (StriCmp (Var->Name, MappedName) == 0) {
      SEnvCheckValidMappings (Var);
      Var->Flags |= VAR_ID_ACCESSED;
      if (Var->Flags & VAR_ID_INVALID) {
        return NULL;
      }
      break;
    }
  }
  //
  // Restore the path
  //
  *Ptr = c;

  //
  // If the mapped device was not found, return NULL
  //
  if (Link == &SEnvMap) {
    DEBUG (
      (EFI_D_VARIABLE,
      "SEnvNameToPath: Mapping for '%es' not found\n",
      Path)
      );
    return NULL;
  }
  //
  // If we found it as part of the path, skip the path over it
  //
  if (MappedName == Path) {
    *pPath = Ptr + 1;
  }
  //
  // Return the target mapping
  //
  return Var;
}

EFI_DEVICE_PATH_PROTOCOL *
SEnvIFileNameToPath (
  IN CHAR16               *Path
  )
/*++

Routine Description:

  Builds a device path from the filename string.  Note that the
  device name must already be stripped off of the file name string

Arguments:
  
  Path  - The pointer to the path
  
Returns:

--*/
{
  CHAR16                    *LPath;
  CHAR16                    *ps;
  EFI_DEVICE_PATH_PROTOCOL  *DPath;
  EFI_DEVICE_PATH_PROTOCOL  *Node;
  EFI_DEVICE_PATH_PROTOCOL  *NewPath;
  CHAR16                    *Buffer;

  ASSERT_LOCKED (&SEnvLock);

  DPath = NULL;
  Buffer = StrDuplicate (Path);
  ASSERT (Buffer);

  //
  // If no path, return the root
  //
  if (!*Path) {
    DPath = FileDevicePath (NULL, L"\\");
	goto Done;
  }
  //
  // Build a file path for the name component(s)
  //
  ps = Buffer;
  while (*ps) {
    LPath = ps;                                          
    while (*ps) {                                        
      //                                                 
      // if buffer has run out, just handle to LPath     
      //                                                 
      if (*ps == '\\') {                                 
        if (ps - LPath > MAX_ARG_LENGTH - 2) {
          *(ps++) = 0;
          break;
        }
      }
      ps += 1;
    }

	  //
    // If we have part of a path name, append it to the device path
    //
    Node          = FileDevicePath (NULL, LPath);
    NewPath       = AppendDevicePath (DPath, Node);
    FreePool (Node);
    if (DPath) {
      FreePool (DPath);
    }

    DPath = NewPath;
    ASSERT (DPath);
  }
Done:
  FreePool (Buffer);

  return DPath;
}

EFI_DEVICE_PATH_PROTOCOL *
SEnvFileNameToPath (
  IN CHAR16               *Path
  )
/*++

Routine Description:

Arguments:

  Path   - The pointer to the path

Returns:

--*/
{
  EFI_DEVICE_PATH_PROTOCOL  *FilePath;

  AcquireLock (&SEnvLock);
  FilePath = SEnvIFileNameToPath (Path);
  ReleaseLock (&SEnvLock);
  return FilePath;
}

EFI_DEVICE_PATH_PROTOCOL *
SEnvINameToPath (
  IN CHAR16               *Path
  )
/*++

Routine Description:

  Convert a file system style name to an file path    

Arguments:

  Path  - The pointer to the path

Returns:

--*/
{
  EFI_DEVICE_PATH_PROTOCOL  *DPath;
  EFI_DEVICE_PATH_PROTOCOL  *FPath;
  EFI_DEVICE_PATH_PROTOCOL  *RPath;
  EFI_DEVICE_PATH_PROTOCOL  *FilePath;
  EFI_DEVICE_PATH_PROTOCOL  *NewPath;
  VARIABLE_ID               *Var;
  BOOLEAN                   FreeDPath;

  DPath     = NULL;
  RPath     = NULL;
  FPath     = NULL;
  FilePath  = NULL;
  FreeDPath = FALSE;

  ASSERT_LOCKED (&SEnvLock);

  //
  // Get the device for the name, and advance past the device name
  //
  Var = SEnvMapDeviceFromName (&Path);
  if (!Var) {
    DEBUG ((EFI_D_VARIABLE, "SEnvNameToPath: mapped device not found\n"));
    goto Done;
  }
  //
  // Start the file path with this mapping
  //
  DPath = (EFI_DEVICE_PATH_PROTOCOL *) Var->u.Value;

  //
  // If the path is relative, append the current dir of the device to the dpath
  //
  if (*Path != '\\') {
    RPath   = SEnvIFileNameToPath (Var->CurDir ? Var->CurDir : L"\\");
    NewPath = AppendDevicePath (DPath, RPath);
    DPath   = NewPath;
    //
    // if AllocatePool fail in function AppendDevicePath, ;
    //
    ASSERT (DPath);

    FreeDPath = TRUE;
  }
  if (StrCmp (L".", Path) == 0) {
    //
    // Need not append "." device path, use DPath directly w/o freeing it
    //
    FilePath  = DPath;
    FreeDPath = FALSE;
  } else {
    //
    // Build a file path for the rest of the name string
    //
    FPath = SEnvIFileNameToPath (Path);

    //
    // Append the 2 paths
    //
    FilePath = AppendDevicePath (DPath, FPath);
  }
  ASSERT (FilePath);

Done:
  if (DPath && FreeDPath) {
    FreePool (DPath);
  }

  if (RPath) {
    FreePool (RPath);
  }

  if (FPath) {
    FreePool (FPath);
  }

  return FilePath;
}

EFI_DEVICE_PATH_PROTOCOL *
EFIAPI
SEnvNameToPath (
  IN CHAR16                   *Path
  )
/*++

Routine Description:

Arguments:

  Path - The pointer to the path

Returns:

--*/
{
  EFI_DEVICE_PATH_PROTOCOL  *DPath;

  AcquireLock (&SEnvLock);
  DPath = SEnvINameToPath (Path);
  ReleaseLock (&SEnvLock);

  return DPath;
}

EFI_STATUS
EFIAPI
SEnvCmdCd (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
/*++

Routine Description:

Arguments:

  ImageHandle - The image handle
  SystemTable - The system table

Returns:

  EFI_INVALID_PARAMETER   - Invalid parameter
  EFI_SUCCESS             - Success
  EFI_NOT_FOUND           - Not found

--*/
{
  EFI_DEVICE_PATH_PROTOCOL  *FilePath;
  EFI_STATUS                Status;
  EFI_FILE_HANDLE           OpenDir;
  CHAR16                    *Dir;
  CHAR16                    *CurDir;
  VARIABLE_ID               *Var;
  EFI_FILE_INFO             *FileInfo;
  SHELL_VAR_CHECK_PACKAGE   ChkPck;
  SHELL_VAR_CHECK_CODE      RetCode;
  CHAR16                    *Useful;

  FilePath = NULL;
  EFI_SHELL_APP_INIT (ImageHandle, SystemTable);

  if (!EFI_PROPER_VERSION (0, 99)) {
    PrintToken (
      STRING_TOKEN (STR_SHELLENV_GNC_COMMAND_NOT_SUPPORT),
      HiiEnvHandle,
      L"cd",
      EFI_VERSION_0_99 
      );
    return EFI_UNSUPPORTED;
  }

  Status  = EFI_SUCCESS;
  ZeroMem (&ChkPck, sizeof (SHELL_VAR_CHECK_PACKAGE));

  RetCode = LibCheckVariables (SI, CdCheckList, &ChkPck, &Useful);
  if (VarCheckOk != RetCode) {
    switch (RetCode) {
    case VarCheckUnknown:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_UNKNOWN_FLAG), HiiEnvHandle, L"cd", Useful);
      break;

    case VarCheckDuplicate:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_DUP_FLAG), HiiEnvHandle, L"cd", Useful);
      break;

    default:
      break;
    }

    LibCheckVarFreeVarList (&ChkPck);
    return EFI_INVALID_PARAMETER;
  }
  //
  // Out put help.
  //
  if (LibCheckVarGetFlag (&ChkPck, L"-b") != NULL) {
    EnablePageBreak (DEFAULT_INIT_ROW, DEFAULT_AUTO_LF);
  }

  if (LibCheckVarGetFlag (&ChkPck, L"-?") != NULL) {
    if (IS_OLD_SHELL) {
      PrintToken (STRING_TOKEN (STR_NO_HELP), HiiEnvHandle);
    } else if (ChkPck.ValueCount > 0 ||
             ChkPck.FlagCount > 2 ||
             (2 == ChkPck.FlagCount && !LibCheckVarGetFlag (&ChkPck, L"-b"))
            ) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiEnvHandle, L"cd");
      Status = EFI_INVALID_PARAMETER;
    } else {
      PrintToken (STRING_TOKEN (STR_SHELLENV_CD_VERBOSE_HELP), HiiEnvHandle);
      Status = EFI_SUCCESS;
    }

    LibCheckVarFreeVarList (&ChkPck);
    return Status;
  }

  //
  // If no arguments, print the current directory
  //
  if (ChkPck.ValueCount == 0) {
    LibCheckVarFreeVarList (&ChkPck);
    Dir = SEnvGetCurDir (NULL);
    if (Dir) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_ONE_VAR_S), HiiEnvHandle, Dir);
      FreePool (Dir);
      return EFI_SUCCESS;
    } else {
      PrintToken (STRING_TOKEN (STR_SHELLENV_CD_CURRENT_DIR_NOT_SPEC), HiiEnvHandle, L"cd");
      return EFI_NOT_FOUND;
    }
  }

  AcquireLock (&SEnvLock);

  //
  // If more then 1 argument, syntax
  //
  if (ChkPck.ValueCount > 1) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiEnvHandle, L"cd");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }
  //
  // Find the target device
  //
  Dir = ChkPck.VarList->VarStr;
  Var = SEnvMapDeviceFromName (&Dir);
  if (!Var) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_CD_CANNOT_FIND_MAPPED), HiiEnvHandle, L"cd");
    Status = EFI_NOT_FOUND;
    goto Done;
  }
  //
  // If there's no path specified, print the current path for the device
  //
  if (*Dir == 0) {
    Print (L"%hs%H:%H", Var->Name);
    PrintToken (STRING_TOKEN (STR_SHELLENV_ONE_VAR_S), HiiEnvHandle, Var->CurDir ? Var->CurDir : L"\\");
    Status = EFI_SUCCESS;
    goto Done;
  }
  //
  // Build a file path for the argument
  //
  FilePath = SEnvINameToPath (ChkPck.VarList->VarStr);
  if (!FilePath) {
   Status = EFI_NOT_FOUND;
   goto Done;
  }
  //
  // Open the target directory
  //
  OpenDir = LibOpenFilePath (FilePath, EFI_FILE_MODE_READ);

  if (!OpenDir) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_CD_TARGET_DIR_NOT_FOUND), HiiEnvHandle, L"cd");
    Status = EFI_NOT_FOUND;
    goto Done;
  }
  //
  // Get information on the file path that was opened.
  //
  FileInfo = LibGetFileInfo (OpenDir);
  if (FileInfo == NULL) {
    Status = EFI_NOT_FOUND;
    goto Done;
  }
  //
  // Verify that the file opened is a directory.
  //
  if (!(FileInfo->Attribute & EFI_FILE_DIRECTORY)) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_CD_TARGET_NOT_DIR), HiiEnvHandle, L"cd");
    FreePool (FileInfo);
    OpenDir->Close (OpenDir);
    Status = EFI_NOT_FOUND;
    goto Done;
  }

  FreePool (FileInfo);

  CurDir = SEnvFileHandleToFileName (OpenDir);
  OpenDir->Close (OpenDir);

  //
  // If we have a new path, update the device
  //
  if (CurDir) {
    if (Var->CurDir) {
      FreePool (Var->CurDir);
    }

    Var->CurDir = CurDir;

  } else {
    PrintToken (STRING_TOKEN (STR_SHELLENV_CD_CANNOT_CHANGE_DIR), HiiEnvHandle, L"cd");
  }

  Status = EFI_SUCCESS;

Done:
  ReleaseLock (&SEnvLock);
  if (FilePath) {
    FreePool (FilePath);
  }

  LibCheckVarFreeVarList (&ChkPck);
  return Status;
}

CHAR16 *
EFIAPI
SEnvGetCurDir (
  IN CHAR16       *DeviceName OPTIONAL
  )
/*++

Routine Description:

  N.B. results are allocated in pool

Arguments:

  DeviceName   - The device name
Returns:

--*/
{
  CHAR16          *Dir;
  EFI_LIST_ENTRY  *Link;
  VARIABLE_ID     *Var;

  Dir = NULL;
  if (!DeviceName) {
    DeviceName = SEnvCurDevice;
  }
  //
  // Walk through SEnvMap linked list to get acquired device name
  //
  AcquireLock (&SEnvLock);
  for (Link = SEnvMap.Flink; Link != &SEnvMap; Link = Link->Flink) {
    Var = CR (Link, VARIABLE_ID, Link, VARIABLE_SIGNATURE);
    if (StriCmp (Var->Name, DeviceName) == 0) {
      Dir = PoolPrint (L"%s:%s", Var->Name, Var->CurDir ? Var->CurDir : L"\\");
      break;
    }
  }

  ReleaseLock (&SEnvLock);
  return Dir;
}

EFI_STATUS
SEnvSetCurrentDevice (
  IN CHAR16       *Name
  )
/*++

Routine Description:

Arguments:

  Name   - The device name

Returns:

  EFI_INVALID_PARAMETER   - Invalid parameter

--*/
{
  VARIABLE_ID     *Var;
  EFI_LIST_ENTRY  *Link;
  EFI_STATUS      Status;
  UINTN           Len;
  CHAR16          *NewName;
  CHAR16          c;

  Len = StrLen (Name);
  if (Len < 1) {
    return EFI_INVALID_PARAMETER;
  }
  //
  // If the name ends with a ":" strip it off
  //
  Len -= 1;
  c = Name[Len];
  if (c == ':') {
    Name[Len] = 0;
  }

  Status = EFI_NO_MAPPING;
  AcquireLock (&SEnvLock);

  for (Link = SEnvMap.Flink; Link != &SEnvMap; Link = Link->Flink) {
    Var = CR (Link, VARIABLE_ID, Link, VARIABLE_SIGNATURE);
    if (StriCmp (Var->Name, Name) == 0) {
      NewName = StrDuplicate (Name);
      if (NewName) {
        FreePool (SEnvCurDevice);
        SEnvCurDevice = NewName;
      }

      Status = EFI_SUCCESS;
      break;
    }
  }

  ReleaseLock (&SEnvLock);

  //
  // Restore the name
  //
  Name[Len] = c;
  return Status;
}

VOID
SEnvCheckValidMappings (
  VARIABLE_ID *Var
  )
/*++

Routine Description:

Arguments:

  Var    - The mapping name

Returns:

--*/
{
  EFI_STATUS                Status;

  UINTN                     Index;
  BOOLEAN                   Found;
  UINTN                     NoHandles;
  EFI_HANDLE                *Handles;

  EFI_DEVICE_PATH_PROTOCOL  *DevicePath;
  EFI_DEVICE_PATH_PROTOCOL  *CurDevicePath;
  EFI_BLOCK_IO_PROTOCOL     *BlkIo;

  //
  // Set current mapping info to be invalid
  //
  Found = FALSE;
  Var->Flags |= VAR_ID_INVALID;
  CurDevicePath = (EFI_DEVICE_PATH_PROTOCOL *) Var->u.Str;

  Status = LibLocateHandle (
            ByProtocol,
            &gEfiBlockIoProtocolGuid,
            NULL,
            &NoHandles,
            &Handles
            );

  for (Index = 0; Index < NoHandles; Index++) {
    DevicePath = DevicePathFromHandle (Handles[Index]);
    if (DevicePathCompare (CurDevicePath, DevicePath) != 0) {
      continue;
    }

    Found = TRUE;
    Status = BS->HandleProtocol (
                  Handles[Index],
                  &gEfiBlockIoProtocolGuid,
                  (VOID **) &BlkIo
                  );
    if (!EFI_ERROR (Status)) {
      //
      // We only check media that has no logical partition on it,
      // which means floppy disk, etc.
      //
      if (!BlkIo->Media->LogicalPartition) {
        VOID    *Buffer;
        UINT32  MediaId;
        BOOLEAN MediaPresent;

        Buffer        = AllocatePool (BlkIo->Media->BlockSize);
        MediaId       = BlkIo->Media->MediaId;
        MediaPresent  = BlkIo->Media->MediaPresent;
        BlkIo->ReadBlocks (
                BlkIo,
                BlkIo->Media->MediaId,
                0,
                BlkIo->Media->BlockSize,
                Buffer
                );
        FreePool (Buffer);
        if (MediaPresent != BlkIo->Media->MediaPresent || MediaId != BlkIo->Media->MediaId) {
          if (BlkIo->Media->MediaPresent) {
            BS->ReinstallProtocolInterface (
                  Handles[Index],
                  &gEfiBlockIoProtocolGuid,
                  BlkIo,
                  BlkIo
                  );
            Var->Flags &= ~VAR_ID_INVALID;
          }
        } else {
          if (BlkIo->Media->MediaPresent) {
            Var->Flags &= ~VAR_ID_INVALID;
          }
        }
      } else {
        Var->Flags &= ~VAR_ID_INVALID;
      }
    }
  }

  if (NoHandles) {
    FreePool (Handles);
  }

  if (Found) {
    return ;
  }
  //
  //  In case of fsnt, the fsnt has only file system but no blkio.
  //  So, it needs special deal with.
  //
  Status = LibLocateHandle (
            ByProtocol,
            &gEfiSimpleFileSystemProtocolGuid,
            NULL,
            &NoHandles,
            &Handles
            );

  if (!NoHandles) {
    return ;
  }

  for (Index = 0; Index < NoHandles; Index++) {
    DevicePath = DevicePathFromHandle (Handles[Index]);
    if (DevicePathCompare (CurDevicePath, DevicePath) != 0) {
      continue;
    }

    Var->Flags &= ~VAR_ID_INVALID;
  }

  FreePool (Handles);
}

STATIC
BOOLEAN
IsValidName (
  IN CHAR16           *Name
  )
/*++

Routine Description:
  
  To check if the Name is valid name for mapping.
  
Arguments:

  Name    - the name
  
Returns:
  TRUE    - the name is valid
  FALSE   - the name is invalid

--*/
{
  CHAR16  *Ptr;

  //
  // forbid special chars inside name
  //
  if (StrLen (Name) == 0) {
    return FALSE;
  }

  for (Ptr = Name; *Ptr; Ptr += 1) {
    if (!IsDigit (*Ptr) && !IsAlpha (*Ptr)) {
      return FALSE;
    }
  }

  return TRUE;
}

EFI_STATUS
EFIAPI
SEnvCmdMapGetLineHelp (
  OUT CHAR16                  **Str
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
  return SEnvCmdGetStringByToken (STRING_TOKEN (STR_SHELLENV_MAP_LINE_HELP), Str);
}

EFI_STATUS
EFIAPI
SEnvCmdCdGetLineHelp (
  OUT CHAR16                  **Str
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
  return SEnvCmdGetStringByToken (STRING_TOKEN (STR_SHELLENV_CD_LINE_HELP), Str);
}

EFI_STATUS
SEnvGetFsName (
  IN EFI_DEVICE_PATH_PROTOCOL *DevPath,
  IN BOOLEAN                  ConsistMapping,
  OUT CHAR16                  **Name
  )
{
  VARIABLE_ID     *Var;
  EFI_LIST_ENTRY  *Link;
  EFI_STATUS      Status;

  ASSERT (DevPath);
  Status = EFI_UNSUPPORTED;
  AcquireLock (&SEnvLock);
  for (Link = SEnvMap.Flink; Link != &SEnvMap; Link = Link->Flink) {
    Var = CR (Link, VARIABLE_ID, Link, VARIABLE_SIGNATURE);
    if (!DevicePathIsChildDevice ((EFI_DEVICE_PATH_PROTOCOL *) Var->u.Str, DevPath)) {
      continue;
    }

    if (ConsistMapping) {
      if (Var->Flags & VAR_ID_CONSIST) {
        *Name = StrDuplicate (Var->Name);
        if (*Name == NULL) {
          Status = EFI_OUT_OF_RESOURCES;
        } else {
          Status = EFI_SUCCESS;
        }

        goto Done;

      }
    } else {
      if (!(Var->Flags & VAR_ID_CONSIST)) {
        *Name = StrDuplicate (Var->Name);
        if (*Name == NULL) {
          Status = EFI_OUT_OF_RESOURCES;
        } else {
          Status = EFI_SUCCESS;
        }

        goto Done;
      }
    }
  }

Done:
  ReleaseLock (&SEnvLock);
  return Status;
}

EFI_STATUS
SEnvGetFsDevicePath (
  IN CHAR16                    *Name,
  OUT EFI_DEVICE_PATH_PROTOCOL **DevPath
  )
{
  VARIABLE_ID     *Var;
  EFI_LIST_ENTRY  *Link;
  EFI_STATUS      Status;

  ASSERT (DevPath);
  Status    = EFI_NOT_FOUND;
  *DevPath  = NULL;
  AcquireLock (&SEnvLock);

  //
  //  Traval the map name list and find the Name, return the Name's
  // device path.
  //
  for (Link = SEnvMap.Flink; Link != &SEnvMap; Link = Link->Flink) {
    Var = CR (Link, VARIABLE_ID, Link, VARIABLE_SIGNATURE);
    if (StrCmp (Var->Name, Name) != 0) {
      continue;
    }

    *DevPath  = (EFI_DEVICE_PATH_PROTOCOL *) Var->u.Str;
    Status    = EFI_SUCCESS;
    break;
  }

  ReleaseLock (&SEnvLock);
  return Status;
}

INTN
CompareMappingInfo (
  IN   VOID        *Buffer1,
  IN   VOID        *Buffer2
  )
{
  MAPPING_NAME_INFO *MappingInfo1;
  MAPPING_NAME_INFO *MappingInfo2;
  INTN              MappingValue1;
  INTN              MappingValue2;
  MappingInfo1  = *(MAPPING_NAME_INFO **) Buffer1;
  MappingInfo2  = *(MAPPING_NAME_INFO **) Buffer2;
  //
  //  the compare items order:
  //    Is Fs Device;
  //    Is Removeable;
  //    Is Blk device;
  //    The media type;
  //    The device path.
  //
  MappingValue1 = (MappingInfo1->FSDevice ? 0 : 4) | (MappingInfo1->BlkDevice ? 0 : 1) | (MappingInfo1->Removeable ? 2 : 0);
  MappingValue2 = (MappingInfo2->FSDevice ? 0 : 4) | (MappingInfo2->BlkDevice ? 0 : 1) | (MappingInfo2->Removeable ? 2 : 0);

  if (MappingValue1 != MappingValue2) {
    return MappingValue1 - MappingValue2;
  }

  if (MappingInfo1->MediaType != MappingInfo2->MediaType) {
    return (INTN) MappingInfo1->MediaType - (INTN) MappingInfo2->MediaType;
  }

  return DevicePathConsistMappingCompare (&MappingInfo1->DevicePath, &MappingInfo2->DevicePath);
}

EFI_STATUS
SEnvAddMappingName (
  IN  EFI_HANDLE           Handle,
  IN  CHAR16               *Name,
  IN  BOOLEAN              Consist
  )
/*++
Routine Description:

  Add the name to the mapping name list.

Arguments:

  Handle  - The handle
  Name    - The new name
  Consist - Is consist mapping name

Returns:
 
--*/
{
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath;
  EFI_STATUS                Status;
  EFI_BLOCK_IO_PROTOCOL     *BlockIo;
  VARIABLE_ID               *Var;
  DevicePath = NULL;
  //
  // Get the handle's device path
  //
  DevicePath = DevicePathFromHandle (Handle);
  if (!DevicePath) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_MAP_DEV_PATH_NOT_FOUND), HiiEnvHandle, SEnvHandleToNumber (Handle));
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  Status = BS->HandleProtocol (Handle, &gEfiBlockIoProtocolGuid, &BlockIo);
  //
  // This is just for NT32, because fsntx has no block io protocol installed
  // but fsntx has installed simple file system protocol
  //
  if (EFI_ERROR (Status)) {
    Status = BS->HandleProtocol (Handle, &gEfiSimpleFileSystemProtocolGuid, &BlockIo);
  }

  if (EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_MAP_NO_BLOCK_IO), HiiEnvHandle, SEnvHandleToNumber (Handle));
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  Var = SEnvAddVarToList (&SEnvMap, Name, (UINT8 *) DevicePath, DevicePathSize (DevicePath), TRUE);
  Status = RT->SetVariable (
                Name,
                &SEnvMapId,
                EFI_VARIABLE_BOOTSERVICE_ACCESS,
                DevicePathSize (DevicePath),
                DevicePath
                );
  if (Consist) {
    Var->Flags |= VAR_ID_CONSIST;
  }

Done:
  return Status;
}

MAPPING_NAME_INFO *
FindMappingInfoByHandle (
  IN MAPPING_NAME_INFO         **MappingInfo,
  IN EFI_HANDLE                Handle
  )
{
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath;
  DevicePath = DevicePathFromHandle (Handle);
  for (; *MappingInfo != NULL; MappingInfo++) {
    if (DevicePathCompare ((*MappingInfo)->DevicePath, DevicePath) == 0) {
      return *MappingInfo;
    }
  }

  return NULL;
}

EFI_STATUS
AddMappingInfoByHandle (
  IN  MAPPING_NAME_INFO      **MappingTable,
  IN  EFI_HANDLE             Handle,
  OUT MAPPING_NAME_INFO      **Info
  )
{
  MAPPING_NAME_INFO         *MappingInfo;
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath;

  if (Info == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  DevicePath = DevicePathFromHandle (Handle);
  if (DevicePath == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  MappingInfo = AllocateZeroPool (sizeof (MAPPING_NAME_INFO));
  if (MappingInfo == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  MappingInfo->DevicePath = LibDuplicateDevicePathInstance (DevicePath);
  MappingInfo->Handle     = Handle;
  MappingInfo->FSDevice   = FALSE;
  MappingInfo->BlkDevice  = FALSE;
  MappingInfo->Removeable = FALSE;

  while (*MappingTable) {
    MappingTable++;
  }

  *MappingTable = MappingInfo;
  *Info         = MappingInfo;
  return EFI_SUCCESS;
}

UINTN
GetDeviceMediaType (
  IN  EFI_DEVICE_PATH_PROTOCOL     *DevicePath
  )
{
  ACPI_HID_DEVICE_PATH  *Acpi;

  //
  //  Parse the device path:
  //  Devicepath sub type                 mediatype
  //    MEDIA_HANRDDRIVE_DP      ->       Hard Disk
  //    MEDIA_CDROM_DP           ->       CD Rom
  //    Acpi.HID = 0X0604        ->       Floppy
  //
  if (NULL == DevicePath) {
    return MediaUnknown;
  }

  while (!IsDevicePathEndType (DevicePath)) {
    if (DevicePathType (DevicePath) == MEDIA_DEVICE_PATH) {
      switch (DevicePathSubType (DevicePath)) {
      case MEDIA_HARDDRIVE_DP:
        return MediaHardDisk;

      case MEDIA_CDROM_DP:
        return MediaCDRom;
      }
    }

    if (DevicePathType (DevicePath) == ACPI_DEVICE_PATH) {
      Acpi = (ACPI_HID_DEVICE_PATH *) DevicePath;
      if (EISA_ID_TO_NUM (Acpi->HID) == 0x0604) {
        return MediaFloppy;
      }
    }

    DevicePath = NextDevicePathNode (DevicePath);
  }

  return MediaUnknown;
}

EFI_STATUS
CheckMediaChange (
  VOID
  )
{
  PROTOCOL_INFO         *ProtBlkIo;
  UINTN                 Index;
  EFI_STATUS            Status;
  EFI_BLOCK_IO_PROTOCOL *BlkIo;
  VOID                  *Buffer;

  SEnvLoadHandleTable ();
  SEnvLoadHandleProtocolInfo (NULL);

  AcquireLock (&SEnvGuidLock);
  ProtBlkIo = SEnvGetProtByStr (L"blkio");
  ReleaseLock (&SEnvGuidLock);

  //
  // Check for media change for every block io
  //
  for (Index = 0; Index < ProtBlkIo->NoHandles; Index++) {
    Status = BS->HandleProtocol (
                  ProtBlkIo->Handles[Index],
                  &gEfiBlockIoProtocolGuid,
                  (VOID **) &BlkIo
                  );
    if (!EFI_ERROR (Status)) {
      BlkIo->FlushBlocks (BlkIo);
    }
  }

  for (Index = 0; Index < ProtBlkIo->NoHandles; Index++) {
    Status = BS->HandleProtocol (
                  ProtBlkIo->Handles[Index],
                  &gEfiBlockIoProtocolGuid,
                  (VOID **) &BlkIo
                  );
    if (!EFI_ERROR (Status)) {
      if (!BlkIo->Media->LogicalPartition) {
        Buffer = AllocatePool (BlkIo->Media->BlockSize);
        BlkIo->ReadBlocks (
                BlkIo,
                BlkIo->Media->MediaId,
                0,
                BlkIo->Media->BlockSize,
                Buffer
                );
        FreePool (Buffer);
        BS->ReinstallProtocolInterface (
              ProtBlkIo->Handles[Index],
              &gEfiBlockIoProtocolGuid,
              BlkIo,
              BlkIo
              );
      }
    }
  }

  SEnvFreeHandleTable ();
  return EFI_SUCCESS;
}

EFI_STATUS
Remapping (
  VOID
  )
{
  PROTOCOL_INFO             *ProtFs;
  PROTOCOL_INFO             *ProtBlkIo;
  UINTN                     Index;
  MAPPING_NAME_INFO         *MappingInfo;
  EFI_STATUS                Status;

  UINTN                     MappingCount;
  MAPPING_NAME_INFO         **MappingTable;
  EFI_DEVICE_PATH_PROTOCOL  **HITable;
  UINTN                     BlkCount;
  UINTN                     FsntCount;
  UINTN                     MappingInfoCount;
  POOL_PRINT                Path;

  VARIABLE_ID               *Var;

  HITable   = NULL;
  BlkCount  = 0;
  FsntCount = 0;

  ZeroMem (&Path, sizeof (Path));

  Status = CheckMediaChange ();
  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Delete all the old mappings
  //
  AcquireLock (&SEnvLock);
  while (!IsListEmpty (&SEnvMap)) {
    Var = CR (SEnvMap.Flink, VARIABLE_ID, Link, VARIABLE_SIGNATURE);
    Status = RT->SetVariable (
                  Var->Name,
                  &SEnvMapId,
                  0,
                  0,
                  NULL
                  );
    RemoveEntryList (&Var->Link);
    FreePool (Var);
  }

  SEnvLoadHandleTable ();
  SEnvLoadHandleProtocolInfo (NULL);

  AcquireLock (&SEnvGuidLock);
  ProtFs    = SEnvGetProtByStr (L"fs");
  ProtBlkIo = SEnvGetProtByStr (L"blkio");
  ReleaseLock (&SEnvGuidLock);

  //
  //  Get all the fs handle and device handle.
  //
  MappingCount      = ProtFs->NoHandles + ProtBlkIo->NoHandles;
  MappingTable      = AllocateZeroPool (sizeof (MAPPING_NAME_INFO *) * MappingCount);
  MappingInfoCount  = 0;
  for (Index = 0; Index < ProtFs->NoHandles; Index++) {
    MappingInfo = FindMappingInfoByHandle (MappingTable, ProtFs->Handles[Index]);
    if (MappingInfo == NULL) {
      Status = AddMappingInfoByHandle (MappingTable, ProtFs->Handles[Index], &MappingInfo);
      if (EFI_ERROR (Status)) {
        goto Done;
      }

      MappingInfoCount++;
    }

    MappingInfo->FSDevice = TRUE;
  }

  for (Index = 0; Index < ProtBlkIo->NoHandles; Index++) {
    MappingInfo = FindMappingInfoByHandle (MappingTable, ProtBlkIo->Handles[Index]);
    if (MappingInfo == NULL) {
      Status = AddMappingInfoByHandle (MappingTable, ProtBlkIo->Handles[Index], &MappingInfo);
      if (EFI_ERROR (Status)) {
        goto Done;
      }

      MappingInfoCount++;
    }

    MappingInfo->BlkDevice = TRUE;
  }
  //
  //  Get all the infomations about the handle for gene the consist name.
  //
  ConsistMappingCreateHIDevicePathTable (&HITable);
  for (Index = 0; Index < MappingInfoCount; Index++) {
    MappingTable[Index]->ConsistMappingName = ConsistMappingGenMappingName (MappingTable[Index]->DevicePath, HITable);
    MappingTable[Index]->Removeable         = IsRemoveableDevice (MappingTable[Index]->DevicePath);
    MappingTable[Index]->MediaType          = GetDeviceMediaType (MappingTable[Index]->DevicePath);
  }
  //
  //  The sort order is:
  //    floppy
  //    hard disk
  //    cd rom
  //    unknown device
  //    removeable floppy
  //    removeable hard disk
  //    removeable cd rom
  //    removeable unknown device
  //
  QSort (MappingTable, MappingInfoCount, sizeof (MAPPING_NAME_INFO *), CompareMappingInfo);

  BlkCount  = 0;
  FsntCount = 0;
  for (Index = 0; Index < MappingInfoCount; Index++) {
    //
    //  Suppose the Blkname and FSname is not too large.
    //
    //  the blkname and the fsname is:
    //
    //  fsnt0
    //  fsnt1
    //  fsnt2
    //  ...
    //  fsnt(l)
    //  fs0  .............  blk0
    //  fs1  .............  blk1
    //  fs2  .............  blk2
    //  ...
    //  fs(m).............  blk(m)
    //                      blk(m+1)
    //                      ...
    //                      blk(n)
    //
    if (MappingTable[Index]->BlkDevice) {
      MappingTable[Index]->BlkName = AllocateZeroPool (0x20);
      SPrint (MappingTable[Index]->BlkName, 0x20, L"blk%x", BlkCount);
      if (MappingTable[Index]->FSDevice) {
        MappingTable[Index]->FSName = AllocateZeroPool (0x20);
        SPrint (MappingTable[Index]->FSName, 0x20, L"fs%x", BlkCount);
      }

      BlkCount++;
    } else {
      MappingTable[Index]->FSName = AllocateZeroPool (0x20);
      SPrint (MappingTable[Index]->FSName, 0x20, L"fsnt%x", FsntCount);
      FsntCount++;
    }
  }
  //
  //  Add these consist name, fs name, blk name to map name list.
  //
  for (Index = 0; Index < MappingInfoCount; Index++) {
    if (MappingTable[Index]->FSName != NULL) {
      SEnvAddMappingName (MappingTable[Index]->Handle, MappingTable[Index]->FSName, FALSE);
    }
  }

  for (Index = 0; Index < MappingInfoCount; Index++) {
    if (MappingTable[Index]->BlkName != NULL) {
      SEnvAddMappingName (MappingTable[Index]->Handle, MappingTable[Index]->BlkName, FALSE);
    }
  }

  for (Index = 0; Index < MappingInfoCount; Index++) {
    if (MappingTable[Index]->ConsistMappingName != NULL && MappingTable[Index]->FSDevice) {
      SEnvAddMappingName (MappingTable[Index]->Handle, MappingTable[Index]->ConsistMappingName, TRUE);
    }
  }
  //
  //  Set the path variable.
  //  All the fs name is consist name.
  //
  CatPrint (&Path, L".");
  for (Index = 0; Index < MappingInfoCount; Index++) {
    if (MappingTable[Index]->FSDevice) {
      CatPrint (
        &Path,
        L";%s:\\efi\\tools;%s:\\efi\\boot;%s:\\",
        MappingTable[Index]->ConsistMappingName,
        MappingTable[Index]->ConsistMappingName,
        MappingTable[Index]->ConsistMappingName
        );
    }
  }

  Var = SEnvFindVar (&SEnvEnv, L"path");
  if (Var != NULL) {
    RemoveEntryList (&Var->Link);
    FreePool (Var);
  }

  SEnvAddVar (
    &SEnvEnv,
    &SEnvEnvId,
    L"path",
    (UINT8 *) (Path.str),
    (Path.len + 1) * sizeof (CHAR16),
    TRUE
    );

Done:
  //
  //  Free the resources.
  //
  SEnvFreeHandleTable ();
  ReleaseLock (&SEnvLock);

  for (Index = 0; Index < MappingInfoCount; Index++) {
    if (MappingTable[Index]->ConsistMappingName != NULL) {
      FreePool (MappingTable[Index]->ConsistMappingName);
    }

    if (MappingTable[Index]->DevicePath != NULL) {
      FreePool (MappingTable[Index]->DevicePath);
    }

    if (MappingTable[Index]->FSName != NULL) {
      FreePool (MappingTable[Index]->FSName);
    }

    if (MappingTable[Index]->BlkName != NULL) {
      FreePool (MappingTable[Index]->BlkName);
    }

    FreePool (MappingTable[Index]);
  }

  if (MappingTable != NULL) {
    FreePool (MappingTable);
  }

  if (Path.str != NULL) {
    FreePool (Path.str);
  }

  if (HITable != NULL) {
    ConsistMappingFreeHIDevicePathTable (HITable);
  }

  return EFI_SUCCESS;
}

EFI_STATUS
ParseMediaTypeStr (
  IN  CHAR16           *Str,
  OUT UINTN            *MediaType
  )
{
  UINTN Index;
  UINTN Index2;
  UINTN NameIndex;
  UINTN Type;

  Type = 0;
  //
  //  The string's syntax is "mediatype[,mediatype[...]]"
  //  example:
  //  hd
  //  hd,cd
  //  fp,hd,cd
  //
  //  the mediatype name is accordint the mMediaName[i].Name.
  //
  for (Index = 0; Str[Index] != 0; Index = Index2) {
    for (Index2 = Index; Str[Index2] != ',' && Str[Index2] != 0; Index2++)
      ;
    if (Index2 == Index) {
      return EFI_INVALID_PARAMETER;
    }

    for (NameIndex = 0; mMediaName[NameIndex].MediaType != 0; NameIndex++) {
      if (StrnCmp (Str + Index, mMediaName[NameIndex].Name, Index2 - Index) == 0) {
        Type |= mMediaName[NameIndex].MediaType;
        break;
      }
    }

    if (mMediaName[NameIndex].MediaType == 0) {
      return EFI_INVALID_PARAMETER;
    }

    if (Str[Index2] == ',') {
      Index2++;
    }
  }

  if (Type & MediaUnknown) {
    return EFI_INVALID_PARAMETER;
  }

  *MediaType = Type;
  return EFI_SUCCESS;
}
//
// Compatiable map support
//
VOID
SEnvDumpMappingOld (
  IN UINTN            SLen,
  IN BOOLEAN          Verbose,
  IN VARIABLE_ID      *Var
  )
/*++

Routine Description:

Arguments:

  SLen    - The space length
  Verbose - Verbose
  Var     - The mapping name

Returns:

--*/
{
  CHAR16                    *Ptr;
  EFI_DEVICE_PATH_PROTOCOL  *DPath;
  EFI_STATUS                Status;
  EFI_HANDLE                DeviceHandle;

  Ptr = LibDevicePathToStr ((EFI_DEVICE_PATH_PROTOCOL *) Var->u.Str);
  PrintToken (STRING_TOKEN (STR_SHELLENV_MAP_THREE_VAR), HiiEnvHandle, SLen, Var->Name, Ptr);

  if (Verbose) {
    //
    // lookup handle for this mapping
    //
    DPath = (EFI_DEVICE_PATH_PROTOCOL *) Var->u.Value;
    Status = BS->LocateDevicePath (
                  &gEfiDevicePathProtocolGuid,
                  &DPath,
                  &DeviceHandle
                  );
    if (EFI_ERROR (Status)) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_MAP_HANDLE_NOT_FOUND), HiiEnvHandle, SLen + 3);
    } else {
      PrintToken (STRING_TOKEN (STR_SHELLENV_MAP_HANDLE), HiiEnvHandle, SLen + 3, L"");
      SEnvDHProt (FALSE, FALSE, 0, DeviceHandle, "eng");
    }
    //
    // print current directory for this mapping
    //
    PrintToken (
      STRING_TOKEN (STR_SHELLENV_MAP_TWO_VARS_S),
      HiiEnvHandle,
      SLen + 3,
      L"",
      Var->CurDir ? Var->CurDir : L"\\"
      );
  }

  FreePool (Ptr);
}

EFI_STATUS
SEnvReloadDefaults (
  IN EFI_HANDLE           Image,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
/*++

Routine Description:
 
Arguments:

  Image       - The image handle
  SystemTable - The system table

Returns:

  EFI_SUCCESS - Success

--*/
{
  EFI_LIST_ENTRY  DefCmds;
  POOL_PRINT      Path;
  DEFAULT_CMD     *Cmd;
  PROTOCOL_INFO   *ProtFs;
  PROTOCOL_INFO   *ProtBlkIo;
  UINTN           Index;
  UINTN           HandleNo;
  BOOLEAN         EchoStatus;
  BOOLEAN         Same;
  CHAR16          MapStr[8];
  EFI_LIST_ENTRY  *Link;
  EFI_LIST_ENTRY  *Head;
  VARIABLE_ID     *Var;

  Head = &SEnvMap;

  //
  // There are no settings, build some defaults
  //
  InitializeListHead (&DefCmds);
  ZeroMem (&Path, sizeof (Path));

  AcquireLock (&SEnvLock);
  SEnvLoadHandleTable ();
  SEnvLoadHandleProtocolInfo (NULL);

  AcquireLock (&SEnvGuidLock);
  ProtBlkIo = SEnvGetProtByStr (L"blkio");
  ReleaseLock (&SEnvGuidLock);

  //
  // Check for media change for every block io
  //
  for (Index = 0; Index < ProtBlkIo->NoHandles; Index++) {
    EFI_STATUS            Status;
    EFI_BLOCK_IO_PROTOCOL *BlkIo;

    Status = BS->HandleProtocol (
                  ProtBlkIo->Handles[Index],
                  &gEfiBlockIoProtocolGuid,
                  (VOID **) &BlkIo
                  );
    if (!EFI_ERROR (Status)) {
      BlkIo->FlushBlocks (BlkIo);
    }
  }

  for (Index = 0; Index < ProtBlkIo->NoHandles; Index++) {
    EFI_STATUS            Status;
    EFI_BLOCK_IO_PROTOCOL *BlkIo;
    VOID                  *Buffer;

    Status = BS->HandleProtocol (
                  ProtBlkIo->Handles[Index],
                  &gEfiBlockIoProtocolGuid,
                  (VOID **) &BlkIo
                  );
    if (!EFI_ERROR (Status)) {
      if (!BlkIo->Media->LogicalPartition) {
        BOOLEAN                         MediaPresent;
        UINT32                          MediaId;
        EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FileSystem;
        MediaPresent  = BlkIo->Media->MediaPresent;
        MediaId       = BlkIo->Media->MediaId;
        Buffer        = AllocatePool (BlkIo->Media->BlockSize);
        BlkIo->ReadBlocks (
                BlkIo,
                BlkIo->Media->MediaId,
                0,
                BlkIo->Media->BlockSize,
                Buffer
                );
        FreePool (Buffer);
        FileSystem = NULL;
        //
        // If the handle contains both blockio protocol and file system
        // protocol, mostly it is a floppy. If only contains blockio protocol,
        // this block is raw block, need partition driver to detect partition
        // blocks. In this case, we need to reinstall that blockio protocol
        // interfaces to let partition driver detect media and install
        // partition blocks on that.
        //
        Status = BS->HandleProtocol (
                      ProtBlkIo->Handles[Index],
                      &gEfiSimpleFileSystemProtocolGuid,
                      (VOID **) &FileSystem
                      );
        if (FileSystem) {
          if (MediaPresent != BlkIo->Media->MediaPresent || MediaId != BlkIo->Media->MediaId) {
            BS->ReinstallProtocolInterface (
                  ProtBlkIo->Handles[Index],
                  &gEfiBlockIoProtocolGuid,
                  BlkIo,
                  BlkIo
                  );
          }
        } else {
          BS->ReinstallProtocolInterface (
                ProtBlkIo->Handles[Index],
                &gEfiBlockIoProtocolGuid,
                BlkIo,
                BlkIo
                );
        }
      }
    }
  }
  //
  // New code, try to make mapping list sequence unchanged if current device
  // paths and original device paths are the same
  //
  Same = FALSE;

  //
  // Load handle table and handle protocol info so that we can get
  // the updated info if media changed
  //
  SEnvLoadHandleTable ();
  SEnvLoadHandleProtocolInfo (NULL);
  AcquireLock (&SEnvGuidLock);
  ProtFs    = SEnvGetProtByStr (L"fs");
  ProtBlkIo = SEnvGetProtByStr (L"blkio");
  ReleaseLock (&SEnvGuidLock);

  //
  // Run all the devices that support a File System and add a default
  // mapping and path setting for each device
  //
  CatPrint (&Path, L"set path -v .");
  if (ProtFs != NULL) {
    for (Index = 0; Index < ProtFs->NoHandles; Index++) {
      for (HandleNo = 0; HandleNo < SEnvNoHandles; HandleNo++) {
        if (SEnvHandles[HandleNo] == ProtFs->Handles[Index]) {
          break;
        }
      }

      HandleNo += 1;

      Cmd             = AllocateZeroPool (sizeof (DEFAULT_CMD));
      Cmd->Line       = Cmd->Buffer;
      Cmd->Signature  = DEFAULT_CMD_SIGNATURE;
      SPrint (Cmd->Line, sizeof (Cmd->Buffer), L"map fs%x %x", Index, HandleNo);
      InsertTailList (&DefCmds, &Cmd->Link);

      //
      // Mark existed fs variable
      //
      SPrint (MapStr, sizeof (MapStr), L"fs%x", Index);
      for (Link = Head->Flink; Link != Head;) {
        Var   = CR (Link, VARIABLE_ID, Link, VARIABLE_SIGNATURE);
        Link  = Link->Flink;
        if (!Var->Name) {
          continue;
        }

        if (!StriCmp (Var->Name, MapStr)) {
          Var->Name = NULL;
          break;
        }
      }
      //
      // Append this device to the path
      //
      CatPrint (
        &Path,
        L";fs%x:\\;fs%x:\\efi\\boot;fs%x:\\efi\\tools",
        Index,
        Index,
        Index
        );
    }
  }
  //
  // Run all the devices that support a BlockIo and add a default
  // mapping for the device
  //
  if (ProtBlkIo != NULL) {
    for (Index = 0; Index < ProtBlkIo->NoHandles; Index++) {
      for (HandleNo = 0; HandleNo < SEnvNoHandles; HandleNo++) {
        if (SEnvHandles[HandleNo] == ProtBlkIo->Handles[Index]) {
          break;
        }
      }

      HandleNo += 1;

      Cmd             = AllocateZeroPool (sizeof (DEFAULT_CMD));
      Cmd->Line       = Cmd->Buffer;
      Cmd->Signature  = DEFAULT_CMD_SIGNATURE;
      SPrint (Cmd->Line, sizeof (Cmd->Buffer), L"map blk%x %x", Index, HandleNo);
      InsertTailList (&DefCmds, &Cmd->Link);

      //
      // Mark existed blk variable
      //
      SPrint (MapStr, sizeof (MapStr), L"blk%x", Index);
      for (Link = Head->Flink; Link != Head;) {
        Var   = CR (Link, VARIABLE_ID, Link, VARIABLE_SIGNATURE);
        Link  = Link->Flink;
        if (!Var->Name) {
          continue;
        }

        if (!StriCmp (Var->Name, MapStr)) {
          Var->Name = NULL;
          break;
        }
      }

    }
  }
  //
  // Remove old SEnvMap Info
  //
  for (Link = Head->Flink; Link != Head;) {
    Var = CR (Link, VARIABLE_ID, Link, VARIABLE_SIGNATURE);
    if (Var->Name) {
      RT->SetVariable (Var->Name, &SEnvMapId, 0, 0, NULL);
    }

    Link = Link->Flink;
    RemoveEntryList (&Var->Link);
    FreePool (Var);
  }
  //
  // Release handle table resources & lock
  //
  SEnvFreeHandleTable ();
  ReleaseLock (&SEnvLock);

  //
  // Set Echo Status to off, to aVOID junk output if the it is executed in
  // script file
  //
  EchoStatus = SEnvBatchGetEcho ();
  SEnvBatchSetEcho (FALSE);

  //
  // Execute all the queue commands
  //
  while (!IsListEmpty (&DefCmds)) {
    Cmd = CR (DefCmds.Flink, DEFAULT_CMD, Link, DEFAULT_CMD_SIGNATURE);
    SEnvExecute (Image, Cmd->Line, TRUE);
    RemoveEntryList (&Cmd->Link);
    FreePool (Cmd);
  }

  if (Path.str) {
    SEnvExecute (Image, Path.str, TRUE);
  }
  //
  // Restore original echo status
  //
  SEnvBatchSetEcho (EchoStatus);

  if (Path.str) {
    FreePool (Path.str);
  }
  //
  // Remember current device paths
  //
  return EFI_SUCCESS;
}

EFI_STATUS
SEnvCmdMapOld (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  )
/*++

Routine Description:

  Code for internal "map" command

Arguments:

  ImageHandle  - The image handle
  SystemTable  - The system table

Returns:

  EFI_SUCCESS - Success
  EFI_INVALID_PARAMETER - The invalid parameter

--*/
{
  EFI_LIST_ENTRY            *Link;
  EFI_LIST_ENTRY            *Head;
  VARIABLE_ID               *Var;
  VARIABLE_ID               *Found;
  CHAR16                    *Name;
  CHAR16                    *Value;
  UINTN                     SLen;
  UINTN                     Len;
  UINTN                     Size;
  UINTN                     DataSize;
  BOOLEAN                   Delete;
  BOOLEAN                   Verbose;
  BOOLEAN                   Remap;
  EFI_STATUS                Status;
  UINTN                     Index;
  CHAR16                    *Ptr;
  EFI_HANDLE                Handle;
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath;
  BOOLEAN                   PrtHelp;

  EFI_SHELL_APP_INIT (ImageHandle, SystemTable);

  if (!EFI_PROPER_VERSION (0, 99)) {
    PrintToken (
      STRING_TOKEN (STR_SHELLENV_GNC_COMMAND_NOT_SUPPORT),
      HiiEnvHandle,
      L"map",
      EFI_VERSION_0_99 
      );
    return EFI_UNSUPPORTED;
  }

  Head = &SEnvMap;

  //
  // Initialize variables to aVOID a level 4 warning
  //
  Var     = NULL;

  Name    = NULL;
  Value   = NULL;
  Delete  = FALSE;
  Verbose = FALSE;
  Remap   = FALSE;
  Status  = EFI_SUCCESS;
  Found   = NULL;
  PrtHelp = FALSE;

  //
  // Crack arguments
  //
  for (Index = 1; Index < SI->Argc; Index += 1) {
    Ptr = SI->Argv[Index];
    if (*Ptr == '-') {
      switch (Ptr[1]) {
      case 'd':
      case 'D':
        Delete = TRUE;
        break;

      case 'v':
      case 'V':
        Verbose = TRUE;
        break;

      case 'r':
      case 'R':
        Remap = TRUE;
        break;

      case 'b':
      case 'B':
        EnablePageBreak (DEFAULT_INIT_ROW, DEFAULT_AUTO_LF);
        break;

      case '?':
        PrtHelp = TRUE;
        break;

      default:
        PrintToken (STRING_TOKEN (STR_SHELLENV_MAP_UNKNOWN_FLAG), HiiEnvHandle, Ptr);
        return EFI_INVALID_PARAMETER;
      }

      continue;
    }

    if (!Name) {
      Name = Ptr;
      if (!IsValidName (Name)) {
        PrintToken (STRING_TOKEN (STR_SHELLENV_MAP_INVALID_NAME), HiiEnvHandle, Name);
        return EFI_INVALID_PARAMETER;
      }

      continue;
    }

    if (!Value) {
      Value = Ptr;
      continue;
    }

    PrintToken (STRING_TOKEN (STR_SHELLENV_MAP_TOO_MANY_ARGS), HiiEnvHandle);
    return EFI_INVALID_PARAMETER;
  }

  if (PrtHelp) {
    PrintToken (STRING_TOKEN (STR_NO_HELP), HiiEnvHandle);
    return EFI_SUCCESS;
  }

  if (Delete && Value) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_MAP_TOO_MANY_ARGS), HiiEnvHandle);
    return EFI_INVALID_PARAMETER;
  }
  //
  // Process
  //
  if (Remap && !Value && !Delete) {
    Status  = SEnvReloadDefaults (ImageHandle, SystemTable);
    Remap   = FALSE;
  }

  if (Value || Verbose) {
    SEnvLoadHandleTable ();

    if (Verbose) {
      SEnvLoadHandleProtocolInfo (&gEfiDevicePathProtocolGuid);
    }
  }

  AcquireLock (&SEnvLock);

  SLen = 0;
  for (Link = Head->Flink; Link != Head; Link = Link->Flink) {
    Var = CR (Link, VARIABLE_ID, Link, VARIABLE_SIGNATURE);
    Len = StrLen (Var->Name);
    if (Len > SLen) {
      SLen = Len;
    }
  }

  if (!Name) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_DEVICE_MAPPING_TABLE), HiiEnvHandle);
    for (Link = Head->Flink; Link != Head; Link = Link->Flink) {
      Var = CR (Link, VARIABLE_ID, Link, VARIABLE_SIGNATURE);
      SEnvDumpMappingOld (SLen, Verbose, Var);
    }

  } else {
    //
    // Find the specified value
    //
    for (Link = Head->Flink; Link != Head; Link = Link->Flink) {
      Var = CR (Link, VARIABLE_ID, Link, VARIABLE_SIGNATURE);
      if (StriCmp (Var->Name, Name) == 0) {
        Found = Var;
        break;
      }
    }

    if (Found && Delete) {
      Status = RT->SetVariable (Found->Name, &SEnvMapId, 0, 0, NULL);
    } else if (Value) {
      //
      // Find the handle in question
      //
      Handle = SEnvHandleFromStr (Value);
      if (!Handle) {
        PrintToken (STRING_TOKEN (STR_SHELLENV_MAP_HANDLE_NOT_FOUND_2), HiiEnvHandle, Handle);
        Status = EFI_NOT_FOUND;
        goto Done;
      }
      //
      // Get the handle's device path
      //
      DevicePath = DevicePathFromHandle (Handle);
      if (!DevicePath) {
        PrintToken (STRING_TOKEN (STR_SHELLENV_MAP_DEV_PATH_NOT_FOUND), HiiEnvHandle, Handle);
        Status = EFI_INVALID_PARAMETER;
        goto Done;
      }

      DataSize = DevicePathSize (DevicePath);

      //
      // Add it to the store
      //
      RT->SetVariable (Found ? Found->Name : Name, &SEnvMapId, 0, 0, NULL);
      Status = RT->SetVariable (
                    Found ? Found->Name : Name,
                    &SEnvMapId,
                    EFI_VARIABLE_BOOTSERVICE_ACCESS,
                    DataSize,
                    DevicePath
                    );

      if (!EFI_ERROR (Status)) {
        //
        // Make a new in memory copy
        //
        Size            = sizeof (VARIABLE_ID) + StrSize (Name) + DataSize;
        Var             = AllocateZeroPool (Size);

        Var->Signature  = VARIABLE_SIGNATURE;
        Var->u.Value    = ((UINT8 *) Var) + sizeof (VARIABLE_ID);
        Var->Name       = (CHAR16 *) (Var->u.Value + DataSize);
        Var->ValueSize  = DataSize;
        CopyMem (Var->u.Value, DevicePath, DataSize);
        StrCpy (Var->Name, Found ? Found->Name : Name);
        InsertTailList (Head, &Var->Link);
      }
    } else {
      if (Found) {
        SEnvDumpMappingOld (SLen, Verbose, Var);
      } else {
        PrintToken (STRING_TOKEN (STR_SHELLENV_MAP_CANNOT_FIND_HANDLE), HiiEnvHandle, Name);
      }

      Found = NULL;
    }
    //
    // Remove the old in memory copy if there was one
    //
    if (Found) {
      RemoveEntryList (&Found->Link);
      FreePool (Found);
    }
  }
Done:
  ReleaseLock (&SEnvLock);
  SEnvFreeHandleTable ();

  //
  // Get current mapping info for whole system
  //
  return Status;
}
