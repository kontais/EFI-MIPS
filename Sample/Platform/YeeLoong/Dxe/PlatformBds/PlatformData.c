/*++

Copyright (c) 2004 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name: 

  PlatformData.c

Abstract:
  
  Defined the platform specific device path which will be used by
  platform Bbd to perform the platform policy connect.

--*/

#include "BdsPlatform.h"

//
// Predefined platform default time out value
//
UINT16                      gPlatformBootTimeOutDefault = 10;

//
// Platform specific keyboard device path
//
YL_PLATFORM_UGA_DEVICE_PATH gUgaDevicePath0 = {
  {
    ACPI_DEVICE_PATH,
    ACPI_DP,
    (UINT8) (sizeof(ACPI_HID_DEVICE_PATH)),
    (UINT8) ((sizeof(ACPI_HID_DEVICE_PATH)) >> 8),
    EISA_PNP_ID(0x0A03),
    0
  },
  {
    HARDWARE_DEVICE_PATH,
    HW_PCI_DP,
    (UINT8) (sizeof (PCI_DEVICE_PATH)),
    (UINT8) ((sizeof (PCI_DEVICE_PATH)) >> 8),
    0,
    8
  },
  gEndEntire
};

/*
YL_PLATFORM_UGA_DEVICE_PATH gUgaDevicePath1 = {
  {
    ACPI_DEVICE_PATH,
    ACPI_DP,
    (UINT8) (sizeof(ACPI_HID_DEVICE_PATH)),
    (UINT8) ((sizeof(ACPI_HID_DEVICE_PATH)) >> 8),
    EISA_PNP_ID(0x0A03),
    0
  },
  {
    HARDWARE_DEVICE_PATH,
    HW_PCI_DP,
    (UINT8) (sizeof (PCI_DEVICE_PATH)),
    (UINT8) ((sizeof (PCI_DEVICE_PATH)) >> 8),
    0,
    8
  },
  gEndEntire
};
*/

#if (EFI_SPECIFICATION_VERSION >= 0x00020000)
YL_PLATFORM_GOP_DEVICE_PATH gGopDevicePath0 = {
  {
    ACPI_DEVICE_PATH,
    ACPI_DP,
    (UINT8) (sizeof(ACPI_HID_DEVICE_PATH)),
    (UINT8) ((sizeof(ACPI_HID_DEVICE_PATH)) >> 8),
    EISA_PNP_ID(0x0A03),
    0
  },
  {
    HARDWARE_DEVICE_PATH,
    HW_PCI_DP,
    (UINT8) (sizeof (PCI_DEVICE_PATH)),
    (UINT8) ((sizeof (PCI_DEVICE_PATH)) >> 8),
    0,
    8
  },
  gEndEntire
};

/*
YL_PLATFORM_GOP_DEVICE_PATH gGopDevicePath1 = {
  {
    ACPI_DEVICE_PATH,
    ACPI_DP,
    (UINT8) (sizeof(ACPI_HID_DEVICE_PATH)),
    (UINT8) ((sizeof(ACPI_HID_DEVICE_PATH)) >> 8),
    EISA_PNP_ID(0x0A03),
    0
  },
  {
    HARDWARE_DEVICE_PATH,
    HW_PCI_DP,
    (UINT8) (sizeof (PCI_DEVICE_PATH)),
    (UINT8) ((sizeof (PCI_DEVICE_PATH)) >> 8),
    0,
    8
  },
  gEndEntire
};
*/
#endif

