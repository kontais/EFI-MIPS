/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  VarCheck.c
   
Abstract:

  Provides uniform shell environment variable check and classification

Revision History

--*/

#include "EfiShellLib.h"

VOID
LibCheckVarFreeVarList (
  IN SHELL_VAR_CHECK_PACKAGE    *ChkPck
  )
{
  SHELL_ARG_LIST  *Item;
  ASSERT (ChkPck != NULL);
  
  Item = NULL;
  while (ChkPck->FlagList) {
    Item              = ChkPck->FlagList;
    ChkPck->FlagList  = ChkPck->FlagList->Next;
    FreePool (Item);
  }

  while (ChkPck->VarList) {
    Item            = ChkPck->VarList;
    ChkPck->VarList = ChkPck->VarList->Next;
    FreePool (Item);
  }
}

SHELL_ARG_LIST *
LibCheckVarGetFlag (
  IN SHELL_VAR_CHECK_PACKAGE   *ChkPck,
  IN CHAR16                    *FlagStr
  )
{
  SHELL_ARG_LIST  *Item;
  
  ASSERT (ChkPck);
  ASSERT (FlagStr);
  
  Item = ChkPck->FlagList;
  while (Item) {
    if (0 == StriCmp (Item->FlagStr, FlagStr)) {
      return Item;
    }

    Item = Item->Next;
  }

  return NULL;
}

BOOLEAN
iCompareSetFlag (
  IN CHAR16                      *FlagName,
  IN CHAR16                      *CmdVar
  )
{
  ASSERT (FlagName);
  ASSERT (CmdVar);

  if (StrLen (CmdVar) < StrLen (FlagName)) {
    return FALSE;
  }

  while (*FlagName) {
    if (*FlagName != *CmdVar) {
      break;
    }

    FlagName++;
    CmdVar++;
  }

  return (BOOLEAN)!FlagName[0];
}

SHELL_VAR_CHECK_CODE
LibCheckVariables (
  IN VOID                            *si,
  IN SHELL_VAR_CHECK_ITEM            *CheckList,
  OUT SHELL_VAR_CHECK_PACKAGE        *ChkPck,
  OUT CHAR16                         **Useful
  )
