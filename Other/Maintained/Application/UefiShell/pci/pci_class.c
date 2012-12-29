/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  pci_class.c

Abstract:

  Supporting routines for EFI shell command "pci"

Revision History

--*/

#include "EfiShellLib.h"  // for UINT32 etc.
#include "pci_class.h"    // GetPciClassStrings() prototype
#include "EfiShellLib.h"

#include STRING_DEFINES_FILE

//
// Global Variables
//
EFI_HII_HANDLE  HiiHandle;

PCI_CLASS_ENTRY gClassStringList[];

//
// Declarations of entries which contain printable strings for class codes
// in PCI configuration space
//
PCI_CLASS_ENTRY PCIBlankEntry[];
PCI_CLASS_ENTRY PCISubClass_00[];
PCI_CLASS_ENTRY PCISubClass_01[];
PCI_CLASS_ENTRY PCISubClass_02[];
PCI_CLASS_ENTRY PCISubClass_03[];
PCI_CLASS_ENTRY PCISubClass_04[];
PCI_CLASS_ENTRY PCISubClass_05[];
PCI_CLASS_ENTRY PCISubClass_06[];
PCI_CLASS_ENTRY PCISubClass_07[];
PCI_CLASS_ENTRY PCISubClass_08[];
PCI_CLASS_ENTRY PCISubClass_09[];
PCI_CLASS_ENTRY PCISubClass_0a[];
PCI_CLASS_ENTRY PCISubClass_0b[];
PCI_CLASS_ENTRY PCISubClass_0c[];
PCI_CLASS_ENTRY PCISubClass_0d[];
PCI_CLASS_ENTRY PCISubClass_0e[];
PCI_CLASS_ENTRY PCISubClass_0f[];
PCI_CLASS_ENTRY PCISubClass_10[];
PCI_CLASS_ENTRY PCISubClass_11[];
PCI_CLASS_ENTRY PCIPIFClass_0101[];
PCI_CLASS_ENTRY PCIPIFClass_0300[];
PCI_CLASS_ENTRY PCIPIFClass_0604[];
PCI_CLASS_ENTRY PCIPIFClass_0700[];
PCI_CLASS_ENTRY PCIPIFClass_0701[];
PCI_CLASS_ENTRY PCIPIFClass_0703[];
PCI_CLASS_ENTRY PCIPIFClass_0800[];
PCI_CLASS_ENTRY PCIPIFClass_0801[];
PCI_CLASS_ENTRY PCIPIFClass_0802[];
PCI_CLASS_ENTRY PCIPIFClass_0803[];
PCI_CLASS_ENTRY PCIPIFClass_0904[];
PCI_CLASS_ENTRY PCIPIFClass_0c00[];
PCI_CLASS_ENTRY PCIPIFClass_0c03[];
PCI_CLASS_ENTRY PCIPIFClass_0e00[];

//
// Base class strings entries
//
PCI_CLASS_ENTRY gClassStringList[] = {
  {
    0x00,
    L"Pre 2.0 device",
    PCISubClass_00
  },
  {
    0x01,
    L"Mass Storage Controller",
    PCISubClass_01
  },
  {
    0x02,
    L"Network Controller",
    PCISubClass_02
  },
  {
    0x03,
    L"Display Controller",
    PCISubClass_03
  },
  {
    0x04,
    L"Multimedia Device",
    PCISubClass_04
  },
  {
    0x05,
    L"Memory Controller",
    PCISubClass_05
  },
  {
    0x06,
    L"Bridge Device",
    PCISubClass_06
  },
  {
    0x07,
    L"Simple Communications Controllers",
    PCISubClass_07
  },
  {
    0x08,
    L"Base System Peripherals",
    PCISubClass_08
  },
  {
    0x09,
    L"Input Devices",
    PCISubClass_09
  },
  {
    0x0a,
    L"Docking Stations",
    PCISubClass_0a
  },
  {
    0x0b,
    L"Processors",
    PCISubClass_0b
  },
  {
    0x0c,
    L"Serial Bus Controllers",
    PCISubClass_0c
  },
  {
    0x0d,
    L"Wireless Controllers",
    PCISubClass_0d
  },
  {
    0x0e,
    L"Intelligent IO Controllers",
    PCISubClass_0e
  },
  {
    0x0f,
    L"Satellite Communications Controllers",
    PCISubClass_0f
  },
  {
    0x10,
    L"Encryption/Decryption Controllers",
    PCISubClass_10
  },
  {
    0x11,
    L"Data Acquisition & Signal Processing Controllers",
    PCISubClass_11
  },
  {
    0xff,
    L"Device does not fit in any defined classes",
    PCIBlankEntry
  },
  {
    0x00,
    NULL,
    /* null string ends the list */NULL
  }
};

