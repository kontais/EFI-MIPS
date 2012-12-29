/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  if.c 
  
Abstract:

  Internal Shell cmd "if" & "endif"

Revision History

--*/

#include "shelle.h"
#include "shellenvguid.h"

//
// Internal prototypes
//
EFI_STATUS
CheckIfFileExists (
  IN  CHAR16                   *FileName,
  OUT BOOLEAN                  *FileExists
  );

EFI_STATUS
EFIAPI
SEnvCmdIf (
  IN  EFI_HANDLE               ImageHandle,
  IN  EFI_SYSTEM_TABLE         *SystemTable
  )
/*++

Routine Description:

  Shell command "if" for conditional execution in script files.

Arguments:
  ImageHandle      The image handle
  SystemTable      The system table

Returns:
  EFI_SUCCESS      The command finished sucessfully
  EFI_UNSUPPORTED  Unsupported
  EFI_INVALID_PARAMETER Invalid parameter

--*/
{
  BOOLEAN     ExistNot;
  UINTN       NotPos;
  BOOLEAN     ExistI;
  UINTN       IPos;
  EFI_STATUS  Status;
  CHAR16      *FileName;
  BOOLEAN     FileExists;
  CHAR16      *String1;
  CHAR16      *String2;
  BOOLEAN     Success;
  BOOLEAN     Condition;

  EFI_SHELL_APP_INIT (ImageHandle, SystemTable);

  //
  //  Output help
  //
  if (SI->Argc == 2) {
    if (StriCmp (SI->Argv[1], L"-?") == 0) {
      if (IS_OLD_SHELL) {
        PrintToken (STRING_TOKEN (STR_NO_HELP), HiiEnvHandle);
      } else {
        PrintToken (STRING_TOKEN (STR_SHELLENV_IF_VERBOSE_HELP), HiiEnvHandle);
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
        PrintToken (STRING_TOKEN (STR_SHELLENV_IF_VERBOSE_HELP), HiiEnvHandle);
      }

      return EFI_SUCCESS;
    }
  }

  if (!SEnvBatchIsActive ()) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_IF_ONLY_SUPPORTED_SCRIPT), HiiEnvHandle);
    return EFI_UNSUPPORTED;
  }
  //
  //  There are 2 forms of the if command:
  //    if [/i][not] exist file then
  //    if [/i][not] string1 == string2
  //  Both forms have argument count not less than 4
  //  i switch if the case sensitive.
  //
  if (SI->Argc < 4) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_IF_INCORRECT_SYNTAX), HiiEnvHandle, SEnvGetLineNumber ());
    SEnvSetBatchAbort ();
    return EFI_INVALID_PARAMETER;
  }

  if (!SEnvBatchGetGotoActive ()) {
    //
    // If Goto is not active, do argument checking, judge the condition,
    // then push the statement to stack.
    //
    if ((StriCmp (SI->Argv[1], L"/i") == 0)) {
      ExistI  = TRUE;
      IPos    = 1;
    } else {
      ExistI  = FALSE;
      IPos    = 0;
    }

    NotPos = ExistI ? 2 : 1;
    if ((StriCmp (SI->Argv[NotPos], L"not") == 0)) {
      ExistNot = TRUE;
    } else {
      ExistNot = FALSE;
      NotPos--;
    }

    if (StriCmp (SI->Argv[NotPos + 1], L"exist") == 0) {
      //
      //  first form of the command, test for file existence
      //
      if ((SI->Argc != NotPos + 4) || (StriCmp (SI->Argv[NotPos + 3], L"then") != 0)) {
        PrintToken (STRING_TOKEN (STR_SHELLENV_IF_INCORRECT_SYNTAX), HiiEnvHandle, SEnvGetLineNumber ());
        SEnvSetBatchAbort ();
        return EFI_INVALID_PARAMETER;
      }

      FileName = SI->Argv[NotPos + 2];

      //
      //  Test for file existence
      //
      Status = CheckIfFileExists (FileName, &FileExists);
      if (EFI_ERROR (Status)) {
        PrintToken (
          STRING_TOKEN (STR_SHELLENV_IF_CHECK_FILE_EXISTENCE),
          HiiEnvHandle,
          Status,
          SEnvGetLineNumber ()
          );
        SEnvSetBatchAbort ();
        return Status;
      }

      Status = SEnvBatchPushStmtStack (StmtIf, FALSE);
      if (EFI_ERROR (Status)) {
        PrintToken (STRING_TOKEN (STR_SHELLENV_IF_CANNOT_EXECUTE_SCRIPT), HiiEnvHandle, Status);
        SEnvSetBatchAbort ();
        return Status;
      }

      Condition = (BOOLEAN) (ExistNot ? !FileExists : FileExists);
      Status    = SEnvBatchSetCondition (Condition);
      if (EFI_ERROR (Status)) {
        PrintToken (STRING_TOKEN (STR_SHELLENV_IF_CANNOT_EXECUTE_SCRIPT), HiiEnvHandle, Status);
        SEnvSetBatchAbort ();
        return Status;
      }

    } else {
      //
      //  Second form of the command, compare two strings
      //
      if ((SI->Argc != NotPos + 5) ||
          (StriCmp (SI->Argv[NotPos + 2], L"==") != 0) ||
          (StriCmp (SI->Argv[NotPos + 4], L"then") != 0)
          ) {
        PrintToken (STRING_TOKEN (STR_SHELLENV_IF_INCORRECT_SYNTAX), HiiEnvHandle, SEnvGetLineNumber ());
        SEnvSetBatchAbort ();
        return EFI_INVALID_PARAMETER;
      }

      String1 = SI->Argv[NotPos + 1];
      String2 = SI->Argv[NotPos + 3];

      Status  = SEnvBatchPushStmtStack (StmtIf, FALSE);
      if (EFI_ERROR (Status)) {
        PrintToken (STRING_TOKEN (STR_SHELLENV_IF_CANNOT_EXECUTE_SCRIPT), HiiEnvHandle, Status);
        SEnvSetBatchAbort ();
        return Status;
      }

      if (ExistNot) {
        if (ExistI) {
          Condition = (BOOLEAN) (StriCmp (String1, String2) != 0);
        } else {
          Condition = (BOOLEAN) (StrCmp (String1, String2) != 0);
        }
      } else {
        if (ExistI) {
          Condition = (BOOLEAN) (StriCmp (String1, String2) == 0);
        } else {
          Condition = (BOOLEAN) (StrCmp (String1, String2) == 0);
        }
      }

      Status = SEnvBatchSetCondition (Condition);
      if (EFI_ERROR (Status)) {
        PrintToken (STRING_TOKEN (STR_SHELLENV_IF_CANNOT_EXECUTE_SCRIPT), HiiEnvHandle, Status);
        SEnvSetBatchAbort ();
        return Status;
      }
    }

    return Status;

  } else {
    //
    // If Goto is active, maintain the JumpStmt so that it always points to
    // the current statement, or maintain the ExtraStmtStack if this statement
    // does not belong the the statement stack.
    //
    if (SEnvBatchGetRewind ()) {
      Status = SEnvTryMoveUpJumpStmt (StmtIf, &Success);
      if (EFI_ERROR (Status)) {
        PrintToken (STRING_TOKEN (STR_SHELLENV_IF_CANNOT_EXECUTE_SCRIPT), HiiEnvHandle, Status);
        SEnvSetBatchAbort ();
        return Status;
      }

      if (Success) {
        return Status;
      }
    }

    Status = SEnvBatchPushStmtStack (StmtIf, TRUE);
    if (EFI_ERROR (Status)) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_IF_CANNOT_EXECUTE_SCRIPT), HiiEnvHandle, Status);
      SEnvSetBatchAbort ();
      return Status;
    }

    return Status;
  }
}

