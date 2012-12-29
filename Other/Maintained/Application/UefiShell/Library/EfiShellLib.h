/*++

Copyright (c) 2005 - 2007, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:
  
    EfiShellLib.h

Abstract:

    EFI Shell library functions
    Header file for EfiShellLib.c

Revision History

--*/
#ifndef _EFI_SHELL_LIB_INCLUDE_
#define _EFI_SHELL_LIB_INCLUDE_

#include "Tiano.h"
#include "shellenv.h"
#include "Pci22.h"

#include EFI_ARCH_PROTOCOL_DEFINITION (Bds)
#include EFI_ARCH_PROTOCOL_DEFINITION (Cpu)
#include EFI_ARCH_PROTOCOL_DEFINITION (Metronome)
#include EFI_ARCH_PROTOCOL_DEFINITION (RealTimeClock)
#include EFI_ARCH_PROTOCOL_DEFINITION (Reset)
#include EFI_ARCH_PROTOCOL_DEFINITION (Runtime)
#include EFI_ARCH_PROTOCOL_DEFINITION (StatusCode)
#include EFI_ARCH_PROTOCOL_DEFINITION (Timer)
#include EFI_ARCH_PROTOCOL_DEFINITION (Variable)
#include EFI_ARCH_PROTOCOL_DEFINITION (VariableWrite)
#include EFI_ARCH_PROTOCOL_DEFINITION (WatchdogTimer)
#include EFI_GUID_DEFINITION (Acpi)
#include EFI_GUID_DEFINITION (ConsoleInDevice)
#include EFI_GUID_DEFINITION (ConsoleOutDevice)
#include EFI_GUID_DEFINITION (DxeServices)
#include EFI_GUID_DEFINITION (GlobalVariable)
#include EFI_GUID_DEFINITION (GenericVariable)
#include EFI_GUID_DEFINITION (Gpt)
#include EFI_GUID_DEFINITION (Mps)
#include EFI_GUID_DEFINITION (PrimaryConsoleInDevice)
#include EFI_GUID_DEFINITION (PrimaryConsoleOutDevice)
#include EFI_GUID_DEFINITION (PrimaryStandardErrorDevice)
#include EFI_GUID_DEFINITION (SalSystemTable)
#include EFI_GUID_DEFINITION (SmBios)
#include EFI_GUID_DEFINITION (StandardErrorDevice)
#include EFI_GUID_DEFINITION (FrameworkDevicePath)
#include EFI_PROTOCOL_DEFINITION (BlockIo)
#include EFI_PROTOCOL_DEFINITION (BusSpecificDriverOverride)
#include EFI_PROTOCOL_DEFINITION (ComponentName)
#include EFI_PROTOCOL_DEFINITION (ConsoleControl)
#include EFI_PROTOCOL_DEFINITION (CpuIo)
#include EFI_PROTOCOL_DEFINITION (DataHub)
#include EFI_PROTOCOL_DEFINITION (DebugPort)
#include EFI_PROTOCOL_DEFINITION (DebugSupport)
#include EFI_PROTOCOL_DEFINITION (Decompress)
#include EFI_PROTOCOL_DEFINITION (DeviceIo)
#include EFI_PROTOCOL_DEFINITION (DevicePath)
#include EFI_PROTOCOL_DEFINITION (DiskIo)
#include EFI_PROTOCOL_DEFINITION (DriverBinding)
#include EFI_PROTOCOL_DEFINITION (DriverConfiguration)
#include EFI_PROTOCOL_DEFINITION (DriverDiagnostics)
#include EFI_PROTOCOL_DEFINITION (Ebc)
#include EFI_PROTOCOL_DEFINITION (EfiNetworkInterfaceIdentifier)
#include EFI_PROTOCOL_DEFINITION (FileSystemInfo)
#include EFI_PROTOCOL_DEFINITION (FileSystemVolumeLabelInfo)
#include EFI_PROTOCOL_DEFINITION (FirmwareVolume)
#include EFI_PROTOCOL_DEFINITION (FirmwareVolumeBlock)
#include EFI_PROTOCOL_DEFINITION (FormBrowser)
#include EFI_PROTOCOL_DEFINITION (FormCallback)
#include EFI_PROTOCOL_DEFINITION (Hii)
#include EFI_PROTOCOL_DEFINITION (IsaAcpi)
#include EFI_PROTOCOL_DEFINITION (IsaIo)
#include EFI_PROTOCOL_DEFINITION (LoadedImage)
#include EFI_PROTOCOL_DEFINITION (LoadFile)
#include EFI_PROTOCOL_DEFINITION (PciHostBridgeResourceAllocation)
#include EFI_PROTOCOL_DEFINITION (PciIo)
#include EFI_PROTOCOL_DEFINITION (PciRootBridgeIo)
#include EFI_PROTOCOL_DEFINITION (PxeBaseCode)
#include EFI_PROTOCOL_DEFINITION (PxeBaseCodeCallBack)
#include EFI_PROTOCOL_DEFINITION (ScsiIo)
#include EFI_PROTOCOL_DEFINITION (ScsiPassThru)
#include EFI_PROTOCOL_DEFINITION (SectionExtraction)
#include EFI_PROTOCOL_DEFINITION (SerialIo)
#include EFI_PROTOCOL_DEFINITION (SimpleFileSystem)
#include EFI_PROTOCOL_DEFINITION (SimpleNetwork)
#include EFI_PROTOCOL_DEFINITION (SimplePointer)
#include EFI_PROTOCOL_DEFINITION (SimpleTextOut)
#include EFI_PROTOCOL_DEFINITION (Tcp)
#include EFI_PROTOCOL_DEFINITION (GraphicsOutput)
#include EFI_PROTOCOL_DEFINITION (EdidDiscovered)
#include EFI_PROTOCOL_DEFINITION (EdidActive)
#include EFI_PROTOCOL_DEFINITION (EdidOverride)
#include EFI_PROTOCOL_DEFINITION (UgaDraw)
#include EFI_PROTOCOL_DEFINITION (UgaIo)
#include EFI_PROTOCOL_DEFINITION (UgaSplash)
#include EFI_PROTOCOL_DEFINITION (UnicodeCollation)
#include EFI_PROTOCOL_DEFINITION (UsbHostController)
#include EFI_PROTOCOL_DEFINITION (UsbIo)
#include EFI_PROTOCOL_DEFINITION (VariableStore)
#include EFI_PROTOCOL_DEFINITION (UsbHostController)
#include EFI_PROTOCOL_DEFINITION (ScsiPassThruExt)
#include EFI_PROTOCOL_DEFINITION (ManagedNetwork)
#include EFI_PROTOCOL_DEFINITION (Arp)
#include EFI_PROTOCOL_DEFINITION (Dhcp4)
#include EFI_PROTOCOL_DEFINITION (Tcp4)
#include EFI_PROTOCOL_DEFINITION (Ip4)
#include EFI_PROTOCOL_DEFINITION (Ip4Config)
#include EFI_PROTOCOL_DEFINITION (Udp4)
#include EFI_PROTOCOL_DEFINITION (Mtftp4)
#include EFI_PROTOCOL_DEFINITION (DevicePath)

