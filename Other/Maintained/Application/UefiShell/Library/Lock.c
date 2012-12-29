/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  Lock.c

Abstract:

  Implements FLOCK

Revision History

--*/

#include "EfiShellLib.h"

VOID
InitializeLock (
  IN OUT FLOCK    *Lock,
  IN EFI_TPL      Priority
  )
/*++

Routine Description:

  Initialize a basic mutual exclusion lock.   Each lock
  provides mutual exclusion access at it's task priority
  level.  Since there is no-premption (at any TPL) or
  multiprocessor support, acquiring the lock only consists
  of raising to the locks TPL.

  Note on a debug build the lock is acquired and released
  to help ensure proper usage.
    
Arguments:

  Lock        - The FLOCK structure to initialize

  Priority    - The task priority level of the lock

    
Returns:

  An initialized F Lock structure.

--*/
{
  ASSERT (Lock != NULL);
  
  Lock->Tpl       = Priority;
  Lock->OwnerTpl  = 0;
  Lock->Lock      = 0;
}

VOID
AcquireLock (
  IN FLOCK    *Lock
  )
/*++

Routine Description:

  Raising to the task priority level of the mutual exclusion
  lock, and then acquires ownership of the lock.
    
Arguments:

  Lock        - The lock to acquire
    
Returns:

  Lock owned

--*/
{
  
  ASSERT (Lock);
  
  if (BS) {
    if (BS->RaiseTPL != NULL) {
      Lock->OwnerTpl = BS->RaiseTPL (Lock->Tpl);
    }
  }
  Lock->Lock += 1;
  ASSERT (Lock->Lock == 1);
}

VOID
ReleaseLock (
  IN FLOCK    *Lock
  )
/*++

Routine Description:

  Releases ownership of the mutual exclusion lock, and
  restores the previous task priority level.
    
Arguments:

  Lock        - The lock to release
    
Returns:

  Lock unowned

--*/
{
  EFI_TPL Tpl;

  ASSERT (Lock != NULL);

  Tpl = Lock->OwnerTpl;
  ASSERT (Lock->Lock == 1);
  Lock->Lock -= 1;
  if (BS) {
    if (BS->RestoreTPL != NULL) {
      BS->RestoreTPL (Tpl);
    }
  }
}
