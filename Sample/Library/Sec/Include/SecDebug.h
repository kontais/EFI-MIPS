/*++

Copyright (c) 2011, kontais                                                                   
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  SecDebug.h

Abstract:


--*/

#ifndef _SEC_DEBUG_H_
#define _SEC_DEBUG_H_

VOID SerialOutputHex(UINT32);
VOID SerialOutputString(UINT8*);
VOID SerialSendByte(UINT8);

#endif /* _SEC_DEBUG_H_ */

