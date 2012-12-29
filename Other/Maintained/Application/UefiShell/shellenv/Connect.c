/*++

Copyright (c) 2005 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

    Connect.c
    
Abstract:   

Revision History

--*/

#include "shelle.h"
#include "shellenvguid.h"

SHELL_VAR_CHECK_ITEM    ConnectCheckList[] = {
  {
    L"-r",
    0x01,
    0x02,
    FlagTypeSingle
  },
  {
    L"-c",
    0x02,
    0x01,
    FlagTypeSingle
  },
  {
    L"-b",
    0x04,
    0,
    FlagTypeSingle
  },
  {
    L"-?",
    0x08,
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

SHELL_VAR_CHECK_ITEM          DisconnectCheckList[] = {
  {
    L"-r",
    0x01,
    0,
    FlagTypeSingle
  },
  {
    L"-?",
    0x04,
    0,
    FlagTypeSingle
  },
  {
    L"-b",
    0x04,
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

SHELL_VAR_CHECK_ITEM          ReconnectCheckList[] = {
  {
    L"-r",
    0x01,
    0,
    FlagTypeSingle
  },
  {
    L"-b",
    0x02,
    0,
    FlagTypeSingle
  },
  {
    L"-?",
    0x04,
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
SEnvDisconnectAll (
  VOID
  );

VOID
SEnvConnectAllDriversToAllControllers (
  VOID
  );

VOID
SEnvConnectAllConsoles (
  VOID
  );

EFI_STATUS
EFIAPI
SEnvCmdConnect (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  )
/*++

Routine Description:
  Connect DriverHandle [DeviceHandle]

Arguments:

  ImageHandle  - The image handle
  SystemTable  - The system table

Returns:
  EFI_INVALID_PARAMETER   - Invalid parameter
  EFI_REDIRECTION_NOT_ALLOWED - Can't redirection
  
--*/
{
  EFI_STATUS              Status;
  BOOLEAN                 Recursive;
  BOOLEAN                 Consoles;
  UINTN                   Index;
  UINTN                   HandleNumber;
  UINTN                   NumberOfHandles;
  EFI_HANDLE              HandleBuffer[3];
  BOOLEAN                 GetAllDevices;
  BOOLEAN                 ConnectStatus;
  UINTN                   AllHandleCount;
  EFI_HANDLE              *AllHandleBuffer;
  UINTN                   DriverBindingHandleCount;
  EFI_HANDLE              *DriverBindingHandleBuffer;
  UINTN                   DeviceHandleCount;
  EFI_HANDLE              *DeviceHandleBuffer;
  VOID                    *Instance;
  UINTN                   Driver;
  UINTN                   Device;
  EFI_HANDLE              ContextOverride[2];

  SHELL_VAR_CHECK_CODE    RetCode;
  CHAR16                  *Useful;
  SHELL_VAR_CHECK_PACKAGE ChkPck;
  SHELL_ARG_LIST          *ArgItem;
  UINTN                   ArgValue;

  AllHandleCount            = 0;
  AllHandleBuffer           = NULL;
  DeviceHandleCount         = 0;
  DeviceHandleBuffer        = NULL;
  DriverBindingHandleCount  = 0;
  DriverBindingHandleBuffer = NULL;
  HandleBuffer[0]           = NULL;
  HandleBuffer[1]           = NULL;
  HandleBuffer[2]           = NULL;

  GetAllDevices             = FALSE;
  ConnectStatus             = FALSE;

  Status                    = EFI_SUCCESS;
  ZeroMem (&ChkPck, sizeof (SHELL_VAR_CHECK_PACKAGE));
  EFI_SHELL_APP_INIT (ImageHandle, SystemTable);
   
  //
  // Enable tab key which can pause the output
  //
  EnableOutputTabPause();

  if (!EFI_PROPER_VERSION (1, 10)) {
    PrintToken (
      STRING_TOKEN (STR_SHELLENV_GNC_COMMAND_NOT_SUPPORT),
      HiiEnvHandle,
      L"connect",
      EFI_VERSION_1_10
      );
    Status = EFI_UNSUPPORTED;
    goto ConnectDone;
  }

  RetCode = LibCheckVariables (SI, ConnectCheckList, &ChkPck, &Useful);
  if (VarCheckOk != RetCode) {
    switch (RetCode) {
    case VarCheckDuplicate:

      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_DUP_FLAG), HiiEnvHandle, L"connect", Useful);
      break;

    case VarCheckUnknown:

      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_UNKNOWN_FLAG), HiiEnvHandle, L"connect", Useful);
      break;

    case VarCheckConflict:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_FLAG_CONFLICT), HiiEnvHandle, L"connect", Useful);
      break;

    default:
      break;
    }

    Status = EFI_INVALID_PARAMETER;
    goto ConnectDone;
  }
  //
  //  Out put help.
  //
  if (LibCheckVarGetFlag (&ChkPck, L"-b") != NULL) {
    EnablePageBreak (DEFAULT_INIT_ROW, DEFAULT_AUTO_LF);
  }

  if (LibCheckVarGetFlag (&ChkPck, L"-?") != NULL) {
    if (IS_OLD_SHELL) {
      PrintToken (STRING_TOKEN (STR_NO_HELP), HiiEnvHandle);
    } else if (ChkPck.ValueCount > 0 ||
             ChkPck.FlagCount > 2 ||
             (2 == ChkPck.FlagCount && !LibCheckVarGetFlag (&ChkPck, L"-b"))
            ) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiEnvHandle, L"connect");
      Status = EFI_INVALID_PARAMETER;
    } else {
      PrintToken (STRING_TOKEN (STR_SHELLENV_CONNECT_VERBOSE_HELP), HiiEnvHandle);
      Status = EFI_SUCCESS;
    }
    goto ConnectDone;
  }
  //
  // Setup Handle and Protocol Globals
  //
  SEnvLoadHandleTable ();
  SEnvLoadHandleProtocolInfo (NULL);

  if (ChkPck.ValueCount > 2) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiEnvHandle, L"connect");
    Status = EFI_ABORTED;
    goto ConnectDone;
  }

  Recursive       = FALSE;
  Consoles        = FALSE;
  NumberOfHandles = 0;

  if (LibCheckVarGetFlag (&ChkPck, L"-r") != NULL) {
    if (ChkPck.ValueCount > 0) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_FLAG_CONFLICT), HiiEnvHandle, L"connect", L"-r");
      Status = EFI_INVALID_PARAMETER;
      goto ConnectDone;
    }

    if (SI->RedirArgc != 0) {
      Status = EFI_REDIRECTION_NOT_ALLOWED;
      goto ConnectDone;
    }

    Recursive = TRUE;
  }

  if (LibCheckVarGetFlag (&ChkPck, L"-c") != NULL) {
    Consoles = TRUE;
  }

  if (ChkPck.ValueCount > 0 && (Recursive || Consoles)) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiEnvHandle, L"connect");
    Status = EFI_INVALID_PARAMETER;
    goto ConnectDone;
  }

  ArgItem = ChkPck.VarList;
  while (NULL != ArgItem) {
    ArgValue = (UINTN) StrToUIntegerBase (ArgItem->VarStr, 16, &Status);
    if (EFI_ERROR (Status)) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_ARG), HiiEnvHandle, L"connect", ArgItem->VarStr);
      Status = EFI_INVALID_PARAMETER;
      goto ConnectDone;
    }

    HandleNumber = SEnvHandleNoFromUINT (ArgValue);
    if (HandleNumber == 0) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_HANDLE_NOT_FOUND), HiiEnvHandle, L"connect", ArgItem->VarStr);
      Status = EFI_NOT_FOUND;
      goto ConnectDone;
    } else {
      HandleBuffer[NumberOfHandles++] = SEnvHandles[HandleNumber - 1];
    }

    ArgItem = ArgItem->Next;
  }

  switch (NumberOfHandles) {
  case 0:
    if (Consoles) {
      SEnvConnectAllConsoles ();
      goto ConnectDone;
    }

    if (Recursive) {
      SEnvConnectAllDriversToAllControllers ();
      goto ConnectDone;
    }

    GetAllDevices = TRUE;
    break;

  case 1:
    Status = BS->HandleProtocol (
                  HandleBuffer[0],
                  &gEfiDevicePathProtocolGuid,
                  &Instance
                  );
    if (EFI_ERROR (Status)) {
      DriverBindingHandleCount  = 1;
      DriverBindingHandleBuffer = &HandleBuffer[0];
      GetAllDevices             = TRUE;
    } else {
      DeviceHandleCount   = 1;
      DeviceHandleBuffer  = &HandleBuffer[0];
    }
    break;

  case 2:
    Status = BS->HandleProtocol (
                  HandleBuffer[0],
                  &gEfiDevicePathProtocolGuid,
                  &Instance
                  );
    if (EFI_ERROR (Status)) {
      PrintToken (
        STRING_TOKEN (STR_SHELLENV_CONNECT_HANDLE_ERR),
        HiiEnvHandle,
        L"connect",
        SEnvHandleToNumber (HandleBuffer[0]),
        L"device"
        );
      goto ConnectDone;
    }

    DeviceHandleCount   = 1;
    DeviceHandleBuffer  = &HandleBuffer[0];

    Status = BS->HandleProtocol (
                  HandleBuffer[1],
                  &gEfiDriverBindingProtocolGuid,
                  &Instance
                  );
    if (EFI_ERROR (Status)) {
      PrintToken (
        STRING_TOKEN (STR_SHELLENV_CONNECT_HANDLE_ERR),
        HiiEnvHandle,
        L"connect",
        SEnvHandleToNumber (HandleBuffer[1]),
        L"driver"
        );
      goto ConnectDone;
    }

    DriverBindingHandleCount  = 1;
    DriverBindingHandleBuffer = &HandleBuffer[1];
    break;
  }

  if (GetAllDevices) {
    Status = LibLocateHandle (
              AllHandles,
              NULL,
              NULL,
              &AllHandleCount,
              &AllHandleBuffer
              );
    if (EFI_ERROR (Status)) {
      goto ConnectDone;
    }

    DeviceHandleBuffer = AllocatePool (AllHandleCount * sizeof (EFI_HANDLE));
    if (DeviceHandleBuffer == NULL) {
      goto ConnectDone;
    }

    for (Index = 0; Index < AllHandleCount; Index++) {
      Status = BS->HandleProtocol (
                    AllHandleBuffer[Index],
                    &gEfiDriverBindingProtocolGuid,
                    &Instance
                    );
      if (EFI_ERROR (Status)) {
        DeviceHandleBuffer[DeviceHandleCount++] = AllHandleBuffer[Index];
      }
    }
  }

  for (Device = 0; Device < DeviceHandleCount; Device++) {
    if (DriverBindingHandleCount == 0) {
      Status = BS->ConnectController (
                    DeviceHandleBuffer[Device],
                    NULL,
                    NULL,
                    Recursive
                    );
      if (!EFI_ERROR (Status)) {
        ConnectStatus = TRUE;
        PrintToken (
          STRING_TOKEN (STR_SHELLENV_CONNECT_CONNECT_CONTROLLER),
          HiiEnvHandle,
          SEnvHandleToNumber (DeviceHandleBuffer[Device])
          );
        PrintToken (STRING_TOKEN (STR_SHELLENV_CONNECT_STATUS), HiiEnvHandle, Status);
      }
    } else {

      for (Driver = 0; Driver < DriverBindingHandleCount; Driver++) {
        ContextOverride[0]  = DriverBindingHandleBuffer[Driver];
        ContextOverride[1]  = NULL;
        Status = BS->ConnectController (
                      DeviceHandleBuffer[Device],
                      ContextOverride,
                      NULL,
                      Recursive
                      );
        if (!EFI_ERROR (Status)) {
          ConnectStatus = TRUE;
          PrintToken (
            STRING_TOKEN (STR_SHELLENV_CONNECT_CONNECT_CONTROLLER_2),
            HiiEnvHandle,
            SEnvHandleToNumber (DeviceHandleBuffer[Device]),
            SEnvHandleToNumber (DriverBindingHandleBuffer[Driver])
            );
          PrintToken (STRING_TOKEN (STR_SHELLENV_CONNECT_STATUS), HiiEnvHandle, Status);
        } else if (2 == NumberOfHandles) {
          PrintToken (
            STRING_TOKEN (STR_SHELLENV_CONNECT_FAIL),
            HiiEnvHandle,
            SEnvHandleToNumber (DriverBindingHandleBuffer[Driver]),
            SEnvHandleToNumber (DeviceHandleBuffer[Device])
            );
        }
      }
    }
  }

