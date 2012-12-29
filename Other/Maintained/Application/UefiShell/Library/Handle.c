/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

    Handle.c

Abstract:

  The handle function
  
Revision History

--*/

#include "EfiShellLib.h"

EFI_STATUS
LibLocateHandle (
  IN EFI_LOCATE_SEARCH_TYPE       SearchType,
  IN EFI_GUID                     * Protocol OPTIONAL,
  IN VOID                         *SearchKey OPTIONAL,
  IN OUT UINTN                    *NoHandles,
  OUT EFI_HANDLE                  **Buffer
  )
/*++

Routine Description:

  Function returns an array of handles that support the requested protocol 
  in a buffer allocated from pool.

Arguments:

  SearchType           - Specifies which handle(s) are to be returned.
  Protocol             - Provides the protocol to search by.   
                         This parameter is only valid for SearchType ByProtocol.
  SearchKey            - Supplies the search key depending on the SearchType.
  NoHandles            - The number of handles returned in Buffer.
  Buffer               - A pointer to the buffer to return the requested array of 
                         handles that support Protocol.

Returns:
  
  EFI_SUCCESS           - The result array of handles was returned.
  EFI_NOT_FOUND         - No handles match the search. 
  EFI_OUT_OF_RESOURCES - There is not enough pool memory to store the matching results.

--*/
{
  EFI_STATUS  Status;
  UINTN       BufferSize;
  
  ASSERT (NoHandles != NULL);
  ASSERT (Buffer != NULL);

  //
  // Initialize for GrowBuffer loop
  //
  Status      = EFI_SUCCESS;
  *Buffer     = NULL;
  BufferSize  = 50 * sizeof (EFI_HANDLE);

  //
  // Call the real function
  //
  while (GrowBuffer (&Status, (VOID **) Buffer, BufferSize)) {
    Status = BS->LocateHandle (
                  SearchType,
                  Protocol,
                  SearchKey,
                  &BufferSize,
                  *Buffer
                  );
  }

  *NoHandles = BufferSize / sizeof (EFI_HANDLE);
  if (EFI_ERROR (Status)) {
    *NoHandles = 0;
  }

  return Status;
}

EFI_STATUS
LibLocateHandleByDiskSignature (
  IN UINT8                        MBRType,
  IN UINT8                        SignatureType,
  IN VOID                         *Signature,
  IN OUT UINTN                    *NoHandles,
  OUT EFI_HANDLE                  **Buffer
  )
