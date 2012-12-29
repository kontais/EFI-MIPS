/*++

Copyright (c) 2005 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  ShellEnvInt.c

Abstract:

  Shell enviorment interface functions

Revision History

--*/

#include "EfiShellLib.h"

EFI_STATUS
ShellExecute (
  IN EFI_HANDLE       ImageHandle,
  IN CHAR16           *CmdLine,
  IN BOOLEAN          Output
  )
/*++

Routine Description:
 
  ShellExecute - causes the shell to parse & execute the command line

Arguments:

    ImageHandle - Image handle
    CmdLine     - Command line
    Output      - The output

Returns:
  
--*/
{
  return SE->Execute (ImageHandle, CmdLine, Output);
}

BOOLEAN
ShellBatchIsActive (
  VOID
  )
/*++

Routine Description:
  
Arguments:

   ShellBatchIsActive - return current execution is in batch script or not
   
Returns:
  
--*/
{
  return SE->BatchIsActive ();
}

VOID
EnablePageBreak (
  IN INT32      StartRow,
  IN BOOLEAN    AutoWrap
  )
/*++

Routine Description:
  
Arguments:

    StartRow - Start Row
    AutoWrap - Auto wrap or not

Returns:
  
--*/
{
  SE2->EnablePageBreak (StartRow, AutoWrap);
}

VOID
DisablePageBreak (
  VOID
  )
/*++

Routine Description:
  
Arguments:

Returns:
  
--*/
{
  SE2->DisablePageBreak ();
}

BOOLEAN
GetPageBreak (
  VOID
  )
/*++

Routine Description:
  
Arguments:

Returns:
  
--*/
{
  return SE2->GetPageBreak ();
}

VOID
EnableOutputTabPause (
  VOID
  )
/*++

Routine Description:

  Enable tab key which can pause output

Arguments:

Returns:

--*/
{
  SE2->SetKeyFilter (SE2->GetKeyFilter () | EFI_OUTPUT_PAUSE);
}

VOID
DisableOutputTabPause (
  VOID
  )
/*++

Routine Description:

  disable tab key which can pause output

  Disable pause key

Arguments:

Returns:
  
--*/
{
  SE2->SetKeyFilter (SE2->GetKeyFilter () & ~EFI_OUTPUT_PAUSE);
}

BOOLEAN
GetOutputTabPause (
  VOID
  )
/*++

Routine Description:

  Get the status of tab pause
  
Arguments:

Returns:
  TRUE    - the status of the tab key is enabled
  FALSE - the status of the tab key is disabled
  
--*/
{
  return (BOOLEAN) ((SE2->GetKeyFilter () & EFI_OUTPUT_PAUSE) != 0);
}

VOID
SetKeyFilter (
  IN UINT32      KeyFilter
  )
/*++

Routine Description:
  Set curretn ky filter setting
  
Arguments:  
  KeyFilter   - The new key filter to set
               
Returns:    
--*/
{
  SE2->SetKeyFilter (KeyFilter);
}

UINT32
GetKeyFilter (
  IN VOID
  )
/*++

Routine Description:
  Get curretn ky filter setting
  
Arguments:  
            
  None
  
Returns:    

--*/
{
  return SE2->GetKeyFilter ();
}

BOOLEAN
GetExecutionBreak (
  IN VOID
  )
/*++

Routine Description:
  Get the enable status of the execution break flag.
  
Arguments:
  
  None
  
Returns:

--*/
{
  return SE2->GetExecutionBreak ();
}

CHAR16 *
ShellGetEnv (
  IN CHAR16       *Name
  )
/*++

Routine Description:
  
 ShellGetEnv - returns the current mapping for the Env Name

Arguments:

  Name       - Environment variable name
  
Returns:
  
--*/
{
  return SE->GetEnv (Name);
}

EFI_DEVICE_PATH_PROTOCOL *
ShellGetMap (
  IN CHAR16       *Name
  )
/*++

Routine Description:
  
Arguments:

  Name    -  Mapping Name
Returns:
  
--*/
{
  CHAR16                    *DevName;
  CHAR16                    *Ptr;
  EFI_DEVICE_PATH_PROTOCOL  *DevPath;

  DevName = NULL;
  DevPath = NULL;
  Ptr     = NULL;

  if (Name) {
    DevName = StrDuplicate (Name);
    if (DevName == NULL) {
      return NULL;
    }

    Ptr = StrChr (DevName, ':');
    if (Ptr) {
      *Ptr = 0;
    }
  }

  DevPath = (EFI_DEVICE_PATH_PROTOCOL *) SE->GetMap (DevName);

  if (DevName) {
    FreePool (DevName);
  }

  return DevPath;
}