//
// Subclass strings entries
//
PCI_CLASS_ENTRY PCIBlankEntry[] = {
  {
    0x00,
    L"",
    PCIBlankEntry
  },
  {
    0x00,
    NULL,
    /* null string ends the list */NULL
  }
};

PCI_CLASS_ENTRY PCISubClass_00[] = {
  {
    0x00,
    L"All devices other than VGA",
    PCIBlankEntry
  },
  {
    0x01,
    L"VGA-compatible devices",
    PCIBlankEntry
  },
  {
    0x00,
    NULL,
    /* null string ends the list */NULL
  }
};

PCI_CLASS_ENTRY PCISubClass_01[] = {
  {
    0x00,
    L"SCSI controller",
    PCIBlankEntry
  },
  {
    0x01,
    L"IDE controller",
    PCIPIFClass_0101
  },
  {
    0x02,
    L"Floppy disk controller",
    PCIBlankEntry
  },
  {
    0x03,
    L"IPI controller",
    PCIBlankEntry
  },
  {
    0x04,
    L"RAID controller",
    PCIBlankEntry
  },
  {
    0x80,
    L"Other mass storage controller",
    PCIBlankEntry
  },
  {
    0x00,
    NULL,
    /* null string ends the list */NULL
  }
};

PCI_CLASS_ENTRY PCISubClass_02[] = {
  {
    0x00,
    L"Ethernet controller",
    PCIBlankEntry
  },
  {
    0x01,
    L"Token ring controller",
    PCIBlankEntry
  },
  {
    0x02,
    L"FDDI controller",
    PCIBlankEntry
  },
  {
    0x03,
    L"ATM controller",
    PCIBlankEntry
  },
  {
    0x04,
    L"ISDN controller",
    PCIBlankEntry
  },
  {
    0x80,
    L"Other network controller",
    PCIBlankEntry
  },
  {
    0x00,
    NULL,
    /* null string ends the list */NULL
  }
};

PCI_CLASS_ENTRY PCISubClass_03[] = {
  {
    0x00,
    L"VGA/8514 controller",
    PCIPIFClass_0300
  },
  {
    0x01,
    L"XGA controller",
    PCIBlankEntry
  },
  {
    0x02,
    L"3D controller",
    PCIBlankEntry
  },
  {
    0x80,
    L"Other display controller",
    PCIBlankEntry
  },
  {
    0x00,
    NULL,
    /* null string ends the list */PCIBlankEntry
  }
};

PCI_CLASS_ENTRY PCISubClass_04[] = {
  {
    0x00,
    L"Video device",
    PCIBlankEntry
  },
  {
    0x01,
    L"Audio device",
    PCIBlankEntry
  },
  {
    0x02,
    L"Computer Telephony device",
    PCIBlankEntry
  },
  {
    0x80,
    L"Other multimedia device",
    PCIBlankEntry
  },
  {
    0x00,
    NULL,
    /* null string ends the list */NULL
  }
};

PCI_CLASS_ENTRY PCISubClass_05[] = {
  {
    0x00,
    L"RAM memory controller",
    PCIBlankEntry
  },
  {
    0x01,
    L"Flash memory controller",
    PCIBlankEntry
  },
  {
    0x80,
    L"Other memory controller",
    PCIBlankEntry
  },
  {
    0x00,
    NULL,
    /* null string ends the list */NULL
  }
};

