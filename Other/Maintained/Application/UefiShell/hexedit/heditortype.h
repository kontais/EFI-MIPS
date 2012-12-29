/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  heditortype.h

Abstract:

  data types that are used by editor
  
--*/

#ifndef _HEDITOR_TYPE_H_
#define _HEDITOR_TYPE_H_

#include "EfiShellLib.h"

#include EFI_GUID_DEFINITION (PrimaryConsoleInDevice)
#include EFI_PROTOCOL_DEFINITION (SimplePointer)

#define EDITOR_NAME           L"EFI Hex Editor\0"
#define EDITOR_VERSION        L"0.99\0"

#define EFI_EDITOR_LINE_LIST  'eell'

#define FILE_ATTRIBUTES       EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE
#define FILE_READ_WRITE       EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE
#define FILE_CREATE           EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE

#define DISPLAY_START_ROW     1
#define DISPLAY_START_COLUMN  1
#define DISPLAY_MAX_ROW       HMainEditor.ScreenSize.Row
#define DISPLAY_MAX_COLUMN    HMainEditor.ScreenSize.Column

#define TITLE_BAR_LENGTH      1
#define STATUS_BAR_LENGTH     1
#define MENU_BAR_LENGTH       3

#define TITLE_BAR_LOCATION    DISPLAY_START_ROW
#define STATUS_BAR_LOCATION   (DISPLAY_MAX_ROW - MENU_BAR_LENGTH)
#define INPUT_BAR_LOCATION    STATUS_BAR_LOCATION
#define MENU_BAR_LOCATION     (DISPLAY_MAX_ROW - MENU_BAR_LENGTH + 1)
#define LAST_LINE_LOCATION    DISPLAY_MAX_ROW

#define TEXT_START_ROW        (TITLE_BAR_LOCATION + TITLE_BAR_LENGTH)
#define TEXT_START_COLUMN     DISPLAY_START_COLUMN
#define TEXT_END_ROW          (STATUS_BAR_LOCATION - 1)
#define TEXT_END_COLUMN       DISPLAY_MAX_COLUMN
#define MAX_TEXT_COLUMNS      DISPLAY_MAX_COLUMN
#define MAX_TEXT_ROWS         TEXT_END_ROW

#define HEX_POSITION          (TEXT_START_COLUMN + 9)
#define ASCII_POSITION        ((0x10 * 3) + 2 + HEX_POSITION)

#define NUM_TEXT_ROWS         (MAX_TEXT_ROWS - TEXT_START_ROW + 1)
#define NUM_TEXT_COLUMNS      (MAX_TEXT_COLUMNS - TEXT_START_COLUMN + 1)

#define MIN_POOL_SIZE         125
#define MAX_STRING_LENGTH     127

#define min(a, b)             ((a > b) ? b : a)
#define max(a, b)             ((a > b) ? a : b)

//
// Global variables for input and output
//
#define Out                 ST->ConOut
#define In                  ST->ConIn

#define SCAN_CODE_NULL      0x00
#define SCAN_CODE_UP        0x01
#define SCAN_CODE_DOWN      0x02
#define SCAN_CODE_RIGHT     0x03
#define SCAN_CODE_LEFT      0x04
#define SCAN_CODE_HOME      0x05
#define SCAN_CODE_END       0x06
#define SCAN_CODE_INS       0x07
#define SCAN_CODE_DEL       0x08
#define SCAN_CODE_PGUP      0x09
#define SCAN_CODE_PGDN      0x0A
#define SCAN_CODE_F1        0x0B
#define SCAN_CODE_F2        0x0C
#define SCAN_CODE_F3        0x0D
#define SCAN_CODE_F4        0x0E
#define SCAN_CODE_F5        0x0F
#define SCAN_CODE_F6        0x10
#define SCAN_CODE_F7        0x11
#define SCAN_CODE_F8        0x12
#define SCAN_CODE_F9        0x13
#define SCAN_CODE_F10       0x14
#define SCAN_CODE_F11       0x15
#define SCAN_CODE_F12       0x16
#define SCAN_CODE_ESC       0x17
#define CHAR_BS             0x08
#define CHAR_LF             0x0a
#define CHAR_CR             0x0d

#define IS_VALID_CHAR(x)    (x == SCAN_CODE_NULL)
#define IS_DIRECTION_KEY(x) ((x >= SCAN_CODE_UP) && (x <= SCAN_CODE_PGDN))
#define IS_FUNCTION_KEY(x)  ((x >= SCAN_CODE_F1) && x <= (SCAN_CODE_F12))
#define IS_ESCAPE(x)        (x == SCAN_CODE_ESC)

typedef enum {
  NEW_FILE,
  OPEN_FILE,
  OPEN_DISK,
  OPEN_MEMORY
} IMAGE_TYPE;

typedef struct {
  UINTN Row;
  UINTN Column;
} HEFI_EDITOR_POSITION;

