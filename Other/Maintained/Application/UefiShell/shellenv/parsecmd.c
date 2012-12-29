/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  parsecmd.c 
  
Abstract:

Revision History

--*/

#include "parsecmd.h"

SHELL_SINGLE_ARG *
GetNewArg (
  VOID
  )
/*++
Routine Description:

  Allocate a new SHELL_SINGLE_ARG structure instance.

Arguments:

  None
  
Returns:

--*/
{
  SHELL_SINGLE_ARG  *p;

  p = (SHELL_SINGLE_ARG *) AllocatePool (sizeof (SHELL_SINGLE_ARG));
  if (NULL != p) {
    p->Parts      = NULL;
    p->Result     = NULL;
    p->Attributes = ARG_NO_ATTRIB;
    p->Next       = NULL;
  }

  return p;
}

SHELL_ARG_PART *
GetNewArgPart (
  VOID
  )
/*++
Routine Description:

  Allocate a new SHELL_ARG_PART structure instance
  
Arguments:

  None

Returns:


--*/
{
  SHELL_ARG_PART  *p;

  p = (SHELL_ARG_PART *) AllocatePool (sizeof (SHELL_ARG_PART));
  if (NULL != p) {
    p->Argument = (CHAR16 *) AllocatePool (BUFFER_GROW_UNIT);
    if (NULL == p->Argument) {
      FreePool (p);
      p = NULL;
    } else {
      p->Argument[0]  = 0;
      p->ArgSize      = BUFFER_GROW_UNIT;
      p->Attributes   = ARG_NO_ATTRIB;
      p->Next         = NULL;
    }
  }

  return p;
}

BOOLEAN
InsertChar2ArgPart (
  IN OUT CHAR16             ch,
  IN OUT SHELL_ARG_PART     *part
  )
/*++
Routine Description:

  Insert one character into the buffer of argument 'part'.
  if the buffer is going to overflow, then expand it.
  
Arguments:
  
  ch     - The charactor
  part   - The part
  
Returns:

--*/
{
  CHAR16  *p;
  UINTN   StringSize;

  p = NULL;
  //
  // Get the actual size of the content, StringSize includes NULL
  //
  StringSize = StrSize (part->Argument);
  if (StringSize >= part->ArgSize) {
    p = (CHAR16 *) AllocatePool (part->ArgSize + BUFFER_GROW_UNIT);
    if (NULL == p) {
      return FALSE;
    }

    StrCpy (p, part->Argument);
    FreePool (part->Argument);
    part->Argument = p;
    part->ArgSize += BUFFER_GROW_UNIT;
  }

  StringSize >>= 1;
  StringSize--;
  part->Argument[StringSize]      = ch;
  part->Argument[StringSize + 1]  = 0;

  return TRUE;
}

CHAR16 *
TrimLeftRight (
  IN     CHAR16         *str
  )
/*++
Routine Description:

  Delete spaces at the beginning and the end of input string 'str'

Arguments:

  str   - the string
  
Returns:

--*/
{
  CHAR16  *Tail;
  CHAR16  *NewStr;
  UINTN   NewStrSize;

  //
  // Look for the first non-space character in the string
  //
  while (*str && ' ' == *str) {
    str++;
  }

  if (0 == *str) {
    //
    // The string is equal to ""
    //
    Tail = str;
  } else {
    Tail = str + StrLen (str);
  }
  //
  // Search the first no-space character from the end of the string
  //
  while (' ' == *(Tail - 1) && str < Tail - 1) {
    Tail--;
  }
  //
  // Calculate the size of the string trimed, the size includes NULL
  //
  NewStrSize  = (UINTN) Tail - (UINTN) str + sizeof (CHAR16);
  NewStr      = (CHAR16 *) AllocatePool (NewStrSize);
  if (NULL != NewStr) {
    CopyMem (NewStr, str, NewStrSize);
    NewStr[(NewStrSize >> 1) - 1] = 0;
  }

  return NewStr;
}

BOOLEAN
IsLiteralMark (
  IN     CHAR16         *Start,
  IN     CHAR16         *Current,
  IN     CHAR16         EscapeChar
  )
