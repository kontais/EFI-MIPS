/*++

Copyright (c) 2005 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

    smbiosview.c
    
Abstract:

    Tools of clarify the content of the smbios table.

Revision History

--*/

#include "EfiShellLib.h"
#include "LibSmbiosView.h"
#include "smbiosview.h"
#include "PrintInfo.h"
#include "QueryTable.h"
#include "smbios.h"

extern UINT8                STRING_ARRAY_NAME[];

//
// This is the generated header file which includes whatever needs to be exported (strings + IFR)
//
#include STRING_DEFINES_FILE

UINT8                       gShowType         = SHOW_DETAIL;
STATIC STRUCTURE_STATISTICS *mStatisticsTable = NULL;
STATIC EFI_HANDLE           *mMyImageHandle   = NULL;

//
// Global Variables
//
EFI_HII_HANDLE              HiiHandle;
EFI_GUID                    EfiSmbiosViewGuid = EFI_SMBIOS_GUID;
SHELL_VAR_CHECK_ITEM        SmbiosviewCheckList[] = {
  {
    L"-t",
    0x01,
    0x0e,
    FlagTypeNeedVar
  },
  {
    L"-h",
    0x02,
    0x0d,
    FlagTypeNeedVar
  },
  {
    L"-s",
    0x04,
    0x0b,
    FlagTypeSingle
  },
  {
    L"-a",
    0x08,
    0x07,
    FlagTypeSingle
  },
  {
    L"-b",
    0x10,
    0,
    FlagTypeSingle
  },
  {
    L"-?",
    0x20,
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
// Entry Point
//
EFI_STATUS
InitializeSmbiosViewApplication (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_BOOTSHELL_CODE(
  EFI_APPLICATION_ENTRY_POINT(InitializeSmbiosViewApplication)
)

EFI_STATUS
InitializeSmbiosViewApplication (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
/*++

Routine Description:
  This function Initializes EFI SMBIOS interfaces.

Arguments:
 ImageHandle   - Initialization parameter for Lib C
 SystemTable   - Standard EFI SystemTable for this application
  
Returns:
  EFI_SUCCESS                  - Successfully initialize SMBIOS interface 
  EFI_SMBIOSERR_FAILURE        - Failed to initialize SMBIOS interface 
  EFI_SMBIOSERR_UNSUPPORTED    - System does not support SMBIOS feature

--*/
{
  UINT8                   StructType;
  UINT16                  StructHandle;
  EFI_STATUS              Status;
  BOOLEAN                 RandomView;

  SHELL_VAR_CHECK_CODE    RetCode;
  CHAR16                  *Useful;
  SHELL_ARG_LIST          *Item;
  SHELL_VAR_CHECK_PACKAGE ChkPck;

  mStatisticsTable                = NULL;
  mMyImageHandle                  = &ImageHandle;
  ZeroMem (&ChkPck, sizeof (SHELL_VAR_CHECK_PACKAGE));

  EFI_SHELL_APP_INIT (ImageHandle, SystemTable);
  
  //
  // Enable tab key which can pause the output
  //
  EnableOutputTabPause();

  Status = LibInitializeStrings (&HiiHandle, STRING_ARRAY_NAME, &EfiSmbiosViewGuid);

  if (EFI_ERROR (Status)) {
    return Status;
  }

  RetCode = LibCheckVariables (SI, SmbiosviewCheckList, &ChkPck, &Useful);
  if (VarCheckOk != RetCode) {
    switch (RetCode) {
    case VarCheckLackValue:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_LACK_ARG), HiiHandle, L"smbiosview", Useful);
      break;

    case VarCheckConflict:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_FLAG_CONFLICT), HiiHandle, L"smbiosview", Useful);
      break;

    case VarCheckDuplicate:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_DUP_FLAG), HiiHandle, L"smbiosview", Useful);
      break;

    case VarCheckUnknown:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_UNKNOWN_FLAG), HiiHandle, L"smbiosview", Useful);
      break;

    default:
      break;
    }

    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }
  //
  // Out put help.
  //
  if (LibCheckVarGetFlag (&ChkPck, L"-b") != NULL) {
    EnablePageBreak (DEFAULT_INIT_ROW, DEFAULT_AUTO_LF);
  }

  if (LibCheckVarGetFlag (&ChkPck, L"-?") != NULL) {
    if (ChkPck.ValueCount > 0 ||
        ChkPck.FlagCount > 2 ||
        (2 == ChkPck.FlagCount && !LibCheckVarGetFlag (&ChkPck, L"-b"))
        ) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiHandle, L"smbiosview");
      Status = EFI_INVALID_PARAMETER;
    } else {
      PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_VERBOSE_HELP), HiiHandle);
      Status = EFI_SUCCESS;
    }

    goto Done;
  }

  if (ChkPck.ValueCount > 0) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiHandle, L"smbiosview");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }
  //
  // Init Lib
  //
  Status = LibSmbiosInit ();
  if (EFI_ERROR (Status)) {
    goto Done;
  }
  //
  // build statistics table
  //
  Status = InitSmbiosTableStatistics ();
  if (EFI_ERROR (Status)) {
    goto Done;
  }

  StructType    = STRUCTURE_TYPE_RANDOM;
  RandomView    = TRUE;
  //
  // Initialize the StructHandle to be the first handle
  //
  StructHandle  = STRUCTURE_HANDLE_INVALID;
  LibGetSmbiosStructure (&StructHandle, NULL, NULL);

  Item          = LibCheckVarGetFlag (&ChkPck, L"-t");
  if (Item) {
    StructType = (UINT8) StrToUInteger (Item->VarStr, &Status);
    if (EFI_ERROR (Status)) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_ARG), HiiHandle, L"smbiosview", Item->VarStr);
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }
  }

  Item = LibCheckVarGetFlag (&ChkPck, L"-h");
  if (Item) {
    RandomView   = FALSE;
    StructHandle = (UINT16) (StrToUIntegerBase (Item->VarStr, 16, &Status));
    if (EFI_ERROR (Status)) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_ARG), HiiHandle, L"smbiosview", Item->VarStr);
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }
  }

  if (LibCheckVarGetFlag (&ChkPck, L"-s")) {
    Status = DisplayStatisticsTable (SHOW_DETAIL);
    goto Done;
  }

  if (LibCheckVarGetFlag (&ChkPck, L"-a")) {
    gShowType = SHOW_ALL;
  }
  //
  // Show SMBIOS structure information
  //
  Status = SMBiosView (StructType, StructHandle, gShowType, RandomView);

