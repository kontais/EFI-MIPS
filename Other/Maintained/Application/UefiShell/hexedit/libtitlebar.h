/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  libtitlebar.h

Abstract:

  functions that title bar supports

--*/

#ifndef _LIB_TITLE_BAR_H_
#define _LIB_TITLE_BAR_H_

#include "heditortype.h"

EFI_STATUS
HMainTitleBarInit (
  VOID
  );
EFI_STATUS
HMainTitleBarCleanup (
  VOID
  );
EFI_STATUS
HMainTitleBarRefresh (
  VOID
  );
EFI_STATUS
HMainTitleBarSetTitle (
  CHAR16 *
  );
EFI_STATUS
HMainTitleBarBackup (
  VOID
  );

#endif
