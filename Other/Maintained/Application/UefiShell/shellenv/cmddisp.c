/*++ 

Copyright (c) 2005 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  cmddisp.c
  
Abstract:

  Shell Environment internal command management

Revision History

--*/

#include "shelle.h"
#include "shellenvguid.h"

//
// ////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
////                                                                  ////
////                        Internal prototype                        ////
////                                                                  ////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
EFI_MONOSHELL_CODE (
  
EFI_STATUS
EFIAPI
DriversGetLineHelp (
  OUT CHAR16              **Str
  );

EFI_STATUS
EFIAPI
DrvcfgGetLineHelp (
  OUT CHAR16              **Str
  );

EFI_STATUS
EFIAPI
DrvdiagGetLineHelp (
  OUT CHAR16              **Str
  );

EFI_STATUS
EFIAPI
InitializeResetGetLineHelp (
  OUT CHAR16              **Str
  );

EFI_STATUS
EFIAPI
InitializeReset (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
EFIAPI
InitializeCPGetLineHelp (
  OUT CHAR16              **Str
  );

EFI_STATUS
EFIAPI
InitializeCP (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
EFIAPI
InitializeRMGetLineHelp (
  OUT CHAR16              **Str
  );

EFI_STATUS
EFIAPI
InitializeRM (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
EFIAPI
InitializeDateGetLineHelp (
  OUT CHAR16              **Str
  );

EFI_STATUS
EFIAPI
InitializeDate (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
EFIAPI
DriversMain (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
EFIAPI
InitializeLSGetLineHelp (
  OUT CHAR16              **Str
  );

EFI_STATUS
EFIAPI
InitializeLS (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
EFIAPI
InitializeMkDirGetLineHelp (
  OUT CHAR16              **Str
  );

EFI_STATUS
EFIAPI
InitializeMkDir (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
EFIAPI
InitializeTimeGetLineHelp (
  OUT CHAR16              **Str
  );

EFI_STATUS
EFIAPI
InitializeTime (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
EFIAPI
InitializeVolGetLineHelp (
  OUT CHAR16              **Str
  );

EFI_STATUS
EFIAPI
InitializeVol (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
EFIAPI
InitializeVerGetLineHelp (
  OUT CHAR16              **Str
  );

EFI_STATUS
EFIAPI
InitializeVer (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
EFIAPI
InitializeTypeGetLineHelp (
  OUT CHAR16              **Str
  );

EFI_STATUS
EFIAPI
InitializeType (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
EFIAPI
InitializeMvGetLineHelp (
  OUT CHAR16              **Str
  );

EFI_STATUS
EFIAPI
InitializeMv (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
EFIAPI
InitializeTouchGetLineHelp (
  OUT CHAR16              **Str
  );

EFI_STATUS
EFIAPI
InitializeTouch (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
EFIAPI
InitializeAttribGetLineHelp (
  OUT CHAR16              **Str
  );

EFI_STATUS
EFIAPI
InitializeAttrib (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
EFIAPI
InitializeClsGetLineHelp (
  OUT CHAR16              **Str
  );

EFI_STATUS
EFIAPI
InitializeCls (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
EFIAPI
InitializeLoadGetLineHelp (
  OUT CHAR16              **Str
  );

EFI_STATUS
EFIAPI
InitializeLoad (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
EFIAPI
InitializeUnloadGetLineHelp (
  OUT CHAR16              **Str
  );

EFI_STATUS
EFIAPI
InitializeUnload (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
GuidMain (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  );

EFI_STATUS
OpeninfoMain (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  );

EFI_STATUS
EFIAPI
DrvcfgMain (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  );

EFI_STATUS
EFIAPI
DrvdiagMain (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  );
)
#ifdef EFI_FULLSHELL
EFI_MONOSHELL_CODE (
EFI_STATUS
EFIAPI
InitializeCompGetLineHelp (
  OUT CHAR16              **Str
  );

EFI_STATUS
EFIAPI
InitializeComp (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
EFIAPI
DumpBlockDev (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );
  
EFI_STATUS
EFIAPI
InitializeDblkGetLineHelp (
  OUT CHAR16              **Str
  );
  
EFI_STATUS
EFIAPI
DevicesMain (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  );
  
EFI_STATUS
EFIAPI
InitializeDevicesGetLineHelp (
  OUT CHAR16              **Str
  );

EFI_STATUS
EFIAPI
DevicetreeMain (
  IN EFI_HANDLE               ImageHandle,
  IN EFI_SYSTEM_TABLE         *SystemTable
  );
  
EFI_STATUS
EFIAPI
DevicetreeMainGetLineHelp (
  OUT CHAR16                **Str
  );
  
EFI_STATUS
EFIAPI
DumpMem (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );
  
EFI_STATUS
EFIAPI
DumpMemGetLineHelp (
  OUT CHAR16          **Str
  );
  
EFI_STATUS
EFIAPI
InitializeDumpStore (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  );
  
EFI_STATUS
EFIAPI
InitializeDumpStoreGetLineHelp (
  OUT CHAR16                **Str
  );
  
EFI_STATUS
EFIAPI
InitializeEFIEditor (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );
  
EFI_STATUS
EFIAPI
InitializeEFIEditorGetLineHelp (
  OUT CHAR16              **Str
  );
  
EFI_STATUS
EFIAPI
InitializeCompress (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
EFIAPI
InitializeCompressGetLineHelp (
  OUT CHAR16                     **Str
  );
  
EFI_STATUS
EFIAPI
InitializeDecompress (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );
  
EFI_STATUS
EFIAPI
InitializeDecompressGetLineHelp (
  OUT CHAR16                  **Str
  );
  
EFI_STATUS
EFIAPI
InitializeError (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  );
  
EFI_STATUS
EFIAPI
InitializeErrorGetLineHelp (
  OUT CHAR16              **Str
  );

EFI_STATUS
EFIAPI
GuidMain (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  );

EFI_STATUS
EFIAPI
GuidMainGetLineHelp (
  OUT CHAR16          **Str
  );
  
EFI_STATUS
EFIAPI
InitializeEFIHexEditor (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
EFIAPI
InitializeEFIHexEditorGetLineHelp (
  OUT CHAR16              **Str
  );
  
EFI_STATUS
EFIAPI
InitializeIpConfig (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  );
  
EFI_STATUS
EFIAPI
InitializeIpConfigGetLineHelp (
  OUT CHAR16              **Str
  );
  
EFI_STATUS
EFIAPI
InitializeLoadPciRom (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  );
  
EFI_STATUS
EFIAPI
InitializeLoadPciRomGetLineHelp (
  OUT CHAR16              **Str
  );
  
EFI_STATUS
EFIAPI
DumpMm (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );
  
EFI_STATUS
EFIAPI
DumpMmGetLineHelp (
  OUT CHAR16          **Str
  );
  
EFI_STATUS
EFIAPI
InitializeMemmap (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  );
  
EFI_STATUS
EFIAPI
InitializeMemmapGetLineHelp (
  OUT CHAR16              **Str
  );
  
EFI_STATUS
EFIAPI
DumpIoModify (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );
  
EFI_STATUS
EFIAPI
InitializeMMGetLineHelp (
  OUT CHAR16              **Str
  );
  
EFI_STATUS
EFIAPI
InitializeMode (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  );
  
EFI_STATUS
EFIAPI
InitializeModeGetLineHelp (
  OUT CHAR16              **Str
  );
  
EFI_STATUS
EFIAPI
MountMain (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  );
  
EFI_STATUS
EFIAPI
MountMainGetLineHelp (
  OUT CHAR16              **Str
  );

EFI_STATUS
EFIAPI
OpeninfoMain (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
);

EFI_STATUS
EFIAPI
InitializeOpenInfoGetLineHelp (
  OUT CHAR16              **Str
  );
  
EFI_STATUS
EFIAPI
PciDump (
  IN EFI_HANDLE                             ImageHandle,
  IN EFI_SYSTEM_TABLE                       *SystemTable
  );
  
EFI_STATUS
EFIAPI
InitializePciGetLineHelp (
  OUT CHAR16              **Str
  );
  
EFI_STATUS
EFIAPI
InitializeSerialMode (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  );
  
EFI_STATUS
EFIAPI
InitializeSerialModeGetLineHelp (
  OUT CHAR16              **Str
  );
  
EFI_STATUS
EFIAPI
InitializeSmbiosViewApplication (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
EFIAPI
InitializeSmbiosViewApplicationGetLineHelp (
  OUT CHAR16              **Str
  );
  
EFI_STATUS
EFIAPI
InitializeStall (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );
  
EFI_STATUS
EFIAPI
InitializeStallGetLineHelp (
  OUT CHAR16            **Str
  );
  
EFI_STATUS
EFIAPI
InitializeTelnetMgmt (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  );

EFI_STATUS
EFIAPI
InitializeTelnetmgmtGetLineHelp (
  OUT CHAR16              **Str
  );
  
EFI_STATUS
EFIAPI
InitializeTZone (
  IN EFI_HANDLE             hImageHandle,
  IN EFI_SYSTEM_TABLE    *pSystemTable
  );

EFI_STATUS
EFIAPI
InitializeZoneGetLineHelp (
  OUT CHAR16              **Str
  );
)
#endif

struct {
  SHELLENV_INTERNAL_COMMAND Dispatch;
  CHAR16                    *Cmd;
  SHELLCMD_GET_LINE_HELP    GetLineHelp;
}

SEnvInternalCommands[] = {
  SEnvHelp,
  L"?",
  SEnvHelpGetLineHelp,
  SEnvHelp,
  L"help",
  SEnvHelpGetLineHelp,
  SEnvCmdSet,
  L"set",
  SEnvCmdSetGetLineHelp,
  SEnvCmdAlias,
  L"alias",
  SEnvCmdAliasGetLineHelp,
  SEnvCmdDH,
  L"dh",
  SEnvCmdDHGetLineHelp,
  SEnvCmdConnect,
  L"connect",
  SEnvCmdConnectGetLineHelp,
  SEnvCmdDisconnect,
  L"disconnect",
  SEnvCmdDisconnectGetLineHelp,
  SEnvCmdReconnect,
  L"reconnect",
  SEnvCmdReconnectGetLineHelp,
  SEnvCmdMap,
  L"map",
  SEnvCmdMapGetLineHelp,
  SEnvCmdCd,
  L"cd",
  SEnvCmdCdGetLineHelp,
  SEnvCmdEcho,
  L"echo",
  SEnvCmdEchoGetLineHelp,
  SEnvCmdIf,
  L"if",
  SEnvCmdIfGetLineHelp,
  SEnvCmdElse,
  L"else",
  SEnvCmdElseGetLineHelp,
  SEnvCmdEndif,
  L"endif",
  SEnvCmdEndifGetLineHelp,
  SEnvCmdShift,
  L"shift",
  SEnvCmdShiftGetLineHelp,
  SEnvCmdGoto,
  L"goto",
  SEnvCmdGotoGetLineHelp,
  SEnvCmdFor,
  L"for",
  SEnvCmdForGetLineHelp,
  SEnvCmdEndfor,
  L"endfor",
  SEnvCmdEndforGetLineHelp,
  SEnvCmdPause,
  L"pause",
  SEnvCmdPauseGetLineHelp,
  SEnvExit,
  L"exit",
  SEnvExitGetLineHelp,

  SEnvLoadDefaults,
  L"_load_defaults",
  0,
  SEnvNoUse,
  L"_this_is_not_a_useful_command",
  0,

#ifdef EFI_MONOSHELL
  InitializeUnload,
  L"unload",
  InitializeUnloadGetLineHelp,
  DriversMain,
  L"drivers",
  DriversGetLineHelp,
  DrvcfgMain,
  L"drvcfg",
  DrvcfgGetLineHelp,
  DrvdiagMain,
  L"drvdiag",
  DrvdiagGetLineHelp,
  InitializeReset,
  L"reset",
  InitializeResetGetLineHelp,
  InitializeAttrib,
  L"attrib",
  InitializeAttribGetLineHelp,
  InitializeCls,
  L"cls",
  InitializeClsGetLineHelp,
  InitializeCP,
  L"cp",
  InitializeCPGetLineHelp,
  InitializeDate,
  L"date",
  InitializeDateGetLineHelp,
  InitializeLS,
  L"ls",
  InitializeLSGetLineHelp,
  InitializeMkDir,
  L"mkdir",
  InitializeMkDirGetLineHelp,
  InitializeMv,
  L"mv",
  InitializeMvGetLineHelp,
  InitializeRM,
  L"rm",
  InitializeRMGetLineHelp,
  InitializeVer,
  L"ver",
  InitializeVerGetLineHelp,
  InitializeTouch,
  L"touch",
  InitializeTouchGetLineHelp,
  InitializeTime,
  L"time",
  InitializeTimeGetLineHelp,
  InitializeLoad,
  L"load",
  InitializeLoadGetLineHelp,
  InitializeType,
  L"type",
  InitializeTypeGetLineHelp,
  InitializeVol,
  L"vol",
  InitializeVolGetLineHelp,

#ifdef EFI_FULLSHELL
  InitializeComp,
  L"comp",
  InitializeCompGetLineHelp,
  DumpBlockDev,
  L"dblk",
  InitializeDblkGetLineHelp,
  DevicesMain,
  L"devices",
  InitializeDevicesGetLineHelp,
  DevicetreeMain,
  L"devtree",
  DevicetreeMainGetLineHelp,
  DumpMem,
  L"dmem",
  DumpMemGetLineHelp,
  InitializeDumpStore,
  L"dmpstore",
  InitializeDumpStoreGetLineHelp,
  InitializeEFIEditor,
  L"edit",
  InitializeEFIEditorGetLineHelp,
  InitializeCompress,
  L"eficompress",
  InitializeCompressGetLineHelp,
  InitializeDecompress,
  L"efidecompress",
  InitializeDecompressGetLineHelp,
  InitializeError,
  L"err",
  InitializeErrorGetLineHelp,
  GuidMain,
  L"guid",
  GuidMainGetLineHelp,
  InitializeEFIHexEditor,
  L"hexedit",
  InitializeEFIHexEditorGetLineHelp,
  InitializeIpConfig,
  L"ipconfig",
  InitializeIpConfigGetLineHelp,
  InitializeLoadPciRom,
  L"loadpcirom",
  InitializeLoadPciRomGetLineHelp,
  DumpMm,
  L"mem",
  DumpMmGetLineHelp,
  InitializeMemmap,
  L"memmap",
  InitializeMemmapGetLineHelp,
  DumpIoModify,
  L"mm",
  InitializeMMGetLineHelp,
  InitializeMode,
  L"mode",
  InitializeModeGetLineHelp,
  MountMain,
  L"mount",
  MountMainGetLineHelp,
  OpeninfoMain,
  L"openinfo",
  InitializeOpenInfoGetLineHelp,
  PciDump,
  L"pci",
  InitializePciGetLineHelp,
  InitializeSerialMode,
  L"sermode",
  InitializeSerialModeGetLineHelp,
  InitializeSmbiosViewApplication,
  L"smbiosview",
  InitializeSmbiosViewApplicationGetLineHelp,
  InitializeStall,
  L"stall",
  InitializeStallGetLineHelp,
  InitializeTelnetMgmt,
  L"telnetmgmt",
  InitializeTelnetmgmtGetLineHelp,
  InitializeTZone,
  L"timezone",
  InitializeZoneGetLineHelp,
#endif

#endif

  {
    NULL
  }
};

//
//
//
VOID
SEnvInitCommandTable (
  VOID
  )
/*++

Routine Description:

Arguments:

Returns:

--*/
{
  UINTN Index;

  //
  // Add all of our internal commands to the command dispatch table
  //
  InitializeListHead (&SEnvCmds);
  for (Index = 0; SEnvInternalCommands[Index].Dispatch; Index += 1) {
    SEnvAddCommand (
      SEnvInternalCommands[Index].Dispatch,
      SEnvInternalCommands[Index].Cmd,
      SEnvInternalCommands[Index].GetLineHelp
      );
  }
}

EFI_STATUS
EFIAPI
SEnvAddCommand (
  IN SHELLENV_INTERNAL_COMMAND    Handler,
  IN CHAR16                       *CmdStr,
  IN SHELLCMD_GET_LINE_HELP       GetLineHelp
  )
/*++

Routine Description:

Arguments:

  Handler     - Handler
  CmdStr      - The command name
  GetLineHelp - The interface of get line help

Returns:

--*/
{
  COMMAND         *Cmd;
  COMMAND         *Command;
  EFI_LIST_ENTRY  *Link;

  Command = NULL;
  Cmd     = AllocateZeroPool (sizeof (COMMAND));

  if (Cmd) {
    AcquireLock (&SEnvLock);

    Cmd->Signature    = COMMAND_SIGNATURE;
    Cmd->Dispatch     = Handler;
    Cmd->Cmd          = CmdStr;
    Cmd->GetLineHelp  = GetLineHelp;
    InsertTailList (&SEnvCmds, &Cmd->Link);

    //
    // Find the proper place of Cmd
    //
    for (Link = SEnvCmds.Flink; Link != &SEnvCmds; Link = Link->Flink) {
      Command = CR (Link, COMMAND, Link, COMMAND_SIGNATURE);
      if (StriCmp (Command->Cmd, Cmd->Cmd) > 0) {
        //
        //  Insert it to proper place
        //
        SwapListEntries (&Command->Link, &Cmd->Link);
        break;
      }
    }

    ReleaseLock (&SEnvLock);
  }

  return Cmd ? EFI_SUCCESS : EFI_OUT_OF_RESOURCES;
}

EFI_STATUS
SEnvFreeCommandTable (
  VOID
  )
{
  COMMAND *Cmd;

  AcquireLock (&SEnvLock);

  while (!IsListEmpty (&SEnvCmds)) {
    Cmd = CR (SEnvCmds.Flink, COMMAND, Link, COMMAND_SIGNATURE);
    RemoveEntryList (&Cmd->Link);
    FreePool (Cmd);
  }

  ReleaseLock (&SEnvLock);

  return EFI_SUCCESS;
}

SHELLENV_INTERNAL_COMMAND
SEnvGetCmdDispath (
  IN CHAR16                   *CmdName
  )
/*++

Routine Description:

Arguments:

  CmdName - Command name
  
Returns:

--*/
{
  EFI_LIST_ENTRY            *Link;
  COMMAND                   *Command;
  SHELLENV_INTERNAL_COMMAND Dispatch;

  Dispatch = NULL;
  AcquireLock (&SEnvLock);

  //
  // Walk through SEnvCmds linked list
  // to get corresponding command dispatch
  //
  for (Link = SEnvCmds.Flink; Link != &SEnvCmds; Link = Link->Flink) {
    Command = CR (Link, COMMAND, Link, COMMAND_SIGNATURE);
    if (StriCmp (Command->Cmd, CmdName) == 0) {
      Dispatch = Command->Dispatch;
      break;
    }
  }

  ReleaseLock (&SEnvLock);
  return Dispatch;
}
