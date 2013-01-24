/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  pci_class.h

Abstract:

  Supporting routines for EFI shell command "pci"

Revision History

--*/

#ifndef _PCI_CLASS_H_
#define _PCI_CLASS_H_

#include "Tiano.h"  // for UINT32 etc.
#define PCI_CLASS_STRING_LIMIT  54
//
// Printable strings for Pci class code
//
typedef struct {
  CHAR16  *BaseClass; // Pointer to the PCI base class string
  CHAR16  *SubClass;  // Pointer to the PCI sub class string
  CHAR16  *PIFClass;  // Pointer to the PCI programming interface string
} PCI_CLASS_STRINGS;

//
// a structure holding a single entry, which also points to its lower level
// class
//
typedef struct PCI_CLASS_ENTRY_TAG {
  UINT8                       Code;             // Class, subclass or I/F code
  CHAR16                      *DescText;        // Description string
  struct PCI_CLASS_ENTRY_TAG  *LowerLevelClass; // Subclass or I/F if any
} PCI_CLASS_ENTRY;

VOID
PciGetClassStrings (
  IN      UINT32               ClassCode,
  IN OUT  PCI_CLASS_STRINGS    *ClassStrings
  );

VOID
PciPrintClassCode (
  IN      UINT8                *ClassCodePtr,
  IN      BOOLEAN              IncludePIF
  );

#endif // _PCI_CLASS_H_
