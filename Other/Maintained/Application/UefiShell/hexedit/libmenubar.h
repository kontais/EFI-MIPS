/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

  Module Name:
    libMenuBar.h

  Abstract:
    Definition of the Menu Bar for the text editor

--*/

#ifndef _LIB_MENU_BAR_H_
#define _LIB_MENU_BAR_H_

#include "heditortype.h"

EFI_STATUS
HMainMenuBarInit (
  VOID
  );
EFI_STATUS
HMainMenuBarCleanup (
  VOID
  );
EFI_STATUS
HMainMenuBarHide (
  VOID
  );
EFI_STATUS
HMainMenuBarRefresh (
  VOID
  );
EFI_STATUS
HMainMenuBarHandleInput (
  EFI_INPUT_KEY *
  );
EFI_STATUS
HMainMenuBarBackup (
  VOID
  );

EFI_STATUS
HMainCommandOpenFile (
  VOID
  );
EFI_STATUS
HMainCommandOpenDisk (
  VOID
  );
EFI_STATUS
HMainCommandOpenMemory (
  VOID
  );

EFI_STATUS
HMainCommandSaveBuffer (
  VOID
  );

EFI_STATUS
HMainCommandSelectStart (
  VOID
  );
EFI_STATUS
HMainCommandSelectEnd (
  VOID
  );

EFI_STATUS
HMainCommandCut (
  VOID
  );
EFI_STATUS
HMainCommandPaste (
  VOID
  );

EFI_STATUS
HMainCommandGoToOffset (
  VOID
  );

EFI_STATUS
HMainCommandExit (
  VOID
  );

#endif
