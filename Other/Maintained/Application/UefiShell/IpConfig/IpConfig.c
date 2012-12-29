/*++

Copyright (c) 2005 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  IpConfig.c
  
Abstract:

  Shell command "IpConfig"


Revision History

--*/

#include "EfiShellLib.h"
#include "IpConfig.h"

extern UINT8  STRING_ARRAY_NAME[];

//
// This is the generated header file which includes whatever needs to be exported (strings + IFR)
//
#include STRING_DEFINES_FILE

#include EFI_PROTOCOL_DEFINITION (PxeBaseCode)
#include EFI_PROTOCOL_DEFINITION (SimpleNetwork)
#include EFI_PROTOCOL_DEFINITION (PxeDhcp4)

//
// Global Variables
//
EFI_HII_HANDLE  HiiHandle;
EFI_GUID        EfiIpConfigGuid = EFI_IPCONFIG_GUID;
SHELL_VAR_CHECK_ITEM  IpconfigCheckList[] = {
  {
    L"-r",
    0x01,
    0x16,
    FlagTypeSingle
  },
  {
    L"-m",
    0x02,
    0x11,
    FlagTypeNeedVar
  },
  {
    L"-c",
    0x04,
    0x11,
    FlagTypeNeedVar
  },
  {
    L"-b",
    0x08,
    0,
    FlagTypeSingle
  },
  {
    L"-?",
    0x10,
    0x07,
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
// MACRO
//
#define IPV4_MAC_ADDRESS_LEN  6
#define EFI_IP_V4_BYTES       4
#define IP_STRING_MAX_LENGTH  32

EFI_STATUS
InitializeIpConfig (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  );

EFI_STATUS
EfiInetAddr (
  IN  CHAR16            *Ptr,
  OUT EFI_IP_ADDRESS    *Ip
  );

EFI_STATUS
EfiRestartPxeBaseCode (
  EFI_PXE_BASE_CODE_PROTOCOL   *PxeBaseCode,
  EFI_PXE_DHCP4_PROTOCOL       *PxeDhcp4
  );

DHCP4_OP *
FindDhcp4OptionInBuffer (
  UINT8 Op,
  UINTN *SkipPtr,
  UINT8 *Buffer,
  UINTN BufferLen
  )
/*++

Routine Description:

  Find the Dhcp4 option in the buffer

Arguments:

  Op           - The option
  SkipPtr      - Skip number
  Buffer       - The buffer
  BufferLen    - Buffer length

Returns:

  The DHCP4 option.

--*/
{
  UINTN TmpSkip;

  //
  // A couple of stupidity checks.
  //
  if (Buffer == NULL || BufferLen < 4) {
    return NULL;
  }
  //
  // Assume a skip value of zero if a NULL skip pointer was passed in.
  //
  if (SkipPtr == NULL) {
    SkipPtr = &TmpSkip;
    TmpSkip = 0;
  }
  //
  //
  //
  for (;;) {
    //
    // If this is what we are looking for, return a pointer to the option
    // if the skip counter is zero.
    //
    if (*Buffer == Op) {
      if (*SkipPtr-- == 0) {
        return (DHCP4_OP *) Buffer;
      }
    }
    //
    // Special checks for PAD and END options.
    //
    switch (*Buffer) {
    case DHCP4_END:
      //
      // Return not found when END is reached.
      //
      return NULL;

    case DHCP4_PAD:
      //
      // PAD options are always one byte in length.
      //
      ++Buffer;

      if (--BufferLen == 0) {
        return NULL;
      }

      continue;
    }
    //
    // Return not found if there is not enough space left for more options.
    //
    if (BufferLen == 1) {
      return NULL;
    }

    if ((UINTN) (Buffer[1] + 2) >= BufferLen) {
      return NULL;
    }
    //
    // Skip past current option.
    //
    Buffer += 2 + Buffer[1];
  }
}

DHCP4_OP *
FindDhcp4Option (
  UINT8         Op,
  UINTN         Skip,
  DHCP4_PACKET  *Packet
  )
/*++

Routine Description:

  Find the Dhcp4 option in the buffer

Arguments:

  Op           - The option
  Skip         - Skip number
  Packet       - The Dhcp4 packet

Returns:

  The DHCP4 option.

--*/
{
  DHCP4_OP  *MaxMessageSizePtr;
  DHCP4_OP  *OptionOverloadPtr;
  DHCP4_OP  *OpPtr;
  UINTN     OptionLen;

  //
  // Do nothing if Packet pointer is NULL.
  //
  if (Packet == NULL) {
    return NULL;
  }
  //
  // Only the first DHCP4_END can be found.
  //
  if (Op == DHCP4_END && Skip != 0) {
    return NULL;
  }
  //
  // Determine maximum length of options[] field.
  //
  OptionLen = sizeof Packet->dhcp4.options;

  MaxMessageSizePtr = FindDhcp4OptionInBuffer (
                        DHCP4_MAX_MESSAGE_SIZE,
                        NULL,
                        Packet->dhcp4.options,
                        OptionLen
                        );

  if (MaxMessageSizePtr != NULL) {
    //
    // If the option length is not valid (2), return not found.
    //
    if (MaxMessageSizePtr->len != 2) {
      return NULL;
    }

    OptionLen = (MaxMessageSizePtr->data[0] << 8) | MaxMessageSizePtr->data[1];

    //
    // If the maximum message length is less than the minimum DHCP4 packet length,
    // return not found.
    //
    if (OptionLen < 300) {
      return NULL;
    }
    //
    // Set the new options[] field length by subtracting out the fixed portion of
    // the packet header.
    //
    OptionLen -= (UINT8 *) Packet->dhcp4.options - (UINT8 *) Packet;
  }
  //
  // Determine if the sname[] and fname[] fields are overloaded.
  //
  OptionOverloadPtr = FindDhcp4OptionInBuffer (
                        DHCP4_OPTION_OVERLOAD,
                        NULL,
                        Packet->dhcp4.options,
                        OptionLen
                        );

  if (OptionOverloadPtr->len != 1 || (OptionOverloadPtr->data[0] & 3) == 0) {
    //
    // Ignore sname[] and fname[] fields if the overload option is not valid.
    //
    OptionOverloadPtr = NULL;
  }
  //
  // Now, look for the caller's option.
  //
  OpPtr = FindDhcp4OptionInBuffer (
            Op,
            &Skip,
            Packet->dhcp4.options,
            OptionLen
            );

  if (OpPtr != NULL || Skip == 0 || OptionOverloadPtr == NULL) {
    return OpPtr;
  }

  if (OptionOverloadPtr->data[0] & DHCP4_OVERLOAD_FNAME) {
    OpPtr = FindDhcp4OptionInBuffer (
              Op,
              &Skip,
              Packet->dhcp4.fname,
              sizeof Packet->dhcp4.fname
              );

    if (OpPtr != NULL || Skip == 0) {
      return OpPtr;
    }
  }

  if (OptionOverloadPtr->data[0] & DHCP4_OVERLOAD_SNAME) {
    OpPtr = FindDhcp4OptionInBuffer (
              Op,
              &Skip,
              Packet->dhcp4.sname,
              sizeof Packet->dhcp4.sname
              );
  }

  return OpPtr;
}

EFI_BOOTSHELL_CODE(
  EFI_APPLICATION_ENTRY_POINT(InitializeIpConfig)
)

EFI_STATUS
InitializeIpConfig (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
/*++
Routine Description:

  Command entry point. 

Arguments:

  ImageHandle     The image handle. 
  SystemTable     The system table.

Returns:

  EFI_SUCCESS      - The command completed successfully
  EFI_NOT_STARTED  - PXE bc DHCP has not started yet
  EFI_NOT_FOUND    - Depending protocol not found
  EFI_ABORTED      - Set IP address error

--*/
{
  EFI_STATUS                  Status;
  EFI_IP_ADDRESS              IpAddress;
  EFI_IP_ADDRESS              IpMask;
  BOOLEAN                     NewIp;
  BOOLEAN                     NewMsk;
  EFI_SIMPLE_NETWORK_PROTOCOL *SimpleNetwork;
  EFI_PXE_BASE_CODE_PROTOCOL  *PxeBaseCode;
  EFI_PXE_DHCP4_PROTOCOL      *PxeDhcp4;
  UINTN                       Index;
  UINT8                       *Ptr;
  UINTN                       Instance;
  UINTN                       Count;
  UINTN                       Bits;
  UINT32                      Mask;
  BOOLEAN                     TurnFlag;
  EFI_HANDLE                  *HandleBuffer;
  CHAR16                      *Useful;
  SHELL_ARG_LIST              *Item;
  SHELL_VAR_CHECK_CODE        RetCode;
  SHELL_VAR_CHECK_PACKAGE     ChkPck;

  TurnFlag  = FALSE;
  //
  // We are no being installed as an internal command driver, initialize
  // as an nshell app and run
  //
  EFI_SHELL_APP_INIT (ImageHandle, SystemTable);
  
  //
  // Enable tab key which can pause the output
  //
  EnableOutputTabPause();
  
  //
  // Register our string package with HII and return the handle to it.
  // If previously registered we will simply receive the handle
  //
  Status = LibInitializeStrings (&HiiHandle, STRING_ARRAY_NAME, &EfiIpConfigGuid);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (!EFI_PROPER_VERSION (0, 99)) {
    PrintToken (
      STRING_TOKEN (STR_SHELLENV_GNC_COMMAND_NOT_SUPPORT),
      HiiHandle,
      L"ipconfig",
      EFI_VERSION_0_99 
      );
    Status = EFI_UNSUPPORTED;
    goto Done;
  }

  NewIp   = FALSE;
  NewMsk  = FALSE;
  ZeroMem (&ChkPck, sizeof (SHELL_VAR_CHECK_PACKAGE));
  ZeroMem (&IpAddress, sizeof (EFI_IP_ADDRESS));
  ZeroMem (&IpMask, sizeof (EFI_IP_ADDRESS));

  LibFilterNullArgs ();
  RetCode = LibCheckVariables (SI, IpconfigCheckList, &ChkPck, &Useful);
  if (VarCheckOk != RetCode) {
    switch (RetCode) {
    case VarCheckConflict:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_FLAG_CONFLICT), HiiHandle, L"ipconfig", Useful);
      break;

    case VarCheckDuplicate:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_DUP_FLAG), HiiHandle, L"ipconfig", Useful);
      break;

    case VarCheckLackValue:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_LACK_ARG), HiiHandle, L"ipconfig", Useful);
      break;

    case VarCheckUnknown:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_UNKNOWN_FLAG), HiiHandle, L"ipconfig", Useful);
      break;

    default:
      break;
    }

    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  if (LibCheckVarGetFlag (&ChkPck, L"-b")) {
    EnablePageBreak (DEFAULT_INIT_ROW, DEFAULT_AUTO_LF);
  }

  if (LibCheckVarGetFlag (&ChkPck, L"-?")) {
    if (ChkPck.ValueCount > 0 ||
        ChkPck.FlagCount > 2 ||
        (2 == ChkPck.FlagCount && !LibCheckVarGetFlag (&ChkPck, L"-b"))
        ) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiHandle, L"ipconfig");
      Status = EFI_INVALID_PARAMETER;
    } else {
      PrintToken (STRING_TOKEN (STR_IPCONFIG_VERBOSEHELP), HiiHandle);
      Status = EFI_SUCCESS;
    }

    goto Done;
  }

  if (ChkPck.ValueCount > 1) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiHandle, L"ipconfig");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }
  //
  // Validate arguments
  //
  if (!LibCheckVarGetFlag (&ChkPck, L"-r")) {
    Item = GetFirstArg (&ChkPck);
    if (Item) {
      Status = EfiInetAddr (Item->VarStr, &IpAddress);
      if (EFI_ERROR (Status)) {
        PrintToken (STRING_TOKEN (STR_IPCONFIG_INVALID_ARG), HiiHandle, Item->VarStr);
        goto Done;
      }

      NewIp = TRUE;
    }

    Item = LibCheckVarGetFlag (&ChkPck, L"-m");
    if (Item) {
      Status = EfiInetAddr (Item->VarStr, &IpMask);
      if (EFI_ERROR (Status)) {
        PrintToken (STRING_TOKEN (STR_IPCONFIG_INVALID_ARG), HiiHandle, Item->VarStr);
        goto Done;
      }

      Mask = 0;
      for (Index = 0; Index < 4; Index++) {
        Mask |= (IpMask.v4.Addr[3 - Index] << (Index * 8));
      }

      for (Bits = 0; Bits < 32; Bits++) {
        if (!TurnFlag && (Mask & (1 << Bits))) {
          TurnFlag = TRUE;
        }

        if (TurnFlag && !(Mask & (1 << Bits))) {
          PrintToken (STRING_TOKEN (STR_IPCONFIG_INVALID_ARG), HiiHandle, Item->VarStr);
          Status = EFI_INVALID_PARAMETER;
          goto Done;
        }
      }

      NewMsk = TRUE;
    }
  }

  Instance  = 0;
  Item      = LibCheckVarGetFlag (&ChkPck, L"-c");
  if (Item) {
    Instance = (UINTN) StrToUInteger (Item->VarStr, &Status);
    if (EFI_ERROR (Status)) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_ARG), HiiHandle, L"ipconfig", Item->VarStr);
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }
  }
  //
  // Get MAC address from simple network protocol
  //
  Status = LibLocateHandle (
            ByProtocol,
            &gEfiSimpleNetworkProtocolGuid,
            NULL,
            &Count,
            &HandleBuffer
            );

  if (EFI_ERROR (Status) || (Count == 0)) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_LOC_PROT_ERR_EX), HiiHandle, L"ipconfig", L"Simple network protocol");
    Status = EFI_NOT_FOUND;
    goto Done;
  }

  if (Instance >= Count) {
    PrintToken (STRING_TOKEN (STR_IPCONFIG_INS_NOT_SUPPORT), HiiHandle, Instance);
    Status = EFI_NOT_AVAILABLE_YET;
    FreePool (HandleBuffer);
    goto Done;
  }
  //
  // Get SNP Protocol
  //
  Status = BS->HandleProtocol (
                HandleBuffer[Instance],
                &gEfiSimpleNetworkProtocolGuid,
                (VOID **) &SimpleNetwork
                );
  if (EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_LOC_PROT_ERR_EX), HiiHandle, L"ipconfig", L"Simple network protocol");
    FreePool (HandleBuffer);
    goto Done;
  }

  PrintToken (STRING_TOKEN (STR_IPCONFIG_MAC_ADDRESS), HiiHandle);
  Ptr = (UINT8 *) (UINTN) &SimpleNetwork->Mode->CurrentAddress.Addr;
  for (Index = 0; Index < IPV4_MAC_ADDRESS_LEN; Index += 1) {
    PrintToken (STRING_TOKEN (STR_IPCONFIG_ONE_VAR), HiiHandle, Ptr[Index]);
  }

  Print (L"\n");

  PrintToken (STRING_TOKEN (STR_IPCONFIG_BROADCAST), HiiHandle);
  Ptr = (UINT8 *) (UINTN) &SimpleNetwork->Mode->BroadcastAddress.Addr;
  for (Index = 0; Index < IPV4_MAC_ADDRESS_LEN; Index += 1) {
    PrintToken (STRING_TOKEN (STR_IPCONFIG_ONE_VAR), HiiHandle, Ptr[Index]);
  }

  Print (L"\n");

  //
  // Get Pxe Protocol
  //
  Status = BS->HandleProtocol (
                HandleBuffer[Instance],
                &gEfiPxeBaseCodeProtocolGuid,
                (VOID **) &PxeBaseCode
                );
  if (EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_LOC_PROT_ERR_EX), HiiHandle, L"ipconfig", L"PXE base code protocol");
    FreePool (HandleBuffer);
    goto Done;
  }
  //
  // Get DHCP4 protocol
  //
  Status = BS->HandleProtocol (
                HandleBuffer[Instance],
                &gEfiPxeDhcp4ProtocolGuid,
                (VOID **) &PxeDhcp4
                );
  if (EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_LOC_PROT_ERR_EX), HiiHandle, L"ipconfig", L"PXE DHCP4 protocol");
    FreePool (HandleBuffer);
    goto Done;;
  }

  Item = LibCheckVarGetFlag (&ChkPck, L"-r");
  if (Item) {
    //
    // Restart the PXE Base Code DHCP
    //
    Status = EfiRestartPxeBaseCode (PxeBaseCode, PxeDhcp4);
    if (EFI_ERROR (Status)) {
      FreePool (HandleBuffer);
      Status = EFI_NOT_STARTED;
      goto Done;
    }
  } else {
    if (!NewIp) {
      CopyMem (&IpAddress, &PxeBaseCode->Mode->StationIp, sizeof (EFI_IP_ADDRESS));
    }

    if (!NewMsk) {
      CopyMem (&IpMask, &PxeBaseCode->Mode->SubnetMask, sizeof (EFI_IP_ADDRESS));
    }
    //
    // call Pxe base code protocol to set station IP address
    //
    Status = PxeBaseCode->SetStationIp (
                            PxeBaseCode,
                            &IpAddress,
                            &IpMask
                            );

    if (EFI_ERROR (Status)) {
      PrintToken (STRING_TOKEN (STR_IPCONFIG_SET_IP), HiiHandle, Status);
      FreePool (HandleBuffer);
      Status = EFI_ABORTED;
      goto Done;
    }
  }

  PrintToken (
    STRING_TOKEN (STR_IPCONFIG_IP_ADDRESS),
    HiiHandle,
    PxeBaseCode->Mode->StationIp.v4.Addr[0],
    PxeBaseCode->Mode->StationIp.v4.Addr[1],
    PxeBaseCode->Mode->StationIp.v4.Addr[2],
    PxeBaseCode->Mode->StationIp.v4.Addr[3]
    );
  PrintToken (
    STRING_TOKEN (STR_IPCONFIG_IP_MASK),
    HiiHandle,
    PxeBaseCode->Mode->SubnetMask.v4.Addr[0],
    PxeBaseCode->Mode->SubnetMask.v4.Addr[1],
    PxeBaseCode->Mode->SubnetMask.v4.Addr[2],
    PxeBaseCode->Mode->SubnetMask.v4.Addr[3]
    );

  FreePool (HandleBuffer);

