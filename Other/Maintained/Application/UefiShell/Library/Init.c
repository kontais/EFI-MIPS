/*++

Copyright (c) 2005 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:
  
  Init.c


Abstract:




Revision History

--*/

#include "EfiShellLib.h"

extern EFI_UNICODE_COLLATION_PROTOCOL LibStubUnicodeInterface;

extern EFI_GUID                       ShellEnvProtocol;

VOID
InitializeShellLib (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
/*++

Routine Description:

  Initializes EFI library for use
    
Arguments:

  ImageHandle - Image handle
  SystemTable - Firmware's EFI system table
    
Returns:

  None

--*/
{
  EFI_LOADED_IMAGE_PROTOCOL *LoadedImage;
  EFI_STATUS                Status;
  CHAR8                     *LangCode;

  if (!ShellLibInitialized) {
    ShellLibInitialized = TRUE;

    //
    // Set up global pointer to the system table, boot services table,
    // and runtime services table
    //
    ST  = SystemTable;
    BS  = SystemTable->BootServices;
    RT  = SystemTable->RuntimeServices;

    //
    // Initialize pool allocation type
    //
    if (ImageHandle) {
      Status = BS->HandleProtocol (
                    ImageHandle,
                    &gEfiLoadedImageProtocolGuid,
                    (VOID *) &LoadedImage
                    );

      if (!EFI_ERROR (Status)) {
        PoolAllocationType = LoadedImage->ImageDataType;
      }

    }
    //
    // Initialize Guid table
    //
    InitializeLibPlatform (ImageHandle, SystemTable);
  }

  if (ImageHandle && UnicodeInterface == &LibStubUnicodeInterface) {
    LangCode  = LibGetVariable (VarLanguage, &gEfiGlobalVariableGuid);
    Status    = InitializeUnicodeSupport (LangCode);
    if (EFI_ERROR (Status)) {
      Status = InitializeUnicodeSupport (LanguageCodeEnglish);
    }

    if (LangCode) {
      FreePool (LangCode);
    }
  }

  DEBUG_CODE (
    EFIDebugVariable ();
    EfiDebugAssertInit ();
  )
}

EFI_STATUS
InitializeUnicodeSupport (
  CHAR8 *LangCode
  )
/*++

Routine Description:

  Initializes Unicode support
    
Arguments:

  LangCode - Language Code specified
    
Returns:

  None

--*/
{
  EFI_UNICODE_COLLATION_PROTOCOL  *Ui;
  EFI_STATUS                      Status;
  CHAR8                           *Languages;
  UINTN                           Index;
  UINTN                           Position;
  UINTN                           Length;
  UINTN                           NoHandles;
  EFI_HANDLE                      *Handles;
  EFI_STATUS                      ReturnStatus;

  ReturnStatus = EFI_UNSUPPORTED;
  //
  // If we don't know it, lookup the current language code
  //
  LibLocateHandle (ByProtocol, &gEfiUnicodeCollationProtocolGuid, NULL, &NoHandles, &Handles);
  if (!LangCode || !NoHandles) {
    goto Done;
  }
  //
  // Check all driver's for a matching language code
  //
  for (Index = 0; Index < NoHandles; Index++) {
    Status = BS->HandleProtocol (Handles[Index], &gEfiUnicodeCollationProtocolGuid, (VOID *) &Ui);
    if (EFI_ERROR (Status)) {
      continue;
    }
    //
    // Check for a matching language code
    //
    Languages = Ui->SupportedLanguages;
    Length    = strlena (Languages);
    for (Position = 0; Position < Length; Position += ISO_639_2_ENTRY_SIZE) {
      //
      // If this code matches, use this driver
      //
      if (CompareMem (Languages + Position, LangCode, ISO_639_2_ENTRY_SIZE) == 0) {
        UnicodeInterface  = Ui;
        ReturnStatus      = EFI_SUCCESS;
        goto Done;
      }
    }
  }

Done:
  //
  // Cleanup
  //
  if (Handles) {
    FreePool (Handles);
  }

  return ReturnStatus;
}

EFI_HII_PROTOCOL *HiiProt        = NULL;
EFI_HII_HANDLE   HiiLibHandle    = (EFI_HII_HANDLE) 0;
BOOLEAN          HiiInitialized  = FALSE;
UINTN            NumStrings      = 0;
EFI_GUID         SESGuid         = EFI_SE_EXT_SIGNATURE_GUID;

EFI_STATUS
LibInitializeStrings (
  OUT EFI_HII_HANDLE    *HiiHandle,
  IN UINT8              *StringPack,
  IN EFI_GUID           *StringPackGuid
  )
/*++

Routine Description:
  Register our included string package to HII and return the HII handle to the data.
  If previously registered, simply return the handle.

Arguments:

  HiiLibHandle    - A pointer to the handle which is used to reference our string data.
  StringPack      - String package
  StringPackGuid  - String package guid

Returns:
  EFI_SUCCESS     - Command completed successfully

--*/
{
  EFI_STATUS        Status;
  EFI_HII_PACKAGES  *HiiPackages;
  VOID              **Package;

  ASSERT (HiiHandle);
  
  //
  // If we've already initialized once, then don't do it again. This is
  // done to support building shell commands standalone.
  //
  if (HiiInitialized == TRUE) {
    NumStrings++;
    *HiiHandle = HiiLibHandle;
    return EFI_SUCCESS;
  }

  HiiInitialized = TRUE;
  //
  // Find the HII protocol
  //
  Status = LibLocateProtocol (&gEfiHiiProtocolGuid, (VOID **) &HiiProt);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  HiiPackages = AllocateZeroPool (sizeof (EFI_HII_PACKAGES) + sizeof (VOID *));
  if (HiiPackages == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  HiiPackages->GuidId           = StringPackGuid;
  HiiPackages->NumberOfPackages = 1;
  Package                       = (VOID **) (((UINT8 *) HiiPackages) + sizeof (EFI_HII_PACKAGES));
  *Package                      = (EFI_HII_STRING_PACK *) StringPack;
  Status                        = HiiProt->NewPack (HiiProt, HiiPackages, HiiHandle);
  NumStrings++;

  HiiLibHandle = *HiiHandle;
  FreePool (HiiPackages);
  return Status;
}

EFI_STATUS
LibUnInitializeStrings (
  VOID
  )
{
  EFI_STATUS  Status;

  Status = EFI_SUCCESS;
  NumStrings--;
  if (HiiProt != NULL && NumStrings == 0) {
    Status = HiiProt->RemovePack (HiiProt, HiiLibHandle);
  }

  return Status;
}

EFI_STATUS
LibInitializeShellApplication (
  IN EFI_HANDLE                   ImageHandle,
  IN EFI_SYSTEM_TABLE             *SystemTable
  )
{
  EFI_STATUS  Status;
  EFI_HANDLE  *HandleBuffer;
  UINTN       HandleNum;
  UINTN       HandleIndex;
  //
  // Shell app lib is a super set of the default lib.
  // Initialize the default lib first
  //
  InitializeShellLib (ImageHandle, SystemTable);
  //
  // Connect to the shell interface
  //
  Status = BS->HandleProtocol (ImageHandle, &ShellInterfaceProtocol, (VOID *) &SI);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "InitShellApp: Application not started from Shell\n"));
    BS->Exit (ImageHandle, Status, 0, NULL);
  }
  //
  // Connect to the shell environment
  //
  Status = BS->HandleProtocol (
                ImageHandle,
                &ShellEnvProtocol,
                (VOID *) &SE2
                );
  if (EFI_ERROR (Status) || !(CompareGuid (&SE2->SESGuid, &SESGuid) == 0 &&
    (SE2->MajorVersion > EFI_SHELL_MAJOR_VER ||
      (SE2->MajorVersion == EFI_SHELL_MAJOR_VER && SE2->MinorVersion >= EFI_SHELL_MINOR_VER))
    )
  ) {
    Status = LibLocateHandle (
              ByProtocol,
              &ShellEnvProtocol,
              NULL,
              &HandleNum,
              &HandleBuffer
              );
    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR, "InitShellApp: Shell environment interfaces not found\n"));
      BS->Exit (ImageHandle, Status, 0, NULL);
    }

    Status = EFI_NOT_FOUND;
    for (HandleIndex = 0; HandleIndex < HandleNum; HandleIndex++) {
      BS->HandleProtocol (
           HandleBuffer[HandleIndex],
           &ShellEnvProtocol,
           (VOID *) &SE2
           );
      if (CompareGuid (&SE2->SESGuid, &SESGuid) == 0 &&
        (SE2->MajorVersion > EFI_SHELL_MAJOR_VER ||
          (SE2->MajorVersion == EFI_SHELL_MAJOR_VER && SE2->MinorVersion >= EFI_SHELL_MINOR_VER)
        )
      ) {
        Status = EFI_SUCCESS;
        break;
      }
    }

    FreePool (HandleBuffer);

    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR, "InitShellApp: Shell environment interfaces not found\n"));
      BS->Exit (ImageHandle, Status, Status, NULL);
    }
  }

  SE = (EFI_SHELL_ENVIRONMENT *) SE2;
  //
  // Disable the page break output mode in default
  //
  DisablePageBreak ();
  
  //
  // Disable the tab key pause output mode in default
  DisableOutputTabPause ();
  
  //
  // Done with init
  //
  return Status;
}

