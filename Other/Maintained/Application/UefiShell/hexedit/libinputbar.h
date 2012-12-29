/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

  Module Name:
    libInputBar.h

  Abstract:
    Definition of the user input bar (the same position with the Status Bar)

--*/

#ifndef _LIB_INPUT_BAR_H_
#define _LIB_INPUT_BAR_H_

#include "heditortype.h"

EFI_STATUS
HMainInputBarInit (
  VOID
  );
EFI_STATUS
HMainInputBarCleanup (
  VOID
  );
EFI_STATUS
HMainInputBarRefresh (
  VOID
  );
EFI_STATUS
HMainInputBarSetPrompt (
  CHAR16 *
  );
EFI_STATUS
HMainInputBarSetStringSize (
  UINTN
  );

#endif
