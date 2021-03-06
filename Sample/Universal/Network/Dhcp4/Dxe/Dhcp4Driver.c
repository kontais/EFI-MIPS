/*++

Copyright (c) 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED. 


Module Name:

  Dhcp4Driver.c

Abstract:

--*/
#include "Dhcp4Impl.h"
#include "Dhcp4Driver.h"

EFI_DRIVER_BINDING_PROTOCOL gDhcp4DriverBinding = {
  Dhcp4DriverBindingSupported,
  Dhcp4DriverBindingStart,
  Dhcp4DriverBindingStop,
  0xa,
  NULL,
  NULL
};

EFI_SERVICE_BINDING_PROTOCOL mDhcp4ServiceBindingTemplete = {
  Dhcp4ServiceBindingCreateChild,
  Dhcp4ServiceBindingDestroyChild
};

EFI_DRIVER_ENTRY_POINT (Dhcp4DriverEntryPoint)

EFI_STATUS
EFIAPI
Dhcp4DriverEntryPoint (
  IN EFI_HANDLE             ImageHandle,
  IN EFI_SYSTEM_TABLE       *SystemTable
  )
/*++

Routine Description:

  Entry point of the DHCP driver to install various protocols.

Arguments:

  ImageHandle - The driver's image handle
  SystemTable - The system table

Returns:

  EFI_SUCCESS - All the related protocols are installed.
  Others      - Failed to install the protocols.

--*/
{
  return NetLibInstallAllDriverProtocols (
           ImageHandle,
           SystemTable,
           &gDhcp4DriverBinding,
           ImageHandle,
           &gDhcp4ComponentName,
           NULL,
           NULL
           );
}

EFI_STATUS
EFIAPI
Dhcp4DriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   ControllerHandle,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath OPTIONAL
  )
/*++

  Routine Description:
    Test to see if DHCP driver supports the ControllerHandle. 

  Arguments:
    This                - Protocol instance pointer.
    ControllerHandle    - Handle of device to test
    RemainingDevicePath - Optional parameter use to pick a specific child 
                          device to start.

  Returns:
    EFI_SUCCES          - This driver supports this device
    other               - This driver does not support this device

--*/
{
  EFI_STATUS  Status;

  Status = gBS->OpenProtocol (
                  ControllerHandle,
                  &gEfiUdp4ServiceBindingProtocolGuid,
                  NULL,
                  This->DriverBindingHandle,
                  ControllerHandle,
                  EFI_OPEN_PROTOCOL_TEST_PROTOCOL
                  );

  return Status;
}


EFI_STATUS
DhcpConfigUdpIo (
  IN UDP_IO_PORT            *UdpIo,
  IN VOID                   *Context
  )
/*++

Routine Description:

  Configure the default UDP child to receive all the DHCP traffics
  on this network interface.

Arguments:

  UdpIo   - The UDP IO port to configure
  Context - The context to the function

Returns:

  EFI_SUCCESS - The UDP IO port is successfully configured.
  Others      - Failed to configure the UDP child.

--*/
{
  EFI_UDP4_CONFIG_DATA      UdpConfigData;

  UdpConfigData.AcceptBroadcast           = TRUE;
  UdpConfigData.AcceptPromiscuous         = FALSE;
  UdpConfigData.AcceptAnyPort             = FALSE;
  UdpConfigData.AllowDuplicatePort        = TRUE;
  UdpConfigData.TypeOfService             = 0;
  UdpConfigData.TimeToLive                = 64;
  UdpConfigData.DoNotFragment             = FALSE;
  UdpConfigData.ReceiveTimeout            = 0;
  UdpConfigData.TransmitTimeout           = 0;

  UdpConfigData.UseDefaultAddress         = FALSE;
  UdpConfigData.StationPort               = DHCP_CLIENT_PORT;
  UdpConfigData.RemotePort                = DHCP_SERVER_PORT;

  EFI_IP4 (UdpConfigData.StationAddress)  = 0;
  EFI_IP4 (UdpConfigData.SubnetMask)      = 0;
  EFI_IP4 (UdpConfigData.RemoteAddress)   = 0;

  return UdpIo->Udp->Configure (UdpIo->Udp, &UdpConfigData);;
}


