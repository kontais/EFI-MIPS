/*++
 
Copyright (c) 2005 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  mount.c
  
Abstract:

  Shell command "mount"



Revision History

--*/

#include "EfiShellLib.h"
#include "mount.h"

extern UINT8  STRING_ARRAY_NAME[];

//
// This is the generated header file which includes whatever needs to be exported (strings + IFR)
//
#include STRING_DEFINES_FILE

//
// Global Variables
//
EFI_HII_HANDLE  HiiHandle;
EFI_GUID        EfiMountGuid = EFI_MOUNT_GUID;
SHELL_VAR_CHECK_ITEM      MountCheckList[] = {
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

STATIC
BOOLEAN
NameHasSpace (
  IN CHAR16           *Name
  )
{
  CHAR16  *Ptr;

  //
  // forbid space char inside name.
  //
  for (Ptr = Name; *Ptr; Ptr += 1) {
    if (*Ptr == ' ') {
      return FALSE;
    }
  }

  return TRUE;
}

EFI_BOOTSHELL_CODE(
  EFI_APPLICATION_ENTRY_POINT(MountMain)
)
//
//
//
EFI_STATUS
MountMain (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  );

//
//
//
EFI_STATUS
MountMain (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  )
/*++

Routine Description:

  Code for external shell "mount" command.

Arguments:

  ImageHandle - The image handle
  SystemTable - The system table

Returns:

--*/
{
  EFI_STATUS                Status;
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath;
  EFI_BLOCK_IO_PROTOCOL     *BlkIo;
  EFI_HANDLE                Handle;
  EFI_HANDLE                *HandleItertor;
  UINT8                     *Buffer;

  DEFAULT_CMD               Cmd;
  UINTN                     HandleNo;
  CHAR16                    *Sname;

  SHELL_VAR_CHECK_CODE      RetCode;
  CHAR16                    *Useful;
  SHELL_VAR_CHECK_PACKAGE   ChkPck;

  ZeroMem (&ChkPck, sizeof (SHELL_VAR_CHECK_PACKAGE));
  EFI_SHELL_APP_INIT (ImageHandle, SystemTable);
  
  //
  // Enable tab key which can pause the output
  //
  EnableOutputTabPause();

  Status = LibInitializeStrings (&HiiHandle, STRING_ARRAY_NAME, &EfiMountGuid);

  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (!EFI_PROPER_VERSION (0, 99)) {
    PrintToken (
      STRING_TOKEN (STR_SHELLENV_GNC_COMMAND_NOT_SUPPORT),
      HiiHandle,
      L"mount",
      EFI_VERSION_0_99 
      );
    Status = EFI_UNSUPPORTED;
    goto Done;
  }

  RetCode = LibCheckVariables (SI, MountCheckList, &ChkPck, &Useful);
  if (VarCheckOk != RetCode) {
    switch (RetCode) {
    case VarCheckDuplicate:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_DUP_FLAG), HiiHandle, L"mount", Useful);
      break;

    case VarCheckUnknown:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_UNKNOWN_FLAG), HiiHandle, L"mount", Useful);
      break;

    default:
      break;
    }

    Status = EFI_INVALID_PARAMETER;
    goto Done;
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
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiHandle, L"mount");
      Status = EFI_INVALID_PARAMETER;
    } else {
      PrintToken (STRING_TOKEN (STR_MOUNT_VERBOSE_HELP), HiiHandle);
      Status = EFI_SUCCESS;
    }

    goto Done;
  }

  if (ChkPck.ValueCount < 1) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_FEW), HiiHandle, L"mount");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  } else if (ChkPck.ValueCount == 1) {
    Sname = NULL;
  } else if (ChkPck.ValueCount == 2) {
    Sname = ChkPck.VarList->Next->VarStr;
  } else {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiHandle, L"mount");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }
  //
  // Check for the device mapping
  //
  DevicePath = (EFI_DEVICE_PATH_PROTOCOL *) ShellGetMap (ChkPck.VarList->VarStr);
  if (DevicePath == NULL) {
    Status = EFI_INVALID_PARAMETER;
    PrintToken (
      STRING_TOKEN (STR_SHELLENV_MOUNT_INVALID_DEV_MAPPING),
      HiiHandle,
      L"mount",
      ChkPck.VarList->VarStr,
      Status
      );
    goto Done;
  }

  Status = BS->LocateDevicePath (
                &gEfiBlockIoProtocolGuid,
                &DevicePath,
                &Handle
                );
  if (EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_MOUNT_DEV_NOT_BLOCKIO), HiiHandle, L"mount", ChkPck.VarList->VarStr, Status);
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  Status = BS->HandleProtocol (Handle, &gEfiBlockIoProtocolGuid, (VOID *) &BlkIo);
  if (EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_MOUNT_DEV_NOT_BLOCKIO), HiiHandle, L"mount", ChkPck.VarList->VarStr, Status);
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }
  //
  //
  //
  Buffer = AllocatePool (BlkIo->Media->BlockSize);
  if (Buffer == NULL) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_OUT_RESOURCE), HiiHandle, L"mount");
    Status = EFI_OUT_OF_RESOURCES;
    goto Done;
  }
  //
  // In EFI the file system driver registers to get notified when DiskIo
  // Devices are added to the system. DiskIo devices register to get notified
  // when BlkIo devices are added to the system. So when a block device shows
  // up a DiskIo is added, and then a file system if added if the media
  // contains a file system. This works great, but when you boot with no media
  // in the device and then put media in the device there is no way to make
  // the notify happen.
  //
  // This code reads a block device. If the BlkIo device returns
  // EFI_MEDIA_CHANGE then it must reinstall in BlkIo protocol. This cause the
  // notifies that make the file system show up. The 4 loops is just a guess it
  // has no magic meaning.
  //
  PrintToken (STRING_TOKEN (STR_SHELLENV_MOUNT_FORCE_FS_MOUNT), HiiHandle, Status);
  BlkIo->FlushBlocks (BlkIo);
  BlkIo->ReadBlocks (
          BlkIo,
          BlkIo->Media->MediaId,
          0,
          BlkIo->Media->BlockSize,
          Buffer
          );
  BS->ReinstallProtocolInterface (
        Handle,
        &gEfiBlockIoProtocolGuid,
        BlkIo,
        BlkIo
        );

  if (Sname) {
    ShellInitHandleEnumerator ();
    HandleNo = 0;
    do {
      Status = ShellNextHandle (&HandleItertor);
      HandleNo++;
      if (HandleItertor == NULL && Status == EFI_SUCCESS) {
        break;
      }

      if (Handle == *HandleItertor) {
        break;
      }
    } while (HandleItertor != NULL);

    PrintToken (STRING_TOKEN (STR_SHELLENV_MOUNT_NMAP), HiiHandle, Sname, HandleNo);
    Cmd.Line = Cmd.Buffer;
    if (!NameHasSpace (Sname)) {
      SPrint (Cmd.Line, sizeof (Cmd.Buffer), L"map \"%s\" 0x%x", Sname, HandleNo);
    } else {
      SPrint (Cmd.Line, sizeof (Cmd.Buffer), L"map %s 0x%x", Sname, HandleNo);
    }

    ShellExecute (ImageHandle, Cmd.Line, TRUE);

    ShellCloseHandleEnumerator ();
  }

  FreePool (Buffer);
  Status = EFI_SUCCESS;
Done:
  LibCheckVarFreeVarList (&ChkPck);
  LibUnInitializeStrings ();
  return Status;

}

EFI_STATUS
MountMainGetLineHelp (
  OUT CHAR16              **Str
  )
/*++

Routine Description:

  Get the command's line help

Arguments:

  Str - The line help

Returns:

  EFI_SUCCESS  - Success

--*/
{
  return LibCmdGetStringByToken (STRING_ARRAY_NAME, &EfiMountGuid, STRING_TOKEN (STR_MOUNT_LINE_HELP), Str);
}
