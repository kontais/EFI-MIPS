/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name: 

  for.c
  
Abstract:

  Internal Shell cmd "for" & "endfor"

Revision History

--*/

#include "shelle.h"
#include "shellenvguid.h"

/*
BOOLEAN
CheckInterValue (
  IN CHAR16                        *wszValueStr,
  OUT INT32                        *pnValue
  )
{
  UINTN                                    uPos = 0;
  INT32                                    nSign = 0;
  UINTN                                    uDigitStart = 0;
  BOOLEAN                                bDigitFound = FALSE;
  
  while (wszValueStr[uPos]) {
    if (L'+' == wszValueStr[uPos] || L'-' == wszValueStr[uPos]) {
      if (0 != nSign) {
        // sign already resolved, error
        return FALSE;
      }
      nSign = (L'+' == wszValueStr[uPos]) ? 1 : -1;
      uDigitStart = uPos + 1;
    } else if (wszValueStr[uPos] < L'0' || wszValueStr[uPos] > L'9') {
      // not digits or sign symbol, error
      return FALSE;
    } else {
      if (0 == nSign) {
      	  // no sign symbol before the first digit, take it as postive
        nSign = 1;
        uDigitStart = uPos;
      }
      bDigitFound = TRUE;
    }
    uPos++;
  } // end of while

  if (FALSE == bDigitFound) {
    return FALSE;
  }

  *pnValue = Atoi (wszValueStr + uDigitStart) * nSign;
  return TRUE;
}

BOOLEAN
ExtractFromArg (
  IN CHAR16                             *wszRng,
  OUT INT32                             *pnStart,
  OUT INT32                             *pnEnd,
  OUT INT32                             *pnStep
  )
{
  UINTN                                     uStrLen = 0;
  UINTN                                     uPos = 1;
  CHAR16                                   *pBreak1 = NULL;
  CHAR16                                   *pBreak2 = NULL;
  INT32                                      nTmp;
  

  uStrLen = StrLen (wszRng);
  // there should be at least 5 characters
  if (uStrLen < 5) {
    return FALSE;
  }

  if (L'<' != wszRng[0] || L'>' != wszRng[uStrLen - 1]) {
    return FALSE;
  }

  while (wszRng[uPos] != L'>') {
    if (L',' == wszRng[uPos]) {
      if (NULL == pBreak1) {
        pBreak1 = &wszRng[uPos];
      } else if (NULL == pBreak2) {
        pBreak2 = &wszRng[uPos];
        break;
      }
    }
  } // end of while

  // no ',' found
  if (NULL == pBreak1) {
    return FALSE;
  } else if (pBreak1 == &wszRng[1]) {
    // nothing between '<' and first ','
    return FALSE;
  } else if (pBreak1 == &wszRng[uStrLen - 2]) {
    // nothing between first ',' and '>'
    return FALSE;
  } else if (pBreak1 - &wszRng[0] > 7) {
    // too big number
    return FALSE;
  }
  
  if (NULL != pBreak2) {
    if (pBreak2 = pBreak1 + 1) {
      // nothing between adjacent ','
      return FALSE;
    } else if (pBreak2 == &wszRng[uStrLen - 2]) {
      // nothing between second ',' and '>'
      return FALSE;
    } else if (pBreak2 - pBreak1 > 7) {
      // too big number
      return FALSE;
    }
  }

  *pBreak1 = 0;
  if (FALSE == CheckInterValue(&wszRng[1], &nTmp)) {
    *pBreak1 = L',';
    return FALSE;
  } else {
    *pBreak1 = L',';
    *pnStart = nTmp;
  }

  if (NULL != pBreak2) {
    *pBreak2 = 0;
    if (FALSE == CheckInterValue(pBreak1 + 1, &nTmp)) {
      *pBreak2 = L',';
      return FALSE;
    } else {
      *pBreak2 = L',';
      *pnEnd = nTmp;
    }
  } else {
    pBreak2 = pBreak1;
  }
  
  if (&wszRng[uStrLen - 1] - pBreak2 > 7) {
    // too big number
    return FALSE;
  }

  wszRng[uStrLen - 1] = 0;
  if (FALSE == CheckInterValue(pBreak2 + 1, &nTmp)) {
    wszRng[uStrLen - 1] = L',';
    return FALSE;
  } else {
    wszRng[uStrLen - 1] = L',';
    if (pBreak1 == pBreak2) {
      *pnEnd = nTmp;
      *pnStep = 1;
    } else {
      *pnStep = nTmp;
    }
  }  
  return TRUE;
}

EFI_STATUS
SEnvBatchPushFor2Stack (
  IN  EFI_BATCH_STMT_TYPE            StmtType,
  IN  BOOLEAN                        PushExtraStack
  )
/*++

Routine Description:

  Push a node into statement stack.

Arguments:
  StmtType         Statement type to be pushed
  PushExtraStack   Push into extra statement stack or current statement stack

Returns:
  EFI_SUCCESS      The function finished sucessfully

--
{
  EFI_BATCH_STATEMENT                *Stmt;
  EFI_LIST_ENTRY                     FileList;
  EFI_LIST_ENTRY                     *Link;
  EFI_BATCH_SCRIPT                   *Script;
  EFI_BATCH_VAR_VALUE                *VarValue;
  SHELL_FILE_ARG                     *Arg;
  INTN                                  Index;
  EFI_STATUS                         Status;
  CHAR16                             Char;

  INT32                               nStart, nEnd, nStep;
  CHAR16                             wszValue[10];
  
  Status   = EFI_SUCCESS;
  VarValue = NULL;

  // this function only deals with 'for %var run <...>'
  if (StmtFor != StmtType) {
    return EFI_ABORTED;
  }
  
  if (IsListEmpty (&ScriptStack.ScriptList)) {
    return EFI_ABORTED;
  }
  
  Script = CR (
             ScriptStack.ScriptList.Flink, 
             EFI_BATCH_SCRIPT, 
             Link,
             EFI_BATCH_SCRIPT_SIGNATURE
             );

  Stmt = AllocateZeroPool (sizeof (EFI_BATCH_STATEMENT));
  if (Stmt == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Stmt->Signature    = EFI_BATCH_STMT_SIGNATURE;
  Stmt->StmtType     = StmtType;
  Stmt->BeginFilePos = Script->FilePosition;
  InitializeListHead (&Stmt->StmtInfo.ForInfo.ValueList);

  //
  // If we're pushing if statement to extra statement stack, set condition 
  // to TRUE, for in case we need to jump into this statement later, the 
  // condition should be treated as TRUE.
  //  
  if (Stmt->StmtType == StmtFor) {
    Stmt->StmtInfo.ForInfo.BeginLineNum = SEnvGetLineNumber();
  }

  //
  // if we're pushing FOR statement to the current statement stack, expand the
  // command line argument to a list of variable values.
  //
  if (Stmt->StmtType == StmtFor && 
      !PushExtraStack && 
      SEnvBatchGetCondition ()
      ) {

    if (FALSE == ExtractFromArg(SI->Argv[3], &nStart, &nEnd, &nStep)) {
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }

    Stmt->StmtInfo.ForInfo.VariableName[0] = SI->Argv[1][0];
    
    for (Index = nStart; Index <= nEnd; Index += nStep) {     
        VarValue = AllocateZeroPool (sizeof(EFI_BATCH_VAR_VALUE));
        if (VarValue == NULL) {
          Status = EFI_OUT_OF_RESOURCES;
          goto Done;
        }

        SPrint (wszValue, sizeof(wszValue), "%d", Index);
        VarValue->Signature = EFI_BATCH_VAR_SIGNATURE;        
        VarValue->Value = AllocateZeroPool (StrLen (wszValue));
        if (VarValue->Value == NULL) {
          Status = EFI_OUT_OF_RESOURCES;
          goto Done;
        }
        StrCpy (VarValue->Value, wszValue]);
     
        InsertTailList (&Stmt->StmtInfo.ForInfo.ValueList, &VarValue->Link);     
    }
  }
  
  //
  // Push the node to stack and increment the nest level
  //
  if (PushExtraStack) {
    InsertHeadList (&ExtraStmtStack.StmtList, &Stmt->Link);
    ExtraStmtStack.NestLevel ++;
  
  } else {
    InsertHeadList (&Script->StmtStack.StmtList, &Stmt->Link);
    Script->StmtStack.NestLevel ++;
  }

Done:
  if (EFI_ERROR (Status)) {
    if (VarValue != NULL) {
      if (VarValue->Value != NULL) {
        FreePool (VarValue->Value);
      }
      FreePool (VarValue);
    }
    
    if (Stmt->StmtType == StmtFor) {
      while (!IsListEmpty (&Stmt->StmtInfo.ForInfo.ValueList)) {
        Link = Stmt->StmtInfo.ForInfo.ValueList.Flink;
        
        VarValue = CR (
                     Link, 
                     EFI_BATCH_VAR_VALUE, 
                     Link, 
                     EFI_BATCH_VAR_SIGNATURE
                     );
                     
        if (VarValue->Value != NULL) {
          FreePool (VarValue->Value);
        }
        
        RemoveEntryList (&VarValue->Link);
        FreePool (VarValue);
      }
    }
    
    ShellFreeFileList (&FileList);
    FreePool (Stmt);
  }
    
  return Status;
}
*/
EFI_STATUS
SEnvCmdForRun (
  IN EFI_HANDLE                         hImageHandle,
  IN EFI_SYSTEM_TABLE                   *pSystemTable
  )
{
  EFI_STATUS  Status;
  BOOLEAN     bSuccess;

  Status = EFI_SUCCESS;
  if (!SEnvBatchGetGotoActive ()) {
    //
    // Extra check. This checking is only done when Goto is not active, for
    // it's legal for encountering a loop use a same named variable as the
    // current loop when we're searching for the target label.
    //
    if ((StrLen (SI->Argv[1]) != 1) || !IsAlpha (SI->Argv[1][0])) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_FOR_INCORRECT_SYNTAX), HiiEnvHandle, L"for", SEnvGetLineNumber ());
      SEnvSetBatchAbort ();
      return EFI_INVALID_PARAMETER;
    }
    //
    //  If Goto is not active, then push it to the statement stack.
    //
    Status = SEnvBatchPushFor2Stack (StmtFor, FALSE);
    if (EFI_ERROR (Status)) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_FOR_INCORRECT_SYNTAX), HiiEnvHandle, L"for", SEnvGetLineNumber ());
      SEnvSetBatchAbort ();
      return Status;
    }

  } else {
    //
    // If Goto is active, maintain the JumpStmt or the extra statement stack,
    // so that JumpStmt points to the current statement, or extra stack holds
    // the current statement.
    //
    if (SEnvBatchGetRewind ()) {
      Status = SEnvTryMoveUpJumpStmt (StmtFor, &bSuccess);
      if (EFI_ERROR (Status)) {
        PrintToken (STRING_TOKEN (STR_SHELLENV_FOR_INCORRECT_SYNTAX), HiiEnvHandle, L"for", SEnvGetLineNumber ());
        SEnvSetBatchAbort ();
        return Status;
      }

      if (bSuccess) {
        return Status;
      }
    }

    Status = SEnvBatchPushFor2Stack (StmtFor, TRUE);
    if (EFI_ERROR (Status)) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_FOR_INCORRECT_SYNTAX), HiiEnvHandle, L"for", SEnvGetLineNumber ());
      SEnvSetBatchAbort ();
      return Status;
    }
  }

  return Status;
}

