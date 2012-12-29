/*++

Copyright (c) 2009, kontais                                                                          
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  EfiImage.h

Abstract:

  EFI image format for ELF 32bit. 

--*/

#ifndef _EFI_IMAGE_H_
#define _EFI_IMAGE_H_

#include "Elf.h"

//
// ELF Subsystem type for EFI images
//
#define EFI_IMAGE_SUBSYSTEM_EFI_APPLICATION         10
#define EFI_IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER 11
#define EFI_IMAGE_SUBSYSTEM_EFI_RUNTIME_DRIVER      12

//
//         A SAL runtime driver does not get fixed up when a transition to
//         virtual mode is made. In all other cases it should be treated
//         like a EFI_IMAGE_SUBSYSTEM_EFI_RUNTIME_DRIVER image
//
#define EFI_IMAGE_SUBSYSTEM_SAL_RUNTIME_DRIVER  13

//
// ELF Machine type for EFI images
//
#define EFI_IMAGE_MACHINE_MIPS      EM_MIPS
#define EFI_IMAGE_MACHINE_PPC       EM_PPC
#define EFI_IMAGE_MACHINE_IA32      EM_386
#define EFI_IMAGE_MACHINE_IA64      0x0200
#define EFI_IMAGE_MACHINE_X64       0x8664


//
// #define EFI_IMAGE_MACHINE_FCODE     0xfc0d
//
#define EFI_IMAGE_MACHINE_EBC       0x0EBC

#define EFI_IMAGE_ELF_SIGNATURE     0x464c457f  //0x7fELF
#define EFI_IMAGE_DOS_SIGNATURE     0x5A4D      // MZ
#define EFI_IMAGE_OS2_SIGNATURE     0x454E      // NE
#define EFI_IMAGE_OS2_SIGNATURE_LE  0x454C      // LE
#define EFI_IMAGE_NT_SIGNATURE      0x00004550  // PE00
#define EFI_IMAGE_EDOS_SIGNATURE    0x44454550  // PEED
//
// * Other names and brands may be claimed as the property of others.
//
//
// Directory format.
//
typedef struct {
  UINT32  VirtualAddress;
  UINT32  Size;
} EFI_IMAGE_DATA_DIRECTORY;

#define EFI_IMAGE_NUMBER_OF_DIRECTORY_ENTRIES 16

typedef struct {
  UINT16  Magic;
  UINT8   MajorLinkerVersion;
  UINT8   MinorLinkerVersion;
  UINT32  SizeOfCode;
  UINT32  SizeOfInitializedData;
  UINT32  SizeOfUninitializedData;
  UINT32  AddressOfEntryPoint;
  UINT32  BaseOfCode;
  UINT32  BaseOfData;
  UINT32  BaseOfBss;
  UINT32  GprMask;
  UINT32  CprMask[4];
  UINT32  GpValue;
} EFI_IMAGE_ROM_OPTIONAL_HEADER;

#define EFI_IMAGE_ROM_OPTIONAL_HDR_MAGIC      0x107
#define EFI_IMAGE_SIZEOF_ROM_OPTIONAL_HEADER  sizeof (EFI_IMAGE_ROM_OPTIONAL_HEADER)


//
// Include processor specific definition of EFI_IMAGE_OPTIONAL_HEADER so the
// type name EFI_IMAGE_OPTIONAL_HEADER is appropriate to the build.  Same for
// EFI_IMAGE_NT_HEADERS.  These definitions MUST be used by ALL EFI code.
//
#include "EfiElfOptionalHeader.h"


//
// Directory Entries
//
#define EFI_IMAGE_DIRECTORY_ENTRY_EXPORT      0
#define EFI_IMAGE_DIRECTORY_ENTRY_IMPORT      1
#define EFI_IMAGE_DIRECTORY_ENTRY_RESOURCE    2
#define EFI_IMAGE_DIRECTORY_ENTRY_EXCEPTION   3
#define EFI_IMAGE_DIRECTORY_ENTRY_SECURITY    4
#define EFI_IMAGE_DIRECTORY_ENTRY_BASERELOC   5
#define EFI_IMAGE_DIRECTORY_ENTRY_DEBUG       6
#define EFI_IMAGE_DIRECTORY_ENTRY_COPYRIGHT   7
#define EFI_IMAGE_DIRECTORY_ENTRY_GLOBALPTR   8
#define EFI_IMAGE_DIRECTORY_ENTRY_TLS         9
#define EFI_IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG 10

//
// Section header format.
//
#define EFI_IMAGE_SIZEOF_SHORT_NAME 8

