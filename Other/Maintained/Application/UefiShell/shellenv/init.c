/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  init.c
   
Abstract:

  Shell Environment driver

Revision History

--*/

#include "shelle.h"
#include "shellenvguid.h"

extern UINT8              STRING_ARRAY_NAME[];
extern EFI_STATUS
FakeInitializeHiiDatabase (
  EFI_HANDLE,
  EFI_SYSTEM_TABLE *,
  EFI_HII_PROTOCOL **
  );

EFI_LIST_ENTRY            SEnvCurMapping;
ENV_SHELL_INTERFACE_ITEM  *mOldSEnv = NULL;
//
//
//
EFI_STATUS
EFIAPI
InitializeShellEnvironment (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

//
//
//
EFI_BOOTSHELL_CODE(
  EFI_DRIVER_ENTRY_POINT (InitializeShellEnvironment)
)

EFI_STATUS
EFIAPI
InitializeShellEnvironment (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
/*++

Routine Description:

Arguments:

  ImageHandle     - The handle for this driver

  SystemTable     - The system table

Returns:

  EFI_SUCCESS     - EFI file system driver is enabled

--*/
{
  EFI_STATUS                Status;
  UINTN                     HandleNum;
  EFI_HANDLE                *HandleBuffer;
  ENV_SHELL_INTERFACE_ITEM  *OldInterface;
  EFI_GUID                  EfiShellEnvGuid = EFI_SHELLENV_GUID;
  //
  // Initialize EFI library
  //
  InitializeShellLib (ImageHandle, SystemTable);
  //
  // Register our string package with HII and return the handle to it.
  // If previously registered we will simply receive the handle
  //
  Status = LibInitializeStrings (&HiiEnvHandle, STRING_ARRAY_NAME, &EfiShellEnvGuid);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Initialize globals
  //
  InitializeLock (&SEnvLock, EFI_TPL_APPLICATION);
  InitializeLock (&SEnvGuidLock, EFI_TPL_NOTIFY);

  if (SEnvCmds.Flink == NULL) {
    SEnvInitCommandTable ();
  }

  SEnvInitProtocolInfo ();
  SEnvInitVariables ();
  SEnvInitHandleGlobals ();
  SEnvInitMap ();
  SEnvLoadInternalProtInfo ();
  SEnvConIoInitDosKey ();
  SEnvInitBatch ();

  //
  // Open the console proxy to substitute for the console in system table
  //
  SEnvOpenConsoleProxy (
    SystemTable->ConsoleInHandle,
    &SystemTable->ConIn,
    SystemTable->ConsoleOutHandle,
    &SystemTable->ConOut
    );

  //
  // Install shellenv handle (or override the existing one)
  //
  OldInterface = AllocateZeroPool (sizeof (ENV_SHELL_INTERFACE_ITEM));
  ASSERT (OldInterface);
  OldInterface->Next  = mOldSEnv;
  mOldSEnv            = OldInterface;
  Status = LibLocateHandle (
            ByProtocol,
            &ShellEnvProtocol,
            NULL,
            &HandleNum,
            &HandleBuffer
            );
  if (EFI_ERROR (Status)) {
    Status = BS->InstallProtocolInterface (
                  &ImageHandle,
                  &ShellEnvProtocol,
                  EFI_NATIVE_INTERFACE,
                  &SEnvInterface2
                  );
    ASSERT (!EFI_ERROR (Status));
  } else {
    //
    // For EFI1.1 only support one SEnv protocol, here, we assume
    // that there are only one Handle can be located.
    //
    mOldSEnv->Handle = HandleBuffer[0];
    FreePool (HandleBuffer);
    Status = BS->HandleProtocol (
                  mOldSEnv->Handle,
                  &ShellEnvProtocol,
                  &(mOldSEnv->Interface)
                  );
    ASSERT (!EFI_ERROR (Status));
    Status = BS->ReinstallProtocolInterface (
                  mOldSEnv->Handle,
                  &ShellEnvProtocol,
                  mOldSEnv->Interface,
                  &SEnvInterface2
                  );
    ASSERT (!EFI_ERROR (Status));
  }
  //
  // Initialize linked list for current mapping
  //
  InitializeListHead (&SEnvCurMapping);

  return EFI_SUCCESS;
}

VOID
EFIAPI
SEnvFreeResources (
  VOID
  )
/*++

Routine Description:
  Free all resources allocated in Shell

Arguments:

Returns:

--*/
{
  //
  // 1st we should remove strings registered for shell environment
  //
  EFI_NO_NT_EMULATOR_CODE (
    //
    // In NT32, if you don't delete the shell.dll, all the lauched shell.efi actually will share
    // the same dll, so the global variables are exactly the same copy. If a nested child shell freed
    // the command table, the parent will get nothing in that table when it gets control again.
    //
    // So, we choose to not free the table in NT32. Another way could be deleting the shell.dll before
    // execution of shell.
    //
    SEnvFreeCommandTable ();
  )
  LibUnInitializeStrings ();
  //
  // we need more code here to free resources.
  //
}

EFI_SHELL_INTERFACE *
EFIAPI
SEnvNewShell (
  IN EFI_HANDLE                   ImageHandle
  )
{
  EFI_SHELL_INTERFACE *ShellInt;
  EFI_STATUS          Status;

  //
  // Allocate a new structure
  //
  ShellInt = AllocateZeroPool (sizeof (EFI_SHELL_INTERFACE));
  ASSERT (ShellInt);

  //
  // Fill in the SI pointer
  //
  Status = BS->HandleProtocol (
                ImageHandle,
                &gEfiLoadedImageProtocolGuid,
                (VOID *) &ShellInt->Info
                );

  if (EFI_ERROR (Status) || ShellInt->Info == NULL) {
    FreePool (ShellInt);
    return NULL;
  }
  //
  // Fill in the std file handles
  //
  ShellInt->ImageHandle = ImageHandle;
  ShellInt->StdIn       = &SEnvIOFromCon;
  ShellInt->StdOut      = &SEnvIOFromCon;
  ShellInt->StdErr      = &SEnvErrIOFromCon;

  return ShellInt;
}

EFI_STATUS
UnInstallShellEnvironment (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  EFI_STATUS                Status;
  ENV_SHELL_INTERFACE_ITEM  *Temp;
  ASSERT (mOldSEnv);
  if (mOldSEnv->Interface == NULL) {
    Status = BS->UninstallProtocolInterface (
                  ImageHandle,
                  &ShellEnvProtocol,
                  &SEnvInterface2
                  );
    ASSERT (!EFI_ERROR (Status));
  } else {
    Status = BS->ReinstallProtocolInterface (
                  mOldSEnv->Handle,
                  &ShellEnvProtocol,
                  &SEnvInterface2,
                  mOldSEnv->Interface
                  );
    ASSERT (!EFI_ERROR (Status));
  }

  Temp      = mOldSEnv;
  mOldSEnv  = mOldSEnv->Next;
  FreePool (Temp);

  //
  // Close the console proxy opened in Shell to restore the
  // console in system table
  //
  SEnvCloseConsoleProxy (
    SystemTable->ConsoleInHandle,
    &SystemTable->ConIn,
    SystemTable->ConsoleOutHandle,
    &SystemTable->ConOut
    );

  SEnvFreeResources ();
  return Status;
}