/*++

Routine Description:

  Function returns an array of handles that support the requested protocol in 
  a buffer allocated from pool.

Arguments:

  MBRType              - Specifies the type of MBR to search for.  This can either 
                         be the PC AT compatible MBR or an EFI Partition Table Header.
  SignatureType        - Specifies the type of signature to look for in the MBR.  
                         This can either be a 32 bit signature, or a GUID signature.
  Signature            - A pointer to a 32 bit disk signature or a pointer to a GUID 
                         disk signature. The type depends on SignatureType.
  NoHandles            - The number of handles returned in Buffer.
  Buffer               - A pointer to the buffer to return the requested array of 
                         handles that support Protocol.

Returns:
  
  EFI_SUCCESS           - The result array of handles was returned.
  EFI_NOT_FOUND         - No handles match the search. 
  EFI_OUT_OF_RESOURCES - There is not enough pool memory to store the matching results.

--*/
{
  EFI_STATUS                Status;
  UINTN                     BufferSize;
  UINTN                     NoBlockIoHandles;
  EFI_HANDLE                *BlockIoBuffer;
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath;
  UINTN                     Index;
  EFI_DEVICE_PATH_PROTOCOL  *Start;
  EFI_DEVICE_PATH_PROTOCOL  *Next;
  EFI_DEVICE_PATH_PROTOCOL  *DevPath;
  HARDDRIVE_DEVICE_PATH     *HardDriveDevicePath;
  BOOLEAN                   Match;
  BOOLEAN                   PreviousNodeIsHardDriveDevicePath;

  ASSERT (NoHandles != NULL);
  ASSERT (Buffer != NULL);
  //
  // Initialize for GrowBuffer loop
  //
  BlockIoBuffer = NULL;
  BufferSize    = 50 * sizeof (EFI_HANDLE);

  //
  // Call the real function
  //
  while (GrowBuffer (&Status, (VOID **) &BlockIoBuffer, BufferSize)) {
    //
    // Get list of device handles that support the BLOCK_IO Protocol.
    //
    Status = BS->LocateHandle (
                  ByProtocol,
                  &gEfiBlockIoProtocolGuid,
                  NULL,
                  &BufferSize,
                  BlockIoBuffer
                  );
  }

  NoBlockIoHandles = BufferSize / sizeof (EFI_HANDLE);
  if (EFI_ERROR (Status)) {
    NoBlockIoHandles = 0;
  }
  //
  // If there was an error or there are no device handles that support
  // the BLOCK_IO Protocol, then return.
  //
  if (NoBlockIoHandles == 0) {
    ASSERT(BlockIoBuffer != NULL);
    FreePool (BlockIoBuffer);
    *NoHandles  = 0;
    *Buffer     = NULL;
    return Status;
  }
  //
  // Loop through all the device handles that support the BLOCK_IO Protocol
  //
  *NoHandles = 0;

  for (Index = 0; Index < NoBlockIoHandles; Index++) {

    Status = BS->HandleProtocol (
                  BlockIoBuffer[Index],
                  &gEfiDevicePathProtocolGuid,
                  (VOID *) &DevicePath
                  );

    //
    // Search DevicePath for a Hard Drive Media Device Path node.
    // If one is found, then see if it matches the signature that was
    // passed in.  If it does match, and the next node is the End of the
    // device path, and the previous node is not a Hard Drive Media Device
    // Path, then we have found a match.
    //
    Match = FALSE;

    if (DevicePath != NULL) {

      PreviousNodeIsHardDriveDevicePath = FALSE;

      DevPath = DevicePath;
      Start = DevPath;
      //
      // Check for end of device path type
      //
      for (;;) {

        if ((DevicePathType (DevPath) == MEDIA_DEVICE_PATH) && (DevicePathSubType (DevPath) == MEDIA_HARDDRIVE_DP)) {

          HardDriveDevicePath = (HARDDRIVE_DEVICE_PATH *) (DevPath);

          if (PreviousNodeIsHardDriveDevicePath == FALSE) {

            Next = NextDevicePathNode (DevPath);
            if (IsDevicePathEndType (Next)) {
              if ((HardDriveDevicePath->MBRType == MBRType) && (HardDriveDevicePath->SignatureType == SignatureType)) {
                switch (SignatureType) {
                case SIGNATURE_TYPE_MBR:
                  if (*((UINT32 *) (Signature)) == * (UINT32 *) (&(HardDriveDevicePath->Signature[0]))) {
                    Match = TRUE;
                  }
                  break;

                case SIGNATURE_TYPE_GUID:
                  if (CompareGuid ((EFI_GUID *) Signature, (EFI_GUID *) (&(HardDriveDevicePath->Signature[0]))) == 0) {
                    Match = TRUE;
                  }
                  break;
                }
              }
            }
          }

          PreviousNodeIsHardDriveDevicePath = TRUE;
        } else {
          PreviousNodeIsHardDriveDevicePath = FALSE;
        }

        if (IsDevicePathEnd (DevPath)) {
          break;
        }

        DevPath = NextDevicePathNode (DevPath);
      }

    }

    if (Match == FALSE) {
      BlockIoBuffer[Index] = NULL;
    } else {
      *NoHandles = *NoHandles + 1;
    }
  }
  //
  // If there are no matches, then return
  //
  if (*NoHandles == 0) {
    FreePool (BlockIoBuffer);
    *NoHandles  = 0;
    *Buffer     = NULL;
    return EFI_SUCCESS;
  }
  //
  // Allocate space for the return buffer of device handles.
  //
  *Buffer = AllocatePool (*NoHandles * sizeof (EFI_HANDLE));

  if (*Buffer == NULL) {
    FreePool (BlockIoBuffer);
    *NoHandles  = 0;
    *Buffer     = NULL;
    return EFI_OUT_OF_RESOURCES;
  }
  //
  // Build list of matching device handles.
  //
  *NoHandles = 0;
  for (Index = 0; Index < NoBlockIoHandles; Index++) {
    if (BlockIoBuffer[Index] != NULL) {
      (*Buffer)[*NoHandles] = BlockIoBuffer[Index];
      *NoHandles            = *NoHandles + 1;
    }
  }

  FreePool (BlockIoBuffer);

  return EFI_SUCCESS;
}

EFI_FILE_SYSTEM_INFO *
LibFileSystemInfo (
  IN EFI_FILE_HANDLE      FHand
  )
/*++

Routine Description:

  Function gets the file system information from an open file descriptor, 
  and stores it in a buffer allocated from pool.

Arguments:

  FHand         - A file handle

Returns:
  
  A pointer to a buffer with file information or NULL is returned

--*/
{
  EFI_STATUS            Status;
  EFI_FILE_SYSTEM_INFO  *Buffer;
  UINTN                 BufferSize;
  
  ASSERT (FHand != NULL);
  
  //
  // Initialize for GrowBuffer loop
  //
  Buffer      = NULL;
  BufferSize  = SIZE_OF_EFI_FILE_SYSTEM_INFO + 200;

  //
  // Call the real function
  //
  while (GrowBuffer (&Status, (VOID **) &Buffer, BufferSize)) {
    Status = FHand->GetInfo (
                      FHand,
                      &gEfiFileSystemInfoGuid,
                      &BufferSize,
                      Buffer
                      );
  }

  return Buffer;
}

