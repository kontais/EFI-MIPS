/*++

Copyright (c) 2012, kontais                                                                   
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  Data.c

Abstract:

  Global data in the EXT2 Filesystem driver

Revision History

--*/

#include "Ext2.h"

//
// Globals
//
//
// Ext2FsLock - Global lock for synchronizing all requests.
//
EFI_LOCK Ext2FsLock = EFI_INITIALIZE_LOCK_VARIABLE(EFI_TPL_CALLBACK);

//
// Filesystem interface functions
//
EFI_FILE                        Ext2FileInterface = {
  EFI_FILE_HANDLE_REVISION,
  Ext2Open,
  Ext2Close,
  Ext2Delete,
  Ext2Read,
  Ext2Write,
  Ext2GetPosition,
  Ext2SetPosition,
  Ext2GetInfo,
  Ext2SetInfo,
  Ext2Flush
};

