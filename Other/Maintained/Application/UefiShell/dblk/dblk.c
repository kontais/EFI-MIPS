/*++

Copyright (c) 2005 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  dblk.c
  
Abstract:
  shell command "dblk". 

Revision History 

--*/

#include "EfiShellLib.h"
#include "dblk.h"

extern UINT8  STRING_ARRAY_NAME[];

//
// This is the generated header file which includes whatever needs to be exported (strings + IFR)
//
#include STRING_DEFINES_FILE

//
// Global Variables
//
EFI_HII_HANDLE  HiiDblkHandle;
EFI_GUID        EfiDblkGuid = EFI_DBLK_GUID;
SHELL_VAR_CHECK_ITEM      DblkCheckList[] = {
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


EFI_STATUS
DumpBlockDev (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_BOOTSHELL_CODE(
  EFI_APPLICATION_ENTRY_POINT(DumpBlockDev)
)

EFI_STATUS
DumpBlockDev (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
/*++

Routine Description:

  Dump Data from block IO devices.

Arguments:
  ImageHandle     The image handle. 
  SystemTable     The system table.

Returns:
  EFI_SUCCESS             - Command completed successfully
  EFI_INVALID_PARAMETER   - Command usage error
  EFI_NOT_FOUND           - Files not found

Notes:  
  dblk BlockDeviceName [LBA] [# Blocks]
    if no Address default address is LBA 0
    if no # Blocks then # Blocks is 1

--*/
{
  UINT64                    BlockAddress;
  UINTN                     NumBlocks;
  UINTN                     ByteSize;
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath;
  EFI_STATUS                Status;
  EFI_BLOCK_IO_PROTOCOL     *BlkIo;
  VOID                      *Buffer;

  SHELL_VAR_CHECK_CODE      RetCode;
  CHAR16                    *Useful;
  SHELL_ARG_LIST            *Item;
  SHELL_VAR_CHECK_PACKAGE   ChkPck;

  ZeroMem (&ChkPck, sizeof (SHELL_VAR_CHECK_PACKAGE));

  EFI_SHELL_APP_INIT (ImageHandle, SystemTable);

  //
  // Enable tab key which can pause the output
  //
  EnableOutputTabPause();
  
  //
  // Register our string package with HII and return the handle to it.
  // If previously registered we will simply receive the handle
  //
  Status = LibInitializeStrings (&HiiDblkHandle, STRING_ARRAY_NAME, &EfiDblkGuid);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (!EFI_PROPER_VERSION (0, 99)) {
    PrintToken (
      STRING_TOKEN (STR_SHELLENV_GNC_COMMAND_NOT_SUPPORT),
      HiiDblkHandle,
      L"dblk",
      EFI_VERSION_0_99 
      );
    Status = EFI_UNSUPPORTED;
    goto Done;
  }

  RetCode = LibCheckVariables (SI, DblkCheckList, &ChkPck, &Useful);
  if (VarCheckOk != RetCode) {
    switch (RetCode) {
    case VarCheckDuplicate:

      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_DUP_FLAG), HiiDblkHandle, L"dblk", Useful);
      break;

    case VarCheckUnknown:

      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_UNKNOWN_FLAG), HiiDblkHandle, L"dblk", Useful);
      break;

    default:
      break;
    }

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
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiDblkHandle, L"dblk");
      Status = EFI_INVALID_PARAMETER;
    } else {
      PrintToken (STRING_TOKEN (STR_HELPINFO_DBLK_VERBOSEHELP), HiiDblkHandle);
      Status = EFI_SUCCESS;
    }

    goto Done;
  }

  if (ChkPck.ValueCount < 1) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_FEW), HiiDblkHandle, L"dblk");
    goto Done;
  }

  if (ChkPck.ValueCount > 3) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiDblkHandle, L"dblk");
    goto Done;
  }

  Item        = ChkPck.VarList;
  DevicePath  = (EFI_DEVICE_PATH_PROTOCOL *) SE->GetMap (Item->VarStr);

  if (DevicePath == NULL) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_NO_MAPPED_DEV_EX), HiiDblkHandle, L"dblk", Item->VarStr);
    goto Done;
  }

  Item = Item->Next;
  //
  // Get block address and number of blocks
  //
  if (Item) {
    BlockAddress = StrToUInt (Item->VarStr, 16, &Status);
    if (EFI_ERROR (Status)) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_ARG), HiiDblkHandle, L"dblk", Item->VarStr);
      goto Done;
    }

    Item = Item->Next;
  } else {
    BlockAddress = 0;
  }

  if (Item) {
    NumBlocks = (UINTN) StrToUInt (Item->VarStr, 16, &Status);
    if (EFI_ERROR (Status)) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_ARG), HiiDblkHandle, L"dblk", Item->VarStr);
      goto Done;
    }

    Item = Item->Next;
  } else {
    NumBlocks = 1;
  }
  //
  // Check for the device mapping
  //
  Status = LibDevicePathToInterface (&gEfiBlockIoProtocolGuid, DevicePath, (VOID **) &BlkIo);
  if (EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_DEBUG_DEV_NOT_IO), HiiDblkHandle, L"dblk", Status);
    goto Done;
  }

  if (NumBlocks == 0) {
    PrintToken (STRING_TOKEN (STR_DEBUG_BLOCK_NOT_ZERO), HiiDblkHandle, L"dblk");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  if (NumBlocks > 0x10) {
    NumBlocks = 0x10;
  }

  if (BlockAddress > BlkIo->Media->LastBlock) {
    PrintToken (
      STRING_TOKEN (STR_DEBUG_BLOCK_ADDR_OUT_RANGE),
      HiiDblkHandle,
      L"dblk",
      BlkIo->Media->LastBlock
      );
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  if (BlockAddress + NumBlocks - 1 > BlkIo->Media->LastBlock) {
    NumBlocks = (UINT32) (BlkIo->Media->LastBlock - BlockAddress + 1);
  }
  //
  // Read and display block data
  //
  ByteSize  = BlkIo->Media->BlockSize * NumBlocks;
  Buffer    = AllocatePool (ByteSize);
  if (Buffer) {
    PrintToken (STRING_TOKEN (STR_DEBUG_LBA_SIZE), HiiDblkHandle, BlockAddress, ByteSize, BlkIo);
    Status = BlkIo->ReadBlocks (BlkIo, BlkIo->Media->MediaId, BlockAddress, ByteSize, Buffer);
    if (Status == EFI_SUCCESS) {
      PrivateDumpHex (2, 0, ByteSize, Buffer);
      DblkStructsPrint (Buffer, BlkIo->Media->BlockSize, BlockAddress, BlkIo);
    } else {
      PrintToken (STRING_TOKEN (STR_DEBUG_READ_ERROR), HiiDblkHandle, L"dblk", Status);
    }

    FreePool (Buffer);
  }

Done:
  LibCheckVarFreeVarList (&ChkPck);
  LibUnInitializeStrings ();
  return Status;
}

EFI_STATUS
InitializeDblkGetLineHelp (
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
  return LibCmdGetStringByToken (
          STRING_ARRAY_NAME,
          &EfiDblkGuid,
          STRING_TOKEN (STR_HELPINFO_DBLK_LINEHELP),
          Str
          );
}