/*++
Routine Description:

Arguments:

  si        - The SI
  CheckList - The check list
  ChkPck    - The check package
  Useful    - The useful

Returns:

--*/
{
  UINTN               Index;
  UINTN               Item;
  UINT32              Mask;
  SHELL_ARG_LIST      **p;
  BOOLEAN             QuoteVar;
  EFI_SHELL_INTERFACE *ShellInt;
  EFI_SHELL_ARG_INFO  *ArgInfo;

  ASSERT (si != NULL);         
  ASSERT (CheckList != NULL);
  ASSERT (ChkPck != NULL);
  ASSERT (Useful != NULL);

  ShellInt            = (EFI_SHELL_INTERFACE *) si;
  Mask                = 0;
  ChkPck->FlagCount   = 0;
  ChkPck->ValueCount  = 0;
  QuoteVar            = FALSE;
  //
  // ArgInfo array contains associated properties of each argument
  //
  ArgInfo = ShellInt->ArgInfo;
  for (Index = 1; Index < ShellInt->Argc; Index++) {
    if (ArgInfo) {
      //
      // If an argument was totally quoted on the command line,
      // we don't think it is possible to be a flag
      //
      QuoteVar = (BOOLEAN) ((ArgInfo[Index].Attributes & ARG_IS_QUOTED) && !(ArgInfo[Index].Attributes & ARG_PARTIALLY_QUOTED));
    }

    if (QuoteVar ||
    //
    // If an argument has the attribute of ARG_FIRST_CHAR_IS_ESC,
    // we don't think it is possible to be a flag.
    //
    (ArgInfo && (ArgInfo[Index].Attributes & ARG_FIRST_CHAR_IS_ESC)) ||
    //
    // If an argument's first-half was in quotation,
    // we don't think it is possible to be a flag.
    //
    (ArgInfo && (ArgInfo[Index].Attributes & ARG_FIRST_HALF_QUOTED)) ||
    //
    // If the first char of an argument is neither - nor +,
    // of course it can not be a flag.
    //
    (L'-' != ShellInt->Argv[Index][0] && L'+' != ShellInt->Argv[Index][0])) {
      //
      // Find the last element in the VarList, then allocate a new element
      // to store the current argument.
      //
      p = &ChkPck->VarList;
      while (*p) {
        p = &((*p)->Next);
      }

      *p = (SHELL_ARG_LIST *) AllocateZeroPool (sizeof (SHELL_ARG_LIST));
      if (NULL == *p) {
        return VarCheckOutOfMem;
      }
      (*p)->VarStr  = ShellInt->Argv[Index];
      (*p)->Index   = Index;
      ChkPck->ValueCount++;
      continue;
    }
    //
    // We got here, the current argument will be taken as a flag.
    //
    for (Item = 0; CheckList[Item].FlagStr; Item++) {
      if (FlagTypeNeedSet == CheckList[Item].FlagType) {
        if (!iCompareSetFlag (CheckList[Item].FlagStr, ShellInt->Argv[Index])) {
          continue;
        }
      } else if (StriCmp (ShellInt->Argv[Index], CheckList[Item].FlagStr)) {
        continue;
      }
      //
      // check conflict
      //
      if (Mask & CheckList[Item].ConflictMask) {
        *Useful = ShellInt->Argv[Index];
        return VarCheckConflict;
      }
      //
      // check duplicate
      //
      p = &ChkPck->FlagList;
      while (*p) {
        if (FlagTypeNeedSet == CheckList[Item].FlagType) {
          if (iCompareSetFlag ((*p)->FlagStr, ShellInt->Argv[Index])) {
            *Useful = (*p)->FlagStr;
            return VarCheckDuplicate;
          }
        } else if (0 == StriCmp (ShellInt->Argv[Index], (*p)->FlagStr)) {
          *Useful = ShellInt->Argv[Index];
          return VarCheckDuplicate;
        }

        p = &(*p)->Next;
      }

      *p = AllocateZeroPool (sizeof (SHELL_ARG_LIST));
      if (NULL == *p) {
        return VarCheckOutOfMem;
      }
      (*p)->Index = Index;

      switch (CheckList[Item].FlagType) {
      case FlagTypeNeedSet:
        (*p)->FlagStr = CheckList[Item].FlagStr;
        (*p)->VarStr  = &ShellInt->Argv[Index][StrLen (CheckList[Item].FlagStr)];
        break;

      case FlagTypeSingle:
        (*p)->FlagStr = ShellInt->Argv[Index];
        break;

      case FlagTypeNeedVar:
        if (Index < ShellInt->Argc - 1) {
          (*p)->FlagStr = ShellInt->Argv[Index];
          (*p)->VarStr  = ShellInt->Argv[Index + 1];
          Index++;
        } else {
          *Useful = ShellInt->Argv[Index];
          return VarCheckLackValue;
        }
        break;

      default:
        ASSERT (FALSE);
      }

      Mask |= CheckList[Item].FlagID;
      ChkPck->FlagCount++;
      break;
    }

    if (NULL == CheckList[Item].FlagStr) {
      *Useful = ShellInt->Argv[Index];
      return VarCheckUnknown;
    }
  }

  return VarCheckOk;
}

EFI_STATUS
_GetRedirFilename (
  IN     CHAR16               *RedirFlag,
  OUT CHAR16                  **FullFilename
  )
{
  UINTN           Index;
  EFI_LIST_ENTRY  FileList;
  EFI_STATUS      Status;
  SHELL_FILE_ARG  *File;
  
  ASSERT (FullFilename != NULL);
  ASSERT ((SI->RedirArgc % 2) == 0);
  
  for (Index = 0; Index < SI->RedirArgc; Index += 2) {
    if (StrStr (SI->RedirArgv[Index], RedirFlag) == 1) {
      break;
    }
  }

  if (Index == SI->RedirArgc) {
    return EFI_NOT_FOUND;
  }

  InitializeListHead (&FileList);
  Status = ShellFileMetaArg (SI->RedirArgv[Index + 1], &FileList);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (IsListEmpty (&FileList) || FileList.Flink->Flink != &FileList) {
    return EFI_INVALID_PARAMETER;
  }

  File          = CR (FileList.Flink, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);
  *FullFilename = StrDuplicate (File->FullName);
  if (NULL == *FullFilename) {
    return EFI_OUT_OF_RESOURCES;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
LibGetStdRedirFilename (
  OUT CHAR16             **FullFilename
  )
{
  return _GetRedirFilename (L">", FullFilename);
}

EFI_STATUS
LibGetErrRedirFilename (
  OUT CHAR16             **FullFilename
  )
{
  return _GetRedirFilename (L"2>", FullFilename);
}
