/*++

Copyright (c) 2011, kontais                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  PciCfgLib.h
  
Abstract:
  
--*/
#ifndef _PCI_CFG_LIB_H_
#define _PCI_CFG_LIB_H_

UINT32
BonitoPciRead (
  IN  UINT64  Address
  )
;

VOID
BonitoPciWrite (
  IN  UINT64  Address,
  IN  UINT32  Data
  )
;

VOID
PciBreakAddress (
  IN UINT64    Address, 
  OUT UINT32   *Bus, 
  OUT UINT32   *Dev,
  OUT UINT32   *Fun,
  OUT UINT32   *Reg
  )
;

#endif /* _PCI_CFG_LIB_H_ */

