/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  VarCheck.h

Abstract:

  declares shell variable check functions

Revision History

--*/

#ifndef _VARCHECK_H_
#define _VARCHECK_H_

#include "EfiShellLib.h"
//
// Shell command variable checking support
//
typedef enum {
  FlagTypeSingle      = 0,
  FlagTypeNeedVar,
  FlagTypeNeedSet,
  FlagTypeSkipUnknown
} SHELL_VAR_CHECK_FLAG_TYPE;

typedef enum {
  ARG_NO_ATTRIB         = 0x0,
  ARG_IS_QUOTED         = 0x1,
  ARG_PARTIALLY_QUOTED  = 0x2,
  ARG_FIRST_HALF_QUOTED = 0x4,
  ARG_FIRST_CHAR_IS_ESC = 0x8
};

typedef struct {
  CHAR16                    *FlagStr;
  UINT32                    FlagID;
  UINT32                    ConflictMask;
  SHELL_VAR_CHECK_FLAG_TYPE FlagType;
} SHELL_VAR_CHECK_ITEM;

typedef struct _SHELL_ARG_LIST {
  CHAR16                  *FlagStr;
  CHAR16                  *VarStr;
  UINTN                   Index;
  struct _SHELL_ARG_LIST  *Next;
} SHELL_ARG_LIST;

#define GetNextArg(a)   ((a)->Next)
#define GetFirstArg(a)  ((a)->VarList)
#define GetFirstFlag(a) ((a)->FlagList)

typedef struct {
  SHELL_ARG_LIST  *FlagList;
  SHELL_ARG_LIST  *VarList;
  UINTN           FlagCount;
  UINTN           ValueCount;
} SHELL_VAR_CHECK_PACKAGE;

typedef enum {
  VarCheckReserved      = -1,
  VarCheckOk            = 0,
  VarCheckDuplicate,
  VarCheckConflict,
  VarCheckUnknown,
  VarCheckLackValue,
  VarCheckOutOfMem
} SHELL_VAR_CHECK_CODE;

VOID
LibCheckVarFreeVarList (
  IN SHELL_VAR_CHECK_PACKAGE    *ChkPck
  );

SHELL_ARG_LIST  *
LibCheckVarGetFlag (
  IN SHELL_VAR_CHECK_PACKAGE   *ChkPck,
  IN CHAR16                    *FlagStr
  );

SHELL_VAR_CHECK_CODE
LibCheckVariables (
  IN VOID                             *ShellInt,
  IN SHELL_VAR_CHECK_ITEM             *CheckList,
  OUT SHELL_VAR_CHECK_PACKAGE         *ChkPck,
  OUT CHAR16                          **Useful
  );

SHELL_VAR_CHECK_CODE
LibCheckRedirVariables (
  IN VOID                             *si,
  IN SHELL_VAR_CHECK_ITEM             *CheckList,
  OUT SHELL_VAR_CHECK_PACKAGE         *ChkPck,
  OUT CHAR16                          **Useful
  );

EFI_STATUS
LibGetStdRedirFilename (
  OUT CHAR16             **FullFilename
  );

EFI_STATUS
LibGetErrRedirFilename (
  OUT CHAR16             **FullFilename
  );

#endif
