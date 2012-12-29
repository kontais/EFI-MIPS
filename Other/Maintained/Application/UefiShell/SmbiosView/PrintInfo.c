/*++

Copyright (c) 2005 - 2006 Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

    PrintInfo.c
    
Abstract:

    Module for clarifying the content of the smbios structure element information.

Revision History

--*/

#include "EfiShellLib.h"
#include "PrintInfo.h"
#include "LibSmbiosView.h"
#include "QueryTable.h"
#include "EventLogInfo.h"
#include "smbios.h"
#include STRING_DEFINES_FILE

//
// Get the certain bit of 'value'
//
#define BIT(value, bit) ((value) & ((UINT64) 1) << (bit))

//
//////////////////////////////////////////////////////////
//  Macros of print structure element, simplify coding.
//
#define PrintPendingString(pStruct, type, element) \
  do { \
    CHAR8 StringBuf[64]; \
    SetMem (StringBuf, sizeof (StringBuf), 0x00); \
    SmbiosGetPendingString ((pStruct), (pStruct->type->element), StringBuf); \
    APrint (#element); \
    APrint (": %a\n", StringBuf); \
  } while (0);

#define PrintStructValue(pStruct, type, element) \
  do { \
    APrint (#element); \
    APrint (": %d\n", (pStruct->type->element)); \
  } while (0);

#define PrintStructValueH(pStruct, type, element) \
  do { \
    APrint (#element); \
    APrint (": 0x%x\n", (pStruct->type->element)); \
  } while (0);

#define PrintBitField(pStruct, type, element, size) \
  do { \
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_DUMP), HiiHandle); \
    APrint (#element); \
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_SIZE), HiiHandle, size); \
    DumpHex (0, 0, size, &(pStruct->type->element)); \
  } while (0);

//
/////////////////////////////////////////
//
STATIC
VOID
MemToString (
  IN OUT VOID   *Dest,
  IN VOID       *Src,
  IN UINTN      Length
  )
/*++

Routine Description:
  Copy Length of Src buffer to Dest buffer, 
  add a NULL termination to Dest buffer.

Arguments:
  Dest      - Destination buffer head
  Src       - Source buffer head
  Length    - Length of buffer to be copied

Returns:
  None.

--*/
{
  UINT8 *SrcBuffer;
  UINT8 *DestBuffer;
  SrcBuffer   = (UINT8 *) Src;
  DestBuffer  = (UINT8 *) Dest;
  //
  // copy byte by byte
  //
  while (Length--) {
    *DestBuffer++ = *SrcBuffer++;
  }
  //
  // append a NULL terminator
  //
  *DestBuffer = '\0';
}

//
//////////////////////////////////////////////
//
// Functions below is to show the information
//
VOID
SmbiosPrintEPSInfo (
  IN  SMBIOS_STRUCTURE_TABLE  *SmbiosTable,
  IN  UINT8                   Option
  )
/*++

Routine Description:
  Print the info of EPS(Entry Point Structure)

Arguments:
  SmbiosTable    - Pointer to the SMBIOS table entry point
  Option         - Display option

Returns: None

--*/
{
  UINT8 Anchor[5];
  UINT8 InAnchor[6];

  if (SmbiosTable == NULL) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_SMBIOSTABLE_NULL), HiiHandle);
    return ;
  }

  if (Option == SHOW_NONE) {
    return ;
  }

  if (Option >= SHOW_NORMAL) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_ENTRY_POINT_SIGN), HiiHandle);
    MemToString (Anchor, SmbiosTable->AnchorString, 4);
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_ANCHOR_STR), HiiHandle, Anchor);
    PrintToken (
      STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_EPS_CHECKSUM),
      HiiHandle,
      SmbiosTable->EntryPointStructureChecksum
      );
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_ENTRY_POINT_LEN), HiiHandle, SmbiosTable->EntryPointLength);
    PrintToken (
      STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_VERSION),
      HiiHandle,
      SmbiosTable->MajorVersion,
      SmbiosTable->MinorVersion
      );
    PrintToken (
      STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_NUMBER_STRUCT),
      HiiHandle,
      SmbiosTable->NumberOfSmbiosStructures
      );
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_MAX_STRUCT_SIZE), HiiHandle, SmbiosTable->MaxStructureSize);
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_TABLE_ADDR), HiiHandle, SmbiosTable->TableAddress);
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_TABLE_LENGTH), HiiHandle, SmbiosTable->TableLength);

  }
  //
  // If SHOW_ALL, also print followings.
  //
  if (Option >= SHOW_DETAIL) {
    PrintToken (
      STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_ENTRY_POINT_REVISION),
      HiiHandle,
      SmbiosTable->EntryPointRevision
      );
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_BCD_REVISION), HiiHandle, SmbiosTable->SmbiosBcdRevision);
    //
    // Since raw data is not string, add a NULL terminater.
    //
    MemToString (InAnchor, SmbiosTable->IntermediateAnchorString, 5);
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_INTER_ACHOR), HiiHandle, InAnchor);
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_INTER_CHECKSUM), HiiHandle, SmbiosTable->IntermediateChecksum);
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_FORMATTED_AREA), HiiHandle);
    DumpHex (2, 0, 5, SmbiosTable->FormattedArea);
  }

  Print (L"\n");
}

EFI_STATUS
SmbiosPrintStructure (
  IN  SMBIOS_STRUCTURE_POINTER  *pStruct,
  IN  UINT8                     Option
  )
