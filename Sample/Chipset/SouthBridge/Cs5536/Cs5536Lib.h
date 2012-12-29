/*++

Copyright (c) 2012, kontais                                                                   
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  Cs5536Lib.h

Abstract:


--*/


#ifndef  _CS5536_LIB_H_
#define  _CS5536_LIB_H_

EFI_STATUS
Cs5536PciRead (
  IN UINT64     Address, 
  IN OUT UINT32 *Data
);

EFI_STATUS
Cs5536PciWrite (
  IN UINT64 Address, 
  IN UINT32 Data
);

#endif /* _CS5536_LIB_H__ */

