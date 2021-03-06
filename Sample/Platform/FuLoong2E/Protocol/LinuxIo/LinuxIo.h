/*++

Copyright (c) 2004 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  LinuxIo.h

Abstract:

--*/

#ifndef _LINUX_IO_H_
#define _LINUX_IO_H_

#define EFI_LINUX_IO_PROTOCOL_GUID \
  { \
    0x96eb4ad6, 0xa32a, 0x11d4, {0xbc, 0xfd, 0x0, 0x80, 0xc7, 0x3c, 0x88, 0x81} \
  }

//
// The following APIs require EfiWinNT.h. In some environmnets the GUID
// definitions are needed but the EfiWinNT.h is not included.
// EfiWinNT.h is needed to support WINDOWS API requirements.
//
#ifdef _EFI_LINUX_H_

#include EFI_PROTOCOL_DEFINITION (LinuxThunk)

typedef struct {
  EFI_LINUX_THUNK_PROTOCOL *LinuxThunk;
  EFI_GUID                  *TypeGuid;
  CHAR16                    *EnvString;
  UINT16                    InstanceNumber;
} EFI_LINUX_IO_PROTOCOL;

#endif

extern EFI_GUID gEfiLinuxIoProtocolGuid;

//
// The following GUIDs are used in EFI_LINUX_IO_PROTOCOL_GUID
// Device paths. They map 1:1 with NT envirnment variables. The variables
// define what virtual hardware the emulator/LinuxBusDriver will produce.
//
//
// EFI_LINUX_VIRTUAL_DISKS
//
#define EFI_LINUX_VIRTUAL_DISKS_GUID \
  { \
    0xc95a928, 0xa006, 0x11d4, {0xbc, 0xfa, 0x0, 0x80, 0xc7, 0x3c, 0x88, 0x81} \
  }

extern EFI_GUID gEfiLinuxVirtualDisksGuid;

//
// EFI_LINUX_PHYSICAL_DISKS
//
#define EFI_LINUX_PHYSICAL_DISKS_GUID \
  { \
    0xc95a92f, 0xa006, 0x11d4, {0xbc, 0xfa, 0x0, 0x80, 0xc7, 0x3c, 0x88, 0x81} \
  }

extern EFI_GUID gEfiLinuxPhysicalDisksGuid;

//
// EFI_LINUX_FILE_SYSTEM
//
#define EFI_LINUX_FILE_SYSTEM_GUID \
  { \
    0xc95a935, 0xa006, 0x11d4, {0xbc, 0xfa, 0x0, 0x80, 0xc7, 0x3c, 0x88, 0x81} \
  }

extern EFI_GUID gEfiLinuxFileSystemGuid;

//
// EFI_LINUX_SERIAL_PORT
//
#define EFI_LINUX_SERIAL_PORT_GUID \
  { \
    0xc95a93d, 0xa006, 0x11d4, {0xbc, 0xfa, 0x0, 0x80, 0xc7, 0x3c, 0x88, 0x81} \
  }

extern EFI_GUID gEfiLinuxSerialPortGuid;

//
// EFI_LINUX_UGA
//
#define EFI_LINUX_UGA_GUID \
  { \
    0xab248e99, 0xabe1, 0x11d4, {0xbd, 0xd, 0x0, 0x80, 0xc7, 0x3c, 0x88, 0x81} \
  }

extern EFI_GUID gEfiLinuxUgaGuid;

//
// EFI_LINUX_GOP
//
#define EFI_LINUX_GOP_GUID \
  { \
    0x4e11e955, 0xccca, 0x11d4, {0xbd, 0xd, 0x0, 0x80, 0xc7, 0x3c, 0x88, 0x81} \
  }

extern EFI_GUID gEfiLinuxGopGuid;

//
// EFI_LINUX_CONSOLE
//
#define EFI_LINUX_CONSOLE_GUID \
  { \
    0xba73672c, 0xa5d3, 0x11d4, {0xbd, 0x0, 0x0, 0x80, 0xc7, 0x3c, 0x88, 0x81} \
  }

extern EFI_GUID gEfiLinuxConsoleGuid;

//
// EFI_LINUX_MEMORY
//
#define EFI_LINUX_MEMORY_GUID \
  { \
    0x99042912, 0x122a, 0x11d4, {0xbd, 0xd, 0x0, 0x80, 0xc7, 0x3c, 0x88, 0x81} \
  }

extern EFI_GUID gEfiLinuxMemoryGuid;

//
// EFI_LINUX_CPU_MODEL
//
#define EFI_LINUX_CPU_MODEL_GUID \
  { \
    0xbee9b6ce, 0x2f8a, 0x11d4, {0xbd, 0xd, 0x0, 0x80, 0xc7, 0x3c, 0x88, 0x81} \
  }

extern EFI_GUID gEfiLinuxCPUModelGuid;

//
// EFI_LINUX_CPU_SPEED
//
#define EFI_LINUX_CPU_SPEED_GUID \
  { \
    0xd4f29055, 0xe1fb, 0x11d4, {0xbd, 0xd, 0x0, 0x80, 0xc7, 0x3c, 0x88, 0x81} \
  }

extern EFI_GUID gEfiLinuxCPUSpeedGuid;

//
// EFI_LINUX_PASS_THROUGH
//
#define EFI_LINUX_PASS_THROUGH_GUID \
  { \
    0xcc664eb8, 0x3c24, 0x4086, {0xb6, 0xf6, 0x34, 0xe8, 0x56, 0xbc, 0xe3, 0x6e} \
  }

extern EFI_GUID gEfiLinuxPassThroughGuid;

#endif
