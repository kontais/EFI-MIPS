/*++

Copyright (c) 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED. 

Module Name:

  Dhcp4Option.c

Abstract:

  Function to validate, parse, process the DHCP options
  
--*/

#include "Dhcp4Impl.h"

//
// A list of the format of DHCP Options sorted by option tag
// to validate a dhcp message. Refere the comments of the
// DHCP_OPTION_FORMAT structure.
//
STATIC
DHCP_OPTION_FORMAT 
DhcpOptionFormats [] = {
  {DHCP_TAG_NETMASK,        DHCP_OPTION_IP,     1, 1  , TRUE},
  {DHCP_TAG_TIME_OFFSET,    DHCP_OPTION_INT32,  1, 1  , FALSE},
  {DHCP_TAG_ROUTER,         DHCP_OPTION_IP,     1, -1 , TRUE},
  {DHCP_TAG_TIME_SERVER,    DHCP_OPTION_IP,     1, -1 , FALSE},
  {DHCP_TAG_NAME_SERVER,    DHCP_OPTION_IP,     1, -1 , FALSE},
  {DHCP_TAG_DNS_SERVER,     DHCP_OPTION_IP,     1, -1 , FALSE},
  {DHCP_TAG_LOG_SERVER,     DHCP_OPTION_IP,     1, -1 , FALSE},
  {DHCP_TAG_COOKIE_SERVER,  DHCP_OPTION_IP,     1, -1 , FALSE},
  {DHCP_TAG_LPR_SERVER,     DHCP_OPTION_IP,     1, -1 , FALSE},
  {DHCP_TAG_IMPRESS_SERVER, DHCP_OPTION_IP,     1, -1 , FALSE},
  {DHCP_TAG_RL_SERVER,      DHCP_OPTION_IP,     1, -1 , FALSE},
  {DHCP_TAG_HOSTNAME,       DHCP_OPTION_INT8,   1, -1 , FALSE},
  {DHCP_TAG_BOOTFILE_LEN,   DHCP_OPTION_INT16,  1, 1  , FALSE},
  {DHCP_TAG_DUMP,           DHCP_OPTION_INT8,   1, -1 , FALSE},
  {DHCP_TAG_DOMAINNAME,     DHCP_OPTION_INT8,   1, -1 , FALSE},
  {DHCP_TAG_SWAP_SERVER,    DHCP_OPTION_IP,     1, 1  , FALSE},
  {DHCP_TAG_ROOTPATH,       DHCP_OPTION_INT8,   1, -1 , FALSE},
  {DHCP_TAG_EXTEND_PATH,    DHCP_OPTION_INT8,   1, -1 , FALSE},
  
  {DHCP_TAG_IPFORWARD,      DHCP_OPTION_SWITCH, 1, 1  , FALSE},
  {DHCP_TAG_NONLOCAL_SRR,   DHCP_OPTION_SWITCH, 1, 1  , FALSE},
  {DHCP_TAG_POLICY_SRR,     DHCP_OPTION_IPPAIR, 1, -1 , FALSE},
  {DHCP_TAG_EMTU,           DHCP_OPTION_INT16,  1, 1  , FALSE},
  {DHCP_TAG_TTL,            DHCP_OPTION_INT8,   1, 1  , FALSE},
  {DHCP_TAG_PATHMTU_AGE,    DHCP_OPTION_INT32,  1, 1  , FALSE},
  {DHCP_TAG_PATHMTU_PLATEAU,DHCP_OPTION_INT16,  1, -1 , FALSE},
  
  {DHCP_TAG_IFMTU,          DHCP_OPTION_INT16,  1, 1  , FALSE},
  {DHCP_TAG_SUBNET_LOCAL,   DHCP_OPTION_SWITCH, 1, 1  , FALSE},
  {DHCP_TAG_BROADCAST,      DHCP_OPTION_IP,     1, 1  , FALSE},
  {DHCP_TAG_DISCOVER_MASK,  DHCP_OPTION_SWITCH, 1, 1  , FALSE},
  {DHCP_TAG_SUPPLY_MASK,    DHCP_OPTION_SWITCH, 1, 1  , FALSE},
  {DHCP_TAG_DISCOVER_ROUTE, DHCP_OPTION_SWITCH, 1, 1  , FALSE},
  {DHCP_TAG_ROUTER_SOLICIT, DHCP_OPTION_IP,     1, 1  , FALSE},
  {DHCP_TAG_STATIC_ROUTE,   DHCP_OPTION_IPPAIR, 1, -1 , FALSE},

  {DHCP_TAG_TRAILER,        DHCP_OPTION_SWITCH, 1, 1  , FALSE},
  {DHCP_TAG_ARPAGE,         DHCP_OPTION_INT32,  1, 1  , FALSE},
  {DHCP_TAG_ETHER_ENCAP,    DHCP_OPTION_SWITCH, 1, 1  , FALSE},
  
  {DHCP_TAG_TCP_TTL,        DHCP_OPTION_INT8,   1, 1  , FALSE},
  {DHCP_TAG_KEEP_INTERVAL,  DHCP_OPTION_INT32,  1, 1  , FALSE},
  {DHCP_TAG_KEEP_GARBAGE,   DHCP_OPTION_SWITCH, 1, 1  , FALSE},

  {DHCP_TAG_NIS_DOMAIN,     DHCP_OPTION_INT8,   1, -1 , FALSE},
  {DHCP_TAG_NIS_SERVER,     DHCP_OPTION_IP,     1, -1 , FALSE},
  {DHCP_TAG_NTP_SERVER,     DHCP_OPTION_IP,     1, -1 , FALSE},
  {DHCP_TAG_VENDOR,         DHCP_OPTION_INT8,   1, -1 , FALSE},
  {DHCP_TAG_NBNS,           DHCP_OPTION_IP,     1, -1 , FALSE},
  {DHCP_TAG_NBDD,           DHCP_OPTION_IP,     1, -1 , FALSE},
  {DHCP_TAG_NBTYPE,         DHCP_OPTION_INT8,   1, 1  , FALSE},
  {DHCP_TAG_NBSCOPE,        DHCP_OPTION_INT8,   1, -1 , FALSE},
  {DHCP_TAG_XFONT,          DHCP_OPTION_IP,     1, -1 , FALSE},
  {DHCP_TAG_XDM,            DHCP_OPTION_IP,     1, -1 , FALSE},

  {DHCP_TAG_REQUEST_IP,     DHCP_OPTION_IP,     1, 1  , FALSE},
  {DHCP_TAG_LEASE,          DHCP_OPTION_INT32,  1, 1  , TRUE},
  {DHCP_TAG_OVERLOAD,       DHCP_OPTION_INT8,   1, 1  , TRUE},
  {DHCP_TAG_TYPE,           DHCP_OPTION_INT8,   1, 1  , TRUE},
  {DHCP_TAG_SERVER_ID,      DHCP_OPTION_IP,     1, 1  , TRUE},
  {DHCP_TAG_PARA_LIST,      DHCP_OPTION_INT8,   1, -1 , FALSE},
  {DHCP_TAG_MESSAGE,        DHCP_OPTION_INT8,   1, -1 , FALSE},
  {DHCP_TAG_MAXMSG,         DHCP_OPTION_INT16,  1, 1  , FALSE},
  {DHCP_TAG_T1,             DHCP_OPTION_INT32,  1, 1  , TRUE},
  {DHCP_TAG_T2,             DHCP_OPTION_INT32,  1, 1  , TRUE},
  {DHCP_TAG_VENDOR_CLASS,   DHCP_OPTION_INT8,   1, -1 , FALSE},
  {DHCP_TAG_CLIENT_ID,      DHCP_OPTION_INT8,   2, -1 , FALSE},

  {DHCP_TAG_NISPLUS,        DHCP_OPTION_INT8,   1, -1 , FALSE},
  {DHCP_TAG_NISPLUS_SERVER, DHCP_OPTION_IP,     1, -1 , FALSE},

  {DHCP_TAG_TFTP,           DHCP_OPTION_INT8,   1, -1 , FALSE},
  {DHCP_TAG_BOOTFILE,       DHCP_OPTION_INT8,   1, -1 , FALSE},

  {DHCP_TAG_MOBILEIP,       DHCP_OPTION_IP,     0, -1 , FALSE},
  {DHCP_TAG_SMTP,           DHCP_OPTION_IP,     1, -1 , FALSE},
  {DHCP_TAG_POP3,           DHCP_OPTION_IP,     1, -1 , FALSE},
  {DHCP_TAG_NNTP,           DHCP_OPTION_IP,     1, -1 , FALSE},
  {DHCP_TAG_WWW,            DHCP_OPTION_IP,     1, -1 , FALSE},
  {DHCP_TAG_FINGER,         DHCP_OPTION_IP,     1, -1 , FALSE},
  {DHCP_TAG_IRC,            DHCP_OPTION_IP,     1, -1 , FALSE},
  {DHCP_TAG_STTALK,         DHCP_OPTION_IP,     1, -1 , FALSE}, 
  {DHCP_TAG_STDA,           DHCP_OPTION_IP,     1, -1 , FALSE},
  
  {DHCP_TAG_CLASSLESS_ROUTE,DHCP_OPTION_INT8,   5, -1 , FALSE},
};

