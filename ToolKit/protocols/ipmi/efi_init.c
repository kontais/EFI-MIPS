/*
 * Copyright (c) 1999, 2000
 * Intel Corporation.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 
 * 3. All advertising materials mentioning features or use of this software must
 *    display the following acknowledgement:
 * 
 *    This product includes software developed by Intel Corporation and its
 *    contributors.
 * 
 * 4. Neither the name of Intel Corporation or its contributors may be used to
 *    endorse or promote products derived from this software without specific
 *    prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY INTEL CORPORATION AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL INTEL CORPORATION OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */

/******************************************************************************
*
*   Revision History:
*   $Log: efi_init.c,v $
*   Revision 1.1.1.1  2006/05/30 06:16:11  hhzhou
*   no message
*
 * 
 *    Rev 1.3   14 Jul 2000 10:51:52   eamaya
 * - Updated KCS protocol per addenda to IPMI spec.
 *   Note that this update is not backward compatible
 *   to previous KCS protocol.
 * - Added ability to unload the driver.
 * 
 *    Rev 1.2   14 Jun 2000 12:48:26   eamaya
 * - Updated to work with SMBIOS table to look up KCS
 * address in Record 38 and override default KCS address CA2.
 * - Updated revision from 1.0 to 1.1
*
******************************************************************************/

#include "efiipmi.h"	// must be first
#include "efilib.h"
#include "efilibc.h"
#include "smbios.h"
#include "kcs.h"

//
//  Initialization entry points
//
extern EFI_STATUS			InitIPMIInterface (VOID);

extern EFI_IPMI_INTERFACE	EfiIPMIInterface;

extern EFI_DEVICE_PATH		EndDevicePath[];

//
//  Globals
//
EFI_DEVICE_IO_INTERFACE		*GlobalIoFncs;

//
//  Needed Protocols
//
static EFI_GUID	IPMIGuid            = EFI_IPMI_PROTOCOL;

//
//  Loaded protocol workaround SystemTable
//
static EFI_SYSTEM_TABLE	BackupSystemTable;

//
// ----------------------------------------------------------------------------
//
// Name 
//		IPMIUnload
//
// Abstract:
//		Called when an unload request is made for this protocol
//
// Parameters:
//		ImageHandle	- Handle to our image
//
// Return Value:
//		EFI_SUCCESS
//
// ----------------------------------------------------------------------------
//

static EFI_STATUS
IPMIUnload ( EFI_HANDLE	ImageHandle )
{
	EFI_STATUS Status;

	//
	//  Remove our interface
	//
	Status = BS->UninstallProtocolInterface (ImageHandle, &IPMIGuid, &EfiIPMIInterface);
	if (EFI_ERROR(Status))
		Print(L"Error %x unloding interface\n", Status);

	//
	//  Cleanup libc and exit
	//
	_LIBC_Cleanup();

	return EFI_SUCCESS;
}

// ----------------------------------------------------------------------------
//
// Name 
//		EfiIPMIEntry
//
// Abstract:
//		Main entry point to the ipmi protocol interface.
//
// Parameters:
//		ImageHandle			- Handle to our image
//		*SystemTable		- Pointer to EFI system table
//
// Return Value:
//		EFI_STATUS
//
// ----------------------------------------------------------------------------
//

EFI_STATUS EFIAPI EfiIPMIEntry ( EFI_HANDLE			ImageHandle,
                                 EFI_SYSTEM_TABLE	*SystemTable
                               )

{
	EFI_STATUS	        Status, StatusSMBIOS;
	EFI_HANDLE	        Handle;
	EFI_LOADED_IMAGE	*Image;
	UINTN		        BufferSize = sizeof(Handle);
	EFI_DEVICE_PATH     *DevicePath;
	SMBIOSType38		*ptrSMBIOSType38;
	UINT16				Len, Key=0xFFFF;
	
	//
	//  Workaround loaded protocol bug
	//
	BackupSystemTable = *SystemTable;

	//
	//  Initialize the EFI Library.  This will set ST, BT, and RT globals
	//
	InitializeLib(ImageHandle, &BackupSystemTable);

	//
	//  Make sure we are not already loaded
	//
	Status = BS->LocateHandle (ByProtocol, &IPMIGuid, NULL, &BufferSize, &Handle);
	if (Status != EFI_NOT_FOUND)
	{
		return (EFI_ALREADY_STARTED);
	}	
	
	//
	//  Initialize the Device IO protocol
	//
	DevicePath = EndDevicePath;
	Status = BS->LocateDevicePath (&DeviceIoProtocol, &DevicePath, &Handle);

	if ( !EFI_ERROR ( Status ) ) 
    {
		GlobalIoFncs = NULL;
		Status = BS->HandleProtocol (Handle, &DeviceIoProtocol, (VOID*)&GlobalIoFncs);
	}

	if (GlobalIoFncs == NULL)
	{
		DEBUG ((D_LOAD, "GlobalIoFncs Not Found\n"));
		return EFI_UNSUPPORTED;
	}

	// 
	//  Get our image protocol so we can install an unload handler.
	//
   	Status = BS->HandleProtocol (ImageHandle, &LoadedImageProtocol, (VOID*)&Image);
	if ( EFI_ERROR( Status ) )
	{
		return EFI_LOAD_ERROR;
	}

	// 
	//  Install unload handler
	//
	Image->Unload = IPMIUnload;

	//
	//  It looks like we're going to run - initialize the C library
	//
	InitializeLibC (ImageHandle, &BackupSystemTable);

	//
	// Initialize IPMI interface
    //
	Status = InitIPMIInterface();

	if ( !EFI_ERROR ( Status)  )
    {

		// Install our interface
		Handle = NULL;
		Status = BS->InstallProtocolInterface
			                (
								&ImageHandle,
								&IPMIGuid,
								EFI_NATIVE_INTERFACE,
								&EfiIPMIInterface
							);

		if ( !EFI_ERROR ( Status ) )
		{
			//
			// Get KCS Address from SMBIOS table, Record Type 38
			// Note that if we can't get the KCS address from SMBIOS table,
			// the default address is then used and no errors are reported.
			//
			StatusSMBIOS = SMBIOS_Initialize ( ImageHandle, SystemTable );

			if ( !EFI_ERROR ( StatusSMBIOS ) )
			{

				StatusSMBIOS = SMBIOS_GetStructure 
				                      ( eSMBIOSType38,
									    0xFFFF,				// Get 1st structure
										(void**)&ptrSMBIOSType38,
										&Len,
										&Key
									  );

				if ( !EFI_ERROR ( StatusSMBIOS ) )
				{
					//
					// Replace default address with this one.  Note that bit 0 indicates 
					// whether address is in I/O space or memory mapped, per IPMI spec.
					//
					KCSAddress = (UINT16) (ptrSMBIOSType38->BaseAddress & 0xFFFE);

					// De-allocate memory that was allocated by SMBIOS_GetStructure()
					SMBIOS_FreeStructure ( ptrSMBIOSType38 );
				}
			}
		}
	}

	//
	//  If there were any errors, uninitilize libc
	//
	if ( EFI_ERROR( Status ) )
	{
		_LIBC_Cleanup();
	}
	//
	//  Print a warning if we did not obtain KCS address from the SMBIOS table
	//
	else if ( EFI_ERROR( StatusSMBIOS ) )
	{
		Print(L"Warning: No SMBIOS entry for KCS address - using fixed value of 0x%x\n", KCSAddress);
	}

	return Status;
}

// -------------------------------END------------------------------------------