EFI_STATUS
CheckIfFileExists (
  IN  CHAR16                   *FileName,
  OUT BOOLEAN                  *FileExists
  )
/*++

Routine Description:

  Check file parameter to see if file exists.  Wildcards are supported. If 
  the argument expands to more than one file names, we still return TRUE in 
  the output parameter FileExists.

Arguments:
  FileName         The file name needs to check existence
  FileExists       Output if the file exists

Returns:
  EFI_SUCCESS      The function completed successfully

--*/
{
  EFI_LIST_ENTRY  FileList;
  EFI_LIST_ENTRY  *Link;
  SHELL_FILE_ARG  *Arg;
  EFI_STATUS      Status;
  UINTN           FileCount;

  *FileExists = FALSE;
  FileCount   = 0;
  InitializeListHead (&FileList);

  //
  // Attempt to open the file, expanding any wildcards.
  //
  Status = ShellFileMetaArg (FileName, &FileList);
  if (EFI_ERROR (Status)) {
    if (Status == EFI_NOT_FOUND) {
      ShellFreeFileList (&FileList);
      return EFI_SUCCESS;

    } else {
      return Status;
    }
  }
  //
  // Go through the list and count the files which already exists
  //
  FileCount = 0;
  for (Link = FileList.Flink; Link != &FileList; Link = Link->Flink) {
    Arg = CR (Link, SHELL_FILE_ARG, Link, SHELL_FILE_ARG_SIGNATURE);
    if (Arg->Handle) {
      //
      //  Non-NULL handle means file was there and open-able
      //
      FileCount += 1;
    }
  }

  if (FileCount > 0) {
    //
    //  Found one or more files, so set the flag to be TRUE
    //
    *FileExists = TRUE;
  }

  ShellFreeFileList (&FileList);
  return Status;
}

