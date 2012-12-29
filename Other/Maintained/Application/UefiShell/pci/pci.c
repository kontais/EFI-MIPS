/*++

Copyright (c) 2005 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  pci.c
  
Abstract: 
  EFI shell command "pci"

Revision History

--*/

#include "EfiShellLib.h"
#include "pci22.h"
#include "pci_class.h"
#include "pci.h"
#include "Acpi.h"

extern UINT8      STRING_ARRAY_NAME[];

//
// This is the generated header file which includes whatever needs to be exported (strings + IFR)
//
#include STRING_DEFINES_FILE

#include EFI_PROTOCOL_DEFINITION (PciRootBridgeIo)

EFI_STATUS
PciDump (
  IN EFI_HANDLE                             ImageHandle,
  IN EFI_SYSTEM_TABLE                       *SystemTable
  );

EFI_STATUS
PciFindProtocolInterface (
  IN  EFI_HANDLE                            *HandleBuf,
  IN  UINTN                                 HandleCount,
  IN  UINT16                                Segment,
  IN  UINT16                                Bus,
  OUT EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL       **IoDev
  );

EFI_STATUS
PciGetProtocolAndResource (
  IN  EFI_HANDLE                            Handle,
  OUT EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL       **IoDev,
  OUT EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR     **Descriptors
  );

EFI_STATUS
PciGetNextBusRange (
  IN OUT EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR  **Descriptors,
  OUT    UINT16                             *MinBus,
  OUT    UINT16                             *MaxBus,
  OUT    BOOLEAN                            *IsEnd
  );

EFI_STATUS
PciExplainData (
  IN PCI_CONFIG_SPACE                       *ConfigSpace,
  IN UINT64                                 Address,
  IN EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL        *IoDev
  );

EFI_STATUS
PciExplainDeviceData (
  IN PCI_DEVICE_HEADER                      *Device,
  IN UINT64                                 Address,
  IN EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL        *IoDev
  );

EFI_STATUS
PciExplainBridgeData (
  IN PCI_BRIDGE_HEADER                      *Bridge,
  IN UINT64                                 Address,
  IN EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL        *IoDev
  );

EFI_STATUS
PciExplainBar (
  IN UINT32                                 *Bar,
  IN UINT16                                 *Command,
  IN UINT64                                 Address,
  IN EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL        *IoDev,
  IN OUT UINTN                              *Index
  );

EFI_STATUS
PciExplainCardBusData (
  IN PCI_CARDBUS_HEADER                     *CardBus,
  IN UINT64                                 Address,
  IN EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL        *IoDev
  );

EFI_STATUS
PciExplainStatus (
  IN UINT16                                 *Status,
  IN BOOLEAN                                MainStatus,
  IN PCI_HEADER_TYPE                        HeaderType
  );

EFI_STATUS
PciExplainCommand (
  IN UINT16                                 *Command
  );

EFI_STATUS
PciExplainBridgeControl (
  IN UINT16                                 *BridgeControl,
  IN PCI_HEADER_TYPE                        HeaderType
  );

EFI_STATUS
PciExplainCapabilityStruct (
  IN  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL         *IoDev,
  IN UINT64                                   Address,
  IN  UINT8                                   CapPtr
  );

EFI_STATUS
PciExplainPciExpress (
  IN  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL         *IoDev,
  IN  UINT64                                  Address,
  IN  UINT8                                   CapabilityPtr
  );