EFI_STATUS
Dhcp4CloseService (
  IN DHCP_SERVICE           *DhcpSb
  )
/*++

Routine Description:

  Destory the DHCP service. The Dhcp4 service may be partly initialized, 
  or partly destoried. If a resource is destoried, it is marked as so in
  case the destory failed and being called again later.

Arguments:

  DhcpSb  - The DHCP service instance to destory.

Returns:

  EFI_SUCCESS - The DHCP service is successfully closed.

--*/
{
  DhcpCleanLease (DhcpSb);

  if (DhcpSb->UdpIo != NULL) {
    UdpIoFreePort (DhcpSb->UdpIo);
    DhcpSb->UdpIo = NULL;
  }

  if (DhcpSb->Timer != NULL) {
    gBS->SetTimer (DhcpSb->Timer, TimerCancel, 0);
    gBS->CloseEvent (DhcpSb->Timer);

    DhcpSb->Timer = NULL;
  }

  return EFI_SUCCESS;
}


EFI_STATUS
Dhcp4CreateService (
  IN  EFI_HANDLE            Controller,
  IN  EFI_HANDLE            ImageHandle,
  OUT DHCP_SERVICE          **Service
  )
/*++

Routine Description:

  Create a new DHCP service binding instance for the controller.

Arguments:

  Controller  - The controller to install DHCP service binding protocol onto
  ImageHandle - The driver's image handle
  Service     - The variable to receive the created DHCP service instance.

Returns:

  EFI_OUT_OF_RESOURCES - Failed to allocate resource .
  EFI_SUCCESS          - The DHCP service instance is created.

--*/
{
  DHCP_SERVICE              *DhcpSb;
  EFI_STATUS                Status;

  *Service  = NULL;
  DhcpSb    = NetAllocateZeroPool (sizeof (DHCP_SERVICE));
  
  if (DhcpSb == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  DhcpSb->Signature       = DHCP_SERVICE_SIGNATURE;
  DhcpSb->ServiceBinding  = mDhcp4ServiceBindingTemplete;
  DhcpSb->ServiceState    = DHCP_UNCONFIGED;
  DhcpSb->InDestory       = FALSE;
  DhcpSb->Controller      = Controller;
  DhcpSb->Image           = ImageHandle;
  NetListInit (&DhcpSb->Children);
  DhcpSb->DhcpState       = Dhcp4Stopped;
  DhcpSb->Xid             = NET_RANDOM (NetRandomInitSeed ());

  //
  // Create various resources, UdpIo, Timer, and get Mac address
  //
  Status = gBS->CreateEvent (
                  EFI_EVENT_NOTIFY_SIGNAL | EFI_EVENT_TIMER,
                  EFI_TPL_CALLBACK,
                  DhcpOnTimerTick,
                  DhcpSb,
                  &DhcpSb->Timer
                  );

  if (EFI_ERROR (Status)) {
    goto ON_ERROR;
  }

  DhcpSb->UdpIo = UdpIoCreatePort (Controller, ImageHandle, DhcpConfigUdpIo, NULL);

  if (DhcpSb->UdpIo == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto ON_ERROR;
  }

  DhcpSb->HwLen  = (UINT8) DhcpSb->UdpIo->SnpMode.HwAddressSize;
  DhcpSb->HwType = DhcpSb->UdpIo->SnpMode.IfType;
  DhcpSb->Mac    = DhcpSb->UdpIo->SnpMode.CurrentAddress;

  *Service       = DhcpSb;
  return EFI_SUCCESS;

ON_ERROR:
  Dhcp4CloseService (DhcpSb);
  NetFreePool (DhcpSb);

  return Status;
}

EFI_STATUS
EFIAPI
Dhcp4DriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   ControllerHandle,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath OPTIONAL
  )
