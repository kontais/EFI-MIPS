/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

  Module Name:
    libstatusbar.h

  Abstract:
    Definition for the Status Bar - the display for the 
    status of the editor

--*/

#ifndef _LIB_STATUS_BAR_H_
#define _LIB_STATUS_BAR_H_

#include "editortype.h"

EFI_STATUS
MainStatusBarInit (
  VOID
  );
EFI_STATUS
MainStatusBarCleanup (
  VOID
  );
EFI_STATUS
MainStatusBarRefresh (
  VOID
  );
EFI_STATUS
MainStatusBarHide (
  VOID
  );
EFI_STATUS
MainStatusBarSetStatusString (
  CHAR16 *
  );
EFI_STATUS
MainStatusBarSetMode (
  BOOLEAN
  );
EFI_STATUS
MainStatusBarBackup (
  VOID
  );

#endif
