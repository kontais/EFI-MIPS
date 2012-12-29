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

#ifndef _EFI_SHELL_PARSE_CMD_H_
#define _EFI_SHELL_PARSE_CMD_H_

#include "shelle.h"

#define BUFFER_GROW_UNIT  20
#define ESCAPING_CHAR     '^'

extern
EFI_STATUS
SEnvBatchFindArgument (
  IN  UINTN                          ArgumentNum,
  OUT CHAR16                         **Value,
  OUT EFI_SHELL_ARG_INFO             **ArgInfo
  );

extern
EFI_STATUS
SEnvBatchFindVariable (
  IN  CHAR16                         *VariableName,
  OUT CHAR16                         **Value
  );

extern
CHAR16  *
EFIAPI
SEnvGetEnv (
  IN CHAR16          *Name
  );

typedef enum {
  VAR_TYPE_ENVIRONMENT,
  VAR_TYPE_POSITION,
  VAR_TYPE_INDEX
} VAR_SUBSTITUTION_TYPE;

typedef struct _SHELL_ARG_PART {
  CHAR16                  *Argument;
  UINTN                   ArgSize;
  UINT32                  Attributes;
  struct _SHELL_ARG_PART  *Next;
}
SHELL_ARG_PART, *SHELL_ARG_PART_PTR;

typedef struct _SHELL_SINGLE_ARG {
  SHELL_ARG_PART            *Parts;
  CHAR16                    *Result;
  UINT32                    Attributes;
  struct _SHELL_SINGLE_ARG  *Next;
}
SHELL_SINGLE_ARG, *SHELL_SINGLE_ARG_PTR;

typedef struct _SHELL_PARSED_ARGS {
  UINTN             ArgCount;
  SHELL_SINGLE_ARG  *ArgList;
} SHELL_PARSED_ARGS;

VOID
FreeParsedArgs (
  IN     SHELL_PARSED_ARGS    *args
  );

EFI_STATUS
ProcessQuotationMarks (
  IN     CHAR16                   *CmdLine,
  IN OUT SHELL_PARSED_ARGS        *args
  );

EFI_STATUS
ShellVarSubstitution (
  IN OUT SHELL_PARSED_ARGS    *args
  );

VOID
MoveArgsToHeadOfAnother (
  IN     SHELL_PARSED_ARGS      *src_args,
  OUT SHELL_PARSED_ARGS         *dst_args
  );

VOID
DeleteHeadArg (
  IN OUT SHELL_PARSED_ARGS    *args
  );

#endif