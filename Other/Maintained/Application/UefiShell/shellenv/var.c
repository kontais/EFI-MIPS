/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  var.c
  
Abstract: 

  Shell environment variable management

Revision History

--*/

#include "shelle.h"
#include "shellenvguid.h"

//
//  The shell running mode
//
CHAR16  *mShellMode[] = {
  EFI_SHELL_COMPATIBLE_MODE_VER,
  EFI_SHELL_ENHANCED_MODE_VER,
  NULL
};

typedef
EFI_STATUS
(*SPECIAL_VARIABLE_CHECK_FUNCTION) (
  IN CHAR16                                *Value,
  IN BOOLEAN                               Delete,
  IN BOOLEAN                               Volatile
  );

typedef
EFI_STATUS
(*SPECIAL_VARIABLE_GET_FUNCTION) (
  OUT CHAR16                           **Value
  );

typedef struct {
  CHAR16                          *CmdName;
  CHAR16                          *VarName;
  SPECIAL_VARIABLE_CHECK_FUNCTION SpecCheckFun;
  SPECIAL_VARIABLE_GET_FUNCTION   SpecGetFun;
} SPECIAL_VARIABLE_TABLE;

EFI_STATUS
SpecShellModeCheckFun (
  IN CHAR16                                *Value,
  IN BOOLEAN                               Delete,
  IN BOOLEAN                               Volatile
  );

EFI_STATUS
SpecShellModeGetFun (
  OUT CHAR16                               **Value
  );

SPECIAL_VARIABLE_TABLE  mSpecVarTable[] = {
  {
    L"set",
    L"efishellmode",
    SpecShellModeCheckFun,
    SpecShellModeGetFun
  },
  {
    NULL,
    NULL,
    NULL,
    NULL
  }
};

SHELL_VAR_CHECK_ITEM    VarCheckList[] = {
  {
    L"-d",
    0x01,
    0x02,
    FlagTypeSingle
  },
  {
    L"-v",
    0x02,
    0x01,
    FlagTypeSingle
  },
  {
    L"-b",
    0x04,
    0,
    FlagTypeSingle
  },
  {
    L"-?",
    0x08,
    0,
    FlagTypeSingle
  },
  {
    NULL,
    0,
    0,
    0
  }
};

//
// The different variable categories
//
EFI_LIST_ENTRY          SEnvEnv;
EFI_LIST_ENTRY          SEnvMap;
EFI_LIST_ENTRY          SEnvAlias;

VOID
SEnvInitVariables (
  VOID
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  CHAR16          *Name;
  CHAR16          *Data;
  UINTN           BufferSize;
  UINTN           NameSize;
  UINTN           DataSize;
  EFI_GUID        Id;
  EFI_LIST_ENTRY  *ListHead;
  VARIABLE_ID     *Var;
  EFI_STATUS      Status;
  BOOLEAN         IsString;
  UINT32          Attributes;

  //
  // Initialize the different variable lists
  //
  InitializeListHead (&SEnvEnv);
  InitializeListHead (&SEnvMap);
  InitializeListHead (&SEnvAlias);

  BufferSize  = 1024;
  Name        = AllocatePool (BufferSize);
  Data        = AllocatePool (BufferSize);
  ASSERT (Name && Data);

  Var = NULL;

  //
  // Read all the variables in the system and collect ours
  //
  Name[0] = 0;
  for (;;) {
    NameSize  = BufferSize;
    Status    = RT->GetNextVariableName (&NameSize, Name, &Id);
    if (EFI_ERROR (Status)) {
      break;
    }
    //
    // See if it's a shellenv variable
    //
    ListHead  = NULL;
    IsString  = FALSE;
    if (CompareGuid (&Id, &SEnvEnvId) == 0) {
      ListHead  = &SEnvEnv;
      IsString  = TRUE;
    }

    if (CompareGuid (&Id, &SEnvAliasId) == 0) {
      ListHead  = &SEnvAlias;
      IsString  = TRUE;
    }

    if (ListHead) {
      DataSize  = BufferSize;
      Status    = RT->GetVariable (Name, &Id, &Attributes, &DataSize, Data);

      if (!EFI_ERROR (Status)) {
        //
        // Add this value
        //
        SEnvAddVarToList (
          ListHead,
          Name,
          (UINT8 *) Data,
          DataSize,
          (BOOLEAN) ((Attributes & EFI_VARIABLE_NON_VOLATILE) == 0)
          );
      }
    }
    //
    // If this is a protocol entry, add it
    //
    if (CompareGuid (&Id, &SEnvProtId) == 0) {

      DataSize  = BufferSize;
      Status    = RT->GetVariable (Name, &Id, &Attributes, &DataSize, Data);

      if (!EFI_ERROR (Status) && DataSize == sizeof (EFI_GUID)) {
        SEnvIAddProtocol (FALSE, (EFI_GUID *) Data, NULL, NULL, Name);
      } else {
        DEBUG (
          (EFI_D_INIT | EFI_D_WARN,
          "SEnvInitVariables: skipping bogus protocol id %s\n",
          Var->Name)
          );
        RT->SetVariable (Name, &SEnvProtId, 0, 0, NULL);
      }
    }
  }

  FreePool (Name);
  FreePool (Data);
}