//
// Global Variables
//
PCI_CONFIG_SPACE  *mConfigSpace;
EFI_HII_HANDLE    HiiHandle;
EFI_GUID          EfiPciGuid = EFI_PCI_GUID;
SHELL_VAR_CHECK_ITEM    PciCheckList[] = {
  {
    L"-s",
    0x01,
    0,
    FlagTypeNeedVar
  },
  {
    L"-i",
    0x02,
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

EFI_BOOTSHELL_CODE(
  EFI_APPLICATION_ENTRY_POINT(PciDump)
)

EFI_STATUS
PciDump (
  IN EFI_HANDLE                             ImageHandle,
  IN EFI_SYSTEM_TABLE                       *SystemTable
  )
/*++

Routine Description:

  Command entry point. Parses command line arguments and execute it. If 
  needed, calls internal function to perform certain operation.

Arguments:

  ImageHandle     The image handle.
  SystemTable     The system table.

Returns:

  EFI_SUCCESS           - The command completed successfully
  EFI_INVALID_PARAMETER - Invalid parameter
  EFI_OUT_OF_RESOURCES  - Out of resources

--*/
{
  UINT16                            Segment;
  UINT16                            Bus;
  UINT16                            Device;
  UINT16                            Func;
  UINT64                            Address;
  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL   *IoDev;
  EFI_STATUS                        Status;
  PCI_COMMON_HEADER                 PciHeader;
  PCI_CONFIG_SPACE                  ConfigSpace;
  UINTN                             ScreenCount;
  UINTN                             TempColumn;
  UINTN                             ScreenSize;
  BOOLEAN                           ExplainData;
  UINTN                             Index;
  UINTN                             SizeOfHeader;
  BOOLEAN                           PrintTitle;
  UINTN                             HandleBufSize;
  EFI_HANDLE                        *HandleBuf;
  UINTN                             HandleCount;
  EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR *Descriptors;
  UINT16                            MinBus;
  UINT16                            MaxBus;
  BOOLEAN                           IsEnd;

  SHELL_VAR_CHECK_CODE              RetCode;
  CHAR16                            *Useful;
  SHELL_ARG_LIST                    *Item;
  SHELL_VAR_CHECK_PACKAGE           ChkPck;

  IoDev = NULL;
  HandleBuf = NULL;
  ZeroMem (&ChkPck, sizeof (SHELL_VAR_CHECK_PACKAGE));
  EFI_SHELL_APP_INIT (ImageHandle, SystemTable);
  
  //
  // Enable tab key which can pause the output
  //
  EnableOutputTabPause();

  Status = LibInitializeStrings (&HiiHandle, STRING_ARRAY_NAME, &EfiPciGuid);

  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (!EFI_PROPER_VERSION (0, 99)) {
    PrintToken (
      STRING_TOKEN (STR_SHELLENV_GNC_COMMAND_NOT_SUPPORT),
      HiiHandle,
      L"pci",
      EFI_VERSION_0_99 
      );
    Status = EFI_UNSUPPORTED;
    goto Done;
  }

  RetCode = LibCheckVariables (SI, PciCheckList, &ChkPck, &Useful);
  if (VarCheckOk != RetCode) {
    switch (RetCode) {
    case VarCheckUnknown:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_UNKNOWN_FLAG), HiiHandle, L"pci", Useful);
      break;

    case VarCheckDuplicate:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_DUP_FLAG), HiiHandle, L"pci", Useful);
      break;

    case VarCheckLackValue:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_LACK_ARG), HiiHandle, L"pci", Useful);
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
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiHandle, L"pci");
      Status = EFI_INVALID_PARAMETER;
    } else {
      PrintToken (STRING_TOKEN (STR_HELPINFO_PCI_VERBOSEHELP), HiiHandle);
      Status = EFI_SUCCESS;
    }

    goto Done;
  }
  //
  // Get all instances of PciRootBridgeIo. Allocate space for 1 EFI_HANDLE and
  // call LibLocateHandle(), if EFI_BUFFER_TOO_SMALL is returned, allocate enough
  // space for handles and call it again.
  //
  HandleBufSize = sizeof (EFI_HANDLE);
  HandleBuf     = (EFI_HANDLE *) AllocatePool (HandleBufSize);
  if (HandleBuf == NULL) {
    PrintToken (STRING_TOKEN (STR_PCI2_OUT_RESOURCES), HiiHandle);
    Status = EFI_OUT_OF_RESOURCES;
    goto Done;
  }

  Status = BS->LocateHandle (
                ByProtocol,
                &gEfiPciRootBridgeIoProtocolGuid,
                NULL,
                &HandleBufSize,
                HandleBuf
                );

  if (Status == EFI_BUFFER_TOO_SMALL) {
    HandleBuf = ReallocatePool (HandleBuf, sizeof (EFI_HANDLE), HandleBufSize);
    if (HandleBuf == NULL) {
      PrintToken (STRING_TOKEN (STR_PCI2_OUT_RESOURCES), HiiHandle);
      Status = EFI_OUT_OF_RESOURCES;
      goto Done;
    }

    Status = BS->LocateHandle (
                  ByProtocol,
                  &gEfiPciRootBridgeIoProtocolGuid,
                  NULL,
                  &HandleBufSize,
                  HandleBuf
                  );
  }

  if (EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_PCI2_LOCATE_HANDLE), HiiHandle, Status);
    goto Done;
  }

  HandleCount = HandleBufSize / sizeof (EFI_HANDLE);
  //
  // Argument Count == 1(no other argument): enumerate all pci functions
  //
  if (ChkPck.ValueCount == 0) {
    ST->ConOut->QueryMode (
                  ST->ConOut,
                  ST->ConOut->Mode->Mode,
                  &TempColumn,
                  &ScreenSize
                  );

    ScreenCount = 0;
    ScreenSize -= 4;
    if ((ScreenSize & 1) == 1) {
      ScreenSize -= 1;
    }

    PrintTitle = TRUE;

    //
    // For each handle, which decides a segment and a bus number range,
    // enumerate all devices on it.
    //
    for (Index = 0; Index < HandleCount; Index++) {
      Status = PciGetProtocolAndResource (
                HandleBuf[Index],
                &IoDev,
                &Descriptors
                );
      if (EFI_ERROR (Status)) {
        PrintToken (STRING_TOKEN (STR_PCI2_HANDLE_PROTOCOL), HiiHandle, Status);
        goto Done;
      }
      //
      // No document say it's impossible for a RootBridgeIo protocol handle
      // to have more than one address space descriptors, so find out every
      // bus range and for each of them do device enumeration.
      //
      while (TRUE) {
        Status = PciGetNextBusRange (&Descriptors, &MinBus, &MaxBus, &IsEnd);

        if (EFI_ERROR (Status)) {
          PrintToken (STRING_TOKEN (STR_PCI2_BUS), HiiHandle, Status);
          goto Done;
        }

        if (IsEnd) {
          break;
        }

        for (Bus = MinBus; Bus <= MaxBus; Bus++) {
          //
          // For each devices, enumerate all functions it contains
          //
          for (Device = 0; Device <= PCI_MAX_DEVICE; Device++) {
            //
            // For each function, read its configuration space and print summary
            //
            for (Func = 0; Func <= PCI_MAX_FUNC; Func++) {
              Address = CALC_EFI_PCI_ADDRESS (Bus, Device, Func, 0);
              IoDev->Pci.Read (
                          IoDev,
                          EfiPciWidthUint16,
                          Address,
                          1,
                          &PciHeader.VendorId
                          );

              //
              // If VendorId = 0xffff, there does not exist a device at this
              // location. For each device, if there is any function on it,
              // there must be 1 function at Function 0. So if Func = 0, there
              // will be no more functions in the same device, so we can break
              // loop to deal with the next device.
              //
              if (PciHeader.VendorId == 0xffff && Func == 0) {
                break;
              }

              if (PciHeader.VendorId != 0xffff) {

                if (PrintTitle) {
                  PrintToken (STRING_TOKEN (STR_PCI2_SEG_BUS), HiiHandle);
                  Print (L"   ---  ---  ---  ----\n");
                  PrintTitle = FALSE;
                }

                IoDev->Pci.Read (
                            IoDev,
                            EfiPciWidthUint32,
                            Address,
                            sizeof (PciHeader) / sizeof (UINT32),
                            &PciHeader
                            );

                PrintToken (
                  STRING_TOKEN (STR_PCI2_FOUR_VARS),
                  HiiHandle,
                  IoDev->SegmentNumber,
                  Bus,
                  Device,
                  Func
                  );

                PciPrintClassCode (PciHeader.ClassCode, FALSE);
                PrintToken (
                  STRING_TOKEN (STR_PCI2_VENDOR),
                  HiiHandle,
                  PciHeader.VendorId,
                  PciHeader.DeviceId,
                  PciHeader.ClassCode[0]
                  );

                ScreenCount += 2;
                if (ScreenCount >= ScreenSize && ScreenSize != 0) {
                  //
                  // If ScreenSize == 0 we have the console redirected so don't
                  //  block updates
                  //
                  ScreenCount = 0;
                }
                //
                // If this is not a multi-function device, we can leave the loop
                // to deal with the next device.
                //
                if (Func == 0 && ((PciHeader.HeaderType & HEADER_TYPE_MULTI_FUNCTION) == 0x00)) {
                  break;
                }
              }
            }
          }
        }
        //
        // If Descriptor is NULL, Configuration() returns EFI_UNSUPPRORED,
        // we assume the bus range is 0~PCI_MAX_BUS. After enumerated all
        // devices on all bus, we can leave loop.
        //
        if (Descriptors == NULL) {
          break;
        }
      }
    }

    Status = EFI_SUCCESS;
    goto Done;
  }

  if (ChkPck.ValueCount == 1) {
    PrintToken (STRING_TOKEN (STR_PCI2_TOO_FEW_ARGS), HiiHandle);
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }
  //
  // Arg count >= 3, dump binary of specified function, interpret if necessary
  //
  if (ChkPck.ValueCount > 3) {
    PrintToken (STRING_TOKEN (STR_PCI2_TOO_MANY_ARGS), HiiHandle);
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  ExplainData                   = FALSE;
  Segment                       = 0;
  Bus                           = 0;
  Device                        = 0;
  Func                          = 0;
  if (LibCheckVarGetFlag (&ChkPck, L"-i") != NULL) {
    ExplainData = TRUE;
  }

  Item = LibCheckVarGetFlag (&ChkPck, L"-s");
  if (NULL != Item) {
    Segment = (UINT16) StrToUIntegerBase (Item->VarStr, 16, &Status);
    if (EFI_ERROR (Status)) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_ARG), HiiHandle, L"pci", Item->VarStr);
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }
  }

  //
  // The first Argument(except "-i") is assumed to be Bus number, second
  // to be Device number, and third to be Func number.
  //
  Item = ChkPck.VarList;
  if (NULL != Item) {
    Bus = (UINT16) StrToUIntegerBase (Item->VarStr, 16, &Status);
    if (EFI_ERROR (Status) || Bus > MAX_BUS_NUMBER) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_ARG), HiiHandle, L"pci", Item->VarStr);
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }

    Item = Item->Next;
  }

  if (NULL != Item) {
    Device = (UINT16) StrToUIntegerBase (Item->VarStr, 16, &Status);
    if (EFI_ERROR (Status) || Device > MAX_DEVICE_NUMBER) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_ARG), HiiHandle, L"pci", Item->VarStr);
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }

    Item = Item->Next;
  }

  if (NULL != Item) {
    Func = (UINT16) StrToUIntegerBase (Item->VarStr, 16, &Status);
    if (EFI_ERROR (Status) || Func > MAX_FUNCTION_NUMBER) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_ARG), HiiHandle, L"pci", Item->VarStr);
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }
  }


  //
  // Find the protocol interface who's in charge of current segment, and its
  // bus range covers the current bus
  //
  Status = PciFindProtocolInterface (
            HandleBuf,
            HandleCount,
            Segment,
            Bus,
            &IoDev
            );

  if (EFI_ERROR (Status)) {
    PrintToken (
      STRING_TOKEN (STR_PCI2_CANNOT_FIND_PROTOCOL),
      HiiHandle,
      Segment,
      Bus
      );

    goto Done;
  }

  Address = CALC_EFI_PCI_ADDRESS (Bus, Device, Func, 0);
  Status = IoDev->Pci.Read (
                        IoDev,
                        EfiPciWidthUint8,
                        Address,
                        sizeof (ConfigSpace),
                        &ConfigSpace
                        );

  if (EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_PCI2_CANNOT_READ_CONFIG), HiiHandle, Status);
    goto Done;
  }

  mConfigSpace = &ConfigSpace;
  PrintToken (
    STRING_TOKEN (STR_PCI2_SEGMENT_BUS),
    HiiHandle,
    Segment,
    Bus,
    Device,
    Func,
    Segment,
    Bus,
    Device,
    Func
    );

  //
  // Dump standard header of configuration space
  //
  SizeOfHeader = sizeof (ConfigSpace.Common) + sizeof (ConfigSpace.NonCommon);

  DumpHex (2, 0, SizeOfHeader, &ConfigSpace);
  Print (L"\n");

  //
  // Dump device dependent Part of configuration space
  //
  DumpHex (
    2,
    SizeOfHeader,
    sizeof (ConfigSpace) - SizeOfHeader,
    ConfigSpace.Data
    );

  //
  // If "-i" appears in command line, interpret data in configuration space
  //
  if (ExplainData) {
    Status = PciExplainData (&ConfigSpace, Address, IoDev);
  }
Done:
  if (HandleBuf != NULL) {
    FreePool (HandleBuf);
  }
  LibCheckVarFreeVarList (&ChkPck);
  LibUnInitializeStrings ();
  return Status;
}

EFI_STATUS
PciFindProtocolInterface (
  IN  EFI_HANDLE                            *HandleBuf,
  IN  UINTN                                 HandleCount,
  IN  UINT16                                Segment,
  IN  UINT16                                Bus,
  OUT EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL       **IoDev
  )