ConnectDone:
  //
  // Clean up our dynamically allocated data
  //
  if (GetAllDevices) {
    FreePool (AllHandleBuffer);
    FreePool (DeviceHandleBuffer);
  }

  SEnvFreeHandleTable ();
  LibCheckVarFreeVarList (&ChkPck);
  if (ConnectStatus) {
    Status = EFI_SUCCESS;
  }

  return Status;
}

EFI_STATUS
EFIAPI
SEnvCmdDisconnect (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  )
/*++

Routine Description:
  Disconnect DriverHandle [DeviceHandle]

Arguments:

  ImageHandle - The image handle
  SystemTable - The system table

Returns:
  EFI_INVALID_PARAMETER       - Invalid parameter
  EFI_REDIRECTION_NOT_ALLOWED - Can't redirection
  EFI_INVALID_PARAMETER       - Invalid parameter
  EFI_NOT_FOUND               - Not found
  EFI_UNSUPPORTED             - Unsupported
  
--*/
{
  EFI_STATUS                    Status;
  EFI_STATUS                    ReturnStatus;
  UINTN                         HandleNumber;
  EFI_HANDLE                    DriverImageHandle;
  EFI_HANDLE                    DeviceHandle;
  EFI_HANDLE                    ChildHandle;
  EFI_SIMPLE_TEXT_OUT_PROTOCOL  *TextOut;
  EFI_SIMPLE_TEXT_IN_PROTOCOL   *TextIn;
  EFI_DEVICE_PATH_PROTOCOL      *DevPath;
  EFI_HANDLE                    *DeviceHandleBuffer;
  UINTN                         DeviceHandleCount;
  UINTN                         Index;
  CHAR16                        *TempDevHandle;
  SHELL_VAR_CHECK_CODE          RetCode;
  CHAR16                        *Useful;
  SHELL_ARG_LIST                *Item;
  UINTN                         ItemValue;
  SHELL_VAR_CHECK_PACKAGE       ChkPck;

  Status  = EFI_SUCCESS;
  Useful  = NULL;
  Item    = NULL;
  ReturnStatus = EFI_SUCCESS;
  ZeroMem (&ChkPck, sizeof (SHELL_VAR_CHECK_PACKAGE));

  EFI_SHELL_APP_INIT (ImageHandle, SystemTable);

  if (!EFI_PROPER_VERSION (1, 10)) {
    PrintToken (
      STRING_TOKEN (STR_SHELLENV_GNC_COMMAND_NOT_SUPPORT),
      HiiEnvHandle,
      L"disconnect",
      EFI_VERSION_1_10
      );
    ReturnStatus = EFI_UNSUPPORTED;
    goto DisconnectDone;
  }

  RetCode = LibCheckVariables (SI, DisconnectCheckList, &ChkPck, &Useful);
  if (VarCheckOk != RetCode) {
    switch (RetCode) {
    case VarCheckDuplicate:

      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_DUP_FLAG), HiiEnvHandle, L"disconnect", Useful);
      break;

    case VarCheckUnknown:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_UNKNOWN_FLAG), HiiEnvHandle, L"disconnect", Useful);
      break;

    default:
      break;
    }

    ReturnStatus = EFI_INVALID_PARAMETER;
    goto DisconnectDone;
  }
  //
  //  Out put help.
  //
  if (LibCheckVarGetFlag (&ChkPck, L"-b") != NULL) {
    EnablePageBreak (DEFAULT_INIT_ROW, DEFAULT_AUTO_LF);
  }

  if (LibCheckVarGetFlag (&ChkPck, L"-?") != NULL) {
    if (IS_OLD_SHELL) {
      PrintToken (STRING_TOKEN (STR_NO_HELP), HiiEnvHandle);
    } else if (ChkPck.ValueCount > 0 ||
             ChkPck.FlagCount > 2 ||
             (2 == ChkPck.FlagCount && !LibCheckVarGetFlag (&ChkPck, L"-b"))
            ) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiEnvHandle, L"disconnect");
      Status = EFI_INVALID_PARAMETER;
    } else {
      PrintToken (STRING_TOKEN (STR_SHELLENV_DISCONNECT_VERBOSE_HELP), HiiEnvHandle);
      Status = EFI_SUCCESS;
    }

    goto DisconnectDone;
  }

  Status            = EFI_SUCCESS;
  DeviceHandle      = NULL;
  DriverImageHandle = NULL;
  ChildHandle       = NULL;

  //
  // If command line is disconnect all the do that.
  //
  if (LibCheckVarGetFlag (&ChkPck, L"-r") != NULL) {
    if (ChkPck.ValueCount > 0) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_FLAG_CONFLICT), HiiEnvHandle, L"disconnect", L"-r");
      ReturnStatus = EFI_INVALID_PARAMETER;
      goto DisconnectDone;
    }

    if (SI->RedirArgc != 0) {
      ReturnStatus = EFI_REDIRECTION_NOT_ALLOWED;
      goto DisconnectDone;
    }

    SEnvDisconnectAll ();
    SEnvConnectAllConsoles ();
    ReturnStatus = EFI_SUCCESS;
    goto Done;
  }

  if (ChkPck.ValueCount < 1) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_FEW), HiiEnvHandle, L"disconnect");
    ReturnStatus = EFI_INVALID_PARAMETER;
    goto DisconnectDone;
  }

  if (ChkPck.ValueCount > 3) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiEnvHandle, L"disconnect");
    ReturnStatus = EFI_INVALID_PARAMETER;
    goto DisconnectDone;
  }
  //
  // Setup Handle and Protocol Globals
  //
  SEnvLoadHandleTable ();
  SEnvLoadHandleProtocolInfo (NULL);
  TempDevHandle = NULL;
  Item          = ChkPck.VarList;
  DeviceHandle  = NULL;
  ItemValue     = (UINTN) StrToUInt (Item->VarStr, 16, &Status);
  if (EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_ARG), HiiEnvHandle, L"disconnect", Item->VarStr);
    ReturnStatus = EFI_INVALID_PARAMETER;
    goto DisconnectDone;
  }

  HandleNumber = SEnvHandleNoFromUINT (ItemValue);
  if (HandleNumber != 0) {
    DeviceHandle  = SEnvHandles[HandleNumber - 1];
    TempDevHandle = StrDuplicate (Item->VarStr);
  } else {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_HANDLE_NOT_FOUND), HiiEnvHandle, L"disconnect", Item->VarStr);
    ReturnStatus = EFI_NOT_FOUND;
    goto DisconnectDone;
  }

  Item              = Item->Next;

  DriverImageHandle = NULL;
  if (NULL != Item) {
    ItemValue = (UINTN) StrToUIntegerBase (Item->VarStr, 16, &Status);
    if (EFI_ERROR (Status)) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_ARG), HiiEnvHandle, L"disconnect", Item->VarStr);
      ReturnStatus = EFI_INVALID_PARAMETER;
      goto DisconnectDone;
    }

    HandleNumber = SEnvHandleNoFromUINT (ItemValue);
    if (HandleNumber != 0) {
      DriverImageHandle = SEnvHandles[HandleNumber - 1];
    } else {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_HANDLE_NOT_FOUND), HiiEnvHandle, L"disconnect", Item->VarStr);
      ReturnStatus = EFI_NOT_FOUND;
      goto DisconnectDone;
    }

    Item = Item->Next;
  }

  if (DeviceHandle == NULL && DriverImageHandle == NULL) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_HANDLE_NOT_FOUND), HiiEnvHandle, L"disconnect", Item->VarStr);
    ReturnStatus = EFI_NOT_FOUND;
    goto DisconnectDone;
  }

  ChildHandle = NULL;
  if (NULL != Item) {
    ItemValue = (UINTN) StrToUIntegerBase (Item->VarStr, 16, &Status);
    if (EFI_ERROR (Status)) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_ARG), HiiEnvHandle, L"disconnect", Item->VarStr);
      ReturnStatus = EFI_INVALID_PARAMETER;
      goto DisconnectDone;
    }

    HandleNumber = SEnvHandleNoFromUINT (ItemValue);
    if (HandleNumber != 0) {
      ChildHandle = SEnvHandles[HandleNumber - 1];
    } else {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_HANDLE_NOT_FOUND), HiiEnvHandle, L"disconnect", Item->VarStr);
      ReturnStatus = EFI_NOT_FOUND;
      goto DisconnectDone;
    }
  }

  if (DeviceHandle != NULL) {
    Status = BS->HandleProtocol (
                  DeviceHandle,
                  &gEfiDevicePathProtocolGuid,
                  (VOID **) &DevPath
                  );
    if (EFI_ERROR (Status)) {
      PrintToken (
        STRING_TOKEN (STR_SHELLENV_CONNECT_DISC_DEVICE_PATH_PROTOCOL),
        HiiEnvHandle,
        L"disconnect",
        SEnvHandleToNumber (TempDevHandle)
        );
      ReturnStatus = EFI_INVALID_PARAMETER;
      goto DisconnectDone;
    }

    ReturnStatus = BS->DisconnectController (
                        DeviceHandle,
                        DriverImageHandle,
                        ChildHandle
                        );
  } else {
    Status = LibLocateHandle (
              ByProtocol,
              &gEfiDevicePathProtocolGuid,
              NULL,
              &DeviceHandleCount,
              &DeviceHandleBuffer
              );
    if (EFI_ERROR (Status)) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_CONNECT_DISC_DEVICE_PATH_PROTOCOL), HiiEnvHandle, L"disconnect");
      ReturnStatus = EFI_UNSUPPORTED;
      goto DisconnectDone;
    }

    ReturnStatus = EFI_NOT_FOUND;
    for (Index = 0; Index < DeviceHandleCount; Index++) {
      Status = BS->DisconnectController (
                    DeviceHandleBuffer[Index],
                    DriverImageHandle,
                    ChildHandle
                    );
      if (!EFI_ERROR (Status)) {
        ReturnStatus = EFI_SUCCESS;
      }
    }

    if (DeviceHandleCount != 0) {
      FreePool (DeviceHandleBuffer);
    }
  }

