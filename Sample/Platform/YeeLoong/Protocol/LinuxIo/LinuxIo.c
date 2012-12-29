/*++

Copyright (c) 2004 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  LinuxIo.c

Abstract:

  This protocol allows an EFI driver (ELF) in the Linux emulation envirnment
  to make Win32 API calls.

--*/

//#include "EfiLinux.h"
#include "Tiano.h"
#include EFI_PROTOCOL_DEFINITION (LinuxIo)

EFI_GUID  gEfiLinuxIoProtocolGuid = EFI_LINUX_IO_PROTOCOL_GUID;

EFI_GUID_STRING(&gEfiLinuxIoProtocolGuid, "EFI Linux I/O Protocol", "Linux API I/O protocol");

EFI_GUID  gEfiLinuxVirtualDisksGuid   = EFI_LINUX_VIRTUAL_DISKS_GUID;
EFI_GUID  gEfiLinuxPhysicalDisksGuid  = EFI_LINUX_PHYSICAL_DISKS_GUID;
EFI_GUID  gEfiLinuxFileSystemGuid     = EFI_LINUX_FILE_SYSTEM_GUID;
EFI_GUID  gEfiLinuxSerialPortGuid     = EFI_LINUX_SERIAL_PORT_GUID;
EFI_GUID  gEfiLinuxUgaGuid            = EFI_LINUX_UGA_GUID;
EFI_GUID  gEfiLinuxGopGuid            = EFI_LINUX_GOP_GUID;
EFI_GUID  gEfiLinuxConsoleGuid        = EFI_LINUX_CONSOLE_GUID;
EFI_GUID  gEfiLinuxMemoryGuid         = EFI_LINUX_MEMORY_GUID;
EFI_GUID  gEfiLinuxCPUModelGuid       = EFI_LINUX_CPU_MODEL_GUID;
EFI_GUID  gEfiLinuxCPUSpeedGuid       = EFI_LINUX_CPU_SPEED_GUID;
EFI_GUID  gEfiLinuxPassThroughGuid    = EFI_LINUX_PASS_THROUGH_GUID;