STATIC
DHCP_OPTION_FORMAT *
DhcpFindOptionFormat (
  IN UINT8                  Tag
  )
/*++

Routine Description:

  Binary search the DhcpOptionFormats array to find the format 
  information about a specific option.

Arguments:

  Tag - The option's tag.

Returns:

  The point to the option's format, NULL if not found.

--*/
{
  INTN                      Left;
  INTN                      Right;
  INTN                      Middle;

  Left  = 0;
  Right = sizeof (DhcpOptionFormats) / sizeof (DHCP_OPTION_FORMAT) - 1;

  while (Right >= Left) {
    Middle = (Left + Right) / 2;
    
    if (Tag == DhcpOptionFormats[Middle].Tag) {
      return &DhcpOptionFormats[Middle];
    }

    if (Tag < DhcpOptionFormats[Middle].Tag) {
      Right = Middle - 1;
    } else {
      Left = Middle + 1;
    }
  }

  return NULL;
}

STATIC
BOOLEAN
DhcpOptionIsValid (
  IN DHCP_OPTION_FORMAT     *Format,
  IN UINT8                  *OptValue,
  IN INTN                   Len
  )
/*++

Routine Description:

  Validate whether a single DHCP option is valid according to its format.

Arguments:

  Format    - The option's format
  OptValue  - The value of the option
  Len       - The length of the option value

Returns:

  TRUE is the option is valid, otherwise FALSE.

--*/
{
  INTN                      Unit;
  INTN                      Occur;
  INTN                      Index;

  Unit = 0;

  switch (Format->Type) {
  case DHCP_OPTION_SWITCH:
  case DHCP_OPTION_INT8:
    Unit = 1;
    break;

  case DHCP_OPTION_INT16:
    Unit = 2;
    break;

  case DHCP_OPTION_INT32:
  case DHCP_OPTION_IP:
    Unit = 4;
    break;

  case DHCP_OPTION_IPPAIR:
    Unit = 8;
    break;
  }

  ASSERT (Unit != 0);

  //
  // Validate that the option appears in the full units.
  //
  if ((Len % Unit) != 0) {
    return FALSE;
  }
  
  //
  // Validate the occurance of the option unit is with in [MinOccur, MaxOccur]
  //
  Occur = Len / Unit;

  if (((Format->MinOccur != -1) && (Occur < Format->MinOccur)) ||
      ((Format->MaxOccur != -1) && (Occur > Format->MaxOccur))) {
    return FALSE;
  }
  
  //
  // If the option is of type switch, only 0/1 are valid values.
  //
  if (Format->Type == DHCP_OPTION_SWITCH) {
    for (Index = 0; Index < Occur; Index++) {
      if ((OptValue[Index] != 0) && (OptValue[Index] != 1)) {
        return FALSE;
      }
    }
  }

  return TRUE;
}

