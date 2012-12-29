/*++

Copyright (c) 2005 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  LoadPciRom.c
  
Abstract:

  Shell app "LoadPciRom"


Revision History

--*/

#include "EfiShellLib.h"
#include "LoadPciRom.h"
#include "Pci22.h"

extern UINT8    STRING_ARRAY_NAME[];

//
// This is the generated header file which includes whatever needs to be exported (strings + IFR)
//
#include STRING_DEFINES_FILE

#include "EfiImage.h"

EFI_STATUS
LoadPciRomConnectAllDriversToAllControllers (
  VOID
  );
//
//
//
EFI_STATUS
InitializeLoadPciRom (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  );

EFI_STATUS
LoadEfiDriversFromRomImage (
  VOID                      *RomBar,
  UINTN                     RomSize,
  CHAR16                    *FileName
  );

EFI_HANDLE      gMyImageHandle;
EFI_HII_HANDLE  HiiHandle;
EFI_GUID        EfiLoadPciRomGuid = EFI_LOADPCIROM_GUID;
SHELL_VAR_CHECK_ITEM    LPRCheckList[] = {
  {
    L"-nc",
    0x01,
    0,
    FlagTypeSingle
  },
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

//
//
//
EFI_BOOTSHELL_CODE(
  EFI_APPLICATION_ENTRY_POINT(InitializeLoadPciRom)
)

EFI_STATUS
InitializeLoadPciRom (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
/*++

Routine Description:

  Command entry pointer

Arguments:

  ImageHandle - The image handle
  SystemTable - The system table

Returns:

  EFI_INVALID_PARAMETER - Invalid parameter

--*/
{
  CHAR16                  **Argv;
  EFI_LIST_ENTRY          RomFileList;
  EFI_LIST_ENTRY          *Link;
  SHELL_FILE_ARG          *RomFileArg;
  UINTN                   SourceSize;
  UINT8                   *File1Buffer;
  EFI_STATUS              Status;

  BOOLEAN                 Connect;
  SHELL_VAR_CHECK_CODE    RetCode;
  CHAR16                  *Useful;
  SHELL_ARG_LIST          *Item;
  SHELL_VAR_CHECK_PACKAGE ChkPck;

  //
  // Local variable initializations
  //
  File1Buffer = NULL;
  InitializeListHead (&RomFileList);
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

  Status = LibInitializeStrings (&HiiHandle, STRING_ARRAY_NAME, &EfiLoadPciRomGuid);

  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (!EFI_PROPER_VERSION (1, 10)) {
    PrintToken (
      STRING_TOKEN (STR_SHELLENV_GNC_COMMAND_NOT_SUPPORT),
      HiiHandle,
      L"loadpcirom",
      EFI_VERSION_1_10
      );
    Status = EFI_UNSUPPORTED;
    goto Done;
  }

  gMyImageHandle = ImageHandle;
  Argv  = SI->Argv;

  //
  // verify number of arguments
  //
  LibFilterNullArgs ();
  RetCode = LibCheckVariables (SI, LPRCheckList, &ChkPck, &Useful);
  if (VarCheckOk != RetCode) {
    switch (RetCode) {
    case VarCheckUnknown:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_UNKNOWN_FLAG), HiiHandle, L"loadpcirom", Useful);
      break;

    case VarCheckDuplicate:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_DUP_FLAG), HiiHandle, L"loadpcirom", Useful);
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
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiHandle, L"loadpcirom");
      Status = EFI_INVALID_PARAMETER;
    } else {
      PrintToken (STRING_TOKEN (STR_HELPINFO_LOADPCIROM_VERBOSEHELP), HiiHandle);
      Status = EFI_SUCCESS;
    }
    goto Done;
  }

  if (ChkPck.ValueCount < 1) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_FEW), HiiHandle, L"loadpcirom");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  Connect = TRUE;
  if (LibCheckVarGetFlag (&ChkPck, L"-nc") != NULL) {
    Connect = FALSE;
  }
  //
  // open all the rom files
  //
  Item = ChkPck.VarList;
  while (Item != NULL) {
    Status = ShellFileMetaArg (Item->VarStr, &RomFileList);
    if (EFI_ERROR (Status) || IsListEmpty (&RomFileList)) {
      PrintToken (STRING_TOKEN (STR_LOADPCIROM_CANNOT_OPEN), HiiHandle, Argv[1], Status);
      goto Done;
    }

    Item = Item->Next;
  }

  for (Link = RomFileList.Flink; Link != &RomFileList; Link = Link->Flink) {

    RomFileArg = CR (Link, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);
    if (RomFileArg->Status != EFI_SUCCESS) {
      continue;
    }
    //
    // Open error
    //
    if (EFI_ERROR (RomFileArg->Status) || !RomFileArg->Handle) {
      PrintToken (STRING_TOKEN (STR_LOADPCIROM_CANNOT_OPEN), HiiHandle, RomFileArg->FullName, RomFileArg->Status);
      Status = RomFileArg->Status;
      goto Done;
    }
    //
    // directory
    //
    if (RomFileArg->Info && (RomFileArg->Info->Attribute & EFI_FILE_DIRECTORY)) {
      PrintToken (STRING_TOKEN (STR_LOADPCIROM_FIRST_ARG_NOT_DIR), HiiHandle);
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }
    //
    // Allocate buffers for both files
    //
    SourceSize  = (UINTN) RomFileArg->Info->FileSize;
    File1Buffer = AllocatePool (SourceSize);
    if (File1Buffer == NULL) {
      PrintToken (STRING_TOKEN (STR_LOADPCIROM_OUT_OF_MEM), HiiHandle);
      Status = EFI_OUT_OF_RESOURCES;
      goto Done;
    }

    Status = RomFileArg->Handle->Read (RomFileArg->Handle, &SourceSize, File1Buffer);
    if (EFI_ERROR (Status)) {
      PrintToken (STRING_TOKEN (STR_LOADPCIROM_READ_ERROR), HiiHandle, RomFileArg->FullName, Status);
      goto Done;
    }

    Status = LoadEfiDriversFromRomImage (
              File1Buffer,
              SourceSize,
              RomFileArg->FileName
              );

    PrintToken (STRING_TOKEN (STR_LOADPCIROM_IMAGE_LOADED), HiiHandle, RomFileArg->FileName, Status);

  }

  if (Connect) {
    Status = LoadPciRomConnectAllDriversToAllControllers ();
  }