Done:
  //
  // Release resources
  //
  if (mStatisticsTable != NULL) {
    //
    // Release statistics table
    //
    FreePool (mStatisticsTable);
    mStatisticsTable = NULL;
  }

  LibSmbiosCleanup ();

  return Status;
}

EFI_STATUS
SMBiosView (
  IN  UINT8   QueryType,
  IN  UINT16  QueryHandle,
  IN  UINT8   Option,
  IN  BOOLEAN RandomView
  )
/*++
  Routine Description:
    Query all structures Data from SMBIOS table and Display  
    the information to users as required display option.
  
  Aguments:
    QueryType     - Structure type to view
    QueryHandle   - Structure handle to view
    Option        - Display option: none,outline,normal,detail.

  Returns:
    EFI_SUCCESS           - print is successful
    EFI_BAD_BUFFER_SIZE   - structure is out of the range of SMBIOS table
--*/
{
  UINT16                    Handle;
  UINT8                     Buffer[1024];
  //
  // bigger than SMBIOS_STRUCTURE_TABLE.MaxStructureSize
  //
  UINT16                    Length;
  UINTN                     Index;
  UINT16                    Offset;
  //
  // address offset from structure table head.
  //
  UINT32                    TableHead;
  //
  // structure table head.
  //
  EFI_STATUS                Status;
  SMBIOS_STRUCTURE_POINTER  pStruct;
  SMBIOS_STRUCTURE_TABLE    *SMBiosTable;

  SMBiosTable = NULL;
  LibSmbiosGetEPS (&SMBiosTable);
  if (SMBiosTable == NULL) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_SMBIOSVIEW_CANNOT_ACCESS_TABLE), HiiHandle);
    return EFI_BAD_BUFFER_SIZE;
  }

  if (CompareMem (SMBiosTable->AnchorString, "_SM_", 4) == 0) {
    //
    // Have get SMBIOS table
    //
    SmbiosPrintEPSInfo (SMBiosTable, Option);

    Print (L"=========================================================\n");
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_SMBIOSVIEW_QUERY_STRUCT_COND), HiiHandle);

    if (QueryType == STRUCTURE_TYPE_RANDOM) {
      PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_SMBIOSVIEW_QUERYTYPE_RANDOM), HiiHandle);
    } else {
      PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_SMBIOSVIEW_QUERYTYPE), HiiHandle, QueryType);
    }

    if (RandomView) {
      PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_SMBIOSVIEW_QUERYHANDLE_RANDOM), HiiHandle);
    } else {
      PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_SMBIOSVIEW_QUERYHANDLE), HiiHandle, QueryHandle);
    }

    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_SMBIOSVIEW_SHOWTYPE), HiiHandle);
    Print (GetShowTypeString (gShowType));
    Print (L"\n\n");

    //
    // Get internal commands, such as change options.
    //
    Status = WaitEnter ();
    if (EFI_ERROR (Status)) {
      if (Status == EFI_ABORTED) {
        return EFI_SUCCESS;
      }

      return Status;
    }
    //
    // Searching and display structure info
    //
    Handle    = QueryHandle;
    TableHead = SMBiosTable->TableAddress;
    Offset    = 0;
    for (Index = 0; Index < SMBiosTable->NumberOfSmbiosStructures; Index++) {
      //
      // if reach the end of table, break..
      //
      if (Handle == STRUCTURE_HANDLE_INVALID) {
        break;
      }
      //
      // handle then point to the next!
      //
      if (LibGetSmbiosStructure (&Handle, Buffer, &Length) != DMI_SUCCESS) {
        break;
      }
      Offset      = (UINT16) (Offset + Length);
      pStruct.Raw = Buffer;

      //
      // if QueryType==Random, print this structure.
      // if QueryType!=Random, but Hdr->Type==QueryType, also print it.
      // only if QueryType != Random and Hdr->Type != QueryType, skiped it.
      //
      if (QueryType != STRUCTURE_TYPE_RANDOM && pStruct.Hdr->Type != QueryType) {
        continue;
      }

      Print (L"\n=========================================================\n");
      PrintToken (
        STRING_TOKEN (STR_SMBIOSVIEW_SMBIOSVIEW_TYPE_HANDLE_DUMP_STRUCT),
        HiiHandle,
        pStruct.Hdr->Type,
        pStruct.Hdr->Handle
        );
      PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_SMBIOSVIEW_INDEX_LENGTH), HiiHandle, Index, Length);
      //
      // Addr of structure in structure in table
      //
      PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_SMBIOSVIEW_ADDR), HiiHandle, TableHead + Offset);
      DumpHex (0, 0, Length, Buffer);

      //
      // Get internal commands, such as change options.
      //
      Status = WaitEnter ();
      if (EFI_ERROR (Status)) {
        if (Status == EFI_ABORTED) {
          return EFI_SUCCESS;
        }

        return Status;
      }

      if (gShowType != SHOW_NONE) {
        //
        // check structure legality
        //
        SmbiosCheckStructure (&pStruct);

        //
        // Print structure information
        //
        SmbiosPrintStructure (&pStruct, gShowType);
        Print (L"\n");

        //
        // Get internal commands, such as change options.
        //
        Status = WaitEnter ();
        if (EFI_ERROR (Status)) {
          if (Status == EFI_ABORTED) {
            return EFI_SUCCESS;
          }

          return Status;
        }
      }
      if (!RandomView) {
        break;
      }
    }

    Print (L"\n=========================================================\n");
    return EFI_SUCCESS;
  }

  return EFI_BAD_BUFFER_SIZE;
}