/*++

  Routine Description:
    Start this driver on ControllerHandle.

  Arguments:
    This                - Protocol instance pointer.
    ControllerHandle    - Handle of device to bind driver to
    RemainingDevicePath - Optional parameter use to pick a specific child 
                          device to start.

  Returns:
    EFI_SUCCES          - This driver is added to ControllerHandle
    EFI_ALREADY_STARTED - This driver is already running on ControllerHandle
    other               - This driver does not support this device

--*/
{
  DHCP_SERVICE              *DhcpSb;
  EFI_STATUS                Status;

  //
  // First: test for the DHCP4 Protocol
  //
  Status = gBS->OpenProtocol (
                  ControllerHandle,
                  &gEfiDhcp4ServiceBindingProtocolGuid,
                  NULL,
                  This->DriverBindingHandle,
                  ControllerHandle,
                  EFI_OPEN_PROTOCOL_TEST_PROTOCOL
                  );

  if (Status == EFI_SUCCESS) {
    return EFI_ALREADY_STARTED;
  }

  Status = Dhcp4CreateService (ControllerHandle, This->DriverBindingHandle, &DhcpSb);

  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = gBS->SetTimer (DhcpSb->Timer, TimerPeriodic, TICKS_PER_SECOND);

  if (EFI_ERROR (Status)) {
    goto ON_ERROR;
  }
  
  //
  // Install the Dhcp4ServiceBinding Protocol onto ControlerHandle
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &ControllerHandle,
                  &gEfiDhcp4ServiceBindingProtocolGuid,
                  &DhcpSb->ServiceBinding,
                  NULL
                  );

  if (EFI_ERROR (Status)) {
    goto ON_ERROR;
  }

  return Status;

ON_ERROR:
  Dhcp4CloseService (DhcpSb);
  NetFreePool (DhcpSb);
  return Status;
}

EFI_STATUS
EFIAPI
Dhcp4DriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN  EFI_HANDLE                   ControllerHandle,
  IN  UINTN                        NumberOfChildren,
  IN  EFI_HANDLE                   *ChildHandleBuffer
  )
/*++

  Routine Description:
    Stop this driver on ControllerHandle.

  Arguments:
    This              - Protocol instance pointer.
    ControllerHandle  - Handle of device to stop driver on 
    NumberOfChildren  - Number of Handles in ChildHandleBuffer. If number of 
                        children is zero stop the entire bus driver.
    ChildHandleBuffer - List of Child Handles to Stop.

  Returns:
    EFI_SUCCES          - This driver is removed ControllerHandle
    other               - This driver was not removed from this device

--*/
{
  EFI_SERVICE_BINDING_PROTOCOL  *ServiceBinding;
  DHCP_SERVICE                  *DhcpSb;
  DHCP_PROTOCOL                 *Instance;
  EFI_HANDLE                    NicHandle;
  EFI_STATUS                    Status;
  EFI_TPL                       OldTpl;

  //
  // DHCP driver opens UDP child, So, the ControllerHandle is the
  // UDP child handle. locate the Nic handle first.
  //
  NicHandle = NetLibGetNicHandle (ControllerHandle, &gEfiUdp4ProtocolGuid);
  
  if (NicHandle == NULL) {
    return EFI_SUCCESS;
  }
  
   Status = gBS->OpenProtocol (
                  NicHandle,
                  &gEfiDhcp4ServiceBindingProtocolGuid,
                  (VOID **) &ServiceBinding,
                  This->DriverBindingHandle,
                  NicHandle,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );

  if (EFI_ERROR (Status)) {
    return EFI_DEVICE_ERROR;
  }

  DhcpSb = DHCP_SERVICE_FROM_THIS (ServiceBinding);

  if (DhcpSb->InDestory) {
    return EFI_SUCCESS;
  }

  OldTpl            = NET_RAISE_TPL (NET_TPL_LOCK);
  DhcpSb->InDestory = TRUE;

  //
  // Don't use NET_LIST_FOR_EACH_SAFE here, Dhcp4ServiceBindingDestoryChild
  // may cause other child to be deleted.
  //
  while (!NetListIsEmpty (&DhcpSb->Children)) {
    Instance = NET_LIST_HEAD (&DhcpSb->Children, DHCP_PROTOCOL, Link);
    Dhcp4ServiceBindingDestroyChild (ServiceBinding, Instance->Handle);
  }

  if (DhcpSb->NumChildren != 0) {
    Status = EFI_DEVICE_ERROR;
    goto ON_ERROR;
  }

  DhcpSb->ServiceState  = DHCP_DESTORY;

  Status = gBS->UninstallProtocolInterface (
                  NicHandle,
                  &gEfiDhcp4ServiceBindingProtocolGuid,
                  ServiceBinding
                  );

  if (EFI_ERROR (Status)) {
    goto ON_ERROR;
  }

  Dhcp4CloseService (DhcpSb);
  NET_RESTORE_TPL (OldTpl);

  NetFreePool (DhcpSb);
  return EFI_SUCCESS;

ON_ERROR:
  DhcpSb->InDestory = FALSE;
  NET_RESTORE_TPL (OldTpl);
  return Status;
}