PCI_CLASS_ENTRY PCISubClass_06[] = {
  {
    0x00,
    L"Host/PCI bridge",
    PCIBlankEntry
  },
  {
    0x01,
    L"PCI/ISA bridge",
    PCIBlankEntry
  },
  {
    0x02,
    L"PCI/EISA bridge",
    PCIBlankEntry
  },
  {
    0x03,
    L"PCI/Micro Channel bridge",
    PCIBlankEntry
  },
  {
    0x04,
    L"PCI/PCI bridge",
    PCIPIFClass_0604
  },
  {
    0x05,
    L"PCI/PCMCIA bridge",
    PCIBlankEntry
  },
  {
    0x06,
    L"NuBus bridge",
    PCIBlankEntry
  },
  {
    0x07,
    L"CardBus bridge",
    PCIBlankEntry
  },
  {
    0x08,
    L"RACEway bridge",
    PCIBlankEntry
  },
  {
    0x80,
    L"Other bridge type",
    PCIBlankEntry
  },
  {
    0x00,
    NULL,
    /* null string ends the list */NULL
  }
};

PCI_CLASS_ENTRY PCISubClass_07[] = {
  {
    0x00,
    L"Serial controller",
    PCIPIFClass_0700
  },
  {
    0x01,
    L"Parallel port",
    PCIPIFClass_0701
  },
  {
    0x02,
    L"Multiport serial controller",
    PCIBlankEntry
  },
  {
    0x03,
    L"Modem",
    PCIPIFClass_0703
  },
  {
    0x80,
    L"Other communication device",
    PCIBlankEntry
  },
  {
    0x00,
    NULL,
    /* null string ends the list */NULL
  }
};

PCI_CLASS_ENTRY PCISubClass_08[] = {
  {
    0x00,
    L"PIC",
    PCIPIFClass_0800
  },
  {
    0x01,
    L"DMA controller",
    PCIPIFClass_0801
  },
  {
    0x02,
    L"System timer",
    PCIPIFClass_0802
  },
  {
    0x03,
    L"RTC controller",
    PCIPIFClass_0803
  },
  {
    0x04,
    L"Generic PCI Hot-Plug controller",
    PCIBlankEntry
  },
  {
    0x80,
    L"Other system peripheral",
    PCIBlankEntry
  },
  {
    0x00,
    NULL,
    /* null string ends the list */NULL
  }
};

PCI_CLASS_ENTRY PCISubClass_09[] = {
  {
    0x00,
    L"Keyboard controller",
    PCIBlankEntry
  },
  {
    0x01,
    L"Digitizer (pen)",
    PCIBlankEntry
  },
  {
    0x02,
    L"Mouse controller",
    PCIBlankEntry
  },
  {
    0x03,
    L"Scanner controller",
    PCIBlankEntry
  },
  {
    0x04,
    L"Gameport controller",
    PCIPIFClass_0904
  },
  {
    0x80,
    L"Other input controller",
    PCIBlankEntry
  },
  {
    0x00,
    NULL,
    /* null string ends the list */NULL
  }
};

PCI_CLASS_ENTRY PCISubClass_0a[] = {
  {
    0x00,
    L"Generic docking station",
    PCIBlankEntry
  },
  {
    0x80,
    L"Other type of docking station",
    PCIBlankEntry
  },
  {
    0x00,
    NULL,
    /* null string ends the list */NULL
  }
};

PCI_CLASS_ENTRY PCISubClass_0b[] = {
  {
    0x00,
    L"386",
    PCIBlankEntry
  },
  {
    0x01,
    L"486",
    PCIBlankEntry
  },
  {
    0x02,
    L"Pentium",
    PCIBlankEntry
  },
  {
    0x10,
    L"Alpha",
    PCIBlankEntry
  },
  {
    0x20,
    L"PowerPC",
    PCIBlankEntry
  },
  {
    0x30,
    L"MIPS",
    PCIBlankEntry
  },
  {
    0x40,
    L"Co-processor",
    PCIBlankEntry
  },
  {
    0x80,
    L"Other processor",
    PCIBlankEntry
  },
  {
    0x00,
    NULL,
    /* null string ends the list */NULL
  }
};

PCI_CLASS_ENTRY PCISubClass_0c[] = {
  {
    0x00,
    L"Firewire(IEEE 1394)",
    PCIPIFClass_0c03
  },
  {
    0x01,
    L"ACCESS.bus",
    PCIBlankEntry
  },
  {
    0x02,
    L"SSA",
    PCIBlankEntry
  },
  {
    0x03,
    L"USB",
    PCIPIFClass_0c00
  },
  {
    0x04,
    L"Fibre Channel",
    PCIBlankEntry
  },
  {
    0x05,
    L"System Management Bus",
    PCIBlankEntry
  },
  {
    0x80,
    L"Other bus type",
    PCIBlankEntry
  },
  {
    0x00,
    NULL,
    /* null string ends the list */NULL
  }
};