CHAR16 *
SEnvIGetStr (
  IN CHAR16           *Name,
  IN EFI_LIST_ENTRY   *Head
  )
/*++

Routine Description:

Arguments:

  Name - The variable name
  Head - The variable list
Returns:

--*/
{
  VARIABLE_ID *Var;
  CHAR16      *Value;

  AcquireLock (&SEnvLock);

  //
  // Walk through linked list to find corresponding Var
  //
  Value = NULL;
  Var   = SEnvFindVar (Head, Name);
  if (Var != NULL) {
    Value = Var->u.Str;
  }

  ReleaseLock (&SEnvLock);

  return Value;
}

CHAR16 *
EFIAPI
SEnvGetMap (
  IN CHAR16           *Name
  )
/*++

Routine Description:

Arguments:

  Name - The variable name

Returns:

--*/
{
  return SEnvIGetStr (Name, &SEnvMap);
}

CHAR16 *
EFIAPI
SEnvGetEnv (
  IN CHAR16           *Name
  )
/*++

Routine Description:

Arguments:

  Name - The variable name

Returns:

--*/
{
  UINTN       Index;
  CHAR16      *Str;
  EFI_STATUS  Status;

  for (Index = 0; mSpecVarTable[Index].CmdName != NULL; Index++) {
    if (StriCmp (mSpecVarTable[Index].CmdName, L"set") == 0 &&
        StriCmp (mSpecVarTable[Index].VarName, Name) == 0 &&
        mSpecVarTable[Index].SpecGetFun != NULL
        ) {
      Status = mSpecVarTable[Index].SpecGetFun (&Str);
      if (EFI_ERROR (Status)) {
        return NULL;
      }

      return Str;
    }
  }

  return SEnvIGetStr (Name, &SEnvEnv);
}

CHAR16 *
SEnvGetAlias (
  IN CHAR16           *Name
  )
/*++

Routine Description:

Arguments:

  Name - The variable name

Returns:

--*/
{
  return SEnvIGetStr (Name, &SEnvAlias);
}

VOID
SEnvSortVarList (
  IN EFI_LIST_ENTRY           *Head
  )
/*++

Routine Description:

Arguments:

  Head - The variable list head

Returns:

--*/
{
  ASSERT_LOCKED (&SEnvLock);

  return ;
}

VARIABLE_ID *
SEnvAddVarToList (
  IN EFI_LIST_ENTRY           *Head,
  IN CHAR16                   *Name,
  IN UINT8                    *Value,
  IN UINTN                    ValueSize,
  IN BOOLEAN                  Volatile
  )
/*++

Routine Description:

  Add a variable to the variable list

Arguments:
  Head        - The head of the variable list
  Name        - The name of the variable
  Value       - The value of the variable
  ValueSize   - The value size of the variable
  Volatile    - Indicate whether the variable is volatile

Returns:

--*/
{
  UINTN       Size;
  VARIABLE_ID *Var;

  Size  = sizeof (VARIABLE_ID) + StrSize (Name) + ValueSize;
  Var   = AllocateZeroPool (Size);
  if (Var == NULL) {
    return NULL;
  }

  Var->Signature  = VARIABLE_SIGNATURE;
  Var->u.Value    = ((UINT8 *) Var) + sizeof (VARIABLE_ID);
  Var->Name       = (CHAR16 *) (Var->u.Value + ValueSize);
  Var->ValueSize  = ValueSize;
  CopyMem (Var->u.Value, Value, ValueSize);
  StrCpy (Var->Name, Name);
  Var->Flags = (UINT8) (Volatile ? 0 : VAR_ID_NON_VOL);

  InsertTailList (Head, &Var->Link);
  return Var;
}