/*++

Routine Description:
  This function print the content of the structure pointed by pStruct

Arguments:
  pStruct     - point to the structure to be printed
  Option      - print option of information detail

Returns:
  EFI_SUCCESS             - Successfully Printing this function 
  EFI_INVALID_PARAMETER   - Invalid Structure
  EFI_UNSUPPORTED         - Unsupported

--*/
{
  UINT8 Index;
  UINT8 *Buffer;

  Buffer = (UINT8 *) (UINTN) (pStruct->Raw);

  if (pStruct == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (Option == SHOW_NONE) {
    return EFI_SUCCESS;
  }
  //
  // Display structure header
  //
  DisplayStructureTypeInfo (pStruct->Hdr->Type, SHOW_DETAIL);
  PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_FORMAT_PART_LEN), HiiHandle, pStruct->Hdr->Length);
  PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_STRUCT_HANDLE), HiiHandle, pStruct->Hdr->Handle);

  if (Option == SHOW_OUTLINE) {
    return EFI_SUCCESS;
  }

  switch (pStruct->Hdr->Type) {
  //
  //
  //
  case 0:
    PrintPendingString (pStruct, Type0, Vendor);
    PrintPendingString (pStruct, Type0, BiosVersion);
    PrintStructValue (pStruct, Type0, BiosSegment);
    PrintPendingString (pStruct, Type0, BiosReleaseDate);
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_BIOS_SIZE), 64 * (pStruct->Type0->BiosSize + 1));

    if (Option < SHOW_DETAIL) {
      PrintStructValueH (pStruct, Type0, BiosCharacteristics);
    } else {
      DisplayBiosCharacteristics (pStruct->Type0->BiosCharacteristics, Option);

      //
      // The length of above format part is 0x12 bytes,
      // Ext bytes are following, size = 'len-0x12'.
      // If len-0x12 > 0, then
      //    there are extension bytes (byte1, byte2, byte3...)
      // And byte3 not stated in spec, so dump all extension bytes(1, 2, 3..)
      //
      if ((Buffer[1] - (CHAR8) 0x12) > 0) {
        DisplayBiosCharacteristicsExt1 (Buffer[0x12], Option);
      }

      if ((Buffer[1] - (CHAR8) 0x12) > 1) {
        DisplayBiosCharacteristicsExt2 (Buffer[0x13], Option);
      }

      if ((Buffer[1] - (CHAR8) 0x12) > 2) {
        PrintBitField (
          pStruct,
          Type0,
          BiosCharacteristics,
          Buffer[1] - (CHAR8) 0x12
          );
      }
    }
    break;

  //
  // System Information (Type 1)
  //
  case 1:
    PrintPendingString (pStruct, Type1, Manufacturer);
    PrintPendingString (pStruct, Type1, ProductName);
    PrintPendingString (pStruct, Type1, Version);
    PrintPendingString (pStruct, Type1, SerialNumber);
    PrintBitField (pStruct, Type1, Uuid, 16);
    DisplaySystemWakeupType (pStruct->Type1->WakeUpType, Option);
    break;

  case 2:
    PrintPendingString (pStruct, Type2, Manufacturer);
    PrintPendingString (pStruct, Type2, ProductName);
    PrintPendingString (pStruct, Type2, Version);
    PrintPendingString (pStruct, Type2, SerialNumber);
    break;

  //
  // System Enclosure (Type 3)
  //
  case 3:
    PrintPendingString (pStruct, Type3, Manufacturer);
    PrintStructValue (pStruct, Type3, Type);
    DisplaySystemEnclosureType (pStruct->Type3->Type, Option);
    PrintPendingString (pStruct, Type3, Version);
    PrintPendingString (pStruct, Type3, SerialNumber);
    PrintPendingString (pStruct, Type3, AssetTag);
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_BOOTUP_STATE), HiiHandle);
    DisplaySystemEnclosureStatus (pStruct->Type3->BootupState, Option);
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_POWER_SUPPLY_STATE), HiiHandle);
    DisplaySystemEnclosureStatus (pStruct->Type3->PowerSupplyState, Option);
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_THERMAL_STATE), HiiHandle);
    DisplaySystemEnclosureStatus (pStruct->Type3->ThermalState, Option);
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_SECURITY_STATUS), HiiHandle);
    DisplaySESecurityStatus (pStruct->Type3->SecurityStatus, Option);
    PrintBitField (pStruct, Type3, OemDefined, 4);
    break;

  //
  // Processor Information (Type 4)
  //
  case 4:
    PrintStructValue (pStruct, Type4, Socket);
    DisplayProcessorType (pStruct->Type4->ProcessorType, Option);
    DisplayProcessorFamily (pStruct->Type4->ProcessorFamily, Option);
    PrintPendingString (pStruct, Type4, ProcessorManufacture);
    PrintBitField (pStruct, Type4, ProcessorId, 8);
    PrintPendingString (pStruct, Type4, ProcessorVersion);
    DisplayProcessorVoltage (pStruct->Type4->Voltage, Option);
    PrintStructValue (pStruct, Type4, ExternalClock);
    PrintStructValue (pStruct, Type4, MaxSpeed);
    PrintStructValue (pStruct, Type4, CurrentSpeed);
    DisplayProcessorStatus (pStruct->Type4->Status, Option);
    DisplayProcessorUpgrade (pStruct->Type4->ProcessorUpgrade, Option);
    PrintStructValueH (pStruct, Type4, L1CacheHandle);
    PrintStructValueH (pStruct, Type4, L2CacheHandle);
    PrintStructValueH (pStruct, Type4, L3CacheHandle);
    PrintPendingString (pStruct, Type4, SerialNumber);
    PrintPendingString (pStruct, Type4, AssetTag);
    PrintPendingString (pStruct, Type4, PartNumber);
    break;

  //
  // Memory Controller Information (Type 5)
  //
  case 5:
    {
      UINT8 SlotNum;
      SlotNum = pStruct->Type5->AssociatedMemorySlotNum;

      DisplayMcErrorDetectMethod (pStruct->Type5->ErrDetectMethod, Option);
      DisplayMcErrorCorrectCapability (pStruct->Type5->ErrCorrectCapability, Option);
      PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_SUPOPRT), HiiHandle);
      DisplayMcInterleaveSupport (pStruct->Type5->SupportInterleave, Option);
      PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_CURRENT), HiiHandle);
      DisplayMcInterleaveSupport (pStruct->Type5->CurrentInterleave, Option);
      DisplayMaxMemoryModuleSize (pStruct->Type5->MaxMemoryModuleSize, SlotNum, Option);
      DisplayMcMemorySpeeds (pStruct->Type5->SupportSpeed, Option);
      DisplayMmMemoryType (pStruct->Type5->SupportMemoryType, Option);
      DisplayMemoryModuleVoltage (pStruct->Type5->MemoryModuleVoltage, Option);
      PrintStructValue (pStruct, Type5, AssociatedMemorySlotNum);
      //
      // According to SMBIOS Specification, offset 0x0F
      //
      DisplayMemoryModuleConfigHandles ((UINT16 *) (&Buffer[0x0F]), SlotNum, Option);
      DisplayMcErrorCorrectCapability (Buffer[0x0F + 2 * SlotNum], Option);
    }
    break;

  //
  // Memory Module Information (Type 6)
  //
  case 6:
    PrintPendingString (pStruct, Type6, SocketDesignation);
    DisplayMmBankConnections (pStruct->Type6->BankConnections, Option);
    PrintStructValue (pStruct, Type6, CurrentSpeed);
    DisplayMmMemoryType (pStruct->Type6->CurrentMemoryType, Option);
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_INSTALLED), HiiHandle);
    DisplayMmMemorySize (pStruct->Type6->InstalledSize, Option);
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_ENABLED), HiiHandle);
    DisplayMmMemorySize (pStruct->Type6->EnabledSize, Option);
    DisplayMmErrorStatus (pStruct->Type6->ErrorStatus, Option);
    break;

  //
  // Cache Information (Type 7)
  //
  case 7:
    PrintPendingString (pStruct, Type7, SocketDesignation);
    PrintStructValueH (pStruct, Type7, CacheConfiguration);
    PrintStructValueH (pStruct, Type7, MaximumCacheSize);
    PrintStructValueH (pStruct, Type7, InstalledSize);
    PrintStructValueH (pStruct, Type7, SupportedSRAMType);
    PrintStructValueH (pStruct, Type7, CurrentSRAMType);
    DisplayCacheSRAMType (pStruct->Type7->CurrentSRAMType, Option);
    PrintStructValueH (pStruct, Type7, CacheSpeed);
    DisplayCacheErrCorrectingType (pStruct->Type7->ErrorCorrectionType, Option);
    DisplayCacheSystemCacheType (pStruct->Type7->SystemCacheType, Option);
    DisplayCacheAssociativity (pStruct->Type7->Associativity, Option);
    break;

  //
  // Port Connector Information  (Type 8)
  //
  case 8:
    PrintPendingString (pStruct, Type8, InternalReferenceDesignator);
    Print (L"Internal ");
    DisplayPortConnectorType (pStruct->Type8->InternalConnectorType, Option);
    PrintPendingString (pStruct, Type8, ExternalReferenceDesignator);
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_EXTERNAL), HiiHandle);
    DisplayPortConnectorType (pStruct->Type8->ExternalConnectorType, Option);
    DisplayPortType (pStruct->Type8->PortType, Option);
    break;

  //
  // System Slots (Type 9)
  //
  case 9:
    PrintPendingString (pStruct, Type9, SlotDesignation);
    DisplaySystemSlotType (pStruct->Type9->SlotType, Option);
    DisplaySystemSlotDataBusWidth (pStruct->Type9->SlotDataBusWidth, Option);
    DisplaySystemSlotCurrentUsage (pStruct->Type9->CurrentUsage, Option);
    DisplaySystemSlotLength (pStruct->Type9->SlotLength, Option);
    DisplaySystemSlotId (
      pStruct->Type9->SlotID,
      pStruct->Type9->SlotType,
      Option
      );
    DisplaySlotCharacteristics1 (pStruct->Type9->SlotCharacteristics1, Option);
    DisplaySlotCharacteristics2 (pStruct->Type9->SlotCharacteristics2, Option);
    break;

  //
  // On Board Devices Information (Type 10)
  //
  case 10:
    {
      UINTN NumOfDevice;
      NumOfDevice = (pStruct->Type10->Hdr.Length - sizeof (SMBIOS_HEADER)) / (2 * sizeof (UINT8));
      for (Index = 0; Index < NumOfDevice; Index++) {
        DisplayOnboardDeviceTypes (pStruct->Type10->Device[Index].DeviceType, Option);
        PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_DESC_STRING), HiiHandle);
        APrint (LibGetSmbiosString (pStruct, pStruct->Type10->Device[Index].DescriptionString));
      }
    }
    break;

  case 11:
    PrintStructValue (pStruct, Type11, StringCount);
    for (Index = 1; Index <= pStruct->Type11->StringCount; Index++) {
      APrint ("%a\n", LibGetSmbiosString (pStruct, Index));
    }
    break;

  case 12:
    PrintStructValue (pStruct, Type12, StringCount);
    for (Index = 1; Index <= pStruct->Type12->StringCount; Index++) {
      APrint ("%a\n", LibGetSmbiosString (pStruct, Index));
    }
    break;

  case 13:
    PrintStructValue (pStruct, Type13, InstallableLanguages);
    PrintStructValue (pStruct, Type13, Flags);
    PrintBitField (pStruct, Type13, reserved, 15);
    PrintPendingString (pStruct, Type13, CurrentLanguages);
    break;

  case 14:
    PrintPendingString (pStruct, Type14, GroupName);
    PrintStructValue (pStruct, Type14, ItemType);
    PrintStructValue (pStruct, Type14, ItemHandle);
    break;

  //
  // System Event Log (Type 15)
  //
  case 15:
    {
      EVENTLOGTYPE  *Ptr;
      UINT8         Count;
      UINT8         *AccessMethodAddress;

      PrintStructValueH (pStruct, Type15, LogAreaLength);
      PrintStructValueH (pStruct, Type15, LogHeaderStartOffset);
      PrintStructValueH (pStruct, Type15, LogDataStartOffset);
      DisplaySELAccessMethod (pStruct->Type15->AccessMethod, Option);
      PrintStructValueH (pStruct, Type15, AccessMethodAddress);
      DisplaySELLogStatus (pStruct->Type15->LogStatus, Option);
      PrintStructValueH (pStruct, Type15, LogChangeToken);
      DisplaySysEventLogHeaderFormat (pStruct->Type15->LogHeaderFormat, Option);
      PrintStructValueH (pStruct, Type15, NumberOfSupportedLogTypeDescriptors);
      PrintStructValueH (pStruct, Type15, LengthOfLogTypeDescriptor);

      Count = pStruct->Type15->NumberOfSupportedLogTypeDescriptors;
      if (Count > 0) {
        Ptr = pStruct->Type15->EventLogTypeDescriptors;

        //
        // Display all Event Log type descriptors supported by system
        //
        for (Index = 0; Index < Count; Index++, Ptr++) {
          PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_SUPOPRTED_EVENT), HiiHandle, Index + 1);
          DisplaySELTypes (Ptr->LogType, Option);
          DisplaySELVarDataFormatType (Ptr->DataFormatType, Option);
        }

        if (Option >= SHOW_DETAIL) {
          switch (pStruct->Type15->AccessMethod) {
          case 03:
            AccessMethodAddress = (UINT8 *) (UINTN) (pStruct->Type15->AccessMethodAddress);
            break;

          case 00:
          case 01:
          case 02:
          case 04:
          default:
            PrintToken (
              STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_ACCESS_METHOD_NOT_SUPOPRTED),
              HiiHandle,
              pStruct->Type15->AccessMethod
              );
            return EFI_UNSUPPORTED;
          }
          //
          // Display Event Log Header
          //
          // Starting offset (or index) within the nonvolatile storage
          // of the event-log's header, from the Access Method Address
          //
          DisplaySysEventLogHeader (
            pStruct->Type15->LogHeaderFormat,
            AccessMethodAddress + pStruct->Type15->LogHeaderStartOffset
            );

          //
          // Display all Event Log data
          //
          // Starting offset (or index) within the nonvolatile storage
          // of the event-log's first data byte, from the Access Method Address(0x14)
          //
          DisplaySysEventLogData (
            AccessMethodAddress + pStruct->Type15->LogDataStartOffset,
            (UINT16)
            (
            pStruct->Type15->LogAreaLength -
            (pStruct->Type15->LogDataStartOffset - pStruct->Type15->LogDataStartOffset)
            )
            );
        }

      }
    }
    break;

  //
  // Physical Memory Array (Type 16)
  //
  case 16:
    DisplayPMALocation (pStruct->Type16->Location, Option);
    DisplayPMAUse (pStruct->Type16->Use, Option);
    DisplayPMAErrorCorrectionTypes (
      pStruct->Type16->MemoryErrorCorrection,
      Option
      );
    PrintStructValueH (pStruct, Type16, MaximumCapacity);
    PrintStructValueH (pStruct, Type16, MemoryErrorInformationHandle);
    PrintStructValueH (pStruct, Type16, NumberOfMemoryDevices);
    break;

  //
  // Memory Device (Type 17)
  //
  case 17:
    PrintStructValueH (pStruct, Type17, MemoryArrayHandle);
    PrintStructValueH (pStruct, Type17, MemoryErrorInformationHandle);
    PrintStructValue (pStruct, Type17, TotalWidth);
    PrintStructValue (pStruct, Type17, DataWidth);
    PrintStructValue (pStruct, Type17, Size);
    DisplayMemoryDeviceFormFactor (pStruct->Type17->FormFactor, Option);
    PrintStructValueH (pStruct, Type17, DeviceSet);
    PrintPendingString (pStruct, Type17, DeviceLocator);
    PrintPendingString (pStruct, Type17, BankLocator);
    DisplayMemoryDeviceType (pStruct->Type17->MemoryType, Option);
    DisplayMemoryDeviceTypeDetail (pStruct->Type17->TypeDetail, Option);
    PrintStructValueH (pStruct, Type17, Speed);
    PrintPendingString (pStruct, Type17, Manufacturer);
    PrintPendingString (pStruct, Type17, SerialNumber);
    PrintPendingString (pStruct, Type17, AssetTag);
    PrintPendingString (pStruct, Type17, PartNumber);
    break;

  //
  // 32-bit Memory Error Information (Type 18)
  //
  case 18:
    DisplayMemoryErrorType (pStruct->Type18->ErrorType, Option);
    DisplayMemoryErrorGranularity (
      pStruct->Type18->ErrorGranularity,
      Option
      );
    DisplayMemoryErrorOperation (pStruct->Type18->ErrorOperation, Option);
    PrintStructValueH (pStruct, Type18, VendorSyndrome);
    PrintStructValueH (pStruct, Type18, MemoryArrayErrorAddress);
    PrintStructValueH (pStruct, Type18, DeviceErrorAddress);
    PrintStructValueH (pStruct, Type18, ErrorResolution);
    break;

  //
  // Memory Array Mapped Address (Type 19)
  //
  case 19:
    PrintStructValueH (pStruct, Type19, StartingAddress);
    PrintStructValueH (pStruct, Type19, EndingAddress);
    PrintStructValueH (pStruct, Type19, MemoryArrayHandle);
    PrintStructValueH (pStruct, Type19, PartitionWidth);
    break;

  //
  // Memory Device Mapped Address  (Type 20)
  //
  case 20:
    PrintStructValueH (pStruct, Type20, StartingAddress);
    PrintStructValueH (pStruct, Type20, EndingAddress);
    PrintStructValueH (pStruct, Type20, MemoryDeviceHandle);
    PrintStructValueH (pStruct, Type20, MemoryArrayMappedAddressHandle);
    PrintStructValueH (pStruct, Type20, PartitionRowPosition);
    PrintStructValueH (pStruct, Type20, InterleavePosition);
    PrintStructValueH (pStruct, Type20, InterleavedDataDepth);
    break;

  //
  // Built-in Pointing Device  (Type 21)
  //
  case 21:
    DisplayPointingDeviceType (pStruct->Type21->Type, Option);
    DisplayPointingDeviceInterface (pStruct->Type21->Interface, Option);
    PrintStructValue (pStruct, Type21, NumberOfButtons);
    break;

  //
  // Portable Battery  (Type 22)
  //
  case 22:
    PrintPendingString (pStruct, Type22, Location);
    PrintPendingString (pStruct, Type22, Manufacturer);
    PrintPendingString (pStruct, Type22, ManufactureDate);
    PrintPendingString (pStruct, Type22, SerialNumber);
    PrintPendingString (pStruct, Type22, DeviceName);
    DisplayPBDeviceChemistry (
      pStruct->Type22->DeviceChemistry,
      Option
      );
    PrintStructValueH (pStruct, Type22, DeviceCapacity);
    PrintStructValueH (pStruct, Type22, DesignVoltage);
    PrintPendingString (pStruct, Type22, SBDSVersionNumber);
    PrintStructValueH (pStruct, Type22, MaximumErrorInBatteryData);
    PrintStructValueH (pStruct, Type22, SBDSSerialNumber);
    DisplaySBDSManufactureDate (
      pStruct->Type22->SBDSManufactureDate,
      Option
      );
    PrintPendingString (pStruct, Type22, SBDSDeviceChemistry);
    PrintStructValueH (pStruct, Type22, DesignCapacityMultiplier);
    PrintStructValueH (pStruct, Type22, OEMSpecific);
    break;

  case 23:
    DisplaySystemResetCapabilities (
      pStruct->Type23->Capabilities,
      Option
      );
    PrintStructValueH (pStruct, Type23, ResetCount);
    PrintStructValueH (pStruct, Type23, ResetLimit);
    PrintStructValueH (pStruct, Type23, TimerInterval);
    PrintStructValueH (pStruct, Type23, Timeout);
    break;

  case 24:
    DisplayHardwareSecuritySettings (
      pStruct->Type24->HardwareSecuritySettings,
      Option
      );
    break;

  case 25:
    PrintStructValueH (pStruct, Type25, NextScheduledPowerOnMonth);
    PrintStructValueH (pStruct, Type25, NextScheduledPowerOnDayOfMonth);
    PrintStructValueH (pStruct, Type25, NextScheduledPowerOnHour);
    PrintStructValueH (pStruct, Type25, NextScheduledPowerOnMinute);
    PrintStructValueH (pStruct, Type25, NextScheduledPowerOnSecond);
    break;

  case 26:
    PrintPendingString (pStruct, Type26, Description);
    DisplayVPLocation (pStruct->Type26->LocationAndStatus, Option);
    DisplayVPStatus (pStruct->Type26->LocationAndStatus, Option);
    PrintStructValueH (pStruct, Type26, MaximumValue);
    PrintStructValueH (pStruct, Type26, MinimumValue);
    PrintStructValueH (pStruct, Type26, Resolution);
    PrintStructValueH (pStruct, Type26, Tolerance);
    PrintStructValueH (pStruct, Type26, Accuracy);
    PrintStructValueH (pStruct, Type26, OEMDefined);
    PrintStructValueH (pStruct, Type26, NominalValue);
    break;

  case 27:
    PrintStructValueH (pStruct, Type27, TemperatureProbeHandle);
    DisplayCoolingDeviceStatus (pStruct->Type27->DeviceTypeAndStatus, Option);
    DisplayCoolingDeviceType (pStruct->Type27->DeviceTypeAndStatus, Option);
    PrintStructValueH (pStruct, Type27, CoolingUnitGroup);
    PrintStructValueH (pStruct, Type27, OEMDefined);
    PrintStructValueH (pStruct, Type27, NominalSpeed);
    break;

  case 28:
    PrintPendingString (pStruct, Type28, Description);
    DisplayTemperatureProbeStatus (pStruct->Type28->LocationAndStatus, Option);
    DisplayTemperatureProbeLoc (pStruct->Type28->LocationAndStatus, Option);
    PrintStructValueH (pStruct, Type28, MaximumValue);
    PrintStructValueH (pStruct, Type28, MinimumValue);
    PrintStructValueH (pStruct, Type28, Resolution);
    PrintStructValueH (pStruct, Type28, Tolerance);
    PrintStructValueH (pStruct, Type28, Accuracy);
    PrintStructValueH (pStruct, Type28, OEMDefined);
    PrintStructValueH (pStruct, Type28, NominalValue);
    break;

  case 29:
    PrintPendingString (pStruct, Type29, Description);
    DisplayECPStatus (pStruct->Type29->LocationAndStatus, Option);
    DisplayECPLoc (pStruct->Type29->LocationAndStatus, Option);
    PrintStructValueH (pStruct, Type29, MaximumValue);
    PrintStructValueH (pStruct, Type29, MinimumValue);
    PrintStructValueH (pStruct, Type29, Resolution);
    PrintStructValueH (pStruct, Type29, Tolerance);
    PrintStructValueH (pStruct, Type29, Accuracy);
    PrintStructValueH (pStruct, Type29, OEMDefined);
    PrintStructValueH (pStruct, Type29, NominalValue);
    break;

  case 30:
    PrintPendingString (pStruct, Type30, ManufacturerName);
    DisplayOBRAConnections (pStruct->Type30->Connections, Option);
    break;

  case 31:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_STRUCT_TYPE31), HiiHandle);
    break;

  case 32:
    PrintBitField (pStruct, Type32, Reserved, 6);
    DisplaySystemBootStatus (pStruct->Type32->BootStatus[0], Option);
    break;

  case 33:
    DisplayMemoryErrorType (pStruct->Type33->ErrorType, Option);
    DisplayMemoryErrorGranularity (
      pStruct->Type33->ErrorGranularity,
      Option
      );
    DisplayMemoryErrorOperation (pStruct->Type33->ErrorOperation, Option);
    PrintStructValueH (pStruct, Type33, VendorSyndrome);
    PrintStructValueH (pStruct, Type33, MemoryArrayErrorAddress);
    PrintStructValueH (pStruct, Type33, DeviceErrorAddress);
    PrintStructValueH (pStruct, Type33, ErrorResolution);
    break;

  //
  // Management Device  (Type 34)
  //
  case 34:
    PrintPendingString (pStruct, Type34, Description);
    DisplayMDType (pStruct->Type34->Type, Option);
    PrintStructValueH (pStruct, Type34, Address);
    PrintStructValueH (pStruct, Type34, AddressType);
    break;

  case 35:
    PrintPendingString (pStruct, Type35, Description);
    PrintStructValueH (pStruct, Type35, ManagementDeviceHandle);
    PrintStructValueH (pStruct, Type35, ComponentHandle);
    PrintStructValueH (pStruct, Type35, ThresholdHandle);
    break;

  case 36:
    PrintStructValueH (pStruct, Type36, LowerThresholdNonCritical);
    PrintStructValueH (pStruct, Type36, UpperThresholdNonCritical);
    PrintStructValueH (pStruct, Type36, LowerThresholdCritical);
    PrintStructValueH (pStruct, Type36, UpperThreaholdCritical);
    PrintStructValueH (pStruct, Type36, LowerThresholdNonRecoverable);
    PrintStructValueH (pStruct, Type36, UpperThresholdNonRecoverable);
    break;

  //
  // Memory Channel  (Type 37)
  //
  case 37:
    {
      UINT8         Count;
      MEMORYDEVICE  *Ptr;
      DisplayMemoryChannelType (pStruct->Type37->ChannelType, Option);
      PrintStructValueH (pStruct, Type37, MaximumChannelLoad);
      PrintStructValueH (pStruct, Type37, MemoryDeviceCount);

      Count = pStruct->Type37->MemoryDeviceCount;
      Ptr   = pStruct->Type37->MemoryDevice;
      for (Index = 0; Index < Count; Index++) {
        PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_MEM_DEVICE), HiiHandle, Index + 1);
        PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_DEV_LOAD), HiiHandle, Ptr->DeviceLoad);
        PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_DEV_HANDLE), HiiHandle, Ptr->DeviceHandle);
      }
    }
    break;

  //
  // IPMI Device Information  (Type 38)
  //
  case 38:
    DisplayIPMIDIBMCInterfaceType (pStruct->Type38->InterfaceType, Option);
    PrintStructValueH (pStruct, Type38, IPMISpecificationRevision);
    PrintStructValueH (pStruct, Type38, I2CSlaveAddress);
    PrintStructValueH (pStruct, Type38, NVStorageDeviceAddress);
    PrintStructValueH (pStruct, Type38, BaseAddress);
    break;

  //
  // System Power Supply (Type 39)
  //
  case 39:
    PrintStructValueH (pStruct, Type39, PowerUnitGroup);
    PrintPendingString (pStruct, Type39, Location);
    PrintPendingString (pStruct, Type39, DeviceName);
    PrintPendingString (pStruct, Type39, Manufacturer);
    PrintPendingString (pStruct, Type39, SerialNumber);
    PrintPendingString (pStruct, Type39, AssetTagNumber);
    PrintPendingString (pStruct, Type39, ModelPartNumber);
    PrintPendingString (pStruct, Type39, RevisionLevel);
    PrintStructValueH (pStruct, Type39, MaxPowerCapacity);
    DisplaySPSCharacteristics (
      pStruct->Type39->PowerSupplyCharacteristics,
      Option
      );
    PrintStructValueH (pStruct, Type39, InputVoltageProbeHandle);
    PrintStructValueH (pStruct, Type39, CoolingDeviceHandle);
    PrintStructValueH (pStruct, Type39, InputCurrentProbeHandle);
    break;

  case 126:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_INACTIVE_STRUCT), HiiHandle);
    break;

  case 127:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_THIS_STRUCT_END_TABLE), HiiHandle);
    break;

  default:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_STRUCT_TYPE_UNDEFINED), HiiHandle);
    break;
  }

  return EFI_SUCCESS;
}