/*++
Routine Description:
  To tell if the character pointed by 'Current' is led by escaping char
  'Start' is the very beginning of the string containing 'Current'
  
Arguments:
 
  Start      - The start string
  Current    - Current postion
  EscapeChar - The escape char
  
Return:

--*/
{
  CHAR16  *p;

  p = Current - 1;
  if (p < Start || EscapeChar != *p) {
    return FALSE;
  }

  while (p > Start && EscapeChar == *p) {
    p--;
  }
  //
  // If the number of continous escaping char before *Current is odd, it means *Current has been escaped
  //
  if (0 == (((UINTN) Current - (UINTN) p) / sizeof (UINT16)) % 2) {
    return TRUE;
  }

  return FALSE;
}

VOID
FreeParts (
  IN     SHELL_ARG_PART     *part
  )
/*++
Routine Description:

  Free all the memory occupied by SHELL_ARG_PART argument

Arguments:

  Part   - A pointer to SHELL_ARG_PART
  
Returns:

--*/
{
  SHELL_ARG_PART  *p;

  ASSERT (NULL != part);

  while (part) {
    if (part->Argument) {
      FreePool (part->Argument);
    }

    p     = part;
    part  = part->Next;
    FreePool (p);
  }
}

VOID
FreeParsedArg (
  IN     SHELL_SINGLE_ARG   *arg
  )
/*++
Routine Description:

  Free all the memory occupied by SHELL_SINGLE_ARG

Arguments:

  arg       - A pointer to the SHELL_SINGLE_ARG
  
Returns:

--*/
{
  ASSERT (arg);

  if (arg->Parts) {
    FreeParts (arg->Parts);
  }

  if (arg->Result) {
    FreePool (arg->Result);
  }

  FreePool (arg);
}

VOID
FreeParsedArgs (
  IN     SHELL_PARSED_ARGS    *args
  )
/*++
Routine Description:

  Free all memory occupied by all the elements described by args,
  but args itself.

Arguments:

  args - A pointer to the SHELL_PARSED_ARGS
  
Returns:

--*/
{
  SHELL_SINGLE_ARG  *arg;

  ASSERT (args);
  if (args->ArgCount) {
    ASSERT (args->ArgList);
    while (args->ArgList) {
      arg           = args->ArgList;
      args->ArgList = args->ArgList->Next;
      FreeParsedArg (arg);
    }

    args->ArgCount = 0;
  }
}

EFI_STATUS
InitializeArgs (
  IN OUT SHELL_PARSED_ARGS      *args,
  OUT SHELL_SINGLE_ARG_PTR      **CurrentArg,
  OUT SHELL_ARG_PART_PTR        **CurrentPart
  )
{
  SHELL_SINGLE_ARG_PTR  *cur_arg;
  SHELL_ARG_PART_PTR    *cur_part;

  args->ArgCount  = 0;
  args->ArgList   = GetNewArg ();
  if (NULL == args->ArgList) {
    return EFI_OUT_OF_RESOURCES;
  }

  args->ArgCount    = 1;

  cur_arg           = &args->ArgList;
  (*cur_arg)->Parts = GetNewArgPart ();
  if (NULL == (*cur_arg)->Parts) {
    return EFI_OUT_OF_RESOURCES;
  }

  cur_part      = &(*cur_arg)->Parts;

  *CurrentArg   = cur_arg;
  *CurrentPart  = cur_part;

  return EFI_SUCCESS;
}

EFI_STATUS
OnMetPossibleQuotationStart (
  IN     CHAR16               *Start,
  IN OUT BOOLEAN              *Quoted,
  IN OUT SHELL_ARG_PART_PTR   **CurrentPart
  )
/*++
Routine Description:

  When we meet a quotation mark, we need to find out if this is a valid quotation mark.
  If it is, we need to find out whether we can find a valid pair of quotation marks.
  
Arguments:

  Start       - The start possion
  Quoted      - Is Quoted
  CurrentPart - The current argument part

Returns:
  EFI_OUT_OF_RESOURCES  - Out of resources
  EFI_SUCCESS           - Success

--*/
{
  CHAR16              *sp;
  SHELL_ARG_PART_PTR  *cur_part;

  sp = Start + 1;
  while (*sp) {
    if ('"' == *sp && !IsLiteralMark (Start, sp, ESCAPING_CHAR)) {
      //
      // We found a valid quotation mark!
      //
      break;
    }

    sp++;
  }

  if (*sp) {
    *Quoted = TRUE;
    //
    // The beginning of a quotation also means the beginning of a part
    //
    cur_part  = &(**CurrentPart)->Next;
    *cur_part = GetNewArgPart ();
    if (NULL == *cur_part) {
      return EFI_OUT_OF_RESOURCES;
    }

    *CurrentPart = cur_part;
  } else {
    *Quoted = FALSE;
    //
    // This can keep the current " staying in the command line:
    // (e.g. "abc)
    //
  }

  return EFI_SUCCESS;
}