EFI_STATUS
SEnvAddVar (
  IN EFI_LIST_ENTRY           *Head,
  IN EFI_GUID                 *Guid,
  IN CHAR16                   *Name,
  IN UINT8                    *Value,
  IN UINTN                    ValueSize,
  IN BOOLEAN                  Volatile
  )
/*++

Routine Description:
  
  Add a variable to both the variable store and the variable list
  
Arguments:
  Head        - The head of the variable list
  Guid        - The GUID of the variable
  Name        - The name of the variable
  Value       - The value of the variable
  ValueSize   - The value size of the variable
  Volatile    - Indicate whether the variable is volatile

Returns:

--*/
{
  EFI_STATUS  Status;

  Status = RT->SetVariable (
                Name,
                Guid,
                EFI_VARIABLE_BOOTSERVICE_ACCESS | (Volatile ? 0 : EFI_VARIABLE_NON_VOLATILE),
                ValueSize,
                Value
                );

  if (!EFI_ERROR (Status)) {
    SEnvAddVarToList (Head, Name, Value, ValueSize, Volatile);
  }

  return Status;
}

VARIABLE_ID *
SEnvFindVar (
  IN EFI_LIST_ENTRY           *Head,
  IN CHAR16                   *Name
  )
/*++

Routine Description:

  Find the specified variable in the specified variable list

Arguments:
  Head        - The head of the variable list to find in
  Name        - The name of the variable to find

Returns:
  If not found, return NULL. Otherwise, return the pointer to the variable.

--*/
{
  EFI_LIST_ENTRY  *Link;
  VARIABLE_ID     *Var;
  VARIABLE_ID     *Found;

  Found = NULL;
  for (Link = Head->Flink; Link != Head; Link = Link->Flink) {
    Var = CR (Link, VARIABLE_ID, Link, VARIABLE_SIGNATURE);
    if (StriCmp (Var->Name, Name) == 0) {
      Found = Var;
      break;
    }
  }

  return Found;
}

EFI_STATUS
SEnvCmdGetStringByToken (
  IN     UINT16         Token,
  IN OUT CHAR16         **Str
  )
{
  EFI_STATUS  Status;
  CHAR16      *String;

  ASSERT (Str);
  *Str = NULL;
  Status = LibGetStringByToken (
            HiiEnvHandle,
            Token,
            NULL,
            &String
            );
  if (!EFI_ERROR (Status)) {
    *Str = String;
  }

  return Status;
}

EFI_STATUS
EFIAPI
SEnvCmdAliasGetLineHelp (
  IN OUT CHAR16         **Str
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
  return SEnvCmdGetStringByToken (STRING_TOKEN (STR_SHELLENV_ALIAS_LINE_HELP), Str);
}

EFI_STATUS
EFIAPI
SEnvCmdSetGetLineHelp (
  IN OUT CHAR16         **Str
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
  return SEnvCmdGetStringByToken (STRING_TOKEN (STR_SHELLENV_SET_LINE_HELP), Str);
}

BOOLEAN
_IsReserved (
  IN CHAR16                   *Name,
  IN CHAR16                   *Cmd
  )
{
  UINTN Index;

  for (Index = 0; mSpecVarTable[Index].CmdName != NULL; Index++) {
    if (!StriCmp (Cmd, mSpecVarTable[Index].CmdName) && !StriCmp (Name, mSpecVarTable[Index].VarName)) {
      return TRUE;
    }
  }

  return FALSE;
}

EFI_STATUS
SEnvCmdSA (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable,
  IN EFI_LIST_ENTRY           *Head,
  IN EFI_GUID                 *Guid,
  IN CHAR16                   *CmdName,
  IN UINT16                   VHlpToken
  )