EFI_FILE_SYSTEM_VOLUME_LABEL_INFO *
LibFileSystemVolumeLabelInfo (
  IN EFI_FILE_HANDLE      FHand
  )
/*++

Routine Description:

  Function gets the file system information from an open file descriptor, 
  and stores it in a buffer allocated from pool.

Arguments:

  FHand         - A file handle

Returns:
  
  A pointer to a buffer with file information or NULL is returned

--*/
{
  EFI_STATUS                        Status;
  EFI_FILE_SYSTEM_VOLUME_LABEL_INFO *Buffer;
  UINTN                             BufferSize;

  ASSERT (FHand != NULL);

  //
  // Initialize for GrowBuffer loop
  //
  Buffer      = NULL;
  BufferSize  = SIZE_OF_EFI_FILE_SYSTEM_VOLUME_LABEL_INFO + 200;

  //
  // Call the real function
  //
  while (GrowBuffer (&Status, (VOID **) &Buffer, BufferSize)) {
    Status = FHand->GetInfo (
                      FHand,
                      &gEfiFileSystemVolumeLabelInfoIdGuid,
                      &BufferSize,
                      Buffer
                      );
  }

  return Buffer;
}

EFI_STATUS
LibInstallProtocolInterfaces (
  IN OUT EFI_HANDLE             *Handle,
  ...
  )
/*++

Routine Description:

  Function installs one or more protocol interfaces into the boot services environment.

Arguments:

  Handle               - The handle to install the new protocol interfaces on, or NULL if a 
                         new handle is to be allocated.

  ...                  - A variable argument list containing pairs of protocol GUIDs and 
                         protocol interfaces.

Returns:
  
  EFI_SUCCESS           - All the protocol interfaces were installed.
  EFI_OUT_OF_RESOURCES - There was not enough memory in pool to install all the protocols.

--*/
{
  VA_LIST     args;
  EFI_STATUS  Status;
  EFI_GUID    *Protocol;
  VOID        *Interface;
  EFI_TPL     OldTpl;
  UINTN       Index;
  EFI_HANDLE  OldHandle;

  ASSERT (Handle);
  
  //
  // Syncronize with notifcations
  //
  OldTpl    = BS->RaiseTPL (EFI_TPL_NOTIFY);
  OldHandle = *Handle;

  //
  // Install the protocol interfaces
  //
  Index   = 0;
  Status  = EFI_SUCCESS;
  VA_START (args, Handle);

  while (!EFI_ERROR (Status)) {
    //
    // If protocol is NULL, then it's the end of the list
    //
    Protocol = VA_ARG (args, EFI_GUID *);
    if (!Protocol) {
      break;
    }

    Interface = VA_ARG (args, VOID *);

    //
    // Install it
    //
    BS->RestoreTPL (OldTpl);
    DEBUG ((EFI_D_INFO, "LibInstallProtocolInterface: %d %x\n", Protocol, Interface));
    BS->RaiseTPL (EFI_TPL_NOTIFY);

    Status = BS->InstallProtocolInterface (
                  Handle,
                  Protocol,
                  EFI_NATIVE_INTERFACE,
                  Interface
                  );
    if (EFI_ERROR (Status)) {
      break;
    }

    Index += 1;
  }
  //
  // If there was an error, remove all the interfaces that were
  // installed without any errors
  //
  if (EFI_ERROR (Status)) {
    VA_START (args, Handle);
    while (Index) {
      Protocol  = VA_ARG (args, EFI_GUID *);
      Interface = VA_ARG (args, VOID *);
      BS->UninstallProtocolInterface (
            *Handle,
            Protocol,
            Interface
            );
      Index -= 1;
    }

    *Handle = OldHandle;
  }
  //
  // Done
  //
  BS->RestoreTPL (OldTpl);
  return Status;
}

VOID
LibUninstallProtocolInterfaces (
  IN EFI_HANDLE             Handle,
  ...
  )
