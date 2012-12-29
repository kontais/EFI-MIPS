/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution. The full text of the license may be found at         
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  Perf.c

Abstract:

  Support for Performance primatives. 

--*/

#include "EfiShellLib.h"

#include EFI_PROTOCOL_DEFINITION (Performance)

EFI_PERFORMANCE_PROTOCOL  *Perf                         = NULL;
EFI_GUID                  gEfiPerformanceProtocolGuid2  = EFI_PERFORMANCE_PROTOCOL_GUID;

EFI_STATUS
StartMeasure (
  EFI_HANDLE            Handle,
  IN UINT16             *Token,
  IN UINT16             *Host,
  IN UINT64             Ticker
  )
/*++

Routine Description:

  Start to gauge on a specified handle, token and host, with Ticker as start tick.

Arguments:

  Handle  - Handle to measure
  Token   - Help info about the data
  Host    - Host name
  Ticker  - Ticker as start tick

Returns:

  Status code.

--*/
{
  EFI_STATUS  Status;
  if (!Perf) {
    Status = BS->LocateProtocol (
                  &gEfiPerformanceProtocolGuid2,
                  NULL,
                  (VOID **) &Perf
                  );
    if (Status != EFI_SUCCESS) {
      return Status;
    }
  }

  return (Perf->StartGauge) (Perf, Handle, Token, Host, Ticker);
}

EFI_STATUS
EndMeasure (
  EFI_HANDLE            Handle,
  IN UINT16             *Token,
  IN UINT16             *Host,
  IN UINT64             Ticker
  )
/*++

Routine Description:

  End gauging on a specified handle, token and host, with Ticker as end tick.

Arguments:

  Handle  - Handle to measure
  Token   - Help info about the data
  Host    - Host name
  Ticker  - Ticker as end tick

Returns:

  Status code.

--*/
{
  EFI_STATUS  Status;
  if (!Perf) {
    Status = BS->LocateProtocol (
                  &gEfiPerformanceProtocolGuid2,
                  NULL,
                  (VOID **) &Perf
                  );
    if (Status != EFI_SUCCESS) {
      return Status;
    }
  }

  return (Perf->EndGauge) (Perf, Handle, Token, Host, Ticker);
}