Done:
  ShellFreeFileList (&RomFileList);
  LibCheckVarFreeVarList (&ChkPck);
  LibUnInitializeStrings ();
  return Status;
}

EFI_STATUS
LoadEfiDriversFromRomImage (
  VOID                      *RomBar,
  UINTN                     RomSize,
  CHAR16                    *FileName
  )
/*++

Routine Description:
  Command entry point. 

Arguments:

  RomBar   - Rom
  RomSize  - Rom size
  FileName - The file name

Returns:
  EFI_SUCCESS             - The command completed successfully
  EFI_INVALID_PARAMETER   - Command usage error
  EFI_UNSUPPORTED         - Protocols unsupported
  EFI_OUT_OF_RESOURCES    - Out of memory
  Other value             - Unknown error

--*/
{
  EFI_PCI_EXPANSION_ROM_HEADER  *EfiRomHeader;
  PCI_DATA_STRUCTURE            *Pcir;
  UINTN                         ImageIndex;
  UINTN                         RomBarOffset;
  UINT32                        ImageSize;
  UINT16                        ImageOffset;
  EFI_HANDLE                    ImageHandle;
  EFI_STATUS                    Status;
  EFI_STATUS                    retStatus;
  CHAR16                        RomFileName[280];
  EFI_DEVICE_PATH_PROTOCOL      *FilePath;
  BOOLEAN                       SkipImage;
  UINT32                        DestinationSize;
  UINT32                        ScratchSize;
  UINT8                         *Scratch;
  VOID                          *ImageBuffer;
  VOID                          *DecompressedImageBuffer;
  UINT32                        ImageLength;
  EFI_DECOMPRESS_PROTOCOL       *Decompress;

  ImageIndex    = 0;
  retStatus     = EFI_NOT_FOUND;
  RomBarOffset  = (UINTN) RomBar;

  do {

    EfiRomHeader = (EFI_PCI_EXPANSION_ROM_HEADER *) (UINTN) RomBarOffset;

    if (EfiRomHeader->Signature != 0xaa55) {
      PrintToken (STRING_TOKEN (STR_LOADPCIROM_IMAGE_CORRUPT), HiiHandle, ImageIndex);
      return retStatus;
    }

    Pcir      = (PCI_DATA_STRUCTURE *) (UINTN) (RomBarOffset + EfiRomHeader->PcirOffset);
    ImageSize = Pcir->ImageLength * 512;

    if ((Pcir->CodeType == PCI_CODE_TYPE_EFI_IMAGE) &&
        (EfiRomHeader->EfiSignature == EFI_PCI_EXPANSION_ROM_HEADER_EFISIGNATURE)
        ) {

      if ((EfiRomHeader->EfiSubsystem == EFI_IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER) ||
          (EfiRomHeader->EfiSubsystem == EFI_IMAGE_SUBSYSTEM_EFI_RUNTIME_DRIVER)
          ) {
        ImageOffset             = EfiRomHeader->EfiImageHeaderOffset;
        ImageSize               = EfiRomHeader->InitializationSize * 512;

        ImageBuffer             = (VOID *) (UINTN) (RomBarOffset + ImageOffset);
        ImageLength             = ImageSize - ImageOffset;
        DecompressedImageBuffer = NULL;

        //
        // decompress here if needed
        //
        SkipImage = FALSE;
        if (EfiRomHeader->CompressionType > EFI_PCI_EXPANSION_ROM_HEADER_COMPRESSED) {
          SkipImage = TRUE;
        }

        if (EfiRomHeader->CompressionType == EFI_PCI_EXPANSION_ROM_HEADER_COMPRESSED) {
          Status = BS->LocateProtocol (&gEfiDecompressProtocolGuid, NULL, &Decompress);
          if (EFI_ERROR (Status)) {
            PrintToken (STRING_TOKEN (STR_LOADPCIROM_DECOMP_NOT_FOUND), HiiHandle);
            SkipImage = TRUE;
          } else {
            SkipImage = TRUE;
            Status = Decompress->GetInfo (
                                  Decompress,
                                  ImageBuffer,
                                  ImageLength,
                                  &DestinationSize,
                                  &ScratchSize
                                  );
            if (!EFI_ERROR (Status)) {
              DecompressedImageBuffer = AllocatePool (DestinationSize);
              if (ImageBuffer != NULL) {
                Scratch = AllocatePool (ScratchSize);
                if (Scratch != NULL) {
                  Status = Decompress->Decompress (
                                        Decompress,
                                        ImageBuffer,
                                        ImageLength,
                                        DecompressedImageBuffer,
                                        DestinationSize,
                                        Scratch,
                                        ScratchSize
                                        );
                  if (!EFI_ERROR (Status)) {
                    ImageBuffer = DecompressedImageBuffer;
                    ImageLength = DestinationSize;
                    SkipImage   = FALSE;
                  }

                  FreePool (Scratch);
                }
              }
            }
          }
        }

        if (SkipImage == FALSE) {
          //
          // load image and start image
          //
          SPrint (RomFileName, sizeof (RomFileName), L"%s[%d]", FileName, ImageIndex);
          FilePath = FileDevicePath (NULL, RomFileName);

          Status = BS->LoadImage (
                        TRUE,
                        gMyImageHandle,
                        FilePath,
                        ImageBuffer,
                        ImageLength,
                        &ImageHandle
                        );
          if (EFI_ERROR (Status)) {
            PrintToken (STRING_TOKEN (STR_LOADPCIROM_LOAD_IMAGE_ERROR), HiiHandle, ImageIndex, Status);
          } else {
            Status = BS->StartImage (ImageHandle, NULL, NULL);
            if (EFI_ERROR (Status)) {
              PrintToken (STRING_TOKEN (STR_LOADPCIROM_START_IMAGE), HiiHandle, ImageIndex, Status);
            } else {
              retStatus = Status;
            }
          }
        }

        if (DecompressedImageBuffer != NULL) {
          FreePool (DecompressedImageBuffer);
        }

      }
    }

    RomBarOffset = RomBarOffset + ImageSize;
    ImageIndex++;
  } while (((Pcir->Indicator & 0x80) == 0x00) && ((RomBarOffset - (UINTN) RomBar) < RomSize));

  return retStatus;
}

