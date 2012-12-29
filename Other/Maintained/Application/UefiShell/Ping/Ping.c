/*++

Copyright (c) 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  Ping.c

Abstract:

  Implementation of shell application Ping.

--*/

#include "EfiShellLib.h"
#include "CpuFuncs.h"
#include EFI_ARCH_PROTOCOL_DEFINITION (Cpu)
#include EFI_PROTOCOL_DEFINITION (Ip4)
#include "Ping.h"
#include STRING_DEFINES_FILE
extern UINT8 STRING_ARRAY_NAME[];

EFI_HII_HANDLE  HiiHandle;

EFI_GUID  EfiPingGuid = EFI_PING_GUID;
SHELL_VAR_CHECK_ITEM  PingCheckList[] = {
  {
    L"-n",
    0x1,
    0x6,
    FlagTypeNeedVar
  },
  {
    L"-l",
    0x2,
    0x5,
    FlagTypeNeedVar
  },
  {
    L"-?",
    0x4,
    0x3,
    FlagTypeSingle
  },
  {
    NULL,
    0,
    0,
    0
  }
};

EFI_IPv4_ADDRESS  DestinationIp;
EFI_LIST_ENTRY    IcmpTxTimeoutList;
UINT16            Identifier;
UINT32            RxCount;
UINT64            Frequency;
UINT32            RttSum;
UINT32            RttMin;
UINT32            RttMax;

STATIC
UINT64
GetTimerValue (
  VOID
  )
/*++

Routine Description:

  Get the current timestamp.

Arguments:

  None.

Returns:

  The current timestamp.

--*/
{
  return EfiReadTsc ();
}

STATIC
EFI_STATUS
GetFrequency (
  VOID
  )
/*++

Routine Description:

  Calculate the frequency.

Arguments:

  None.

Returns:

  EFI_SUCCESS - The frequency is calculated.
  other       - Some error occurs.

--*/
{
  EFI_STATUS             Status;
  EFI_CPU_ARCH_PROTOCOL  *Cpu;
  UINT64                 CurrentTick;
  UINT64                 TimerPeriod;

  //
  // Locate the Cpu Arch Protocol.
  //
  Status = BS->LocateProtocol (&gEfiCpuArchProtocolGuid, NULL, &Cpu);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = Cpu->GetTimerValue (Cpu, 0, &CurrentTick, &TimerPeriod);
  if (EFI_ERROR (Status)) {
    //
    // The WinntGetTimerValue will return EFI_UNSUPPORTED. Set the
    // TimerPeriod by ourselves.
    //
    TimerPeriod = 1000000;
  }

  //
  // The TimerPeriod is in femtosecond (1 femtosecond is  1e-15 second), so 1e+12
  // is divided by TimerPeriod to produce the Feq in tick/ms.
  //
  Frequency = DivU64x32 (1000000000000, (UINTN) TimerPeriod, NULL);

  return EFI_SUCCESS;
}

STATIC
UINT32
CalculateTime (
  UINT64  StartTick,
  UINT64  EndTick
  )
/*++

Routine Description:

  Caculate the time for the period from StartTick to EndTick, the result is 
  in millisencod.

Arguments:

  StartTick - The value of the start tick.
  EndTick   - The value of the end tick.

Returns:

  The result time period, in millisecond.

--*/
{
  UINT32  ElapsedTime;

  //
  // ElapsedTime is in ms.
  //
  ElapsedTime = (UINT32) DivU64x32 (EndTick - StartTick, (UINTN) (Frequency), NULL);

  return ElapsedTime;
}

STATIC
EFI_STATUS
ParseIpAddress (
  IN     CHAR16            *IpStr,
  IN OUT EFI_IPv4_ADDRESS  *IpAddress
  )