/*++

Routine Description:

  Function removes one or more protocol interfaces into the boot services environment.

Arguments:

  Handle               - The handle to remove the protocol interfaces from.

  ...                  - A variable argument list containing pairs of protocol GUIDs and 
                         protocol interfaces.

Returns:
  
  EFI_SUCCESS           - All the protocol interfaces were removed.
  EFI_OUT_OF_RESOURCES - One of the protocol interfaces could not be found.

--*/
{
  VA_LIST     args;
  EFI_STATUS  Status;
  EFI_GUID    *Protocol;
  VOID        *Interface;

  VA_START (args, Handle);
  for (;;) {
    //
    // If protocol is NULL, then it's the end of the list
    //
    Protocol = VA_ARG (args, EFI_GUID *);
    if (!Protocol) {
      break;
    }

    Interface = VA_ARG (args, VOID *);

    //
    // Uninstall it
    //
    Status = BS->UninstallProtocolInterface (Handle, Protocol, Interface);
    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR, "LibUninstallProtocolInterfaces: failed %g, %x\n", Protocol, Handle));
    }
  }
}

EFI_STATUS
LibReinstallProtocolInterfaces (
  IN OUT EFI_HANDLE             *Handle,
  ...
  )
/*++

Routine Description:

  Function replaces one or more protocol interfaces into the boot services environment.

Arguments:

  Handle               - The handle to reinstall the protocol interfaces to.

  ...                  - A variable argument list containing pairs of protocol GUIDs and 
                         protocol interfaces.

Returns:
  
  EFI_SUCCESS           - All the protocol interfaces were replaced.
  EFI_OUT_OF_RESOURCES - One of the protocol interfaces could not be found.

--*/
{
  VA_LIST     args;
  EFI_STATUS  Status;
  EFI_GUID    *Protocol;
  VOID        *OldInterface;
  VOID        *NewInterface;
  EFI_TPL     OldTpl;
  UINTN       Index;

  //
  // Syncronize with notifcations
  //
  OldTpl = BS->RaiseTPL (EFI_TPL_NOTIFY);

  //
  // Install the protocol interfaces
  //
  Index   = 0;
  Status  = EFI_SUCCESS;
  VA_START (args, Handle);

  while (!EFI_ERROR (Status)) {
    //
    // If protocol is NULL, then it's the end of the list
    //
    Protocol = VA_ARG (args, EFI_GUID *);
    if (!Protocol) {
      break;
    }

    OldInterface  = VA_ARG (args, VOID *);
    NewInterface  = VA_ARG (args, VOID *);

    //
    // Reinstall it
    //
    Status = BS->ReinstallProtocolInterface (
                  Handle,
                  Protocol,
                  OldInterface,
                  NewInterface
                  );
    if (EFI_ERROR (Status)) {
      break;
    }

    Index += 1;
  }
  //
  // If there was an error, undo all the interfaces that were
  // reinstalled without any errors
  //
  if (EFI_ERROR (Status)) {
    VA_START (args, Handle);
    while (Index) {
      Protocol      = VA_ARG (args, EFI_GUID *);
      OldInterface  = VA_ARG (args, VOID *);
      NewInterface  = VA_ARG (args, VOID *);

      BS->ReinstallProtocolInterface (
            Handle,
            Protocol,
            NewInterface,
            OldInterface
            );

      Index -= 1;
    }
  }
  //
  // Done
  //
  BS->RestoreTPL (OldTpl);
  return Status;
}

UINTN
ShellHandleNoFromIndex (
  IN UINTN        Value
  )
{

  UINTN TotalHandleNo;
  UINTN HandleNo;

  TotalHandleNo = SE2->HandleEnumerator.GetNum ();

  HandleNo      = Value;
  HandleNo      = HandleNo > TotalHandleNo ? 0 : HandleNo;
  return HandleNo;

}

EFI_HANDLE
ShellHandleFromIndex (
  IN UINTN        Value
  )
/*++

Routine Description:

Arguments:

  Value   - The index

Returns:

--*/
{
  UINTN       HandleNo;
  UINTN       TotalHandleNo;
  UINTN       CurHandleNo;

  EFI_HANDLE  *Handle;

  CurHandleNo   = SE2->HandleEnumerator.Reset (0);
  TotalHandleNo = SE2->HandleEnumerator.GetNum ();
  HandleNo      = Value;

  if (HandleNo > TotalHandleNo - 1) {
    return NULL;

  }

  SE2->HandleEnumerator.Skip (HandleNo);
  SE2->HandleEnumerator.Next (&Handle);

  //
  // restore context
  //
  SE2->HandleEnumerator.Reset (CurHandleNo);
  return *Handle;
}

UINTN
ShellHandleToIndex (
  IN  EFI_HANDLE  Handle
  )
