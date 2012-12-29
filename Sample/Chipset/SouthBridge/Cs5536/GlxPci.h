/*++

Copyright (c) 2011, kontais                                                                   
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  GlxPci.h

Abstract:


--*/
#ifndef _GLX_PCI_H_
#define _GLX_PCI_H_

//
// PCI bus device function
//
#define  CS5536_PCI_BUS         0
#define  CS5536_PCI_IDSEL       14
#define  CS5536_PCI_FUNC_ISA    0
#define  CS5536_PCI_FUNC_FLASH  1
#define  CS5536_PCI_FUNC_IDE    2
#define  CS5536_PCI_FUNC_ACC    3
#define  CS5536_PCI_FUNC_OHCI   4
#define  CS5536_PCI_FUNC_EHCI   5
#define  CS5536_PCI_FUNC_UDC    6
#define  CS5536_PCI_FUNC_OTG    7

#define  CS5536_PCI_FUNC_START  0
#define  CS5536_PCI_FUNC_END    7
#define  CS5536_PCI_FUNC_COUNT  (CS5536_PCI_FUNC_END - CS5536_PCI_FUNC_START + 1)

/* MSR access through PCI configuration space */
#define PCI_MSR_CTRL    0x00f0
#define PCI_MSR_ADDR    0x00f4
#define PCI_MSR_LO32    0x00f8
#define PCI_MSR_HI32    0x00fc


#endif /* _GLX_PCI_H_ */