EFI_STATUS
OnEndOfQuotation (
  IN     CHAR16                 *Current,
  IN OUT SHELL_ARG_PART_PTR     **CurrentPart
  )
/*++
Routine Description:

  When we meet the end of a quotation, it means an end of a part.
  But if this is an end of an argument we don't have to allocate a new SHELL_ARG_PART structure.

Arguments:

  Current     - Current string
  CurrentPart - The current argument part.
  
Returns:

  EFI_OUT_OF_RESOURCES - Out of resources
  EFI_SUCCESS          - Success

--*/
{
  SHELL_ARG_PART_PTR  *cur_part;

  if (*(Current + 1) && ' ' != *(Current + 1)) {
    //
    // If the next character is not a NULL or a space, we know that the current argument has not finished yet
    // So we should start a new part of the current argument
    //
    cur_part  = &(**CurrentPart)->Next;
    *cur_part = GetNewArgPart ();
    if (NULL == *cur_part) {
      return EFI_OUT_OF_RESOURCES;
    }

    *CurrentPart = cur_part;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
OnMetValidSpace (
  IN OUT CHAR16                 **Current,
  IN OUT SHELL_PARSED_ARGS      *args,
  IN OUT SHELL_SINGLE_ARG_PTR   **CurrentArg,
  IN OUT SHELL_ARG_PART_PTR     **CurrentPart
  )
/*++
Routine Description:

  When we meet a valid space character, it means we are probably going to process next argument,
  if there really is a next argument we need to allocate new SHELL_SINGLE_ARG structure for it.
  
Arguments:

  Current     - Current string
  args        - The shell pared arguments
  CurrentArg  - Current argument
  CurrentPart - Current argument part
  
Returns:

  EFI_OUT_OF_RESOURCES - Out of resources
  EFI_SUCCESS - Success

--*/
{
  CHAR16                *p;
  SHELL_SINGLE_ARG_PTR  *cur_arg;
  SHELL_ARG_PART_PTR    *cur_part;

  p = *Current;
  //
  // If we are not in a pair of ", it means that
  // we are about to enter into the next possible argument segment.
  // There might be multiple continous spaces, so skip them if there are
  //
  while (*p && ' ' == *(p + 1)) {
    p++;
  }

  *Current = p;

  if (*(p + 1) && '#' != *(p + 1)) {
    //
    // In fact, we do have the next argument.
    // Allocate a new SHELL_SINGLE_ARG structure for this argument
    // and let CurrentArg point at this new structure.
    //
    cur_arg   = &(**CurrentArg)->Next;
    *cur_arg  = GetNewArg ();
    if (NULL == *cur_arg) {
      return EFI_OUT_OF_RESOURCES;
    }

    args->ArgCount++;
    //
    // CurrentPart now should point at the first part of the new argument
    //
    cur_part  = &(*cur_arg)->Parts;
    *cur_part = GetNewArgPart ();
    if (NULL == *cur_part) {
      return EFI_OUT_OF_RESOURCES;
    }

    *CurrentArg   = cur_arg;
    *CurrentPart  = cur_part;

  } else if ('#' == *(p + 1)) {
    //
    // No arguments any more, anything after # should be considered as comments
    //
    *(p + 1) = 0;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
ProcessQuotationMarks (
  IN     CHAR16                 *CmdLine,
  IN OUT SHELL_PARSED_ARGS      *args
  )
/*++
Routine Description:
  Process all the quotation marks which are not escaped by '^'
  And seperate arguments into seperate SHELL_SINGLE_ARG structure instances

  Each argument can have one or more parts, a part can be defined by a pair of quotation marks, for example:
  abc"%1"xyz has 3 parts, they are respectively abc, %1 and xyz
  abc has just one part - abc
  "abc" has 2 parts, they are respectively NULL and abc
  "hello""world"! has 5 parts, they are respectively NULL, hello, NULL, world and !
  
Arguments:
  
  CmdLine     - The command line
  args        - The shell parsed arguments.

Returns:
  EFI_INVALID_PARAMETER   - Invalid parameter
  
--*/
{
  CHAR16                *p;
  CHAR16                *StrStart;
  BOOLEAN               Quoted;
  SHELL_SINGLE_ARG_PTR  *CurrentArg;
  SHELL_ARG_PART_PTR    *CurrentPart;
  EFI_STATUS            Status;

  if (NULL == CmdLine || NULL == args) {
    return EFI_INVALID_PARAMETER;
  }

  Quoted          = FALSE;
  CurrentArg      = NULL;
  CurrentPart     = NULL;
  args->ArgCount  = 0;
  args->ArgList   = NULL;
  Status          = EFI_SUCCESS;
  //
  // Trim off heading and tailing space characters
  //
  StrStart = TrimLeftRight (CmdLine);
  if (NULL == StrStart) {
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  Status = InitializeArgs (args, &CurrentArg, &CurrentPart);
  if (EFI_ERROR (Status)) {
    goto Done;
  }

  p = StrStart;
  while (*p) {
    switch (*p) {
    case '#':
      if (SEnvBatchIsActive ()) {
        while (*p) {
          p++;
        }

        p--;
      } else {
        InsertChar2ArgPart (*p, *CurrentPart);
      }
      break;

    case '"':
      if (!Quoted) {
        //
        // If we are not in a pair of quotation marks currently,
        // search for next valid quotation mark that can constitue
        // a pair of quotation marks with current one.
        //
        Status = OnMetPossibleQuotationStart (p, &Quoted, &CurrentPart);
        if (EFI_ERROR (Status)) {
          goto Done;
        }

        if (Quoted) {
          //
          // If this part was in quotation, then mark its attributes with
          // ARG_IS_QUOTED.
          //
          (*CurrentPart)->Attributes |= ARG_IS_QUOTED;
        }

      } else {
        //
        // This means the end of a quotation, meanwhile it means the end of the current part
        //
        Quoted  = FALSE;
        Status  = OnEndOfQuotation (p, &CurrentPart);
        if (EFI_ERROR (Status)) {
          goto Done;
        }
      }
      break;

    case ' ':
      if (Quoted) {
        //
        // If we are in a pair of '"',
        // put this space directly into the current arg structure
        //
        InsertChar2ArgPart (*p, *CurrentPart);
      } else {
        //
        // This space is not inside a pair of quotation marks
        // we should take care of it
        //
        Status = OnMetValidSpace (&p, args, &CurrentArg, &CurrentPart);
        if (EFI_ERROR (Status)) {
          goto Done;
        }
      }
      break;

    case '^':
      //
      // Put the current '^' at the tail of the buffer of CurrentPart
      //
      InsertChar2ArgPart (*p, *CurrentPart);
      if (*(p + 1)) {
        //
        // If the next character after current '^' is not NULL,
        // put it into buffer too.
        //
        InsertChar2ArgPart (*(p + 1), *CurrentPart);
        p++;
      }
      break;

    default:
      //
      // For regular character, just put it into buffer
      //
      InsertChar2ArgPart (*p, *CurrentPart);
      break;
    }

    p++;
    //
    // here is the end of the big while
    //
  }

Done:
  if (StrStart) {
    FreePool (StrStart);
  }

  if (EFI_ERROR (Status)) {
    FreeParsedArgs (args);
  }

  return Status;
}

EFI_STATUS
LookupEnvVar (
  IN     CHAR16         *VarName,
  IN OUT CHAR16         **Content
  )
/*++
Routine Description:

  Look up environment variable named VarName

Arguments:

  VarName   - Variable name
  Content   - Content
  
Returns:

--*/
{
  EFI_STATUS  Status;

  ASSERT (NULL != Content);
  ASSERT (NULL != VarName);
  Status    = EFI_SUCCESS;
  *Content  = SEnvGetEnv (VarName);
  if (NULL == *Content) {
    Status = EFI_NOT_FOUND;
  }

  return Status;
}

EFI_STATUS
LookupPosVar (
  IN     CHAR16                 VarName,
  OUT CHAR16                    **Content,
  OUT EFI_SHELL_ARG_INFO        *ArgInfo
  )
/*++
Routine Description:

  Look up positional variable named VarName
  
Arguments:

  VarName    - Varable name
  Content    - The Content
  ArgInfo    - The arguments infomation
  
Returns:

--*/
{
  EFI_SHELL_ARG_INFO  *Info;
  EFI_STATUS          Status;

  ASSERT (NULL != Content);
  ASSERT (NULL != ArgInfo);

  Status = SEnvBatchFindArgument (VarName - '0', Content, &Info);
  if (!EFI_ERROR (Status)) {
    CopyMem (ArgInfo, Info, sizeof (EFI_SHELL_ARG_INFO));
  }
  return Status;
}

EFI_STATUS
LookupIndexVar (
  IN     CHAR16         *VarName,
  IN OUT CHAR16         **Content
  )
/*++
Routine Description:

  Look up index variable named VarName

Arguments:

  VarName    - Varable name
  Content    - The Content
  
Returns:

--*/
{
  CHAR16 Var[3];

  ASSERT (NULL != Content);
  ASSERT (NULL != VarName);

  Var[0]  = '%';
  Var[1]  = VarName[0];
  Var[2]  = 0;
  return SEnvBatchFindVariable (Var, Content);
}

EFI_STATUS
VarSubstitute (
  IN     CHAR16                 *VarName,
  IN     VAR_SUBSTITUTION_TYPE  Type,
  IN     BOOLEAN                IsInBatch,
  IN OUT SHELL_ARG_PART         *part
  )
/*++
Routine Description:
  Substitute the variable named VarName, the type of the variable will be
  decided by Type.

Arguments:

  VarName    - Varable name
  Type       - The substitution type
  IsInBatch  - Is in a batch file
  part       - The shell arguments part
  
Returns:

--*/
{
  EFI_STATUS          Status;
  CHAR16              *VarContent;
  EFI_SHELL_ARG_INFO  ArgInfo;

  VarContent = NULL;
  ZeroMem (&ArgInfo, sizeof (EFI_SHELL_ARG_INFO));

  switch (Type) {
  case VAR_TYPE_ENVIRONMENT:
    if (IsInBatch && SEnvBatchVarIsLastError (VarName)) {
      VarContent  = SEnvBatchGetLastError ();
      Status      = EFI_SUCCESS;
      break;
    }

    Status = LookupEnvVar (VarName, &VarContent);
    break;

  case VAR_TYPE_POSITION:
    Status = LookupPosVar (VarName[0], &VarContent, &ArgInfo);
    if (!EFI_ERROR (Status) && !StrLen (part->Argument)) {
      part->Attributes |= ArgInfo.Attributes;
    }
    break;

  case VAR_TYPE_INDEX:
    Status = LookupIndexVar (VarName, &VarContent);
    break;

  default:
    Status = EFI_INVALID_PARAMETER;
  }

  if (!EFI_ERROR (Status)) {
    while (*VarContent) {
      InsertChar2ArgPart (*VarContent, part);
      VarContent++;
    }
  }

  return Status;
}

BOOLEAN
IsValidEnvVarNameChar (
  IN     CHAR16         VarName
  )
/*++
Routine Description:

  Judge if the string pointed by VarName is a valid environment variable name

Arguments:

  VarName  - The varable name
  
Returns:

--*/
{
  if ('_' == VarName ||
      ('0' <= VarName && '9' >= VarName) ||
      ('A' <= VarName && 'Z' >= VarName) ||
      ('a' <= VarName && 'z' >= VarName)
      ) {
    return TRUE;
  }

  return FALSE;
}

EFI_STATUS
OnMetPersentageSign (
  IN OUT CHAR16                 **Current,
  IN     BOOLEAN                IsInBatch,
  IN     BOOLEAN                AtBeginning,
  OUT SHELL_ARG_PART_PTR        tp
  )
{
  CHAR16      *tmp;
  CHAR16      VarName[256];
  UINTN       Index;
  BOOLEAN     ValidChar;
  EFI_STATUS  Status;

  ASSERT (Current);
  ASSERT (*Current);

  Index       = 0;
  VarName[0]  = 0;
  ValidChar   = TRUE;
  tmp         = *Current + 1;
  while (*tmp) {
    if ('%' == *tmp) {
      break;
    } else if ('^' == *tmp) {
      tmp++;
    }

    if (!IsValidEnvVarNameChar (*tmp)) {
      ValidChar = FALSE;
      break;
    }

    VarName[Index++] = *tmp;
    tmp++;
  }

  Status = EFI_NOT_FOUND;
  if (*tmp && ValidChar) {
    //
    // We got here, it means that a pattern of %var% was found.
    //
    VarName[Index] = 0;
    Status = VarSubstitute (
              VarName,
              VAR_TYPE_ENVIRONMENT,
              SEnvBatchIsActive (),
              tp
              );
  }

  if (EFI_NOT_FOUND == Status) {
    if (IsInBatch) {
      if (IsDigit (VarName[0])) {
        Status = VarSubstitute (
                  VarName,
                  VAR_TYPE_POSITION,
                  SEnvBatchIsActive (),
                  tp
                  );
      }
  
      if (IsAlpha (VarName[0])) {
        Status = VarSubstitute (
                  VarName,
                  VAR_TYPE_INDEX,
                  SEnvBatchIsActive (),
                  tp
                  );
      }
  
      if (!EFI_ERROR (Status)) {
        //
        // We got here, it means either substitution of %n or %x succeeded,
        // step over it and continue.
        //
        *Current += 2;
      } else if (EFI_NOT_FOUND == Status) {
        Status = EFI_SUCCESS;
        if (IsDigit (VarName[0])) {
          *Current += 2;
        } else if (0 == *tmp || !ValidChar) {
          (*Current)++;
        } else {
          *Current = tmp + 1;
        }
      }
    } else {
      if (0 == *tmp || !ValidChar)
      {
        tmp = *Current + 1; 
      }
      while((*Current) < tmp)
      {
        InsertChar2ArgPart(**Current, tp);
        (*Current) ++;
      }
      Status = EFI_SUCCESS;
    }
  } else {
    *Current = tmp + 1;
  }

  return Status;
}

EFI_STATUS
ProcessVarSubstitution (
  IN OUT SHELL_ARG_PART     *part
  )
/*++
Routine Description:

  Perform variable substitution on the content held by arg

Arguments:

  part - The shell arguments part
  
Returns:

  EFI_OUT_OF_RESOURCES - Out of resources

--*/
{
  CHAR16          *p;
  CHAR16          *VarContent;
  SHELL_ARG_PART  *tp;
  EFI_STATUS      Status;

  ASSERT (NULL != part);
  Status  = EFI_SUCCESS;
  tp      = GetNewArgPart ();
  if (NULL == tp) {
    return EFI_OUT_OF_RESOURCES;
  }

  if (part->Argument[0] == ESCAPING_CHAR) {
    //
    // If the first char of this part is a escaping char,
    // then mark its attributes with ARG_FIRST_CHAR_IS_ESC.
    //
    part->Attributes |= ARG_FIRST_CHAR_IS_ESC;
  }

  p = part->Argument;
  while (*p && !EFI_ERROR (Status)) {
    switch (*p) {
    case ESCAPING_CHAR:
      p++;
      if (*p) {
        InsertChar2ArgPart (*p, tp);
        p++;
      }
      break;

    case '%':
      Status = OnMetPersentageSign (
                &p,
                SEnvBatchIsActive (),
                (BOOLEAN)!StrLen (tp->Argument),
                tp
                );
      break;

    default:
      InsertChar2ArgPart (*p, tp);
      p++;
    }
    //
    // This is the end of while statement
    //
  }

  if (!EFI_ERROR (Status)) {
    part->Argument[0] = 0;
    part->Attributes |= tp->Attributes;
    VarContent = tp->Argument;
    while (*VarContent) {
      InsertChar2ArgPart (*VarContent, part);
      VarContent++;
    }
  }

  FreeParts (tp);

  return Status;
}

EFI_STATUS
MergeResults (
  IN OUT SHELL_SINGLE_ARG   *arg
  )
/*++
Routine Description:

  Merge the strings in different parts into a entire string
  
Arguments:

  arg   - The single argument
  
Returns:
  EFI_OUT_OF_RESOURCES - Out of resources
  EFI_SUCCESS - Success

--*/
{
  UINTN           StringLen;
  SHELL_ARG_PART  *part;
  BOOLEAN         FirstValidPart;
  BOOLEAN         HasPartNotQuoted;

  ASSERT (NULL != arg);
  ASSERT (NULL == arg->Result);

  part              = arg->Parts;
  StringLen         = 0;
  HasPartNotQuoted  = FALSE;
  while (part) {
    StringLen += StrLen (part->Argument);
    part = part->Next;
  }

  arg->Result = (CHAR16 *) AllocatePool ((StringLen + 1) << 1);
  if (NULL == arg->Result) {
    return EFI_OUT_OF_RESOURCES;
  }

  arg->Result[0]  = 0;

  part            = arg->Parts;
  FirstValidPart  = TRUE;
  while (part) {
    if (part->Argument[0]) {
      if (FirstValidPart) {
        //
        // If this part is the first non-NULL part
        // To see if the first char in this part is a escaping char
        // If it is then set ARG_FIRST_CHAR_IS_ESC flag in this argument's
        // attributes.
        //
        arg->Attributes |= (part->Attributes & ARG_FIRST_CHAR_IS_ESC);
        if (part->Attributes & ARG_IS_QUOTED) {
          //
          // If the attributes of the first non-NULL part has flag
          // ARG_IS_QUOTED, then set ARG_FIRST_HALF_QUOTED flag in
          // this argument's attributes
          //
          // ARG_FIRST_HALF_QUOTED means arguments like this:
          // "abc"123, "-fl"ag, etc.
          //
          arg->Attributes |= ARG_FIRST_HALF_QUOTED;
        }
      }

      StrCat (arg->Result, part->Argument);
      //
      // If any of the parts has attribute of ARG_IS_QUOTED, then this
      // argument should have such attribute
      //
      arg->Attributes |= (part->Attributes & ARG_IS_QUOTED);
      if (!(part->Attributes & ARG_IS_QUOTED)) {
        HasPartNotQuoted = TRUE;
      }

      if (arg->Attributes & ARG_IS_QUOTED) {
        if (HasPartNotQuoted) {
          //
          // If there are at least one part quoted and at least one
          // part not quoted, then this argument should have the
          // attribute of ARG_PARTIALLY_QUOTED
          //
          // ARG_PARTIALLY_QUOTED means arguments like this:
          // "abc"123, ab"12"xyz, "1"abc"0", etc.
          //
          // "hello" has attribute of ARG_IS_QUOTED but doesn't have
          // attribute of ARG_PARTIALLY_QUOTED.
          //
          arg->Attributes |= ARG_PARTIALLY_QUOTED;
        }
      }

      FirstValidPart = FALSE;
    }

    part = part->Next;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
ShellVarSubstitution (
  IN OUT SHELL_PARSED_ARGS    *args
  )
{
  EFI_STATUS        Status;
  SHELL_SINGLE_ARG  *arg;
  SHELL_ARG_PART    *part;

  ASSERT (args);
  ASSERT (args->ArgList);

  arg     = args->ArgList;
  Status  = EFI_SUCCESS;
  while (!EFI_ERROR (Status) && arg) {
    part = arg->Parts;
    while (!EFI_ERROR (Status) && part) {
      Status  = ProcessVarSubstitution (part);
      part    = part->Next;
    }

    if (!EFI_ERROR (Status)) {
      Status = MergeResults (arg);
    }

    arg = arg->Next;
  }

  return Status;
}

VOID
MoveArgsToHeadOfAnother (
  IN     SHELL_PARSED_ARGS      *src_args,
  OUT SHELL_PARSED_ARGS         *dst_args
  )
{
  SHELL_SINGLE_ARG  *tmp;

  ASSERT (src_args);
  ASSERT (dst_args);

  if (0 == src_args->ArgCount) {
    return ;
  }

  tmp = src_args->ArgList;
  while (tmp->Next) {
    tmp = tmp->Next;
  }

  ASSERT (NULL == tmp->Next);
  tmp->Next         = dst_args->ArgList;
  dst_args->ArgList = src_args->ArgList;
  dst_args->ArgCount += src_args->ArgCount;
  src_args->ArgList   = NULL;
  src_args->ArgCount  = 0;

  return ;
}

VOID
DeleteHeadArg (
  IN OUT SHELL_PARSED_ARGS    *args
  )
{
  SHELL_SINGLE_ARG  *tmp;

  ASSERT (args);

  if (0 == args->ArgCount) {
    return ;
  }

  tmp           = args->ArgList;
  args->ArgList = tmp->Next;
  FreeParsedArg (tmp);
  args->ArgCount--;
}
