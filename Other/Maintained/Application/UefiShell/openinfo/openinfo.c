/*++
 
Copyright (c) 2005 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  openinfo.c
  
Abstract:

  Shell command "openinfo"



Revision History

--*/

#include "openinfo.h"
#include "EfiShellLib.h"

extern UINT8  STRING_ARRAY_NAME[];

//
// This is the generated header file which includes whatever needs to be exported (strings + IFR)
//
#include STRING_DEFINES_FILE

//
// Global Variables
//
EFI_HII_HANDLE  HiiHandle;
EFI_GUID        EfiOpeninfoGuid = EFI_OPENINFO_GUID;
SHELL_VAR_CHECK_ITEM    OpeninfoCheckList[] = {
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

VOID
ImageTok (
  IN EFI_HANDLE       h,
  IN VOID             *Interface
  )
/*++

Routine Description:

Arguments:

  h         - The EFI handle
  Interface - The interface

Returns:

--*/
{
  EFI_LOADED_IMAGE_PROTOCOL *Image;
  CHAR16                    *ImageName;

  CHAR16                    *Tok;

  Image     = Interface;
  ImageName = LibGetImageName (Image);
  if (ImageName != NULL) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_DPROT_IMAGE_2), HiiHandle);
    Print (ImageName);
    Print (L") ");
    FreePool (ImageName);
  } else {
    Tok = LibDevicePathToStr (Image->FilePath);
    PrintToken (STRING_TOKEN (STR_SHELLENV_DPROT_IMAGE), HiiHandle, Tok);
    FreePool (Tok);
  }
}

EFI_BOOTSHELL_CODE(
  EFI_APPLICATION_ENTRY_POINT(OpeninfoMain)
)
//
//
//
EFI_STATUS
OpeninfoMain (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  );