typedef struct {
  UINT8 Name[EFI_IMAGE_SIZEOF_SHORT_NAME];
  union {
    UINT32  PhysicalAddress;
    UINT32  VirtualSize;
  } Misc;
  UINT32  VirtualAddress;
  UINT32  SizeOfRawData;
  UINT32  PointerToRawData;
  UINT32  PointerToRelocations;
  UINT32  PointerToLinenumbers;
  UINT16  NumberOfRelocations;
  UINT16  NumberOfLinenumbers;
  UINT32  Characteristics;
} EFI_IMAGE_SECTION_HEADER;

#define EFI_IMAGE_SIZEOF_SECTION_HEADER       40

#define EFI_IMAGE_SCN_TYPE_NO_PAD             0x00000008  // Reserved.
#define EFI_IMAGE_SCN_CNT_CODE                0x00000020
#define EFI_IMAGE_SCN_CNT_INITIALIZED_DATA    0x00000040
#define EFI_IMAGE_SCN_CNT_UNINITIALIZED_DATA  0x00000080

#define EFI_IMAGE_SCN_LNK_OTHER               0x00000100  // Reserved.
#define EFI_IMAGE_SCN_LNK_INFO                0x00000200  // Section contains comments or some other type of information.
#define EFI_IMAGE_SCN_LNK_REMOVE              0x00000800  // Section contents will not become part of image.
#define EFI_IMAGE_SCN_LNK_COMDAT              0x00001000

#define EFI_IMAGE_SCN_ALIGN_1BYTES            0x00100000
#define EFI_IMAGE_SCN_ALIGN_2BYTES            0x00200000
#define EFI_IMAGE_SCN_ALIGN_4BYTES            0x00300000
#define EFI_IMAGE_SCN_ALIGN_8BYTES            0x00400000
#define EFI_IMAGE_SCN_ALIGN_16BYTES           0x00500000
#define EFI_IMAGE_SCN_ALIGN_32BYTES           0x00600000
#define EFI_IMAGE_SCN_ALIGN_64BYTES           0x00700000

#define EFI_IMAGE_SCN_MEM_DISCARDABLE         0x02000000
#define EFI_IMAGE_SCN_MEM_NOT_CACHED          0x04000000
#define EFI_IMAGE_SCN_MEM_NOT_PAGED           0x08000000
#define EFI_IMAGE_SCN_MEM_SHARED              0x10000000
#define EFI_IMAGE_SCN_MEM_EXECUTE             0x20000000
#define EFI_IMAGE_SCN_MEM_READ                0x40000000
#define EFI_IMAGE_SCN_MEM_WRITE               0x80000000

//
// Symbol format.
//
#define EFI_IMAGE_SIZEOF_SYMBOL 18

//
// Section values.
//
// Symbols have a section number of the section in which they are
// defined. Otherwise, section numbers have the following meanings:
//
#define EFI_IMAGE_SYM_UNDEFINED (UINT16) 0  // Symbol is undefined or is common.
#define EFI_IMAGE_SYM_ABSOLUTE  (UINT16) -1 // Symbol is an absolute value.
#define EFI_IMAGE_SYM_DEBUG     (UINT16) -2 // Symbol is a special debug item.
//
// Type (fundamental) values.
//
#define EFI_IMAGE_SYM_TYPE_NULL   0   // no type.
#define EFI_IMAGE_SYM_TYPE_VOID   1   //
#define EFI_IMAGE_SYM_TYPE_CHAR   2   // type character.
#define EFI_IMAGE_SYM_TYPE_SHORT  3   // type short integer.
#define EFI_IMAGE_SYM_TYPE_INT    4
#define EFI_IMAGE_SYM_TYPE_LONG   5
#define EFI_IMAGE_SYM_TYPE_FLOAT  6
#define EFI_IMAGE_SYM_TYPE_DOUBLE 7
#define EFI_IMAGE_SYM_TYPE_STRUCT 8
#define EFI_IMAGE_SYM_TYPE_UNION  9
#define EFI_IMAGE_SYM_TYPE_ENUM   10  // enumeration.
#define EFI_IMAGE_SYM_TYPE_MOE    11  // member of enumeration.
#define EFI_IMAGE_SYM_TYPE_BYTE   12
#define EFI_IMAGE_SYM_TYPE_WORD   13
#define EFI_IMAGE_SYM_TYPE_UINT   14
#define EFI_IMAGE_SYM_TYPE_DWORD  15