/*++

Routine Description:

Arguments:

  Handle - The handle

Returns:

--*/
{
  UINTN       Index;
  UINTN       TotalHandleNo;
  UINTN       CurHandleNo;

  EFI_HANDLE  *HandleIterator;

  CurHandleNo   = SE2->HandleEnumerator.Reset (0);

  TotalHandleNo = SE2->HandleEnumerator.GetNum ();

  for (Index = 0; Index < TotalHandleNo; Index++) {
    SE2->HandleEnumerator.Next (&HandleIterator);
    if (*HandleIterator == Handle) {
      //
      // restore context
      //
      CurHandleNo = SE2->HandleEnumerator.Reset (CurHandleNo);
      return Index + 1;
    }
  }
  //
  // restore context
  //
  SE2->HandleEnumerator.Reset (CurHandleNo);
  return 0;
}

UINTN
ShellHandleNoFromStr (
  IN CHAR16       *Str
  )
/*++

Routine Description:

Arguments:

  Str  - The string

Returns:

--*/
{
  UINTN HandleNo;
  UINTN TotalHandleNo;

  ASSERT (Str);
  
  HandleNo      = Xtoi (Str);
  TotalHandleNo = SE2->HandleEnumerator.GetNum ();

  HandleNo      = HandleNo > TotalHandleNo ? 0 : HandleNo;
  return HandleNo;
}

EFI_STATUS
LibScanHandleDatabase (
  EFI_HANDLE  DriverBindingHandle, OPTIONAL
  UINT32      *DriverBindingHandleIndex, OPTIONAL
  EFI_HANDLE  ControllerHandle, OPTIONAL
  UINT32      *ControllerHandleIndex, OPTIONAL
  UINTN       *HandleCount,
  EFI_HANDLE  **HandleBuffer,
  UINT32      **HandleType
  )

