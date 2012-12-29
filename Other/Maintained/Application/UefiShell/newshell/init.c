/*++

Copyright (c) 2005 - 2007, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name: 

  init.c
  
Abstract:

  Shell entry point

--*/

#include "FakeHii.h"
#include "nshell.h"

extern UINT8    STRING_ARRAY_NAME[];
extern BOOLEAN  gHiiInitialized = FALSE;

//
// This is the generated header file which includes whatever needs to be exported (strings + IFR)
//
#include STRING_DEFINES_FILE

//
// Globals
//
CHAR16              *ShellEnvPathName[] = {
  L"shellenv.efi",
  L"efi\\shellenv.efi",
  L"efi\\tools\\shellenv.efi",
  NULL
};

EFI_CONSOLE_CONTROL_SCREEN_MODE  mOldCurrentMode = EfiConsoleControlScreenText;

EFI_SHELL_INTERFACE *OldSI;

EFI_STATUS
DumpShellPerformanceData (
  VOID
  );

EFI_HII_HANDLE      HiiNewshellHandle;

//
// Prototypes
//
EFI_STATUS
EFIAPI
InitializeShell (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
_ShellLoadEnvDriver (
  IN EFI_HANDLE           ImageHandle
  );

EFI_STATUS
NShellPrompt (
  IN EFI_HANDLE           ImageHandle
  );

BOOLEAN
ParseLoadOptions (
  EFI_HANDLE  ImageHandle,
  OUT CHAR16  **CommandLine,
  OUT CHAR16  **CurrentDir
  );

VOID
SetShellKeyFilter (
  IN   UINT32          ShellStatus
  );

EFI_STATUS
_GetFsDpOfImg (
  IN     EFI_HANDLE               ImgHnd,
  IN OUT EFI_DEVICE_PATH_PROTOCOL **DevPath,
  IN OUT EFI_DEVICE_PATH_PROTOCOL **FilePath
  )
{
  EFI_STATUS                Status;
  EFI_LOADED_IMAGE_PROTOCOL *img;
  EFI_DEVICE_PATH_PROTOCOL  *dp;

  Status = BS->HandleProtocol (
                ImgHnd,
                &gEfiLoadedImageProtocolGuid,
                &img
                );
  if (!EFI_ERROR (Status)) {
    Status = BS->HandleProtocol (
                  img->DeviceHandle,
                  &gEfiDevicePathProtocolGuid,
                  &dp
                  );
    if (!EFI_ERROR (Status)) {
      *DevPath  = DuplicateDevicePath (dp);
      *FilePath = DuplicateDevicePath (img->FilePath);
    }
  }

  return Status;
}

CHAR16 *
_GetValidFilePath (
  EFI_DEVICE_PATH_PROTOCOL        *FilePath
  )
{
  CHAR16  *p;
  CHAR16  *q;
  CHAR16  *Org;

  q   = LibDevicePathToStr (FilePath);
  Org = AllocateZeroPool (StrSize (q));
  if (NULL == Org) {
    return Org;
  }

  p = Org;
  while (*q) {
    *p = *q;
    p++;
    q++;
    if (*q == '\\' || *q == '/') {
      if (*(p - 1) == '\\') {
        q++;
      } else {
        *q = '\\';
      }
    }
  }

  return Org;
}

EFI_STATUS
_LocateStartup (
  IN     EFI_DEVICE_PATH_PROTOCOL *DevPath,
  IN     EFI_DEVICE_PATH_PROTOCOL *FilePath,
  IN OUT CHAR16                   **Startup
  )
{
  EFI_STATUS  Status;
  CHAR16      *p;
  CHAR16      *fs;
  UINTN       Size;

  Status = SE2->GetFsName (DevPath, FALSE, &fs);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  p = _GetValidFilePath (FilePath);
  if (NULL == p) {
    FreePool (fs);
    return EFI_OUT_OF_RESOURCES;
  }

  Size      = StrSize (fs) + StrSize (p) + sizeof (CHAR16) + StrSize (L"\\"STARTUP_FILE_NAME);
  *Startup  = AllocateZeroPool (Size);
  if (NULL == *Startup) {
    FreePool (p);
    FreePool (fs);
    return EFI_OUT_OF_RESOURCES;
  }

  SPrint (*Startup, Size, L"%s:%s", fs, p);
  FreePool (p);
  FreePool (fs);

  p = *Startup + StrLen (*Startup);
  while (p > *Startup) {
    if ('\\' == *p) {
      *p = 0;
      break;
    }

    p--;
  }

  if (*p) {
    FreePool (*Startup);
    return EFI_UNSUPPORTED;
  }

  StrCat (*Startup, L"\\"STARTUP_FILE_NAME);

  return EFI_SUCCESS;
}

EFI_STATUS
_DoInit (
  IN     EFI_HANDLE             ImageHandle,
  IN     EFI_SYSTEM_TABLE       *SystemTable
  )
{
  EFI_STATUS                       Status;
  EFI_CONSOLE_CONTROL_PROTOCOL     *ConsoleControl;
  EFI_GUID                         EfiLibGuid = EFI_NSHELL_GUID;
  InitializeShellLib (ImageHandle, SystemTable);

  //
  // If EFI_CONSOLE_CONTROL_PROTOCOL is available,
  // use it to switch to text mode first.
  //
  Status = LibLocateProtocol (
             &gEfiConsoleControlProtocolGuid,
             &ConsoleControl
             );
  if (!EFI_ERROR (Status)) {
    Status = ConsoleControl->GetMode (ConsoleControl, &mOldCurrentMode, NULL, NULL);
    if (!EFI_ERROR (Status) && mOldCurrentMode != EfiConsoleControlScreenText) {
      ConsoleControl->SetMode (ConsoleControl, EfiConsoleControlScreenText);
    }
  }

  //
  // When running under EFI1.1, there is no HII support
  // so we have to provide our embedded HII support
  // call FakeInitializeHiiDatabase to install our fake HII protocol if neccessary
  //
  Status = FakeInitializeHiiDatabase (
            ImageHandle,
            SystemTable
            );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Register our string package with HII and return the handle to it.
  //
  Status = LibInitializeStrings (&HiiNewshellHandle, STRING_ARRAY_NAME, &EfiLibGuid);

  return EFI_SUCCESS;
}

EFI_STATUS
_ProcessLoadOptions (
  IN     EFI_HANDLE           ImageHandle,
  IN     EFI_SYSTEM_TABLE     *SystemTable,
  IN OUT CHAR16               **CommandLine,
  IN OUT CHAR16               **CurDir
  )
{
  CHAR16  CmdLine[256];
  CHAR16  *Tmp;

  if (!ParseLoadOptions (ImageHandle, CommandLine, CurDir)) {
    return EFI_SUCCESS;
  }
  //
  // Skip the 1st argument which should be us.
  //
  while (**CommandLine != L' ' && **CommandLine != 0) {
    (*CommandLine)++;
  }
  //
  // Get to the beginning of the next argument.
  //
  while (**CommandLine == L' ') {
    (*CommandLine)++;
  }
  //
  // If there was a current working directory, set it.
  //
  if (*CurDir) {
    //
    // Set a mapping
    //
    CmdLine[0] = '@';
    StrCpy (CmdLine + 1, *CurDir);
    for (Tmp = CmdLine; *Tmp && *Tmp != L':'; Tmp++)
      ;
    if (*Tmp) {
      Tmp += 1;
      *Tmp = 0;
      StrCat (CmdLine, L" > NUL");
      ShellExecute (ImageHandle, CmdLine, FALSE);
    }
    //
    // Now change to that directory
    //
    StrCpy (CmdLine, L"@cd ");
    if ((StrLen (CmdLine) + StrLen (*CurDir) + sizeof (CHAR16)) < (sizeof (CmdLine) / sizeof (CHAR16))) {
      StrCat (CmdLine, *CurDir);
      StrCat (CmdLine, L" > NUL");
      ShellExecute (ImageHandle, CmdLine, FALSE);
    }
  }
  //
  // Have the shell execute the remaining command line.
  //
  if (**CommandLine != 0) {
    return ShellExecute (ImageHandle, *CommandLine, TRUE);
  }

  return EFI_SUCCESS;
}

EFI_STATUS
_EnableShellEnv (
  IN     EFI_HANDLE           ImageHandle,
  IN     EFI_SYSTEM_TABLE     *SystemTable
  )
{
  EFI_STATUS  Status;

  Status = EFI_ABORTED;
  EFI_MONOSHELL_CODE (
    //
    // Load shell environment in monolithic shell.
    //
    Status        = InitializeShellEnvironment (ImageHandle, SystemTable);
  )
  //
  // If the shell environment is not loaded, load it now.
  //
  if (EFI_ERROR (Status)) {
    EFI_BOOTSHELL_CODE(
      Status = _ShellLoadEnvDriver (ImageHandle);
    )
    if (EFI_ERROR (Status)) {
      Status = LibLocateProtocol (&ShellEnvProtocol, &SE);
      if (EFI_ERROR (Status)) {
        PrintToken (STRING_TOKEN (STR_NSHELL_ENV_DRIVER), HiiNewshellHandle);
        return Status;
      }
    }
  }

  return Status;
}

EFI_STATUS
_InstallShellInterface (
  IN EFI_HANDLE           ImageHandle,
  IN BOOLEAN              *IsRootInstance
  )
{
  EFI_STATUS  Status;

  ASSERT (IsRootInstance);

  Status = BS->HandleProtocol (
                ImageHandle,
                &ShellInterfaceProtocol,
                (VOID **) &OldSI
                );
  if (EFI_ERROR (Status)) {
    *IsRootInstance = TRUE;
  }

  Status = LibLocateProtocol (&ShellEnvProtocol, &SE);
  ASSERT (!EFI_ERROR (Status));
  SI = SE->NewShell (ImageHandle);

  if (*IsRootInstance) {
    Status = LibInstallProtocolInterfaces (
              &ImageHandle,
              &ShellInterfaceProtocol,
              SI,
              NULL
              );
  } else {
    Status = LibReinstallProtocolInterfaces (
              ImageHandle,
              &ShellInterfaceProtocol,
              OldSI,
              SI,
              NULL
              );
  }

  return Status;
}

EFI_STATUS
_ProcessStartup (
  IN EFI_HANDLE                    ImageHandle,
  IN EFI_DEVICE_PATH_PROTOCOL      *FsDp,
  IN EFI_DEVICE_PATH_PROTOCOL      *FileDp
  )
{
  EFI_STATUS      Status;
  UINT32          TimeOut;
  EFI_INPUT_KEY   Key;
  CHAR16          *Startup;
  EFI_FILE_HANDLE FHandle;
  CHAR16          *TimeOutString;

  //
  // Wait a key for 1s
  // So as to offer the opportunity to stop startup.nsh
  //
  TimeOut = 50;
  TimeOutString = SE->GetEnv(L"StartupDelay");
  if (TimeOutString != NULL) {
    TimeOut = (UINT32)StrToUInteger(TimeOutString, &Status) * 10;
    if (EFI_ERROR (Status)) {
      TimeOut = 50;
    }
  }
  Print (L"\n");
  Status  = ST->ConIn->ReadKeyStroke (ST->ConIn, &Key);
  while (TimeOut > 0 && EFI_ERROR (Status)) {
    if (0 == (TimeOut % 10)) {
      PrintToken (
        STRING_TOKEN (STR_NSHELL_SKIP_STARTUP),
        HiiNewshellHandle,
        TimeOut / 10
        );
    }

    BS->Stall (100000);
    Status = ST->ConIn->ReadKeyStroke (ST->ConIn, &Key);
    TimeOut--;
  }

  Print (L"\n");
  //
  // If user pressed 'ESC' key before running of Startup.nsh
  // Then skip the execution of statup.nsh
  //
  if (Status == EFI_SUCCESS) {
    if (Key.UnicodeChar == 0 && Key.ScanCode == SCAN_ESC) {
      return EFI_SUCCESS;
    }
  }
  //
  // Run startup script (if any)
  //
  Status  = EFI_NOT_FOUND;
  Startup = NULL;

  if (FsDp) {
    Status = _LocateStartup (FsDp, FileDp, &Startup);
    if (!EFI_ERROR (Status)) {
      Status = LibOpenFileByName (
                Startup,
                &FHandle,
                EFI_FILE_MODE_READ,
                0
                );
      if (!EFI_ERROR (Status)) {
        LibCloseFile (FHandle);
      }
    }
  }

  if (EFI_ERROR (Status)) {
    Startup = StrDuplicate (STARTUP_FILE_NAME);
  }

  Status = ShellExecute (ImageHandle, Startup, FALSE);
  FreePool (Startup);
  return Status;
}

EFI_STATUS
_CleanUpOnExit (
  IN EFI_HANDLE                     ImageHandle,
  IN EFI_SYSTEM_TABLE               *SystemTable
  )
{
  EFI_STATUS                       Status;
  EFI_STATUS                       ConsoleControlStatus;
  EFI_CONSOLE_CONTROL_PROTOCOL     *ConsoleControl;
  EFI_CONSOLE_CONTROL_SCREEN_MODE  CurrentMode;

  Status = EFI_SUCCESS;
  if (OldSI) {
    //
    // Retore the Shell Interface Protocol installed by parent nshell
    //
    Status = LibReinstallProtocolInterfaces (
              ImageHandle,
              &ShellInterfaceProtocol,
              SI,
              OldSI,
              NULL
              );
    ASSERT (!EFI_ERROR (Status));
  } else {
    //
    // This is the root, just un-install it
    //
    LibUninstallProtocolInterfaces (
      ImageHandle,
      &ShellInterfaceProtocol,
      SI,
      NULL
      );
  }

  if (SI != NULL) {
    FreePool (SI);
  }

  EFI_MONOSHELL_CODE (
    //
    // Uninstall Shell Environment Protocol
    //
    UnInstallShellEnvironment (ImageHandle, SystemTable);
  )

  //
  // Uninstall string registered for NShell
  //
  LibUnInitializeStrings ();
  FakeUninstallHiiDatabase();

  //
  // If EFI_CONSOLE_CONTROL_PROTOCOL is available,
  // switch back to the original console mode.
  //
  ConsoleControlStatus = LibLocateProtocol (
                           &gEfiConsoleControlProtocolGuid,
                           &ConsoleControl
                           );
  if (!EFI_ERROR (ConsoleControlStatus)) {
    ConsoleControlStatus = ConsoleControl->GetMode (ConsoleControl, &CurrentMode, NULL, NULL);
    if (!EFI_ERROR (ConsoleControlStatus) && CurrentMode != mOldCurrentMode) {
      ConsoleControl->SetMode (ConsoleControl, mOldCurrentMode);
    }
  }
  return Status;
}
//
// Shell entry point
//
EFI_DRIVER_ENTRY_POINT (InitializeShell)

EFI_STATUS
EFIAPI
InitializeShell (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
/*++

Routine Description:

Arguments:

  ImageHandle     - The handle for this driver

  SystemTable     - The system table

Returns:

--*/
{
  EFI_STATUS                Status;
  BOOLEAN                   IsRootInstance;
  CHAR16                    *CommandLine;
  CHAR16                    *CurrentDir;
  EFI_DEVICE_PATH_PROTOCOL  *FsDp;
  EFI_DEVICE_PATH_PROTOCOL  *FileDp;

  //
  // The shell may be started as either:
  //  1. the first time with no shell environment loaded yet
  //  2. not the first time, but with a shell environment loaded
  //  3. as a child of a parent shell image
  //
  IsRootInstance  = FALSE;

  Status          = _DoInit (ImageHandle, SystemTable);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = _EnableShellEnv (ImageHandle, SystemTable);
  if (EFI_ERROR (Status)) {
    //
    // Free all resources allocated in Shell
    //
    _CleanUpOnExit (
      ImageHandle,
      SystemTable
      );
    BS->Exit (ImageHandle, Status, 0, NULL);
  }

  ASSERT (!EFI_ERROR (Status));
  //
  // When the shell starts, turn off the watchdog timer
  //
  BS->SetWatchdogTimer (0x0000, 0x0000, 0x0000, NULL);
  //
  // Check to see if we're a child of a previous shell.
  //
  Status = _InstallShellInterface (ImageHandle, &IsRootInstance);
  ASSERT (!EFI_ERROR (Status));

  //
  // Now we can initialize like a normal shell app.
  //
  EFI_SHELL_APP_INIT (ImageHandle, SystemTable);

  //
  // Since command '_load_defaults' and 'map -r' might invalidate
  // the fs related handles, so we should get the device paths of
  // file path and file system device before they are executed.
  //
  Status = _GetFsDpOfImg (
            ImageHandle,
            &FsDp,
            &FileDp
            );
  if (EFI_ERROR (Status)) {
    FsDp    = NULL;
    FileDp  = NULL;
  }
  //
  // Set shell environment defaults if necessary
  //
  EFI_MONOSHELL_CODE (
    ShellExecute (ImageHandle, L"_load_defaults", TRUE);
    PrintToken (
      STRING_TOKEN (STR_NSHELL_VERSION),
      HiiNewshellHandle,
      (ST->Hdr.Revision >> 16),
      (ST->Hdr.Revision & 0xffff),
      (ST->FirmwareRevision >> 16),
      (ST->FirmwareRevision & 0xffff),
      EFI_SHELL_ENHANCED_MODE_VER
      );
    //
    // Dump device mappings, -r to sync with current hardware
    //
    ShellExecute (ImageHandle, L"map -r -f", TRUE);
  )

  ASSERT (ST->ConIn != NULL);

  //
  // If there are load options, assume they contain a command line and
  // possible current working directory.
  //
  _ProcessLoadOptions (
    ImageHandle,
    SystemTable,
    &CommandLine,
    &CurrentDir
    );
  EFI_NO_MONOSHELL_CODE (
    if (CommandLine != NULL && CommandLine[0] != 0) {
      if (FsDp) {
        FreePool (FsDp);
        FreePool (FileDp);
      }
  
      Status = EFI_SUCCESS;
      goto Done;
    }
  )

  if (IsRootInstance) {
    Status = _ProcessStartup (
              ImageHandle,
              FsDp,
              FileDp
              );
  }

  if (FsDp) {
    FreePool (FsDp);
    FreePool (FileDp);
  }
  //
  // EFI Shell main loop
  //
  SE2->IncrementShellNestingLevel ();

  Status = EFI_SUCCESS;
  while (Status != -1) {
    Status = NShellPrompt (ImageHandle);
    EFI_NT_EMULATOR_CODE (
      //
      // In NT32, all the individual images share one single DLL,
      // the cleanup of nested child might damage the parent's
      // global data.
      //
      // here we re-init SI to solve this problem, only for NT32
      //
      if (-2 == Status) {
        Status = BS->HandleProtocol (
                      ImageHandle,
                      &ShellInterfaceProtocol,
                      &SI
                      );
        ASSERT (!EFI_ERROR (Status));
      }
    )
  }

  SE2->DecrementShellNestingLevel ();

  //
  // Set status to -2 to inform parent shell that a nested
  // child shell has just quited.
  //
  Status = IsRootInstance ? EFI_SUCCESS : (EFI_STATUS) -2;
  EFI_NO_MONOSHELL_CODE (
    Done :
  )
  //
  // Free all resources allocated in Shell
  //
  _CleanUpOnExit (
    ImageHandle,
    SystemTable
    );
  return Status;
}

EFI_STATUS
_ShellLoadEnvDriverByPath (
  IN EFI_HANDLE           ParentImageHandle,
  IN EFI_HANDLE           DeviceHandle
  )
{
  EFI_STATUS                Status;
  EFI_DEVICE_PATH_PROTOCOL  *FilePath;
  EFI_HANDLE                NewImageHandle;
  UINTN                     Index;
  BOOLEAN                   SearchNext;

  Status = EFI_SUCCESS;

  //
  // If there's no device to search forget it
  //
  if (!DeviceHandle) {
    return EFI_NOT_FOUND;
  }
  //
  // Try loading shellenv from each path
  //
  SearchNext = TRUE;
  for (Index = 0; ShellEnvPathName[Index] && SearchNext; Index++) {
    //
    // Load it
    //
    FilePath = FileDevicePath (DeviceHandle, ShellEnvPathName[Index]);
    ASSERT (FilePath);
    Status = BS->LoadImage (
                  FALSE,
                  ParentImageHandle,
                  FilePath,
                  NULL,
                  0,
                  &NewImageHandle
                  );
    FreePool (FilePath);

    //
    // Only search the next path if it was not found on this path
    //
    SearchNext = FALSE;
    if (Status == EFI_LOAD_ERROR || Status == EFI_NOT_FOUND) {
      SearchNext = TRUE;
    }
    //
    // If there was no error, start the image
    //
    if (!EFI_ERROR (Status)) {
      Status = BS->StartImage (NewImageHandle, NULL, NULL);
    }
  }

  return Status;
}

EFI_STATUS
_ShellLoadEnvDriver (
  IN EFI_HANDLE           ImageHandle
  )
{
  EFI_STATUS                Status;
  EFI_LOADED_IMAGE_PROTOCOL *Image;
  UINTN                     Index;
  UINTN                     NoHandles;
  EFI_HANDLE                *Handles;
  EFI_SHELL_ENVIRONMENT2    *Junk;
  EFI_GUID                  SESGuid = EFI_SE_EXT_SIGNATURE_GUID;
  //
  // If we already have a loaded driver, we don't need to load anymore.
  //
  Status = LibLocateProtocol (
            &ShellEnvProtocol,
            (VOID *) &Junk
            );
  if (!EFI_ERROR (Status)) {
    if (CompareGuid (&Junk->SESGuid, &SESGuid) == 0) {
      if (Junk->MajorVersion >= EFI_NSHELL_MAJOR_VERSION) {
        return Status;
      }
    }
  }
  //
  // Get the file path for the current image
  //
  Status = BS->HandleProtocol (
                ImageHandle,
                &gEfiLoadedImageProtocolGuid,
                (VOID *) &Image
                );
  ASSERT (!EFI_ERROR (Status));

  //
  // Attempt to load shellenv
  //
  Status = _ShellLoadEnvDriverByPath (Image->ParentHandle, Image->DeviceHandle);
  if (EFI_ERROR (Status)) {
    //
    // shellenv was not found.  Search all file systems for it
    //
    Status = LibLocateHandle (
              ByProtocol,
              &gEfiSimpleFileSystemProtocolGuid,
              NULL,
              &NoHandles,
              &Handles
              );

    for (Index = 0; Index < NoHandles; Index++) {
      Status = _ShellLoadEnvDriverByPath (Image->ParentHandle, Handles[Index]);
      if (!EFI_ERROR (Status)) {
        break;
      }
    }

    if (Handles) {
      FreePool (Handles);
    }
  }
  //
  // Done
  //
  if (!EFI_ERROR (Status)) {
    Status = LibLocateProtocol (
              &ShellEnvProtocol,
              (VOID *) &Junk
              );
    if (!EFI_ERROR (Status)) {
      Status = EFI_UNSUPPORTED;
      if (CompareGuid (&Junk->SESGuid, &SESGuid) == 0) {
        if (Junk->MajorVersion >= EFI_NSHELL_MAJOR_VERSION) {
          Status = EFI_SUCCESS;
        }
      }
    }
  }

  return Status;
}

EFI_STATUS
NShellPrompt (
  IN EFI_HANDLE           ImageHandle
  )
{
  UINTN       Column;
  UINTN       Row;
  CHAR16      *CmdLine;
  CHAR16      *CurDir;
  UINTN       BufferSize;
  EFI_STATUS  Status;
  UINTN       Index;
  BOOLEAN     AddLine;

  //
  // If there is no console output device, then a shell prompt can not be
  // displayed, so ASSERT
  //
  ASSERT (ST->ConOut != NULL);

  //
  // Prompt for input
  //
  ST->ConOut->SetCursorPosition (ST->ConOut, 0, ST->ConOut->Mode->CursorRow);

  CurDir = ShellCurDir (NULL);
  if (CurDir) {
    PrintToken (STRING_TOKEN (STR_NSHELL_CURDIR), HiiNewshellHandle, CurDir);
    FreePool (CurDir);
  } else {
    PrintToken (STRING_TOKEN (STR_NSHELL_ESHELL), HiiNewshellHandle);
  }
  //
  // Get screen setting to decide size of the command line buffer
  //
  ST->ConOut->QueryMode (ST->ConOut, ST->ConOut->Mode->Mode, &Column, &Row);
  BufferSize  = Column * Row * sizeof (CHAR16);
  CmdLine     = AllocateZeroPool (BufferSize);
  if (CmdLine == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  //
  // If there is no console input device, then a command can not be read,
  // so ASSERT
  //
  ASSERT (ST->ConIn != NULL);

  SetShellKeyFilter (EFI_SHELL_PROMPT);

  //
  // Read a line from the console
  //
  Status = SI->StdIn->Read (SI->StdIn, &BufferSize, CmdLine);

  //
  // Null terminate the string and parse it
  //
  if (!EFI_ERROR (Status)) {
    CmdLine[BufferSize / sizeof (CHAR16)] = 0;
    AddLine = FALSE;
    for (Index = 0; CmdLine[Index] != 0; Index++) {
      if (CmdLine[Index] != L' ') {
        AddLine = TRUE;
        break;
      }
    }

    SetShellKeyFilter (EFI_SHELL_PREEXECUTION);
    Status = ShellExecute (ImageHandle, CmdLine, TRUE);
    SetShellKeyFilter (EFI_SHELL_POSTEXECUTION);

    if (AddLine) {
      Print (L"\n");
    }
  }
  //
  // Done with this command
  //
  FreePool (CmdLine);
  return Status;
}

BOOLEAN
ParseLoadOptions (
  EFI_HANDLE  ImageHandle,
  OUT CHAR16  **CommandLine,
  OUT CHAR16  **CurrentDir
  )
{
  EFI_LOADED_IMAGE_PROTOCOL *Image;
  EFI_STATUS                Status;

  //
  // Set defaults.
  //
  *CommandLine  = NULL;
  *CurrentDir   = NULL;

  Status = BS->HandleProtocol (
                ImageHandle,
                &gEfiLoadedImageProtocolGuid,
                (VOID *) &Image
                );

  if (!EFI_ERROR (Status)) {

    CHAR16  *CmdLine;

    //
    // Make sure it is power of 2
    //
    UINT32  CmdSize;

    CmdLine = Image->LoadOptions;
    CmdSize = Image->LoadOptionsSize &~1;

    if (CmdLine && CmdSize) {
      //
      // Set command line pointer for caller
      //
      *CommandLine = CmdLine;

      //
      // See if current working directory was passed.
      //
      while ((*CmdLine != 0) && CmdSize) {
        CmdLine++;
        CmdSize -= sizeof (CHAR16);
      }
      //
      // If a current working directory was passed, set it.
      //
      if (CmdSize > sizeof (CHAR16)) {
        CmdLine += 1;
        *CurrentDir = CmdLine;
      }

      return TRUE;
    }
  }

  return FALSE;
}

VOID
SetShellKeyFilter (
  IN   UINT32          ShellStatus
  )
/*++

Routine Description:
  Set related key filter according to current Shell status

Arguments:

  ShellStatus     - A const to show the current Shell status.

Returns:  

--*/
{
  switch (ShellStatus) {
  case EFI_SHELL_PROMPT:
    SetKeyFilter (EFI_OUTPUT_SCROLL);
    break;

  case EFI_SHELL_PREEXECUTION:
    SetKeyFilter (EFI_OUTPUT_PAUSE | EFI_EXECUTION_BREAK);

  case EFI_SHELL_POSTEXECUTION:
    break;

  default:
    break;
  }
}
