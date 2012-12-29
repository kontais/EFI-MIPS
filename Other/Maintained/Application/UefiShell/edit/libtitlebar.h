/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

  Module Name:
    libTitleBar.h

  Abstract:
    declares interface functions
    
--*/

#ifndef _LIB_TITLE_BAR_H_
#define _LIB_TITLE_BAR_H_

#include "editortype.h"

EFI_STATUS
MainTitleBarInit (
  VOID
  );
EFI_STATUS
MainTitleBarCleanup (
  VOID
  );

EFI_STATUS
MainTitleBarRefresh (
  VOID
  );

EFI_STATUS
MainTitleBarSetTitle (
  CHAR16 *
  );
EFI_STATUS
MainTitleBarBackup (
  VOID
  );

#endif