EFI_STATUS
EFIAPI
SEnvCmdFor (
  IN  EFI_HANDLE               ImageHandle,
  IN  EFI_SYSTEM_TABLE         *SystemTable
  )
/*++

Routine Description:

  Shell command "for" for loop in script files.

Arguments:
  ImageHandle      The image handle
  SystemTable      The system table

Returns:
  EFI_SUCCESS      The command finished sucessfully
  EFI_UNSUPPORTED  Unsupported
  EFI_INVALID_PARAMETER Invalid parameter
--*/
{
  EFI_STATUS  Status;
  BOOLEAN     Success;

  EFI_SHELL_APP_INIT (ImageHandle, SystemTable);

  //
  //  Output help
  //
  if (SI->Argc == 2) {
    if (StriCmp (SI->Argv[1], L"-?") == 0) {
      if (IS_OLD_SHELL) {
        PrintToken (STRING_TOKEN (STR_NO_HELP), HiiEnvHandle);
      } else {
        PrintToken (STRING_TOKEN (STR_SHELLENV_FOR_VERBOSE_HELP), HiiEnvHandle);
      }

      return EFI_SUCCESS;
    }
  } else if (SI->Argc == 3) {
    if ((StriCmp (SI->Argv[1], L"-?") == 0 && StriCmp (SI->Argv[2], L"-b") == 0) ||
        (StriCmp (SI->Argv[2], L"-?") == 0 && StriCmp (SI->Argv[1], L"-b") == 0)
        ) {
      EnablePageBreak (DEFAULT_INIT_ROW, DEFAULT_AUTO_LF);
      if (IS_OLD_SHELL) {
        PrintToken (STRING_TOKEN (STR_NO_HELP), HiiEnvHandle);
      } else {
        PrintToken (STRING_TOKEN (STR_SHELLENV_FOR_VERBOSE_HELP), HiiEnvHandle);
      }

      return EFI_SUCCESS;
    }
  }

  if (!SEnvBatchIsActive ()) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_ONLY_SCRIPT), HiiEnvHandle, L"for");
    return EFI_UNSUPPORTED;
  }
  //
  // Check the command line syntax. The syntax of statement for is:
  //   for %<var> in <string | file [[string | file]...]>
  //
  if (SI->Argc < 4) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_FOR_INCORRECT_SYNTAX), HiiEnvHandle, L"for", SEnvGetLineNumber ());
    SEnvSetBatchAbort ();
    return EFI_INVALID_PARAMETER;
  }

  if (0 == StriCmp (SI->Argv[2], L"run")) {
    return SEnvCmdForRun (ImageHandle, SystemTable);
  } else if (StriCmp (SI->Argv[2], L"in") != 0) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_FOR_INCORRECT_SYNTAX), HiiEnvHandle, L"for", SEnvGetLineNumber ());
    SEnvSetBatchAbort ();
    return EFI_INVALID_PARAMETER;
  }

  if (!SEnvBatchGetGotoActive ()) {
    //
    // Extra check. This checking is only done when Goto is not active, for
    // it's legal for encountering a loop use a same named variable as the
    // current loop when we're searching for the target label.
    //
    if ((StrLen (SI->Argv[1]) != 1) || !IsAlpha (SI->Argv[1][0])) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_FOR_INCORRECT_SYNTAX), HiiEnvHandle, L"for", SEnvGetLineNumber ());
      SEnvSetBatchAbort ();
      return EFI_INVALID_PARAMETER;
    }
    //
    //  If Goto is not active, then push it to the statement stack.
    //
    Status = SEnvBatchPushStmtStack (StmtFor, FALSE);
    if (EFI_ERROR (Status)) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_FOR_INCORRECT_SYNTAX), HiiEnvHandle, L"for", SEnvGetLineNumber ());
      SEnvSetBatchAbort ();
      return Status;
    }

  } else {
    //
    // If Goto is active, maintain the JumpStmt or the extra statement stack,
    // so that JumpStmt points to the current statement, or extra stack holds
    // the current statement.
    //
    if (SEnvBatchGetRewind ()) {
      Status = SEnvTryMoveUpJumpStmt (StmtFor, &Success);
      if (EFI_ERROR (Status)) {
        PrintToken (STRING_TOKEN (STR_SHELLENV_FOR_INCORRECT_SYNTAX), HiiEnvHandle, L"for", SEnvGetLineNumber ());
        SEnvSetBatchAbort ();
        return Status;
      }

      if (Success) {
        return Status;
      }
    }

    Status = SEnvBatchPushStmtStack (StmtFor, TRUE);
    if (EFI_ERROR (Status)) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_FOR_INCORRECT_SYNTAX), HiiEnvHandle, L"for", SEnvGetLineNumber ());
      SEnvSetBatchAbort ();
      return Status;
    }
  }

  return Status;
}