{
  EFI_STATUS                          Status;
  UINTN                               HandleIndex;
  EFI_GUID                            **ProtocolGuidArray;
  UINTN                               ArrayCount;
  UINTN                               ProtocolIndex;
  EFI_OPEN_PROTOCOL_INFORMATION_ENTRY *OpenInfo;
  UINTN                               OpenInfoCount;
  UINTN                               OpenInfoIndex;
  UINTN                               ChildIndex;
  BOOLEAN                             DriverBindingHandleIndexValid;
  BOOLEAN                             ControllerHandleIndexValid;

  DriverBindingHandleIndexValid = FALSE;
  if (DriverBindingHandleIndex != NULL) {
    *DriverBindingHandleIndex = 0xffffffff;
  }

  ControllerHandleIndexValid = FALSE;
  if (ControllerHandleIndex != NULL) {
    *ControllerHandleIndex = 0xffffffff;
  }

  *HandleCount  = 0;
  *HandleBuffer = NULL;
  *HandleType   = NULL;

  //
  // Retrieve the list of all handles from the handle database
  //
  Status = BS->LocateHandleBuffer (
                AllHandles,
                NULL,
                NULL,
                HandleCount,
                HandleBuffer
                );
  if (EFI_ERROR (Status)) {
    goto Error;
  }

  *HandleType = AllocatePool (*HandleCount * sizeof (UINT32));
  if (*HandleType == NULL) {
    goto Error;
  }

  for (HandleIndex = 0; HandleIndex < *HandleCount; HandleIndex++) {
    //
    // Assume that the handle type is unknown
    //
    (*HandleType)[HandleIndex] = EFI_HANDLE_TYPE_UNKNOWN;

    if (DriverBindingHandle != NULL &&
        DriverBindingHandleIndex != NULL &&
        (*HandleBuffer)[HandleIndex] == DriverBindingHandle
        ) {
      *DriverBindingHandleIndex     = (UINT32) HandleIndex;
      DriverBindingHandleIndexValid = TRUE;
    }

    if (ControllerHandle != NULL && ControllerHandleIndex != NULL && (*HandleBuffer)[HandleIndex] == ControllerHandle) {
      *ControllerHandleIndex      = (UINT32) HandleIndex;
      ControllerHandleIndexValid  = TRUE;
    }

  }

  for (HandleIndex = 0; HandleIndex < *HandleCount; HandleIndex++) {
    //
    // Retrieve the list of all the protocols on each handle
    //
    Status = BS->ProtocolsPerHandle (
                  (*HandleBuffer)[HandleIndex],
                  &ProtocolGuidArray,
                  &ArrayCount
                  );
    if (!EFI_ERROR (Status)) {

      for (ProtocolIndex = 0; ProtocolIndex < ArrayCount; ProtocolIndex++) {

        if (CompareGuid (ProtocolGuidArray[ProtocolIndex], &gEfiLoadedImageProtocolGuid) == 0) {
          (*HandleType)[HandleIndex] |= EFI_HANDLE_TYPE_IMAGE_HANDLE;
        }

        if (CompareGuid (ProtocolGuidArray[ProtocolIndex], &gEfiDriverBindingProtocolGuid) == 0) {
          (*HandleType)[HandleIndex] |= EFI_HANDLE_TYPE_DRIVER_BINDING_HANDLE;
        }

        if (CompareGuid (ProtocolGuidArray[ProtocolIndex], &gEfiDriverConfigurationProtocolGuid) == 0) {
          (*HandleType)[HandleIndex] |= EFI_HANDLE_TYPE_DRIVER_CONFIGURATION_HANDLE;
        }

        if (CompareGuid (ProtocolGuidArray[ProtocolIndex], &gEfiDriverDiagnosticsProtocolGuid) == 0) {
          (*HandleType)[HandleIndex] |= EFI_HANDLE_TYPE_DRIVER_DIAGNOSTICS_HANDLE;
        }

        if (CompareGuid (ProtocolGuidArray[ProtocolIndex], &gEfiComponentNameProtocolGuid) == 0) {
          (*HandleType)[HandleIndex] |= EFI_HANDLE_TYPE_COMPONENT_NAME_HANDLE;
        }

        if (CompareGuid (ProtocolGuidArray[ProtocolIndex], &gEfiDevicePathProtocolGuid) == 0) {
          (*HandleType)[HandleIndex] |= EFI_HANDLE_TYPE_DEVICE_HANDLE;
        }
        //
        // Retrieve the list of agents that have opened each protocol
        //
        Status = BS->OpenProtocolInformation (
                      (*HandleBuffer)[HandleIndex],
                      ProtocolGuidArray[ProtocolIndex],
                      &OpenInfo,
                      &OpenInfoCount
                      );
        if (!EFI_ERROR (Status)) {

          for (OpenInfoIndex = 0; OpenInfoIndex < OpenInfoCount; OpenInfoIndex++) {
            if (DriverBindingHandle != NULL && OpenInfo[OpenInfoIndex].AgentHandle == DriverBindingHandle) {
              if ((OpenInfo[OpenInfoIndex].Attributes & EFI_OPEN_PROTOCOL_BY_DRIVER) == EFI_OPEN_PROTOCOL_BY_DRIVER) {
                //
                // Mark the device handle as being managed by the driver specified by DriverBindingHandle
                //
                (*HandleType)[HandleIndex] |= (EFI_HANDLE_TYPE_DEVICE_HANDLE | EFI_HANDLE_TYPE_CONTROLLER_HANDLE);
                //
                // Mark the DriverBindingHandle as being a driver that is managing at least one controller
                //
                if (DriverBindingHandleIndexValid) {
                  (*HandleType)[*DriverBindingHandleIndex] |= EFI_HANDLE_TYPE_DEVICE_DRIVER;
                }
              }

              if ((
                    OpenInfo[OpenInfoIndex].Attributes & EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER
                ) == EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER
                  ) {
                //
                // Mark the DriverBindingHandle as being a driver that is managing at least one child controller
                //
                if (DriverBindingHandleIndexValid) {
                  (*HandleType)[*DriverBindingHandleIndex] |= EFI_HANDLE_TYPE_BUS_DRIVER;
                }
              }

              if (ControllerHandle != NULL && (*HandleBuffer)[HandleIndex] == ControllerHandle) {
                if ((
                      OpenInfo[OpenInfoIndex].Attributes & EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER
                  ) == EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER
                    ) {
                  for (ChildIndex = 0; ChildIndex < *HandleCount; ChildIndex++) {
                    if ((*HandleBuffer)[ChildIndex] == OpenInfo[OpenInfoIndex].ControllerHandle) {
                      (*HandleType)[ChildIndex] |= (EFI_HANDLE_TYPE_DEVICE_HANDLE | EFI_HANDLE_TYPE_CHILD_HANDLE);
                    }
                  }
                }
              }
            }

            if (DriverBindingHandle == NULL && OpenInfo[OpenInfoIndex].ControllerHandle == ControllerHandle) {
              if ((OpenInfo[OpenInfoIndex].Attributes & EFI_OPEN_PROTOCOL_BY_DRIVER) == EFI_OPEN_PROTOCOL_BY_DRIVER) {
                for (ChildIndex = 0; ChildIndex < *HandleCount; ChildIndex++) {
                  if ((*HandleBuffer)[ChildIndex] == OpenInfo[OpenInfoIndex].AgentHandle) {
                    (*HandleType)[ChildIndex] |= EFI_HANDLE_TYPE_DEVICE_DRIVER;
                  }
                }
              }

              if ((
                    OpenInfo[OpenInfoIndex].Attributes & EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER
                ) == EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER
                  ) {
                (*HandleType)[HandleIndex] |= EFI_HANDLE_TYPE_PARENT_HANDLE;
                for (ChildIndex = 0; ChildIndex < *HandleCount; ChildIndex++) {
                  if ((*HandleBuffer)[ChildIndex] == OpenInfo[OpenInfoIndex].AgentHandle) {
                    (*HandleType)[ChildIndex] |= EFI_HANDLE_TYPE_BUS_DRIVER;
                  }
                }
              }
            }
          }

          FreePool (OpenInfo);
        }
      }

      FreePool (ProtocolGuidArray);
    }
  }

  return EFI_SUCCESS;

Error:
  if (*HandleType != NULL) {
    FreePool (*HandleType);
  }

  if (*HandleBuffer != NULL) {
    FreePool (*HandleBuffer);
  }

  *HandleCount  = 0;
  *HandleBuffer = NULL;
  *HandleType   = NULL;

  return Status;
}