EFI_STATUS
WaitEnter (
  VOID
  )
{
  UINTN       Index;
  EFI_STATUS  Status;
  CHAR16      InputStr[3];
  if (gShowType == SHOW_ALL) {
    Print (L"\n");
    return EFI_SUCCESS;
  }

  PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_SMBIOSVIEW_ENTER_Q), HiiHandle);
  Input (L"$", InputStr, 3);

  //
  // skip space characters
  //
  for (Index = 0; InputStr[Index] == L' '; Index++)
    ;

  //
  // Use this opportunity to control!
  //
  if (InputStr[Index] == ':') {
    Index++;
    if (InputStr[Index] == '?') {
      //
      // Press "?" to show help
      //
      ShowHelpInfo ();
      Status = WaitEnter ();
      if (EFI_ERROR (Status)) {
        return Status;
      }
    } else if ((InputStr[Index] == 'q') || (InputStr[Index] == 'Q')) {
      //
      // Press 'q' to quit application
      //
      return EFI_ABORTED;
    } else if ((InputStr[Index] >= L'0') && InputStr[Index] <= L'3') {
      //
      // Change display ShowType Option(0-3)
      //
      gShowType = (UINT8) (InputStr[Index] - (CHAR16) (L'0'));
    } else {
      PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_SMBIOSVIEW_UNKNOWN_INTERNAL_COMMAND), HiiHandle);
    }
  } else if (InputStr[Index] == '/') {
    Index++;
    if (InputStr[Index] == '?') {
      //
      // Press "/?" to show help
      //
      ShowHelpInfo ();
      Status = WaitEnter ();
      if (EFI_ERROR (Status)) {
        return Status;
      }
    } else {
      PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_SMBIOSVIEW_UNKNOWN_INTERNAL_COMMAND), HiiHandle);
    }
  } else {
    Print (L"");
  }

  return EFI_SUCCESS;
}

