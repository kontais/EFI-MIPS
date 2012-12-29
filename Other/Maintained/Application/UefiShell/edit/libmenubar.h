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

#include "editortype.h"

EFI_STATUS
MainMenuBarInit (
  VOID
  );
EFI_STATUS
MainMenuBarCleanup (
  VOID
  );
EFI_STATUS
MainMenuBarBackup (
  VOID
  );

EFI_STATUS
MainMenuBarHide (
  VOID
  );
EFI_STATUS
MainMenuBarRefresh (
  VOID
  );
EFI_STATUS
MainMenuBarHandleInput (
  EFI_INPUT_KEY *
  );

EFI_STATUS
MainCommandOpenFile (
  VOID
  );
EFI_STATUS
MainCommandSaveFile (
  VOID
  );

EFI_STATUS
MainCommandExit (
  VOID
  );

EFI_STATUS
MainCommandCutLine (
  VOID
  );
EFI_STATUS
MainCommandPasteLine (
  VOID
  );

EFI_STATUS
MainCommandSearch (
  VOID
  );
EFI_STATUS
MainCommandSearchReplace (
  VOID
  );
EFI_STATUS
MainCommandGotoLine (
  VOID
  );

#endif