VOID
DhcpInitProtocol (
  IN DHCP_SERVICE           *DhcpSb,
  IN DHCP_PROTOCOL          *Instance
  )
/*++

Routine Description:

  Initialize a new DHCP child

Arguments:

  DhcpSb    - The dhcp service instance
  Instance  - The dhcp instance to initialize

Returns:

  None

--*/
{
  Instance->Signature         = DHCP_PROTOCOL_SIGNATURE;
  Instance->Dhcp4Protocol     = mDhcp4ProtocolTemplate;
  NetListInit (&Instance->Link);
  Instance->Handle            = NULL;
  Instance->Service           = DhcpSb;
  Instance->InDestory         = FALSE;
  Instance->CompletionEvent   = NULL;
  Instance->RenewRebindEvent  = NULL;
  Instance->Token             = NULL;
}

EFI_STATUS
EFIAPI
Dhcp4ServiceBindingCreateChild (
  IN EFI_SERVICE_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                    *ChildHandle
  )
/*++

  Routine Description:
    Creates a child handle with a set of DHCP4 services.

  Arguments:
    This         - Protocol instance pointer.
    ChildHandle  - Pointer to the handle of the child to create.  If it 
                   is NULL, then a new handle is created.  If it is not 
                   NULL, then the DHCP4 services are added to the existing 
                   child handle.

  Returns:
    EFI_SUCCES           - The child handle was created with the DHCP4 services
    EFI_OUT_OF_RESOURCES - There are not enough resources to create the child
    other                - The child handle was not created

--*/
{
  DHCP_SERVICE              *DhcpSb;
  DHCP_PROTOCOL             *Instance;
  EFI_STATUS                Status;
  EFI_TPL                   OldTpl;
  VOID                      *Udp4;

  if ((This == NULL) || (ChildHandle == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  Instance = NetAllocatePool (sizeof (*Instance));
  
  if (Instance == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  DhcpSb = DHCP_SERVICE_FROM_THIS (This);
  DhcpInitProtocol (DhcpSb, Instance);

  //
  // Install DHCP4 onto ChildHandle
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                  ChildHandle,
                  &gEfiDhcp4ProtocolGuid,
                  &Instance->Dhcp4Protocol,
                  NULL
                  );

  if (EFI_ERROR (Status)) {
    NetFreePool (Instance);
    return Status;
  }

  Instance->Handle  = *ChildHandle;

  //
  // Open the Udp4 protocol BY_CHILD.
  //
  Status = gBS->OpenProtocol (
                  DhcpSb->UdpIo->UdpHandle,
                  &gEfiUdp4ProtocolGuid,
                  (VOID **) &Udp4,
                  gDhcp4DriverBinding.DriverBindingHandle,
                  Instance->Handle,
                  EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER
                  );
  if (EFI_ERROR (Status)) {
    gBS->UninstallMultipleProtocolInterfaces (
           Instance->Handle,
           &gEfiDhcp4ProtocolGuid,
           &Instance->Dhcp4Protocol,
           NULL
           );

    NetFreePool (Instance);
    return Status;
  }

  OldTpl = NET_RAISE_TPL (NET_TPL_LOCK);
  
  NetListInsertTail (&DhcpSb->Children, &Instance->Link);
  DhcpSb->NumChildren++;

  NET_RESTORE_TPL (OldTpl);

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
Dhcp4ServiceBindingDestroyChild (
  IN EFI_SERVICE_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                    ChildHandle
  )
/*++

  Routine Description:
    Destroys a child handle with a set of DHCP4 services.

  Arguments:
    This         - Protocol instance pointer.
    ChildHandle  - Handle of the child to destroy

  Returns:
    EFI_SUCCES            - The DHCP4 service is removed from the child handle
    EFI_UNSUPPORTED       - The child handle does not support the DHCP4 service
    EFI_INVALID_PARAMETER - Child handle is not a valid EFI Handle.
    EFI_ACCESS_DENIED     - The child handle could not be destroyed because its 
                            DHCP4 services are being used.
    other                 - The child handle was not destroyed

--*/
{
  DHCP_SERVICE              *DhcpSb;
  DHCP_PROTOCOL             *Instance;
  EFI_DHCP4_PROTOCOL        *Dhcp;
  EFI_TPL                   OldTpl;
  EFI_STATUS                Status;

  if ((This == NULL) || (ChildHandle == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Retrieve the private context data structures
  //
  Status = gBS->OpenProtocol (
                  ChildHandle,
                  &gEfiDhcp4ProtocolGuid,
                  (VOID **) &Dhcp,
                  gDhcp4DriverBinding.DriverBindingHandle,
                  ChildHandle,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );

  if (EFI_ERROR (Status)) {
    return EFI_UNSUPPORTED;
  }

  Instance  = DHCP_INSTANCE_FROM_THIS (Dhcp);
  DhcpSb    = DHCP_SERVICE_FROM_THIS (This);

  if (Instance->Service != DhcpSb) {
    return EFI_INVALID_PARAMETER;
  }
  
  //
  // A child can be destoried more than once. For example,
  // Dhcp4DriverBindingStop will destory all of its children.
  // when caller driver is being stopped, it will destory the
  // dhcp child it opens.
  //
  if (Instance->InDestory) {
    return EFI_SUCCESS;
  }

  OldTpl = NET_RAISE_TPL (NET_TPL_LOCK);
  Instance->InDestory = TRUE;

  //
  // Close the Udp4 protocol.
  //
  gBS->CloseProtocol (
         DhcpSb->UdpIo->UdpHandle,
         &gEfiUdp4ProtocolGuid,
         gDhcp4DriverBinding.DriverBindingHandle,
         ChildHandle
         );

  //
  // Uninstall the DHCP4 protocol first to enable a top down destruction.
  //
  Status = gBS->UninstallProtocolInterface (
                  ChildHandle,
                  &gEfiDhcp4ProtocolGuid,
                  Dhcp
                  );

  if (EFI_ERROR (Status)) {
    Instance->InDestory = FALSE;
    
    NET_RESTORE_TPL (OldTpl);
    return Status;
  }

  if (DhcpSb->ActiveChild == Instance) {
    DhcpYieldControl (DhcpSb);
  }

  NetListRemoveEntry (&Instance->Link);
  DhcpSb->NumChildren--;

  NET_RESTORE_TPL (OldTpl);

  NetFreePool (Instance);
  return EFI_SUCCESS;
}