VOID
ShowHelpInfo (
  VOID
  )
{
  //
  // print help info
  //
  Print (L"\n============================================================\n");
  PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_SMBIOSVIEW_SMBIOS_UTILITY), HiiHandle);
  PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_SMBIOSVIEW_USAGE), HiiHandle);
  PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_SMBIOSVIEW_HSMBIOSVIEW), HiiHandle);
  PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_SMBIOSVIEW_EXAMPLES), HiiHandle);
  PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_SMBIOSVIEW_SHOW_STAT_INFO), HiiHandle);
  PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_SMBIOSVIEW_SHOW_ALL_STRUCT_TYPE), HiiHandle);
  PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_SMBIOSVIEW_SHOW_STRUCT_HANDLE), HiiHandle);
  PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_SMBIOSVIEW_SHOW_ALL_OUTPUT_TO_FILE), HiiHandle);
  Print (L"\n");
  //
  // Internal command
  //
  PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_SMBIOSVIEW_INTERNAL_COMMANDS), HiiHandle);
  PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_SMBIOSVIEW_QUIT_SMBIOSVIEW), HiiHandle);
  PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_SMBIOSVIEW_CHANGE_DISPLAY_NONE), HiiHandle);
  PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_SMBIOSVIEW_CHANGE_DISPLAY_OUTLINE), HiiHandle);
  PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_SMBIOSVIEW_CHANGE_DISPLAY_NORMAL), HiiHandle);
  PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_SMBIOSVIEW_CHANGE_DISPLAY_DETAIL), HiiHandle);
  PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_SMBIOSVIEW_SHOW_HELP), HiiHandle);
  Print (L"\n============================================================\n");
}

VOID
ExitApp (
  EFI_STATUS Status
  )
{
  Print (L"\n");
  if (mStatisticsTable != NULL) {
    //
    // Release statistics table
    //
    FreePool (mStatisticsTable);
    mStatisticsTable = NULL;
  }
  //
  // clean up Lib resource
  //
  LibSmbiosCleanup ();
  //
  // Exit application
  //
  BS->Exit (*mMyImageHandle, Status, 0, NULL);
  mMyImageHandle = NULL;
}

EFI_STATUS
InitSmbiosTableStatistics (
  VOID
  )
{
  UINT16                    Handle;
  UINT8                     Buffer[1024];
  UINT16                    Length;
  UINT16                    Offset;
  UINT16                    Index;

  SMBIOS_STRUCTURE_POINTER  pStruct;
  SMBIOS_STRUCTURE_TABLE    *SMBiosTable;
  STRUCTURE_STATISTICS      *pStatistics;

  SMBiosTable = NULL;
  LibSmbiosGetEPS (&SMBiosTable);
  if (SMBiosTable == NULL) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_SMBIOSVIEW_CANNOT_ACCESS_TABLE), HiiHandle);
    return EFI_NOT_FOUND;
  }

  if (CompareMem (SMBiosTable->AnchorString, "_SM_", 4) != 0) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_SMBIOSVIEW_SMBIOS_TABLE), HiiHandle);
    return EFI_INVALID_PARAMETER;
  }
  //
  // Allocate memory to mStatisticsTable
  //
  if (mStatisticsTable != NULL) {
    FreePool (mStatisticsTable);
    mStatisticsTable = NULL;
  }

  mStatisticsTable = (STRUCTURE_STATISTICS *) AllocatePool (SMBiosTable->NumberOfSmbiosStructures * sizeof (STRUCTURE_STATISTICS));

  if (mStatisticsTable == NULL) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_SMBIOSVIEW_OUT_OF_MEM), HiiHandle);
    return EFI_OUT_OF_RESOURCES;
  }

  Offset      = 0;
  pStatistics = mStatisticsTable;

  //
  // search from the first one
  //
  Handle = STRUCTURE_HANDLE_INVALID;
  LibGetSmbiosStructure (&Handle, NULL, NULL);
  for (Index = 1; Index <= SMBiosTable->NumberOfSmbiosStructures; Index++) {
    //
    // If reach the end of table, break..
    //
    if (Handle == STRUCTURE_HANDLE_INVALID) {
      break;
    }
    //
    // After LibGetSmbiosStructure(), handle then point to the next!
    //
    if (LibGetSmbiosStructure (&Handle, Buffer, &Length) != DMI_SUCCESS) {
      break;
    }

    pStruct.Raw = Buffer;
    Offset      = (UINT16) (Offset + Length);

    //
    // general statistics
    //
    pStatistics->Index  = Index;
    pStatistics->Type   = pStruct.Hdr->Type;
    pStatistics->Handle = pStruct.Hdr->Handle;
    pStatistics->Length = Length;
    pStatistics->Addr   = Offset;

    pStatistics         = &mStatisticsTable[Index];
  }

  return EFI_SUCCESS;
}