EFI_STATUS
EFIAPI
SEnvCmdEndif (
  IN EFI_HANDLE                ImageHandle,
  IN EFI_SYSTEM_TABLE          *SystemTable
  )
/*++


Routine Description:

  Shell command "endif" for conditional execution in script files.

Arguments:
  ImageHandle      The image handle
  SystemTable      The system table

Returns:
  EFI_SUCCESS           The command finished sucessfully
  EFI_UNSUPPORTED       Unsupported
  EFI_INVALID_PARAMETER Invalid parameter
  EFI_ABORTED           Aborted

--*/
{
  EFI_STATUS          Status;
  EFI_BATCH_STATEMENT *Stmt;

  EFI_SHELL_APP_INIT (ImageHandle, SystemTable);

  if (!SEnvBatchIsActive ()) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_IF_ENDIF_ONLY_SUPPORTED_SCRIPT), HiiEnvHandle);
    return EFI_UNSUPPORTED;
  }

  if (SI->Argc > 1) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_IF_ENDIF_TOO_MANY_ARGS), HiiEnvHandle, SEnvGetLineNumber ());
    SEnvSetBatchAbort ();
    return EFI_INVALID_PARAMETER;
  }

  if (!SEnvBatchGetGotoActive ()) {
    //
    // If Goto is not active, pop from the statement stack
    //
    Stmt = SEnvBatchStmtStackTop ();
    if (Stmt == NULL || Stmt->StmtType != StmtIf) {
      PrintToken (
        STRING_TOKEN (STR_SHELLENV_IF_ENDIF_NO_CORRESPONDING),
        HiiEnvHandle,
        SEnvGetLineNumber ()
        );
      SEnvSetBatchAbort ();
      return EFI_ABORTED;
    }

    Status = SEnvBatchPopStmtStack (1, FALSE);
    if (EFI_ERROR (Status)) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_IF_ENDIF_CANNOT_EXECUTE_SCRIPT), HiiEnvHandle, Status);
      SEnvSetBatchAbort ();
      return Status;
    }

  } else {
    //
    // If Goto is active, if ExtraStmtStack is not empty, pop a node from it,
    // otherwise move the JumpStmt down so that it still points to current
    // statement.
    //
    if (!SEnvBatchExtraStackEmpty ()) {
      Stmt = SEnvBatchExtraStackTop ();
      if (Stmt->StmtType != StmtIf) {
        PrintToken (
          STRING_TOKEN (STR_SHELLENV_IF_ENDIF_NO_CORRESPONDING),
          HiiEnvHandle,
          SEnvGetLineNumber ()
          );
        SEnvSetBatchAbort ();
        return EFI_ABORTED;
      }

      Status = SEnvBatchPopStmtStack (1, TRUE);
      if (EFI_ERROR (Status)) {
        PrintToken (STRING_TOKEN (STR_SHELLENV_IF_ENDIF_CANNOT_EXECUTE_SCRIPT), HiiEnvHandle, Status);
        SEnvSetBatchAbort ();
        return Status;
      }

    } else {
      Status = SEnvMoveDownJumpStmt (StmtIf);
      if (EFI_ERROR (Status)) {
        if (Status == EFI_NOT_FOUND) {
          PrintToken (
            STRING_TOKEN (STR_SHELLENV_IF_ENDIF_NO_CORRESPONDING),
            HiiEnvHandle,
            SEnvGetLineNumber ()
            );

        } else {
          PrintToken (STRING_TOKEN (STR_SHELLENV_IF_ENDIF_CANNOT_EXECUTE_SCRIPT), HiiEnvHandle, Status);
        }

        SEnvSetBatchAbort ();
        return Status;
      }
    }
  }

  return Status;
}

EFI_STATUS
EFIAPI
SEnvCmdElse (
  IN  EFI_HANDLE               ImageHandle,
  IN  EFI_SYSTEM_TABLE         *SystemTable
  )
