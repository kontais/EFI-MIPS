/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  Lock.h

Abstract:

  Infomation about the lock function.
 
Revision History

--*/
#ifndef _SHELL_LOCK_H
#define _SHELL_LOCK_H

typedef struct _FLOCK {
  EFI_TPL Tpl;
  EFI_TPL OwnerTpl;
  UINTN   Lock;
} FLOCK;

VOID
InitializeLock (
  IN OUT FLOCK    *Lock,
  IN EFI_TPL      Priority
  );

VOID
AcquireLock (
  IN FLOCK    *Lock
  );

VOID
ReleaseLock (
  IN FLOCK    *Lock
  );

#endif