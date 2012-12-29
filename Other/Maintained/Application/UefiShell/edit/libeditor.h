/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

  Module Name:
    libEditor.h

  Abstract:
    Defines the Main Editor data type - 
     - Global variables 
     - Instances of the other objects of the editor
     - Main Interfaces

--*/

#ifndef _LIB_EDITOR_H_
#define _LIB_EDITOR_H_

#include "editortype.h"

EFI_STATUS
MainEditorInit (
  VOID
  );
EFI_STATUS
MainEditorCleanup (
  VOID
  );
EFI_STATUS
MainEditorRefresh (
  VOID
  );
EFI_STATUS
MainEditorKeyInput (
  VOID
  );
EFI_STATUS
MainEditorBackup (
  VOID
  );

EFI_STATUS
MainEditorSetCutLine (
  EFI_EDITOR_LINE *
  );

#endif