CHAR16 *
ShellCurDir (
  IN CHAR16               *DeviceName OPTIONAL
  )
/*++

Routine Description:
  
  returns the current directory on the current mapped device
  
Arguments:

  DeviceName - Device name
Returns:

Notes:
 Results are allocated from pool.  The caller must free the pool
  
--*/
{
  return SE->CurDir (DeviceName);
}

EFI_STATUS
ShellFileMetaArg (
  IN CHAR16               *Arg,
  IN OUT EFI_LIST_ENTRY   *ListHead
  )
/*++

Routine Description:
  
Arguments:

  Arg      - The argument
  ListHead - The list head
Returns:
  
--*/
{
  return SE->FileMetaArg (Arg, ListHead);
}

EFI_STATUS
ShellFileMetaArgNoWildCard (
  IN CHAR16               *Arg,
  IN OUT EFI_LIST_ENTRY   *ListHead
  )
/*++

Routine Description:
  
Arguments:

  Arg      - The argument
  ListHead - The list head

Returns:
  
--*/
{
  return SE2->FileMetaArgNoWildCard (Arg, ListHead);
}

EFI_STATUS
ShellFreeFileList (
  IN OUT EFI_LIST_ENTRY   *ListHead
  )
/*++

Routine Description:
  
Arguments:

  ListHead   - The list head
Returns:
  
--*/
{
  return SE->FreeFileList (ListHead);
}

UINTN
ShellGetHandleNum (
  VOID
  )
{
  return SE2->HandleEnumerator.GetNum ();
}

VOID
ShellInitHandleEnumerator (
  VOID
  )
{
  //
  // very simple wrapper of shell environment protocol
  //
  SE2->HandleEnumerator.Init ();
}

EFI_STATUS
ShellNextHandle (
  IN OUT   EFI_HANDLE            **Handle
  )
{
  //
  // very simple wrapper of shell environment protocol
  //
  return SE2->HandleEnumerator.Next (Handle);
}

EFI_STATUS
ShellSkipHandle (
  IN UINTN                  SkipNum
  )
{
  //
  // very simple wrapper of shell environment protocol
  //
  return SE2->HandleEnumerator.Skip (SkipNum);
}

VOID
ShellResetHandleEnumerator (
  VOID
  )
{
  //
  // very simple wrapper of shell environment protocol
  //
  SE2->HandleEnumerator.Reset (0);
}

VOID
ShellCloseHandleEnumerator (
  VOID
  )
{
  //
  // very simple wrapper of shell environment protocol
  //
  SE2->HandleEnumerator.Close ();
}

EFI_STATUS
ShellDelDupFileArg (
  IN EFI_LIST_ENTRY   *ListHead
  )
/*++

Routine Description:
  
Arguments:

  ListHead - The list head
  
Returns:
  
--*/
{
  return SE2->DelDupFileArg (ListHead);
}

VOID
ShellCloseConsoleProxy (
  IN     EFI_HANDLE                       ConInHandle,
  IN OUT EFI_SIMPLE_TEXT_IN_PROTOCOL      **ConIn,
  IN     EFI_HANDLE                       ConOutHandle,
  IN OUT EFI_SIMPLE_TEXT_OUT_PROTOCOL     **ConOut
  )
{
  SE2->CloseConsoleProxy (ConInHandle, ConIn, ConOutHandle, ConOut);
}

VOID
ShellFreeResources (
  VOID
  )
/*++

Routine Description:
  
Arguments:

Returns:
  
--*/
{
  SE->FreeResources ();
}

VOID
ShellInitProtocolInfoEnumerator (
  IN VOID
  )
{
  SE2->ProtocolInfoEnumerator.Init ();
}

EFI_STATUS
ShellNextProtocolInfo (
  IN OUT PROTOCOL_INFO            **ProtocolInfo
  )
{
  return SE2->ProtocolInfoEnumerator.Next (ProtocolInfo);
}

EFI_STATUS
ShellSkipProtocolInfo (
  IN UINTN                        SkipNum
  )
{
  //
  // very simple wrapper of shell environment protocol
  //
  return SE2->ProtocolInfoEnumerator.Skip (SkipNum);
}

VOID
ShellResetProtocolInfoEnumerator (
  VOID
  )
{
  //
  // very simple wrapper of shell environment protocol
  //
  SE2->ProtocolInfoEnumerator.Reset ();
}

VOID
ShellCloseProtocolInfoEnumerator (
  VOID
  )
{
  //
  // very simple wrapper of shell environment protocol
  //
  SE2->ProtocolInfoEnumerator.Close ();
}