Done:
  Status = BS->HandleProtocol (
                SystemTable->ConsoleOutHandle,
                &gEfiSimpleTextOutProtocolGuid,
                (VOID **) &TextOut
                );
  if (EFI_ERROR (Status)) {
    SEnvConnectAllConsoles ();
    SEnvConnectAllDriversToAllControllers ();
    SEnvConnectAllConsoles ();
  }

  Status = BS->HandleProtocol (
                SystemTable->ConsoleInHandle,
                &gEfiSimpleTextInProtocolGuid,
                (VOID **) &TextIn
                );
  if (EFI_ERROR (Status)) {
    SEnvConnectAllConsoles ();
    SEnvConnectAllDriversToAllControllers ();
    SEnvConnectAllConsoles ();
  }

  PrintToken (
    STRING_TOKEN (STR_SHELLENV_CONNECT_DISC_CONTROLLER),
    HiiEnvHandle,
    L"disconnect",
    SEnvHandleToNumber (DeviceHandle),
    SEnvHandleToNumber (DriverImageHandle),
    SEnvHandleToNumber (ChildHandle),
    ReturnStatus
    );

  SEnvFreeHandleTable ();
  
DisconnectDone:
  LibCheckVarFreeVarList (&ChkPck);
  return ReturnStatus;
}

