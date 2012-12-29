/*++

Copyright (c) 2005 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name: 

  mm.c
  
Abstract: 

  shell command "mm". Modify memory/IO/PCI/MMIO.

Revision History

--*/

#include "EfiShellLib.h"
#include "mm.h"

extern UINT8  STRING_ARRAY_NAME[];

//
// This is the generated header file which includes whatever needs to be exported (strings + IFR)
//
#include STRING_DEFINES_FILE

#include EFI_PROTOCOL_DEFINITION (PciRootBridgeIo)

typedef enum {
  EfiMemory,
  EFIMemoryMappedIo,
  EfiIo,
  EfiPciConfig,
  EfiPciEConfig
} EFI_ACCESS_TYPE;

EFI_STATUS
DumpIoModify (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

VOID
ReadMem (
  IN  EFI_IO_WIDTH  Width,
  IN  UINT64        Address,
  IN  UINTN         Size,
  IN  VOID          *Buffer
  );

VOID
WriteMem (
  IN  EFI_IO_WIDTH  Width,
  IN  UINT64        Address,
  IN  UINTN         Size,
  IN  VOID          *Buffer
  );

STATIC
BOOLEAN
GetHex (
  IN  UINT16  *str,
  OUT UINT64  *data
  );

//
// Global Variables
//
EFI_HII_HANDLE HiiHandle;
EFI_GUID EfiIomodGuid = EFI_IOMOD_GUID;
SHELL_VAR_CHECK_ITEM    IomodCheckList[] = {
  {
    L"-MMIO",
    0x01,
    0x10E,
    FlagTypeSingle
  },
  {
    L"-MEM",
    0x02,
    0x10D,
    FlagTypeSingle
  },
  {
    L"-IO",
    0x04,
    0x10B,
    FlagTypeSingle
  },
  {
    L"-PCI",
    0x08,
    0x107,
    FlagTypeSingle
  },
  {
    L"-PCIE",
    0x100,
    0x0F,
    FlagTypeSingle
  },
  {
    L"-w",
    0x10,
    0,
    FlagTypeNeedVar
  },
  {
    L"-n",
    0x20,
    0,
    FlagTypeSingle
  },
  {
    L"-b",
    0x40,
    0,
    FlagTypeSingle
  },
  {
    L"-?",
    0x80,
    0,
    FlagTypeSingle
  },
  {
    NULL,
    0,
    0,
    0
  }
};

UINT64 MaxNum[9]      = { 0xff, 0xffff, 0xffffffff, 0xffffffffffffffff };

EFI_BOOTSHELL_CODE(
  EFI_APPLICATION_ENTRY_POINT(DumpIoModify)
)

EFI_STATUS
OldShellParserMM (
  OUT EFI_IO_WIDTH       *Width,
  OUT EFI_ACCESS_TYPE    *AccessType,
  OUT UINT64             *Address,
  OUT UINT64             *Value,
  OUT UINTN              *Size,
  OUT BOOLEAN            *Interactive,
  OUT CHAR16             **ValueStr
  );

EFI_STATUS
GetPciEAddressFromInputAddress (
  IN UINT64                 InputAddress,
  OUT UINT64                *PciEAddress
  );

EFI_STATUS
DumpIoModify (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
/*++

Routine Description:
  Dump or modify MEM/IO/PCI/MMIO.

Arguments:
  ImageHandle     The image handle. 
  SystemTable     The system table.

Returns:
  EFI_SUCCESS             - Command completed successfully
  EFI_INVALID_PARAMETER   - Command usage error

Notes:
  For Shell Mode 1.1.1:
  MM Address [Width] [;[MEM | MMIO | IO | PCI]] [:Value]
    1|2|4|8 supported byte widths - Default is 1 byte. 
    ;MEM = Memory, 
    ;MMIO = Memory Mapped IO, 
    ;IO = in/out, 
    ;PCI = PCI Config space (format: 000000ssbbddffrr)
    Default access type is memory (MEM)

  For Shell Mode 1.1.2:
    MM Address [Width] [-[MEM | MMIO | IO | PCI | PCIE]] [:Value]
    1|2|4|8 supported byte widths - Default is 1 byte. 
    -MEM = Memory, 
    -MMIO = Memory Mapped IO, 
    -IO = in/out, 
    -PCI = PCI Config space (format: 000000ssbbddffrr)
    -PCIE = PCIE Config space (format: 000000ssbbddffrrr)
    Default access type is memory (MEM)

--*/
{
  EFI_STATUS                      Status;
  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL *IoDev;
  UINT64                          Address;
  UINT64                          PciEAddress;
  UINT64                          Value;
  UINT32                          SegmentNumber;
  EFI_IO_WIDTH                    Width;
  EFI_ACCESS_TYPE                 AccessType;
  UINT64                          Buffer;
  UINTN                           Index;
  UINTN                           Size;
  CHAR16                          *AddressStr;
  CHAR16                          *ValueStr;
  BOOLEAN                         Complete;
  CHAR16                          InputStr[80];
  BOOLEAN                         Interactive;
  EFI_HANDLE                      *HandleBuffer;
  UINTN                           BufferSize;

  SHELL_VAR_CHECK_CODE            RetCode;
  CHAR16                          *Useful;
  SHELL_ARG_LIST                  *Item;
  UINTN                           ItemValue;
  SHELL_VAR_CHECK_PACKAGE         ChkPck;

  Address       = 0;
  PciEAddress   = 0;
  IoDev         = NULL;
  HandleBuffer  = NULL;
  BufferSize    = 0;
  SegmentNumber = 0;
  ZeroMem (&ChkPck, sizeof (SHELL_VAR_CHECK_PACKAGE));

  EFI_SHELL_APP_INIT (ImageHandle, SystemTable);
  
  //
  // Enable tab key which can pause the output
  //
  EnableOutputTabPause();

  Status = LibInitializeStrings (&HiiHandle, STRING_ARRAY_NAME, &EfiIomodGuid);

  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (!EFI_PROPER_VERSION (1, 10)) {
    PrintToken (
      STRING_TOKEN (STR_SHELLENV_GNC_COMMAND_NOT_SUPPORT),
      HiiHandle,
      L"mm",
      EFI_VERSION_1_10
      );
    Status = EFI_UNSUPPORTED;
    goto Done;
  }

  if (IS_OLD_SHELL) {
    Status = OldShellParserMM (&Width, &AccessType, &Address, &Value, &Size, &Interactive, &ValueStr);
    if (EFI_ERROR (Status)) {
      if (-1 == Status) {
        PrintToken (STRING_TOKEN (STR_HELPINFO_MM_VERBOSEHELP), HiiHandle);
        Status = EFI_SUCCESS;
      }
      goto Done;
    }
  } else {
    //
    // Parse arguments
    //
    Width       = EfiPciWidthUint8;
    Size        = 1;
    AccessType  = EfiMemory;
    AddressStr  = NULL;
    ValueStr    = NULL;
    Interactive = TRUE;

    RetCode     = LibCheckVariables (SI, IomodCheckList, &ChkPck, &Useful);
    if (VarCheckOk != RetCode) {
      switch (RetCode) {
      case VarCheckUnknown:
        PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_UNKNOWN_FLAG), HiiHandle, L"mm", Useful);
        break;

      case VarCheckConflict:
        PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_FLAG_CONFLICT), HiiHandle, L"mm", Useful);
        break;

      case VarCheckDuplicate:
        PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_DUP_FLAG), HiiHandle, L"mm", Useful);
        break;

      case VarCheckLackValue:
        PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_LACK_ARG), HiiHandle, L"mm", Useful);
        break;

      default:
        break;
      }

      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }
    //
    // Out put help.
    //
    if (LibCheckVarGetFlag (&ChkPck, L"-b") != NULL) {
      EnablePageBreak (DEFAULT_INIT_ROW, DEFAULT_AUTO_LF);
    }

    if (LibCheckVarGetFlag (&ChkPck, L"-?") != NULL) {
      if (ChkPck.ValueCount > 0 ||
          ChkPck.FlagCount > 2 ||
          (2 == ChkPck.FlagCount && !LibCheckVarGetFlag (&ChkPck, L"-b"))
          ) {
        PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiHandle, L"mv");
        Status = EFI_INVALID_PARAMETER;
      } else {
        PrintToken (STRING_TOKEN (STR_HELPINFO_MM_VERBOSEHELP), HiiHandle);
        Status = EFI_SUCCESS;
      }
      goto Done;
    }

    if (ChkPck.ValueCount < 1) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_FEW), HiiHandle, L"mm");
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }

    if (ChkPck.ValueCount > 2) {
      PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_TOO_MANY), HiiHandle, L"mm");
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }

    if (LibCheckVarGetFlag (&ChkPck, L"-MMIO") != NULL) {
      AccessType = EFIMemoryMappedIo;
    } else if (LibCheckVarGetFlag (&ChkPck, L"-MEM") != NULL) {
      AccessType = EfiMemory;
    } else if (LibCheckVarGetFlag (&ChkPck, L"-IO") != NULL) {
      AccessType = EfiIo;
    } else if (LibCheckVarGetFlag (&ChkPck, L"-PCI") != NULL) {
      AccessType = EfiPciConfig;
    } else if (LibCheckVarGetFlag (&ChkPck, L"-PCIE") != NULL) {
      AccessType = EfiPciEConfig;
    }

    if (LibCheckVarGetFlag (&ChkPck, L"-n") != NULL) {
      Interactive = FALSE;
    }

    Item = LibCheckVarGetFlag (&ChkPck, L"-w");
    if (NULL != Item) {
      ItemValue = (UINTN) StrToUInteger (Item->VarStr, &Status);
      if (EFI_ERROR (Status)) {
        PrintToken (STRING_TOKEN (STR_IOMOD_INVALID_WIDTH), HiiHandle, L"mm");
        Status = EFI_INVALID_PARAMETER;
        goto Done;
      }

      switch (ItemValue) {
      case 1:
        Width = EfiPciWidthUint8;
        Size  = 1;
        break;

      case 2:
        Width = EfiPciWidthUint16;
        Size  = 2;
        break;

      case 4:
        Width = EfiPciWidthUint32;
        Size  = 4;
        break;

      case 8:
        Width = EfiPciWidthUint64;
        Size  = 8;
        break;

      default:
        PrintToken (STRING_TOKEN (STR_IOMOD_INVALID_WIDTH), HiiHandle, L"mm");
        Status = EFI_INVALID_PARAMETER;
        goto Done;
      }
    }

    Item = ChkPck.VarList;
    if (NULL != Item) {
      Address = (UINT64) (StrToUIntegerBase (Item->VarStr, 16, &Status));
      if (EFI_ERROR (Status)) {
        PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_ARG), HiiHandle, L"mm", Item->VarStr);
        Status = EFI_INVALID_PARAMETER;
        goto Done;
      }

      Item = Item->Next;
    }

    if (NULL != Item) {
      Value = (UINT64) (StrToUIntegerBase (Item->VarStr, 16, &Status));
      if (EFI_ERROR (Status)) {
        PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_ARG), HiiHandle, L"mm", Item->VarStr);
        Status = EFI_INVALID_PARAMETER;
        goto Done;
      } else {
        switch (Size) {
        case 1:
          if (Value > 0xFF) {
            Status = EFI_INVALID_PARAMETER;
          }
          break;

        case 2:
          if (Value > 0xFFFF) {
            Status = EFI_INVALID_PARAMETER;
          }
          break;

        case 4:
          if (Value > 0xFFFFFFFF) {
            Status = EFI_INVALID_PARAMETER;
          }
          break;

        default:
          break;
        }
      }

      if (EFI_ERROR (Status)) {
        PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_INVALID_ARG), HiiHandle, L"mm", Item->VarStr);
        Status = EFI_INVALID_PARAMETER;
        goto Done;
      }

      ValueStr = Item->VarStr;
    }
  }

  if ((Address & (Size - 1)) != 0) {
    PrintToken (STRING_TOKEN (STR_IOMOD_ADDRESS_NOT_ALIGNED), HiiHandle, L"mm", (UINTN) Address, Size);
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }
  //
  // locate DeviceIO protocol interface
  //
  if (AccessType != EfiMemory) {
    Status = LibLocateHandle (
               ByProtocol,
               &gEfiPciRootBridgeIoProtocolGuid,
               NULL,
               &BufferSize,
               &HandleBuffer
               );
    if (EFI_ERROR (Status)) {
      PrintToken (STRING_TOKEN (STR_IOMOD_HANDLE_PCI), HiiHandle, L"mm", Status);
      goto Done;
    }
    //
    // In the case of PCI or PCIE
    // Get segment number and mask the segment bits in Address
    //
    if (AccessType == EfiPciEConfig) {
      SegmentNumber = (UINT32) RShiftU64 (Address, 36) & 0xff;
      Address      &= 0xfffffffff;
    } else {
      if (AccessType == EfiPciConfig) {
        SegmentNumber = (UINT32) RShiftU64 (Address, 32) & 0xff;
        Address      &= 0xffffffff;
      }
    }
    //
    // Find the EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL of the specified segment number
    //
    for (Index = 0; Index < BufferSize; Index++) {
      Status = BS->HandleProtocol (
                     HandleBuffer[Index],
                     &gEfiPciRootBridgeIoProtocolGuid,
                     (VOID *) &IoDev
                     );
      if (EFI_ERROR (Status)) {
        continue;
      }
      if (IoDev->SegmentNumber != SegmentNumber) {
        IoDev = NULL;
      }
    }
    if (IoDev == NULL) {
      // TODO add token
      PrintToken (STRING_TOKEN (STR_IOMOD_SEGMENT_NOT_FOUND), HiiHandle, SegmentNumber);
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }
  }

  if (AccessType == EfiIo && Address + Size > 0x10000) {
    PrintToken (STRING_TOKEN (STR_IOMOD_IO_ADDRESS), HiiHandle, L"mm");
    Status = EFI_INVALID_PARAMETER;
    goto Done;
  }

  if (AccessType == EfiPciEConfig) {
    GetPciEAddressFromInputAddress (Address, &PciEAddress);
  }

  //
  // Set value
  //
  if (ValueStr) {
    if (AccessType == EFIMemoryMappedIo) {
      IoDev->Mem.Write (IoDev, Width, Address, 1, &Value);
    } else if (AccessType == EfiIo) {
      IoDev->Io.Write (IoDev, Width, Address, 1, &Value);
    } else if (AccessType == EfiPciConfig) {
      IoDev->Pci.Write (IoDev, Width, Address, 1, &Value);
    } else if (AccessType == EfiPciEConfig) {
      IoDev->Pci.Write (IoDev, Width, PciEAddress, 1, &Buffer);
    } else {
      WriteMem (Width, Address, 1, &Value);
    }

    Status = EFI_SUCCESS;
    goto Done;
  }
  //
  // non-interactive mode
  //
  if (Interactive == FALSE) {
    Buffer = 0;
    if (AccessType == EFIMemoryMappedIo) {
      PrintToken (STRING_TOKEN (STR_IOMOD_HMMIO), HiiHandle);
      IoDev->Mem.Read (IoDev, Width, Address, 1, &Buffer);
    } else if (AccessType == EfiIo) {
      PrintToken (STRING_TOKEN (STR_IOMOD_HIO), HiiHandle);
      IoDev->Io.Read (IoDev, Width, Address, 1, &Buffer);
    } else if (AccessType == EfiPciConfig) {
      PrintToken (STRING_TOKEN (STR_IOMOD_HPCI), HiiHandle);
      IoDev->Pci.Read (IoDev, Width, Address, 1, &Buffer);
    } else if (AccessType == EfiPciEConfig) {
      PrintToken (STRING_TOKEN (STR_IOMOD_HPCIE), HiiHandle);
      IoDev->Pci.Read (IoDev, Width, PciEAddress, 1, &Buffer);
    } else {
      PrintToken (STRING_TOKEN (STR_IOMOD_HMEM), HiiHandle);
      ReadMem (Width, Address, 1, &Buffer);
    }

    PrintToken (STRING_TOKEN (STR_IOMOD_ADDRESS), HiiHandle, Address);
    if (Size == 1) {
      PrintToken (STRING_TOKEN (STR_IOMOD_BUFFER_2), HiiHandle, Buffer);
    } else if (Size == 2) {
      PrintToken (STRING_TOKEN (STR_IOMOD_BUFFER_4), HiiHandle, Buffer);
    } else if (Size == 4) {
      PrintToken (STRING_TOKEN (STR_IOMOD_BUFFER_8), HiiHandle, Buffer);
    } else if (Size == 8) {
      PrintToken (STRING_TOKEN (STR_IOMOD_BUFFER_16), HiiHandle, Buffer);
    }

    Print (L"\n");
    Status = EFI_SUCCESS;
    goto Done;
  }
  //
  // interactive mode
  //
  Complete = FALSE;
  do {
    if (AccessType == EfiIo && Address + Size > 0x10000) {
      PrintToken (STRING_TOKEN (STR_IOMOD_IO_ADDRESS_2), HiiHandle, L"mm");
      break;
    }
    
    Buffer = 0;
    if (AccessType == EFIMemoryMappedIo) {
      PrintToken (STRING_TOKEN (STR_IOMOD_HMMIO), HiiHandle);
      IoDev->Mem.Read (IoDev, Width, Address, 1, &Buffer);
    } else if (AccessType == EfiIo) {
      PrintToken (STRING_TOKEN (STR_IOMOD_HIO), HiiHandle);
      IoDev->Io.Read (IoDev, Width, Address, 1, &Buffer);
    } else if (AccessType == EfiPciConfig) {
      PrintToken (STRING_TOKEN (STR_IOMOD_HPCI), HiiHandle);
      IoDev->Pci.Read (IoDev, Width, Address, 1, &Buffer);
    } else if (AccessType == EfiPciEConfig) {
      PrintToken (STRING_TOKEN (STR_IOMOD_HPCIE), HiiHandle);
      IoDev->Pci.Read (IoDev, Width, PciEAddress, 1, &Buffer);
    } else {
      PrintToken (STRING_TOKEN (STR_IOMOD_HMEM), HiiHandle);
      ReadMem (Width, Address, 1, &Buffer);
    }

    PrintToken (STRING_TOKEN (STR_IOMOD_ADDRESS), HiiHandle, Address);

    if (Size == 1) {
      PrintToken (STRING_TOKEN (STR_IOMOD_BUFFER_2), HiiHandle, Buffer);
    } else if (Size == 2) {
      PrintToken (STRING_TOKEN (STR_IOMOD_BUFFER_4), HiiHandle, Buffer);
    } else if (Size == 4) {
      PrintToken (STRING_TOKEN (STR_IOMOD_BUFFER_8), HiiHandle, Buffer);
    } else if (Size == 8) {
      PrintToken (STRING_TOKEN (STR_IOMOD_BUFFER_16), HiiHandle, Buffer);
    }
    //
    // wait user input to modify
    //
    Input (L" > ", InputStr, sizeof (InputStr) / sizeof (CHAR16));

    //
    // skip space characters
    //
    for (Index = 0; InputStr[Index] == ' '; Index++)
      ;

    //
    // parse input string
    //
    if (InputStr[Index] == '.' || InputStr[Index] == 'q' || InputStr[Index] == 'Q') {
      Complete = TRUE;
    } else if (InputStr[Index] == CHAR_NULL) {
      //
      // Continue to next address
      //
    } else if (GetHex (InputStr + Index, &Buffer) && Buffer <= MaxNum[Width]) {
      if (AccessType == EFIMemoryMappedIo) {
        IoDev->Mem.Write (IoDev, Width, Address, 1, &Buffer);
      } else if (AccessType == EfiIo) {
        IoDev->Io.Write (IoDev, Width, Address, 1, &Buffer);
      } else if (AccessType == EfiPciConfig) {
        IoDev->Pci.Write (IoDev, Width, Address, 1, &Buffer);
      } else if (AccessType == EfiPciEConfig) {
        IoDev->Pci.Write (IoDev, Width, PciEAddress, 1, &Buffer);
      } else {
        WriteMem (Width, Address, 1, &Buffer);
      }
    } else {
      PrintToken (STRING_TOKEN (STR_IOMOD_ERROR), HiiHandle);
      continue;
    }

    Address += Size;
    if (AccessType == EfiPciEConfig) {
      GetPciEAddressFromInputAddress (Address, &PciEAddress);
    }
    Print (L"\n");
  } while (!Complete);

  Status = EFI_SUCCESS;
