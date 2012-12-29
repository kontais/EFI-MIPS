/*++

Copyright (c) 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  ShellNetHelper.h

Abstract:

--*/
#ifndef _EFI_SHELL_NET_HELPER_H_
#define _EFI_SHELL_NET_HELPER_H_

#include EFI_PROTOCOL_DEFINITION (ServiceBinding)

enum {
  EFI_IP_PROTO_ICMP      = 0x01,

  IP4_ADDR_CLASSA        = 1,
  IP4_ADDR_CLASSB,
  IP4_ADDR_CLASSC,
  IP4_ADDR_CLASSD,
  IP4_ADDR_CLASSE,

  IP4_MASK_NUM           = 33,
};

//
//Iterate through the doule linked list. NOT delete safe
//
#define EFI_LIST_FOR_EACH(Entry, ListHead)    \
  for(Entry = (ListHead)->Flink; Entry != (ListHead); Entry = Entry->Flink)

//
//Iterate through the doule linked list. This is delete-safe.
//Don't touch NextEntry
//
#define EFI_LIST_FOR_EACH_SAFE(Entry, NextEntry, ListHead)            \
  for(Entry = (ListHead)->Flink, NextEntry = Entry->Flink;\
      Entry != (ListHead); Entry = NextEntry, NextEntry = Entry->Flink)


typedef UINT32 IP4_ADDR;

#define NTOHL(x)   \
  (UINT32)((((UINT32)(x) & 0xff) << 24)    | (((UINT32)(x) & 0xff00) << 8 ) | \
           (((UINT32)(x) & 0xff0000) >> 8) | (((UINT32)(x) & 0xff000000) >> 24))

#define IP4_EQUAL_MASK(Ip1, Ip2, NetMask) \
          (((Ip1) & (NetMask)) == ((Ip2) & (NetMask)))

#define EFI_IP4_TO_U32(EfiIpAddr)   (*(IP4_ADDR*)((EfiIpAddr).Addr))
#define EFI_IP4_NTOHL(EfiIp)        (NTOHL (EFI_IP4_TO_U32 ((EfiIp))))

#define SHELL_MEM_EQUAL(pMac1, pMac2, Len) \
        (CompareMem((pMac1), (pMac2), Len) == 0)

#define SHELL_IS_DIGIT(Ch)  (('0' <= (Ch)) && ((Ch) <= '9'))

BOOLEAN
ShellIp4IsUnicast (
  IN IP4_ADDR           Ip, 
  IN IP4_ADDR           NetMask
  );

EFI_STATUS 
ShellCreateServiceChild (
  IN  EFI_HANDLE  ControllerHandle,
  IN  EFI_HANDLE  ImageHandle,
  IN  EFI_GUID    *ServiceBindingGuid,
  OUT EFI_HANDLE  *ChildHandle
);

EFI_STATUS 
ShellDestroyServiceChild (
  IN  EFI_HANDLE  ControllerHandle,
  IN  EFI_HANDLE  ImageHandle,
  IN  EFI_GUID    *ServiceBindingGuid,
  IN  EFI_HANDLE  ChildHandle
);
#endif
