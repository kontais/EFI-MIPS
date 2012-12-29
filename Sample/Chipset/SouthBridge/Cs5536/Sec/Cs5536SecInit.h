/*++

Copyright (c) 2009, kontais                                                                   
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  Cs5536SecInit.h

Abstract:


--*/
#ifndef _CS5536_SEC_INIT_H_
#define _CS5536_SEC_INIT_H_

/*
 * you should change the LBAR if you want to match the pciscan dispatched address.
 */
#define  DIVIL_BASE_ADDR  0xB000
#define  SMB_BASE_ADDR    (DIVIL_BASE_ADDR | 0x390)
#define  GPIO_BASE_ADDR   (DIVIL_BASE_ADDR | 0x000)
#define  MFGPT_BASE_ADDR  (DIVIL_BASE_ADDR | 0x300)
#define  ACPI_BASE_ADDR   (DIVIL_BASE_ADDR | 0x340)
#define  PMS_BASE_ADDR    (DIVIL_BASE_ADDR | 0x280)

#endif /* _CS5536_SEC_INIT_H_ */

