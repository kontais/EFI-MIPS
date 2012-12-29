/*++

Copyright (c) 2005 - 2007, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  FakeHii.c

Abstract:

  HII support for running under non-Tiano environment
  According to the requirement that Tiano shell.efi should be able to run under EFI1.1, we should embed HII support inside
  shell, because there is no native HII support in EFI1.1.
  We are just to impelment a subset of the complete HII, because not all the features of HII are required in shell

Revision History

--*/

#include "FakeHii.h"

EFI_HANDLE mFakeHiiHandle = NULL;

EFI_STATUS
FakeInitializeHiiDatabase (
  IN     EFI_HANDLE                         ImageHandle,
  IN     EFI_SYSTEM_TABLE                   *SystemTable
  )
/*++

Routine Description:
  Initialize fake HII database

Arguments:

  ImageHandle - The image handle
  SystemTable - The system table

Returns:
  EFI_SUCCESS - Initialized successfully
  others - failed
--*/
{
  EFI_STATUS            Status;
  UINTN                 HandleCount;
  EFI_HANDLE            *HandleBuffer;
  EFI_FAKE_HII_DATA     *HiiData;
  EFI_HANDLE            Handle;
  BOOLEAN               HiiInstalled;

  HiiInstalled = FALSE;
  HiiData = NULL;
  Status = LibLocateHandle (
            ByProtocol,
            &gEfiHiiProtocolGuid,
            NULL,
            &HandleCount,
            &HandleBuffer
            );
  if (!EFI_ERROR (Status)) {
    if (NULL != HandleBuffer) {
      FreePool (HandleBuffer);
      HiiInstalled = TRUE;
    }
  }

  if (!HiiInstalled)   
  {
    HiiData = AllocateZeroPool (sizeof (EFI_FAKE_HII_DATA));
    if (NULL == HiiData) {
      return EFI_OUT_OF_RESOURCES;
    }
  
    HiiData->Signature                = EFI_FAKE_HII_DATA_SIGNATURE;
    HiiData->DatabaseHead             = NULL;
    HiiData->Hii.NewPack              = FakeHiiNewPack;
    HiiData->Hii.RemovePack           = FakeHiiRemovePack;
    HiiData->Hii.FindHandles          = FakeHiiFindHandles;
    HiiData->Hii.GetPrimaryLanguages  = FakeHiiGetPrimaryLanguages;
    HiiData->Hii.GetString            = FakeHiiGetString;
    HiiData->Hii.GetForms             = FakeHiiGetForms;
  
    Handle = NULL;
    Status = BS->InstallProtocolInterface (
                  &mFakeHiiHandle,
                  &gEfiHiiProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &HiiData->Hii
                  );
  
    if (EFI_ERROR (Status)) {
      FreePool (HiiData);
      return Status;
    } 
  }
  
  return Status;
}


EFI_STATUS
FakeUninstallHiiDatabase (
  VOID
  )
{                                    
  EFI_STATUS              Status;
  EFI_HII_PROTOCOL        *FakeHii;
  EFI_FAKE_HII_DATA       *HiiData;
  
  if (mFakeHiiHandle != NULL) {
    Status = BS->HandleProtocol (
                  mFakeHiiHandle,
                  &gEfiHiiProtocolGuid,
                  &FakeHii
                  );
    Status = BS->UninstallProtocolInterface (
                  mFakeHiiHandle,
                  &gEfiHiiProtocolGuid,
                  FakeHii
                  );
    HiiData = EFI_FAKE_HII_DATA_FROM_THIS (FakeHii);
    mFakeHiiHandle = NULL;
    FreePool (HiiData);
  }
  
  return EFI_SUCCESS;
}

EFI_STATUS
FakeGetPackSize (
  IN  VOID                *Pack,
  OUT UINTN               *PackSize,
  OUT UINT32              *NumberOfTokens
  )
/*++

Routine Description:
  Determines the passed in Pack's size and returns the value.
  
Arguments:

  Pack           - The package
  PackSize       - The package size
  NumberOfTokens - The tokens number

Returns: 

  EFI_SUCCESS   - Success
  EFI_NOT_FOUND - Not found

--*/
{
  EFI_HII_STRING_PACK *StringPack;
  UINT16              Type;
  UINT32              Length;

  *PackSize = 0;

  Type      = EFI_HII_IFR;
  if (!CompareMem (&((EFI_HII_PACK_HEADER *) Pack)->Type, &Type, sizeof (UINT16))) {
    //
    // The header contains the full IFR length
    //
    CopyMem (&Length, &((EFI_HII_PACK_HEADER *) Pack)->Length, sizeof (Length));
    *PackSize = (UINTN) Length;
    return EFI_SUCCESS;
  }

  Type = EFI_HII_STRING;
  if (!CompareMem (&((EFI_HII_PACK_HEADER *) Pack)->Type, &Type, sizeof (UINT16))) {
    //
    // The header contains the STRING package length
    // The assumption is that the strings for all languages
    // are a contiguous block of data and there is a series of
    // these package instances which will terminate with a NULL package
    // instance.
    //
    StringPack = (EFI_HII_STRING_PACK *) Pack;

    //
    // There may be multiple instances packed together of strings
    // so we must walk the self describing structures until we encounter
    // the NULL structure to determine the full size.
    //
    CopyMem (&Length, &StringPack->Header.Length, sizeof (Length));
    if (NumberOfTokens != NULL) {
      CopyMem (NumberOfTokens, &StringPack->NumStringPointers, sizeof (UINT32));
    }

    while (Length != 0) {
      *PackSize   = *PackSize + Length;
      StringPack  = (EFI_HII_STRING_PACK *) ((CHAR8 *) StringPack + Length);
      CopyMem (&Length, &StringPack->Header.Length, sizeof (Length));
    }
    //
    // Encountered a length of 0, so let's add the space for the NULL terminator
    // pack's length and call it done.
    //
    *PackSize = *PackSize + sizeof (EFI_HII_STRING_PACK);
    return EFI_SUCCESS;
  }
  //
  // We only determine the size of the non-global Package types.
  // If neither IFR or STRING data were found, return an error
  //
  return EFI_NOT_FOUND;
}

