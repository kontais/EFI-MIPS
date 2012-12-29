/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

   smbiosview.h
    
Abstract:

    Tools of clarify the content of the smbios table.

Revision History

--*/

#ifndef _SMBIOS_VIEW_H
#define _SMBIOS_VIEW_H

#define STRUCTURE_TYPE_RANDOM     (UINT8) 0xFE
#define STRUCTURE_TYPE_INVALID    (UINT8) 0xFF

#define STRUCTURE_HANDLE_INVALID  (UINT16) 0xFFFF

typedef struct {
  UINT16  Index;
  UINT8   Type;
  UINT16  Handle;
  UINT16  Addr;   // offset from table head
  UINT16  Length; // total structure length
} STRUCTURE_STATISTICS;

EFI_STATUS
InitializeBiosViewApplication (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
SMBiosView (
  IN  UINT8     QueryType,
  IN  UINT16    QueryHandle,
  IN  UINT8     Option,
  IN  BOOLEAN   RandomView
  );

EFI_STATUS
WaitEnter (
  VOID
  );

VOID
ShowHelpInfo (
  VOID
  );

VOID
ExitApp (
  EFI_STATUS status
  );

EFI_STATUS
InitSmbiosTableStatistics (
  VOID
  );

EFI_STATUS
DisplayStatisticsTable (
  IN UINT8 Option
  );

CHAR16        *
GetShowTypeString (
  UINT8 ShowType
  );

extern UINT8  gShowType;

#endif