VOID
DisplayBiosCharacteristics (
  UINT64  chara,
  UINT8   Option
  )
{
  //
  // Print header
  //
  PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_BIOS_CHAR), HiiHandle);
  //
  // print option
  //
  PRINT_INFO_OPTION (chara, Option);

  //
  // Check all the bits and print information
  // This function does not use Table because table of bits
  //   are designed not to deal with UINT64
  //
  if (BIT (chara, 0) != 0) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_RESERVED_BIT), HiiHandle);
  }

  if (BIT (chara, 1) != 0) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_RESERVED_BIT), HiiHandle);
  }

  if (BIT (chara, 2) != 0) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_UNKNOWN_BIT), HiiHandle);
  }

  if (BIT (chara, 3) != 0) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_BIOS_CHAR_NOT_SUPPORTED), HiiHandle);
  }

  if (BIT (chara, 4) != 0) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_ISA_SUPPORTED), HiiHandle);
  }

  if (BIT (chara, 5) != 0) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_MSA_SUPPORTED), HiiHandle);
  }

  if (BIT (chara, 6) != 0) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_EISA_SUPPORTED), HiiHandle);
  }

  if (BIT (chara, 7) != 0) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_PCI_SUPPORTED), HiiHandle);
  }

  if (BIT (chara, 8) != 0) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_PC_CARD_SUPPORTED), HiiHandle);
  }

  if (BIT (chara, 9) != 0) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_PLUG_PLAY_SUPPORTED), HiiHandle);
  }

  if (BIT (chara, 10) != 0) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_APM_SUPPORTED), HiiHandle);
  }

  if (BIT (chara, 11) != 0) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_BIOS_UPGRADEABLE), HiiHandle);
  }

  if (BIT (chara, 12) != 0) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_BIOS_SHADOWING), HiiHandle);
  }

  if (BIT (chara, 13) != 0) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_VESA_SUPPORTED), HiiHandle);
  }

  if (BIT (chara, 14) != 0) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_ECSD_SUPPORT), HiiHandle);
  }

  if (BIT (chara, 15) != 0) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_BOOT_FORM_CD_SUPPORTED), HiiHandle);
  }

  if (BIT (chara, 16) != 0) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_SELECTED_BOOT_SUPPORTED), HiiHandle);
  }

  if (BIT (chara, 17) != 0) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_BIOS_ROM_SOCKETED), HiiHandle);
  }

  if (BIT (chara, 18) != 0) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_BOOT_FROM_PC_CARD), HiiHandle);
  }

  if (BIT (chara, 19) != 0) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_EDD_ENHANCED_DRIVER), HiiHandle);
  }

  if (BIT (chara, 20) != 0) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_JAPANESE_FLOPPY_NEC), HiiHandle);
  }

  if (BIT (chara, 21) != 0) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_JAPANESE_FLOPPY_TOSHIBA), HiiHandle);
  }

  if (BIT (chara, 22) != 0) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_FLOPPY_SERVICES_SUPPORTED), HiiHandle);
  }

  if (BIT (chara, 23) != 0) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_ONE_POINT_TWO_MB), HiiHandle);
  }

  if (BIT (chara, 24) != 0) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_720_KB), HiiHandle);
  }

  if (BIT (chara, 25) != 0) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_TWO_POINT_EIGHT_EIGHT_MB), HiiHandle);
  }

  if (BIT (chara, 26) != 0) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_PRINT_SCREEN_SUPPORT), HiiHandle);
  }

  if (BIT (chara, 27) != 0) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_KEYBOARD_SERV_SUPPORT), HiiHandle);
  }

  if (BIT (chara, 28) != 0) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_SERIAL_SERVICES_SUPPORT), HiiHandle);
  }

  if (BIT (chara, 29) != 0) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_PRINTER_SERVICES_SUPPORT), HiiHandle);
  }

  if (BIT (chara, 30) != 0) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_MONO_VIDEO_SUPPORT), HiiHandle);
  }

  if (BIT (chara, 31) != 0) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_NEC_PC_98), HiiHandle);
  }
  //
  // Just print the reserved
  //
  PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_BITS_32_47), HiiHandle);
  PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_BITS_48_64), HiiHandle);
}

