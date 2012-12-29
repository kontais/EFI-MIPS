/*++

Copyright (c) 2009, kontais                                                    
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  EfiElfOptionalHeader.h

Abstract:

  ELF MIPS处理器信息相关绑定

--*/

#ifndef _EFI_ELF_OPTIONAL_HEADER_H_
#define _EFI_ELF_OPTIONAL_HEADER_H_

#define EFI_IMAGE_MACHINE_TYPE (EFI_IMAGE_MACHINE_MIPS)

#define EFI_IMAGE_MACHINE_TYPE_SUPPORTED(Machine) \
  (((Machine) == EFI_IMAGE_MACHINE_MIPS) || ((Machine) == EFI_IMAGE_MACHINE_EBC))

//#define EFI_IMAGE_MACHINE_CROSS_TYPE_SUPPORTED(Machine)

//#define EFI_IMAGE_NT_OPTIONAL_HDR_MAGIC EFI_IMAGE_NT_OPTIONAL_HDR32_MAGIC
//typedef EFI_IMAGE_OPTIONAL_HEADER32 EFI_IMAGE_OPTIONAL_HEADER;
typedef Elf32_Ehdr    EFI_IMAGE_ELF_HEADERS;

#endif // _EFI_ELF_OPTIONAL_HEADER_H_

