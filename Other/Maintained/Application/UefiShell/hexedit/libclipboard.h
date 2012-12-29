/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

  Module Name:
    libclipboard.h

  Abstract:
    Defines DiskImage - the view of the file that is visible at any point, 
    as well as the event handlers for editing the file

--*/

#ifndef _LIB_CLIP_BOARD_H_
#define _LIB_CLIP_BOARD_H_

#include "heditortype.h"

EFI_STATUS
HClipBoardInit (
  VOID
  );
EFI_STATUS
HClipBoardCleanup (
  VOID
  );

EFI_STATUS
HClipBoardSet (
  UINT8   *,
  UINTN
  );
UINTN
HClipBoardGet (
  UINT8 **
  );

#endif
