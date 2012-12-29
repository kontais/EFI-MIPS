/*++

Copyright (c) 2009, kontais                                                                   
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  SystemLib.c

Abstract:

  Implement the protocol that for POSIX lib for Linux emulation envirnment.

--*/

#include "SecMain.h"

int 
SystemLib_printf (
  const char * Format, 
  ...
  )
{
  int Return;
  va_list Marker;

  va_start (Marker, Format);
  Return = vprintf (Format,Marker);
  va_end (Marker);
  
  return Return;
}

int 
SystesmLib_flush_cache (
  char *addr, 
  const int nbytes,
  const int op
  )
{
  return _flush_cache(addr, nbytes,op);
}


EFI_SYSTEM_LIB_PROTOCOL mSystemLib = {
  EFI_SYSTEM_LIB_PROTOCOL_SIGNATURE,
  SystemLib_printf,
  _flush_cache
};

EFI_SYSTEM_LIB_PROTOCOL *gSys = &mSystemLib;