EFI_STATUS
LibGetHandleDatabaseSubset (
  EFI_HANDLE  DriverBindingHandle,
  EFI_HANDLE  ControllerHandle,
  UINT32      Mask,
  UINTN       *MatchingHandleCount,
  EFI_HANDLE  **MatchingHandleBuffer
  )

{
  EFI_STATUS  Status;
  UINTN       HandleCount;
  EFI_HANDLE  *HandleBuffer;
  UINT32      *HandleType;
  UINTN       HandleIndex;

  ASSERT (MatchingHandleCount != NULL);

  *MatchingHandleCount = 0;
  if (MatchingHandleBuffer != NULL) {
    *MatchingHandleBuffer = NULL;
  }

  HandleBuffer  = NULL;
  HandleType    = NULL;

  Status = LibScanHandleDatabase (
            DriverBindingHandle,
            NULL,
            ControllerHandle,
            NULL,
            &HandleCount,
            &HandleBuffer,
            &HandleType
            );
  if (EFI_ERROR (Status)) {
    goto Done;
  }
  //
  // Count the number of handles that match the attributes in Mask
  //
  for (HandleIndex = 0; HandleIndex < HandleCount; HandleIndex++) {
    if ((HandleType[HandleIndex] & Mask) == Mask) {
      (*MatchingHandleCount)++;
    }
  }
  //
  // If no handles match the attributes in Mask then return EFI_NOT_FOUND
  //
  if (*MatchingHandleCount == 0) {
    Status = EFI_NOT_FOUND;
    goto Done;
  }

  if (MatchingHandleBuffer == NULL) {
    Status = EFI_SUCCESS;
    goto Done;
  }
  //
  // Allocate a handle buffer for the number of handles that matched the attributes in Mask
  //
  *MatchingHandleBuffer = AllocatePool (*MatchingHandleCount * sizeof (EFI_HANDLE));
  if (*MatchingHandleBuffer == NULL) {
    goto Done;
  }
  //
  // Fill the allocated buffer with the handles that matched the attributes in Mask
  //
  *MatchingHandleCount = 0;
  for (HandleIndex = 0; HandleIndex < HandleCount; HandleIndex++) {
    if ((HandleType[HandleIndex] & Mask) == Mask) {
      (*MatchingHandleBuffer)[(*MatchingHandleCount)++] = HandleBuffer[HandleIndex];
    }
  }

  Status = EFI_SUCCESS;

Done:
  //
  // Free the buffers alocated by LibScanHandleDatabase()
  //
  if (HandleBuffer != NULL) {
    FreePool (HandleBuffer);
  }

  if (HandleType != NULL) {
    FreePool (HandleType);
  }

  return Status;
}

EFI_STATUS
LibGetManagingDriverBindingHandles (
  EFI_HANDLE  ControllerHandle,
  UINTN       *DriverBindingHandleCount,
  EFI_HANDLE  **DriverBindingHandleBuffer
  )

{
  return LibGetHandleDatabaseSubset (
          NULL,
          ControllerHandle,
          EFI_HANDLE_TYPE_DRIVER_BINDING_HANDLE | EFI_HANDLE_TYPE_DEVICE_DRIVER,
          DriverBindingHandleCount,
          DriverBindingHandleBuffer
          );
}

EFI_STATUS
LibGetParentControllerHandles (
  EFI_HANDLE  ControllerHandle,
  UINTN       *ParentControllerHandleCount,
  EFI_HANDLE  **ParentControllerHandleBuffer
  )

{
  return LibGetHandleDatabaseSubset (
          NULL,
          ControllerHandle,
          EFI_HANDLE_TYPE_PARENT_HANDLE,
          ParentControllerHandleCount,
          ParentControllerHandleBuffer
          );
}

EFI_STATUS
LibGetManagedChildControllerHandles (
  EFI_HANDLE  DriverBindingHandle,
  EFI_HANDLE  ControllerHandle,
  UINTN       *ChildControllerHandleCount,
  EFI_HANDLE  **ChildControllerHandleBuffer
  )

