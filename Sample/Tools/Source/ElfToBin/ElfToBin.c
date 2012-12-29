/*++

Copyright 2010, Kontais                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

    ElfToBin.c
    
Abstract:
    Extract  executable elf file's .text, .data, .bss section to a binanry file.
    (objcopy -O binary --set-section-flags .bss=alloc,load,contents x.elf x.bin)

--*/

#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "Tiano.h"
#include "Elf.h"

#define PROMGRAM_HEADER_MAX  (10)

typedef struct _ElfToBinFileInfo {
  UINTN  FileOffset;
  UINTN  FileSize;
  UINTN  MemSize;
} ETB_INFO;

STATIC
EFI_STATUS
GetElfHeader32 (
  FILE       *In, 
  Elf32_Ehdr *Ehdr32
);

STATIC
EFI_STATUS
GetElfHeader64 (
  FILE       *In, 
  Elf64_Ehdr *Ehdr64
);

STATIC
EFI_STATUS
GetElfProgramHeader32 (
  FILE       *In, 
  Elf32_Ehdr *Ehdr32, 
  Elf32_Phdr *Phdr32
);

STATIC
EFI_STATUS
GetElfProgramHeader64 (
  FILE       *In, 
  Elf64_Ehdr *Ehdr64, 
  Elf64_Phdr *Phdr64
);