STATIC
EFI_STATUS
DhcpGetParameter (
  IN UINT8                  Tag,
  IN INTN                   Len,
  IN UINT8                  *Data,
  IN DHCP_PARAMETER         *Para
  )
/*++

Routine Description:

  Extract the client interested options, all the parameters are
  converted to host byte order.

Arguments:

  Tag   - The DHCP option tag
  Len   - The length of the option
  Data  - The value of the DHCP option
  Para  - The variable to save the interested parameter

Returns:

  EFI_SUCCESS           - The DHCP option is successfully extracted.
  EFI_INVALID_PARAMETER - The DHCP option is mal-formated

--*/
{
  switch (Tag) {
  case DHCP_TAG_NETMASK:
    Para->NetMask = NetGetUint32 (Data);
    break;

  case DHCP_TAG_ROUTER:
    //
    // Return the first router to consumer which is the preferred one
    //
    Para->Router = NetGetUint32 (Data);
    break;

  case DHCP_TAG_LEASE:
    Para->Lease = NetGetUint32 (Data);
    break;

  case DHCP_TAG_OVERLOAD:
    Para->Overload = *Data;

    if ((Para->Overload < 1) || (Para->Overload > 3)) {
      return EFI_INVALID_PARAMETER;
    }
    break;

  case DHCP_TAG_TYPE:
    Para->DhcpType = *Data;

    if ((Para->DhcpType < 1) || (Para->DhcpType > 9)) {
      return EFI_INVALID_PARAMETER;
    }
    break;

  case DHCP_TAG_SERVER_ID:
    Para->ServerId = NetGetUint32 (Data);
    break;

  case DHCP_TAG_T1:
    Para->T1 = NetGetUint32 (Data);
    break;

  case DHCP_TAG_T2:
    Para->T2 = NetGetUint32 (Data);
    break;
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
DhcpIterateBufferOptions (
  IN  UINT8                 *Buffer,
  IN  INTN                  BufLen,
  IN  DHCP_CHECK_OPTION     Check,            OPTIONAL
  IN  VOID                  *Context,
  OUT UINT8                 *Overload         OPTIONAL
  )
/*++

Routine Description:

  Inspect all the options in a single buffer. DHCP options may be contained
  in several buffers, such as the BOOTP options filed, boot file or server
  name. Each option buffer is required to end with DHCP_TAG_EOP.

Arguments:

  Buffer    - The buffer which contains DHCP options
  BufLen    - The length of the buffer
  Check     - The callback function for each option found
  Context   - The opaque parameter for the Check
  Overload  - variable to save the value of DHCP_TAG_OVERLOAD option. 

Returns:

  EFI_SUCCESS           - All the options are valid
  EFI_INVALID_PARAMETER - The options are mal-formated.

--*/
{
  INTN                      Cur;
  UINT8                     Tag;
  UINT8                     Len;

  Cur = 0;

  while (Cur < BufLen) {
    Tag = Buffer[Cur];

    if (Tag == DHCP_TAG_PAD) {
      Cur++;
      continue;
    } else if (Tag == DHCP_TAG_EOP) {
      return EFI_SUCCESS;
    }

    Cur++;
    
    if (Cur == BufLen) {
      return EFI_INVALID_PARAMETER;
    }

    Len = Buffer[Cur++];
    
    if (Cur + Len > BufLen) {
      return EFI_INVALID_PARAMETER;
    }

    if ((Tag == DHCP_TAG_OVERLOAD) && (Overload != NULL)) {
      if (Len != 1) {
        return EFI_INVALID_PARAMETER;
      }

      *Overload = Buffer[Cur];
    }

    if ((Check != NULL) && EFI_ERROR (Check (Tag, Len, Buffer + Cur, Context))) {
      return EFI_INVALID_PARAMETER;
    }

    Cur += Len;
  }
  
  //
  // Each option buffer is expected to end with an EOP
  //
  return EFI_INVALID_PARAMETER;
}

EFI_STATUS
DhcpIterateOptions (
  IN  EFI_DHCP4_PACKET      *Packet,
  IN  DHCP_CHECK_OPTION     Check,        OPTIONAL
  IN  VOID                  *Context
  )
/*++

Routine Description:

  Iterate through a DHCP message to visit each option. First inspect
  all the options in the OPTION field. Then if overloaded, inspect
  the options in FILENAME and SERVERNAME fields. One option may be
  encoded in several places. See RFC 3396 Encoding Long Options in DHCP

Arguments:

  Packet  - The DHCP packet to check the options for
  Check   - The callback function to be called for each option found
  Context - The opaque parameter for Check

Returns:

  EFI_SUCCESS - The DHCP packet's options are well formated
  Others      - The DHCP packet's options are not well formated

--*/
{
  EFI_STATUS                Status;
  UINT8                     Overload;

  Overload = 0;

  Status   = DhcpIterateBufferOptions (
               Packet->Dhcp4.Option,
               Packet->Length - sizeof (EFI_DHCP4_HEADER) - sizeof (UINT32),
               Check,
               Context,
               &Overload
               );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  if ((Overload == DHCP_OVERLOAD_FILENAME) || (Overload == DHCP_OVERLOAD_BOTH)) {
    Status = DhcpIterateBufferOptions (
               Packet->Dhcp4.Header.BootFileName,
               128,
               Check,
               Context,
               NULL
               );

    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  if ((Overload == DHCP_OVERLOAD_SVRNAME) || (Overload == DHCP_OVERLOAD_BOTH)) {
    Status = DhcpIterateBufferOptions (
               Packet->Dhcp4.Header.ServerName,
               64,
               Check,
               Context,
               NULL
               );

    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
DhcpGetOptionLen (
  IN UINT8                  Tag,
  IN UINT8                  Len,
  IN UINT8                  *Data,
  IN VOID                   *Context
  )
/*++

Routine Description:

  Call back function to DhcpiterateOptions to compute each option's
  length. It just adds the data length of all the occurances of this 
  Tag. Context is an array of 256 DHCP_OPTION_COUNT.

Arguments:

  Tag     - The current option to check
  Len     - The length of the option data
  Data    - The option data
  Context - The context, which is a array of 256 DHCP_OPTION_COUNT.

Returns:

  EFI_SUCCESS - It always returns EFI_SUCCESS.

--*/
{
  DHCP_OPTION_COUNT         *OpCount;

  OpCount             = (DHCP_OPTION_COUNT *) Context;
  OpCount[Tag].Offset = OpCount[Tag].Offset + Len;

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
DhcpFillOption (
  IN UINT8                  Tag,
  IN UINT8                  Len,
  IN UINT8                  *Data,
  IN VOID                   *Context
  )
/*++

Routine Description:

  Call back function to DhcpiterateOptions to consolidate each option's
  data. There are maybe several occurance of the same option.

Arguments:

  Tag     - The option to consolidate its data
  Len     - The length of option data
  Data    - The data of the option's current occurance
  Context - The context, which is DHCP_OPTION_CONTEXT. This array is 
            just a wrap to pass THREE parameters. 

Returns:

  EFI_SUCCESS - It always returns EFI_SUCCESS

--*/
{
  DHCP_OPTION_CONTEXT       *OptContext;
  DHCP_OPTION_COUNT         *OptCount;
  DHCP_OPTION               *Options;
  UINT8                     *Buf;
  UINT8                     Index;

  OptContext  = (DHCP_OPTION_CONTEXT *) Context;

  OptCount    = OptContext->OpCount;
  Index       = OptCount[Tag].Index;
  Options     = OptContext->Options;
  Buf         = OptContext->Buf;

  if (Options[Index].Data == NULL) {
    Options[Index].Tag  = Tag;
    Options[Index].Data = Buf + OptCount[Tag].Offset;
  }

  NetCopyMem (Buf + OptCount[Tag].Offset, Data, Len);

  OptCount[Tag].Offset  = OptCount[Tag].Offset + Len;
  Options[Index].Len    = Options[Index].Len + Len;
  return EFI_SUCCESS;
}

EFI_STATUS
DhcpParseOption (
  IN  EFI_DHCP4_PACKET      *Packet,
  OUT INTN                  *Count,
  OUT DHCP_OPTION           **OptionPoint
  )
/*++

Routine Description:

  Parse the options of a DHCP packet. It supports RFC 3396: Encoding 
  Long Options in DHCP. That is, it will combine all the option value 
  of all the occurances of each option. 

  A little bit of implemenation:
  It adopts the "Key indexed counting" algorithm. First, it allocates
  an array of 256 DHCP_OPTION_COUNTs because DHCP option tag is encoded
  as a UINT8. It then iterates the DHCP packet to get data length of
  each option by calling DhcpIterOptions with DhcpGetOptionLen. Now, it
  knows the number of present options and their length. It allocates a
  array of DHCP_OPTION and a continous buffer after the array to put 
  all the options' data. Each option's data is pointed to by the Data 
  field in DHCP_OPTION structure. At last, it call DhcpIterateOptions 
  with DhcpFillOption to fill each option's data to its position in the
  buffer.
  
Arguments:

  Packet      - The DHCP packet to parse the options
  Count       - The number of valid dhcp options present in the packet
  OptionPoint - The array that contains the DHCP options. Caller should free it.

Returns:

  EFI_OUT_OF_RESOURCES  - Failed to allocate memory to parse the packet.
  EFI_INVALID_PARAMETER - The options are mal-formated
  EFI_SUCCESS           - The options are parsed into OptionPoint

--*/
{
  DHCP_OPTION_CONTEXT       Context;
  DHCP_OPTION               *Options;
  DHCP_OPTION_COUNT         *OptCount;
  EFI_STATUS                Status;
  UINT16                    TotalLen;
  INTN                      OptNum;
  INTN                      Index;

  ASSERT ((Count != NULL) && (OptionPoint != NULL));

  //
  // First compute how many options and how long each option is
  // with the "Key indexed counting" algorithms.
  //
  OptCount = NetAllocateZeroPool (DHCP_MAX_OPTIONS * sizeof (DHCP_OPTION_COUNT));

  if (OptCount == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Status = DhcpIterateOptions (Packet, DhcpGetOptionLen, OptCount);

  if (EFI_ERROR (Status)) {
    goto ON_EXIT;
  }

  //
  // Before the loop, Offset is the length of the option. After loop,
  // OptCount[Index].Offset specifies the offset into the continuous
  // option value buffer to put the data.
  //
  TotalLen  = 0;
  OptNum    = 0;
  
  for (Index = 0; Index < DHCP_MAX_OPTIONS; Index++) {
    if (OptCount[Index].Offset != 0) {
      OptCount[Index].Index   = (UINT8) OptNum;

      TotalLen                = TotalLen + OptCount[Index].Offset;
      OptCount[Index].Offset  = TotalLen - OptCount[Index].Offset;

      OptNum++;
    }
  }

  *Count        = OptNum;
  *OptionPoint  = NULL;

  if (OptNum == 0) {
    goto ON_EXIT;
  }

  //
  // Allocate a buffer to hold the DHCP options, and after that, a 
  // continuous buffer to put all the options' data.
  //
  Options = NetAllocateZeroPool (OptNum * sizeof (DHCP_OPTION) + TotalLen);

  if (Options == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto ON_EXIT;
  }

  Context.OpCount = OptCount;
  Context.Options = Options;
  Context.Buf     = (UINT8 *) (Options + OptNum);

  Status          = DhcpIterateOptions (Packet, DhcpFillOption, &Context);

  if (EFI_ERROR (Status)) {
    NetFreePool (Options);
    goto ON_EXIT;
  }

  *OptionPoint = Options;

ON_EXIT:
  NetFreePool (OptCount);
  return Status;
}

EFI_STATUS
DhcpValidateOptions (
  IN  EFI_DHCP4_PACKET      *Packet,
  OUT DHCP_PARAMETER        **Para       OPTIONAL
  )
/*++

Routine Description:

  Validate the packet's options. If necessary, allocate
  and fill in the interested parameters.

Arguments:

  Packet  - The packet to validate the options
  Para    - The variable to save the DHCP parameters.

Returns:

  EFI_OUT_OF_RESOURCES  - Failed to allocate memory to validate the packet.
  EFI_INVALID_PARAMETER - The options are mal-formated
  EFI_SUCCESS           - The options are parsed into OptionPoint

--*/
{
  DHCP_PARAMETER            Parameter;
  DHCP_OPTION_FORMAT        *Format;
  DHCP_OPTION               *AllOption;
  DHCP_OPTION               *Option;
  EFI_STATUS                Status;
  BOOLEAN                   Updated;
  INTN                      Count;
  INTN                      Index;

  if (Para != NULL) {
    *Para = NULL;
  }

  AllOption = NULL;
  Status    = DhcpParseOption (Packet, &Count, &AllOption);

  if (EFI_ERROR (Status) || (Count == 0)) {
    return Status;
  }

  Updated = FALSE;
  NetZeroMem (&Parameter, sizeof (Parameter));

  for (Index = 0; Index < Count; Index++) {
    Option = &AllOption[Index];

    //
    // Find the format of the option then validate it.
    //
    Format = DhcpFindOptionFormat (Option->Tag);
    
    if (Format == NULL) {
      continue;
    }

    if (!DhcpOptionIsValid (Format, Option->Data, Option->Len)) {
      Status = EFI_INVALID_PARAMETER;
      goto ON_EXIT;
    }
    
    //
    // Get the client interested parameters
    //
    if (Format->Alert && (Para != NULL)) {
      Updated = TRUE;
      Status  = DhcpGetParameter (Option->Tag, Option->Len, Option->Data, &Parameter);

      if (EFI_ERROR (Status)) {
        goto ON_EXIT;
      }
    }
  }

  if (Updated && (Para != NULL)) {
    if ((*Para = NetAllocatePool (sizeof (DHCP_PARAMETER))) == NULL) {
      Status = EFI_OUT_OF_RESOURCES;
      goto ON_EXIT;
    }

    **Para = Parameter;
  }

ON_EXIT:
  NetFreePool (AllOption);
  return Status;
}


UINT8 *
DhcpAppendOption (
  IN UINT8                  *Buf,
  IN UINT8                  Tag,
  IN UINT16                 DataLen,
  IN UINT8                  *Data
  )
/*++

Routine Description:

  Append an option to the memory, if the option is longer than
  255 bytes, splits it into several options.

Arguments:

  Buf     - The buffer to append the option to
  Tag     - The option's tag
  DataLen - The length of the option's data
  Data    - The option's data

Returns:

  The position to append the next option

--*/
{
  INTN                      Index;
  INTN                      Len;

  ASSERT (DataLen != 0);

  for (Index = 0; Index < (DataLen + 254) / 255; Index++) {
    Len      = NET_MIN (255, DataLen - Index * 255);

    *(Buf++) = Tag;
    *(Buf++) = (UINT8) Len;
    NetCopyMem (Buf, Data + Index * 255, Len);

    Buf     += Len;
  }

  return Buf;
}

EFI_STATUS
DhcpBuild (
  IN  EFI_DHCP4_PACKET        *SeedPacket,
  IN  UINT32                  DeleteCount,
  IN  UINT8                   *DeleteList     OPTIONAL,
  IN  UINT32                  AppendCount,
  IN  EFI_DHCP4_PACKET_OPTION *AppendList[]   OPTIONAL,
  OUT EFI_DHCP4_PACKET        **NewPacket
  )
/*++

Routine Description:

  Build a new DHCP packet from a seed packet. Options may be deleted or
  appended. The caller should free the NewPacket when finished using it.

Arguments:

  SeedPacket  - The seed packet to start with
  DeleteCount - The number of options to delete
  DeleteList  - The options to delete from the packet 
  AppendCount - The number of options to append
  AppendList  - The options to append to the packet 
  NewPacket   - The new packet, allocated and built by this function.

Returns:

  EFI_OUT_OF_RESOURCES - Failed to allocate memory
  EFI_SUCCESS          - The packet is build.
  
--*/
{
  DHCP_OPTION               *Mark;
  DHCP_OPTION               *SeedOptions;
  EFI_DHCP4_PACKET          *Packet;
  EFI_STATUS                Status;
  INTN                      Count;
  UINT32                    Index;
  UINT32                    Len;
  UINT8                     *Buf;

  //
  // Use an array of DHCP_OPTION to mark the existance
  // and position of each valid options.
  //
  Mark = NetAllocatePool (sizeof (DHCP_OPTION) * DHCP_MAX_OPTIONS);
  
  if (Mark == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  for (Index = 0; Index < DHCP_MAX_OPTIONS; Index++) {
    Mark[Index].Tag = (UINT8) Index;
    Mark[Index].Len = 0;
  }
  
  //
  // Get list of the options from the seed packet, then put
  // them to the mark array according to their tags.
  //
  SeedOptions = NULL;
  Status      = DhcpParseOption (SeedPacket, &Count, &SeedOptions);

  if (EFI_ERROR (Status)) {
    goto ON_ERROR;
  }

  for (Index = 0; Index < (UINT32) Count; Index++) {
    Mark[SeedOptions[Index].Tag] = SeedOptions[Index];
  }
  
  //
  // Mark the option's length is zero if it is in the DeleteList.
  //
  for (Index = 0; Index < DeleteCount; Index++) {
    Mark[DeleteList[Index]].Len = 0;
  }
  
  //
  // Add or replace the option if it is in the append list.
  //
  for (Index = 0; Index < AppendCount; Index++) {
    Mark[AppendList[Index]->OpCode].Len  = AppendList[Index]->Length;
    Mark[AppendList[Index]->OpCode].Data = AppendList[Index]->Data;
  }
  
  //
  // compute the new packet length. No need to add 1 byte for
  // EOP option since EFI_DHCP4_PACKET includes one extra byte
  // for option. It is necessary to split the option if it is
  // longer than 255 bytes.
  //
  Len = sizeof (EFI_DHCP4_PACKET);

  for (Index = 0; Index < DHCP_MAX_OPTIONS; Index++) {
    if (Mark[Index].Len != 0) {
      Len += ((Mark[Index].Len + 254) / 255) * 2 + Mark[Index].Len;
    }
  }

  Status  = EFI_OUT_OF_RESOURCES;
  Packet  = (EFI_DHCP4_PACKET *) NetAllocatePool (Len);

  if (Packet == NULL) {
    goto ON_ERROR;
  }

  Packet->Size         = Len;
  Packet->Length       = 0;
  Packet->Dhcp4.Header = SeedPacket->Dhcp4.Header;
  Packet->Dhcp4.Magik  = DHCP_OPTION_MAGIC;
  Buf                  = Packet->Dhcp4.Option;

  for (Index = 0; Index < DHCP_MAX_OPTIONS; Index++) {
    if (Mark[Index].Len != 0) {
      Buf = DhcpAppendOption (Buf, Mark[Index].Tag, Mark[Index].Len, Mark[Index].Data);
    }
  }

  *(Buf++)        = DHCP_TAG_EOP;
  Packet->Length  = sizeof (EFI_DHCP4_HEADER) + sizeof (UINT32) 
                      + (UINT32) (Buf - Packet->Dhcp4.Option);

  *NewPacket      = Packet;
  Status          = EFI_SUCCESS;

ON_ERROR:
  if (SeedOptions != NULL) {
    NetFreePool (SeedOptions);
  }

  NetFreePool (Mark);
  return Status;
}