VOID
DisplayBiosCharacteristicsExt1 (
  UINT8 byte1,
  UINT8 Option
  )
{
  PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_BIOS_CHAR_EXTENSION), HiiHandle);
  //
  // Print option
  //
  PRINT_INFO_OPTION (byte1, Option);

  //
  // check bit and print
  //
  if (BIT (byte1, 0) != 0) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_ACPI_SUPPORTED), HiiHandle);
  }

  if (BIT (byte1, 1) != 0) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_USB_LEGACY_SUPPORTED), HiiHandle);
  }

  if (BIT (byte1, 2) != 0) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_AGP_SUPPORTED), HiiHandle);
  }

  if (BIT (byte1, 3) != 0) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_I2O_BOOT_SUPPORTED), HiiHandle);
  }

  if (BIT (byte1, 4) != 0) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_LS_120_BOOT_SUPPORTED), HiiHandle);
  }

  if (BIT (byte1, 5) != 0) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_ATAPI_ZIP_DRIVE), HiiHandle);
  }

  if (BIT (byte1, 6) != 0) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_1394_BOOT_SUPPORTED), HiiHandle);
  }

  if (BIT (byte1, 7) != 0) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_SMART_BATTERY_SUPPORTED), HiiHandle);
  }
}

VOID
DisplayBiosCharacteristicsExt2 (
  UINT8 byte2,
  UINT8 Option
  )
{
  PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_BIOS_CHAR_EXTENSION_2), HiiHandle);
  //
  // Print option
  //
  PRINT_INFO_OPTION (byte2, Option);

  if (BIT (byte2, 0) != 0) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_BIOS_BOOT_SPEC_SUPP), HiiHandle);
  }

  if (BIT (byte2, 1) != 0) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_FUNCTION_KEY_INIT), HiiHandle);
  }

  PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_BITS_RSVD_FOR_FUTURE), HiiHandle);
}