/*++

Routine Description:

  This function finds out the protocol which is in charge of the given 
  segment, and its bus range covers the current bus number. It lookes 
  each instances of RootBridgeIoProtocol handle, until the one meets the
  criteria is found. 

Arguments:

  HandleBuf       Buffer which holds all PCI_ROOT_BRIDIGE_IO_PROTOCOL handles
  HandleCount     Count of all PCI_ROOT_BRIDIGE_IO_PROTOCOL handles
  Segment         Segment number of device we are dealing with
  Bus             Bus number of device we are dealing with
  IoDev           Handle used to access configuration space of PCI device

Returns:

  EFI_SUCCESS           - The command completed successfully
  EFI_INVALID_PARAMETER - Invalid parameter

--*/
{
  UINTN                             Index;
  EFI_STATUS                        Status;
  BOOLEAN                           FoundInterface;
  EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR *Descriptors;
  UINT16                            MinBus;
  UINT16                            MaxBus;
  BOOLEAN                           IsEnd;

  FoundInterface = FALSE;
  //
  // Go through all handles, until the one meets the criteria is found
  //
  for (Index = 0; Index < HandleCount; Index++) {
    Status = PciGetProtocolAndResource (HandleBuf[Index], IoDev, &Descriptors);
    if (EFI_ERROR (Status)) {
      return Status;
    }
    //
    // When Descriptors == NULL, the Configuration() is not implemented,
    // so we only check the Segment number
    //
    if (Descriptors == NULL && Segment == (*IoDev)->SegmentNumber) {
      return EFI_SUCCESS;
    }

    if ((*IoDev)->SegmentNumber != Segment) {
      continue;
    }

    while (TRUE) {
      Status = PciGetNextBusRange (&Descriptors, &MinBus, &MaxBus, &IsEnd);
      if (EFI_ERROR (Status)) {
        return Status;
      }

      if (IsEnd) {
        break;
      }

      if (MinBus <= Bus && MaxBus >= Bus) {
        FoundInterface = TRUE;
        break;
      }
    }
  }

  if (FoundInterface) {
    return EFI_SUCCESS;
  } else {
    return EFI_INVALID_PARAMETER;
  }
}

EFI_STATUS
PciGetProtocolAndResource (
  IN  EFI_HANDLE                            Handle,
  OUT EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL       **IoDev,
  OUT EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR     **Descriptors
  )
/*++

Routine Description:

  This function gets the protocol interface from the given handle, and
  obtains its address space descriptors.

Arguments:

  Handle          The PCI_ROOT_BRIDIGE_IO_PROTOCOL handle
  IoDev           Handle used to access configuration space of PCI device
  Descriptors     Points to the address space descriptors

Returns:

  EFI_SUCCESS     The command completed successfully

--*/
{
  EFI_STATUS  Status;

  //
  // Get inferface from protocol
  //
  Status = BS->HandleProtocol (
                Handle,
                &gEfiPciRootBridgeIoProtocolGuid,
                IoDev
                );

  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Call Configuration() to get address space descriptors
  //
  Status = (*IoDev)->Configuration (*IoDev, Descriptors);
  if (Status == EFI_UNSUPPORTED) {
    *Descriptors = NULL;
    return EFI_SUCCESS;

  } else {
    return Status;
  }
}

EFI_STATUS
PciGetNextBusRange (
  IN OUT EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR  **Descriptors,
  OUT    UINT16                             *MinBus,
  OUT    UINT16                             *MaxBus,
  OUT    BOOLEAN                            *IsEnd
  )
