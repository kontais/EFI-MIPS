/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

  Module Name:
    libMemImage.h

  Abstract:
    Defines MemImage - the view of the file that is visible at any point, 
    as well as the event handlers for editing the file

--*/

#ifndef _LIB_MEM_IMAGE_H_
#define _LIB_MEM_IMAGE_H_

#include "heditortype.h"

EFI_STATUS
HMemImageInit (
  VOID
  );
EFI_STATUS
HMemImageCleanup (
  VOID
  );
EFI_STATUS
HMemImageBackup (
  VOID
  );

EFI_STATUS
HMemImageSetMemOffsetSize (
  IN UINTN,
  IN UINTN
  );

EFI_STATUS
HMemImageRead (
  IN UINTN,
  IN UINTN,
  IN BOOLEAN
  );
EFI_STATUS
HMemImageSave (
  IN UINTN,
  IN UINTN
  );

#endif