VOID
DisplayProcessorFamily (
  UINT8 Family,
  UINT8 Option
  )
{
  //
  // Print prompt message
  //
  PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_PROCESSOR_FAMILY), HiiHandle);
  //
  // Print option
  //
  PRINT_INFO_OPTION (Family, Option);

  //
  // Use switch to check
  //
  switch (Family) {
  case 0x01:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_OTHER), HiiHandle);
    break;

  case 0x02:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_UNKNOWN), HiiHandle);
    break;

  case 0x03:
    Print (L"8086\n");
    break;

  case 0x04:
    Print (L"80286\n");
    break;

  case 0x05:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_INTEL386_PROCESSOR), HiiHandle);
    break;

  case 0x06:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_INTEL486_PROCESSOR), HiiHandle);
    break;

  case 0x07:
    Print (L"8087\n");
    break;

  case 0x08:
    Print (L"80287\n");
    break;

  case 0x09:
    Print (L"80387\n");
    break;

  case 0x0A:
    Print (L"80487\n");
    break;

  case 0x0B:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_PENTIUM_PROC_FAMILY), HiiHandle);
    break;

  case 0x0C:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_PENTIUM_PRO_PROC), HiiHandle);
    break;

  case 0x0D:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_PENTIUM_II_PROC), HiiHandle);
    break;

  case 0x0E:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_PENTIUM_PROC_MMX), HiiHandle);
    break;

  case 0x0F:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_CELERON_PROC), HiiHandle);
    break;

  case 0x10:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_PENTIUM_XEON_PROC), HiiHandle);
    break;

  case 0x11:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_PENTIUM_III_PROC), HiiHandle);
    break;

  case 0x12:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_M1_FAMILY), HiiHandle);
    break;

  case 0x19:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_K5_FAMILY), HiiHandle);
    break;

  case 0x20:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_POWER_PC_FAMILY), HiiHandle);
    break;

  case 0x21:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_POWER_PC_601), HiiHandle);
    break;

  case 0x22:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_POWER_PC_603), HiiHandle);
    break;

  case 0x23:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_POWER_PC_603_PLUS), HiiHandle);
    break;

  case 0x24:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_POWER_PC_604), HiiHandle);
    break;

  case 0x30:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_ALPHA_FAMILY_2), HiiHandle);
    break;

  case 0x40:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_MIPS_FAMILY), HiiHandle);
    break;

  case 0x50:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_SPARC_FAMILY), HiiHandle);
    break;

  case 0x60:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_68040_FAMILY), HiiHandle);
    break;

  case 0x61:
    Print (L"68xx\n");
    break;

  case 0x62:
    Print (L"68000\n");
    break;

  case 0x63:
    Print (L"68010\n");
    break;

  case 0x64:
    Print (L"68020\n");
    break;

  case 0x65:
    Print (L"68030\n");
    break;

  case 0x70:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_HOBBIT_FAMILY), HiiHandle);
    break;

  case 0x80:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_WEITEK), HiiHandle);
    break;

  case 0x90:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_PA_RISC_FAMILY), HiiHandle);
    break;

  case 0xA0:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_V30_FAMILY), HiiHandle);
    break;

  case 0xB0:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_PENTIUM_III_XEON), HiiHandle);
    break;

  default:
    //
    // In order to reduce code quality notice of
    // case & break not pair, so
    // move multiple case into the else part and
    // use if/else to check value.
    //
    if (Family >= 0x13 && Family <= 0x18) {
      PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_RSVD_FOR_SPEC_M1), HiiHandle);
    } else if (Family >= 0x1A && Family <= 0x1F) {
      PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_RSVD_FOR_SPEC_K5), HiiHandle);
    } else if (Family >= 0xB1 && Family <= 0xBF) {
      PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_RSVD_FOR_SPEC_PENTIUM), HiiHandle);
    } else {
      PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_UNDEFINED_PROC_FAMILY), HiiHandle);
    }
  }
  //
  // end switch
  //
}