EFI_STATUS
DisplayStatisticsTable (
  IN   UINT8   Option
  )
{
  UINTN                   Index;
  UINTN                   Num;
  STRUCTURE_STATISTICS    *pStatistics;
  SMBIOS_STRUCTURE_TABLE  *SMBiosTable;
  EFI_STATUS              Status;

  SMBiosTable = NULL;
  if (Option < SHOW_OUTLINE) {
    return EFI_SUCCESS;
  }
  //
  // display EPS information firstly
  //
  LibSmbiosGetEPS (&SMBiosTable);
  if (SMBiosTable == NULL) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_SMBIOSVIEW_CANNOT_ACCESS_TABLE), HiiHandle);
    return EFI_UNSUPPORTED;
  }

  Print (L"\n============================================================\n");
  SmbiosPrintEPSInfo (SMBiosTable, Option);

  if (Option < SHOW_NORMAL) {
    return EFI_SUCCESS;
  }

  if (mStatisticsTable == NULL) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_SMBIOSVIEW_CANNOT_ACCESS_STATS), HiiHandle);
    return EFI_NOT_FOUND;
  }

  Print (L"============================================================\n");
  pStatistics = &mStatisticsTable[0];
  Num         = SMBiosTable->NumberOfSmbiosStructures;
  //
  // display statistics table content
  //
  for (Index = 1; Index <= Num; Index++) {
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_SMBIOSVIEW_INDEX), HiiHandle, pStatistics->Index);
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_SMBIOSVIEW_TYPE), HiiHandle, pStatistics->Type);
    PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_SMBIOSVIEW_HANDLE), HiiHandle, pStatistics->Handle);
    if (Option >= SHOW_DETAIL) {
      PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_SMBIOSVIEW_OFFSET), HiiHandle, pStatistics->Addr);
      PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_SMBIOSVIEW_LENGTH), HiiHandle, pStatistics->Length);
    }

    Print (L"\n");
    pStatistics = &mStatisticsTable[Index];
    //
    // Display 20 lines and wait for a page break
    //
    if (Index % 20 == 0) {
      PrintToken (STRING_TOKEN (STR_SMBIOSVIEW_SMBIOSVIEW_ENTER_CONTINUE), HiiHandle);
      Status = WaitEnter ();
      if (EFI_ERROR (Status)) {
        if (Status == EFI_ABORTED) {
          return EFI_SUCCESS;
        }

        return Status;
      }
    }
  }

  return EFI_SUCCESS;
}

CHAR16 *
GetShowTypeString (
  UINT8 ShowType
  )
{
  //
  // show type
  //
  switch (ShowType) {

  case SHOW_NONE:
    return L"SHOW_NONE";

  case SHOW_OUTLINE:
    return L"SHOW_OUTLINE";

  case SHOW_NORMAL:
    return L"SHOW_NORMAL";

  case SHOW_DETAIL:
    return L"SHOW_DETAIL";

  case SHOW_ALL:
    return L"SHOW_ALL";

  default:
    return L"Undefined type";
  }
}

EFI_STATUS
InitializeSmbiosViewApplicationGetLineHelp (
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
  return LibCmdGetStringByToken (STRING_ARRAY_NAME, &EfiSmbiosViewGuid, STRING_TOKEN (STR_SMBIOSVIEW_LINE_HELP), Str);
}
