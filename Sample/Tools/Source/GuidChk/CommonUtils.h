/*++

Copyright (c) 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:  

  CommonUtils.h
  
Abstract:

  Common utility defines and structure definitions.
  
--*/

#ifndef _COMMON_UTILS_H_
#define _COMMON_UTILS_H_

//
// Basic types
//
typedef unsigned char  UINT8;
typedef char           INT8;
typedef unsigned short UINT16;
typedef unsigned int   UINT32;
typedef void           VOID;
typedef long long      UINT64;
typedef unsigned char  CHAR8;
typedef unsigned int   UINTN;
typedef long long      INT64;

typedef short WORD;
typedef int   DWORD;
typedef char  CHAR;

typedef UINT8 BOOLEAN;
typedef UINT32 STATUS;

typedef void *HANDLE;
#define INVALID_HANDLE_VALUE    ((HANDLE)(-1))

#define FILE_TYPE_UNKNOWN   0x0000
#define FILE_TYPE_DISK      0x0001
#define FILE_TYPE_CHAR      0x0002
#define FILE_TYPE_PIPE      0x0003
#define FILE_TYPE_REMOTE    0x8000

#define FILE_ATTRIBUTE_READONLY         0x00000001
#define FILE_ATTRIBUTE_HIDDEN           0x00000002
#define FILE_ATTRIBUTE_SYSTEM           0x00000004
#define FILE_ATTRIBUTE_DIRECTORY        0x00000010
#define FILE_ATTRIBUTE_ARCHIVE          0x00000020
#define FILE_ATTRIBUTE_NORMAL           0x00000080
#define FILE_ATTRIBUTE_TEMPORARY        0x00000100

#define TRUE            1
#define FALSE           0

#define STATUS_SUCCESS  0
#define STATUS_WARNING  1
#define STATUS_ERROR    2

//
// Linked list of strings
//
typedef struct _STRING_LIST {
  struct _STRING_LIST *Next;
  char                *Str;
} STRING_LIST;

int
CreateGuidList (
  INT8    *OutFileName
  )
;

#endif // #ifndef _COMMON_UTILS_H_
