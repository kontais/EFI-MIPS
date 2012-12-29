/*++

Copyright (c) 2005 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  mem.c
  
Abstract: 

  shell command "dmem".

Revision History

--*/

#include "MemCommonPart.h"

EFI_STATUS
DumpMem (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_BOOTSHELL_CODE(
  EFI_APPLICATION_ENTRY_POINT(DumpMem)
)

EFI_STATUS
DumpMem (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
/*++
Routine Description:

  mem [Address] [Size] -MMIO
    if no Address default address is EFI System Table
    if no size default size is 512;
    if -MMIO then use memory mapped IO and not system memory
    
Arguments:
 
   ImageHandle - The image handle
   SystemTable - The system table

--*/
{
  EFI_STATUS  Status;

  //
  // The End Device Path represents the Root of the tree, thus get the global IoDev
  //  for the system
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
  Status = LibInitializeStrings (&HiiDmemHandle, STRING_ARRAY_NAME, &EfiDmemGuid);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (!EFI_PROPER_VERSION (1, 10)) {
    PrintToken (
      STRING_TOKEN (STR_SHELLENV_GNC_COMMAND_NOT_SUPPORT),
      HiiDmemHandle,
      L"dmem",
      EFI_VERSION_1_10
      );
    Status = EFI_UNSUPPORTED;
    goto Done;
  }

  if (IS_OLD_SHELL) {
    Status = DmemMainProcOld (ImageHandle, SystemTable);
  } else {
    Status = DmemMainProc (ImageHandle, SystemTable);
  }

Done:
  LibUnInitializeStrings ();

  return Status;
}

EFI_STATUS
DumpMemGetLineHelp (
  OUT CHAR16          **Str
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
  return LibCmdGetStringByToken (STRING_ARRAY_NAME, &EfiDmemGuid, STRING_TOKEN (STR_DMEM_LINE_HELP), Str);
}