PCI_CLASS_ENTRY PCISubClass_0d[] = {
  {
    0x00,
    L"iRDA compatible controller",
    PCIBlankEntry
  },
  {
    0x01,
    L"Consumer IR controller",
    PCIBlankEntry
  },
  {
    0x10,
    L"RF controller",
    PCIBlankEntry
  },
  {
    0x80,
    L"Other type of wireless controller",
    PCIBlankEntry
  },
  {
    0x00,
    NULL,
    /* null string ends the list */NULL
  }
};

PCI_CLASS_ENTRY PCISubClass_0e[] = {
  {
    0x00,
    L"I2O Architecture",
    PCIPIFClass_0e00
  },
  {
    0x00,
    NULL,
    /* null string ends the list */NULL
  }
};

PCI_CLASS_ENTRY PCISubClass_0f[] = {
  {
    0x00,
    L"TV",
    PCIBlankEntry
  },
  {
    0x01,
    L"Audio",
    PCIBlankEntry
  },
  {
    0x02,
    L"Voice",
    PCIBlankEntry
  },
  {
    0x03,
    L"Data",
    PCIBlankEntry
  },
  {
    0x00,
    NULL,
    /* null string ends the list */NULL
  }
};

PCI_CLASS_ENTRY PCISubClass_10[] = {
  {
    0x00,
    L"Network & computing Encrypt/Decrypt",
    PCIBlankEntry
  },
  {
    0x01,
    L"Entertainment Encrypt/Decrypt",
    PCIBlankEntry
  },
  {
    0x80,
    L"Other Encrypt/Decrypt",
    PCIBlankEntry
  },
  {
    0x00,
    NULL,
    /* null string ends the list */NULL
  }
};

PCI_CLASS_ENTRY PCISubClass_11[] = {
  {
    0x00,
    L"DPIO modules",
    PCIBlankEntry
  },
  {
    0x80,
    L"Other DAQ & SP controllers",
    PCIBlankEntry
  },
  {
    0x00,
    NULL,
    /* null string ends the list */NULL
  }
};

//
// Programming Interface entries
//
PCI_CLASS_ENTRY PCIPIFClass_0101[] = {
  {
    0x00,
    L"",
    PCIBlankEntry
  },
  {
    0x01,
    L"OM-primary",
    PCIBlankEntry
  },
  {
    0x02,
    L"PI-primary",
    PCIBlankEntry
  },
  {
    0x03,
    L"OM/PI-primary",
    PCIBlankEntry
  },
  {
    0x04,
    L"OM-secondary",
    PCIBlankEntry
  },
  {
    0x05,
    L"OM-primary, OM-secondary",
    PCIBlankEntry
  },
  {
    0x06,
    L"PI-primary, OM-secondary",
    PCIBlankEntry
  },
  {
    0x07,
    L"OM/PI-primary, OM-secondary",
    PCIBlankEntry
  },
  {
    0x08,
    L"OM-secondary",
    PCIBlankEntry
  },
  {
    0x09,
    L"OM-primary, PI-secondary",
    PCIBlankEntry
  },
  {
    0x0a,
    L"PI-primary, PI-secondary",
    PCIBlankEntry
  },
  {
    0x0b,
    L"OM/PI-primary, PI-secondary",
    PCIBlankEntry
  },
  {
    0x0c,
    L"OM-secondary",
    PCIBlankEntry
  },
  {
    0x0d,
    L"OM-primary, OM/PI-secondary",
    PCIBlankEntry
  },
  {
    0x0e,
    L"PI-primary, OM/PI-secondary",
    PCIBlankEntry
  },
  {
    0x0f,
    L"OM/PI-primary, OM/PI-secondary",
    PCIBlankEntry
  },
  {
    0x80,
    L"Master",
    PCIBlankEntry
  },
  {
    0x81,
    L"Master, OM-primary",
    PCIBlankEntry
  },
  {
    0x82,
    L"Master, PI-primary",
    PCIBlankEntry
  },
  {
    0x83,
    L"Master, OM/PI-primary",
    PCIBlankEntry
  },
  {
    0x84,
    L"Master, OM-secondary",
    PCIBlankEntry
  },
  {
    0x85,
    L"Master, OM-primary, OM-secondary",
    PCIBlankEntry
  },
  {
    0x86,
    L"Master, PI-primary, OM-secondary",
    PCIBlankEntry
  },
  {
    0x87,
    L"Master, OM/PI-primary, OM-secondary",
    PCIBlankEntry
  },
  {
    0x88,
    L"Master, OM-secondary",
    PCIBlankEntry
  },
  {
    0x89,
    L"Master, OM-primary, PI-secondary",
    PCIBlankEntry
  },
  {
    0x8a,
    L"Master, PI-primary, PI-secondary",
    PCIBlankEntry
  },
  {
    0x8b,
    L"Master, OM/PI-primary, PI-secondary",
    PCIBlankEntry
  },
  {
    0x8c,
    L"Master, OM-secondary",
    PCIBlankEntry
  },
  {
    0x8d,
    L"Master, OM-primary, OM/PI-secondary",
    PCIBlankEntry
  },
  {
    0x8e,
    L"Master, PI-primary, OM/PI-secondary",
    PCIBlankEntry
  },
  {
    0x8f,
    L"Master, OM/PI-primary, OM/PI-secondary",
    PCIBlankEntry
  },
  {
    0x00,
    NULL,
    /* null string ends the list */NULL
  }
};