//
//
//
EFI_STATUS
OpeninfoMain (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  )
/*++

Routine Description:

  Code for external shell "openinfo" command.

Arguments:

  ImageHandle - The image handle
  SystemTable - The system table

Returns:

  EFI_INVALID_PARAMETER - Invalid Parameter
  
--*/
{
  EFI_STATUS                          Status;
  UINTN                               HandleIndex;
  EFI_HANDLE                          DumpHandle;
  PROTOCOL_INFO                       *Prot;
  UINTN                               Index;
  UINTN                               Count;
  VOID                                *Interface;
  EFI_OPEN_PROTOCOL_INFORMATION_ENTRY *OpenBuffer;
  UINTN                               OpenBufferCount;
  EFI_DRIVER_BINDING_PROTOCOL         *DriverBinding;

  SHELL_VAR_CHECK_CODE                RetCode;
  CHAR16                              *Useful;
  UINTN                               ItemValue;
  SHELL_VAR_CHECK_PACKAGE             ChkPck;

  ZeroMem (&ChkPck, sizeof (SHELL_VAR_CHECK_PACKAGE));
  EFI_SHELL_APP_INIT (ImageHandle, SystemTable);
  
  //
  // Enable tab key which can pause the output
  //
  EnableOutputTabPause();

  Status = LibInitializeStrings (&HiiHandle, STRING_ARRAY_NAME, &EfiOpeninfoGuid);

  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (!EFI_PROPER_VERSION (1, 10)) {
    PrintToken (
      STRING_TOKEN (STR_SHELLENV_GNC_COMMAND_NOT_SUPPORT),
      HiiHandle,
      L"openinfo",
      EFI_VERSION_1_10
      );
    Status = EFI_UNSUPPORTED;
    goto Done;
  }

  RetCode = LibCheckVariables (SI, OpeninfoCheckList, &ChkPck, &Useful);
  if (VarCheckOk != RetCode) {
    switch (RetCode) {
    case VarCheckUnknown:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_UNKNOWN_FLAG), HiiHandle, L"openinfo", Useful);
      break;

    case VarCheckDuplicate:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_DUP_FLAG), HiiHandle, L"openinfo", Useful);
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
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiHandle, L"openinfo");
      Status = EFI_INVALID_PARAMETER;
    } else {
      PrintToken (STRING_TOKEN (STR_HELPINFO_OPENINFO_VERBOSEHELP), HiiHandle);
      Status = EFI_SUCCESS;
    }
    goto Done;
  }

  if (ChkPck.ValueCount < 1) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_FEW), HiiHandle, L"openinfo", Useful);
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  if (ChkPck.ValueCount > 1) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiHandle, L"openinfo", Useful);
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  ItemValue = (UINTN) StrToUIntegerBase ((ChkPck.VarList->VarStr), 16, &Status);
  if (EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_ARG), HiiHandle, L"openinfo", ChkPck.VarList->VarStr);
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  //
  // Setup Handle and Protocol Globals
  //
  ShellInitProtocolInfoEnumerator ();

  HandleIndex = ShellHandleNoFromIndex (ItemValue);
  if (HandleIndex == 0) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_OPENINFO_HANDLE_OUT_OF_RANGE), HiiHandle, L"openinfo", ItemValue);
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  DumpHandle = ShellHandleFromIndex (HandleIndex - 1);

  PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_NHANDLE), HiiHandle, HandleIndex, DumpHandle);

  do {
    Status = ShellNextProtocolInfo (&Prot);
    if (Prot == NULL && Status == EFI_SUCCESS) {
      break;
    }

    for (Index = 0; Index < Prot->NoHandles; Index++) {

      if (GetExecutionBreak ()) {
        Status = EFI_ABORTED;
        break;
      }

      if (Prot->Handles[Index] == DumpHandle) {
        //
        // This handle supports this protocol, dump it
        //
        PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_ONE_VAR_HS_NEWLINE), HiiHandle, Prot->IdString);
        Status = BS->OpenProtocolInformation (
                      DumpHandle,
                      &Prot->ProtocolId,
                      &OpenBuffer,
                      &OpenBufferCount
                      );
        if (!EFI_ERROR (Status)) {
          for (Count = 0; Count < OpenBufferCount; Count++) {
            //
            // Print Image info.
            //
            if (ShellHandleToIndex (OpenBuffer[Count].ControllerHandle) == 0) {
              PrintToken (
                STRING_TOKEN (STR_SHELLENV_PROTID_DRV_CTRL_CNT),
                HiiHandle,
                ShellHandleToIndex (OpenBuffer[Count].AgentHandle),
                OpenBuffer[Count].OpenCount
                );
            } else {
              PrintToken (
                STRING_TOKEN (STR_SHELLENV_PROTID_DRV_CTRL_CNT_2),
                HiiHandle,
                ShellHandleToIndex (OpenBuffer[Count].AgentHandle),
                ShellHandleToIndex (OpenBuffer[Count].ControllerHandle),
                OpenBuffer[Count].OpenCount
                );
            }
            //
            // Handle different conditions
            //
            switch (OpenBuffer[Count].Attributes) {
            case EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL:
              PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_ONE_VAR_HS_NO_SPACES), HiiHandle, L"HandProt ");
              break;

            case EFI_OPEN_PROTOCOL_GET_PROTOCOL:
              PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_ONE_VAR_HS_NO_SPACES), HiiHandle, L"GetProt  ");
              break;

            case EFI_OPEN_PROTOCOL_TEST_PROTOCOL:
              PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_ONE_VAR_HS_NO_SPACES), HiiHandle, L"TestProt ");
              break;

            case EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER:
              PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_ONE_VAR_HS_NO_SPACES), HiiHandle, L"Child    ");
              break;

            case EFI_OPEN_PROTOCOL_BY_DRIVER:
              PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_ONE_VAR_HS_NO_SPACES), HiiHandle, L"Driver   ");
              break;

            case EFI_OPEN_PROTOCOL_EXCLUSIVE:
              PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_ONE_VAR_HS_NO_SPACES), HiiHandle, L"Exclusive");
              break;

            case EFI_OPEN_PROTOCOL_BY_DRIVER | EFI_OPEN_PROTOCOL_EXCLUSIVE:
              PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_ONE_VAR_HS_NO_SPACES), HiiHandle, L"DriverEx ");
              break;

            default:
              PrintToken (STRING_TOKEN (STR_SHELLENV_PROTID_ONE_VAR_HS_NO_SPACES), HiiHandle, L"Unknown  ");
              break;
            }
            //
            // Dump image token info
            //
            Status = BS->HandleProtocol (
                          OpenBuffer[Count].AgentHandle,
                          &gEfiDriverBindingProtocolGuid,
                          (VOID **) &DriverBinding
                          );
            if (!EFI_ERROR (Status)) {
              Status = BS->HandleProtocol (
                            DriverBinding->ImageHandle,
                            &gEfiLoadedImageProtocolGuid,
                            (VOID **) &Interface
                            );
              if (!EFI_ERROR (Status)) {
                ImageTok (DriverBinding->ImageHandle, Interface);
              }
            }

            Print (L"\n");
          }

          FreePool (OpenBuffer);
        }
      }
    }
  } while (Prot != NULL);
  
Done:
  ShellCloseProtocolInfoEnumerator ();
  LibCheckVarFreeVarList (&ChkPck);
  LibUnInitializeStrings ();
  return Status;
}

EFI_STATUS
InitializeOpenInfoGetLineHelp (
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
          &EfiOpeninfoGuid,
          STRING_TOKEN (STR_HELPINFO_OPENINFO_LINEHELP),
          Str
          );
}