EFI_STATUS
SEnvDisconnectAll (
  VOID
  )
/*++

Routine Description:
  Disconnect All Handles and Exit the shell. This lets us go back to the BSD

Arguments:

Returns:
  
  EFI_SUCCESS - Success
  
--*/
{
  EFI_STATUS  Status;
  UINTN       AllHandleCount;
  EFI_HANDLE  *AllHandleBuffer;
  UINTN       Index;
  UINTN       NewIndex;
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
    //
    // Check whether the handle is still in handle database
    // as DisconnectController will destroy handles
    //
    Status = LibLocateHandle (
               AllHandles,
               NULL,
               NULL,
               &HandleCount,
               &HandleBuffer
               );
    if (EFI_ERROR (Status)) {
      return Status;
    }
    for (NewIndex = 0; NewIndex < HandleCount; NewIndex++) {
      if (HandleBuffer[NewIndex] == AllHandleBuffer[Index]) {
        break;
      }
    }
    FreePool (HandleBuffer);
    if (NewIndex == HandleCount) {
      //
      // The handle has been removed, skip it
      //
      continue;
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
      return Status;
    }

    Device = TRUE;
    
    if (HandleType[NewIndex] & EFI_HANDLE_TYPE_DRIVER_BINDING_HANDLE) {
      Device = FALSE;
    }

    if (HandleType[NewIndex] & EFI_HANDLE_TYPE_IMAGE_HANDLE) {
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
        if (HandleType[NewIndex] & EFI_HANDLE_TYPE_DEVICE_HANDLE) {
          //
          // Found a root controller handle, disconnect it
          //
          Status = BS->DisconnectController (
                        AllHandleBuffer[Index],
                        NULL,
                        NULL
                        );
        }
      }
    }

    FreePool (HandleBuffer);
    FreePool (HandleType);
  }

  FreePool (AllHandleBuffer);
  return EFI_SUCCESS;
}

