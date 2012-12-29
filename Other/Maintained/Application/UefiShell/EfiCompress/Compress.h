/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  Compress.h

Abstract:

  Header file for compression routine
  
--*/

#ifndef _EFI_SHELL_COMPRESS_H_
#define _EFI_SHELL_COMPRESS_H_

#define EFI_COMPRESS_GUID \
  { \
    0xfe977fed, 0x19f2, 0x4438, 0xaa, 0x5e, 0x51, 0x9a, 0x68, 0xbc, 0x67, 0x94 \
  }

EFI_STATUS
Compress (
  IN      UINT8   *SrcBuffer,
  IN      UINT32  SrcSize,
  IN      UINT8   *DstBuffer,
  IN OUT  UINT32  *DstSize
  );

/*++

Routine Description:

  The compression routine.

Arguments:

  SrcBuffer   - The buffer storing the source data
  SrcSize     - The size of source data
  DstBuffer   - The buffer to store the compressed data
  DstSize     - On input, the size of DstBuffer; On output,
                the size of the actual compressed data.

Returns:

  EFI_BUFFER_TOO_SMALL  - The DstBuffer is too small. In this case,
                DstSize contains the size needed.
  EFI_SUCCESS           - Compression is successful.

--*/

#endif