VOID
DisplayProcessorVoltage (
  UINT8 Voltage,
  UINT8 Option
  )
/*++
Routine Description:
  Bit 7 Set to 0, indicating 'legacy' mode for processor voltage
  Bits 6:4  Reserved, must be zero
  Bits 3:0  Voltage Capability. 
            A Set bit indicates that the voltage is supported.
    Bit 0 - 5V
    Bit 1 - 3.3V
    Bit 2 - 2.9V
    Bit 3 - Reserved, must be zero. 

  Note: 
    Setting of multiple bits indicates the socket is configurable
    If bit 7 is set to 1, the remaining seven bits of the field are set to 
    contain the processor's current voltage times 10.  
    For example, the field value for a processor voltage of 1.8 volts would be 
    92h = 80h + (1.8 * 10) = 80h + 18 = 80h +12h.

Arguments:
  Voltage  - The Voltage
  Option   - The option
  
Returns:

--*/
{
  PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_PROC_INFO), HiiHandle);
  //
  // Print option
  //
  PRINT_INFO_OPTION (Voltage, Option);

  if (BIT (Voltage, 7) != 0) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_PROC_CURRENT_VOLTAGE), HiiHandle, (Voltage - 0x80));
  } else {
    if (BIT (Voltage, 0) != 0) {
      PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_5V_SUPOPRTED), HiiHandle);
    }

    if (BIT (Voltage, 1) != 0) {
      PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_33V_SUPPORTED), HiiHandle);
    }

    if (BIT (Voltage, 2) != 0) {
      PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_29V_SUPPORTED), HiiHandle);
    }
    //
    // check the reserved zero bits:
    //
    if (BIT (Voltage, 3) != 0) {
      PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_BIT3_NOT_ZERO), HiiHandle);
    }

    if (BIT (Voltage, 4) != 0) {
      PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_BIT4_NOT_ZERO), HiiHandle);
    }

    if (BIT (Voltage, 5) != 0) {
      PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_BIT5_NOT_ZERO), HiiHandle);
    }

    if (BIT (Voltage, 6) != 0) {
      PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_BIT6_NOT_ZERO), HiiHandle);
    }
  }
}

VOID
DisplayProcessorStatus (
  UINT8 Status,
  UINT8 Option
  )
/*++
Routine Description:

Bit 7 Reserved, must be 0
Bit 6   CPU Socket Populated  
 1 - CPU Socket Populated 
 0 - CPU Socket UnpopulatedBits 
 5:3  Reserved, must be zero
 Bits 2:0 CPU Status  
  0h - Unknown  
  1h - CPU Enabled  
  2h - CPU Disabled by User via BIOS Setup  
  3h - CPU Disabled By BIOS (POST Error)  
  4h - CPU is Idle, waiting to be enabled.  
  5-6h - Reserved   
  7h - Other

Arguments:
  Status  - The status
  Option  - The option
  
Returns:

--*/
{
  PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_PROC_STATUS), HiiHandle);
  PRINT_INFO_OPTION (Status, Option);

  if (BIT (Status, 7) != 0) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_ERROR_BIT7), HiiHandle);
  } else if (BIT (Status, 5) != 0) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_ERROR_BIT5), HiiHandle);
  } else if (BIT (Status, 4) != 0) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_ERROR_BIT4), HiiHandle);
  } else if (BIT (Status, 3) != 0) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_ERROR_BIT3), HiiHandle);
  }
  //
  // Check BIT 6
  //
  if (BIT (Status, 6) != 0) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_CPU_SOCKET_POPULATED), HiiHandle);
  } else {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_CPU_SOCKET_UNPOPULATED), HiiHandle);
  }
  //
  // Check BITs 2:0
  //
  switch (Status & 0x07) {
  case 0:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_UNKNOWN), HiiHandle);
    break;

  case 1:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_CPU_ENABLED), HiiHandle);
    break;

  case 2:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_CPU_DISABLED_BY_USER), HiiHandle);
    break;

  case 3:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_CPU_DIABLED_BY_BIOS), HiiHandle);
    break;

  case 4:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_CPU_IDLE), HiiHandle);
    break;

  case 7:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_OTHERS), HiiHandle);
    break;

  default:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_RESERVED), HiiHandle);
  }
}

VOID
DisplayMaxMemoryModuleSize (
  UINT8 Size,
  UINT8 SlotNum,
  UINT8 Option
  )
{
  UINTN MaxSize;
  PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_SIZE_LARGEST_MEM), HiiHandle);
  //
  // MaxSize is determined by follow formula
  //
  MaxSize = 1 << Size;
  PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_ONE_VAR_MB), HiiHandle, MaxSize);

  if (Option >= SHOW_DETAIL) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_MAX_AMOUNT_MEM), HiiHandle);
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_ONE_VAR_MB), HiiHandle, MaxSize, SlotNum, MaxSize * SlotNum);
  }
}

VOID
DisplayMemoryModuleConfigHandles (
  UINT16 *Handles,
  UINT8  SlotNum,
  UINT8  Option
  )
{
  UINT8 Index;
  PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_HANDLES_CONTROLLED), HiiHandle, SlotNum);

  if (Option >= SHOW_DETAIL) {
    //
    // No handle, Handles is INVALID.
    //
    if (SlotNum == 0) {
      return ;
    }

    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_HANDLES_LIST_CONTROLLED), HiiHandle);
    for (Index = 0; Index < SlotNum; Index++) {
      PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_HANDLE), HiiHandle, Index + 1, Handles[Index]);
    }
  }
}
//
// Memory Module Information (Type 6)
//
VOID
DisplayMmBankConnections (
  UINT8 BankConnections,
  UINT8 Option
  )
{
  UINT8 High;
  UINT8 Low;

  PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_BANK_CONNECTIONS), HiiHandle);
  //
  // Print option
  //
  PRINT_INFO_OPTION (BankConnections, Option);

  //
  // Divide it to high and low
  //
  High  = (UINT8) (BankConnections & 0xF0);
  Low   = (UINT8) (BankConnections & 0x0F);
  if (High != 0xF) {
    if (Low != 0xF) {
      PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_BANK_RAS), HiiHandle, High, Low, High, Low);
    } else {
      PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_BANK_RAS_2), HiiHandle, High, High);
    }
  } else {
    if (Low != 0xF) {
      PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_BANK_RAS_2), HiiHandle, Low, Low);
    } else {
      PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_NO_BANKS_CONNECTED), HiiHandle);
    }
  }
}

