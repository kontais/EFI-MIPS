/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  tzone.h

Abstract:

  declares interface functions

Revision History

--*/

#ifndef _TIME_ZONE_H
#define _TIME_ZONE_H

#include "EfiShellLib.h"

#define EFI_TZONE_GUID \
  { \
    0x8a34eb81, 0xd290, 0x4877, \
    { \
      0xa7, 0x7b, 0x3a, 0x16, 0xd8, 0xe2, 0xf6, 0xaa \
    } \
  }

#endif
