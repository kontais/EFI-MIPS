/*++
Copyright (c) 2012, kontais                                               
Copyright (c) 2004 - 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module name:
    Rtl81x9Snp.c

Abstract:

--*/

#include "Rtl81x9.h"

EFI_SIMPLE_NETWORK_PROTOCOL Rtl81x9SimpleNetworkProtocol = {
  EFI_SIMPLE_NETWORK_PROTOCOL_REVISION,
  Rtl81x9Snp_Start,
  Rtl81x9Snp_Stop,
  Rtl81x9Snp_Initialize,
  Rtl81x9Snp_Reset,
  Rtl81x9Snp_Shutdown,
  Rtl81x9Snp_ReceiveFilters,
  Rtl81x9Snp_StationAddress,
  Rtl81x9Snp_Statistics,
  Rtl81x9Snp_MCastIpToMac,
  Rtl81x9Snp_NvData,
  Rtl81x9Snp_GetStatus,
  Rtl81x9Snp_Transmit,
  Rtl81x9Snp_Receive,
  NULL,
  NULL
};

