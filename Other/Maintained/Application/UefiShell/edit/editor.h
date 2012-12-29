/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

  Module Name: 
    Editor.h

  Abstract:
    Main include file for text editor

--*/

#ifndef _EFI_EDITOR_H_
#define _EFI_EDITOR_H_

#define EFI_EDIT_GUID \
  { \
    0x89e82255, 0x5fad, 0x4be8, 0xb2, 0xce, 0xad, 0xa8, 0x38, 0xf9, 0x6c, 0x43 \
  }

#include "editortype.h"

#include "libeditor.h"
#include "libfilebuffer.h"
#include "libtitlebar.h"
#include "libstatusbar.h"
#include "libinputbar.h"
#include "libmenubar.h"
#include "libmisc.h"
//
// This is the generated header file which includes whatever needs to be exported (strings + IFR)
//
#include STRING_DEFINES_FILE

extern EFI_EDITOR_GLOBAL_EDITOR MainEditor;
extern BOOLEAN                  EditorFirst;
extern BOOLEAN                  EditorExit;

extern EFI_HII_HANDLE           HiiHandle;
extern EFI_HII_PROTOCOL         *Hii;

#endif // _EFI_EDITOR_H_
