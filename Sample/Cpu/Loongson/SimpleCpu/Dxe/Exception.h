/*++

Copyright (c) 2012, kontais                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:
  Exception.h

Abstract:

--*/
#ifndef _EXCEPTION_H_
#define _EXCEPTION_H_

VOID
ExceptionHandler (
  IN EFI_EXCEPTION_TYPE    InterruptType,
  IN EFI_SYSTEM_CONTEXT    SystemContext
  )
;

VOID
ExceptionHandlerADESL (
  IN EFI_EXCEPTION_TYPE    InterruptType,
  IN EFI_SYSTEM_CONTEXT    SystemContext
  )
;

EFI_STATUS
UnalignedAccess (
  IN UINT32        OpCode,
  IN UINT32        Addr,
  IN OUT UINT32    *Data
  )
;

UINT32
ExceptionExcuteBranch (
   IN EFI_SYSTEM_CONTEXT    SystemContext,
   OUT UINT32               *Pc
  )
;

#endif