/*++

Routine Description:

  This function get the next bus range of given address space descriptors.
  It also moves the pointer backward a node, to get prepared to be called
  again.

Arguments:

  Descriptors     points to current position of a serial of address space 
                  descriptors
  MinBus          The lower range of bus number
  ManBus          The upper range of bus number
  IsEnd           Meet end of the serial of descriptors 
  
Returns:

  EFI_SUCCESS     The command completed successfully

--*/
{
  *IsEnd = FALSE;

  //
  // When *Descriptors is NULL, Configuration() is not implemented, so assume
  // range is 0~PCI_MAX_BUS
  //
  if ((*Descriptors) == NULL) {
    *MinBus = 0;
    *MaxBus = PCI_MAX_BUS;
    return EFI_SUCCESS;
  }
  //
  // *Descriptors points to one or more address space descriptors, which
  // ends with a end tagged descriptor. Examine each of the descriptors,
  // if a bus typed one is found and its bus range covers bus, this handle
  // is the handle we are looking for.
  //
  if ((*Descriptors)->Desc == ACPI_END_TAG_DESCRIPTOR) {
    *IsEnd = TRUE;
  }

  while ((*Descriptors)->Desc != ACPI_END_TAG_DESCRIPTOR) {
    if ((*Descriptors)->ResType == ACPI_ADDRESS_SPACE_TYPE_BUS) {
      *MinBus = (UINT16) (*Descriptors)->AddrRangeMin;
      *MaxBus = (UINT16) (*Descriptors)->AddrRangeMax;
    }

    (*Descriptors)++;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
PciExplainData (
  IN PCI_CONFIG_SPACE                       *ConfigSpace,
  IN UINT64                                 Address,
  IN EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL        *IoDev
  )
/*++

Routine Description:

  Explain the data in PCI configuration space. The part which is common for
  PCI device and bridge is interpreted in this function. It calls other 
  functions to interpret data unique for device or bridge.

Arguments:

  ConfigSpace     Data in PCI configuration space
  Address         Address used to access configuration space of this PCI device
  IoDev           Handle used to access configuration space of PCI device

Returns:

  EFI_SUCCESS     The command completed successfully

--*/
{
  PCI_COMMON_HEADER *Common;
  PCI_HEADER_TYPE   HeaderType;
  EFI_STATUS        Status;
  UINT8             CapPtr;

  Common = &(ConfigSpace->Common);

  Print (L"\n");

  //
  // Print Vendor Id and Device Id
  //
  PrintToken (
    STRING_TOKEN (STR_PCI2_VENDOR_ID),
    HiiHandle,
    INDEX_OF (&(Common->VendorId)),
    Common->VendorId
    );

  PrintToken (
    STRING_TOKEN (STR_PCI2_DEVICE_ID),
    HiiHandle,
    INDEX_OF (&(Common->DeviceId)),
    Common->DeviceId
    );

  //
  // Print register Command
  //
  PciExplainCommand (&(Common->Command));

  //
  // Print register Status
  //
  PciExplainStatus (&(Common->Status), TRUE, PciUndefined);

  //
  // Print register Revision ID
  //
  PrintToken (
    STRING_TOKEN (STR_PCI2_REVISION_ID),
    HiiHandle,
    INDEX_OF (&(Common->RevisionId)),
    Common->RevisionId
    );

  //
  // Print register BIST
  //
  PrintToken (STRING_TOKEN (STR_PCI2_BIST), HiiHandle, INDEX_OF (&(Common->BIST)));
  if ((Common->BIST & PCI_BIT_7) != 0) {
    PrintToken (STRING_TOKEN (STR_PCI2_CAPABLE_RETURN), HiiHandle, 0x0f & Common->BIST);

  } else {
    PrintToken (STRING_TOKEN (STR_PCI2_INCAPABLE), HiiHandle);
  }
  //
  // Print register Cache Line Size
  //
  PrintToken (
    STRING_TOKEN (STR_PCI2_CACHE_LINE_SIZE),
    HiiHandle,
    INDEX_OF (&(Common->CacheLineSize)),
    Common->CacheLineSize
    );

  //
  // Print register Latency Timer
  //
  PrintToken (
    STRING_TOKEN (STR_PCI2_LATENCY_TIMER),
    HiiHandle,
    INDEX_OF (&(Common->PrimaryLatencyTimer)),
    Common->PrimaryLatencyTimer
    );

  //
  // Print register Header Type
  //
  PrintToken (
    STRING_TOKEN (STR_PCI2_HEADER_TYPE),
    HiiHandle,
    INDEX_OF (&(Common->HeaderType)),
    Common->HeaderType
    );

  if ((Common->HeaderType & PCI_BIT_7) != 0) {
    PrintToken (STRING_TOKEN (STR_PCI2_MULTI_FUNCTION), HiiHandle);

  } else {
    PrintToken (STRING_TOKEN (STR_PCI2_SINGLE_FUNCTION), HiiHandle);
  }

  HeaderType = (UINT8) (Common->HeaderType & 0x7f);
  switch (HeaderType) {
  case PciDevice:
    PrintToken (STRING_TOKEN (STR_PCI2_PCI_DEVICE), HiiHandle);
    break;

  case PciP2pBridge:
    PrintToken (STRING_TOKEN (STR_PCI2_P2P_BRIDGE), HiiHandle);
    break;

  case PciCardBusBridge:
    PrintToken (STRING_TOKEN (STR_PCI2_CARDBUS_BRIDGE), HiiHandle);
    break;

  default:
    PrintToken (STRING_TOKEN (STR_PCI2_RESERVED), HiiHandle);
    HeaderType = PciUndefined;
  }

  //
  // Print register Class Code
  //
  PrintToken (STRING_TOKEN (STR_PCI2_CLASS), HiiHandle);
  PciPrintClassCode ((UINT8 *) Common->ClassCode, TRUE);
  Print (L"\n");

  //
  // Interpret remaining part of PCI configuration header depending on
  // HeaderType
  //
  CapPtr  = 0;
  Status  = EFI_SUCCESS;
  switch (HeaderType) {
  case PciDevice:
    Status = PciExplainDeviceData (
              &(ConfigSpace->NonCommon.Device),
              Address,
              IoDev
              );
    CapPtr = ConfigSpace->NonCommon.Device.CapabilitiesPtr;
    break;

  case PciP2pBridge:
    Status = PciExplainBridgeData (
              &(ConfigSpace->NonCommon.Bridge),
              Address,
              IoDev
              );
    CapPtr = ConfigSpace->NonCommon.Bridge.CapabilitiesPtr;
    break;

  case PciCardBusBridge:
    Status = PciExplainCardBusData (
              &(ConfigSpace->NonCommon.CardBus),
              Address,
              IoDev
              );
    CapPtr = ConfigSpace->NonCommon.CardBus.CapabilitiesPtr;
    break;
  }
  //
  // If Status bit4 is 1, dump or explain capability structure
  //
  if ((Common->Status) & EFI_PCI_STATUS_CAPABILITY) {
    PciExplainCapabilityStruct (IoDev, Address, CapPtr);
  }

  return Status;
}

EFI_STATUS
PciExplainDeviceData (
  IN PCI_DEVICE_HEADER                      *Device,
  IN UINT64                                 Address,
  IN EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL        *IoDev
  )
/*++

Routine Description:

  Explain the device specific part of data in PCI configuration space.

Arguments:

  Device          Data in PCI configuration space
  Address         Address used to access configuration space of this PCI device
  IoDev           Handle used to access configuration space of PCI device

Returns:

  EFI_SUCCESS     The command completed successfully

--*/
{
  UINTN       Index;
  BOOLEAN     BarExist;
  EFI_STATUS  Status;
  UINTN       BarCount;

  //
  // Print Base Address Registers(Bar). When Bar = 0, this Bar does not
  // exist. If these no Bar for this function, print "none", otherwise
  // list detail information about this Bar.
  //
  PrintToken (STRING_TOKEN (STR_PCI2_BASE_ADDR), HiiHandle, INDEX_OF (Device->Bar));

  BarExist  = FALSE;
  BarCount  = sizeof (Device->Bar) / sizeof (Device->Bar[0]);
  for (Index = 0; Index < BarCount; Index++) {
    if (Device->Bar[Index] == 0) {
      continue;
    }

    if (!BarExist) {
      BarExist = TRUE;
      PrintToken (STRING_TOKEN (STR_PCI2_START_TYPE), HiiHandle);
      Print (L"  --------------------------------------------------------------------------");
    }

    Status = PciExplainBar (
              &(Device->Bar[Index]),
              &(mConfigSpace->Common.Command),
              Address,
              IoDev,
              &Index
              );

    if (EFI_ERROR (Status)) {
      break;
    }
  }

  if (!BarExist) {
    PrintToken (STRING_TOKEN (STR_PCI2_NONE), HiiHandle);

  } else {
    Print (L"\n  --------------------------------------------------------------------------");
  }

  //
  // Print register Expansion ROM Base Address
  //
  if ((Device->ROMBar & PCI_BIT_0) == 0) {
    PrintToken (STRING_TOKEN (STR_PCI2_EXPANSION_ROM_DISABLED), HiiHandle, INDEX_OF (&(Device->ROMBar)));

  } else {
    PrintToken (
      STRING_TOKEN (STR_PCI2_EXPANSION_ROM_BASE),
      HiiHandle,
      INDEX_OF (&(Device->ROMBar)),
      Device->ROMBar
      );
  }
  //
  // Print register Cardbus CIS ptr
  //
  PrintToken (
    STRING_TOKEN (STR_PCI2_CARDBUS_CIS),
    HiiHandle,
    INDEX_OF (&(Device->CardBusCISPtr)),
    Device->CardBusCISPtr
    );

  //
  // Print register Sub-vendor ID and subsystem ID
  //
  PrintToken (
    STRING_TOKEN (STR_PCI2_SUB_VENDOR_ID),
    HiiHandle,
    INDEX_OF (&(Device->SubVendorId)),
    Device->SubVendorId
    );

  PrintToken (
    STRING_TOKEN (STR_PCI2_SUBSYSTEM_ID),
    HiiHandle,
    INDEX_OF (&(Device->SubSystemId)),
    Device->SubSystemId
    );

  //
  // Print register Capabilities Ptr
  //
  PrintToken (
    STRING_TOKEN (STR_PCI2_CAPABILITIES_PTR),
    HiiHandle,
    INDEX_OF (&(Device->CapabilitiesPtr)),
    Device->CapabilitiesPtr
    );

  //
  // Print register Interrupt Line and interrupt pin
  //
  PrintToken (
    STRING_TOKEN (STR_PCI2_INTERRUPT_LINE),
    HiiHandle,
    INDEX_OF (&(Device->InterruptLine)),
    Device->InterruptLine
    );

  PrintToken (
    STRING_TOKEN (STR_PCI2_INTERRUPT_PIN),
    HiiHandle,
    INDEX_OF (&(Device->InterruptPin)),
    Device->InterruptPin
    );

  //
  // Print register Min_Gnt and Max_Lat
  //
  PrintToken (
    STRING_TOKEN (STR_PCI2_MIN_GNT),
    HiiHandle,
    INDEX_OF (&(Device->MinGnt)),
    Device->MinGnt
    );

  PrintToken (
    STRING_TOKEN (STR_PCI2_MAX_LAT),
    HiiHandle,
    INDEX_OF (&(Device->MaxLat)),
    Device->MaxLat
    );

  return EFI_SUCCESS;
}

EFI_STATUS
PciExplainBridgeData (
  IN  PCI_BRIDGE_HEADER                     *Bridge,
  IN  UINT64                                Address,
  IN  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL       *IoDev
  )
/*++

Routine Description:

  Explain the bridge specific part of data in PCI configuration space.

Arguments:

  Bridge          Bridge specific data region in PCI configuration space
  Address         Address used to access configuration space of this PCI device
  IoDev           Handle used to access configuration space of PCI device

Returns:

  EFI_SUCCESS     The command completed successfully

--*/
{
  UINTN       Index;
  BOOLEAN     BarExist;
  UINTN       BarCount;
  UINT32      IoAddress32;
  EFI_STATUS  Status;

  //
  // Print Base Address Registers. When Bar = 0, this Bar does not
  // exist. If these no Bar for this function, print "none", otherwise
  // list detail information about this Bar.
  //
  PrintToken (STRING_TOKEN (STR_PCI2_BASE_ADDRESS), HiiHandle, INDEX_OF (&(Bridge->Bar)));

  BarExist  = FALSE;
  BarCount  = sizeof (Bridge->Bar) / sizeof (Bridge->Bar[0]);

  for (Index = 0; Index < BarCount; Index++) {
    if (Bridge->Bar[Index] == 0) {
      continue;
    }

    if (!BarExist) {
      BarExist = TRUE;
      PrintToken (STRING_TOKEN (STR_PCI2_START_TYPE_2), HiiHandle);
      Print (L"  --------------------------------------------------------------------------");
    }

    Status = PciExplainBar (
              &(Bridge->Bar[Index]),
              &(mConfigSpace->Common.Command),
              Address,
              IoDev,
              &Index
              );

    if (EFI_ERROR (Status)) {
      break;
    }
  }

  if (!BarExist) {
    PrintToken (STRING_TOKEN (STR_PCI2_NONE), HiiHandle);
  } else {
    Print (L"\n  --------------------------------------------------------------------------");
  }

  //
  // Expansion register ROM Base Address
  //
  if ((Bridge->ROMBar & PCI_BIT_0) == 0) {
    PrintToken (STRING_TOKEN (STR_PCI2_NO_EXPANSION_ROM), HiiHandle, INDEX_OF (&(Bridge->ROMBar)));

  } else {
    PrintToken (
      STRING_TOKEN (STR_PCI2_EXPANSION_ROM_BASE_2),
      HiiHandle,
      INDEX_OF (&(Bridge->ROMBar)),
      Bridge->ROMBar
      );
  }
  //
  // Print Bus Numbers(Primary, Secondary, and Subordinate
  //
  PrintToken (
    STRING_TOKEN (STR_PCI2_BUS_NUMBERS),
    HiiHandle,
    INDEX_OF (&(Bridge->PrimaryBus)),
    INDEX_OF (&(Bridge->SecondaryBus)),
    INDEX_OF (&(Bridge->SubordinateBus))
    );

  Print (L"               ------------------------------------------------------\n");

  PrintToken (STRING_TOKEN (STR_PCI2_BRIDGE), HiiHandle, Bridge->PrimaryBus);
  PrintToken (STRING_TOKEN (STR_PCI2_BRIDGE), HiiHandle, Bridge->SecondaryBus);
  PrintToken (STRING_TOKEN (STR_PCI2_BRIDGE), HiiHandle, Bridge->SubordinateBus);

  //
  // Print register Secondary Latency Timer
  //
  PrintToken (
    STRING_TOKEN (STR_PCI2_SECONDARY_TIMER),
    HiiHandle,
    INDEX_OF (&(Bridge->SecondaryLatencyTimer)),
    Bridge->SecondaryLatencyTimer
    );

  //
  // Print register Secondary Status
  //
  PciExplainStatus (&(Bridge->SecondaryStatus), FALSE, PciP2pBridge);

  //
  // Print I/O and memory ranges this bridge forwards. There are 3 resource
  // types: I/O, memory, and pre-fetchable memory. For each resource type,
  // base and limit address are listed.
  //
  PrintToken (STRING_TOKEN (STR_PCI2_RESOURCE_TYPE), HiiHandle);
  Print (L"----------------------------------------------------------------------\n");

  //
  // IO Base & Limit
  //
  IoAddress32 = (Bridge->IoBaseUpper << 16 | Bridge->IoBase << 8);
  IoAddress32 &= 0xfffff000;
  PrintToken (
    STRING_TOKEN (STR_PCI2_TWO_VARS),
    HiiHandle,
    INDEX_OF (&(Bridge->IoBase)),
    IoAddress32
    );

  IoAddress32 = (Bridge->IoLimitUpper << 16 | Bridge->IoLimit << 8);
  IoAddress32 |= 0x00000fff;
  PrintToken (STRING_TOKEN (STR_PCI2_ONE_VAR), HiiHandle, IoAddress32);

  //
  // Memory Base & Limit
  //
  PrintToken (
    STRING_TOKEN (STR_PCI2_MEMORY),
    HiiHandle,
    INDEX_OF (&(Bridge->MemoryBase)),
    (Bridge->MemoryBase << 16) & 0xfff00000
    );

  PrintToken (
    STRING_TOKEN (STR_PCI2_ONE_VAR),
    HiiHandle,
    (Bridge->MemoryLimit << 16) | 0x000fffff
    );

  //
  // Pre-fetch-able Memory Base & Limit
  //
  PrintToken (
    STRING_TOKEN (STR_PCI2_PREFETCHABLE),
    HiiHandle,
    INDEX_OF (&(Bridge->PrefetchableMemBase)),
    Bridge->PrefetchableBaseUpper,
    (Bridge->PrefetchableMemBase << 16) & 0xfff00000
    );

  PrintToken (
    STRING_TOKEN (STR_PCI2_TWO_VARS_2),
    HiiHandle,
    Bridge->PrefetchableLimitUpper,
    (Bridge->PrefetchableMemLimit << 16) | 0x000fffff
    );

  //
  // Print register Capabilities Pointer
  //
  PrintToken (
    STRING_TOKEN (STR_PCI2_CAPABILITIES_PTR_2),
    HiiHandle,
    INDEX_OF (&(Bridge->CapabilitiesPtr)),
    Bridge->CapabilitiesPtr
    );

  //
  // Print register Bridge Control
  //
  PciExplainBridgeControl (&(Bridge->BridgeControl), PciP2pBridge);

  //
  // Print register Interrupt Line & PIN
  //
  PrintToken (
    STRING_TOKEN (STR_PCI2_INTERRUPT_LINE_2),
    HiiHandle,
    INDEX_OF (&(Bridge->InterruptLine)),
    Bridge->InterruptLine
    );

  PrintToken (
    STRING_TOKEN (STR_PCI2_INTERRUPT_PIN),
    HiiHandle,
    INDEX_OF (&(Bridge->InterruptPin)),
    Bridge->InterruptPin
    );

  return EFI_SUCCESS;
}

EFI_STATUS
PciExplainBar (
  IN UINT32                                 *Bar,
  IN UINT16                                 *Command,
  IN UINT64                                 Address,
  IN EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL        *IoDev,
  IN OUT UINTN                              *Index
  )
/*++

Routine Description:

  Explain the Base Address Register(Bar) in PCI configuration space.

Arguments:

  Bar             Points to the Base Address Register intended to interpret
  Command         Points to the register Command
  Address         Address used to access configuration space of this PCI device
  IoDev           Handle used to access configuration space of PCI device
  Index           The Index

Returns:

  EFI_SUCCESS     The command completed successfully

--*/
{
  UINT16  OldCommand;
  UINT16  NewCommand;
  UINT64  Bar64;
  UINT32  OldBar32;
  UINT32  NewBar32;
  UINT64  OldBar64;
  UINT64  NewBar64;
  BOOLEAN IsMem;
  BOOLEAN IsBar32;
  UINT64  RegAddress;

  IsBar32   = TRUE;
  Bar64     = 0;
  NewBar32  = 0;
  NewBar64  = 0;

  //
  // According the bar type, list detail about this bar, for example: 32 or
  // 64 bits; pre-fetchable or not.
  //
  if ((*Bar & PCI_BIT_0) == 0) {
    //
    // This bar is of memory type
    //
    IsMem = TRUE;

    if ((*Bar & PCI_BIT_1) == 0 && (*Bar & PCI_BIT_2) == 0) {
      PrintToken (STRING_TOKEN (STR_PCI2_BAR), HiiHandle, *Bar & 0xfffffff0);
      PrintToken (STRING_TOKEN (STR_PCI2_MEM), HiiHandle);
      PrintToken (STRING_TOKEN (STR_PCI2_32_BITS), HiiHandle);

    } else if ((*Bar & PCI_BIT_1) == 0 && (*Bar & PCI_BIT_2) != 0) {
      Bar64 = 0x0;
      CopyMem (&Bar64, Bar, sizeof (UINT32));
      PrintToken (STRING_TOKEN (STR_PCI2_ONE_VAR_2), HiiHandle, RShiftU64 ((Bar64 & 0xfffffffffffffff0), 32));
      PrintToken (STRING_TOKEN (STR_PCI2_ONE_VAR_3), HiiHandle, Bar64 & 0xfffffffffffffff0);
      PrintToken (STRING_TOKEN (STR_PCI2_MEM), HiiHandle);
      PrintToken (STRING_TOKEN (STR_PCI2_64_BITS), HiiHandle);
      IsBar32 = FALSE;
      *Index += 1;

    } else {
      //
      // Reserved
      //
      PrintToken (STRING_TOKEN (STR_PCI2_BAR), HiiHandle, *Bar & 0xfffffff0);
      PrintToken (STRING_TOKEN (STR_PCI2_MEM_2), HiiHandle);
    }

    if ((*Bar & PCI_BIT_3) == 0) {
      PrintToken (STRING_TOKEN (STR_PCI2_NO), HiiHandle);

    } else {
      PrintToken (STRING_TOKEN (STR_PCI2_YES), HiiHandle);
    }

  } else {
    //
    // This bar is of io type
    //
    IsMem = FALSE;
    PrintToken (STRING_TOKEN (STR_PCI2_ONE_VAR_4), HiiHandle, *Bar & 0xfffffffc);
    Print (L"I/O                               ");
  }

  //
  // Get BAR length(or the amount of resource this bar demands for). To get
  // Bar length, first we should temporarily disable I/O and memory access
  // of this function(by set bits in the register Command), then write all
  // "1"s to this bar. The bar value read back is the amount of resource
  // this bar demands for.
  //
  //
  // Disable io & mem access
  //
  OldCommand  = *Command;
  NewCommand  = (UINT16) (OldCommand & 0xfffc);
  RegAddress  = Address | INDEX_OF (Command);
  IoDev->Pci.Write (IoDev, EfiPciWidthUint16, RegAddress, 1, &NewCommand);

  RegAddress = Address | INDEX_OF (Bar);

  //
  // Read after write the BAR to get the size
  //
  if (IsBar32) {
    OldBar32  = *Bar;
    NewBar32  = 0xffffffff;

    IoDev->Pci.Write (IoDev, EfiPciWidthUint32, RegAddress, 1, &NewBar32);
    IoDev->Pci.Read (IoDev, EfiPciWidthUint32, RegAddress, 1, &NewBar32);
    IoDev->Pci.Write (IoDev, EfiPciWidthUint32, RegAddress, 1, &OldBar32);

    if (IsMem) {
      NewBar32  = NewBar32 & 0xfffffff0;
      NewBar32  = (~NewBar32) + 1;

    } else {
      NewBar32  = NewBar32 & 0xfffffffc;
      NewBar32  = (~NewBar32) + 1;
      NewBar32  = NewBar32 & 0x0000ffff;
    }
  } else {

    OldBar64 = 0x0;
    CopyMem (&OldBar64, Bar, sizeof (UINT32));
    NewBar64 = 0xffffffffffffffff;

    IoDev->Pci.Write (IoDev, EfiPciWidthUint32, RegAddress, 2, &NewBar64);
    IoDev->Pci.Read (IoDev, EfiPciWidthUint32, RegAddress, 2, &NewBar64);
    IoDev->Pci.Write (IoDev, EfiPciWidthUint32, RegAddress, 2, &OldBar64);

    if (IsMem) {
      NewBar64  = NewBar64 & 0xfffffffffffffff0;
      NewBar64  = (~NewBar64) + 1;

    } else {
      NewBar64  = NewBar64 & 0xfffffffffffffffc;
      NewBar64  = (~NewBar64) + 1;
      NewBar64  = NewBar64 & 0x000000000000ffff;
    }
  }
  //
  // Enable io & mem access
  //
  RegAddress = Address | INDEX_OF (Command);
  IoDev->Pci.Write (IoDev, EfiPciWidthUint16, RegAddress, 1, &OldCommand);

  if (IsMem) {
    if (IsBar32) {
      PrintToken (STRING_TOKEN (STR_PCI2_NEWBAR_32), HiiHandle, NewBar32);
      PrintToken (STRING_TOKEN (STR_PCI2_NEWBAR_32_2), HiiHandle, NewBar32 + (*Bar & 0xfffffff0) - 1);

    } else {
      PrintToken (STRING_TOKEN (STR_PCI2_RSHIFT), HiiHandle, RShiftU64 (NewBar64, 32));
      PrintToken (STRING_TOKEN (STR_PCI2_RSHIFT), HiiHandle, (UINT32) NewBar64);
      Print (L"  ");
      PrintToken (
        STRING_TOKEN (STR_PCI2_RSHIFT),
        HiiHandle,
        RShiftU64 ((NewBar64 + (Bar64 & 0xfffffffffffffff0) - 1), 32)
        );
      PrintToken (STRING_TOKEN (STR_PCI2_RSHIFT), HiiHandle, (UINT32) (NewBar64 + (Bar64 & 0xfffffffffffffff0) - 1));

    }
  } else {
    PrintToken (STRING_TOKEN (STR_PCI2_NEWBAR_32_3), HiiHandle, NewBar32);
    PrintToken (STRING_TOKEN (STR_PCI2_NEWBAR_32_4), HiiHandle, NewBar32 + (*Bar & 0xfffffffc) - 1);
  }

  return EFI_SUCCESS;
}

EFI_STATUS
PciExplainCardBusData (
  IN PCI_CARDBUS_HEADER                     *CardBus,
  IN UINT64                                 Address,
  IN EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL        *IoDev
  )
/*++

Routine Description:

  Explain the cardbus specific part of data in PCI configuration space.

Arguments:

  CardBus         CardBus specific region of PCI configuration space
  Address         Address used to access configuration space of this PCI device
  IoDev           Handle used to access configuration space of PCI device

Returns:

  EFI_SUCCESS     The command completed successfully

--*/
{
  BOOLEAN           Io32Bit;
  PCI_CARDBUS_DATA  *CardBusData;

  PrintToken (
    STRING_TOKEN (STR_PCI2_CARDBUS_SOCKET),
    HiiHandle,
    INDEX_OF (&(CardBus->CardBusSocketReg)),
    CardBus->CardBusSocketReg
    );

  //
  // Print Secondary Status
  //
  PciExplainStatus (&(CardBus->SecondaryStatus), FALSE, PciCardBusBridge);

  //
  // Print Bus Numbers(Primary bus number, CardBus bus number, and
  // Subordinate bus number
  //
  PrintToken (
    STRING_TOKEN (STR_PCI2_BUS_NUMBERS_2),
    HiiHandle,
    INDEX_OF (&(CardBus->PciBusNumber)),
    INDEX_OF (&(CardBus->CardBusBusNumber)),
    INDEX_OF (&(CardBus->SubordinateBusNumber))
    );

  Print (L"               ------------------------------------------------------\n");

  PrintToken (STRING_TOKEN (STR_PCI2_CARDBUS), HiiHandle, CardBus->PciBusNumber);
  PrintToken (STRING_TOKEN (STR_PCI2_CARDBUS_2), HiiHandle, CardBus->CardBusBusNumber);
  PrintToken (STRING_TOKEN (STR_PCI2_CARDBUS_3), HiiHandle, CardBus->SubordinateBusNumber);

  //
  // Print CardBus Latency Timer
  //
  PrintToken (
    STRING_TOKEN (STR_PCI2_CARDBUS_LATENCY),
    HiiHandle,
    INDEX_OF (&(CardBus->CardBusLatencyTimer)),
    CardBus->CardBusLatencyTimer
    );

  //
  // Print Memory/Io ranges this cardbus bridge forwards
  //
  PrintToken (STRING_TOKEN (STR_PCI2_RESOURCE_TYPE_2), HiiHandle);
  Print (L"----------------------------------------------------------------------\n");

  PrintToken (
    STRING_TOKEN (STR_PCI2_MEM_3),
    HiiHandle,
    INDEX_OF (&(CardBus->MemoryBase0)),
    CardBus->BridgeControl & PCI_BIT_8 ? L"    Prefetchable" : L"Non-Prefetchable",
    CardBus->MemoryBase0 & 0xfffff000,
    CardBus->MemoryLimit0 | 0x00000fff
    );

  PrintToken (
    STRING_TOKEN (STR_PCI2_MEM_3),
    HiiHandle,
    INDEX_OF (&(CardBus->MemoryBase1)),
    CardBus->BridgeControl & PCI_BIT_9 ? L"    Prefetchable" : L"Non-Prefetchable",
    CardBus->MemoryBase1 & 0xfffff000,
    CardBus->MemoryLimit1 | 0x00000fff
    );

  Io32Bit = (BOOLEAN) (CardBus->IoBase0 & PCI_BIT_0);
  PrintToken (
    STRING_TOKEN (STR_PCI2_IO_2),
    HiiHandle,
    INDEX_OF (&(CardBus->IoBase0)),
    Io32Bit ? L"          32 bit" : L"          16 bit",
    CardBus->IoBase0 & (Io32Bit ? 0xfffffffc : 0x0000fffc),
    CardBus->IoLimit0 & (Io32Bit ? 0xffffffff : 0x0000ffff) | 0x00000003
    );

  Io32Bit = (BOOLEAN) (CardBus->IoBase1 & PCI_BIT_0);
  PrintToken (
    STRING_TOKEN (STR_PCI2_IO_2),
    HiiHandle,
    INDEX_OF (&(CardBus->IoBase1)),
    Io32Bit ? L"          32 bit" : L"          16 bit",
    CardBus->IoBase1 & (Io32Bit ? 0xfffffffc : 0x0000fffc),
    CardBus->IoLimit1 & (Io32Bit ? 0xffffffff : 0x0000ffff) | 0x00000003
    );

  //
  // Print register Interrupt Line & PIN
  //
  PrintToken (
    STRING_TOKEN (STR_PCI2_INTERRUPT_LINE_3),
    HiiHandle,
    INDEX_OF (&(CardBus->InterruptLine)),
    CardBus->InterruptLine,
    INDEX_OF (&(CardBus->InterruptPin)),
    CardBus->InterruptPin
    );

  //
  // Print register Bridge Control
  //
  PciExplainBridgeControl (&(CardBus->BridgeControl), PciCardBusBridge);

  //
  // Print some registers in data region of PCI configuration space for cardbus
  // bridge. Fields include: Sub VendorId, Subsystem ID, and Legacy Mode Base
  // Address.
  //
  CardBusData = (PCI_CARDBUS_DATA *) ((UINT8 *) CardBus + sizeof (PCI_CARDBUS_HEADER));

  PrintToken (
    STRING_TOKEN (STR_PCI2_SUB_VENDOR_ID_2),
    HiiHandle,
    INDEX_OF (&(CardBusData->SubVendorId)),
    CardBusData->SubVendorId,
    INDEX_OF (&(CardBusData->SubSystemId)),
    CardBusData->SubSystemId
    );

  PrintToken (
    STRING_TOKEN (STR_PCI2_OPTIONAL),
    HiiHandle,
    INDEX_OF (&(CardBusData->LegacyBase)),
    CardBusData->LegacyBase
    );

  return EFI_SUCCESS;
}

EFI_STATUS
PciExplainStatus (
  IN UINT16                                 *Status,
  IN BOOLEAN                                MainStatus,
  IN PCI_HEADER_TYPE                        HeaderType
  )
/*++

Routine Description:

  Explain each meaningful bit of register Status. The definition of Status is
  slightly different depending on the PCI header type.

Arguments:

  Status          Points to the content of register Status
  MainStatus      Indicates if this register is main status(not secondary 
                  status)
  HeaderType      Header type of this PCI device
  
Returns:

  EFI_SUCCESS     The command completed successfully

--*/
{
  if (MainStatus) {
    PrintToken (STRING_TOKEN (STR_PCI2_STATUS), HiiHandle, INDEX_OF (Status), *Status);

  } else {
    PrintToken (STRING_TOKEN (STR_PCI2_SECONDARY_STATUS), HiiHandle, INDEX_OF (Status), *Status);
  }

  PrintToken (STRING_TOKEN (STR_PCI2_NEW_CAPABILITIES), HiiHandle, (*Status & PCI_BIT_4) != 0);

  //
  // Bit 5 is meaningless for CardBus Bridge
  //
  if (HeaderType == PciCardBusBridge) {
    PrintToken (STRING_TOKEN (STR_PCI2_66_CAPABLE), HiiHandle, (*Status & PCI_BIT_5) != 0);

  } else {
    PrintToken (STRING_TOKEN (STR_PCI2_66_CAPABLE_2), HiiHandle, (*Status & PCI_BIT_5) != 0);
  }

  PrintToken (STRING_TOKEN (STR_PCI2_FAST_BACK), HiiHandle, (*Status & PCI_BIT_7) != 0);

  PrintToken (STRING_TOKEN (STR_PCI2_MASTER_DATA), HiiHandle, (*Status & PCI_BIT_8) != 0);
  //
  // Bit 9 and bit 10 together decides the DEVSEL timing
  //
  PrintToken (STRING_TOKEN (STR_PCI2_DEVSEL_TIMING), HiiHandle);
  if ((*Status & PCI_BIT_9) == 0 && (*Status & PCI_BIT_10) == 0) {
    PrintToken (STRING_TOKEN (STR_PCI2_FAST), HiiHandle);

  } else if ((*Status & PCI_BIT_9) != 0 && (*Status & PCI_BIT_10) == 0) {
    PrintToken (STRING_TOKEN (STR_PCI2_MEDIUM), HiiHandle);

  } else if ((*Status & PCI_BIT_9) == 0 && (*Status & PCI_BIT_10) != 0) {
    PrintToken (STRING_TOKEN (STR_PCI2_SLOW), HiiHandle);

  } else {
    PrintToken (STRING_TOKEN (STR_PCI2_RESERVED_2), HiiHandle);
  }

  PrintToken (
    STRING_TOKEN (STR_PCI2_SIGNALED_TARGET),
    HiiHandle,
    (*Status & PCI_BIT_11) != 0
    );

  PrintToken (
    STRING_TOKEN (STR_PCI2_RECEIVED_TARGET),
    HiiHandle,
    (*Status & PCI_BIT_12) != 0
    );

  PrintToken (
    STRING_TOKEN (STR_PCI2_RECEIVED_MASTER),
    HiiHandle,
    (*Status & PCI_BIT_13) != 0
    );

  if (MainStatus) {
    PrintToken (
      STRING_TOKEN (STR_PCI2_SIGNALED_ERROR),
      HiiHandle,
      (*Status & PCI_BIT_14) != 0
      );

  } else {
    PrintToken (
      STRING_TOKEN (STR_PCI2_RECEIVED_ERROR),
      HiiHandle,
      (*Status & PCI_BIT_14) != 0
      );
  }

  PrintToken (
    STRING_TOKEN (STR_PCI2_DETECTED_ERROR),
    HiiHandle,
    (*Status & PCI_BIT_15) != 0
    );

  return EFI_SUCCESS;
}

EFI_STATUS
PciExplainCommand (
  IN UINT16                                 *Command
  )
/*++

Routine Description:

  Explain each meaningful bit of register Command. 

Arguments:

  Command         Points to the content of register Command
  
Returns:

  EFI_SUCCESS     The command completed successfully

--*/
{
  //
  // Print the binary value of register Command
  //
  PrintToken (STRING_TOKEN (STR_PCI2_COMMAND), HiiHandle, INDEX_OF (Command), *Command);

  //
  // Explain register Command bit by bit
  //
  PrintToken (
    STRING_TOKEN (STR_PCI2_SPACE_ACCESS_DENIED),
    HiiHandle,
    (*Command & PCI_BIT_0) != 0
    );

  PrintToken (
    STRING_TOKEN (STR_PCI2_MEMORY_SPACE),
    HiiHandle,
    (*Command & PCI_BIT_1) != 0
    );

  PrintToken (
    STRING_TOKEN (STR_PCI2_BEHAVE_BUS_MASTER),
    HiiHandle,
    (*Command & PCI_BIT_2) != 0
    );

  PrintToken (
    STRING_TOKEN (STR_PCI2_MONITOR_SPECIAL_CYCLE),
    HiiHandle,
    (*Command & PCI_BIT_3) != 0
    );

  PrintToken (
    STRING_TOKEN (STR_PCI2_MEM_WRITE_INVALIDATE),
    HiiHandle,
    (*Command & PCI_BIT_4) != 0
    );

  PrintToken (
    STRING_TOKEN (STR_PCI2_PALETTE_SNOOPING),
    HiiHandle,
    (*Command & PCI_BIT_5) != 0
    );

  PrintToken (
    STRING_TOKEN (STR_PCI2_ASSERT_PERR),
    HiiHandle,
    (*Command & PCI_BIT_6) != 0
    );

  PrintToken (
    STRING_TOKEN (STR_PCI2_DO_ADDR_STEPPING),
    HiiHandle,
    (*Command & PCI_BIT_7) != 0
    );

  PrintToken (
    STRING_TOKEN (STR_PCI2_SERR_DRIVER),
    HiiHandle,
    (*Command & PCI_BIT_8) != 0
    );

  PrintToken (
    STRING_TOKEN (STR_PCI2_FAST_BACK_2),
    HiiHandle,
    (*Command & PCI_BIT_9) != 0
    );

  return EFI_SUCCESS;
}

EFI_STATUS
PciExplainBridgeControl (
  IN UINT16                                 *BridgeControl,
  IN PCI_HEADER_TYPE                        HeaderType
  )
/*++

Routine Description:

  Explain each meaningful bit of register Bridge Control. 

Arguments:

  BridgeControl   Points to the content of register Bridge Control
  HeaderType      The headertype
  
Returns:

  EFI_SUCCESS     The command completed successfully

--*/
{
  PrintToken (
    STRING_TOKEN (STR_PCI2_BRIDGE_CONTROL),
    HiiHandle,
    INDEX_OF (BridgeControl),
    *BridgeControl
    );

  PrintToken (
    STRING_TOKEN (STR_PCI2_PARITY_ERROR),
    HiiHandle,
    (*BridgeControl & PCI_BIT_0) != 0
    );
  PrintToken (
    STRING_TOKEN (STR_PCI2_SERR_ENABLE),
    HiiHandle,
    (*BridgeControl & PCI_BIT_1) != 0
    );
  PrintToken (
    STRING_TOKEN (STR_PCI2_ISA_ENABLE),
    HiiHandle,
    (*BridgeControl & PCI_BIT_2) != 0
    );
  PrintToken (
    STRING_TOKEN (STR_PCI2_VGA_ENABLE),
    HiiHandle,
    (*BridgeControl & PCI_BIT_3) != 0
    );
  PrintToken (
    STRING_TOKEN (STR_PCI2_MASTER_ABORT),
    HiiHandle,
    (*BridgeControl & PCI_BIT_5) != 0
    );

  //
  // Register Bridge Control has some slight differences between P2P bridge
  // and Cardbus bridge from bit 6 to bit 11.
  //
  if (HeaderType == PciP2pBridge) {
    PrintToken (
      STRING_TOKEN (STR_PCI2_SECONDARY_BUS_RESET),
      HiiHandle,
      (*BridgeControl & PCI_BIT_6) != 0
      );
    PrintToken (
      STRING_TOKEN (STR_PCI2_FAST_ENABLE),
      HiiHandle,
      (*BridgeControl & PCI_BIT_7) != 0
      );
    PrintToken (
      STRING_TOKEN (STR_PCI2_PRIMARY_DISCARD_TIMER),
      HiiHandle,
      (*BridgeControl & PCI_BIT_8) ? L"2^10" : L"2^15"
      );
    PrintToken (
      STRING_TOKEN (STR_PCI2_SECONDARY_DISCARD_TIMER),
      HiiHandle,
      (*BridgeControl & PCI_BIT_9) ? L"2^10" : L"2^15"
      );
    PrintToken (
      STRING_TOKEN (STR_PCI2_DISCARD_TIMER_STATUS),
      HiiHandle,
      (*BridgeControl & PCI_BIT_10) != 0
      );
    PrintToken (
      STRING_TOKEN (STR_PCI2_DISCARD_TIMER_SERR),
      HiiHandle,
      (*BridgeControl & PCI_BIT_11) != 0
      );

  } else {
    PrintToken (
      STRING_TOKEN (STR_PCI2_CARDBUS_RESET),
      HiiHandle,
      (*BridgeControl & PCI_BIT_6) != 0
      );
    PrintToken (
      STRING_TOKEN (STR_PCI2_IREQ_ENABLE),
      HiiHandle,
      (*BridgeControl & PCI_BIT_7) != 0
      );
    PrintToken (
      STRING_TOKEN (STR_PCI2_WRITE_POSTING_ENABLE),
      HiiHandle,
      (*BridgeControl & PCI_BIT_10) != 0
      );
  }

  return EFI_SUCCESS;
}

EFI_STATUS
PciExplainCapabilityStruct (
  IN  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL         *IoDev,
  IN UINT64                                   Address,
  IN  UINT8                                   CapPtr
  )
{
  UINT8   CapabilityPtr;
  UINT16  CapabilityEntry;
  UINT8   CapabilityID;
  UINT64  RegAddress;

  CapabilityPtr = CapPtr;

  //
  // Go through the Capability list
  //
  while (CapabilityPtr > 0x3F) {
    //
    // Mask it to DWORD alignment per PCI spec
    //
    CapabilityPtr &= 0xFC;

    RegAddress = Address + CapabilityPtr;
    IoDev->Pci.Read (IoDev, EfiPciWidthUint16, RegAddress, 1, &CapabilityEntry);

    CapabilityID = (UINT8) CapabilityEntry;

    //
    // Explain PciExpress data
    //
    if (EFI_PCI_CAPABILITY_ID_PCIEXP == CapabilityID) {
      PciExplainPciExpress (IoDev, Address, CapabilityPtr);
      return EFI_SUCCESS;
    }
    //
    // Explain other capabilities here
    //
    CapabilityPtr = (UINT8) (CapabilityEntry >> 8);
  }

  return EFI_SUCCESS;
}

EFI_STATUS
PciExplainPciExpress (
  IN  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL         *IoDev,
  IN  UINT64                                  Address,
  IN  UINT8                                   CapabilityPtr
  )
{

  PCIE_CAP_STURCTURE  PciExpressCap;
  EFI_STATUS          Status;
  UINT64              RegAddress;
  UINT8               Bus;
  UINT8               Dev;
  UINT8               Func;
  UINT8               *ExRegBuffer;
  UINTN               ExtendRegSize;
  UINT64              Pciex_Address;

  RegAddress = Address + CapabilityPtr;
  IoDev->Pci.Read (
              IoDev,
              EfiPciWidthUint32,
              RegAddress,
              sizeof (PciExpressCap) / sizeof (UINT32),
              &PciExpressCap
              );

  Print (L"\nPci Express device capability structure:\n");
  //
  // Print PciExpress Capability ID, should be 0x10 per PCI sepc.
  //
  PrintToken (
    STRING_TOKEN (STR_PCIEX_CAPABILITY_CAPID),
    HiiHandle,
    0x00,
    PciExpressCap.PcieCapId
    );

  //
  // Print Next capability pointer in PciExpress Capability structure
  //
  PrintToken (
    STRING_TOKEN (STR_PCIEX_NEXTCAP_PTR),
    HiiHandle,
    0x01,
    PciExpressCap.NextCapPtr
    );

  //
  // Print Capabilities register in PciExpress Capability structure
  //
  PrintToken (
    STRING_TOKEN (STR_PCIEX_CAP_REGISTER),
    HiiHandle,
    0x02,
    PciExpressCap.PcieCapReg
    );

  //
  // Print Device capabilities in PciExpress Capability structure
  //
  PrintToken (
    STRING_TOKEN (STR_PCIEX_DEVICE_CAP),
    HiiHandle,
    0x04,
    PciExpressCap.PcieDeviceCap
    );

  //
  // Print DeviceStatus in PciExpress Capability structure
  //
  PrintToken (
    STRING_TOKEN (STR_PCIEX_DEVICE_CONTROL),
    HiiHandle,
    0x08,
    PciExpressCap.DeviceControl
    );

  //
  // Print DeviceStatus in PciExpress Capability structure
  //
  PrintToken (
    STRING_TOKEN (STR_PCIEX_DEVICE_STATUS),
    HiiHandle,
    0x0A,
    PciExpressCap.DeviceStatus
    );

  //
  // Print Link Capabilities in PciExpress Capability structure
  //
  PrintToken (
    STRING_TOKEN (STR_PCIEX_LINK_CAPABILITIES),
    HiiHandle,
    0x0C,
    PciExpressCap.LinkCap
    );

  //
  // Print LinkControl in PciExpress Capability structure
  //
  PrintToken (
    STRING_TOKEN (STR_PCIEX_LINK_CONTROL),
    HiiHandle,
    0x10,
    PciExpressCap.LinkControl
    );

  //
  // Print LinkStatus in PciExpress Capability structure
  //
  PrintToken (
    STRING_TOKEN (STR_PCIEX_LINK_STATUS),
    HiiHandle,
    0x12,
    PciExpressCap.LinkStatus
    );

  //
  // Print RootCap in PciExpress Capability structure
  //
  PrintToken (
    STRING_TOKEN (STR_PCIEX_SLOT_CAPABILITIES),
    HiiHandle,
    0x14,
    PciExpressCap.SlotCap
    );

  //
  // Print SlotControl in PciExpress Capability structure
  //
  PrintToken (
    STRING_TOKEN (STR_PCIEX_SLOT_CONTROL),
    HiiHandle,
    0x18,
    PciExpressCap.SlotControl
    );

  //
  // Print SlotStatus in PciExpress Capability structure
  //
  PrintToken (
    STRING_TOKEN (STR_PCIEX_SLOT_STATUS),
    HiiHandle,
    0x1A,
    PciExpressCap.SlotStatus
    );

  //
  // Print RootControl in PciExpress Capability structure
  //
  PrintToken (
    STRING_TOKEN (STR_PCIEX_ROOT_CONTROL),
    HiiHandle,
    0x1C,
    PciExpressCap.RootControl
    );

  //
  // Print RsvdP in PciExpress Capability structure
  //
  PrintToken (
    STRING_TOKEN (STR_PCIEX_RSVDP),
    HiiHandle,
    0x1E,
    PciExpressCap.RsvdP
    );

  //
  // Print RootStatus in PciExpress Capability structure
  //
  PrintToken (
    STRING_TOKEN (STR_PCIEX_ROOT_STATUS),
    HiiHandle,
    0x20,
    PciExpressCap.RootStatus
    );

  Bus           = (UINT8) (RShiftU64 (Address, 24));
  Dev           = (UINT8) (RShiftU64 (Address, 16));
  Func          = (UINT8) (RShiftU64 (Address, 8));

  Pciex_Address = CALC_EFI_PCIEX_ADDRESS (Bus, Dev, Func, 0x100);

  ExtendRegSize = 0x1000 - 0x100;

  ExRegBuffer   = (UINT8 *) AllocatePool (ExtendRegSize);

  //
  // PciRootBridgeIo protocol should support pci express extend space IO
  // (Begins at offset 0x100)
  //
  Status = IoDev->Pci.Read (
                        IoDev,
                        EfiPciWidthUint32,
                        Pciex_Address,
                        (ExtendRegSize) / sizeof (UINT32),
                        (VOID *) (ExRegBuffer)
                        );
  if (EFI_ERROR (Status)) {
    FreePool ((VOID *) ExRegBuffer);
    return EFI_UNSUPPORTED;
  }
  //
  // Start outputing PciEx extend space( 0xFF-0xFFF)
  //
  Print (L"\n%HStart dumping PCIex extended configuration space (0x100 - 0xFFF).%N\n\n");

  DumpHex (
    2,
    0x100,
    ExtendRegSize,
    (VOID *) (ExRegBuffer)
    );

  FreePool ((VOID *) ExRegBuffer);
  return EFI_SUCCESS;
}

EFI_STATUS
InitializePciGetLineHelp (
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
          &EfiPciGuid,
          STRING_TOKEN (STR_HELPINFO_PCI_LINEHELP),
          Str
          );
}
