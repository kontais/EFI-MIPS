/*++

Copyright (c) 2012, kontais                                                                   
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  PmonLib.h

Abstract:


--*/
#ifndef _PMON_LIB_H_
#define _PMON_LIB_H_

void PmonEntry(void);
int  InitShell(void);

int  MemoryPuts(char *);
void video_puts(const char *);

void delay(int);


#endif /* _PMON_LIB_H_ */