EFI_STATUS
SEnvConnectDevicePath (
  EFI_DEVICE_PATH_PROTOCOL  *DevicePathToConnect
  )

{
  EFI_STATUS                Status;
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath;
  EFI_DEVICE_PATH_PROTOCOL  *OriginalDevicePath;
  EFI_DEVICE_PATH_PROTOCOL  *Instance;
  EFI_DEVICE_PATH_PROTOCOL  *RemainingDevicePath;
  EFI_DEVICE_PATH_PROTOCOL  *Next;
  EFI_HANDLE                Handle;
  EFI_HANDLE                PreviousHandle;
  UINTN                     Size;

  if (DevicePathToConnect == NULL) {
    return EFI_SUCCESS;
  }

  DevicePath          = DuplicateDevicePath (DevicePathToConnect);
  OriginalDevicePath  = DevicePath;

  if (DevicePath == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  do {

    Instance  = DevicePathInstance (&DevicePath, &Size);

    Next      = Instance;
    while (!IsDevicePathEndType (Next)) {
      Next = NextDevicePathNode (Next);
    }

    SetDevicePathEndNode (Next);

    PreviousHandle = NULL;
    do {
      //
      // Find the handle that best matches the Device Path. If it is only a
      // partial match the remaining part of the device path is returned in
      // RemainingDevicePath.
      //
      RemainingDevicePath = Instance;
      Status = BS->LocateDevicePath (
                    &gEfiDevicePathProtocolGuid,
                    &RemainingDevicePath,
                    &Handle
                    );
      if (!EFI_ERROR (Status)) {

        if (Handle == PreviousHandle) {
          Status = EFI_NOT_FOUND;
        } else {
          PreviousHandle = Handle;
          //
          // Connect all drivers that apply to Handle and RemainingDevicePath
          // If no drivers are connected Handle, then return EFI_NOT_FOUND
          // The Recursive flag is FALSE so only one level will be expanded.
          //
          Status = BS->ConnectController (
                        Handle,
                        NULL,
                        RemainingDevicePath,
                        FALSE
                        );
        }
      }
      //
      // Loop until RemainingDevicePath is an empty device path
      //
    } while (!EFI_ERROR (Status) && !IsDevicePathEnd (RemainingDevicePath));

  } while (DevicePath != NULL);

  if (OriginalDevicePath != NULL) {
    FreePool (OriginalDevicePath);
  }
  //
  // A handle with DevicePath exists in the handle database
  //
  return EFI_SUCCESS;
}

VOID
SEnvConnectConsole (
  CHAR16      *VariableName,
  EFI_GUID    *PrimaryGuid,
  EFI_GUID    *ConsoleGuid,
  EFI_HANDLE  *ConsoleHandle,
  VOID        **ConsoleInterface
  )
{
  EFI_STATUS                Status;
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath;
  UINTN                     Index;
  UINTN                     ConsoleIndex;
  UINTN                     AllHandleCount;
  EFI_HANDLE                *AllHandleBuffer;
  VOID                      *Interface;

  *ConsoleHandle    = NULL;
  *ConsoleInterface = NULL;
  ConsoleIndex      = 0;

  DevicePath        = LibGetVariable (VariableName, &gEfiGlobalVariableGuid);
  if (DevicePath != NULL) {
    SEnvConnectDevicePath (DevicePath);
    FreePool (DevicePath);
  }

  AllHandleBuffer = NULL;
  Status = BS->LocateHandleBuffer (
                ByProtocol,
                PrimaryGuid,
                NULL,
                &AllHandleCount,
                &AllHandleBuffer
                );
  if (!EFI_ERROR (Status) && AllHandleCount > 0) {
    *ConsoleHandle = AllHandleBuffer[0];
  } else if (*ConsoleHandle == NULL) {
    AllHandleBuffer = NULL;
    Status = BS->LocateHandleBuffer (
                  ByProtocol,
                  ConsoleGuid,
                  NULL,
                  &AllHandleCount,
                  &AllHandleBuffer
                  );
    //
    // Step 1: If a textout consplitter exists, attach to it, otherwise,
    // Step 2: Attach to first real console device.
    // For ConIn/ConOut device detection, the step 1 does the same thing as
    // previous does (find consplitter), but for StdErr device, it is useful
    // because we need to attach textout consplitter to StdErr first if
    // possible.
    //
    //
    if (!EFI_ERROR (Status) && AllHandleCount > 0) {
      for (Index = 0; Index < AllHandleCount; Index++) {
        Status = BS->HandleProtocol (
                      AllHandleBuffer[Index],
                      &gEfiDevicePathProtocolGuid,
                      &Interface
                      );
        if (EFI_ERROR (Status)) {
          ConsoleIndex = Index;
          break;
        }
      }
    }
  }

  *ConsoleHandle = AllHandleBuffer[ConsoleIndex];

  if (*ConsoleHandle != NULL) {
    BS->HandleProtocol (
          *ConsoleHandle,
          ConsoleGuid,
          ConsoleInterface
          );
  }

  if (AllHandleBuffer) {
    FreePool (AllHandleBuffer);
  }
}

VOID
SEnvConnectAllDriversToAllControllers (
  VOID
  )

{
  EFI_STATUS  Status;
  UINTN       AllHandleCount;
  EFI_HANDLE  *AllHandleBuffer;
  UINTN       Index;
  UINTN       HandleCount;
  UINTN       LastHandleCount;
  EFI_HANDLE  *HandleBuffer;
  UINT32      *HandleType;
  UINTN       HandleIndex;
  BOOLEAN     Parent;
  BOOLEAN     Device;

  LastHandleCount = 0;

  do {

    Status = LibLocateHandle (
              AllHandles,
              NULL,
              NULL,
              &AllHandleCount,
              &AllHandleBuffer
              );
    if (EFI_ERROR (Status)) {
      return ;
    }

    if (LastHandleCount == AllHandleCount) {
      FreePool (AllHandleBuffer);
      break;
    }

    LastHandleCount = AllHandleCount;

    for (Index = 0; Index < AllHandleCount; Index++) {
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
        return ;
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

    FreePool (AllHandleBuffer);
  } while (TRUE);
}

VOID
SEnvConnectAllConsoles (
  VOID
  )

{
  EFI_STATUS  Status;

  //
  // Check current ConIn and ConOut to ensure it is the ConsoleProxy
  // Otherwise, the Console should not be close
  //
  Status = SEnvCheckConsoleProxy (
            ST->ConsoleInHandle,
            &ST->ConIn,
            ST->ConsoleOutHandle,
            &ST->ConOut
            );

  //
  // Indicate the Console is replaced by redirection operation
  // It is not safe to connect Console here
  //
  if (EFI_ERROR (Status)) {
    return ;
  }
  //
  // To prevent the proxy console form being restored to original console,
  // close it temporary before connect all console
  //
  SEnvCloseConsoleProxy (
    ST->ConsoleInHandle,
    &ST->ConIn,
    ST->ConsoleOutHandle,
    &ST->ConOut
    );

  SEnvConnectConsole (
    VarErrorOut,
    &gEfiPrimaryStandardErrorDeviceGuid,
    &gEfiSimpleTextOutProtocolGuid,
    &ST->StandardErrorHandle,
    (VOID **) &ST->StdErr
    );

  SEnvConnectConsole (
    VarConsoleOut,
    &gEfiPrimaryConsoleOutDeviceGuid,
    &gEfiSimpleTextOutProtocolGuid,
    &ST->ConsoleOutHandle,
    (VOID **) &ST->ConOut
    );

  SEnvConnectConsole (
    VarConsoleIn,
    &gEfiPrimaryConsoleInDeviceGuid,
    &gEfiSimpleTextInProtocolGuid,
    &ST->ConsoleInHandle,
    (VOID **) &ST->ConIn
    );

  //
  // Reopen the proxy console
  //
  SEnvOpenConsoleProxy (
    ST->ConsoleInHandle,
    &ST->ConIn,
    ST->ConsoleOutHandle,
    &ST->ConOut
    );

  SetCrc (&ST->Hdr);
}

EFI_STATUS
EFIAPI
SEnvCmdReconnect (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  )

{
  EFI_STATUS                    Status;
  EFI_STATUS                    ReturnStatus;
  UINTN                         HandleNumber;
  EFI_HANDLE                    DriverImageHandle;
  EFI_HANDLE                    DeviceHandle;
  EFI_HANDLE                    ChildHandle;
  EFI_HANDLE                    ContextOverride[2];
  EFI_SIMPLE_TEXT_OUT_PROTOCOL  *TextOut;
  EFI_SIMPLE_TEXT_IN_PROTOCOL   *TextIn;
  SHELL_VAR_CHECK_CODE          RetCode;
  CHAR16                        *Useful;
  SHELL_VAR_CHECK_PACKAGE       ChkPck;
  SHELL_ARG_LIST                *Item;
  UINTN                         ItemValue;

  EFI_SHELL_APP_INIT (ImageHandle, SystemTable);

  Item    = NULL;
  Useful  = NULL;
  Status  = EFI_SUCCESS;
  ReturnStatus = EFI_SUCCESS;
  ZeroMem (&ChkPck, sizeof (SHELL_VAR_CHECK_PACKAGE));

  if (!EFI_PROPER_VERSION (1, 10)) {
    PrintToken (
      STRING_TOKEN (STR_SHELLENV_GNC_COMMAND_NOT_SUPPORT),
      HiiEnvHandle,
      L"reconnect",
      EFI_VERSION_1_10
      );
    ReturnStatus = EFI_UNSUPPORTED;
    goto ReconnectDone;
  }

  RetCode = LibCheckVariables (SI, ReconnectCheckList, &ChkPck, &Useful);
  if (VarCheckOk != RetCode) {
    switch (RetCode) {
    case VarCheckDuplicate:

      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_DUP_FLAG), HiiEnvHandle, L"reconnect", Useful);
      break;

    case VarCheckUnknown:

      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_UNKNOWN_FLAG), HiiEnvHandle, L"reconnect", Useful);
      break;

    default:
      break;
    }

    ReturnStatus = EFI_INVALID_PARAMETER;
    goto ReconnectDone;
  }
  //
  //  Out put help.
  //
  if (LibCheckVarGetFlag (&ChkPck, L"-b") != NULL) {
    EnablePageBreak (DEFAULT_INIT_ROW, DEFAULT_AUTO_LF);
  }

  if (LibCheckVarGetFlag (&ChkPck, L"-?") != NULL) {
    if (IS_OLD_SHELL) {
      PrintToken (STRING_TOKEN (STR_NO_HELP), HiiEnvHandle);
    } else if (ChkPck.ValueCount > 0 ||
             ChkPck.FlagCount > 2 ||
             (2 == ChkPck.FlagCount && !LibCheckVarGetFlag (&ChkPck, L"-b"))
            ) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiEnvHandle, L"reconnect");
      Status = EFI_INVALID_PARAMETER;
    } else {
      PrintToken (STRING_TOKEN (STR_SHELLENV_RECONNECT_VERBOSE_HELP), HiiEnvHandle);
      Status = EFI_SUCCESS;
    }

    goto ReconnectDone;
  }

  if (ChkPck.ValueCount < 1 && LibCheckVarGetFlag (&ChkPck, L"-r") == NULL) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_FEW), HiiEnvHandle, L"reconnect");
    ReturnStatus = EFI_INVALID_PARAMETER;
    goto ReconnectDone;
  }

  if (ChkPck.ValueCount > 3) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiEnvHandle, L"reconnect");
    ReturnStatus = EFI_INVALID_PARAMETER;
    goto ReconnectDone;
  }

  Status            = EFI_SUCCESS;
  DeviceHandle      = NULL;
  DriverImageHandle = NULL;
  ChildHandle       = NULL;

  //
  // If command line is reconnect all the do that.
  //
  if (LibCheckVarGetFlag (&ChkPck, L"-r") != NULL) {
    if (ChkPck.ValueCount > 0) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_FLAG_CONFLICT), HiiEnvHandle, L"reconnect", L"-r");
      ReturnStatus = EFI_INVALID_PARAMETER;
      goto ReconnectDone;
    }

    if (SI->RedirArgc != 0) {
      ReturnStatus = EFI_REDIRECTION_NOT_ALLOWED;
      goto ReconnectDone;
    }

    SEnvDisconnectAll ();
    SEnvConnectAllDriversToAllControllers ();
    SEnvConnectAllConsoles ();
    ReturnStatus = EFI_SUCCESS;
    goto Done;
  }
  //
  // Setup Handle and Protocol Globals
  //
  SEnvLoadHandleTable ();
  SEnvLoadHandleProtocolInfo (NULL);

  DeviceHandle  = NULL;
  Item          = ChkPck.VarList;
  if (NULL != Item) {
    ItemValue = (UINTN) StrToUIntegerBase (Item->VarStr, 16, &Status);
    if (EFI_ERROR (Status)) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_ARG), HiiEnvHandle, L"reconnect", Item->VarStr);
      ReturnStatus = EFI_INVALID_PARAMETER;
      goto ReconnectDone;
    }

    HandleNumber = SEnvHandleNoFromUINT (ItemValue);
    if (HandleNumber != 0) {
      DeviceHandle = SEnvHandles[HandleNumber - 1];
    }

    if (DeviceHandle == NULL) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_HANDLE_NOT_FOUND), HiiEnvHandle, L"reconnect", Item->VarStr);
      ReturnStatus = EFI_NOT_FOUND;
      goto ReconnectDone;
    }

    Item = Item->Next;
  }

  DriverImageHandle = NULL;
  if (NULL != Item) {
    ItemValue = (UINTN) StrToUIntegerBase (Item->VarStr, 16, &Status);
    if (EFI_ERROR (Status)) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_ARG), HiiEnvHandle, L"reconnect", Item->VarStr);
      ReturnStatus = EFI_INVALID_PARAMETER;
      goto ReconnectDone;
    }

    HandleNumber = SEnvHandleNoFromUINT (ItemValue);
    if (HandleNumber != 0) {
      DriverImageHandle = SEnvHandles[HandleNumber - 1];
    }

    if (DriverImageHandle == NULL) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_HANDLE_NOT_FOUND), HiiEnvHandle, L"reconnect", Item->VarStr);
      ReturnStatus = EFI_NOT_FOUND;
      goto ReconnectDone;
    }

    Item = Item->Next;
  }

  ChildHandle = NULL;
  if (NULL != Item) {
    ItemValue = (UINTN) StrToUIntegerBase (Item->VarStr, 16, &Status);
    if (EFI_ERROR (Status)) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_ARG), HiiEnvHandle, L"reconnect", Item->VarStr);
      ReturnStatus = EFI_INVALID_PARAMETER;
      goto ReconnectDone;
    }

    HandleNumber = SEnvHandleNoFromUINT (ItemValue);
    if (HandleNumber != 0) {
      ChildHandle = SEnvHandles[HandleNumber - 1];
    }

    Item = Item->Next;
  }

  if (DeviceHandle != NULL) {
    ReturnStatus = BS->DisconnectController (
                        DeviceHandle,
                        DriverImageHandle,
                        ChildHandle
                        );
    ContextOverride[0]  = DriverImageHandle;
    ContextOverride[1]  = NULL;
    Status = BS->ConnectController (
                  DeviceHandle,
                  ContextOverride,
                  NULL,
                  TRUE
                  );
    if (!EFI_ERROR (ReturnStatus)) {
      ReturnStatus = Status;
    }
  } else {

    SEnvDisconnectAll ();
    SEnvConnectAllDriversToAllControllers ();
    SEnvConnectAllConsoles ();
    ReturnStatus = EFI_SUCCESS;
  }