/*++

Routine Description:

  Parse the IPv4 address in the string pointed to by IpStr and save the result
  in the IpAddress.

Arguments:

  IpStr     - Pointer to the string containing the IPv4 address.
  IpAddress - Pointer to the storage for the parsed IPv4 address.

Returns:

  EFI_SUCCESS           - The IPv4 address string is parsed and the result
                          is in IpAddress.
  EFI_INVALID_PARAMETER - The IPv4 address the string represents is in a wrong format.

--*/
{
  UINTN   Addr;
  UINT32  Index;

  for (Index = 0; Index < 4; Index++) {

    Addr = 0;

    for (; (*IpStr != L'.') && (*IpStr != L'\0'); IpStr++) {

      if ((*IpStr < L'0') || (*IpStr > L'9')) {
        return EFI_INVALID_PARAMETER;       
      }

      Addr = Addr * 10 + (*IpStr - L'0');
    }

    if (Addr > 255) {
      return EFI_INVALID_PARAMETER;
    }

    IpAddress->Addr[Index] = (UINT8) Addr;

    if (*IpStr == L'\0') {
      break;
    }

    //
    // Skip the dot '.'
    //
    IpStr++;
  }

  //
  // All four digits of an IPv4 address are here and no charaters left?
  //
  if ((Index < 3) || (*IpStr != L'\0')) {

    return EFI_INVALID_PARAMETER;
  }

  return EFI_SUCCESS;
}

STATIC
UINT16
NetAddChecksum (
  IN UINT16 ChecksumA,
  IN UINT16 ChecksumB
  )
/*++

Routine Description:

  Add two checksums.

Arguments:

  Checksum1 - The first checksum to be added.
  Checksum2 - The second checksum to be added.

Returns:

  The new checksum.

--*/
{
  UINT32  Sum;

  Sum = ChecksumA + ChecksumB;

  //
  // two UINT16 can only add up to a carry of 1.
  //
  if (Sum >> 16) {
    Sum = (Sum & 0xffff) + 1;
  }

  return (UINT16) Sum;
}

UINT16
NetChecksum (
  IN UINT8   *Buffer,
  IN UINT32  Length
  )
/*++

Routine Description:

  Calculate the internet checksum (see RFC 1071)

Arguments:

  Packet             - Buffer which contains the data to be checksummed
  Length             - Length to be checksummed

Returns:

  Checksum           - Returns the 16 bit ones complement of 
                       ones complement sum of 16 bit words

--*/
{
  UINT32  Sum;
  UINT8   Odd;
  UINT16  *Packet;

  Packet  = (UINT16 *) Buffer;

  Sum     = 0;
  Odd     = (UINT8) (Length & 1);
  Length >>= 1;
  while (Length--) {
    Sum += *Packet++;
  }

  if (Odd) {
    Sum += *(UINT8 *) Packet;
  }

  Sum = (Sum & 0xffff) + (Sum >> 16);

  //
  // in case above carried
  //
  Sum += Sum >> 16;

  return (UINT16) Sum;
}