PCI_CLASS_ENTRY PCIPIFClass_0300[] = {
  {
    0x00,
    L"VGA compatible",
    PCIBlankEntry
  },
  {
    0x01,
    L"8514 compatible",
    PCIBlankEntry
  },
  {
    0x00,
    NULL,
    /* null string ends the list */NULL
  }
};

PCI_CLASS_ENTRY PCIPIFClass_0604[] = {
  {
    0x00,
    L"",
    PCIBlankEntry
  },
  {
    0x01,
    L"Subtractive decode",
    PCIBlankEntry
  },
  {
    0x00,
    NULL,
    /* null string ends the list */NULL
  }
};

PCI_CLASS_ENTRY PCIPIFClass_0700[] = {
  {
    0x00,
    L"Generic XT-compatible",
    PCIBlankEntry
  },
  {
    0x01,
    L"16450-compatible",
    PCIBlankEntry
  },
  {
    0x02,
    L"16550-compatible",
    PCIBlankEntry
  },
  {
    0x03,
    L"16650-compatible",
    PCIBlankEntry
  },
  {
    0x04,
    L"16750-compatible",
    PCIBlankEntry
  },
  {
    0x05,
    L"16850-compatible",
    PCIBlankEntry
  },
  {
    0x06,
    L"16950-compatible",
    PCIBlankEntry
  },
  {
    0x00,
    NULL,
    /* null string ends the list */NULL
  }
};

PCI_CLASS_ENTRY PCIPIFClass_0701[] = {
  {
    0x00,
    L"",
    PCIBlankEntry
  },
  {
    0x01,
    L"Bi-directional",
    PCIBlankEntry
  },
  {
    0x02,
    L"ECP 1.X-compliant",
    PCIBlankEntry
  },
  {
    0x03,
    L"IEEE 1284",
    PCIBlankEntry
  },
  {
    0xfe,
    L"IEEE 1284 target (not a controller)",
    PCIBlankEntry
  },
  {
    0x00,
    NULL,
    /* null string ends the list */NULL
  }
};

PCI_CLASS_ENTRY PCIPIFClass_0703[] = {
  {
    0x00,
    L"Generic",
    PCIBlankEntry
  },
  {
    0x01,
    L"Hayes-compatible 16450",
    PCIBlankEntry
  },
  {
    0x02,
    L"Hayes-compatible 16550",
    PCIBlankEntry
  },
  {
    0x03,
    L"Hayes-compatible 16650",
    PCIBlankEntry
  },
  {
    0x04,
    L"Hayes-compatible 16750",
    PCIBlankEntry
  },
  {
    0x00,
    NULL,
    /* null string ends the list */NULL
  }
};

