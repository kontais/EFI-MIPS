/*++

Copyright (c) 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  IfConfig.c
  
Abstract:

  Shell command "IfConfig"

--*/
#ifndef _EFI_IFCONFIG_H_
#define _EFI_IFCONFIG_H_

#include "EfiShellLib.h"
#include "ShellNetHelper.h"

#include EFI_PROTOCOL_DEFINITION (NicIp4Config)
#include EFI_PROTOCOL_DEFINITION (Arp)

#define EFI_IFCONFIG_GUID \
  { \
    0xb2f9929a, 0x648c, 0x4a9d, 0xb9, 0x92, 0xb1, 0x26, 0xfa, 0x32, 0x65, 0x82 \
  }

#define SEC_TO_NS             10000000

typedef struct {
  EFI_LIST_ENTRY              Link;
  
  EFI_HANDLE                  Handle;
  NIC_ADDR                    NicAddress;
  UINT16                      Name[IP4_NIC_NAME_LENGTH];
  
  EFI_NIC_IP4_CONFIG_PROTOCOL *NicIp4Config;
  EFI_IP4_CONFIG_PROTOCOL     *Ip4Config;

  NIC_IP4_CONFIG_INFO         *ConfigInfo;
} NIC_INFO;

typedef struct {
  EFI_IP_ADDRESS              DestIp;
  EFI_MAC_ADDRESS             DestMac;
  EFI_IP_ADDRESS              LocalIp;
  EFI_MAC_ADDRESS             LocalMac;
  UINT8                       MacLen;
  EFI_EVENT                   OnResolved;
  BOOLEAN                     Duplicate;
} ARP_REQUEST;

#endif