STATIC
VOID
EFIAPI
IcmpEchoRequestSent (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
/*++

Routine Description:

  The callback function of the ICMP transmit event.

Arguments:

  Event   - The event this function is registerd to.
  Context - The context data registerd to the event.

Returns:

  None.

--*/
{
}

STATIC
EFI_STATUS
AddTxInfo (
  IN UINT16  SeqNum,
  IN UINT32  SentTime
  )
/*++

Routine Description:

  Add a ICMP_TX_INFO into the transmit timeout list.

Arguments:

  SeqNum   - The sequence number of the ICMP echo request.
  SentTime - The time this ICMP echo request is sent.

Returns:

  EFI_SUCCESS          - The ICMP_TX_INFO is added into the transmit timeout list.
  EFI_OUT_OF_RESOURCES - No available memory for the ICMP_TX_INFO.

--*/
{
  ICMP_TX_INFO  *TxInfo;

  TxInfo = (ICMP_TX_INFO *) AllocatePool (sizeof (ICMP_TX_INFO));
  if (TxInfo == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  InitializeListHead (&TxInfo->Link);
  TxInfo->SeqNum   = SeqNum;
  TxInfo->SentTime = SentTime;

  InsertTailList (&IcmpTxTimeoutList, &TxInfo->Link);

  return EFI_SUCCESS;
}

STATIC
VOID
RemoveTxInfo (
  IN UINT16 SeqNum
  )
/*++

Routine Description:

  Remove a ICMP_TX_INFO from the transmit timeout list according to the SeqNum.

Arguments:

  SeqNum - The sequence number of the ICMP echo request.

Returns:

  None.

--*/
{
  EFI_LIST_ENTRY  *Node;
  ICMP_TX_INFO    *TxInfo;

  for (Node = IcmpTxTimeoutList.Flink; Node != &IcmpTxTimeoutList; Node = Node->Flink) {

    TxInfo = _CR (Node, ICMP_TX_INFO, Link);

    if (TxInfo->SeqNum == SeqNum) {
      RemoveEntryList (Node);
      FreePool (TxInfo);
      break;
    }
  }
}

STATIC
VOID
CheckTimeout (
  IN UINT32  ElapsedSeconds
  )
/*++

Routine Description:

  Check the timeout of the transmitted ICMP echo request in the timeout list.

Arguments:

  ElapsedSeconds - The seconds elapsed since the first ICMP echo request is sent.

Returns:

  None.

--*/
{
  EFI_LIST_ENTRY  *Node;
  EFI_LIST_ENTRY  *NextNode;
  ICMP_TX_INFO    *TxInfo;

  for (Node = IcmpTxTimeoutList.Flink; Node != &IcmpTxTimeoutList; Node = NextNode) {

    NextNode = Node->Flink;

    TxInfo = _CR (Node, ICMP_TX_INFO, Link);

    if (ElapsedSeconds - TxInfo->SentTime >= ICMP_DEFAULT_TIMEOUT) {
      //
      // Timeout, print the message.
      //
      PrintToken (STRING_TOKEN (STR_PING_TIMEOUT), HiiHandle);
      RemoveEntryList (Node);
      FreePool (TxInfo);
    } else {
      break;
    }
  }
}

STATIC
VOID
CleanTxTimeoutList (
  VOID
  )
/*++

Routine Description:

  Clean the timeout list.

Arguments:

  None.

Returns:

  None.

--*/
{
  EFI_LIST_ENTRY  *Node;
  ICMP_TX_INFO    *TxInfo;

  while (!IsListEmpty (&IcmpTxTimeoutList)) {

    Node   = IcmpTxTimeoutList.Flink;
    TxInfo = _CR (Node, ICMP_TX_INFO, Link);

    RemoveEntryList (Node);
    FreePool (TxInfo);
  }
}

STATIC
VOID
HandleIcmpEchoReply (
  IN EFI_IP4_RECEIVE_DATA  *RxData
  )
/*++

Routine Description:

  Handle the received ICMP packet.

Arguments:

  RxData - Pointer to the EFI_IP4_RECEIVE_DATA passed up by Ip4 protocol.

Returns:

  None.

--*/
{
  EFI_ICMP_ECHO_REPLY  *IcmpEchoReply;
  UINT64               CurrentTick;
  UINT32               Rtt;
  CHAR16               Notation;

  ASSERT (RxData != NULL);

  if ((RxData->Header->Protocol != EFI_IP4_PROTO_ICMP) ||
    (RxData->DataLength < sizeof (EFI_ICMP_ECHO_REPLY))) {
    //
    // Sanity checks.
    //
    return;
  }

  if (!EFI_IP4_EQUAL (RxData->Header->SourceAddress, DestinationIp)) {
    //
    // The source address of this ICMP packet is different from the DestinationIp.
    //
    return;
  }

  IcmpEchoReply = (EFI_ICMP_ECHO_REPLY *) RxData->FragmentTable[0].FragmentBuffer;

  if ((IcmpEchoReply->Type != ICMP_TYPE_ECHO_REPLY) || (IcmpEchoReply->Code != 0)) {
    //
    // It's not an ICMP echo reply packet.
    //
    return;
  }

  if (IcmpEchoReply->Identifier != Identifier) {
    //
    // The Identifier differs.
    //
    return;
  }

  //
  // Remove the TxInfo from the timeout list.
  //
  RemoveTxInfo (IcmpEchoReply->SeqNum);

  RxCount++;

  //
  // Calculate the RTT.
  //
  CurrentTick = GetTimerValue ();
  Rtt         = CalculateTime (IcmpEchoReply->TimerValue, CurrentTick);

  RttSum += Rtt;

  if (Rtt < RttMin) {
    RttMin = Rtt;
  }

  if (Rtt > RttMax) {
    RttMax = Rtt;
  }

  if (Rtt == 0) {
    Notation = L'<';
    Rtt = 1;
  } else {
    Notation = L'=';
  }

  //
  // Print the message, such as RTT, TTL...
  //
  PrintToken (
    STRING_TOKEN (STR_PING_REPLY_INFO),
    HiiHandle,
    RxData->Header->SourceAddress.Addr[0],
    RxData->Header->SourceAddress.Addr[1],
    RxData->Header->SourceAddress.Addr[2],
    RxData->Header->SourceAddress.Addr[3],
    RxData->DataLength - sizeof (EFI_ICMP_ECHO_REPLY) + sizeof (IcmpEchoReply->TimerValue),
    Notation,
    Rtt,
    RxData->Header->TimeToLive
    );
}

STATIC
VOID
EFIAPI
IcmpEchoReplyRcvd (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
/*++

Routine Description:

  The callback function of the ICMP receive event.

Arguments:

  Event   - The event this function is registered to.
  Context - The context registered to this event.

Returns:

  None.

--*/
{
}

STATIC
BOOLEAN
AbortKey (
  VOID
  )
/*++

Routine Description:

  Check whether there is abort key input from the keyboard.

Arguments:

  None

Returns:

  TRUE if some abort key is deteced, otherwise FALSE.

--*/
{
  EFI_INPUT_KEY  Key;
  EFI_STATUS     Status;

  //
  // Check for user intervention.
  //
  while (TRUE) {

    Status = ST->ConIn->ReadKeyStroke (ST->ConIn, &Key);

    if (EFI_ERROR (Status)) {
      return FALSE;
    }

    switch (Key.UnicodeChar) {
      case 0x1B:
        //
        // <Esc>
        //
      case 0x03:
        //
        // <Ctrl+C>
        //
        return TRUE;

      case 0:
        if (Key.ScanCode == 0x17) {
          return TRUE;
        }

      default:
        break;
    }
  }
}

STATIC
BOOLEAN
GetMapping (
  IN EFI_IP4_PROTOCOL  *Ip4
  )
/*++

Routine Description:

  Get available address configuration.

Arguments:

  Ip4 - Pointer to the Ip4 protocol.

Returns:

  TRUE if mapping is done, otherwise FALSE.

--*/
{
  EFI_STATUS         Status;
  EFI_EVENT          TimeoutEvent;
  EFI_IP4_MODE_DATA  Ip4ModeData;
  BOOLEAN            IsMapDone;

  IsMapDone = FALSE;

  Status = BS->CreateEvent (EFI_EVENT_TIMER, EFI_TPL_CALLBACK, NULL, NULL, &TimeoutEvent);
  if (EFI_ERROR (Status)) {
    return IsMapDone;
  }

  //
  // Start the timer, it will timeout after 5 seconds.
  //
  BS->SetTimer (TimeoutEvent, TimerRelative, 50000000);

  while (EFI_ERROR (BS->CheckEvent (TimeoutEvent))) {

    Ip4->Poll (Ip4);

    Ip4->GetModeData (Ip4, &Ip4ModeData, NULL, NULL);

    if (Ip4ModeData.IsConfigured) {

      IsMapDone = TRUE;
      break;
    }
  }

  BS->CloseEvent (TimeoutEvent);

  return IsMapDone;
}

STATIC
EFI_STATUS
OpenIp4Protocol (
  IN  EFI_HANDLE        ImageHandle,
  IN  EFI_HANDLE        *ChildHandle,
  OUT EFI_IP4_PROTOCOL  **Ip4Proto
  )
/*++

Routine Description:

  Create an Ip4 child and open the Ip4 protocol.

Arguments:

  ImageHandle - The image handle.
  ChildHandle - Pointer to the child handle.
  Ip4Proto    - Pointer to the Ip4 protocol.

Returns:

  EFI_SUCCESS - The ip4 protocol is opened.
  other       - Some error occurs.

--*/
{
  EFI_STATUS                    Status;
  EFI_SERVICE_BINDING_PROTOCOL  *Ip4Sb;

  Status = BS->LocateProtocol (&gEfiIp4ServiceBindingProtocolGuid, NULL, &Ip4Sb);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = Ip4Sb->CreateChild (Ip4Sb, ChildHandle);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = BS->OpenProtocol (
                *ChildHandle,
                &gEfiIp4ProtocolGuid,
                (VOID **) Ip4Proto,
                ImageHandle,
                *ChildHandle,
                EFI_OPEN_PROTOCOL_GET_PROTOCOL
                );
  if (EFI_ERROR (Status)) {
    Ip4Sb->DestroyChild (Ip4Sb, *ChildHandle);
  }

  return Status;
}

STATIC
VOID
CloseIp4Protocol (
  IN EFI_HANDLE ImageHandle,
  IN EFI_HANDLE ChildHandle
  )
/*++

Routine Description:

  Close the Ip4 protocol and destroy the Ip4 child.

Arguments:

  ImageHandle - The image handle.
  ChildHandle - The child handle.

Returns:

  None.

--*/
{
  EFI_SERVICE_BINDING_PROTOCOL  *Ip4Sb;

  BS->CloseProtocol (
        ChildHandle,
        &gEfiIp4ProtocolGuid,
        ImageHandle,
        ChildHandle
        );

  BS->LocateProtocol (&gEfiIp4ServiceBindingProtocolGuid, NULL, &Ip4Sb);

  Ip4Sb->DestroyChild (Ip4Sb, ChildHandle);
}

STATIC
EFI_STATUS
PingMainProc (
  IN EFI_HANDLE  ImageHandle,
  IN UINT32      SendNumber,
  IN UINT32      BufferSize
  )
/*++

Routine Description:

  The main procedure of the Ping.

Arguments:

  ImageHandle - The image handle.
  SendNumber  - The required counts of the ICMP echo request packets.
  BufferSize  - The buffer size of the ICMP echo request packets.

Returns:

  EFI_SUCCESS - Ping successfully finished.
  other       - Some error occurs.

--*/
{
  EFI_STATUS                    Status;
  EFI_IP4_PROTOCOL              *Ip4Proto;
  EFI_HANDLE                    Ip4ChildHandle;
  UINT64                        CurrentTick;
  EFI_EVENT                     SendTimer;
  EFI_IP4_COMPLETION_TOKEN      RxToken;
  EFI_IP4_CONFIG_DATA           Ip4ConfigData;
  EFI_IP4_COMPLETION_TOKEN      TxToken;
  EFI_IP4_TRANSMIT_DATA         TxData;
  UINT32                        TxBufferSize;
  EFI_ICMP_ECHO_REQUEST         *IcmpEchoRequest;
  UINT16                        HeadSum;
  UINT16                        TempChecksum;
  UINT32                        TxCount;
  UINT32                        ElapsedSeconds;

  //
  // Initialize the variables.
  //
  InitializeListHead (&IcmpTxTimeoutList);
  RxCount               = 0;
  RttSum                = 0;
  RttMin                = (UINT32) -1;
  RttMax                = 0;
  Ip4ChildHandle        = NULL;
  SendTimer             = NULL;
  TxToken.Event         = NULL;
  RxToken.Event         = NULL;
  RxToken.Packet.RxData = NULL;
  IcmpEchoRequest       = NULL;
  ElapsedSeconds        = 0;

  Status = GetFrequency ();
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Open the Ip4 protocol.
  //
  Status = OpenIp4Protocol (ImageHandle, &Ip4ChildHandle, &Ip4Proto);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Set the configuration data.
  //
  Ip4ConfigData.DefaultProtocol   = EFI_IP4_PROTO_ICMP;
  Ip4ConfigData.AcceptAnyProtocol = FALSE;
  Ip4ConfigData.AcceptBroadcast   = FALSE;
  Ip4ConfigData.AcceptIcmpErrors  = TRUE;
  Ip4ConfigData.AcceptPromiscuous = FALSE;
  Ip4ConfigData.DoNotFragment     = FALSE;
  Ip4ConfigData.RawData           = FALSE;
  Ip4ConfigData.ReceiveTimeout    = 0;
  Ip4ConfigData.TransmitTimeout   = 0;
  Ip4ConfigData.UseDefaultAddress = TRUE;
  Ip4ConfigData.TimeToLive        = 128;
  Ip4ConfigData.TypeOfService     = 0;

  //
  // Configure the Ip4 protocol.
  //
  Status = Ip4Proto->Configure (Ip4Proto, &Ip4ConfigData);
  if (EFI_ERROR (Status) && (Status != EFI_NO_MAPPING)) {
    PrintToken (STRING_TOKEN (STR_PING_CONFIG_IP4), HiiHandle, Status);
    goto Exit;
  }

  if ((Status == EFI_NO_MAPPING) && !GetMapping (Ip4Proto)) {
    //
    // No available default address information.
    //
    PrintToken (STRING_TOKEN (STR_PING_MAPPING), HiiHandle);
    goto Exit;
  }

  PrintToken (
    STRING_TOKEN (STR_PING_START),
    HiiHandle,
    DestinationIp.Addr[0],
    DestinationIp.Addr[1],
    DestinationIp.Addr[2],
    DestinationIp.Addr[3],
    BufferSize
    );

  //
  // Create receive event
  //
  Status = BS->CreateEvent (
                EFI_EVENT_NOTIFY_SIGNAL,
                EFI_TPL_CALLBACK,
                IcmpEchoReplyRcvd,
                &RxToken,
                &RxToken.Event
                );
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  RxToken.Packet.RxData = NULL;
  RxToken.Status        = EFI_NOT_READY;

  //
  // Randomize the identifier first.
  //
  CurrentTick = GetTimerValue ();
  Identifier = (UINT16) ((((UINT32) CurrentTick) >> 8) + SendNumber);

  //
  // Issue the asynchronous receive request.
  //
  Status = Ip4Proto->Receive (Ip4Proto, &RxToken);
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  //
  // Create a timer.
  //
  Status = BS->CreateEvent (EFI_EVENT_TIMER, EFI_TPL_CALLBACK, NULL, NULL, &SendTimer);
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  //
  // Start the periodic timer, the interval is 1 second.
  //
  Status = BS->SetTimer (SendTimer, TimerPeriodic, 10000000);
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  //
  // Create the transmit event.
  //
  Status = BS->CreateEvent (
                 EFI_EVENT_NOTIFY_SIGNAL,
                 EFI_TPL_CALLBACK,
                 IcmpEchoRequestSent,
                 &TxToken,
                 &TxToken.Event
                 );
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  TxBufferSize    = sizeof (EFI_ICMP_ECHO_REQUEST) + BufferSize;
  IcmpEchoRequest = (EFI_ICMP_ECHO_REQUEST *) AllocatePool (TxBufferSize);

  IcmpEchoRequest->Type       = ICMP_TYPE_ECHO_REQUEST;
  IcmpEchoRequest->Code       = 0;
  IcmpEchoRequest->Checksum   = 0;
  IcmpEchoRequest->Identifier = Identifier;
  IcmpEchoRequest->SeqNum     = 0;
  IcmpEchoRequest->TimerValue = 0;

  HeadSum = NetChecksum ((UINT8 *) IcmpEchoRequest, TxBufferSize);

  IcmpEchoRequest->TimerValue = GetTimerValue();

  //
  // Calculate the final checksum and set the Checksum field.
  //
  TempChecksum = NetChecksum ((UINT8 *) &IcmpEchoRequest->TimerValue, sizeof (UINT64));
  IcmpEchoRequest->Checksum = ~NetAddChecksum (HeadSum, TempChecksum);

  TxData.DestinationAddress   = DestinationIp;
  TxData.OverrideData         = 0;
  TxData.OptionsLength        = 0;
  TxData.OptionsBuffer        = NULL;
  TxData.TotalDataLength      = TxBufferSize;
  TxData.FragmentCount        = 1;

  TxData.FragmentTable[0].FragmentBuffer = (VOID *) IcmpEchoRequest;
  TxData.FragmentTable[0].FragmentLength = TxBufferSize;

  TxToken.Packet.TxData = &TxData;
  TxToken.Status        = EFI_ABORTED;

  //
  // Send out the first ICMP echo packet.
  //
  Status = Ip4Proto->Transmit (Ip4Proto, &TxToken);
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  AddTxInfo (IcmpEchoRequest->SeqNum, ElapsedSeconds);

  TxCount = 1;

  while (!AbortKey ()) {
    Ip4Proto->Poll (Ip4Proto);

    if (BS->CheckEvent (SendTimer) == EFI_SUCCESS) {

      ElapsedSeconds++;

      if (TxCount < SendNumber) {

        if (TxToken.Status != EFI_SUCCESS) {
          //
          // If the previous transmit is not finished, cancel it first.
          //
          Ip4Proto->Cancel (Ip4Proto, &TxToken);
        }
        
        //
        // Increase the sequence number and get current tick.
        //
        IcmpEchoRequest->SeqNum++;
        IcmpEchoRequest->TimerValue = GetTimerValue ();

        //
        // Calculate the final checksum and update the checksum field.
        //
        TempChecksum = NetChecksum ((UINT8 *) &IcmpEchoRequest->TimerValue, sizeof (UINT64));
        TempChecksum = NetAddChecksum (TempChecksum, IcmpEchoRequest->SeqNum);
        IcmpEchoRequest->Checksum = ~NetAddChecksum (HeadSum, TempChecksum);

        TxToken.Status = EFI_ABORTED;

        //
        // Transmit a new ICMP echo packet.
        //
        Status = Ip4Proto->Transmit (Ip4Proto, &TxToken);
        if (EFI_ERROR (Status)) {
          break;
        }

        AddTxInfo (IcmpEchoRequest->SeqNum, ElapsedSeconds);

        TxCount++;
      }

      CheckTimeout (ElapsedSeconds);
    }

    if (RxToken.Status == EFI_SUCCESS) {
      //
      // An ICMP packet is received, process it.
      //
      HandleIcmpEchoReply (RxToken.Packet.RxData);

      BS->SignalEvent (RxToken.Packet.RxData->RecycleSignal);

      RxToken.Packet.RxData = NULL;
      RxToken.Status        = EFI_ABORTED;

      //
      // Continue receive the ICMP echo reply packets.
      //
      Status = Ip4Proto->Receive (Ip4Proto, &RxToken);
      if (EFI_ERROR (Status)) {
        break;
      }
    }

    if ((RxCount >= SendNumber) || (ElapsedSeconds + 1 - TxCount >= ICMP_DEFAULT_TIMEOUT)) {
      //
      // All echo replies are received or time out occurs.
      //
      break;
    }
  }

  BS->SetTimer (SendTimer, TimerCancel, 0);

  PrintToken (
    STRING_TOKEN (STR_PING_STAT_HEAD),
    HiiHandle,
    DestinationIp.Addr[0],
    DestinationIp.Addr[1],
    DestinationIp.Addr[2],
    DestinationIp.Addr[3]
    );

  PrintToken (
    STRING_TOKEN (STR_PING_STAT_DETAIL),
    HiiHandle,
    TxCount,
    RxCount,
    TxCount - RxCount,
    (100 * (TxCount - RxCount)) / TxCount
    );

  if (RxCount != 0) {
    PrintToken (STRING_TOKEN (STR_PING_RTT_HEAD), HiiHandle);
    PrintToken (STRING_TOKEN (STR_PING_RTT_DETAIL), HiiHandle, RttMin, RttMax, RttSum / RxCount);
  }
    
Exit:
  
  if (Ip4Proto != NULL) {
    //
    // Reset the Ip4 instance.
    //
    Ip4Proto->Configure (Ip4Proto, NULL);
  }

  //
  // Clean the transmit timeout list.
  //
  CleanTxTimeoutList ();

  if (RxToken.Packet.RxData != NULL) {
    //
    // Try to recycle the RxData if there is any.
    //
    BS->SignalEvent (RxToken.Packet.RxData->RecycleSignal);
  }

  if (RxToken.Event != NULL) {
    BS->CloseEvent (RxToken.Event);
  }

  if (TxToken.Event != NULL) {
    BS->CloseEvent (TxToken.Event);
  }

  if (SendTimer != NULL) {
    BS->CloseEvent (SendTimer);
  }

  if (IcmpEchoRequest != NULL) {
    FreePool (IcmpEchoRequest);
  }

  CloseIp4Protocol (ImageHandle, Ip4ChildHandle);

  return Status;
}

EFI_BOOTSHELL_CODE (EFI_APPLICATION_ENTRY_POINT (EfiMain))

EFI_STATUS
EFIAPI
EfiMain (
  IN  EFI_HANDLE        ImageHandle,
  IN  EFI_SYSTEM_TABLE  *SystemTable
  )
/*++

Routine Description:

  Common entry point.

Arguments:

  ImageHandle - The image handle. 
  SystemTable - The system table.

Returns:

  EFI_SUCCESS           - The command completed successfully.
  EFI_INVALID_PARAMETER - The command line arguments are invalid.

--*/
{
  EFI_STATUS                    Status;
  UINT64                        BufferSize;
  UINT64                        SendNumber;
  CHAR16                        *Useful;
  SHELL_ARG_LIST                *Item;
  SHELL_VAR_CHECK_CODE          RetCode;
  SHELL_VAR_CHECK_PACKAGE       ChkPck;

  //
  // We are now being installed as an internal command driver, initialize
  // as an nshell app and run
  //
  EFI_SHELL_APP_INIT (ImageHandle, SystemTable);

  //
  // Register our string package to HII database.
  //
  EFI_SHELL_STR_INIT (HiiHandle, STRING_ARRAY_NAME, EfiPingGuid);

  ZeroMem (&ChkPck, sizeof (SHELL_VAR_CHECK_PACKAGE));

  Status = EFI_INVALID_PARAMETER;
  LibFilterNullArgs ();
  RetCode = LibCheckVariables (SI, PingCheckList, &ChkPck, &Useful);
  if (VarCheckOk != RetCode) {
    switch (RetCode) {
    case VarCheckConflict:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_FLAG_CONFLICT), HiiHandle, L"Ping", Useful);
      break;

    case VarCheckDuplicate:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_DUP_FLAG), HiiHandle, L"Ping", Useful);
      break;

    case VarCheckLackValue:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_LACK_ARG), HiiHandle, L"Ping", Useful);
      break;

    case VarCheckUnknown:
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_UNKNOWN_FLAG), HiiHandle, L"Ping", Useful);
      break;

    default:
      break;
    }

    goto Done;
  }

  //
  // Output help.
  //
  if (LibCheckVarGetFlag (&ChkPck, L"-?") != NULL) {
    if (ChkPck.ValueCount > 0 ) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiHandle, L"Ping");
    } else {
      PrintToken (STRING_TOKEN (STR_PING_HELP), HiiHandle);
    }
    goto Done;
  }

  if (ChkPck.ValueCount != 1) {
    //
    // Only 1 value count is permitted, that is the TargetIp.
    //
    PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_ARG), HiiHandle, L"Ping", SI->Argv[0]);
    goto Done;
  }

  SendNumber = 10;
  BufferSize = 0;

  Item = LibCheckVarGetFlag (&ChkPck, L"-n");
  if (Item !=  NULL) {
    SendNumber = StrToUInteger (Item->VarStr, &Status);
    if (EFI_ERROR (Status)) {
      PrintToken (STRING_TOKEN (STR_PING_INVALID_COUNT), HiiHandle, Item->VarStr);
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }
  }

  Item = LibCheckVarGetFlag (&ChkPck, L"-l");
  if (Item != NULL) {
    BufferSize = StrToUInteger (Item->VarStr, &Status);
    if (EFI_ERROR (Status)) {
      PrintToken (STRING_TOKEN (STR_PING_INVALID_BUFFER_SIZE), HiiHandle, Item->VarStr);
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }

    if (BufferSize > PING_MAX_BUFFER_SIZE) {
      BufferSize = PING_MAX_BUFFER_SIZE;
    }
  }

  Status = ParseIpAddress (ChkPck.VarList->VarStr, &DestinationIp);
  if (EFI_ERROR (Status)) {
    PrintToken (STRING_TOKEN (STR_PING_INVALID_IP), HiiHandle, ChkPck.VarList->VarStr);
    goto Done;
  }
  
  Status = PingMainProc (
             ImageHandle,
             (UINT32) SendNumber,
             (UINT32) BufferSize
             );

Done:

  LibCheckVarFreeVarList (&ChkPck);

  return Status;
}