PCI_CLASS_ENTRY PCIPIFClass_0800[] = {
  {
    0x00,
    L"Generic 8259",
    PCIBlankEntry
  },
  {
    0x01,
    L"ISA",
    PCIBlankEntry
  },
  {
    0x02,
    L"EISA",
    PCIBlankEntry
  },
  {
    0x10,
    L"IO APIC",
    PCIBlankEntry
  },
  {
    0x20,
    L"IO(x) APIC interrupt controller",
    PCIBlankEntry
  },
  {
    0x00,
    NULL,
    /* null string ends the list */NULL
  }
};

PCI_CLASS_ENTRY PCIPIFClass_0801[] = {
  {
    0x00,
    L"Generic 8237",
    PCIBlankEntry
  },
  {
    0x01,
    L"ISA",
    PCIBlankEntry
  },
  {
    0x02,
    L"EISA",
    PCIBlankEntry
  },
  {
    0x00,
    NULL,
    /* null string ends the list */NULL
  }
};

PCI_CLASS_ENTRY PCIPIFClass_0802[] = {
  {
    0x00,
    L"Generic 8254",
    PCIBlankEntry
  },
  {
    0x01,
    L"ISA",
    PCIBlankEntry
  },
  {
    0x02,
    L"EISA",
    PCIBlankEntry
  },
  {
    0x00,
    NULL,
    /* null string ends the list */NULL
  }
};

PCI_CLASS_ENTRY PCIPIFClass_0803[] = {
  {
    0x00,
    L"Generic",
    PCIBlankEntry
  },
  {
    0x01,
    L"ISA",
    PCIBlankEntry
  },
  {
    0x02,
    L"EISA",
    PCIBlankEntry
  },
  {
    0x00,
    NULL,
    /* null string ends the list */NULL
  }
};

PCI_CLASS_ENTRY PCIPIFClass_0904[] = {
  {
    0x00,
    L"Generic",
    PCIBlankEntry
  },
  {
    0x10,
    L"",
    PCIBlankEntry
  },
  {
    0x00,
    NULL,
    /* null string ends the list */NULL
  }
};

PCI_CLASS_ENTRY PCIPIFClass_0c00[] = {
  {
    0x00,
    L"Universal Host Controller spec",
    PCIBlankEntry
  },
  {
    0x10,
    L"Open Host Controller spec",
    PCIBlankEntry
  },
  {
    0x80,
    L"No specific programming interface",
    PCIBlankEntry
  },
  {
    0xfe,
    L"(Not Host Controller)",
    PCIBlankEntry
  },
  {
    0x00,
    NULL,
    /* null string ends the list */NULL
  }
};

PCI_CLASS_ENTRY PCIPIFClass_0c03[] = {
  {
    0x00,
    L"",
    PCIBlankEntry
  },
  {
    0x10,
    L"Using 1394 OpenHCI spec",
    PCIBlankEntry
  },
  {
    0x00,
    NULL,
    /* null string ends the list */NULL
  }
};

PCI_CLASS_ENTRY PCIPIFClass_0e00[] = {
  {
    0x00,
    L"Message FIFO at offset 40h",
    PCIBlankEntry
  },
  {
    0x01,
    L"",
    PCIBlankEntry
  },
  {
    0x00,
    NULL,
    /* null string ends the list */NULL
  }
};

//
// Implemetations
//
VOID
PciGetClassStrings (
  IN      UINT32               ClassCode,
  IN OUT  PCI_CLASS_STRINGS    *ClassStrings
  )