EFI_STATUS
EFIAPI
SEnvCmdEndfor (
  IN  EFI_HANDLE               ImageHandle,
  IN  EFI_SYSTEM_TABLE         *SystemTable
  )
/*++

Routine Description:

    Shell command "endfor".

Arguments:
  ImageHandle      The image handle
  SystemTable      The system table

Returns:
  EFI_SUCCESS      The command finished sucessfully
  EFI_UNSUPPORTED  Unsupported
  EFI_ABORTED      Aborted
--*/
{
  EFI_STATUS          Status;
  EFI_LIST_ENTRY      *VarLink;
  EFI_BATCH_STATEMENT *Stmt;
  EFI_BATCH_VAR_VALUE *VarValue;

  EFI_SHELL_APP_INIT (ImageHandle, SystemTable);

  if (!SEnvBatchIsActive ()) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_ENDFOR_ONLY_SUPPORTED_SCRIPT), HiiEnvHandle);
    return EFI_UNSUPPORTED;
  }

  if (SI->Argc > 1) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_ENDFOR_TOO_MANY_ARGUMENTS), HiiEnvHandle, SEnvGetLineNumber ());
    SEnvSetBatchAbort ();
    return EFI_INVALID_PARAMETER;
  }

  if (!SEnvBatchGetGotoActive ()) {
    //
    // If Goto is not active, do the following steps:
    //   1. Check for corresponding "for"
    //   2. Delete one node from variable value list
    //   3. If the variable value list is empty, stop looping, otherwise
    //      continue loop
    //
    Stmt = SEnvBatchStmtStackTop ();
    if (Stmt == NULL || Stmt->StmtType != StmtFor) {
      PrintToken (
        STRING_TOKEN (STR_SHELLENV_ENDFOR_NO_CORRESPDING_FOR),
        HiiEnvHandle,
        SEnvGetLineNumber ()
        );
      SEnvSetBatchAbort ();
      return EFI_ABORTED;
    }
    //
    // It's possible for ValueList to be empty(for example, a "for" in a false
    // condition "if"). If so, we need not delete a value node from it.
    //
    if (!IsListEmpty (&Stmt->StmtInfo.ForInfo.ValueList)) {
      VarLink = Stmt->StmtInfo.ForInfo.ValueList.Flink;
      VarValue = CR (
                  VarLink,
                  EFI_BATCH_VAR_VALUE,
                  Link,
                  EFI_BATCH_VAR_SIGNATURE
                  );

      //
      //  Free the string contained in the first node of variable value list
      //
      if (VarValue->Value != NULL) {
        FreePool (VarValue->Value);
        VarValue->Value = NULL;
      }
      //
      //  Remove the first node from the variable value list
      //
      RemoveEntryList (&VarValue->Link);
      FreePool (VarValue);
      VarValue = NULL;
    }
    //
    //  If there is another value, then jump back to top of loop,
    //  otherwise, exit this FOR loop & pop out the statement.
    //
    if (!IsListEmpty (&Stmt->StmtInfo.ForInfo.ValueList)) {
      //
      //  Set script file position back to top of this loop
      //
      SEnvSetLineNumber (Stmt->StmtInfo.ForInfo.BeginLineNum);
      Status = SEnvBatchSetFilePosition (Stmt->BeginFilePos);
      if (EFI_ERROR (Status)) {
        PrintToken (STRING_TOKEN (STR_SHELLENV_FOR_INCORRECT_SYNTAX), HiiEnvHandle, L"for", SEnvGetLineNumber ());
        SEnvSetBatchAbort ();
        return Status;
      }

    } else {
      //
      // Pop the statement out of stack to exit loop
      //
      Status = SEnvBatchPopStmtStack (1, FALSE);
      if (EFI_ERROR (Status)) {
        PrintToken (STRING_TOKEN (STR_SHELLENV_FOR_INCORRECT_SYNTAX), HiiEnvHandle, L"for", SEnvGetLineNumber ());
        SEnvSetBatchAbort ();
        return Status;
      }
    }

    return EFI_SUCCESS;

  } else {
    //
    // if Goto is active, maintain the JumpStmt or the extra statement stack.
    //
    if (!SEnvBatchExtraStackEmpty ()) {
      Stmt = SEnvBatchExtraStackTop ();
      if (Stmt->StmtType != StmtFor) {
        PrintToken (STRING_TOKEN (STR_SHELLENV_ENDFOR_NO_FOR_STATEMENT), HiiEnvHandle, SEnvGetLineNumber ());
        SEnvSetBatchAbort ();
        return EFI_ABORTED;
      }

      Status = SEnvBatchPopStmtStack (1, TRUE);
      if (EFI_ERROR (Status)) {
        PrintToken (STRING_TOKEN (STR_SHELLENV_FOR_INCORRECT_SYNTAX), HiiEnvHandle, L"for", SEnvGetLineNumber ());
        SEnvSetBatchAbort ();
        return Status;
      }

    } else {
      Status = SEnvMoveDownJumpStmt (StmtFor);
      if (EFI_ERROR (Status)) {
        if (Status == EFI_NOT_FOUND) {
          PrintToken (
            STRING_TOKEN (STR_SHELLENV_ENDFOR_NO_CORRESPDING_FOR),
            HiiEnvHandle,
            SEnvGetLineNumber ()
            );
        } else {
          PrintToken (STRING_TOKEN (STR_SHELLENV_FOR_INCORRECT_SYNTAX), HiiEnvHandle, L"for", SEnvGetLineNumber ());
        }

        SEnvSetBatchAbort ();
        return Status;
      }
    }

    return Status;
  }
}

EFI_STATUS
EFIAPI
SEnvCmdForGetLineHelp (
  OUT CHAR16              **Str
  )
/*++

Routine Description:

  Get this command's line help

Arguments:

  Str - The line help

Returns:

  EFI_SUCCESS   - Success

--*/
{
  return SEnvCmdGetStringByToken (STRING_TOKEN (STR_SHELLENV_FOR_LINE_HELP), Str);
}

EFI_STATUS
EFIAPI
SEnvCmdEndforGetLineHelp (
  OUT CHAR16              **Str
  )
/*++

Routine Description:

  Get this command's line help

Arguments:

  Str - The line help

Returns:

  EFI_SUCCESS   - Success

--*/
{
  return EFI_UNSUPPORTED;
}