#include "efilibplat.h"
#include "EfiPart.h"
#include "ShellDebug.h"
#include "CRC.h"
#include "DPath.h"
#include "Event.h"
#include "FileIO.h"
#include "Handle.h"
#include "IO.h"
#include "LinkedList.h"
#include "Lock.h"
#include "Mem.h"
#include "Misc.h"
#include "ShellEnvInt.h"
#include "Str.h"
#include "VarCheck.h"
#include "ConsistMapping.h"

//
//  Environment variable name constants
//
#define VarLanguageCodes    L"LangCodes"
#define VarLanguage         L"Lang"
#define VarTimeout          L"Timeout"
#define VarConsoleIn        L"ConIn"
#define VarConsoleOut       L"ConOut"
#define VarErrorOut         L"ErrOut"
#define VarBootOption       L"Boot%04x"
#define VarBootOrder        L"BootOrder"
#define VarBootNext         L"BootNext"
#define VarBootCurrent      L"BootCurrent"
#define VarDriverOption     L"Driver%04x"
#define VarDriverOrder      L"DriverOrder"
#define VarConsoleInpDev    L"ConInDev"
#define VarConsoleOutDev    L"ConOutDev"
#define VarErrorOutDev      L"ErrOutDev"
#define LanguageCodeEnglish "eng"