void
helpmsg (
  void
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:


Returns:

  GC_TODO: add return values

--*/
{
  printf (
    "ElfToBin ElfFileName BinFileName\n""   ElfFileName = Input file to extract\n""   BinFileName = Output binary file\n"
    );
}

int
main (
  int argc,
  char*argv[]
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  argc  - GC_TODO: add argument description
  argv  - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
{
  FILE        *In;
  FILE        *Out;
  UINTN       Index;
  char        CharC;
  EFI_STATUS  Status;
  Elf32_Ehdr  Ehdr32;
  Elf64_Ehdr  Ehdr64;
  Elf32_Phdr  Phdr32;
  Elf64_Phdr  Phdr64;
  ETB_INFO    EtBFileInfo;


  if (argc != 3) {
    helpmsg ();
    return -1;
  }

  In = fopen (argv[1], "rb");
  if (In == NULL) {
    printf ("Unable to open file \"%s\"\n", argv[1]);
    return -1;
  }

  //
  // Get ELF Header info
  //
  Status = GetElfHeader32(In, &Ehdr32);
  if(EFI_ERROR(Status)) {
    fclose(In);
    return -1;
  }

  //
  // Get Elf32 program header info
  //
  if (Ehdr32.e_ident[EI_CLASS] == ELFCLASS32) {
    Status = GetElfProgramHeader32(In,&Ehdr32,&Phdr32);
    if (EFI_ERROR(Status)) {
      fclose(In);
      return -1;
    }

    EtBFileInfo.FileOffset = (UINTN) Phdr32.p_offset;
    EtBFileInfo.FileSize   = (UINTN) Phdr32.p_filesz;
    EtBFileInfo.MemSize    = (UINTN) Phdr32.p_memsz;
  //
  // Get Elf64 header and program header info
  //
  } else if(Ehdr32.e_ident[EI_CLASS] == ELFCLASS64) {
    //
    // Get Elf64 header info
    //
    Status = GetElfHeader64(In, &Ehdr64);
    if (EFI_ERROR(Status)) {
      fclose(In);
      return -1;
    }
    //
    // Get Elf64 program header info
    //
    Status = GetElfProgramHeader64(In,&Ehdr64,&Phdr64);
    if (EFI_ERROR(Status)) {
      fclose(In);
      return -1;
    }

    EtBFileInfo.FileOffset = (UINTN) Phdr64.p_offset;
    EtBFileInfo.FileSize   = (UINTN) Phdr64.p_filesz;
    EtBFileInfo.MemSize    = (UINTN) Phdr64.p_memsz;

  } else {
    printf("No 32 or 64 bit elf file.\n");
    fclose(In);
    return -1;
  }
  
  //
  // seek to program offset
  //
  if (fseek(In, EtBFileInfo.FileOffset, SEEK_SET) != 0) {
    fclose(In);
    return -1;
  }

  //
  // Open binary file to write
  //
  Out = fopen (argv[2], "wb");
  if (Out == NULL) {
    printf ("Unable to open file \"%s\"\n", argv[2]);
    fclose(In);
    return -1;
  }



  for (Index = 0; Index < EtBFileInfo.FileSize; Index++) {
    CharC = (char) fgetc (In);
    if (feof (In)) {
      break;
    }

    fputc (CharC, Out);
  }

  //
  // File .bss section, we use Index as .bss size
  //
  if ( EtBFileInfo.MemSize > EtBFileInfo.FileSize) {
    Index = EtBFileInfo.MemSize - EtBFileInfo.FileSize;
    while (Index--) {
      fputc(0x00,Out);
    }
  }

  fclose (In);
  fclose (Out);

  return 0;
}

STATIC
EFI_STATUS
GetElfHeader32 (
  FILE       *In, 
  Elf32_Ehdr *Ehdr32
)
{
  if (In == NULL || Ehdr32 == NULL) {
    printf("Invalid Parameter.");
    return EFI_INVALID_PARAMETER;
  }

  if (fseek(In, 0x0, SEEK_SET) != 0) {
    printf("GetElfHeader32 fseek error.\n");
    return EFI_LOAD_ERROR;
  }

  if (fread(Ehdr32, sizeof(Elf32_Ehdr), 1, In) != 1) {
    printf("GetElfHeader32 fread error.\n");
    return EFI_LOAD_ERROR;
  }

  if (Ehdr32->e_type != ET_EXEC) {
    printf("Not an executable elf file.\n");
    return EFI_UNSUPPORTED;
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
GetElfHeader64 (
  FILE       *In, 
  Elf64_Ehdr *Ehdr64
)
{
  if (In == NULL || Ehdr64 == NULL) {
    printf("Invalid Parameter.\n");
    return EFI_INVALID_PARAMETER;
  }

  if (fseek(In, 0x0, SEEK_SET) != 0) {
    printf("GetElfHeader64 fseek error.\n");
    return EFI_LOAD_ERROR;
  }

  if (fread(Ehdr64, sizeof(Elf64_Ehdr), 1, In) != 1) {
    printf("GetElfHeader64 fread error.\n");
    return EFI_LOAD_ERROR;
  }

  if (Ehdr64->e_type != ET_EXEC) {
    printf("Not an executable elf file.\n");
    return EFI_UNSUPPORTED;
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
GetElfProgramHeader32 (
  FILE       *In, 
  Elf32_Ehdr *Ehdr32, 
  Elf32_Phdr *Phdr32
)
{
  UINTN      PhdrNum;
  UINTN      Index;
  UINTN      PhdrIndex;

  Elf32_Phdr AllPhdr32[PROMGRAM_HEADER_MAX];

  if (In == NULL|| Ehdr32 == NULL||Phdr32 == NULL) {
    printf("Invvlid Parameter.\n");
    return EFI_INVALID_PARAMETER;
  }

  PhdrNum = Ehdr32->e_phnum;

  if (PhdrNum > PROMGRAM_HEADER_MAX) {
    printf("Too many program headers.\n");
    return EFI_UNSUPPORTED;
  }

  if (Ehdr32->e_phentsize != sizeof(Elf32_Phdr)) {
    printf("Program header size error.\n");
    return EFI_UNSUPPORTED;
  }

  if (fseek(In, Ehdr32->e_phoff, SEEK_SET) != 0) {
    printf("GetElfProgramHeader32 fseek error.\n");
    return EFI_LOAD_ERROR;
  }

  if (fread(AllPhdr32,sizeof(Elf32_Phdr),PhdrNum,In) != PhdrNum) {
    printf("GetElfProgramHeader32 fread error.\n");
    return EFI_LOAD_ERROR;
  }
  PhdrIndex = 0xFFFFFFFF;
  for (Index = 0; Index < PhdrNum; Index++) {
    if ((AllPhdr32[Index].p_type == PT_LOAD) && (AllPhdr32[Index].p_flags & PF_X)) {
      if (PhdrIndex == 0xFFFFFFFF) {
        PhdrIndex = Index;
      } else {
        printf("Warning:Too many EXEC PT_LOAD headers. Use first\n");
      }
    }
  }

  if (PhdrIndex != 0xFFFFFFFF) {
      memcpy((VOID*)Phdr32,(VOID*)&AllPhdr32[PhdrIndex],sizeof(Elf32_Phdr));
      return EFI_SUCCESS;
  } 

  return EFI_NOT_FOUND;
}

STATIC
EFI_STATUS
GetElfProgramHeader64 (
  FILE       *In, 
  Elf64_Ehdr *Ehdr64, 
  Elf64_Phdr *Phdr64
)
{
  UINTN      PhdrNum;
  UINTN      Index;
  UINTN      PhdrIndex;

  Elf64_Phdr AllPhdr64[PROMGRAM_HEADER_MAX];

  if (In == NULL|| Ehdr64 == NULL||Phdr64 == NULL) {
    printf("Invvlid Parameter.\n");
    return EFI_INVALID_PARAMETER;
  }

  PhdrNum = Ehdr64->e_phnum;

  if (PhdrNum > PROMGRAM_HEADER_MAX) {
    printf("Too many program headers.\n");
    return EFI_UNSUPPORTED;
  }

  if (Ehdr64->e_phentsize != sizeof(Elf64_Phdr)) {
    printf("Program header size error.\n");
    return EFI_UNSUPPORTED;
  }

  if (fseek(In, Ehdr64->e_phoff, SEEK_SET) != 0) {
    printf("GetElfProgramHeader64 fseek error.\n");
    return EFI_LOAD_ERROR;
  }

  if (fread(AllPhdr64,sizeof(Elf64_Phdr),PhdrNum,In) != PhdrNum) {
    printf("GetElfProgramHeader64 fread error.\n");
    return EFI_LOAD_ERROR;
  }
  PhdrIndex = 0xFFFFFFFF;
  for (Index = 0; Index < PhdrNum; Index++) {
    if ((AllPhdr64[Index].p_type == PT_LOAD) && (AllPhdr64[Index].p_flags & PF_X)) {
      if (PhdrIndex == 0xFFFFFFFF) {
        PhdrIndex = Index;
      } else {
        printf("Warning:Too many EXEC PT_LOAD headers. Use first\n");
      }
    }
  }

  if (PhdrIndex != 0xFFFFFFFF) {
      memcpy((VOID*)Phdr64,(VOID*)&AllPhdr64[PhdrIndex],sizeof(Elf64_Phdr));
      return EFI_SUCCESS;
  } 

  return EFI_NOT_FOUND;
}

