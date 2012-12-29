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

    Definition for the Status Bar - 
    the display for the status of the editor

--*/

#ifndef _LIB_STATUS_BAR_H_
#define _LIB_STATUS_BAR_H_

#include "heditortype.h"

EFI_STATUS
HMainStatusBarInit (
  VOID
  );
EFI_STATUS
HMainStatusBarCleanup (
  VOID
  );
EFI_STATUS
HMainStatusBarRefresh (
  VOID
  );
EFI_STATUS
HMainStatusBarHide (
  VOID
  );
EFI_STATUS
HMainStatusBarSetStatusString (
  CHAR16 *
  );
EFI_STATUS
HMainStatusBarSetMode (
  BOOLEAN
  );
EFI_STATUS
HMainStatusBarBackup (
  VOID
  );

#endif
