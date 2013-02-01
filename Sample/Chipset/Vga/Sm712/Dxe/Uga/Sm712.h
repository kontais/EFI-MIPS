/*++

Copyright (c) 2012, kontais                                                                   
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  Sm712Dev.h

Abstract:


--*/

#ifndef _SM721_DEV_H_
#define _SM721_DEV_H_

#define SM712_WIDTH   (1024)
#define SM712_HEIGHT  (600)
#define SM712_DEPTH   (16)
#define SM712_HZ      (60)
#define SM712_VENDORID   (0x126F)
#define SM712_DEVICEID   (0x0712)

EFI_STATUS
UgaBlt(
       IN  SM712_UGA_PRIVATE_DATA                  *Private,
       IN  EFI_UGA_PIXEL                           *BltBuffer OPTIONAL,
       IN  EFI_UGA_BLT_OPERATION                   BltOperation,
       IN  UINTN                                   SourceX,
       IN  UINTN                                   SourceY,
       IN  UINTN                                   DestinationX,
       IN  UINTN                                   DestinationY,
       IN  UINTN                                   Width,
       IN  UINTN                                   Height,
       IN  UINTN                                   Delta OPTIONAL
  )
;

#endif /* _SM721_DEV_H_ */
