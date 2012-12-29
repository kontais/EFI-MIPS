/*++

Copyright (c) 2005 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name: 

  memmap.c
  
Abstract:

  Shell app "memmap"


Revision History

--*/

#include "EfiShellLib.h"
#include "memmap.h"

extern UINT8  STRING_ARRAY_NAME[];

//
// This is the generated header file which includes whatever needs to be exported (strings + IFR)
//
#include STRING_DEFINES_FILE

//
// Global Variables
//
EFI_HII_HANDLE  HiiHandle;
EFI_GUID        EfiMemmapGuid = EFI_MEMMAP_GUID;
SHELL_VAR_CHECK_ITEM    MemmapCheckList[] = {
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
EFI_STATUS
InitializeMemmap (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  );

BOOLEAN
IsRealMemory (
  IN  EFI_MEMORY_TYPE   Type
  );

//
//
//
EFI_BOOTSHELL_CODE(
  EFI_APPLICATION_ENTRY_POINT(InitializeMemmap)
)

EFI_STATUS
InitializeMemmap (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
/*++

Routine Description:

  Displays memory map.

Arguments:

  ImageHandle     The image handle. 
  SystemTable     The system table.

Returns:

  EFI_SUCCESS             - Command completed successfully
  EFI_INVALID_PARAMETER   - Command usage error
  EFI_NOT_FOUND           - Memory map not found

--*/
{
  EFI_STATUS              Status;
  CHAR16                  *Ptr;
  UINTN                   Index;
  UINTN                   DescriptorSize;
  UINT32                  DescriptorVersion;
  UINTN                   NoDesc;
  UINTN                   MapKey;
  UINT64                  Bytes;
  UINT64                  NoPages[EfiMaxMemoryType];
  UINT64                  TotalMemory;
  EFI_MEMORY_DESCRIPTOR   *Desc;
  EFI_MEMORY_DESCRIPTOR   *MemMap;
  SHELL_VAR_CHECK_CODE    RetCode;
  CHAR16                  *Useful;
  SHELL_VAR_CHECK_PACKAGE ChkPck;

  MemMap = NULL;
  //
  // We are no being installed as an internal command driver, initialize
  // as an nshell app and run
  //
  ZeroMem (&ChkPck, sizeof (SHELL_VAR_CHECK_PACKAGE));
  EFI_SHELL_APP_INIT (ImageHandle, SystemTable);
  
  //
  // Enable tab key which can pause the output
  //
  EnableOutputTabPause();

  Status = LibInitializeStrings (&HiiHandle, STRING_ARRAY_NAME, &EfiMemmapGuid);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (!EFI_PROPER_VERSION (0, 99)) {
    PrintToken (
      STRING_TOKEN (STR_SHELLENV_GNC_COMMAND_NOT_SUPPORT),
      HiiHandle,
      L"memmap",
      EFI_VERSION_0_99 
      );
    Status = EFI_UNSUPPORTED;
    goto Done;
  }

  RetCode = LibCheckVariables (SI, MemmapCheckList, &ChkPck, &Useful);
  if (VarCheckOk != RetCode) {
    switch (RetCode) {
    case VarCheckUnknown:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_UNKNOWN_FLAG), HiiHandle, L"memmap", Useful);
      break;

    case VarCheckDuplicate:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_DUP_FLAG), HiiHandle, L"memmap", Useful);
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
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiHandle, L"memmap");
      Status = EFI_INVALID_PARAMETER;
    } else {
      PrintToken (STRING_TOKEN (STR_MEMMAP_VERBOSE_HELP), HiiHandle);
      Status = EFI_SUCCESS;
    }

    goto Done;
  }

  if (ChkPck.ValueCount > 0) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiHandle, L"memmap");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }
  //
  // Get memory map and print out
  //
  MemMap = LibMemoryMap (&NoDesc, &MapKey, &DescriptorSize, &DescriptorVersion);
  if (!MemMap) {
    PrintToken (STRING_TOKEN (STR_MEMMAP_TABLE_NOT_FOUND), HiiHandle, L"memmap");
    Status = EFI_NOT_FOUND;
    goto Done;
  }

  ASSERT (DescriptorVersion == EFI_MEMORY_DESCRIPTOR_VERSION);
  SetMem (NoPages, EfiMaxMemoryType * sizeof (UINT64) , 0);

  Desc = MemMap;
  PrintToken (STRING_TOKEN (STR_MEMMAP_START_END), HiiHandle);
  for (Index = 0; Index < NoDesc; Index += 1) {

    if (GetExecutionBreak ()) {
      Status = EFI_ABORTED;
      goto Done;
    }

    Bytes = LShiftU64 (Desc->NumberOfPages, 12);
    Ptr   = MemoryTypeStr (Desc->Type);
    PrintToken (
      STRING_TOKEN (STR_MEMMAP_FIVE_ARGS),
      HiiHandle,
      Ptr,
      Desc->PhysicalStart,
      Desc->PhysicalStart + Bytes - 1,
      Desc->NumberOfPages,
      Desc->Attribute
      );

    //
    // count pages of each type memory
    //
    NoPages[Desc->Type] += Desc->NumberOfPages;
    Desc = NextMemoryDescriptor (Desc, DescriptorSize);
  }

  Print (L"\n");

  //
  // Print each memory type summary
  //
  for (Index = 0, TotalMemory = 0; Index < EfiMaxMemoryType; Index += 1) {

    if (GetExecutionBreak ()) {
      Status = EFI_ABORTED;
      goto Done;
    }

    if (NoPages[Index]) {
      PrintToken (
        STRING_TOKEN (STR_MEMMAP_PAGES),
        HiiHandle,
        MemoryTypeStr ((EFI_MEMORY_TYPE) Index),
        NoPages[Index],
        LShiftU64 (NoPages[Index], 12)
        );
      //
      // Count total memory
      //
      if (IsRealMemory ((EFI_MEMORY_TYPE) (Index))) {
        TotalMemory += NoPages[Index];
      }
    }
  }

  PrintToken (
    STRING_TOKEN (STR_MEMMAP_TOTAL_MEM),
    HiiHandle,
    RShiftU64 (TotalMemory, 8),
    LShiftU64 (TotalMemory, 12)
    );

Done:
  if (MemMap != NULL) {
    FreePool (MemMap);
  }

  LibCheckVarFreeVarList (&ChkPck);
  LibUnInitializeStrings ();
  return Status;
}

BOOLEAN
IsRealMemory (
  IN  EFI_MEMORY_TYPE   Type
  )
{
  if (Type == EfiLoaderCode ||
      Type == EfiLoaderData ||
      Type == EfiBootServicesCode ||
      Type == EfiBootServicesData ||
      Type == EfiRuntimeServicesCode ||
      Type == EfiRuntimeServicesData ||
      Type == EfiConventionalMemory ||
      Type == EfiACPIReclaimMemory ||
      Type == EfiACPIMemoryNVS ||
      Type == EfiPalCode
      ) {
    return TRUE;
  } else {
    return FALSE;
  }
}

EFI_STATUS
InitializeMemmapGetLineHelp (
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
  return LibCmdGetStringByToken (STRING_ARRAY_NAME, &EfiMemmapGuid, STRING_TOKEN (STR_MEMMAP_LINE_HELP), Str);
}