typedef
EFI_STATUS
(*HEFI_MENU_ITEM_FUNCTION) (
  VOID
  );

typedef struct {
  CHAR16                  Name[50];
  CHAR16                  Key[3];
  HEFI_MENU_ITEM_FUNCTION Function;
} HMENU_ITEMS;

typedef struct _HEFI_EDITOR_LINE {
  UINTN           Signature;
  UINT8           Buffer[0x10];
  UINTN           Size;                             // unit is Unicode
  EFI_LIST_ENTRY  Link;
} HEFI_EDITOR_LINE;

typedef struct _HEFI_EDITOR_MENU_ITEM {
  CHAR16                  NameToken;
  CHAR16                  FunctionKeyToken;
  HEFI_MENU_ITEM_FUNCTION Function;
} HEFI_EDITOR_MENU_ITEM;

typedef struct {
  UINT32  Foreground : 4;
  UINT32  Background : 4;
} HEFI_EDITOR_COLOR_ATTRIBUTES;

typedef union {
  HEFI_EDITOR_COLOR_ATTRIBUTES  Colors;
  UINT8                         Data;
} HEFI_EDITOR_COLOR_UNION;

typedef struct {
  UINTN Columns;
  UINTN Rows;
} HEFI_EDITOR_TEXT_MODE;

typedef struct {
  CHAR16  *TitlePrefix;
} HEFI_EDITOR_TITLE_BAR;

typedef struct {
  CHAR16  *Name;

  UINTN   BlockSize;
  UINTN   Size;
  UINTN   Offset;
} HEFI_EDITOR_DISK_IMAGE;

typedef struct {
  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL *IoFncs;

  UINTN                           Offset;
  UINTN                           Size;
} HEFI_EDITOR_MEM_IMAGE;

typedef struct {
  CHAR16  *FileName;
  UINTN   Size;                                     // file size
  BOOLEAN ReadOnly;                                 // file is read-only or not
} HEFI_EDITOR_FILE_IMAGE;

typedef enum {
  NO_BUFFER,
  DISK_BUFFER,
  MEM_BUFFER,
  FILE_BUFFER
} HEFI_EDITOR_ACTIVE_BUFFER_TYPE;

typedef struct {
  EFI_LIST_ENTRY                  *ListHead;        // list head of lines
  HEFI_EDITOR_LINE                *Lines;           // lines of current file
  UINTN                           NumLines;         // number of lines
  HEFI_EDITOR_LINE                *CurrentLine;     // current line cursor is at
  HEFI_EDITOR_POSITION            DisplayPosition;  // cursor position in screen
  HEFI_EDITOR_POSITION            MousePosition;    // mouse position in screen
  HEFI_EDITOR_POSITION            BufferPosition;   // cursor position in buffer
  UINTN                           LowVisibleRow;    // the lowest visible row of file position
  BOOLEAN                         HighBits;         // cursor is at the high4 bits or low4 bits
  BOOLEAN                         Modified;         // BUFFER is modified or not
  HEFI_EDITOR_ACTIVE_BUFFER_TYPE  BufferType;

  HEFI_EDITOR_FILE_IMAGE          *FileImage;
  HEFI_EDITOR_DISK_IMAGE          *DiskImage;
  HEFI_EDITOR_MEM_IMAGE           *MemImage;

} HEFI_EDITOR_BUFFER_IMAGE;

typedef struct {
  CHAR16  *StatusString;
} HEFI_EDITOR_STATUS_BAR;

typedef struct {
  CHAR16  *Prompt;                                  // input bar prompt string
  CHAR16  *ReturnString;
  UINTN   StringSize;                               // size of ReturnString space size
} HEFI_EDITOR_INPUT_BAR;

typedef struct {
  HEFI_EDITOR_MENU_ITEM *MenuItems;
} HEFI_EDITOR_MENU_BAR;

typedef struct {
  UINT8 *Buffer;
  UINTN Size;
} HEFI_EDITOR_CLIPBOARD;

typedef struct {
  HEFI_EDITOR_TITLE_BAR       *TitleBar;
  HEFI_EDITOR_MENU_BAR        *MenuBar;
  HEFI_EDITOR_STATUS_BAR      *StatusBar;
  HEFI_EDITOR_INPUT_BAR       *InputBar;
  HEFI_EDITOR_BUFFER_IMAGE    *BufferImage;
  HEFI_EDITOR_CLIPBOARD       *Clipboard;

  HEFI_EDITOR_COLOR_UNION     ColorAttributes;
  HEFI_EDITOR_POSITION        ScreenSize;           // row number and column number
  BOOLEAN                     MouseSupported;
  EFI_SIMPLE_POINTER_PROTOCOL *MouseInterface;
  INT32                       MouseAccumulatorX;
  INT32                       MouseAccumulatorY;

  UINTN                       SelectStart;          // starting from 1
  UINTN                       SelectEnd;            // starting from 1
} HEFI_EDITOR_GLOBAL_EDITOR;

#endif
