/*++

Copyright (c) 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  Efi2Linux.h

Abstract:

  Tiano mechanism to create Linux EFI code.

  Only used for Linux emulation environment. This file replaces the Tiano.h
  when you need to access Linux stuff in EFI.

  Don't access Linux APIs directly allways go LinuxThunk protocol.

  In most envirnments gLinux is a pointer to a EFI_LINUX_THUNK_PROTOCOL.

--*/

#ifndef _EFI_LINUX_H_
#define _EFI_LINUX_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <dirent.h>
#include <poll.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include <termios.h>
#include <utime.h>
#include <dlfcn.h>
#include <sys/vfs.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/cachectl.h>

//
// We replaced Tiano.h so we must include it.
//
#include "Tiano.h"

#include EFI_PROTOCOL_DEFINITION (LinuxThunk)

#endif