{
  return LibGetHandleDatabaseSubset (
          DriverBindingHandle,
          ControllerHandle,
          EFI_HANDLE_TYPE_CHILD_HANDLE | EFI_HANDLE_TYPE_DEVICE_HANDLE,
          ChildControllerHandleCount,
          ChildControllerHandleBuffer
          );
}

EFI_STATUS
LibGetManagedControllerHandles (
  EFI_HANDLE  DriverBindingHandle,
  UINTN       *ControllerHandleCount,
  EFI_HANDLE  **ControllerHandleBuffer
  )

{
  return LibGetHandleDatabaseSubset (
          DriverBindingHandle,
          NULL,
          EFI_HANDLE_TYPE_CONTROLLER_HANDLE | EFI_HANDLE_TYPE_DEVICE_HANDLE,
          ControllerHandleCount,
          ControllerHandleBuffer
          );
}

EFI_STATUS
LibGetChildControllerHandles (
  EFI_HANDLE  ControllerHandle,
  UINTN       *HandleCount,
  EFI_HANDLE  **HandleBuffer
  )

{
  EFI_STATUS  Status;
  UINTN       HandleIndex;
  UINTN       DriverBindingHandleCount;
  EFI_HANDLE  *DriverBindingHandleBuffer;
  UINTN       DriverBindingHandleIndex;
  UINTN       ChildControllerHandleCount;
  EFI_HANDLE  *ChildControllerHandleBuffer;
  UINTN       ChildControllerHandleIndex;
  BOOLEAN     Found;
  
  ASSERT (HandleCount != NULL);
  ASSERT (HandleBuffer != NULL);

  *HandleCount  = 0;
  *HandleBuffer = NULL;

  Status = LibGetManagingDriverBindingHandles (
            ControllerHandle,
            &DriverBindingHandleCount,
            &DriverBindingHandleBuffer
            );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = BS->LocateHandleBuffer (
                AllHandles,
                NULL,
                NULL,
                HandleCount,
                HandleBuffer
                );
  if (EFI_ERROR (Status)) {
    FreePool (DriverBindingHandleBuffer);
    return Status;
  }

  *HandleCount = 0;
  for (DriverBindingHandleIndex = 0; DriverBindingHandleIndex < DriverBindingHandleCount; DriverBindingHandleIndex++) {
    Status = LibGetManagedChildControllerHandles (
              DriverBindingHandleBuffer[DriverBindingHandleIndex],
              ControllerHandle,
              &ChildControllerHandleCount,
              &ChildControllerHandleBuffer
              );
    if (EFI_ERROR (Status)) {
      continue;
    }

    for (ChildControllerHandleIndex = 0;
         ChildControllerHandleIndex < ChildControllerHandleCount;
         ChildControllerHandleIndex++
        ) {
      Found = FALSE;
      for (HandleIndex = 0; HandleIndex < *HandleCount; HandleIndex++) {
        if ((*HandleBuffer)[HandleIndex] == ChildControllerHandleBuffer[ChildControllerHandleIndex]) {
          Found = TRUE;
        }
      }

      if (!Found) {
        (*HandleBuffer)[(*HandleCount)++] = ChildControllerHandleBuffer[ChildControllerHandleIndex];
      }
    }

    FreePool (ChildControllerHandleBuffer);
  }

  FreePool (DriverBindingHandleBuffer);

  return EFI_SUCCESS;
}

EFI_STATUS
LibLocateProtocol (
  IN  EFI_GUID    *ProtocolGuid,
  OUT VOID        **Interface
  )
/*++

Routine Description:

  Find the first instance of this Protocol in the system and return it's interface

Arguments:

  ProtocolGuid    - Provides the protocol to search for
  Interface       - On return, a pointer to the first interface that matches ProtocolGuid

Returns:

  EFI_SUCCESS     - A protocol instance matching ProtocolGuid was found

  EFI_NOT_FOUND   - No protocol instances were found that match ProtocolGuid

--*/
{
  EFI_STATUS  Status;
  UINTN       NumberHandles;
  UINTN       Index;
  EFI_HANDLE  *Handles;

  *Interface    = NULL;
  Handles       = NULL;
  NumberHandles = 0;

  Status        = LibLocateHandle (ByProtocol, ProtocolGuid, NULL, &NumberHandles, &Handles);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_INFO, "LibLocateProtocol: Handle not found\n"));
    return Status;
  }

  for (Index = 0; Index < NumberHandles; Index++) {
    Status = BS->HandleProtocol (
                  Handles[Index],
                  ProtocolGuid,
                  Interface
                  );

    if (!EFI_ERROR (Status)) {
      break;
    }
  }

  if (Handles) {
    FreePool (Handles);
  }

  return Status;
}