Done:

  if (HandleBuffer != NULL) {
    FreePool (HandleBuffer);
  }
  LibCheckVarFreeVarList (&ChkPck);
  LibUnInitializeStrings ();
  return Status;
}

VOID
ReadMem (
  IN  EFI_IO_WIDTH  Width,
  IN  UINT64        Address,
  IN  UINTN         Size,
  IN  VOID          *Buffer
  )
{
  do {
    if (Width == EfiPciWidthUint8) {
      *(UINT8 *) Buffer = *(UINT8 *) (UINTN) Address;
      Address -= 1;
    } else if (Width == EfiPciWidthUint16) {
      *(UINT16 *) Buffer = *(UINT16 *) (UINTN) Address;
      Address -= 2;
    } else if (Width == EfiPciWidthUint32) {
      *(UINT32 *) Buffer = *(UINT32 *) (UINTN) Address;
      Address -= 4;
    } else if (Width == EfiPciWidthUint64) {
      *(UINT64 *) Buffer = *(UINT64 *) (UINTN) Address;
      Address -= 8;
    } else {
      PrintToken (STRING_TOKEN (STR_IOMOD_READ_MEM_ERROR), HiiHandle);
      break;
    }
    //
    //
    //
    Size--;
  } while (Size > 0);
}

VOID
WriteMem (
  IN  EFI_IO_WIDTH  Width,
  IN  UINT64        Address,
  IN  UINTN         Size,
  IN  VOID          *Buffer
  )
{
  do {
    if (Width == EfiPciWidthUint8) {
      *(UINT8 *) (UINTN) Address = *(UINT8 *) Buffer;
      Address += 1;
    } else if (Width == EfiPciWidthUint16) {
      *(UINT16 *) (UINTN) Address = *(UINT16 *) Buffer;
      Address += 2;
    } else if (Width == EfiPciWidthUint32) {
      *(UINT32 *) (UINTN) Address = *(UINT32 *) Buffer;
      Address += 4;
    } else if (Width == EfiPciWidthUint64) {
      *(UINT64 *) (UINTN) Address = *(UINT64 *) Buffer;
      Address += 8;
    } else {
      ASSERT (FALSE);
    }
    //
    //
    //
    Size--;
  } while (Size > 0);
}