EFI_STATUS
FakeValidatePack (
  IN   EFI_HII_PROTOCOL                 *This,
  IN   EFI_FAKE_HII_PACKAGE_INSTANCE    *PackageInstance,
  OUT  EFI_FAKE_HII_PACKAGE_INSTANCE    **StringPackageInstance,
  OUT  UINT32                           *TotalStringCount
  )
/*++

Routine Description:
  Verifies that the package instance is using the correct handle for string operations.
  
Arguments:

  This                  - This protocol
  PackageInstance       - The package instance
  StringPackageInstance - The string pacakge instance
  TotalStringCount      - The total string count

Returns: 

  EFI_INVALID_PARAMETER - Invalid parameter
  EFI_SUCCESS           - Success

--*/
{
  EFI_FAKE_HII_DATA             *HiiData;
  EFI_FAKE_HII_HANDLE_DATABASE  *HandleDatabase;
  EFI_FAKE_HII_PACKAGE_INSTANCE *HandlePackageInstance;
  UINT8                         *RawData;
  EFI_GUID                      Guid;
  EFI_HII_IFR_PACK              *FormPack;
  UINTN                         Index;

  if (This == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  HiiData         = EFI_FAKE_HII_DATA_FROM_THIS (This);

  HandleDatabase  = HiiData->DatabaseHead;
  ZeroMem (&Guid, sizeof (EFI_GUID));

  *StringPackageInstance = PackageInstance;

  //
  // Based on if there is IFR data in this package instance, determine
  // what the location is of the beginning of the string data.
  //
  if (PackageInstance->IfrSize > 0) {
    FormPack = (EFI_HII_IFR_PACK *) ((CHAR8 *) (&PackageInstance->IfrData) + sizeof (EFI_HII_PACK_HEADER));
  } else {
    //
    // If there is no IFR data assume the caller knows what they are doing.
    //
    return EFI_SUCCESS;
  }

  RawData = (UINT8 *) FormPack;

  for (Index = 0; RawData[Index] != EFI_IFR_END_FORM_SET_OP;) {
    if (RawData[Index] == EFI_IFR_FORM_SET_OP) {
      //
      // Cache the guid for this formset
      //
      CopyMem (&Guid, &((EFI_IFR_FORM_SET *) &RawData[Index])->Guid, sizeof (EFI_GUID));
      break;
    }

    Index = RawData[Index + 1] + Index;
  }
  //
  // If there is no string package, and the PackageInstance->IfrPack.Guid and PackageInstance->Guid are
  // different, we should return the correct handle for the caller to use for strings.
  //
  if ((PackageInstance->StringSize == 0) && (CompareGuid (&Guid, &PackageInstance->Guid) != 0)) {
    //
    // Search the database for a handle that matches the PackageInstance->Guid
    //
    for (; HandleDatabase != NULL; HandleDatabase = HandleDatabase->NextHandleDatabase) {
      //
      // Get Ifrdata and extract the Guid for it
      //
      HandlePackageInstance = HandleDatabase->Buffer;

      ASSERT (HandlePackageInstance->IfrSize != 0);

      FormPack  = (EFI_HII_IFR_PACK *) ((CHAR8 *) (&HandlePackageInstance->IfrData) + sizeof (EFI_HII_PACK_HEADER));
      RawData   = (UINT8 *) FormPack;

      for (Index = 0; RawData[Index] != EFI_IFR_END_FORM_SET_OP;) {
        if (RawData[Index] == EFI_IFR_FORM_SET_OP) {
          //
          // Cache the guid for this formset
          //
          CopyMem (&Guid, &((EFI_IFR_FORM_SET *) &RawData[Index])->Guid, sizeof (EFI_GUID));
          break;
        }

        Index = RawData[Index + 1] + Index;
      }
      //
      // If the Guid from the new handle matches the original Guid referenced in the original package data
      // return the appropriate package instance data to use.
      //
      if (CompareGuid (&Guid, &PackageInstance->Guid) == 0) {
        if (TotalStringCount != NULL) {
          *TotalStringCount = HandleDatabase->NumberOfTokens;
        }

        *StringPackageInstance = HandlePackageInstance;
      }
    }
    //
    // end for
    //
  } else {
    return EFI_SUCCESS;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
FakeHiiNewPack (
  IN  EFI_HII_PROTOCOL      *This,
  IN  EFI_HII_PACKAGES      *Packages,
  OUT EFI_HII_HANDLE        *Handle
  )
/*++

Routine Description:
  Extracts the various packs from a package list.
  
Arguments:

  This     - This protocol
  Packages - The padckages
  Handle   - The handle

Returns: 

  EFI_INVALID_PARAMETER - Invalid parameter
  EFI_SUCCESS           - Success

--*/
{
  EFI_FAKE_HII_PACKAGE_INSTANCE *PackageInstance;
  EFI_FAKE_HII_DATA             *HiiData;
  EFI_FAKE_HII_HANDLE_DATABASE  *HandleDatabase;
  EFI_FAKE_HII_HANDLE_DATABASE  *Database;
  EFI_FAKE_HII_GLOBAL_DATA      *GlobalData;
  EFI_HII_PACK_HEADER           *PackageHeader;
  EFI_HII_IFR_PACK              *IfrPack;
  EFI_HII_STRING_PACK           *StringPack;
  EFI_HII_HANDLE_PACK           *Handlepack;
  EFI_HII_FONT_PACK             *FontPack;
  EFI_HII_KEYBOARD_PACK         *KeyboardPack;

  EFI_STATUS                    Status;
  UINTN                         IfrSize;
  UINTN                         StringSize;
  UINTN                         TotalStringSize;
  UINTN                         InstanceSize;
  UINTN                         Count;
  UINTN                         Index;
  UINT16                        Member;
  EFI_GUID                      Guid;
  EFI_FORM_SET_STUB             FormSetStub;
  UINT8                         *Location;
  UINT16                        Unicode;
  UINT16                        NumWideGlyphs;
  UINT16                        NumNarrowGlyphs;
  UINT32                        NumberOfTokens;
  UINT32                        TotalTokenNumber;
  UINT8                         *Local;
  EFI_NARROW_GLYPH              *NarrowGlyph;
  EFI_WIDE_GLYPH                *WideGlyph;

  if (Packages->NumberOfPackages == 0 || This == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  HiiData           = EFI_FAKE_HII_DATA_FROM_THIS (This);

  GlobalData        = HiiData->GlobalData;

  Database          = HiiData->DatabaseHead;

  PackageInstance   = NULL;
  IfrPack           = NULL;
  StringPack        = NULL;
  InstanceSize      = 0;
  IfrSize           = 0;
  StringSize        = 0;
  TotalStringSize   = 0;
  NumberOfTokens    = 0;
  TotalTokenNumber  = 0;

  //
  // Search through the passed in Packages for the IfrPack and any StringPack.
  //
  for (Index = 0; Index < Packages->NumberOfPackages; Index++) {

    PackageHeader = *(EFI_HII_PACK_HEADER **) (((UINT8 *) Packages) + sizeof (EFI_HII_PACKAGES) + Index * sizeof (VOID *));

    switch (PackageHeader->Type) {
    case EFI_HII_IFR:
      //
      // There shoule be only one Ifr package.
      //
      ASSERT (IfrPack == NULL);
      IfrPack = (EFI_HII_IFR_PACK *) PackageHeader;
      break;

    case EFI_HII_STRING:
      StringPack = (EFI_HII_STRING_PACK *) PackageHeader;
      //
      // Sending me a String Package. Get its size.
      //
      Status = FakeGetPackSize ((VOID *) StringPack, &StringSize, &NumberOfTokens);
      ASSERT (!EFI_ERROR (Status));

      //
      // The size which FakeGetPackSize() returns include the null terminator. So if multiple
      // string packages are passed in, merge all these packages, and only pad one null terminator.
      //
      if (TotalStringSize > 0) {
        TotalStringSize -= sizeof (EFI_HII_STRING_PACK);
      }

      TotalStringSize += StringSize;
      TotalTokenNumber += NumberOfTokens;
      break;
    }
  }
  //
  // If sending a StringPack without an IfrPack, you must include a GuidId
  //
  if ((StringPack != NULL) && (IfrPack == NULL)) {
    if (Packages->GuidId == NULL) {
      return EFI_INVALID_PARAMETER;
    }
  }
  //
  // If passing in an IfrPack and a GuidId is provided, ensure they are the same value.
  //
  if ((IfrPack != NULL) && (Packages->GuidId != NULL)) {
    Location  = ((UINT8 *) IfrPack);
    Location  = (UINT8 *) (((UINTN) Location) + sizeof (EFI_HII_PACK_HEADER));

    //
    // Advance to the Form Set Op-code
    //
    for (Count = 0; ((EFI_IFR_OP_HEADER *) &Location[Count])->OpCode != EFI_IFR_FORM_SET_OP;) {
      Count = Count + ((EFI_IFR_OP_HEADER *) &Location[Count])->Length;
    }
    //
    // Copy to local variable
    //
    CopyMem (&Guid, &((EFI_IFR_FORM_SET *) &Location[Count])->Guid, sizeof (EFI_GUID));

    //
    // Check to see if IfrPack->Guid != GuidId
    //
    if (CompareGuid (&Guid, Packages->GuidId) != 0) {
      //
      // If a string package is present, the GUIDs should have agreed.  Return an error
      //
      if (StringPack != NULL) {
        return EFI_INVALID_PARAMETER;
      }
    }
  }
  //
  // If someone is passing in a string only, create a dummy IfrPack with a Guid
  // to enable future searching of this data.
  //
  if ((IfrPack == NULL) && (StringPack != NULL)) {
    ZeroMem (&FormSetStub, sizeof (FormSetStub));

    FormSetStub.Header.Type           = EFI_HII_IFR;
    FormSetStub.Header.Length         = sizeof (EFI_FORM_SET_STUB);

    FormSetStub.FormSet.Header.OpCode = EFI_IFR_FORM_SET_OP;
    FormSetStub.FormSet.Header.Length = (UINT8) sizeof (EFI_IFR_FORM_SET);
    //
    // Dummy string
    //
    FormSetStub.FormSet.FormSetTitle = 0x02;
    CopyMem (&FormSetStub.FormSet.Guid, Packages->GuidId, sizeof (EFI_GUID));

    FormSetStub.EndFormSet.Header.OpCode  = EFI_IFR_END_FORM_SET_OP;
    FormSetStub.EndFormSet.Header.Length  = (UINT8) sizeof (EFI_IFR_END_FORM_SET);
    IfrPack = (EFI_HII_IFR_PACK *) &FormSetStub;
  }

  if (IfrPack != NULL) {
    //
    // Sending me an IFR Package. Get its size.
    //
    Status = FakeGetPackSize ((VOID *) IfrPack, &IfrSize, NULL);
    ASSERT (!EFI_ERROR (Status));
  }
  //
  // Prepare the internal package instace buffer to store package data.
  //
  InstanceSize = IfrSize + TotalStringSize;

  if (InstanceSize != 0) {
    PackageInstance = AllocateZeroPool (InstanceSize + sizeof (EFI_FAKE_HII_PACKAGE_INSTANCE));

    ASSERT (PackageInstance);

    //
    // If there is no DatabaseHead allocated - allocate one
    //
    if (HiiData->DatabaseHead == NULL) {
      HiiData->DatabaseHead = AllocateZeroPool (sizeof (EFI_FAKE_HII_HANDLE_DATABASE));
      ASSERT (HiiData->DatabaseHead);
    }
    //
    // If the head is being used (Handle is non-zero), allocate next Database and
    // add it to the linked-list
    //
    if (HiiData->DatabaseHead->Handle != 0) {
      HandleDatabase = AllocateZeroPool (sizeof (EFI_FAKE_HII_HANDLE_DATABASE));

      ASSERT (HandleDatabase);

      for (; Database->NextHandleDatabase != NULL; Database = Database->NextHandleDatabase)
        ;

      //
      // We are sitting on the Database entry which contains the null Next pointer.  Fix it.
      //
      Database->NextHandleDatabase = HandleDatabase;

    }

    Database = HiiData->DatabaseHead;

    //
    // Initialize this instance data
    //
    for (*Handle = 1; Database->NextHandleDatabase != NULL; Database = Database->NextHandleDatabase) {
      //
      // Since the first Database instance will have a passed back handle of 1, we will continue
      // down the linked list of entries until we encounter the end of the linked list.  Each time
      // we go down one level deeper, increment the handle value that will be passed back.
      //
      if (Database->Handle >= *Handle) {
        *Handle = Database->Handle + 1;
      }
    }

    PackageInstance->Handle     = *Handle;
    PackageInstance->IfrSize    = IfrSize;
    PackageInstance->StringSize = TotalStringSize;
    if (Packages->GuidId != NULL) {
      CopyMem (&PackageInstance->Guid, Packages->GuidId, sizeof (EFI_GUID));
    }

    Database->Buffer              = PackageInstance;
    Database->Handle              = PackageInstance->Handle;
    Database->NumberOfTokens      = TotalTokenNumber;
    Database->NextHandleDatabase  = NULL;
  }
  //
  // Copy the Ifr package data into package instance.
  //
  if (IfrSize > 0) {
    CopyMem (&PackageInstance->IfrData, IfrPack, IfrSize);
  }
  //
  // Main loop to store package data into HII database.
  //
  StringSize      = 0;
  TotalStringSize = 0;

  for (Index = 0; Index < Packages->NumberOfPackages; Index++) {

    PackageHeader = *(EFI_HII_PACK_HEADER **) (((UINT8 *) Packages) + sizeof (EFI_HII_PACKAGES) + Index * sizeof (VOID *));

    switch (PackageHeader->Type) {
    case EFI_HII_STRING:
      StringPack = (EFI_HII_STRING_PACK *) PackageHeader;
      //
      // The size which FakeGetPackSize() returns include the null terminator. So if multiple
      // string packages are passed in, merge all these packages, and only pad one null terminator.
      //
      if (TotalStringSize > 0) {
        TotalStringSize -= sizeof (EFI_HII_STRING_PACK);
      }

      FakeGetPackSize ((VOID *) StringPack, &StringSize, &NumberOfTokens);
      CopyMem ((CHAR8 *) (&PackageInstance->IfrData) + IfrSize + TotalStringSize, StringPack, StringSize);

      TotalStringSize += StringSize;
      break;

    case EFI_HII_HANDLES:
      Handlepack                  = (EFI_HII_HANDLE_PACK *) PackageHeader;
      CopyMem (&PackageInstance->HandlePack, Handlepack, sizeof(EFI_HII_HANDLE_PACK));
      break;

    case EFI_HII_FONT:
      FontPack = (EFI_HII_FONT_PACK *) PackageHeader;
      //
      // Add whatever narrow glyphs were passed to us if undefined
      //
      CopyMem (&NumNarrowGlyphs, &FontPack->NumberOfNarrowGlyphs, sizeof (UINT16));
      for (Count = 0; Count <= NumNarrowGlyphs; Count++) {
        Local       = (UINT8 *) (&FontPack->NumberOfWideGlyphs + sizeof (UINT8)) + (sizeof (EFI_NARROW_GLYPH)) * Count;
        NarrowGlyph = (EFI_NARROW_GLYPH *) Local;
        CopyMem (&Member, &NarrowGlyph->UnicodeWeight, sizeof (UINT16));
        //
        // If the glyph is already defined, do not overwrite it.  It is what it is.
        //
        CopyMem (&Unicode, &GlobalData->NarrowGlyphs[Member].UnicodeWeight, sizeof (UINT16));
        if (Unicode == 0) {
          CopyMem (&GlobalData->NarrowGlyphs[Member], Local, sizeof (EFI_NARROW_GLYPH));
        }
      }
      //
      // Add whatever wide glyphs were passed to us if undefined
      //
      CopyMem (&NumWideGlyphs, &FontPack->NumberOfWideGlyphs, sizeof (UINT16));
      for (Count = 0; Count <= NumWideGlyphs; Count++) {
        Local = (UINT8 *) (&FontPack->NumberOfWideGlyphs + sizeof (UINT8)) +
          (sizeof (EFI_NARROW_GLYPH)) *
          NumNarrowGlyphs;
        WideGlyph = (EFI_WIDE_GLYPH *) Local;
        CopyMem (
          &Member,
          (UINTN *) (Local + sizeof (EFI_WIDE_GLYPH) * Count),
          sizeof (UINT16)
          );
        //
        // If the glyph is already defined, do not overwrite it.  It is what it is.
        //
        CopyMem (&Unicode, &GlobalData->WideGlyphs[Member].UnicodeWeight, sizeof (UINT16));
        if (Unicode == 0) {
          Local = (UINT8 *) (&FontPack->NumberOfWideGlyphs + sizeof (UINT8)) +
            (sizeof (EFI_NARROW_GLYPH)) *
            NumNarrowGlyphs;
          WideGlyph = (EFI_WIDE_GLYPH *) Local;
          CopyMem (
            &GlobalData->WideGlyphs[Member],
            (UINTN *) (Local + sizeof (EFI_WIDE_GLYPH) * Count),
            sizeof (EFI_WIDE_GLYPH)
            );
        }
      }
      break;

    case EFI_HII_KEYBOARD:
      KeyboardPack = (EFI_HII_KEYBOARD_PACK *) PackageHeader;
      //
      // Sending me a Keyboard Package
      //
      if (KeyboardPack->DescriptorCount > 105) {
        return EFI_INVALID_PARAMETER;
      }
      //
      // If someone updates the Descriptors with a count of 0, blow aware the overrides.
      //
      if (KeyboardPack->DescriptorCount == 0) {
        ZeroMem (GlobalData->OverrideKeyboardLayout, sizeof (EFI_KEY_DESCRIPTOR) * 106);
      }

      if (KeyboardPack->DescriptorCount < 106 && KeyboardPack->DescriptorCount > 0) {
        //
        // If SystemKeyboard was updated already, then steer changes to the override database
        //
        if (GlobalData->SystemKeyboardUpdate) {
          ZeroMem (GlobalData->OverrideKeyboardLayout, sizeof (EFI_KEY_DESCRIPTOR) * 106);
          for (Count = 0; Count < KeyboardPack->DescriptorCount; Count++) {
            CopyMem (&Member, &KeyboardPack->Descriptor[Count].Key, sizeof (UINT16));
            CopyMem (
              &GlobalData->OverrideKeyboardLayout[Member],
              &KeyboardPack->Descriptor[Count],
              sizeof (EFI_KEY_DESCRIPTOR)
              );
          }
        } else {
          //
          // SystemKeyboard was never updated, so this is likely the keyboard driver setting the System database.
          //
          ZeroMem (GlobalData->SystemKeyboardLayout, sizeof (EFI_KEY_DESCRIPTOR) * 106);
          for (Count = 0; Count < KeyboardPack->DescriptorCount; Count++) {
            CopyMem (&Member, &KeyboardPack->Descriptor->Key, sizeof (UINT16));
            CopyMem (
              &GlobalData->SystemKeyboardLayout[Member],
              &KeyboardPack->Descriptor[Count],
              sizeof (EFI_KEY_DESCRIPTOR)
              );
          }
          //
          // Just updated the system keyboard database, reflect that in the global flag.
          //
          GlobalData->SystemKeyboardUpdate = TRUE;
        }
      }
      break;

    default:
      break;
    }
  }

  return EFI_SUCCESS;

}

EFI_STATUS
EFIAPI
FakeHiiRemovePack (
  IN EFI_HII_PROTOCOL                   *This,
  IN EFI_HII_HANDLE                     Handle
  )
/*++

Routine Description:
  Removes the various packs from a Handle
  
Arguments:

  This   - This protocol
  Handle - The handle

Returns: 

  EFI_INVALID_PARAMETER - Invalid parameter
  EFI_SUCCESS           - Success

--*/
{
  EFI_FAKE_HII_PACKAGE_INSTANCE *PackageInstance;
  EFI_FAKE_HII_DATA             *HiiData;
  EFI_FAKE_HII_HANDLE_DATABASE  *HandleDatabase;
  EFI_FAKE_HII_HANDLE_DATABASE  *PreviousHandleDatabase;
  UINTN                         Count;

  if (This == NULL || Handle == 0) {
    return EFI_INVALID_PARAMETER;
  }

  HiiData         = EFI_FAKE_HII_DATA_FROM_THIS (This);

  HandleDatabase  = HiiData->DatabaseHead;
  PackageInstance = NULL;

  //
  // Initialize the Previous with the Head of the Database
  //
  PreviousHandleDatabase = HandleDatabase;

  for (Count = 0; HandleDatabase != NULL; HandleDatabase = HandleDatabase->NextHandleDatabase) {
    //
    // Match the numeric value with the database entry - if matched,
    // free the package instance and apply fix-up to database linked list
    //
    if (Handle == HandleDatabase->Handle) {
      PackageInstance = HandleDatabase->Buffer;

      //
      // Free the Package Instance
      //
      FreePool (PackageInstance);

      //
      // If this was the only Handle in the database
      //
      if (HiiData->DatabaseHead == HandleDatabase) {
        HiiData->DatabaseHead = NULL;
      }
      //
      // Make the parent->Next point to the current->Next
      //
      PreviousHandleDatabase->NextHandleDatabase = HandleDatabase->NextHandleDatabase;
      FreePool (HandleDatabase);
      return EFI_SUCCESS;
    }
    //
    // If this was not the HandleDatabase entry we were looking for, cache it just in case the next one is
    //
    PreviousHandleDatabase = HandleDatabase;
  }
  //
  // No handle was found - error condition
  //
  if (PackageInstance == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
FakeHiiFindHandles (
  IN EFI_HII_PROTOCOL                     *This,
  IN OUT UINT16                           *HandleBufferLength,
  OUT EFI_HII_HANDLE                      *Handle
  )
/*++

Routine Description:
  Determines the handles that are currently active in the database.
  
Arguments:

  This               - This protocol
  HandleBufferLength - Buffer length
  Handle             - The handle

Returns: 
  EFI_INVALID_PARAMETER - Invalid parameter
  EFI_NOT_FOUND         - Not found
  EFI_SUCCESS           - Success
  EFI_BUFFER_TOO_SMALL  - Buffer too small

--*/
{
  EFI_FAKE_HII_GLOBAL_DATA      *GlobalData;
  EFI_FAKE_HII_HANDLE_DATABASE  *Database;
  EFI_FAKE_HII_DATA             *HiiData;
  UINTN                         HandleCount;

  if (This == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  HiiData     = EFI_FAKE_HII_DATA_FROM_THIS (This);

  GlobalData  = HiiData->GlobalData;

  Database    = HiiData->DatabaseHead;

  if (Database == NULL) {
    *HandleBufferLength = 0;
    return EFI_NOT_FOUND;
  }

  for (HandleCount = 0; Database != NULL; HandleCount++) {
    Database = Database->NextHandleDatabase;
  }
  //
  // Is there a sufficient buffer for the data being passed back?
  //
  if (*HandleBufferLength >= (sizeof (EFI_HII_HANDLE) * HandleCount)) {
    Database = HiiData->DatabaseHead;

    //
    // Copy the Head information
    //
    if (Database->Handle != 0) {
      CopyMem (&Handle[0], &Database->Handle, sizeof (EFI_HII_HANDLE));
      Database = Database->NextHandleDatabase;
    }
    //
    // Copy more data if appropriate
    //
    for (HandleCount = 1; Database != NULL; HandleCount++) {
      CopyMem (&Handle[HandleCount], &Database->Handle, sizeof (EFI_HII_HANDLE));
      Database = Database->NextHandleDatabase;
    }

    *HandleBufferLength = (UINT16) (sizeof (EFI_HII_HANDLE) * HandleCount);
    return EFI_SUCCESS;
  } else {
    //
    // Insufficient buffer length
    //
    *HandleBufferLength = (UINT16) (sizeof (EFI_HII_HANDLE) * HandleCount);
    return EFI_BUFFER_TOO_SMALL;
  }
}

EFI_STATUS
FakeCompareLanguage (
  IN  CHAR16                *LanguageStringLocation,
  IN  CHAR16                *Language
  )
{
  UINT8   *Local;
  UINTN   Index;
  CHAR16  *InputString;
  CHAR16  *OriginalInputString;

  //
  // Allocate a temporary buffer for InputString
  //
  InputString = AllocateZeroPool (0x100);

  ASSERT (InputString);

  OriginalInputString = InputString;

  Local               = (UINT8 *) LanguageStringLocation;

  //
  // Determine the size of this packed string safely (e.g. access by byte), post-increment
  // to include the null-terminator
  //
  for (Index = 0; Local[Index] != 0; Index += 2)
    ;
  //
  // This is a packed structure that this location comes from, so let's make sure
  // the value is aligned by copying it to a local variable and working on it.
  //
  CopyMem (InputString, LanguageStringLocation, Index);

  for (Index = 0; Index < 3; Index++) {
    InputString[Index]  = (CHAR16) (InputString[Index] | 0x20);
    Language[Index]     = (CHAR16) (Language[Index] | 0x20);
  }
  //
  // If the Language is the same return success
  //
  if (CompareMem (LanguageStringLocation, Language, 6) == 0) {
    FreePool (InputString);
    return EFI_SUCCESS;
  }
  //
  // Skip the first three letters that comprised the primary language,
  // see if what is being compared against is a secondary language
  //
  InputString = InputString + 3;

  //
  // If the Language is not the same as the Primary language, see if there are any
  // secondary languages, and if there are see if we have a match.  If not, return an error.
  //
  for (Index = 0; InputString[Index] != 0; Index = Index + 3) {
    //
    // Getting in here means we have a secondary language
    //
    if (CompareMem (&InputString[Index], Language, 6) == 0) {
      FreePool (InputString);
      return EFI_SUCCESS;
    }
  }
  //
  // If nothing was found, return the error
  //
  FreePool (OriginalInputString);
  return EFI_NOT_FOUND;

}

VOID
FakeAsciiToUnicode (
  IN UINT8                                        *Lang,
  IN UINT16                                       *Language
  )
{
  UINT8 Count;

  for (Count = 0; Count < 3; Count++) {
    Language[Count] = (CHAR16) Lang[Count];
  }
}

EFI_STATUS
EFIAPI
FakeHiiGetString (
  IN EFI_HII_PROTOCOL                   *This,
  IN EFI_HII_HANDLE                     Handle,
  IN STRING_REF                         Token,
  IN BOOLEAN                            Raw,
  IN CHAR16                             *LanguageString,
  IN OUT UINTN                          *BufferLength,
  OUT EFI_STRING                        StringBuffer
  )
/*++

Routine Description:
  
    This function extracts a string from a package already registered with the EFI HII database.

Arguments:

  This           - This protocol
  Handle         - Handle
  Token          - The Token
  Raw            - The raw data
  LanguageString - The language string
  BufferLength   - The buffer length
  StringBuffer   - The string buffer

Returns: 

  EFI_INVALID_PARAMETER - Invalid parameter
  EFI_SUCCESS           - Success
  EFI_BUFFER_TOO_SMALL  - Buffer too small

--*/
{
  INTN                          Count;
  EFI_FAKE_HII_PACKAGE_INSTANCE *PackageInstance;
  EFI_FAKE_HII_PACKAGE_INSTANCE *StringPackageInstance;
  EFI_FAKE_HII_DATA             *HiiData;
  EFI_FAKE_HII_HANDLE_DATABASE  *HandleDatabase;
  EFI_HII_STRING_PACK           *StringPack;
  RELOFST                       *StringPointer;
  EFI_STATUS                    Status;
  EFI_STRING                    OriginalStringBuffer;
  UINTN                         DataSize;
  CHAR8                         Lang[3];
  CHAR16                        Language[3];
  UINT32                        Length;
  INTN                          Index;
  RELOFST                       Offset;
  UINT16                        *Local;
  UINT16                        Zero;
  UINT16                        Narrow;
  UINT16                        Wide;
  UINT16                        NoBreak;
  BOOLEAN                       LangFound;

  if (This == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  LangFound       = TRUE;

  DataSize        = sizeof (Lang);

  HiiData         = EFI_FAKE_HII_DATA_FROM_THIS (This);

  PackageInstance = NULL;
  Zero            = 0;
  Narrow          = NARROW_CHAR;
  Wide            = WIDE_CHAR;
  NoBreak         = NON_BREAKING_CHAR;

  //
  // Check numeric value against the head of the database
  //
  for (HandleDatabase = HiiData->DatabaseHead;
       HandleDatabase != NULL;
       HandleDatabase = HandleDatabase->NextHandleDatabase
      ) {
    //
    // Match the numeric value with the database entry - if matched, extract PackageInstance
    //
    if (Handle == HandleDatabase->Handle) {
      PackageInstance = HandleDatabase->Buffer;
      break;
    }
  }
  //
  // No handle was found - error condition
  //
  if (PackageInstance == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = FakeValidatePack (This, PackageInstance, &StringPackageInstance, NULL);

  //
  // If there is no specified language, assume the system default language
  //
  if (LanguageString == NULL) {
    //
    // Get system default language
    //
    Status = RT->GetVariable (
                  L"Lang",
                  &gEfiGlobalVariableGuid,
                  NULL,
                  &DataSize,
                  Lang
                  );

    if (EFI_ERROR (Status)) {
      //
      // If Lang doesn't exist, just use the first language you find
      //
      LangFound = FALSE;
      goto LangNotFound;
    }
    //
    // Convert the ASCII Lang variable to a Unicode Language variable
    //
    FakeAsciiToUnicode (Lang, Language);
  } else {
    //
    // Copy input ISO value to Language variable
    //
    CopyMem (Language, LanguageString, 6);
  }
  //
  // Based on if there is IFR data in this package instance, determine
  // what the location is of the beginning of the string data.
  //
LangNotFound:
  if (StringPackageInstance->IfrSize > 0) {
    StringPack = (EFI_HII_STRING_PACK *) ((CHAR8 *) (&StringPackageInstance->IfrData) + StringPackageInstance->IfrSize);
  } else {
    StringPack = (EFI_HII_STRING_PACK *) (&StringPackageInstance->IfrData);
  }
  //
  // If Token is 0, extract entire string package
  //
  if (Token == 0) {
    OriginalStringBuffer = StringBuffer;
    CopyMem (&Length, &StringPack->Header.Length, sizeof (UINT32));
    //
    // If trying to get the entire string package and have insufficient space.  Return error
    //
    if (Length > *BufferLength) {
      CopyMem (BufferLength, &Length, sizeof (UINT16));
      return EFI_INVALID_PARAMETER;
    }

    for (; Length != 0;) {
      CopyMem (StringBuffer, StringPack, Length);
      StringBuffer  = (CHAR16 *) ((CHAR8 *) (StringBuffer) + Length);
      StringPack    = (EFI_HII_STRING_PACK *) ((CHAR8 *) (StringPack) + Length);
      CopyMem (&Length, &StringPack->Header.Length, sizeof (UINT32));
    }

    StringBuffer = OriginalStringBuffer;
    return EFI_SUCCESS;
  }
  //
  // There may be multiple instances packed together of strings
  // so we must walk the self describing structures until we encounter
  // what we are looking for, and then extract the string we are looking for
  //
  CopyMem (&Length, &StringPack->Header.Length, sizeof (UINT32));
  for (; Length != 0;) {
    //
    // If passed in Language ISO value is in this string pack's language string
    // then we are dealing with the strings we want.
    //
    CopyMem (&Offset, &StringPack->LanguageNameString, sizeof (RELOFST));
    Status = FakeCompareLanguage ((CHAR16 *) ((CHAR8 *) (StringPack) + Offset), Language);

    //
    // If we cannot find the lang variable, we skip this check and use the first language available
    //
    if (LangFound) {
      if (EFI_ERROR (Status)) {
        StringPack = (EFI_HII_STRING_PACK *) ((CHAR8 *) (StringPack) + Length);
        CopyMem (&Length, &StringPack->Header.Length, sizeof (UINT32));
        continue;
      }
    }

    StringPointer = (RELOFST *) (StringPack + 1);

    //
    // We have the right string package - size it, and copy it to the StringBuffer
    //
    if (Token >= StringPack->NumStringPointers) {
      return EFI_INVALID_PARAMETER;
    } else {
      CopyMem (&Offset, &StringPointer[Token], sizeof (RELOFST));
    }
    //
    // Since StringPack is a packed structure, we need to determine the string's
    // size safely, thus byte-wise.  Post-increment the size to include the null-terminator
    //
    Local = (CHAR16 *) ((CHAR8 *) (StringPack) + Offset);
    for (Count = 0; CompareMem (&Local[Count], &Zero, 2); Count++)
      ;
    Count++;

    Count = Count * sizeof (CHAR16);;

    if (*BufferLength >= (UINTN) Count) {
      //
      // Copy the string to the user's buffer
      //
      if (Raw) {
        CopyMem (StringBuffer, (VOID *) ((CHAR8 *) (StringPack) + Offset), Count);
      } else {
        Index = 0;
        Count = -1;
        do {
          Count++;
          for (;
               CompareMem ((CHAR16 *) ((CHAR8 *) (StringPack) + Offset + Count * 2), &Narrow, 2) && CompareMem (
                                                                                                   (CHAR16 *) ((CHAR8 *) (StringPack) + Offset + Count * 2),
               &Wide,
               2
               ) && CompareMem ((CHAR16 *) ((CHAR8 *) (StringPack) + Offset + Count * 2), &NoBreak, 2) && CompareMem (
                                                                                                           (CHAR16 *) ((CHAR8 *) (StringPack) + Offset + Count * 2),
               &Zero,
               2
               );
               Index++, Count++
              ) {
            CopyMem (&StringBuffer[Index], (VOID *) ((CHAR8 *) (StringPack) + Offset + Count * 2), 2);
          }
        } while (CompareMem ((CHAR16 *) ((CHAR8 *) (StringPack) + Offset + Count * 2), &Zero, 2));

        CopyMem (&StringBuffer[Index], (VOID *) ((CHAR8 *) (StringPack) + Offset + Count * 2), 2);

        for (Count = 0; StringBuffer[Count] != 0; Count++)
          ;
        Count++;
        Count = Count * 2;
      }

      *BufferLength = (UINTN) Count;
      return EFI_SUCCESS;
    } else {
      *BufferLength = (UINTN) Count;
      return EFI_BUFFER_TOO_SMALL;
    }

  }

  LangFound = FALSE;
  goto LangNotFound;
}

EFI_STATUS
EFIAPI
FakeHiiGetPrimaryLanguages (
  IN EFI_HII_PROTOCOL                     *This,
  IN EFI_HII_HANDLE                       Handle,
  OUT EFI_STRING                          *LanguageString
  )
/*++

Routine Description:
  
  This function allows a program to determine what the primary languages that are supported on a given handle.

Arguments:

  This           - This protocol
  Handle         - THe handle
  LanguageString - The language string

Returns: 

  EFI_INVALID_PARAMETER - The invalid parameter
  EFI_SUCCESS           - Success

--*/
{
  UINTN                         Count;
  EFI_FAKE_HII_PACKAGE_INSTANCE *PackageInstance;
  EFI_FAKE_HII_PACKAGE_INSTANCE *StringPackageInstance;
  EFI_FAKE_HII_DATA             *HiiData;
  EFI_FAKE_HII_HANDLE_DATABASE  *HandleDatabase;
  EFI_HII_STRING_PACK           *StringPack;
  EFI_HII_STRING_PACK           *Location;
  UINT32                        Length;
  RELOFST                       Token;

  if (This == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  HiiData         = EFI_FAKE_HII_DATA_FROM_THIS (This);

  PackageInstance = NULL;
  //
  // Find matching handle in the handle database. Then get the package instance.
  //
  for (HandleDatabase = HiiData->DatabaseHead;
       HandleDatabase != NULL;
       HandleDatabase = HandleDatabase->NextHandleDatabase
      ) {
    if (Handle == HandleDatabase->Handle) {
      PackageInstance = HandleDatabase->Buffer;
    }
  }
  //
  // No handle was found - error condition
  //
  if (PackageInstance == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  FakeValidatePack (This, PackageInstance, &StringPackageInstance, NULL);

  //
  // Based on if there is IFR data in this package instance, determine
  // what the location is of the beginning of the string data.
  //
  if (StringPackageInstance->IfrSize > 0) {
    StringPack = (EFI_HII_STRING_PACK *) ((CHAR8 *) (&StringPackageInstance->IfrData) + StringPackageInstance->IfrSize);
  } else {
    StringPack = (EFI_HII_STRING_PACK *) (&StringPackageInstance->IfrData);
  }

  Location = StringPack;
  //
  // Remember that the string packages are formed into contiguous blocks of language data.
  //
  CopyMem (&Length, &StringPack->Header.Length, sizeof (UINT32));
  for (Count = 0; Length != 0; Count = Count + 3) {
    StringPack = (EFI_HII_STRING_PACK *) ((CHAR8 *) (StringPack) + Length);
    CopyMem (&Length, &StringPack->Header.Length, sizeof (UINT32));
  }

  *LanguageString = AllocateZeroPool (2 * (Count + 1));

  ASSERT (*LanguageString);

  StringPack = (EFI_HII_STRING_PACK *) Location;

  //
  // Copy the 6 bytes to LanguageString - keep concatenating it.  Shouldn't we just store uint8's since the ISO
  // standard defines the lettering as all US English characters anyway?  Save a few bytes.
  //
  CopyMem (&Length, &StringPack->Header.Length, sizeof (UINT32));
  for (Count = 0; Length != 0; Count = Count + 3) {
    CopyMem (&Token, &StringPack->LanguageNameString, sizeof (RELOFST));
    CopyMem (*LanguageString + Count, (VOID *) ((CHAR8 *) (StringPack) + Token), 6);
    StringPack = (EFI_HII_STRING_PACK *) ((CHAR8 *) (StringPack) + Length);
    CopyMem (&Length, &StringPack->Header.Length, sizeof (UINT32));
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
FakeHiiGetForms (
  IN     EFI_HII_PROTOCOL   *This,
  IN     EFI_HII_HANDLE     Handle,
  IN     EFI_FORM_ID        FormId,
  IN OUT UINTN              *BufferLength,
  OUT    UINT8              *Buffer
  )
/*++

Routine Description:
  
  This function allows a program to extract a form or form package that has 
  previously been registered with the EFI HII database.

Arguments:

  This         - This protocol
  Handle       - The handle
  FormId       - The formId
  BufferLength - The Buffer length
  Buffer       - The Buffer

Returns: 

  EFI_INVALID_PARAMETER - The Invalid parameter
  EFI_NOT_FOUND         - Not found
  EFI_SUCCESS           - Success
  EFI_BUFFER_TOO_SMALL  - Buffer too small

--*/
{
  EFI_FAKE_HII_PACKAGE_INSTANCE *PackageInstance;
  EFI_FAKE_HII_DATA             *HiiData;
  EFI_FAKE_HII_HANDLE_DATABASE  *HandleDatabase;
  EFI_HII_IFR_PACK              *FormPack;
  EFI_IFR_FORM                  *Form;
  EFI_IFR_OP_HEADER             *Location;

  if (This == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  HiiData         = EFI_FAKE_HII_DATA_FROM_THIS (This);

  HandleDatabase  = HiiData->DatabaseHead;

  PackageInstance = NULL;

  //
  // Check numeric value against the head of the database
  //
  for (; HandleDatabase != NULL; HandleDatabase = HandleDatabase->NextHandleDatabase) {
    //
    // Match the numeric value with the database entry - if matched, extract PackageInstance
    //
    if (Handle == HandleDatabase->Handle) {
      PackageInstance = HandleDatabase->Buffer;
      break;
    }
  }
  //
  // No handle was found - error condition
  //
  if (PackageInstance == NULL) {
    return EFI_NOT_FOUND;
  }
  //
  // Based on if there is IFR data in this package instance, determine
  // what the location is of the beginning of the string data.
  //
  if (PackageInstance->IfrSize > 0) {
    FormPack = (EFI_HII_IFR_PACK *) (&PackageInstance->IfrData);
  } else {
    //
    // If there is no IFR data return an error
    //
    return EFI_NOT_FOUND;
  }
  //
  // If requesting the entire Form Package
  //
  if (FormId == 0) {
    //
    // Return an error if buffer is too small
    //
    if (PackageInstance->IfrSize > *BufferLength) {
      *BufferLength = (UINT16) PackageInstance->IfrSize;
      return EFI_BUFFER_TOO_SMALL;
    }

    CopyMem (Buffer, FormPack, PackageInstance->IfrSize);
    return EFI_SUCCESS;
  } else {
    FormPack  = (EFI_HII_IFR_PACK *) ((CHAR8 *) (&PackageInstance->IfrData) + sizeof (EFI_HII_PACK_HEADER));
    Location  = (EFI_IFR_OP_HEADER *) FormPack;

    //
    // Look for the FormId requested
    //
    for (; Location->OpCode != EFI_IFR_END_FORM_SET_OP;) {
      switch (Location->OpCode) {
      case EFI_IFR_FORM_OP:
        Form = (EFI_IFR_FORM *) Location;

        //
        // If we found a Form Op-code and it is of the correct Id, copy it and return
        //
        if (Form->FormId == FormId) {
          if (Location->Length > *BufferLength) {
            *BufferLength = Location->Length;
            return EFI_BUFFER_TOO_SMALL;
          } else {
            for (; Location->OpCode != EFI_IFR_END_FORM_OP;) {
              CopyMem (Buffer, Location, Location->Length);
              Buffer    = Buffer + Location->Length;
              Location  = (EFI_IFR_OP_HEADER *) ((CHAR8 *) (Location) + Location->Length);
            }

            CopyMem (Buffer, Location, Location->Length);
            return EFI_SUCCESS;
          }
        }

      default:
        break;
      }
      //
      // Go to the next Op-Code
      //
      Location = (EFI_IFR_OP_HEADER *) ((CHAR8 *) (Location) + Location->Length);
    }
  }

  return EFI_SUCCESS;
}