VOID
DisplayMmMemorySize (
  UINT8 Size,
  UINT8 Option
  )
/*++
Routine Description:
  Bits 0:6  Size (n), 
      where 2**n is the size in MB with three special-case values:
      7Dh Not determinable (Installed Size only)
      7Eh Module is installed, but no memory has been enabled
      7Fh Not installed
  Bit  7  Defines whether the memory module has a single- (0) 
          or double-bank (1) connection.
          
Arguments:
  Size   - The size
  Option - The option
  
Returns:

--*/
{
  UINT8 Value;
  PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_MEMORY_SIZE), HiiHandle);
  //
  // Print option
  //
  PRINT_INFO_OPTION (Size, Option);

  //
  // Get the low bits(0-6 bit)
  //
  Value = (UINT8) (Size & 0x7F);
  if (Value == 0x7D) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_MEM_SIZE_NOT_DETERMINABLE), HiiHandle);
  } else if (Value == 0x7E) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_MODULE_INSTALLED), HiiHandle);
  } else if (Value == 0x7F) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_NOT_INSTALLED), HiiHandle);
  } else {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_MEM_SIZE), HiiHandle, 1 << Value);
  }

  if (BIT (Size, 7) != 0) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_MEM_MODULE_DOUBLE_BANK), HiiHandle);
  } else {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_MEM_MODULE_SINGLE_BANK), HiiHandle);
  }
}

VOID
DisplaySystemSlotId (
  UINT16  SlotId,
  UINT8   SlotType,
  UINT8   Option
  )
/*++
Routine Description:

  The Slot ID field of the System Slot structure provides a mechanism to 
  correlate the physical attributes of the slot to its logical access method 
  (which varies based on the Slot Type field). 

Arguments:

  SlotId   - The slot ID
  SlotType - The slot type
  Option   - The Option
  
Returns:

--*/
{
  //
  // Display slot type first
  //
  DisplaySystemSlotType (SlotType, Option);

  PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_SLOT_ID), HiiHandle);
  //
  // print option
  //
  PRINT_INFO_OPTION (SlotType, Option);

  switch (SlotType) {
  //
  // Slot Type: MCA
  //
  case 0x04:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_LOGICAL_MICRO_CHAN), HiiHandle);
    if (SlotId > 0 && SlotId < 15) {
      PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_ONE_VAR_D), HiiHandle, SlotId);
    } else {
      PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_ERROR_NOT_1_15), HiiHandle);
    }
    break;

  //
  // EISA
  //
  case 0x05:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_LOGICAL_EISA_NUM), HiiHandle);
    if (SlotId > 0 && SlotId < 15) {
      PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_ONE_VAR_D), HiiHandle, SlotId);
    } else {
      PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_ERROR_NOT_1_15), HiiHandle);
    }
    break;

  //
  // Slot Type: PCI
  //
  case 0x06:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_VALUE_PRESENT), HiiHandle, SlotId);
    break;

  //
  // PCMCIA
  //
  case 0x07:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_IDENTIFIES_ADAPTER_NUM), HiiHandle, SlotId);
    break;
	
  //
  // Slot Type: PCI-E
  //
  case 0xA5:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_VALUE_PRESENT), HiiHandle, SlotId);
    break;

  default:
    if (SlotType >= 0x0E && SlotType <= 0x12) {
      PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_VALUE_PRESENT), HiiHandle, SlotId);
    } else {
      PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_UNDEFINED_SLOT_ID), HiiHandle);
    }
  }
}

VOID
DisplaySystemBootStatus (
  UINT8 Parameter,
  UINT8 Option
  )
{
  PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_SYSTEM_BOOT_STATUS), HiiHandle);
  //
  // Print option
  //
  PRINT_INFO_OPTION (Parameter, Option);

  //
  // Check value and print
  //
  if (Parameter == 0) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_NO_ERRORS_DETECTED), HiiHandle);
  } else if (Parameter == 1) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_NO_BOOTABLE_MEDIA), HiiHandle);
  } else if (Parameter == 2) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_NORMAL_OP_SYSTEM), HiiHandle);
  } else if (Parameter == 3) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_FIRMWARE_DETECTED), HiiHandle);
  } else if (Parameter == 4) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_OP_SYSTEM), HiiHandle);
  } else if (Parameter == 5) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_USER_REQUESTED_BOOT), HiiHandle);
  } else if (Parameter == 6) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_SYSTEM_SECURITY_VIOLATION), HiiHandle);
  } else if (Parameter == 7) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_PREV_REQ_IMAGE), HiiHandle);
  } else if (Parameter == 8) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_WATCHDOG_TIMER), HiiHandle);
  } else if (Parameter >= 9 && Parameter <= 127) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_RSVD_FUTURE_ASSIGNMENT), HiiHandle);
  } else if (Parameter >= 128 && Parameter <= 191) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_VENDOR_OEM_SPECIFIC), HiiHandle);
  } else if (Parameter >= 192 && Parameter <= 255) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_PRODUCT_SPEC_IMPLMENTATION), HiiHandle);
  } else {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_ERROR_VALUE), HiiHandle);
  }
}
//
// Portable Battery (Type 22)
//
VOID
DisplaySBDSManufactureDate (
  UINT16  Date,
  UINT8   Option
  )
/*++
Routine Description:
  The date the cell pack was manufactured, in packed format:
   Bits 15:9  Year, biased by 1980, in the range 0 to 127.
   Bits 8:5 Month, in the range 1 to 12.
   Bits 4:0 Date, in the range 1 to 31.
  For example, 01 February 2000 would be identified as 
  0010 1000 0100 0001b (0x2841).

Arguments:
  Date   - The date
  Option - The option
  
Returns:

--*/
{
  UINTN Day;
  UINTN Month;
  UINTN Year;

  PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_SBDS_MANUFACTURE_DATE), HiiHandle);
  PRINT_INFO_OPTION (Date, Option);
  //
  // Print date
  //
  Day   = Date & 0x001F;
  Month = (Date & 0x00E0) >> 5;
  Year  = ((Date & 0xFF00) >> 8) + 1980;
  PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_MONTH_DAY_YEAR), HiiHandle, Day, Month, Year);

}
//
// System Reset  (Type 23)
//
VOID
DisplaySystemResetCapabilities (
  UINT8 Reset,
  UINT8 Option
  )
/*++
Routine Description:
Identifies the system-reset capabilities for the system.
 Bits 7:6 Reserved for future assignment via this specification, set to 00b.
 Bit 5  System contains a watchdog timer, either True (1) or False (0).
 Bits 4:3 Boot Option on Limit.  
  Identifies the system action to be taken when the Reset Limit is reached, one of:
  00b Reserved, do not use. 
  01b Operating system  
  10b System utilities  
  11b Do not rebootBits 
 2:1  Boot Option.  Indicates the action to be taken following a watchdog reset, one of:    
  00b Reserved, do not use. 
  01b Operating system  
  10b System utilities  
  11b Do not reboot
 Bit 0  Status.  
  1b The system reset is enabled by the user
  0b The system reset is not enabled by the user

Arguments:
  Reset   - Reset
  Option  - The option
  
Returns:

--*/
{
  UINTN Temp;

  PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_SYSTEM_RESET_CAPABILITIES), HiiHandle);
  PRINT_INFO_OPTION (Reset, Option);

  //
  // Check reserved bits 7:6
  //
  if ((Reset & 0xC0) != 0) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_BITS_RESERVED_ZERO), HiiHandle);
  }
  //
  // Watch dog
  //
  if (BIT (Reset, 5) != 0) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_WATCHDOG_TIMER_2), HiiHandle);
  } else {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_SYSTEM_NOT_CONTAIN_TIMER), HiiHandle);
  }
  //
  // Boot Option on Limit
  //
  Temp = (Reset & 0x18) >> 3;
  PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_BOOT_OPTION_LIMIT), HiiHandle);
  switch (Temp) {
  case 0:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_RESERVED), HiiHandle);
    break;

  case 1:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_OP_SYSTEM_2), HiiHandle);
    break;

  case 2:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_SYSTEM_UTIL), HiiHandle);
    break;

  case 3:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_DO_NOT_REBOOT_BITS), HiiHandle);
    break;
  }
  //
  // Boot Option
  //
  Temp = (Reset & 0x06) >> 1;
  PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_BOOT_OPTION), HiiHandle);
  switch (Temp) {
  case 0:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_RESERVED), HiiHandle);
    break;

  case 1:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_OP_SYSTEM_2), HiiHandle);
    break;

  case 2:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_SYSTEM_UTIL), HiiHandle);
    break;

  case 3:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_DO_NOT_REBOOT), HiiHandle);
    break;
  }
  //
  // Reset enable flag
  //
  if ((Reset & 0x01) != 0) {
    Print (L"The system reset is enabled by the user\n");
  } else {
    Print (L"The system reset is disabled by the user\n");
  }
}
//
// Hardware Security (Type 24)
//
VOID
DisplayHardwareSecuritySettings (
  UINT8 Settings,
  UINT8 Option
  )