Done:
  Status = BS->HandleProtocol (
                SystemTable->ConsoleOutHandle,
                &gEfiSimpleTextOutProtocolGuid,
                (VOID **) &TextOut
                );
  if (EFI_ERROR (Status)) {
    SEnvConnectAllConsoles ();
    SEnvConnectAllDriversToAllControllers ();
    SEnvConnectAllConsoles ();
  }

  Status = BS->HandleProtocol (
                SystemTable->ConsoleInHandle,
                &gEfiSimpleTextInProtocolGuid,
                (VOID **) &TextIn
                );
  if (EFI_ERROR (Status)) {
    SEnvConnectAllConsoles ();
    SEnvConnectAllDriversToAllControllers ();
    SEnvConnectAllConsoles ();
  }

  PrintToken (
    STRING_TOKEN (STR_SHELLENV_CONNECT_REC_CONTROLLER),
    HiiEnvHandle,
    SEnvHandleToNumber (DeviceHandle),
    SEnvHandleToNumber (DriverImageHandle),
    SEnvHandleToNumber (ChildHandle),
    ReturnStatus
    );
  SEnvFreeHandleTable ();
ReconnectDone:
  LibCheckVarFreeVarList (&ChkPck);
  return ReturnStatus;
}

EFI_STATUS
EFIAPI
SEnvCmdConnectGetLineHelp (
  OUT CHAR16                  **Str
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
  return SEnvCmdGetStringByToken (STRING_TOKEN (STR_SHELLENV_CONNECT_LINE_HELP), Str);
}

EFI_STATUS
EFIAPI
SEnvCmdDisconnectGetLineHelp (
  OUT CHAR16                  **Str
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
  return SEnvCmdGetStringByToken (STRING_TOKEN (STR_SHELLENV_DISCONNECT_LINE_HELP), Str);
}

EFI_STATUS
EFIAPI
SEnvCmdReconnectGetLineHelp (
  OUT CHAR16                  **Str
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
  return SEnvCmdGetStringByToken (STRING_TOKEN (STR_SHELLENV_RECONNECT_LINE_HELP), Str);
}