#define ISO_639_2_ENTRY_SIZE  3

#define ALIGN_SIZE(a) ((a % MIN_ALIGNMENT_SIZE) ? MIN_ALIGNMENT_SIZE - (a % MIN_ALIGNMENT_SIZE) : 0)

#define EFI_PROPER_VERSION(MajorVer, MinorVer)  (BS->Hdr.Revision >= (MajorVer << 16 | MinorVer))
#define EFI_VERSION_0_99   L"0.99"
#define EFI_VERSION_1_10   L"1.10"

#define EFI_REDIRECTION_SAME        EFIERR (1000)
#define EFI_REDIRECTION_NOT_ALLOWED EFIERR (1001)

//
// The definitions of the key filter flag
//
#define EFI_OUTPUT_SCROLL   0x00000001
#define EFI_OUTPUT_PAUSE    0x00000002
#define EFI_EXECUTION_BREAK 0x00000004

#define EFI_HEX_DISP_SIZE   32

//
//  Global variables
//
extern EFI_SYSTEM_TABLE         *ST;
extern EFI_BOOT_SERVICES        *BS;
extern EFI_RUNTIME_SERVICES     *RT;
extern EFI_SHELL_INTERFACE      *SI;
extern EFI_SHELL_ENVIRONMENT    *SE;
extern EFI_SHELL_ENVIRONMENT2   *SE2;

extern BOOLEAN                        ShellLibInitialized;
extern EFI_UNICODE_COLLATION_PROTOCOL *UnicodeInterface;
extern EFI_DEVICE_PATH_PROTOCOL       LibEndDevicePath[];

//
// shell envionment protocol and interface protocol
//
extern EFI_GUID                 ShellInterfaceProtocol;
extern EFI_GUID                 ShellEnvProtocol;

//
// protocol GUIDs and other miscellaneous GUIDs
//
extern EFI_GUID                 NullGuid;
extern EFI_GUID                 UnknownDeviceGuid;
//
// Memory allocation type
//
extern EFI_MEMORY_TYPE          PoolAllocationType;
extern UINTN                    EFIDebug;

//
//  debug globals
//
extern EFI_SIMPLE_TEXT_OUT_PROTOCOL   *LibRuntimeDebugOut;

//
// Initialization Functions
//
#define EFI_SHELL_APP_INIT(ImageHandle, SystemTable) \
  if (EFI_ERROR (LibInitializeShellApplication (ImageHandle, SystemTable)) \
      ) { \
    return EFI_ABORTED; \
  }

#define EFI_SHELL_STR_INIT(HiiHandle, ArrayName, Guid) \
  if (EFI_ERROR (LibInitializeStrings (&(HiiHandle), (ArrayName), &(Guid))) \
      ) { \
    return EFI_ABORTED; \
  }
  
VOID
InitializeShellLib (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
LibInitializeStrings (
  OUT EFI_HII_HANDLE    *HiiLibHandle,
  IN UINT8              *StringPack,
  IN EFI_GUID           *StringPackGuid
  );

EFI_STATUS
LibUnInitializeStrings (
  VOID
  );

EFI_STATUS
LibInitializeShellApplication (
  IN EFI_HANDLE                   ImageHandle,
  IN EFI_SYSTEM_TABLE             *SystemTable
  );

EFI_STATUS
InitializeUnicodeSupport (
  CHAR8 *LangCode
  );

//
// Math Functions
//
UINT64
LShiftU64 (
  IN UINT64   Operand,
  IN UINTN    Count
  );

UINT64
RShiftU64 (
  IN UINT64   Operand,
  IN UINTN    Count
  );

UINT64
MultU64x32 (
  IN UINT64   Multiplicand,
  IN UINTN    Multiplier
  );

UINT64
DivU64x32 (
  IN UINT64   Dividend,
  IN UINTN    Divisor,
  OUT UINTN   *Remainder OPTIONAL
  );

#endif
