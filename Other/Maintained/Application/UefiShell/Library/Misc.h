/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  Misc.c

Abstract:

  Misc functions

Revision History

--*/
#ifndef _SHELL_MISC_H
#define _SHELL_MISC_H

#define IS_OLD_SHELL  (StrCmp (GetShellMode (), EFI_SHELL_COMPATIBLE_MODE_VER) == 0)

EFI_STATUS
LibGetSystemConfigurationTable (
  IN EFI_GUID *TableGuid,
  IN OUT VOID **Table
  );

EFI_STATUS
LibExtractDataFromHiiHandle (
  IN      EFI_HII_HANDLE      HiiHandle,
  IN OUT  UINT16              *ImageLength,
  OUT     UINT8               *DefaultImage,
  OUT     EFI_GUID            *Guid
  );

EFI_STATUS
LibGetHiiInterface (
  OUT     EFI_HII_PROTOCOL    **Hii
  );

VOID                                  *
LibGetVariable (
  IN CHAR16               *Name,
  IN EFI_GUID             *VendorGuid
  );

VOID                                  *
LibGetVariableAndSize (
  IN CHAR16               *Name,
  IN EFI_GUID             *VendorGuid,
  OUT UINTN               *VarSize
  );

void
ValueToHex (
  IN CHAR16   *Buffer,
  IN UINT64   v
  );

VOID
ValueToString (
  IN CHAR16   *Buffer,
  IN BOOLEAN  Comma,
  IN INT64    v
  );

VOID
GuidToString (
  OUT CHAR16      *Buffer,
  IN EFI_GUID     *Guid
  );

VOID
TimeToString (
  OUT CHAR16      *Buffer,
  IN EFI_TIME     *Time
  );

VOID
StatusToString (
  OUT CHAR16          *Buffer,
  IN EFI_STATUS       Status
  );

EFI_STATUS
WaitForSingleEvent (
  IN EFI_EVENT                  Event,
  IN UINT64                     Timeout OPTIONAL
  );

INTN
CompareGuid (
  IN EFI_GUID     *Guid1,
  IN EFI_GUID     *Guid2
  );

VOID
ValueToHex (
  IN CHAR16                         *Buffer,
  IN UINT64                         v
  );

EFI_MEMORY_DESCRIPTOR             *
LibMemoryMap (
  OUT UINTN                         *NoEntries,
  OUT UINTN                         *MapKey,
  OUT UINTN                         *DescriptorSize,
  OUT UINT32                        *DescriptorVersion
  );

VOID                              *
LibGetVariable (
  IN CHAR16                         *Name,
  IN EFI_GUID                       *VendorGuid
  );

VOID                              *
LibGetVariableAndSize (
  IN CHAR16                         *Name,
  IN EFI_GUID                       *VendorGuid,
  OUT UINTN                         *VarSize
  );

EFI_STATUS
LibDeleteVariable (
  IN CHAR16                         *VarName,
  IN EFI_GUID                       *VarGuid
  );

EFI_STATUS
LibInsertToTailOfBootOrder (
  IN  UINT16                        BootOption,
  IN  BOOLEAN                       OnlyInsertIfEmpty
  );

BOOLEAN
ValidMBR (
  IN  MASTER_BOOT_RECORD            *Mbr,
  IN  EFI_BLOCK_IO_PROTOCOL         *BlkIo
  );

CHAR16        *
LibGetImageName (
  IN  EFI_LOADED_IMAGE_PROTOCOL *Image
  );

EFI_STATUS
ShellGetDeviceName (
  IN EFI_HANDLE   DeviceHandle,
  IN BOOLEAN      UseComponentName,
  IN BOOLEAN      UseDevicePath,
  IN CHAR8        *Language,
  CHAR16          **BestDeviceName,
  EFI_STATUS      *ConfigurationStatus,
  EFI_STATUS      *DiagnosticsStatus,
  BOOLEAN         Display,
  UINTN           Indent
  );

UINT8
DecimaltoBCD (
  IN  UINT8                         DecValue
  );

UINT8
BCDtoDecimal (
  IN  UINT8                         BcdValue
  );

CHAR16        *
GetShellMode (
  VOID
  );

EFI_STATUS
LibGetStringByToken (
  IN     EFI_HII_HANDLE              HiiHandle,
  IN     UINT16                      Token,
  IN     CHAR16                      *Lang,
  IN OUT CHAR16                      **String
  );

VOID
QSort (
  IN OUT VOID                       *Buffer,
  IN     UINTN                      EleNum,
  IN     UINTN                      EleSize,
  IN     INTN                       (*Compare) (VOID *, void *)
  );

VOID
LibFreeArgInfo (
  IN EFI_SHELL_ARG_INFO       *ArgInfo
  );

EFI_STATUS
LibFilterNullArgs (
  VOID
  );

BOOLEAN
GrowBuffer (
  IN OUT EFI_STATUS   *Status,
  IN OUT VOID         **Buffer,
  IN UINTN            BufferSize
  );

CHAR16                                *
MemoryTypeStr (
  IN EFI_MEMORY_TYPE  Type
  );

EFI_STATUS
LibCmdGetStringByToken (
  IN     UINT8                                  *StringPack,
  IN     EFI_GUID                               *StringPackGuid,
  IN     UINT16                                 Token,
  IN OUT CHAR16                                 **Str
  );

VOID
DumpHex (
  IN UINTN        Indent,
  IN UINTN        Offset,
  IN UINTN        DataSize,
  IN VOID         *UserData
  );

BOOLEAN
PrivateDumpHex (
  IN UINTN        Indent,
  IN UINTN        Offset,
  IN UINTN        DataSize,
  IN VOID         *UserData
  );

UINT16 *
LibGetMachineTypeString (
  IN UINT16   MachineType
  );

EFI_STATUS
LibGetImageHeader (
  IN  EFI_DEVICE_PATH_PROTOCOL    *DevicePath,
  OUT Elf32_Ehdr                  *ImageHeader
  );

#endif
