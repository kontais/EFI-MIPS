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
    Definition of the user input bar (same position with the Status Bar)

--*/

#ifndef _LIB_INPUT_BAR_H_
#define _LIB_INPUT_BAR_H_

#include "editortype.h"

EFI_STATUS
MainInputBarInit (
  VOID
  );
EFI_STATUS
MainInputBarCleanup (
  VOID
  );
EFI_STATUS
MainInputBarRefresh (
  VOID
  );
EFI_STATUS
MainInputBarSetPrompt (
  CHAR16 *
  );
EFI_STATUS
MainInputBarSetStringSize (
  UINTN
  );

#endif