/*++
Routine Description:
Identifies the password and reset status for the system:

Bits 7:6    Power-on Password Status, one of:   
  00b Disabled    
  01b Enabled   
  10b Not Implemented   
  11b Unknown 
Bits 5:4    Keyboard Password Status, one of:   
  00b Disabled    
  01b Enabled   
  10b Not Implemented   
  11b Unknown 
Bits 3:2    Administrator Password Status, one  of:   
  00b Disabled    
  01b Enabled   
  10b Not Implemented   
  11b Unknown
Bits 1:0    Front Panel Reset Status, one of:   
  00b Disabled    
  01b Enabled   
  10b Not Implemented   
  11b Unknown

Arguments:
  Settings    - The settings
  Option      - the option
  
Returns:

--*/
{
  UINTN Temp;

  PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_HARDWARE_SECURITY_SET), HiiHandle);
  PRINT_INFO_OPTION (Settings, Option);

  //
  // Power-on Password Status
  //
  Temp = (Settings & 0xC0) >> 6;
  PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_POWER_ON_PASSWORD), HiiHandle);
  switch (Temp) {
  case 0:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_DISABLED), HiiHandle);
    break;

  case 1:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_ENABLED_NEWLINE), HiiHandle);
    break;

  case 2:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_NOT_IMPLEMENTED), HiiHandle);
    break;

  case 3:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_UNKNOWN), HiiHandle);
    break;
  }
  //
  // Keyboard Password Status
  //
  Temp = (Settings & 0x30) >> 4;
  PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_KEYBOARD_PASSWORD), HiiHandle);
  switch (Temp) {
  case 0:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_DISABLED), HiiHandle);
    break;

  case 1:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_ENABLED_NEWLINE), HiiHandle);
    break;

  case 2:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_NOT_IMPLEMENTED), HiiHandle);
    break;

  case 3:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_UNKNOWN), HiiHandle);
    break;
  }
  //
  // Administrator Password Status
  //
  Temp = (Settings & 0x0C) >> 2;
  PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_ADMIN_PASSWORD_STATUS), HiiHandle);
  switch (Temp) {
  case 0:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_DISABLED), HiiHandle);
    break;

  case 1:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_ENABLED_NEWLINE), HiiHandle);
    break;

  case 2:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_NOT_IMPLEMENTED), HiiHandle);
    break;

  case 3:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_UNKNOWN), HiiHandle);
    break;
  }
  //
  // Front Panel Reset Status
  //
  Temp = Settings & 0x3;
  PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_FRONT_PANEL_RESET), HiiHandle);
  switch (Temp) {
  case 0:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_DISABLED), HiiHandle);
    break;

  case 1:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_ENABLED_NEWLINE), HiiHandle);
    break;

  case 2:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_NOT_IMPLEMENTED), HiiHandle);
    break;

  case 3:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_UNKNOWN), HiiHandle);
    break;
  }
}
//
// Out-of-Band Remote Access (Type 30)
//
VOID
DisplayOBRAConnections (
  UINT8   Connections,
  UINT8   Option
  )
{
  PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_CONNECTIONS), HiiHandle);
  PRINT_INFO_OPTION (Connections, Option);

  //
  // Check reserved bits 7:2
  //
  if ((Connections & 0xFC) != 0) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_BITS_RESERVED_ZERO_2), HiiHandle);
  }
  //
  // Outbound Connection
  //
  if (BIT (Connections, 1) != 0) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_OUTBOUND_CONN_ENABLED), HiiHandle);
  } else {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_OTUBOUND_CONN_DISABLED), HiiHandle);
  }
  //
  // Inbound Connection
  //
  if (BIT (Connections, 0) != 0) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_INBOIUND_CONN_ENABLED), HiiHandle);
  } else {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_INBOUND_CONN_DISABLED), HiiHandle);
  }
}
//
// System Power Supply (Type 39)
//
VOID
DisplaySPSCharacteristics (
  UINT16  Characteristics,
  UINT8   Option
  )
{
  UINTN Temp;

  PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_POWER_SUPPLY_CHAR), HiiHandle);
  PRINT_INFO_OPTION (Characteristics, Option);

  //
  // Check reserved bits 15:14
  //
  if ((Characteristics & 0xC000) != 0) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_BITS_15_14_RSVD), HiiHandle);
  }
  //
  // Bits 13:10 - DMTF Power Supply Type
  //
  PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_TYPE), HiiHandle);
  Temp = (Characteristics & 0x1C00) << 10;
  switch (Temp) {
  case 1:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_OTHER_SPACE), HiiHandle);
    break;

  case 2:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_UNKNOWN), HiiHandle);
    break;

  case 3:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_LINEAR), HiiHandle);
    break;

  case 4:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_SWITCHING), HiiHandle);
    break;

  case 5:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_BATTERY), HiiHandle);
    break;

  case 6:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_UPS), HiiHandle);
    break;

  case 7:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_CONVERTER), HiiHandle);
    break;

  case 8:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_REGULATOR), HiiHandle);
    break;

  default:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_RESERVED_2), HiiHandle);
  }
  //
  // Bits 9:7 - Status
  //
  PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_STATUS_DASH), HiiHandle);
  Temp = (Characteristics & 0x380) << 7;
  switch (Temp) {
  case 1:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_OTHER_SPACE), HiiHandle);
    break;

  case 2:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_UNKNOWN), HiiHandle);
    break;

  case 3:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_OK), HiiHandle);
    break;

  case 4:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_NON_CRITICAL), HiiHandle);
    break;

  case 5:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_CRITICAL_POWER_SUPPLY), HiiHandle);
    break;

  default:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_UNDEFINED), HiiHandle);
  }
  //
  // Bits 6:3 - DMTF Input Voltage Range Switching
  //
  PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_INPUT_VOLTAGE_RANGE), HiiHandle);
  Temp = (Characteristics & 0x78) << 3;
  switch (Temp) {
  case 1:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_OTHER_SPACE), HiiHandle);
    break;

  case 2:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_UNKNOWN), HiiHandle);
    break;

  case 3:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_MANUAL), HiiHandle);
    break;

  case 4:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_AUTO_SWITCH), HiiHandle);
    break;

  case 5:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_WIDE_RANGE), HiiHandle);
    break;

  case 6:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_NOT_APPLICABLE), HiiHandle);
    break;

  default:
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_RESERVED_3), HiiHandle);
    break;
  }
  //
  // Power supply is unplugged from the wall
  //
  if (BIT (Characteristics, 2) != 0) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_POWER_SUPPLY_UNPLUGGED), HiiHandle);
  } else {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_POWER_SUPPLY_PLUGGED), HiiHandle);
  }
  //
  // Power supply is present
  //
  if (BIT (Characteristics, 1) != 0) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_POWER_SUPPLY_PRESENT), HiiHandle);
  } else {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_POWER_SUPPLY_NOT_PRESENT), HiiHandle);
  }
  //
  // hot replaceable
  //
  if (BIT (Characteristics, 0) != 0) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_POWER_SUPPLY_REPLACE), HiiHandle);
  } else {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_PRINTINFO_POWER_SUPPLY_NOT_REPLACE), HiiHandle);
  }
}
