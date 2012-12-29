/*++

Copyright (c) 2005 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name: 

  vol.c
  
Abstract:

  EFI Shell command "vol"


Revision History

--*/

#include "EfiShellLib.h"
#include "vol.h"

extern UINT8    STRING_ARRAY_NAME[];

//
// This is the generated header file which includes whatever needs to be exported (strings + IFR)
//
#include STRING_DEFINES_FILE

EFI_HII_HANDLE  HiiHandle;
EFI_GUID        EfiVolGuid = EFI_VOL_GUID;
SHELL_VAR_CHECK_ITEM    VolCheckList[] = {
  {
    L"-n",
    0x01,
    0x02,
    FlagTypeNeedVar
  },
  {
    L"-d",
    0x02,
    0x01,
    FlagTypeSingle
  },
  {
    L"-b",
    0x04,
    0,
    FlagTypeSingle
  },
  {
    L"-?",
    0x08,
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

EFI_STATUS
EFIAPI
InitializeVol (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  );

EFI_STATUS
InitializeVolOld (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  );

STATIC
BOOLEAN
IsValidName (
  IN CHAR16           *Name
  );

EFI_BOOTSHELL_CODE(
  EFI_APPLICATION_ENTRY_POINT(InitializeVol)
)

EFI_STATUS
EFIAPI
InitializeVol (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
/*++

Routine Description:
  Displays volume information for specified file system.

Arguments:
  ImageHandle     The image handle. 
  SystemTable     The system table.

Returns:
  EFI_SUCCESS             - Command completed successfully
  EFI_INVALID_PARAMETER   - Command usage error
  Other value             - Unknown error

--*/
{
  CHAR16                          *VolumeLabel;
  EFI_STATUS                      Status;
  EFI_DEVICE_PATH_PROTOCOL        *DevicePath;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *Vol;
  EFI_BLOCK_IO_PROTOCOL           *BlkIo;
  EFI_BLOCK_IO_MEDIA              *BlkMedia;
  VOID                            *Buffer;
  EFI_FILE_HANDLE                 RootFs;
  EFI_FILE_SYSTEM_INFO            *VolumeInfo;
  UINTN                           Size;
  CHAR16                          *VolName;
  UINTN                           Index;

  SHELL_VAR_CHECK_CODE            RetCode;
  CHAR16                          *Useful;
  SHELL_ARG_LIST                  *Item;
  SHELL_VAR_CHECK_PACKAGE         ChkPck;

  VolumeLabel = NULL;
  VolName     = NULL;

  ZeroMem (&ChkPck, sizeof (SHELL_VAR_CHECK_PACKAGE));
  //
  // We are no being installed as an internal command driver, initialize
  // as an nshell app and run
  //
  EFI_SHELL_APP_INIT (ImageHandle, SystemTable);
  
  //
  // Enable tab key which can pause the output
  //
  EnableOutputTabPause();

  //
  // Register our string package with HII and return the handle to it.
  // If previously registered we will simply receive the handle
  //
  Status = LibInitializeStrings (&HiiHandle, STRING_ARRAY_NAME, &EfiVolGuid);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (!EFI_PROPER_VERSION (0, 99)) {
    PrintToken (
      STRING_TOKEN (STR_SHELLENV_GNC_COMMAND_NOT_SUPPORT),
      HiiHandle,
      L"vol",
      EFI_VERSION_0_99 
      );
    Status = EFI_UNSUPPORTED;
    goto Done;
  }

  if (IS_OLD_SHELL) {
    Status = InitializeVolOld (ImageHandle, SystemTable);
    goto Done;
  }

  RetCode = LibCheckVariables (SI, VolCheckList, &ChkPck, &Useful);
  if (VarCheckOk != RetCode) {
    switch (RetCode) {
    case VarCheckDuplicate:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_DUP_FLAG), HiiHandle, L"vol", Useful);
      break;

    case VarCheckLackValue:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_LACK_ARG), HiiHandle, L"vol", Useful);
      break;

    case VarCheckConflict:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_FLAG_CONFLICT), HiiHandle, L"vol", Useful);
      break;

    case VarCheckUnknown:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_UNKNOWN_FLAG), HiiHandle, L"vol", Useful);
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
  // end of if(VarCheckOk..)
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
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiHandle, L"vol");
      Status = EFI_INVALID_PARAMETER;
    } else {
      PrintToken (STRING_TOKEN (STR_VOL_VERBOSE_HELP), HiiHandle);
      Status = EFI_SUCCESS;
    }

    goto Done;
  }

  if (ChkPck.ValueCount > 1) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiHandle, L"vol");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }
  //
  // Get file system from arguments. If file system not specified,
  // use file system of current directory as default
  //
  if (ChkPck.ValueCount) {
    VolName = StrDuplicate (ChkPck.VarList->VarStr);
  } else {
    VolName = ShellCurDir (NULL);
    if (VolName) {
      for (Index = 0; Index < StrLen (VolName) && VolName[Index] != ':'; Index++) {
        ;
      }

      VolName[Index] = 0;
    } else {
      PrintToken (STRING_TOKEN (STR_VOL_NOT_MAPPED), HiiHandle, L"vol", L"NULL");
      Status = EFI_ABORTED;
      goto Done;
    }
  }

  DevicePath = (EFI_DEVICE_PATH_PROTOCOL *) ShellGetMap (VolName);

  if (DevicePath == NULL) {
    PrintToken (STRING_TOKEN (STR_VOL_NOT_MAPPED), HiiHandle, L"vol", VolName);
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  Status = LibDevicePathToInterface (&gEfiSimpleFileSystemProtocolGuid, DevicePath, (VOID **) &Vol);

  if (EFI_ERROR (Status)) {
    Status = LibDevicePathToInterface (&gEfiBlockIoProtocolGuid, DevicePath, (VOID **) &BlkIo);
    if (!EFI_ERROR (Status)) {

      BlkMedia = BlkIo->Media;

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

      Status = LibDevicePathToInterface (&gEfiSimpleFileSystemProtocolGuid, DevicePath, (VOID **) &Vol);
    }
  }

  if (EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_VOL_FILE_SYSTEM), HiiHandle, L"vol", VolName);
    goto Done;
  }

  Item = LibCheckVarGetFlag (&ChkPck, L"-n");
  if (Item) {
    VolumeLabel = Item->VarStr;
    if (StrLen (VolumeLabel) > 11) {
      PrintToken (STRING_TOKEN (STR_VOL_VOLUME_LABEL_TOO_LONG), HiiHandle, L"vol", VolumeLabel);
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }

    if (!IsValidName (VolumeLabel)) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_ARG), HiiHandle, L"vol", VolumeLabel);
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }
  } else {
    Item = LibCheckVarGetFlag (&ChkPck, L"-d");
    if (Item) {
      VolumeLabel = L"";
    }
  }

  Status = Vol->OpenVolume (Vol, &RootFs);

  if (EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_VOL_CANNOT_OPEN_VOLUME), HiiHandle, L"vol", VolName);
    goto Done;
  }
  //
  // Get volume information of file system
  //
  Size        = SIZE_OF_EFI_FILE_SYSTEM_INFO + 100;
  VolumeInfo  = (EFI_FILE_SYSTEM_INFO *) AllocatePool (Size);
  Status      = RootFs->GetInfo (RootFs, &gEfiFileSystemInfoGuid, &Size, VolumeInfo);

  if (EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_VOL_CANNOT_GET_VOLUME_INFO), HiiHandle, L"vol", VolName);
    FreePool (VolumeInfo);
    goto Done;
  }
  //
  // Set volume label
  //
  if (VolumeLabel) {
    StrCpy (VolumeInfo->VolumeLabel, VolumeLabel);

    Size              = SIZE_OF_EFI_FILE_SYSTEM_INFO + StrSize (VolumeLabel);

    VolumeInfo->Size  = Size;
    Status            = RootFs->SetInfo (RootFs, &gEfiFileSystemInfoGuid, Size, VolumeInfo);

    if (EFI_ERROR (Status)) {
      if (Status == EFI_UNSUPPORTED) {
        PrintToken (STRING_TOKEN (STR_VOL_INVALID_VOLUME_LABEL), HiiHandle, L"vol", VolumeLabel);
      } else {
        PrintToken (STRING_TOKEN (STR_VOL_CANNOT_SET_VOLUME_INFO), HiiHandle, L"vol", VolName);
      }

      FreePool (VolumeInfo);
      goto Done;
    }

    Status = RootFs->GetInfo (RootFs, &gEfiFileSystemInfoGuid, &Size, VolumeInfo);

    if (EFI_ERROR (Status)) {
      PrintToken (STRING_TOKEN (STR_VOL_CANNOT_VERIFY_INFO), HiiHandle, L"vol", VolName);
      FreePool (VolumeInfo);
      goto Done;
    }
  }
  //
  // Print out volume information
  //
  if (StrLen (VolumeInfo->VolumeLabel) == 0) {
    PrintToken (STRING_TOKEN (STR_VOL_NO_LABEL), HiiHandle, VolumeInfo->VolumeLabel);
  } else {
    PrintToken (STRING_TOKEN (STR_VOL_ONE_VAR), HiiHandle, VolumeInfo->VolumeLabel);
  }

  if (VolumeInfo->ReadOnly) {
    Print (L" (ro)\n");
  } else {
    Print (L" (rw)\n");
  }

  PrintToken (STRING_TOKEN (STR_VOL_BYTES_TOTAL), HiiHandle, VolumeInfo->VolumeSize);
  PrintToken (STRING_TOKEN (STR_VOL_BYTES_AVAILABLE), HiiHandle, VolumeInfo->FreeSpace);
  PrintToken (STRING_TOKEN (STR_VOL_BYTES_IN_EACH), HiiHandle, VolumeInfo->BlockSize);

  RootFs->Flush (RootFs);
  RootFs->Close (RootFs);

  FreePool (VolumeInfo);
  Status = EFI_SUCCESS;
