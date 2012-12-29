/*++

  Copyright (c) 1999 Intel Corporation

  Module Name:
    Editor.h

  Abstract:
    Main include file for text editor

--*/


#ifndef _EDITOR_H_
#define _EDITOR_H_


#include "editortype.h"


#include "libeditor.h"
#include "libfilebuffer.h"
#include "libtitlebar.h"
#include "libstatusbar.h"
#include "libinputbar.h"
#include "libmenubar.h"
#include "libMisc.h"

extern  EFI_EDITOR_GLOBAL_EDITOR    MainEditor;
extern  BOOLEAN		EditorFirst;
extern  BOOLEAN		EditorExit;

#endif  // _EDITOR_H
