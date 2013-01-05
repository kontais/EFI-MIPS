/*++

Copyright (c) 2012 kontais

Module Name:

    osloader.c
    
Abstract:


Author:

Revision History

--*/

#include "efi.h"
#include "efilib.h"
#include "elf.h"

VOID
FlushCache (
  VOID
  );


EFI_STATUS
LoadElf64 (
  IN UINT8           *Header,
  IN EFI_FILE_HANDLE File
  )
{
  EFI_STATUS  Status;
  Elf64_Ehdr  *e64;
  Elf64_Phdr  Phdr;
  Elf64_SHalf Idx;
  UINTN       Size;
  UINT16                  InputString[20];
  

  e64  = (Elf64_Ehdr *) Header;

  for (Idx = 0; Idx < e64->e_phnum; Idx ++) {
    Size   = e64->e_phentsize;
    Status = File->SetPosition (File, (e64->e_phoff + Size * Idx));
    Status = File->Read (File,
                         &Size,
                         (VOID*) &Phdr
                         );
    if (Phdr.p_type != PT_LOAD) {
      continue;
    }
    
    Size   = Phdr.p_filesz;
    Status = File->SetPosition (File, Phdr.p_offset);
    Status = File->Read (File,
                         &Size,
                         (VOID*) Phdr.p_vaddr
                         );
    Print(L"File Offset 0x%x Copy To 0x%x\n", (UINT32)Phdr.p_offset, (UINT32)Phdr.p_vaddr);
    if (Phdr.p_memsz > Size) {
      ZeroMem ((VOID*)(Phdr.p_vaddr + Size), (Phdr.p_memsz - Size));
      Print(L"0x%x - 0x%x Set To Zero\n", 
            (UINT32)(Phdr.p_vaddr + Size), 
            (UINT32)(Phdr.p_vaddr + Phdr.p_memsz)
            );
    }
  }  

  FlushCache();
  
  return Status;
}