/*++

Routine Description:

  Code for shell "set" & "alias" command

Arguments:

  ImageHandle - The image handle
  SystemTable - The system table
  Head        - The variable list head
  Guid        - The guid
  CmdName     - The command name
  VHlpToken   - The help token

Returns:

--*/
{
  EFI_LIST_ENTRY          *Link;
  VARIABLE_ID             *Var;
  VARIABLE_ID             *Found;
  CHAR16                  *Name;
  CHAR16                  *Value;
  UINTN                   SLen;
  UINTN                   Len;
  BOOLEAN                 Delete;
  EFI_STATUS              Status;
  BOOLEAN                 Volatile;
  UINTN                   Index;
  SHELL_VAR_CHECK_CODE    RetCode;
  CHAR16                  *Useful;
  SHELL_VAR_CHECK_PACKAGE ChkPck;
  SHELL_ARG_LIST          *ArgItem;

  EFI_SHELL_APP_INIT (ImageHandle, SystemTable);

  Status = LibFilterNullArgs ();
  if (EFI_ERROR (Status)) {
    goto Quit;
  }

  ZeroMem (&ChkPck, sizeof (SHELL_VAR_CHECK_PACKAGE));
  ArgItem = NULL;

  RetCode = LibCheckVariables (SI, VarCheckList, &ChkPck, &Useful);
  if (VarCheckOk != RetCode) {
    switch (RetCode) {
    case VarCheckConflict:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_FLAG_CONFLICT), HiiEnvHandle, CmdName, Useful);
      break;

    case VarCheckDuplicate:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_DUP_FLAG), HiiEnvHandle, CmdName, Useful);
      break;

    case VarCheckUnknown:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_UNKNOWN_FLAG), HiiEnvHandle, CmdName, Useful);
      break;

    default:
      break;
    }

    Status = EFI_INVALID_PARAMETER;
    goto Quit;
  }
  //
  // Initializing variable to aVOID level 4 warning
  //
  Name      = NULL;
  Value     = NULL;
  Delete    = FALSE;
  Status    = EFI_SUCCESS;
  Found     = NULL;
  Var       = NULL;
  Volatile  = FALSE;

  //
  // Crack arguments
  //
  if (ChkPck.ValueCount > 2) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiEnvHandle, CmdName);
    Status = EFI_INVALID_PARAMETER;
    goto Quit;
  }

  if (LibCheckVarGetFlag (&ChkPck, L"-b") != NULL) {
    EnablePageBreak (DEFAULT_INIT_ROW, DEFAULT_AUTO_LF);
  }

  if (LibCheckVarGetFlag (&ChkPck, L"-?") != NULL) {
    if (IS_OLD_SHELL) {
      PrintToken (STRING_TOKEN (STR_NO_HELP), HiiEnvHandle);
      goto Quit;
    }

    if (ChkPck.ValueCount > 0 ||
        ChkPck.FlagCount > 2 ||
        (2 == ChkPck.FlagCount && !LibCheckVarGetFlag (&ChkPck, L"-b"))
        ) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiEnvHandle, CmdName);
      Status = EFI_INVALID_PARAMETER;
      goto Quit;
    }

    PrintToken (VHlpToken, HiiEnvHandle);
    Status = EFI_SUCCESS;
    goto Quit;
  }

  if (LibCheckVarGetFlag (&ChkPck, L"-d") != NULL) {
    if (ChkPck.ValueCount > 1) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiEnvHandle, CmdName);
      Status = EFI_INVALID_PARAMETER;
      goto Quit;
    } else if (ChkPck.ValueCount < 1) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_FEW), HiiEnvHandle, CmdName);
      Status = EFI_INVALID_PARAMETER;
      goto Quit;
    }

    Delete = TRUE;
  }

  if (LibCheckVarGetFlag (&ChkPck, L"-v") != NULL) {
    if (ChkPck.ValueCount < 2) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_FEW), HiiEnvHandle, CmdName);
      Status = EFI_INVALID_PARAMETER;
      goto Quit;
    }

    Volatile = TRUE;
  }

  ArgItem = GetFirstArg (&ChkPck);
  if (NULL != ArgItem) {
    if (SEnvGetCmdDispath (ArgItem->VarStr) && (StriCmp (CmdName, L"alias") == 0)) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_VAR_INTERNAL_COMAMND), HiiEnvHandle, CmdName, ArgItem->VarStr);
      Status = EFI_INVALID_PARAMETER;
      goto Quit;
    } else if (StrSubCmp (L"*", ArgItem->VarStr, 1) == TRUE ||
             StrSubCmp (L"?", ArgItem->VarStr, 1) == TRUE ||
             StrSubCmp (L"<", ArgItem->VarStr, 1) == TRUE ||
             StrSubCmp (L">", ArgItem->VarStr, 1) == TRUE ||
             StrSubCmp (L"#", ArgItem->VarStr, 1) == TRUE ||
             StrSubCmp (L":", ArgItem->VarStr, 1) == TRUE ||
             StrSubCmp (L"^", ArgItem->VarStr, 1) == TRUE ||
             StrSubCmp (L"%", ArgItem->VarStr, 1) == TRUE ||
             StrSubCmp (L"/", ArgItem->VarStr, 1) == TRUE ||
             StrSubCmp (L"|", ArgItem->VarStr, 1) == TRUE ||
             StrSubCmp (L"\"", ArgItem->VarStr, 1) == TRUE
            ) {
      //
      // we donot allow *,?,<,>,#,:,^,%,/,|," in alias or variable name
      //
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_ARG), HiiEnvHandle, CmdName, ArgItem->VarStr);
      Status = EFI_INVALID_PARAMETER;
      goto Quit;
    } else {
      Name = ArgItem->VarStr;
    }

    ArgItem = GetNextArg (ArgItem);
  }

  if (NULL != ArgItem) {
    Value = ArgItem->VarStr;
  }

  if (Delete && Value) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiEnvHandle, CmdName);
    Status = EFI_INVALID_PARAMETER;
    goto Quit;
  }

  if (Name != NULL) {
    for (Index = 0; mSpecVarTable[Index].CmdName != NULL; Index++) {
      if (StriCmp (mSpecVarTable[Index].CmdName, CmdName) == 0 &&
          StriCmp (mSpecVarTable[Index].VarName, Name) == 0 &&
          mSpecVarTable[Index].SpecCheckFun != NULL
          ) {
        Status = mSpecVarTable[Index].SpecCheckFun (Value, Delete, Volatile);
        if (EFI_ERROR (Status)) {
          goto Quit;
        }
      }
    }
  }
  //
  // Process
  //
  AcquireLock (&SEnvLock);

  if (!Name) {
    //
    // dump the list
    //
    SEnvSortVarList (Head);

    SLen = 0;
    for (Link = Head->Flink; Link != Head; Link = Link->Flink) {
      Var = CR (Link, VARIABLE_ID, Link, VARIABLE_SIGNATURE);
      if (_IsReserved (Var->Name, CmdName)) {
        continue;
      }

      Len = StrLen (Var->Name);
      if (Len > SLen) {
        SLen = Len;
      }
    }

    for (Link = Head->Flink; Link != Head; Link = Link->Flink) {
      //
      // Break the execution?
      //
      if (GetExecutionBreak ()) {
        goto Done;
      }

      Var = CR (Link, VARIABLE_ID, Link, VARIABLE_SIGNATURE);
      if (_IsReserved (Var->Name, CmdName)) {
        continue;
      }

      if (Var->Flags & VAR_ID_NON_VOL) {
        PrintToken (
          STRING_TOKEN (STR_SHELLENV_VAR_DUMP_VAR_LIST),
          HiiEnvHandle,
          L' ',
          SLen,
          Var->Name,
          Var->u.Str
          );
      } else {
        PrintToken (
          STRING_TOKEN (STR_SHELLENV_VAR_DUMP_VAR_LIST),
          HiiEnvHandle,
          L'*',
          SLen,
          Var->Name,
          Var->u.Str
          );
      }
    }
  } else {
    //
    // Find the specified value
    //
    Found = SEnvFindVar (Head, Name);

    if (Found && Delete) {
      //
      // Remove it from the store
      //
      Status = RT->SetVariable (Found->Name, Guid, 0, 0, NULL);
      if (Status == EFI_NOT_FOUND) {
        PrintToken (
          STRING_TOKEN (STR_SHELLENV_VAR_NOT_FOUND),
          HiiEnvHandle,
          CmdName,
          Found->Name
          );
      }
    } else if (Value) {
      //
      // Add it to the store
      //
      if (Found && ((Volatile && (Found->Flags & VAR_ID_NON_VOL)) || (!Volatile && !(Found->Flags & VAR_ID_NON_VOL)))) {
        if (Found->Flags & VAR_ID_NON_VOL) {
          PrintToken (
            STRING_TOKEN (STR_SHELLENV_VAR_ALREADY_EXISTS_NONVOLATILE),
            HiiEnvHandle,
            CmdName,
            Found->Name
            );
        } else {
          PrintToken (
            STRING_TOKEN (STR_SHELLENV_VAR_ALREADY_EXISTS_VOLATILE),
            HiiEnvHandle,
            CmdName,
            Found->Name
            );
        }

        Found   = NULL;
        Status  = EFI_ACCESS_DENIED;
      } else {
        Status = SEnvAddVar (
                  Head,
                  Guid,
                  Found ? Found->Name : Name,
                  (UINT8 *) Value,
                  StrSize (Value),
                  Volatile
                  );
      }
    } else {
      if (Found) {
        if (Found->Flags & VAR_ID_NON_VOL) {
          PrintToken (
            STRING_TOKEN (STR_SHELLENV_VAR_DISPLAY_VAR),
            HiiEnvHandle,
            L' ',
            Found->Name,
            Found->u.Str
            );
        } else {
          PrintToken (
            STRING_TOKEN (STR_SHELLENV_VAR_DISPLAY_VAR),
            HiiEnvHandle,
            L'*',
            Found->Name,
            Found->u.Str
            );
        }
      } else {
        PrintToken (
          STRING_TOKEN (STR_SHELLENV_VAR_NOT_FOUND),
          HiiEnvHandle,
          CmdName,
          Name
          );
      }

      Found = NULL;
    }
    //
    // Remove the old in memory copy if there was one
    //
    if (Found && !EFI_ERROR (Status)) {
      RemoveEntryList (&Found->Link);
      FreePool (Found);
    }
  }

