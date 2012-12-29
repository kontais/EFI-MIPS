/*++

Copyright (c) 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  ShellNetHelper.c

Abstract:

--*/
#include "EfiShellLib.h"
#include "ShellNetHelper.h"

IP4_ADDR 
mIp4AllMasks[IP4_MASK_NUM] = {
  0x00000000, 0x80000000, 0xC0000000, 0xE0000000,
  0xF0000000, 0xF8000000, 0xFC000000, 0xFE000000,
  
  0xFF000000, 0xFF800000, 0xFFC00000, 0xFFE00000,
  0xFFF00000, 0xFFF80000, 0xFFFC0000, 0xFFFE0000,

  0xFFFF0000, 0xFFFF8000, 0xFFFFC000, 0xFFFFE000,
  0xFFFFF000, 0xFFFFF800, 0xFFFFFC00, 0xFFFFFE00,
  
  0xFFFFFF00, 0xFFFFFF80, 0xFFFFFFC0, 0xFFFFFFE0,
  0xFFFFFFF0, 0xFFFFFFF8, 0xFFFFFFFC, 0xFFFFFFFE,
  0xFFFFFFFF,  
};

INTN
ShellGetIpClass (
  IN IP4_ADDR     Addr
  )
{
  UINT8     ByteOne;

  ByteOne = (UINT8)(Addr >> 24);

  if ((ByteOne & 0x80) == 0) {      
    return IP4_ADDR_CLASSA;

  } else if ((ByteOne & 0xC0) == 0x80) {      
    return IP4_ADDR_CLASSB;

  } else if ((ByteOne & 0xE0) == 0xC0) {      
    return IP4_ADDR_CLASSC;

  } else if ((ByteOne & 0xF0) == 0xE0) {      
    return IP4_ADDR_CLASSD;

  }else {                 
    return IP4_ADDR_CLASSE;

  } 
}

//
// Is this a valid unicast address? If NetMask is zero, use the 
// IP address's class to get the default mask.
//
BOOLEAN
ShellIp4IsUnicast (
  IN IP4_ADDR           Ip, 
  IN IP4_ADDR           NetMask
  )
{
  INTN                  Class;
  
  Class = ShellGetIpClass (Ip);
  
  if ((Ip == 0) || (Class >= IP4_ADDR_CLASSD)) {
    return FALSE;
  }

  if (NetMask == 0) {
    NetMask = mIp4AllMasks[Class << 3];
  }
  
  if (((Ip & ~NetMask) == ~NetMask) || ((Ip & ~NetMask) == 0)) {
    return FALSE;
  }
  
  return TRUE;
}

EFI_STATUS 
ShellCreateServiceChild (
  IN  EFI_HANDLE  ControllerHandle,
  IN  EFI_HANDLE  ImageHandle,
  IN  EFI_GUID    *ServiceBindingGuid,
  OUT EFI_HANDLE  *ChildHandle
  )
{
  EFI_STATUS                    Status;
  EFI_SERVICE_BINDING_PROTOCOL  *Service;

  if ((ServiceBindingGuid == NULL) || (ChildHandle == NULL)) {
    return EFI_INVALID_PARAMETER;
  }
  
  //
  // Get the ServiceBinding Protocol
  //
  Status = BS->OpenProtocol (
                 ControllerHandle,       
                 ServiceBindingGuid, 
                 (VOID **)&Service,
                 ImageHandle,   
                 ControllerHandle,   
                 EFI_OPEN_PROTOCOL_GET_PROTOCOL
                 );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Create a child
  //
  Status = Service->CreateChild (Service, ChildHandle);
  return Status;
}

EFI_STATUS 
ShellDestroyServiceChild (
  IN  EFI_HANDLE  ControllerHandle,
  IN  EFI_HANDLE  ImageHandle,
  IN  EFI_GUID    *ServiceBindingGuid,
  IN  EFI_HANDLE  ChildHandle
  )
{
  EFI_STATUS                    Status;
  EFI_SERVICE_BINDING_PROTOCOL  *Service;

  if (ServiceBindingGuid == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  
  //
  // Get the ServiceBinding Protocol
  //
  Status = BS->OpenProtocol (
                 ControllerHandle,       
                 ServiceBindingGuid, 
                 (VOID **)&Service,
                 ImageHandle,   
                 ControllerHandle,   
                 EFI_OPEN_PROTOCOL_GET_PROTOCOL
                 );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // destory the child
  //
  Status = Service->DestroyChild (Service, ChildHandle);
  return Status;
}