STATIC
BOOLEAN
GetHex (
  IN  UINT16  *str,
  OUT UINT64  *data
  )
{
  UINTN   u;
  CHAR16  c;
  BOOLEAN Find;

  Find = FALSE;
  //
  // convert hex digits
  //
  u = 0;
  c = *(str++);
  while (c) {
    if (c >= 'a' && c <= 'f') {
      c -= 'a' - 'A';
    }

    if (c == ' ') {
      break;
    }

    if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F')) {
      u     = u << 4 | c - (c >= 'A' ? 'A' - 10 : '0');

      Find  = TRUE;
    } else {
      return FALSE;
    }

    c = *(str++);
  }

  *data = u;
  return Find;
}

EFI_STATUS
OldShellParserMM (
  OUT EFI_IO_WIDTH       *Width,
  OUT EFI_ACCESS_TYPE    *AccessType,
  OUT UINT64             *Address,
  OUT UINT64             *Value,
  OUT UINTN              *Size,
  OUT BOOLEAN            *Interactive,
  OUT CHAR16             **ValueStr
  )
/*++

Routine Description:

  Parse the command line with old shell command grammar

Arguments:

  Width              - The memory width
  AccessType         - The memory access type
  Address            - The memory address
  Value              - The memory value
  Size               - The memory size
  Interactive        - Interactive
  ValueStr           - The value string

Returns:

  EFI_SUCCESS   - Success

--*/
{
  CHAR16  *AddressStr;
  CHAR16  *WidthStr;
  CHAR16  *Ptr;
  UINTN   Index;
  BOOLEAN PrtHelp;
  //
  // Parse arguments
  //
  *Width                = EfiPciWidthUint8;
  *Size                 = 1;
  *AccessType           = EfiMemory;
  AddressStr            = WidthStr = NULL;
  *ValueStr             = NULL;
  *Interactive          = TRUE;
  PrtHelp               = FALSE;
  for (Index = 1; Index < SI->Argc; Index += 1) {
    Ptr = SI->Argv[Index];
    //
    // access type
    //
    if (*Ptr == L';') {
      if (StriCmp (Ptr, L";IO") == 0) {
        *AccessType = EfiIo;
        continue;
      } else if (StriCmp (Ptr, L";PCI") == 0) {
        *AccessType = EfiPciConfig;
        continue;
      } else if (StriCmp (Ptr, L";MEM") == 0) {
        *AccessType = EfiMemory;
        continue;
      } else if (StriCmp (Ptr, L";MMIO") == 0) {
        *AccessType = EFIMemoryMappedIo;
        continue;
      } else {
        PrintToken (STRING_TOKEN (STR_IOMOD_UNKNOWN_ACCESS), HiiHandle, L"mm", Ptr);
        return EFI_INVALID_PARAMETER;
      }
    } else if (*Ptr == ':') {
      //
      // value
      //
      *ValueStr = &Ptr[1];
      *Value    = Xtoi (*ValueStr);
      continue;
    } else if (*Ptr == '-') {
      //
      // command option switch
      //
      switch (Ptr[1]) {
      case 'n':
      case 'N':
        *Interactive = FALSE;
        break;

      case '?':
        PrtHelp = TRUE;
        break;

      case 'b':
      case 'B':
        EnablePageBreak (DEFAULT_INIT_ROW, DEFAULT_AUTO_LF);
        break;

      default:
        PrintToken (STRING_TOKEN (STR_SHELLENV_GNC_UNKNOWN_FLAG), HiiHandle, L"mm", Ptr);
        return EFI_INVALID_PARAMETER;
      };
      continue;
    }

    //
    // Start Address
    //
    if (!AddressStr) {
      AddressStr  = Ptr;
      *Address    = Xtoi (AddressStr);
      continue;
    }
    //
    // access width
    //
    if (!WidthStr) {
      WidthStr = Ptr;
      switch (Xtoi (WidthStr)) {
      case 1:
        *Width  = EfiPciWidthUint8;
        *Size   = 1;
        continue;

      case 2:
        *Width  = EfiPciWidthUint16;
        *Size   = 2;
        continue;

      case 4:
        *Width  = EfiPciWidthUint32;
        *Size   = 4;
        continue;

      case 8:
        *Width  = EfiPciWidthUint64;
        *Size   = 8;
        continue;

      default:
        PrintToken (STRING_TOKEN (STR_IOMOD_INVALID_WIDTH), HiiHandle, L"mm");
        return EFI_INVALID_PARAMETER;
      }
    }
  }

  if (PrtHelp) {
    return (EFI_STATUS) -1;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
InitializeMMGetLineHelp (
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
  return LibCmdGetStringByToken (
          STRING_ARRAY_NAME,
          &EfiIomodGuid,
          STRING_TOKEN (STR_HELPINFO_MM_LINEHELP),
          Str
          );
}

EFI_STATUS
GetPciEAddressFromInputAddress (
  IN UINT64                 InputAddress,
  OUT UINT64                *PciEAddress
  )
/*++

Routine Description:

  Get the PCI-E Address from a PCI address format 0x0000ssbbddffrrr
  where ss is SEGMENT, bb is BUS, dd is DEVICE, ff is FUNCTION
  and rrr is REGISTER (extension format for PCI-E)

Arguments:

  InputAddress - PCI address format on input
  PciEAddress  - PCI-E address extention format

Returns:

  EFI_SUCCESS   - Success

--*/
{
  *PciEAddress = RShiftU64(InputAddress & ~(UINT64) 0xFFF, 4) +
    LShiftU64((UINT16) InputAddress & 0x0FFF, 32);
  return EFI_SUCCESS;
}