/*++

Routine Description:

  Shell command "else" for conditional execution in script files.

Arguments:
  ImageHandle      The image handle
  SystemTable      The system table

Returns:
  EFI_SUCCESS           The command finished sucessfully
  EFI_UNSUPPORTED       Unsupported
  EFI_INVALID_PARAMETER Invalid parameter
  EFI_ABORTED           Aborted

--*/
{
  EFI_STATUS          Status;
  BOOLEAN             Condition;
  EFI_BATCH_STATEMENT *Stmt;

  Status = EFI_SUCCESS;
  EFI_SHELL_APP_INIT (ImageHandle, SystemTable);

  if (!SEnvBatchIsActive ()) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_IF_ELSE_ONLY_SUPPORTED_SCRIPT), HiiEnvHandle);
    return EFI_UNSUPPORTED;
  }

  if (SI->Argc > 1) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_IF_ELSE_TOO_MANY_ARGS), HiiEnvHandle, SEnvGetLineNumber ());
    SEnvSetBatchAbort ();
    return EFI_INVALID_PARAMETER;
  }

  if (!SEnvBatchGetGotoActive ()) {
    Stmt = SEnvBatchStmtStackTop ();
    if (Stmt == NULL || Stmt->StmtType != StmtIf) {
      PrintToken (
        STRING_TOKEN (STR_SHELLENV_IF_ELSE_NO_CORRESPONDING_IF),
        HiiEnvHandle,
        SEnvGetLineNumber ()
        );
      SEnvSetBatchAbort ();
      return EFI_ABORTED;
    }

    if (Stmt->StmtInfo.IfInfo.FoundElse) {
      PrintToken (
        STRING_TOKEN (STR_SHELLENV_IF_ELSE_ONLY_ONE_ELSE),
        HiiEnvHandle,
        SEnvGetLineNumber ()
        );
      SEnvSetBatchAbort ();
      return EFI_ABORTED;
    }
    //
    // Reverse the Condition flag, and set the FoundElse flag
    //
    Condition                       = SEnvBatchGetCondition ();
    Stmt->StmtInfo.IfInfo.FoundElse = TRUE;
    Status                          = SEnvBatchSetCondition ((BOOLEAN)!Condition);
    if (EFI_ERROR (Status)) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_IF_ELSE_CANNOT_EXECUTE), HiiEnvHandle);
      SEnvSetBatchAbort ();
      return Status;
    }

  } else {
    //
    // If Goto is active, if ExtraStmtStack is not empty, set FoundElse flag of
    // its top node, otherwise set the FoundElse flag of the node JumpStmt is
    // pointing to.
    //
    if (!SEnvBatchExtraStackEmpty ()) {
      Stmt = SEnvBatchExtraStackTop ();
      if (Stmt->StmtType != StmtIf) {
        PrintToken (
          STRING_TOKEN (STR_SHELLENV_IF_ELSE_NO_CORRESPONDING_IF),
          HiiEnvHandle,
          SEnvGetLineNumber ()
          );
        SEnvSetBatchAbort ();
        return EFI_ABORTED;
      }

      if (Stmt->StmtInfo.IfInfo.FoundElse) {
        PrintToken (
          STRING_TOKEN (STR_SHELLENV_IF_ELSE_ONLY_ONE_ELSE),
          HiiEnvHandle,
          SEnvGetLineNumber ()
          );
        SEnvSetBatchAbort ();
        return EFI_ABORTED;
      }

      Stmt->StmtInfo.IfInfo.FoundElse = TRUE;

    } else {
      if (SEnvGetJumpStmt ()->StmtInfo.IfInfo.FoundElse) {
        PrintToken (
          STRING_TOKEN (STR_SHELLENV_IF_ELSE_ONLY_ONE_ELSE),
          HiiEnvHandle,
          SEnvGetLineNumber ()
          );
        SEnvSetBatchAbort ();
        return EFI_ABORTED;
      }

      SEnvGetJumpStmt ()->StmtInfo.IfInfo.FoundElse = TRUE;
    }
  }

  return Status;
}

EFI_STATUS
EFIAPI
SEnvCmdIfGetLineHelp (
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
  return SEnvCmdGetStringByToken (STRING_TOKEN (STR_SHELLENV_IF_LINE_HELP), Str);
}

EFI_STATUS
EFIAPI
SEnvCmdEndifGetLineHelp (
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

EFI_STATUS
EFIAPI
SEnvCmdElseGetLineHelp (
  OUT CHAR16              **Str
  )
/*++

Routine Description:

  Get this command's line help

Arguments:

  Str - The line help

Returns:

  EFI_UNSUPPORTED   - This command have no line help

--*/
{
  return EFI_UNSUPPORTED;
}
