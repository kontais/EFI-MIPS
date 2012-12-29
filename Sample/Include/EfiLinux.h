/*++

Copyright (c) 2004, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  EfiLinux.h

Abstract:

  EFI master include file for Linux components.

--*/

#ifndef _EFI_LINUX_H_

#ifdef EFI_LINUX_EMULATOR

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

#endif

#include "Tiano.h"

#endif
