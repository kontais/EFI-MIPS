/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  crc.h

Abstract:

  Infomation about the crc function.
 
Revision History

--*/
#ifndef _CRC_H
#define _CRC_H

VOID
SetCrc (
  IN OUT EFI_TABLE_HEADER           *Hdr
  );

VOID
SetCrcAltSize (
  IN UINTN                          Size,
  IN OUT EFI_TABLE_HEADER           *Hdr
  );

BOOLEAN
CheckCrc (
  IN UINTN                          MaxSize,
  IN OUT EFI_TABLE_HEADER           *Hdr
  );

BOOLEAN
CheckCrcAltSize (
  IN UINTN                          MaxSize,
  IN UINTN                          Size,
  IN OUT EFI_TABLE_HEADER           *Hdr
  );

UINT32
CalculateCrc (
  UINT8 *pt,
  UINTN Size
  );

#endif