//
// Platform specific serial device path
//
NT_ISA_SERIAL_DEVICE_PATH   gNtSerialDevicePath0 = {
  {
    HARDWARE_DEVICE_PATH,
    HW_VENDOR_DP,
    (UINT8) (sizeof (VENDOR_DEVICE_PATH)),
    (UINT8) ((sizeof (VENDOR_DEVICE_PATH)) >> 8),
    EFI_LINUX_THUNK_PROTOCOL_GUID
  },
  {
    HARDWARE_DEVICE_PATH,
    HW_VENDOR_DP,
    (UINT8) (sizeof (WIN_NT_VENDOR_DEVICE_PATH_NODE)),
    (UINT8) ((sizeof (WIN_NT_VENDOR_DEVICE_PATH_NODE)) >> 8),
    EFI_LINUX_SERIAL_PORT_GUID
  },
  {
    MESSAGING_DEVICE_PATH,
    MSG_UART_DP,
    (UINT8) (sizeof (UART_DEVICE_PATH)),
    (UINT8) ((sizeof (UART_DEVICE_PATH)) >> 8),
    0,
    115200,
    8,
    1,
    1
  },
  {
    MESSAGING_DEVICE_PATH,
    MSG_VENDOR_DP,
    (UINT8) (sizeof (VENDOR_DEVICE_PATH)),
    (UINT8) ((sizeof (VENDOR_DEVICE_PATH)) >> 8),
    DEVICE_PATH_MESSAGING_PC_ANSI
  },
  gEndEntire
};

NT_ISA_SERIAL_DEVICE_PATH   gNtSerialDevicePath1 = {
  {
    HARDWARE_DEVICE_PATH,
    HW_VENDOR_DP,
    (UINT8) (sizeof (VENDOR_DEVICE_PATH)),
    (UINT8) ((sizeof (VENDOR_DEVICE_PATH)) >> 8),
    EFI_LINUX_THUNK_PROTOCOL_GUID
  },
  {
    HARDWARE_DEVICE_PATH,
    HW_VENDOR_DP,
    (UINT8) (sizeof (WIN_NT_VENDOR_DEVICE_PATH_NODE)),
    (UINT8) ((sizeof (WIN_NT_VENDOR_DEVICE_PATH_NODE)) >> 8),
    EFI_LINUX_SERIAL_PORT_GUID,
    1
  },
  {
    MESSAGING_DEVICE_PATH,
    MSG_UART_DP,
    (UINT8) (sizeof (UART_DEVICE_PATH)),
    (UINT8) ((sizeof (UART_DEVICE_PATH)) >> 8),
    0,
    115200,
    8,
    1,
    1
  },
  {
    MESSAGING_DEVICE_PATH,
    MSG_VENDOR_DP,
    (UINT8) (sizeof (VENDOR_DEVICE_PATH)),
    (UINT8) ((sizeof (VENDOR_DEVICE_PATH)) >> 8),
    DEVICE_PATH_MESSAGING_PC_ANSI
  },
  gEndEntire
};

//
// Predefined platform default console device path
//
BDS_CONSOLE_CONNECT_ENTRY   gPlatformConsole[] = {
  {
    (EFI_DEVICE_PATH_PROTOCOL *) &gNtSerialDevicePath0,
    (CONSOLE_OUT | CONSOLE_IN)
  },
  {
    (EFI_DEVICE_PATH_PROTOCOL *) &gNtSerialDevicePath1,
    (CONSOLE_OUT | CONSOLE_IN)
  },
  {
    (EFI_DEVICE_PATH_PROTOCOL *) &gUgaDevicePath0,
    (CONSOLE_OUT | CONSOLE_IN)
  },
/*
{
    (EFI_DEVICE_PATH_PROTOCOL *) &gUgaDevicePath1,
    (CONSOLE_OUT | CONSOLE_IN)
  },
*/
#if (EFI_SPECIFICATION_VERSION >= 0x00020000)
  {
    (EFI_DEVICE_PATH_PROTOCOL *) &gGopDevicePath0,
    (CONSOLE_OUT | CONSOLE_IN)
  },
/*
  {
    (EFI_DEVICE_PATH_PROTOCOL *) &gGopDevicePath1,
    (CONSOLE_OUT | CONSOLE_IN)
  },
*/
#endif
  {
    NULL,
    0
  }
};

//
// Predefined platform specific driver option
//
EFI_DEVICE_PATH_PROTOCOL    *gPlatformDriverOption[] = { NULL };

//
// Predefined platform connect sequence
//
EFI_DEVICE_PATH_PROTOCOL    *gPlatformConnectSequence[] = { NULL };