EFI_STATUS
LoadPciRomConnectAllDriversToAllControllers (
  VOID
  )

{
  EFI_STATUS  Status;
  UINTN       AllHandleCount;
  EFI_HANDLE  *AllHandleBuffer;
  UINTN       Index;
  UINTN       HandleCount;
  EFI_HANDLE  *HandleBuffer;
  UINT32      *HandleType;
  UINTN       HandleIndex;
  BOOLEAN     Parent;
  BOOLEAN     Device;

  Status = LibLocateHandle (
            AllHandles,
            NULL,
            NULL,
            &AllHandleCount,
            &AllHandleBuffer
            );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  for (Index = 0; Index < AllHandleCount; Index++) {
    if (GetExecutionBreak ()) {
      Status = EFI_ABORTED;
      goto Done;
    }
    //
    // Scan the handle database
    //
    Status = LibScanHandleDatabase (
              NULL,
              NULL,
              AllHandleBuffer[Index],
              NULL,
              &HandleCount,
              &HandleBuffer,
              &HandleType
              );
    if (EFI_ERROR (Status)) {
      goto Done;
    }

    Device = TRUE;
    if (HandleType[Index] & EFI_HANDLE_TYPE_DRIVER_BINDING_HANDLE) {
      Device = FALSE;
    }

    if (HandleType[Index] & EFI_HANDLE_TYPE_IMAGE_HANDLE) {
      Device = FALSE;
    }

    if (Device) {
      Parent = FALSE;
      for (HandleIndex = 0; HandleIndex < HandleCount; HandleIndex++) {
        if (HandleType[HandleIndex] & EFI_HANDLE_TYPE_PARENT_HANDLE) {
          Parent = TRUE;
        }
      }

      if (!Parent) {
        if (HandleType[Index] & EFI_HANDLE_TYPE_DEVICE_HANDLE) {
          Status = BS->ConnectController (
                        AllHandleBuffer[Index],
                        NULL,
                        NULL,
                        TRUE
                        );
        }
      }
    }

    FreePool (HandleBuffer);
    FreePool (HandleType);
  }

Done:
  FreePool (AllHandleBuffer);
  return Status;
}

EFI_STATUS
InitializeLoadPciRomGetLineHelp (
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
          &EfiLoadPciRomGuid,
          STRING_TOKEN (STR_HELPINFO_LOADPCIROM_LINEHELP),
          Str
          );
}
