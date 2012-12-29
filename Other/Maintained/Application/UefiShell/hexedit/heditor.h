/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

  Module Name:
    HEditor.h

  Abstract:
    Main include file for hex editor

--*/

#ifndef _EFI_SHELL_HEXEDIT_H_
#define _EFI_SHELL_HEXEDIT_H_

#define EFI_HEXEDIT_GUID \
  { \
    0x39d796d9, 0x3544, 0x4166, 0x84, 0x6e, 0x21, 0xf9, 0x1b, 0xe0, 0xa8, 0x72 \
  }

#include "heditortype.h"

#include "libeditor.h"

#include "libbufferimage.h"
#include "libfileimage.h"
#include "libdiskimage.h"
#include "libmemimage.h"

#include "libtitlebar.h"
#include "libstatusbar.h"
#include "libinputbar.h"
#include "libmenubar.h"

#include "libmisc.h"

#include "libclipboard.h"

//
// This is the generated header file which includes whatever needs to be exported (strings + IFR)
//
#include STRING_DEFINES_FILE

extern HEFI_EDITOR_GLOBAL_EDITOR  HMainEditor;
extern BOOLEAN                    HEditorFirst;
extern BOOLEAN                    HEditorExit;

extern EFI_HII_HANDLE             HiiHandle;
extern EFI_HII_PROTOCOL           *Hii;

#endif // _HEDITOR_H