Done:
  LibCheckVarFreeVarList (&ChkPck);
  LibUnInitializeStrings ();
  return EFI_SUCCESS;
}

EFI_STATUS
EfiInetAddr (
  IN  CHAR16            *Ptr,
  OUT EFI_IP_ADDRESS    *Ip
  )
/*++
Routine Description:

  Converts a string containing an (Ipv4) Internet Protocol dotted address into
  a proper address for the IN_ADDR structure.

Arguments:

  Ptr         -  [in] Null-terminated character string representing a number 
                 expressed in the Internet standard "." (dotted) notation. 
  IpAddress   -  The Ip address structure to contain result. 

Returns:

  EFI_SUCCESS           - The command completed successfully
  EFI_INVALID_PARAMETER - Invalid parameter

--*/
{
  UINTN           Index;
  UINT64          Value;
  CHAR16          *NumPtr;
  CHAR16          *BreakPtr;
  CHAR16          ch;
  EFI_IP_ADDRESS  IpAddress;
  EFI_STATUS      Status;

  //
  // reset to zero first
  //
  for (Index = 0; Index < EFI_IP_V4_BYTES; Index += 1) {
    IpAddress.v4.Addr[Index] = 0;
  }

  Index     = 0;
  NumPtr    = Ptr;
  BreakPtr  = Ptr;
  for (;;) {
    while (*BreakPtr && L'.' != *BreakPtr) {
      if (*BreakPtr < L'0' || *BreakPtr > L'9') {
        return EFI_INVALID_PARAMETER;
      }

      BreakPtr++;
    }

    if (BreakPtr - NumPtr < 1 || BreakPtr - NumPtr > 3) {
      return EFI_INVALID_PARAMETER;
    }

    ch        = *BreakPtr;
    *BreakPtr = 0;
    Value     = StrToUInteger (NumPtr, &Status);
    *BreakPtr = ch;
    if (EFI_ERROR (Status) || Value > 255) {
      return EFI_INVALID_PARAMETER;
    }

    IpAddress.v4.Addr[Index] = (UINT8) Value;

    if (Index >= 3 || 0 == *BreakPtr) {
      break;
    }

    BreakPtr++;
    NumPtr = BreakPtr;
    Index++;
  }

  if (Index != 3 || *BreakPtr) {
    return EFI_INVALID_PARAMETER;
  }
  //
  // Return the IP value
  //
  for (Index = 0; Index < EFI_IP_V4_BYTES; Index += 1) {
    Ip->v4.Addr[Index] = IpAddress.v4.Addr[Index];
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EfiRestartPxeBaseCode (
  EFI_PXE_BASE_CODE_PROTOCOL   *PxeBaseCode,
  EFI_PXE_DHCP4_PROTOCOL       *PxeDhcp4
  )
/*++
Routine Description:

  Restart the PXE base code DHCP service.

Arguments:

  PxeBaseCode   -  The Pointer to PXE base code protocol. 
  PxeDhcp4      -  The pointer to PxeDhcp4 protocol.
Returns:

  EFI_SUCCESS           - The command completed successfully
  EFI_INVALID_PARAMETER - Invalid parameter

--*/
{
  EFI_STATUS  Status;
  DHCP4_OP    *SubnetMaskPtr;
  DHCP4_OP    *DhcpOp;

  if (PxeBaseCode == NULL) {
    PrintToken (STRING_TOKEN (STR_IPCONFIG_INVALID_PXE), HiiHandle);
    return EFI_INVALID_PARAMETER;
  }
  //
  // If PXE base code has started, stop it.
  //
  if (PxeBaseCode->Mode->Started) {
    Status = PxeBaseCode->Stop (PxeBaseCode);
    if (EFI_ERROR (Status)) {
      PrintToken (STRING_TOKEN (STR_IPCONFIG_STOP_PXE), HiiHandle, Status);
    }
  }
  //
  // Start DHCP, UseIpv6 == FALSE
  //
  Status = PxeBaseCode->Start (
                          PxeBaseCode,
                          FALSE
                          );

  if (EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_IPCONFIG_START_PXE), HiiHandle, Status);
  }
  //
  // Attempts to complete a DHCPv4 D.O.R.A.
  // (discover / offer / request / acknowledge)
  // SortOffses == TRUE, Sorts the  received offers.
  //
  Status = PxeDhcp4->Run (PxeDhcp4, 0, NULL);

  if (EFI_ERROR (Status)) {
    if (Status != EFI_ALREADY_STARTED) {
      PrintToken (STRING_TOKEN (STR_IPCONFIG_DHCP), HiiHandle, Status);
      return Status;
    }
  }

  if (!PxeDhcp4->Data->SetupCompleted ||
      !PxeDhcp4->Data->InitCompleted ||
      !PxeDhcp4->Data->SelectCompleted ||
      !PxeDhcp4->Data->IsAck
      ) {
    PrintToken (STRING_TOKEN (STR_IPCONFIG_DHCP), HiiHandle, Status);
    return Status;
  }
  //
  // Locate subnet mask inside DHCP options.
  //
  SubnetMaskPtr = FindDhcp4Option (
                    DHCP4_SUBNET_MASK,
                    0,
                    &PxeDhcp4->Data->AckNak
                    );

  if (SubnetMaskPtr == NULL) {
    PrintToken (STRING_TOKEN (STR_IPCONFIG_DHCP), HiiHandle, Status);
    return Status;
  }
  //
  // Set station IP information in PxeBc protocol.
  //
  Status = PxeBaseCode->SetStationIp (
                          PxeBaseCode,
                          (EFI_IP_ADDRESS *) &PxeDhcp4->Data->AckNak.dhcp4.yiaddr,
                          (EFI_IP_ADDRESS *) SubnetMaskPtr->data
                          );

  if (EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_IPCONFIG_DHCP), HiiHandle, Status);
    return Status;
  }

  DhcpOp = FindDhcp4Option (DHCP4_ROUTER_LIST, 0, &PxeDhcp4->Data->AckNak);
  if ((!EFI_ERROR (Status)) && (DhcpOp->len != 0) && (PxeBaseCode->Mode->RouteTableEntries == 0)) {
    PxeBaseCode->Mode->RouteTableEntries++;
    CopyMem (
      &(PxeBaseCode->Mode->RouteTable[PxeBaseCode->Mode->RouteTableEntries - 1].GwAddr),
      DhcpOp->data,
      4
      );
  }

  return EFI_SUCCESS;
}

EFI_STATUS
InitializeIpConfigGetLineHelp (
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
  return LibCmdGetStringByToken (STRING_ARRAY_NAME, &EfiIpConfigGuid, STRING_TOKEN (STR_IPCONFIG_LINEHELP), Str);
}
