/*++
 
Copyright (c) 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  MiscPortInternalConnectorDesignatorFunction.c
  
Abstract: 

  This driver parses the mMiscSubclassDataTable structure and reports
  any generated data to the DataHub.

--*/

#include "MiscSubclassDriver.h"

//
//
//
MISC_SUBCLASS_TABLE_FUNCTION (
  MiscPortInternalConnectorDesignator
  )
/*++
Description:

  This function makes boot time changes to the contents of the
  MiscPortConnectorInformation (Type 8).

Parameters:

  RecordType
    Type of record to be processed from the Data Table.
    mMiscSubclassDataTable[].RecordType

  RecordLen
    Size of static RecordData from the Data Table.
    mMiscSubclassDataTable[].RecordLen

  RecordData
    Pointer to copy of RecordData from the Data Table.  Changes made
    to this copy will be written to the Data Hub but will not alter
    the contents of the static Data Table.

  LogRecordData
    Set *LogRecordData to TRUE to log RecordData to Data Hub.
    Set *LogRecordData to FALSE when there is no more data to log.

Returns:

  EFI_SUCCESS
    All parameters were valid and *RecordData and *LogRecordData have
    been set.

  EFI_UNSUPPORTED
    Unexpected RecordType value.

  EFI_INVALID_PARAMETER
    One of the following parameter conditions was true:
      RecordLen was zero.
      RecordData was NULL.
      LogRecordData was NULL.
--*/
{
  STATIC BOOLEAN                    Done                    = FALSE;
  STATIC PS2_CONN_DEVICE_PATH       mPs2KeyboardDevicePath  = { ACPI, PCI (0x1F, 0x00), LPC (0x0303, 0), END };
  STATIC PS2_CONN_DEVICE_PATH       mPs2MouseDevicePath     = { ACPI, PCI (0x1F, 0x00), LPC (0x0303, 1), END };
  STATIC SERIAL_CONN_DEVICE_PATH    mCom1DevicePath         = { ACPI, PCI (0x1F, 0x00), LPC (0x0501, 0), END };
  STATIC SERIAL_CONN_DEVICE_PATH    mCom2DevicePath         = { ACPI, PCI (0x1F, 0x00), LPC (0x0501, 1), END };
  STATIC PARALLEL_CONN_DEVICE_PATH  mLpt1DevicePath         = { ACPI, PCI (0x1F, 0x00), LPC (0x0401, 0), END };
  STATIC FLOOPY_CONN_DEVICE_PATH    mFloopyADevicePath      = { ACPI, PCI (0x1F, 0x00), LPC (0x0604, 0), END };
  STATIC FLOOPY_CONN_DEVICE_PATH    mFloopyBDevicePath      = { ACPI, PCI (0x1F, 0x00), LPC (0x0604, 1), END };
  STATIC USB_PORT_DEVICE_PATH       mUsb0DevicePath         = { ACPI, PCI (0x1d, 0x00), END };
  STATIC USB_PORT_DEVICE_PATH       mUsb1DevicePath         = { ACPI, PCI (0x1d, 0x01), END };
  STATIC USB_PORT_DEVICE_PATH       mUsb2DevicePath         = { ACPI, PCI (0x1d, 0x02), END };
  STATIC USB_PORT_DEVICE_PATH       mUsb3DevicePath         = { ACPI, PCI (0x1d, 0x07), END };
  STATIC IDE_DEVICE_PATH            mIdeDevicePath          = { ACPI, PCI (0x1F, 0x01), END };
  STATIC GB_NIC_DEVICE_PATH         mGbNicDevicePath            = { ACPI, PCI( 0x03,0x00 ),PCI( 0x1F,0x00 ),PCI( 0x07,0x00 ), END };
  EFI_DEVICE_PATH_PROTOCOL          EndDevicePath           = END;

  //
  // EFI_DEADLOOP();//breakpoint
  //
  // First check for invalid parameters.
  //
  // Shanmu >> to fix the Device Path Issue...
  // if (RecordLen == 0 || RecordData == NULL || LogRecordData == NULL) {
  //
  if (*RecordLen == 0 || RecordData == NULL || LogRecordData == NULL) {
    //
    // End Shanmu
    //
    return EFI_INVALID_PARAMETER;
  }
  //
  // Then check for unsupported RecordType.
  //
  if (RecordType != EFI_MISC_PORT_INTERNAL_CONNECTOR_DESIGNATOR_RECORD_NUMBER) {
    return EFI_UNSUPPORTED;
  }
  //
  // Is this the first time through this function?
  //
  if (!Done) {
    //
    // Yes, this is the first time.  Inspect/Change the contents of the
    // RecordData structure.
    //
    //
    // Device path is only updated here as it was not taking that in static data
    //
    // Shanmu >> to fix the Device Path Issue...
    //

    /*
    switch (((EFI_MISC_PORT_INTERNAL_CONNECTOR_DESIGNATOR *)RecordData)->PortInternalConnectorDesignator) 
    {
      case STR_MISC_PORT_INTERNAL_MOUSE:
        {
          (EFI_DEVICE_PATH_PROTOCOL)((EFI_MISC_PORT_INTERNAL_CONNECTOR_DESIGNATOR *)RecordData)->PortPath = *((EFI_DEVICE_PATH_PROTOCOL*)&mPs2MouseDevicePath);          
        }break;
      case STR_MISC_PORT_INTERNAL_KEYBOARD:
        {
          (EFI_DEVICE_PATH_PROTOCOL)((EFI_MISC_PORT_INTERNAL_CONNECTOR_DESIGNATOR *)RecordData)->PortPath = *((EFI_DEVICE_PATH_PROTOCOL*)&mPs2KeyboardDevicePath);          
        }break;
      case STR_MISC_PORT_INTERNAL_COM1:
        {
          (EFI_DEVICE_PATH_PROTOCOL)((EFI_MISC_PORT_INTERNAL_CONNECTOR_DESIGNATOR *)RecordData)->PortPath = *((EFI_DEVICE_PATH_PROTOCOL*)&mCom1DevicePath);          
        }break;
      case STR_MISC_PORT_INTERNAL_COM2:
        {
          (EFI_DEVICE_PATH_PROTOCOL)((EFI_MISC_PORT_INTERNAL_CONNECTOR_DESIGNATOR *)RecordData)->PortPath = *((EFI_DEVICE_PATH_PROTOCOL*)&mCom2DevicePath);          
        }break;
      case STR_MISC_PORT_INTERNAL_LPT1:
        {
          (EFI_DEVICE_PATH_PROTOCOL)((EFI_MISC_PORT_INTERNAL_CONNECTOR_DESIGNATOR *)RecordData)->PortPath = *((EFI_DEVICE_PATH_PROTOCOL*)&mLpt1DevicePath);          
        }break;
      case STR_MISC_PORT_INTERNAL_USB1:
        {
          (EFI_DEVICE_PATH_PROTOCOL)((EFI_MISC_PORT_INTERNAL_CONNECTOR_DESIGNATOR *)RecordData)->PortPath = *((EFI_DEVICE_PATH_PROTOCOL*)&mUsb0DevicePath);          
        }break;
      case STR_MISC_PORT_INTERNAL_USB2:
        {
          (EFI_DEVICE_PATH_PROTOCOL)((EFI_MISC_PORT_INTERNAL_CONNECTOR_DESIGNATOR *)RecordData)->PortPath = *((EFI_DEVICE_PATH_PROTOCOL*)&mUsb1DevicePath);          
        }break;
      case STR_MISC_PORT_INTERNAL_USB3:
        {
          (EFI_DEVICE_PATH_PROTOCOL)((EFI_MISC_PORT_INTERNAL_CONNECTOR_DESIGNATOR *)RecordData)->PortPath = *((EFI_DEVICE_PATH_PROTOCOL*)&mUsb2DevicePath);          
        }break;
      case STR_MISC_PORT_INTERNAL_NETWORK:
        {
          (EFI_DEVICE_PATH_PROTOCOL)((EFI_MISC_PORT_INTERNAL_CONNECTOR_DESIGNATOR *)RecordData)->PortPath = *((EFI_DEVICE_PATH_PROTOCOL*)&mGbNicDevicePath);          
        }break;
      case STR_MISC_PORT_INTERNAL_FLOPPY:
        {
          (EFI_DEVICE_PATH_PROTOCOL)((EFI_MISC_PORT_INTERNAL_CONNECTOR_DESIGNATOR *)RecordData)->PortPath = *((EFI_DEVICE_PATH_PROTOCOL*)&mFloopyADevicePath);          
        }break;
      case STR_MISC_PORT_INTERNAL_IDE1:
        {
          (EFI_DEVICE_PATH_PROTOCOL)((EFI_MISC_PORT_INTERNAL_CONNECTOR_DESIGNATOR *)RecordData)->PortPath = *((EFI_DEVICE_PATH_PROTOCOL*)&mIdeDevicePath);          
        }break;
      case STR_MISC_PORT_INTERNAL_IDE2:
        {
          (EFI_DEVICE_PATH_PROTOCOL)((EFI_MISC_PORT_INTERNAL_CONNECTOR_DESIGNATOR *)RecordData)->PortPath = *((EFI_DEVICE_PATH_PROTOCOL*)&mIdeDevicePath);          
        }break;
      default:
        {
          (EFI_DEVICE_PATH_PROTOCOL)((EFI_MISC_PORT_INTERNAL_CONNECTOR_DESIGNATOR *)RecordData)->PortPath = EndDevicePath;
        }break;    
    }
    */
    switch (((EFI_MISC_PORT_INTERNAL_CONNECTOR_DESIGNATOR *) RecordData)->PortInternalConnectorDesignator) {
    case STR_MISC_PORT_INTERNAL_MOUSE:
      {
        EfiCopyMem (
          &((EFI_MISC_PORT_INTERNAL_CONNECTOR_DESIGNATOR *) RecordData)->PortPath,
          &mPs2MouseDevicePath,
          EfiDevicePathSize ((EFI_DEVICE_PATH_PROTOCOL *) &mPs2MouseDevicePath)
          );
        *RecordLen = *RecordLen - sizeof (EFI_MISC_PORT_DEVICE_PATH) + EfiDevicePathSize ((EFI_DEVICE_PATH_PROTOCOL *) &mPs2MouseDevicePath);
      }
      break;

    case STR_MISC_PORT_INTERNAL_KEYBOARD:
      {
        EfiCopyMem (
          &((EFI_MISC_PORT_INTERNAL_CONNECTOR_DESIGNATOR *) RecordData)->PortPath,
          &mPs2KeyboardDevicePath,
          EfiDevicePathSize ((EFI_DEVICE_PATH_PROTOCOL *) &mPs2KeyboardDevicePath)
          );
        *RecordLen = *RecordLen - sizeof (EFI_MISC_PORT_DEVICE_PATH) + EfiDevicePathSize ((EFI_DEVICE_PATH_PROTOCOL *) &mPs2KeyboardDevicePath);
      }
      break;

    case STR_MISC_PORT_INTERNAL_COM1:
      {
        EfiCopyMem (
          &((EFI_MISC_PORT_INTERNAL_CONNECTOR_DESIGNATOR *) RecordData)->PortPath,
          &mCom1DevicePath,
          EfiDevicePathSize ((EFI_DEVICE_PATH_PROTOCOL *) &mCom1DevicePath)
          );
        *RecordLen = *RecordLen - sizeof (EFI_MISC_PORT_DEVICE_PATH) + EfiDevicePathSize ((EFI_DEVICE_PATH_PROTOCOL *) &mCom1DevicePath);
      }
      break;

    case STR_MISC_PORT_INTERNAL_COM2:
      {
        EfiCopyMem (
          &((EFI_MISC_PORT_INTERNAL_CONNECTOR_DESIGNATOR *) RecordData)->PortPath,
          &mCom2DevicePath,
          EfiDevicePathSize ((EFI_DEVICE_PATH_PROTOCOL *) &mCom2DevicePath)
          );
        *RecordLen = *RecordLen - sizeof (EFI_MISC_PORT_DEVICE_PATH) + EfiDevicePathSize ((EFI_DEVICE_PATH_PROTOCOL *) &mCom2DevicePath);
      }
      break;

    case STR_MISC_PORT_INTERNAL_FLOPPY:
      {
        EfiCopyMem (
          &((EFI_MISC_PORT_INTERNAL_CONNECTOR_DESIGNATOR *) RecordData)->PortPath,
          &mFloopyADevicePath,
          EfiDevicePathSize ((EFI_DEVICE_PATH_PROTOCOL *) &mFloopyADevicePath)
          );
        *RecordLen = *RecordLen - sizeof (EFI_MISC_PORT_DEVICE_PATH) + EfiDevicePathSize ((EFI_DEVICE_PATH_PROTOCOL *) &mFloopyADevicePath);
      }
      break;

    default:
      {
        EfiCopyMem (
          &((EFI_MISC_PORT_INTERNAL_CONNECTOR_DESIGNATOR *) RecordData)->PortPath,
          &EndDevicePath,
          EfiDevicePathSize ((EFI_DEVICE_PATH_PROTOCOL *) &EndDevicePath)
          );
        *RecordLen = *RecordLen - sizeof (EFI_MISC_PORT_DEVICE_PATH) + EfiDevicePathSize ((EFI_DEVICE_PATH_PROTOCOL *) &EndDevicePath);
      }
      break;
    }
    //
    // End Shanmu
    //
    // Set Done flag to TRUE for next pass through this function.
    // Set *LogRecordData to TRUE so data will get logged to Data Hub.
    //
    Done            = TRUE;
    *LogRecordData  = TRUE;
  } else {
    //
    // No, this is the second time.  Reset the state of the Done flag
    // to FALSE and tell the data logger that there is no more data
    // to be logged for this record type.  If any memory allocations
    // were made by earlier passes, they must be released now.
    //
    Done            = FALSE;
    *LogRecordData  = FALSE;
  }

  return EFI_SUCCESS;
}

/* eof - MiscSystemManufacturerFunction.c */