//
// Type (derived) values.
//
#define EFI_IMAGE_SYM_DTYPE_NULL      0 // no derived type.
#define EFI_IMAGE_SYM_DTYPE_POINTER   1
#define EFI_IMAGE_SYM_DTYPE_FUNCTION  2
#define EFI_IMAGE_SYM_DTYPE_ARRAY     3

//
// Storage classes.
//
#define EFI_IMAGE_SYM_CLASS_END_OF_FUNCTION   (UINT8) -1
#define EFI_IMAGE_SYM_CLASS_NULL              0
#define EFI_IMAGE_SYM_CLASS_AUTOMATIC         1
#define EFI_IMAGE_SYM_CLASS_EXTERNAL          2
#define EFI_IMAGE_SYM_CLASS_STATIC            3
#define EFI_IMAGE_SYM_CLASS_REGISTER          4
#define EFI_IMAGE_SYM_CLASS_EXTERNAL_DEF      5
#define EFI_IMAGE_SYM_CLASS_LABEL             6
#define EFI_IMAGE_SYM_CLASS_UNDEFINED_LABEL   7
#define EFI_IMAGE_SYM_CLASS_MEMBER_OF_STRUCT  8
#define EFI_IMAGE_SYM_CLASS_ARGUMENT          9
#define EFI_IMAGE_SYM_CLASS_STRUCT_TAG        10
#define EFI_IMAGE_SYM_CLASS_MEMBER_OF_UNION   11
#define EFI_IMAGE_SYM_CLASS_UNION_TAG         12
#define EFI_IMAGE_SYM_CLASS_TYPE_DEFINITION   13
#define EFI_IMAGE_SYM_CLASS_UNDEFINED_STATIC  14
#define EFI_IMAGE_SYM_CLASS_ENUM_TAG          15
#define EFI_IMAGE_SYM_CLASS_MEMBER_OF_ENUM    16
#define EFI_IMAGE_SYM_CLASS_REGISTER_PARAM    17
#define EFI_IMAGE_SYM_CLASS_BIT_FIELD         18
#define EFI_IMAGE_SYM_CLASS_BLOCK             100
#define EFI_IMAGE_SYM_CLASS_FUNCTION          101
#define EFI_IMAGE_SYM_CLASS_END_OF_STRUCT     102
#define EFI_IMAGE_SYM_CLASS_FILE              103
#define EFI_IMAGE_SYM_CLASS_SECTION           104
#define EFI_IMAGE_SYM_CLASS_WEAK_EXTERNAL     105

//
// type packing constants
//
#define EFI_IMAGE_N_BTMASK  017
#define EFI_IMAGE_N_TMASK   060
#define EFI_IMAGE_N_TMASK1  0300
#define EFI_IMAGE_N_TMASK2  0360
#define EFI_IMAGE_N_BTSHFT  4
#define EFI_IMAGE_N_TSHIFT  2

//
// Communal selection types.
//
#define EFI_IMAGE_COMDAT_SELECT_NODUPLICATES    1
#define EFI_IMAGE_COMDAT_SELECT_ANY             2
#define EFI_IMAGE_COMDAT_SELECT_SAME_SIZE       3
#define EFI_IMAGE_COMDAT_SELECT_EXACT_MATCH     4
#define EFI_IMAGE_COMDAT_SELECT_ASSOCIATIVE     5

#define EFI_IMAGE_WEAK_EXTERN_SEARCH_NOLIBRARY  1
#define EFI_IMAGE_WEAK_EXTERN_SEARCH_LIBRARY    2
#define EFI_IMAGE_WEAK_EXTERN_SEARCH_ALIAS      3


//
// Line number format.
//
typedef struct {
  union {
    UINT32  SymbolTableIndex; // Symbol table index of function name if Linenumber is 0.
    UINT32  VirtualAddress;   // Virtual address of line number.
  } Type;
  UINT16  Linenumber;         // Line number.
} EFI_IMAGE_LINENUMBER;

#define EFI_IMAGE_SIZEOF_LINENUMBER 6

//
// Union of Elf32_Ehdr, Elf64_Ehdr headers
//
typedef union {
  Elf32_Ehdr    Elf32;
  Elf64_Ehdr    Elf64;
} EFI_IMAGE_ELF_HEADER_UNION;

typedef union {
  Elf32_Ehdr                  *Elf32;
  Elf64_Ehdr                  *Elf64;
  EFI_IMAGE_ELF_HEADER_UNION  *Union;
} EFI_IMAGE_ELF_HEADER_PTR_UNION;

#endif