Done:
  ReleaseLock (&SEnvLock);
Quit:
  LibCheckVarFreeVarList (&ChkPck);
  return Status;
}

EFI_STATUS
EFIAPI
SEnvCmdSet (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  )
/*++

Routine Description:

  Code for internal shell "set" command

Arguments:

  ImageHandle - The image handle
  SystemTable - The system table

Returns:

--*/
{
  //
  // Process 'set' command
  //
  return SEnvCmdSA (
          ImageHandle,
          SystemTable,
          &SEnvEnv,
          &SEnvEnvId,
          L"set",
          STRING_TOKEN (STR_SHELLENV_SET_VERBOSE_HELP)
          );
}

EFI_STATUS
EFIAPI
SEnvCmdAlias (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  )
/*++

Routine Description:

  Code for internal shell "alias" command

Arguments:

  ImageHandle - The image handle
  SystemTable - The system table

Returns:

--*/
{
  //
  // Process 'alias' command
  //
  return SEnvCmdSA (
          ImageHandle,
          SystemTable,
          &SEnvAlias,
          &SEnvAliasId,
          L"alias",
          STRING_TOKEN (STR_SHELLENV_ALIAS_VERBOSE_HELP)
          );
}

EFI_STATUS
SEnvGetShellMode (
  OUT CHAR16       **Mode
  )
{
  if (Mode == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  *Mode = SEnvGetEnv (L"efishellmode");
  return EFI_SUCCESS;
}

EFI_STATUS
SpecShellModeCheckFun (
  IN CHAR16                                *Value,
  IN BOOLEAN                               Delete,
  IN BOOLEAN                               Volatile
  )
{
  UINTN Index;
  if (Delete || !Volatile) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_VAR_RESERVED), HiiEnvHandle, L"set", L"efishellmode");
    return EFI_INVALID_PARAMETER;
  }

  for (Index = 0; mShellMode[Index] != NULL; Index++) {
    if (StrCmp (mShellMode[Index], Value) == 0) {
      return EFI_SUCCESS;
    }
  }

  PrintToken (STRING_TOKEN (STR_SHELLENV_VAR_MODE_OPTIONS), HiiEnvHandle);
  for (Index = 0; mShellMode[Index] != NULL; Index++) {
    Print (mShellMode[Index]);
    Print (L"\n");
  }

  return EFI_INVALID_PARAMETER;
}

EFI_STATUS
SpecShellModeGetFun (
  OUT CHAR16                               **Value
  )
{
  UINTN Index;
  *Value = SEnvIGetStr (L"efishellmode", &SEnvEnv);
  if (*Value != NULL) {
    for (Index = 0; mShellMode[Index] != NULL; Index++) {
      if (StrCmp (mShellMode[Index], *Value) == 0) {
        return EFI_SUCCESS;
      }
    }
  }

  *Value = mShellMode[0];
  return EFI_SUCCESS;
}