/*++
Routine Description:

  Generates printable Unicode strings that represent PCI device class,
  subclass and programmed I/F based on a value passed to the function.

Arguments:

  ClassCode      Value representing the PCI "Class Code" register read from a 
                 PCI device. The encodings are:
                     bits 23:16 - Base Class Code
                     bits 15:8  - Sub-Class Code
                     bits  7:0  - Programming Interface
  ClassStrings   Pointer of PCI_CLASS_STRINGS structure, which contains 
                 printable class strings corresponding to ClassCode. The
                 caller must not modify the strings that are pointed by 
                 the fields in ClassStrings.
Returns:

  None
--*/
{
  INTN            Index;
  UINT8           Code;
  PCI_CLASS_ENTRY *CurrentClass;

  //
  // Assume no strings found
  //
  ClassStrings->BaseClass = L"UNDEFINED";
  ClassStrings->SubClass  = L"UNDEFINED";
  ClassStrings->PIFClass  = L"UNDEFINED";

  CurrentClass = gClassStringList;
  Code = (UINT8) (ClassCode >> 16);
  Index = 0;

  //
  // Go through all entries of the base class, until the entry with a matching
  // base class code is found. If reaches an entry with a null description
  // text, the last entry is met, which means no text for the base class was
  // found, so no more action is needed.
  //
  while (Code != CurrentClass[Index].Code) {
    if (NULL == CurrentClass[Index].DescText) {
      return ;
    }

    Index++;
  }
  //
  // A base class was found. Assign description, and check if this class has
  // sub-class defined. If sub-class defined, no more action is needed,
  // otherwise, continue to find description for the sub-class code.
  //
  ClassStrings->BaseClass = CurrentClass[Index].DescText;
  if (NULL == CurrentClass[Index].LowerLevelClass) {
    return ;
  }
  //
  // find Subclass entry
  //
  CurrentClass  = CurrentClass[Index].LowerLevelClass;
  Code          = (UINT8) (ClassCode >> 8);
  Index         = 0;

  //
  // Go through all entries of the sub-class, until the entry with a matching
  // sub-class code is found. If reaches an entry with a null description
  // text, the last entry is met, which means no text for the sub-class was
  // found, so no more action is needed.
  //
  while (Code != CurrentClass[Index].Code) {
    if (NULL == CurrentClass[Index].DescText) {
      return ;
    }

    Index++;
  }
  //
  // A class was found for the sub-class code. Assign description, and check if
  // this sub-class has programming interface defined. If no, no more action is
  // needed, otherwise, continue to find description for the programming
  // interface.
  //
  ClassStrings->SubClass = CurrentClass[Index].DescText;
  if (NULL == CurrentClass[Index].LowerLevelClass) {
    return ;
  }
  //
  // Find programming interface entry
  //
  CurrentClass  = CurrentClass[Index].LowerLevelClass;
  Code          = (UINT8) ClassCode;
  Index         = 0;

  //
  // Go through all entries of the I/F entries, until the entry with a
  // matching I/F code is found. If reaches an entry with a null description
  // text, the last entry is met, which means no text was found, so no more
  // action is needed.
  //
  while (Code != CurrentClass[Index].Code) {
    if (NULL == CurrentClass[Index].DescText) {
      return ;
    }

    Index++;
  }
  //
  // A class was found for the I/F code. Assign description, done!
  //
  ClassStrings->PIFClass = CurrentClass[Index].DescText;
  return ;
}

VOID
PciPrintClassCode (
  IN      UINT8               *ClassCodePtr,
  IN      BOOLEAN             IncludePIF
  )
/*++
Routine Description:

  Print strings that represent PCI device class, subclass and programmed I/F

Arguments:

  ClassCodePtr   Points to the memory which stores register Class Code in PCI 
                 configuation space.
  IncludePIF     If the printed string should include the programming I/F part
Returns:

  None
--*/
{
  UINT32            ClassCode;
  PCI_CLASS_STRINGS ClassStrings;
  CHAR16            OutputString[PCI_CLASS_STRING_LIMIT + 1];

  ClassCode = 0;
  ClassCode |= ClassCodePtr[0];
  ClassCode |= (ClassCodePtr[1] << 8);
  ClassCode |= (ClassCodePtr[2] << 16);

  //
  // Get name from class code
  //
  PciGetClassStrings (ClassCode, &ClassStrings);

  if (IncludePIF) {
    //
    // Only print base class and sub class name
    //
    PrintToken (
      STRING_TOKEN (STR_PCI_CLASS_THREE_VARS),
      HiiHandle,
      ClassStrings.BaseClass,
      ClassStrings.SubClass,
      ClassStrings.PIFClass
      );

  } else {
    //
    // Print base class, sub class, and programming inferface name
    //
    SPrint (
      OutputString,
      PCI_CLASS_STRING_LIMIT * sizeof (CHAR16),
      L"%s - %s",
      ClassStrings.BaseClass,
      ClassStrings.SubClass
      );

    OutputString[PCI_CLASS_STRING_LIMIT] = 0;
    PrintToken (STRING_TOKEN (STR_PCI_CLASS_ONE_VARS), HiiHandle, OutputString);
  }
}