Done:
  if (VolName != NULL) {
    FreePool (VolName);
  }

  LibCheckVarFreeVarList (&ChkPck);
  LibUnInitializeStrings ();
  return Status;
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
  for (Ptr = Name; *Ptr; Ptr += 1) {
    if (*Ptr < 0x20 ||
        *Ptr == '?' ||
        *Ptr == '^' ||
        *Ptr == '*' ||
        *Ptr == '+' ||
        *Ptr == '=' ||
        *Ptr == '[' ||
        *Ptr == ']' ||
        *Ptr == ':' ||
        *Ptr == ';' ||
        *Ptr == '\"' ||
        *Ptr == '/' ||
        *Ptr == '<' ||
        *Ptr == '>' ||
        *Ptr == '%' ||
        *Ptr == '.' ||
        *Ptr == ' ' ||
        *Ptr == '|'
        ) {
      return FALSE;
    }
  }

  return TRUE;
}

EFI_STATUS
EFIAPI
InitializeVolGetLineHelp (
  OUT CHAR16              **Str
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
  return LibCmdGetStringByToken (STRING_ARRAY_NAME, &EfiVolGuid, STRING_TOKEN (STR_VOL_LINE_HELP), Str);
}

EFI_STATUS
InitializeVolOld (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
/*++

Routine Description:
  Displays volume information for specified file system.

Arguments:
  ImageHandle     The image handle. 
  SystemTable     The system table.

Returns:
  EFI_SUCCESS             - Command completed successfully
  EFI_INVALID_PARAMETER   - Command usage error
  Other value             - Unknown error

--*/
{
  CHAR16                          *VolumeLabel;
  EFI_STATUS                      Status;
  EFI_DEVICE_PATH_PROTOCOL        *DevicePath;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *Vol;
  EFI_BLOCK_IO_PROTOCOL           *BlkIo;
  EFI_BLOCK_IO_MEDIA              *BlkMedia;
  VOID                            *Buffer;
  EFI_FILE_HANDLE                 RootFs;
  EFI_FILE_SYSTEM_INFO            *VolumeInfo;
  UINTN                           Size;
  CHAR16                          *CurDir;
  CHAR16                          *VolName;
  CHAR16                          *Ptr;
  UINTN                           Index;
  BOOLEAN                         GetHelp;

  VolumeLabel = NULL;
  //
  // Get file system from arguments. If file system not specified,
  // use file system of current directory as default
  //
  DevicePath  = NULL;
  VolName     = NULL;
  VolumeLabel = NULL;
  GetHelp     = FALSE;
  for (Index = 1; Index < SI->Argc; Index += 1) {
    Ptr = SI->Argv[Index];
    if (*Ptr == '-') {
      switch (Ptr[1]) {
      case 'b':
      case 'B':
        EnablePageBreak (DEFAULT_INIT_ROW, DEFAULT_AUTO_LF);
        break;

      case '?':
        GetHelp = TRUE;
        break;

      default:
        PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_UNKNOWN_FLAG), HiiHandle, L"vol", Ptr);
        return EFI_INVALID_PARAMETER;
      }
    } else if (VolName == NULL) {
      VolName = Ptr;
    } else if (VolumeLabel == NULL) {
      VolumeLabel = Ptr;
      if (StrLen (VolumeLabel) > 11) {
        PrintToken (STRING_TOKEN (STR_VOL_VOLUME_LABEL_TOO_LONG), HiiHandle, L"vol", VolumeLabel);
        return EFI_INVALID_PARAMETER;
      }
    } else {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiHandle, L"vol");
      return EFI_INVALID_PARAMETER;
    }
  }

  if (GetHelp) {
    PrintToken (STRING_TOKEN (STR_NO_HELP), HiiHandle);
    return EFI_SUCCESS;
  }

  if (VolName == NULL) {
    CurDir = ShellCurDir (NULL);
    if (CurDir) {
      for (Index = 0; Index < StrLen (CurDir) && CurDir[Index] != ':'; Index++) {
        ;
      }

      CurDir[Index] = 0;
      DevicePath    = (EFI_DEVICE_PATH_PROTOCOL *) ShellGetMap (CurDir);
      FreePool (CurDir);
    }
  } else {
    DevicePath = (EFI_DEVICE_PATH_PROTOCOL *) ShellGetMap (VolName);
  }

  if (DevicePath == NULL) {
    PrintToken (STRING_TOKEN (STR_VOL_NOT_MAPPED), HiiHandle, L"vol", VolName);
    return EFI_INVALID_PARAMETER;
  }

  Status = LibDevicePathToInterface (&gEfiSimpleFileSystemProtocolGuid, DevicePath, (VOID **) &Vol);

  if (EFI_ERROR (Status)) {
    Status = LibDevicePathToInterface (&gEfiBlockIoProtocolGuid, DevicePath, (VOID **) &BlkIo);
    if (!EFI_ERROR (Status)) {

      BlkMedia = BlkIo->Media;

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

      Status = LibDevicePathToInterface (&gEfiSimpleFileSystemProtocolGuid, DevicePath, (VOID **) &Vol);
    }
  }

  if (EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_VOL_FILE_SYSTEM), HiiHandle, L"vol", VolName);
    return Status;
  }

  Status = Vol->OpenVolume (Vol, &RootFs);

  if (EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_VOL_CANNOT_OPEN_VOLUME), HiiHandle, L"vol", VolName);
    return Status;
  }
  //
  // Get volume information of file system
  //
  Size        = SIZE_OF_EFI_FILE_SYSTEM_INFO + 100;
  VolumeInfo  = (EFI_FILE_SYSTEM_INFO *) AllocatePool (Size);
  Status      = RootFs->GetInfo (RootFs, &gEfiFileSystemInfoGuid, &Size, VolumeInfo);

  if (EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_VOL_CANNOT_GET_VOLUME_INFO), HiiHandle, L"vol", VolName);
    FreePool (VolumeInfo);
    return Status;
  }
  //
  // Set volume label
  //
  if (VolumeLabel) {
    StrCpy (VolumeInfo->VolumeLabel, VolumeLabel);

    Size              = SIZE_OF_EFI_FILE_SYSTEM_INFO + StrSize (VolumeLabel);

    VolumeInfo->Size  = Size;
    Status            = RootFs->SetInfo (RootFs, &gEfiFileSystemInfoGuid, Size, VolumeInfo);

    if (EFI_ERROR (Status)) {
      if (Status == EFI_UNSUPPORTED) {
        PrintToken (STRING_TOKEN (STR_VOL_INVALID_VOLUME_LABEL), HiiHandle, L"vol", VolumeLabel);
      } else {
        PrintToken (STRING_TOKEN (STR_VOL_CANNOT_SET_VOLUME_INFO), HiiHandle, L"vol", VolName);
      }

      FreePool (VolumeInfo);
      return Status;
    }

    Status = RootFs->GetInfo (RootFs, &gEfiFileSystemInfoGuid, &Size, VolumeInfo);

    if (EFI_ERROR (Status)) {
      PrintToken (STRING_TOKEN (STR_VOL_CANNOT_VERIFY_INFO), HiiHandle, L"vol", VolName);
      FreePool (VolumeInfo);
      return Status;
    }
  }
  //
  // Print out volume information
  //
  if (StrLen (VolumeInfo->VolumeLabel) == 0) {
    PrintToken (STRING_TOKEN (STR_VOL_NO_LABEL), HiiHandle);
  } else {
    PrintToken (STRING_TOKEN (STR_VOL_ONE_VAR), HiiHandle, VolumeInfo->VolumeLabel);
  }

  if (VolumeInfo->ReadOnly) {
    Print (L" (ro)\n");
  } else {
    Print (L" (rw)\n");
  }

  PrintToken (STRING_TOKEN (STR_VOL_BYTES_TOTAL), HiiHandle, VolumeInfo->VolumeSize);
  PrintToken (STRING_TOKEN (STR_VOL_BYTES_AVAILABLE), HiiHandle, VolumeInfo->FreeSpace);
  PrintToken (STRING_TOKEN (STR_VOL_BYTES_IN_EACH), HiiHandle, VolumeInfo->BlockSize);

  RootFs->Flush (RootFs);
  RootFs->Close (RootFs);

  FreePool (VolumeInfo);

  return EFI_SUCCESS;
}
