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

/*++

Module Name:

    EfiInterface.c
    
Abstract:

    TTY emulation library initialization routines.


Revision History

--*/

#include <atk_libc.h>
#include <atk_libtty.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <sys/ttycom.h>
#include <sys/poll.h>
#include <fcntl.h>
#include <unistd.h>
#include "../libc/sys/filedesc.h"

static int	_ttyLibIsInitialized = FALSE;
static int	_ttyInterfaces = -1;
static int	_LineDisc = TTYDISC; 

static EFI_GUID	SerialGuid     = SERIAL_IO_PROTOCOL;
//{ 0x11111111, 0x1111, 0x1111, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11 };

static EFI_GUID	DevicePathGuid = DEVICE_PATH_PROTOCOL;

#define	MAX_TTY_NAME	8
typedef struct _ttyListEntry {
	SERIAL_IO_INTERFACE		*pIface;
	EFI_DEVICE_PATH			*DevicePath;
	BOOLEAN					InUse;
	char					ttyName[ MAX_TTY_NAME ];
	struct termios			termios;
	UINT32					ModemControl;
} ttyListEntry_t;

typedef struct _ttyInstance {
	ttyListEntry_t	*Dev;
	int				fd;
} ttyInstance_t;

static ttyListEntry_t	*ttyList = NULL;

#define DEFAULT_TERMIOS_IFLAG		(IGNBRK | INPCK)
#define DEFAULT_TERMIOS_OFLAG		0
#define DEFAULT_TERMIOS_LFLAG		0

tcflag_t
Efi2cflags( SERIAL_IO_MODE	*Mode )
{
	tcflag_t	Flags = (CREAD | CLOCAL);

	switch (Mode->DataBits) {
		case 5: Flags |= CS5; break;
		case 6: Flags |= CS6; break;
		case 7: Flags |= CS7; break;
		case 8: Flags |= CS8; break;
	}
	switch (Mode->Parity) {
		case EvenParity: Flags |= PARENB;            break;
		case OddParity:	 Flags |= (PARENB | PARODD); break;
	};

	if (Mode->StopBits == TwoStopBits)	Flags |= CSTOPB;

	return Flags;
}

void
cflags2Efi( tcflag_t Flags, SERIAL_IO_MODE *Mode )
{
	Mode->DataBits = (Flags & CSIZE) == CS5 ? 5 :
					 (Flags & CSIZE) == CS6 ? 6 :
					 (Flags & CSIZE) == CS7 ? 7 : 8;

	Mode->Parity = (Flags & (PARENB|PARODD)) ==  PARENB         ? EvenParity :
	               (Flags & (PARENB|PARODD)) == (PARENB|PARODD) ? OddParity  : NoParity;

	Mode->StopBits = Flags & CSTOPB ? TwoStopBits : OneStopBit;
}

UINT32
UsecPerChar( UINT64 BaudRate)
{
	UINT32	Timeout;

	Timeout = (1000000 / (UINT32)BaudRate) * 10;
	return (Timeout);
}

/*++

Routine Description:

    Map read(2) call to tty operation

Arguments:
    
    Buffer		- Pointer to input buffer
	BufSize		- On input, size of input buffer.  On output, number of bytes read.
	DevSpecific	- Pointer to device specific context.

Returns:

    EFI status code

--*/
static EFI_STATUS
ttyRead(
    VOID   *Buffer,
    UINTN  *BufSize,
    VOID   *DevSpecific
    )
{
	ttyInstance_t	*ptty = DevSpecific;
	EFI_STATUS		Status;
	UINTN			DataRead, DataRemaining, Total;
	UINT8			*Buf;
	BOOLEAN			Done;

	Done = FALSE;
	Buf = Buffer;
	DataRemaining = *BufSize;
	Total = 0;
	while ( DataRemaining && !Done ) {
		/*
		 *  Read data from tty
		 */
		DataRead = DataRemaining;
		Status = ptty->Dev->pIface->Read( ptty->Dev->pIface, &DataRead, Buf );
		if ( EFI_ERROR(Status) ) {
			if (Status == EFI_TIMEOUT) {
				INT32	flags = 0;

				_GetOpenFileFlags(ptty->fd, &flags);
				if (flags & O_NONBLOCK) {
					/*
					 *  Convert error code to get EAGAIN on timeouts with
					 *  no data returned
					 */
					if (DataRead > 0) {
						Status = EFI_SUCCESS;
					} else {
						Status = EFI_NOT_READY;
					}
					/*
					 *  non-blocking timeout - we're done
					 */
					Done = TRUE;
				}
			} else {
				/*
				 *  non-timeout releated error - we're done
				 */
				Done = TRUE;
			}
		}

		/*
		 *  Update for next pass
		 */
		Buf += DataRead;
		Total += DataRead;
		DataRemaining -= DataRead;
	}

	/*
	 *  Update how much we actually read
	 */
	*BufSize = Total;
	return (Status);
}

/*++

Routine Description:

    Map write(2) call to tty operation

Arguments:
    
    Buffer		- Pointer to input buffer
	BufSize		- On input, size of output buffer.  On output, number of bytes written.
	DevSpecific	- Pointer to device specific context.

Returns:

    EFI status code

--*/
static EFI_STATUS
ttyWrite(
    VOID   *Buffer,
    UINTN  *BufSize,
    VOID   *DevSpecific
    )
{
	ttyInstance_t	*ptty = DevSpecific;
	EFI_STATUS		Status;
	UINTN			DataWritten, DataRemaining, Total;
	UINT8			*Buf;
	BOOLEAN			Done;

	Done = FALSE;
	Buf = Buffer;
	DataRemaining = *BufSize;
	Total = 0;
	while ( DataRemaining && !Done ) {
		/*
		 *  Read data from tty
		 */
		DataWritten = DataRemaining;
		Status = ptty->Dev->pIface->Write( ptty->Dev->pIface, &DataWritten, Buf );
		if ( EFI_ERROR(Status) ) {
			if (Status == EFI_TIMEOUT) {
				INT32	flags = 0;

				_GetOpenFileFlags(ptty->fd, &flags);
				if (flags & O_NONBLOCK) {
					/*
					 *  Convert error code to get EAGAIN on timeouts with
					 *  no data returned
					 */
					if (DataWritten > 0) {
						Status = EFI_SUCCESS;
					} else {
						Status = EFI_NOT_READY;
					}
					/*
					 *  non-blocking timeout - we're done
					 */
					Done = TRUE;
				}
			} else {
				/*
				 *  non-timeout releated error - we're done
				 */
				Done = TRUE;
			}
		}

		/*
		 *  Update for next pass
		 */
		Buf += DataWritten;
		Total += DataWritten;
		DataRemaining -= DataWritten;
	}

	/*
	 *  Update how much we actually read
	 */
	*BufSize = Total;
	return (Status);
}

/*++

Routine Description:

    Map close(2) call to tty operation

Arguments:
    
	DevSpecific	- Pointer to device specific context.

Returns:

    EFI status code

--*/
static EFI_STATUS
ttyClose(
    VOID  *DevSpecific
    )
{
	ttyInstance_t	*ptty = DevSpecific;

	/*
	 *  Clear in use flag.
	 */
	ptty->Dev->InUse--;
	if (ptty->Dev->InUse < 0) {
		ptty->Dev->InUse = 0; /* should never happen */
	}

	free (ptty);
	return (EFI_SUCCESS);
}

/*++

Routine Description:

    Map lseek(2) call to tty operation

Arguments:
    
    Position	- Offset into file
	Whence		- Relativity of Position.
	DevSpecific	- Pointer to device specific context.

Returns:

    EFI_UNSUPPORTED - This call is not supported on a tty.

--*/
static EFI_STATUS
ttySeek(
    IN UINT64  *Position,
    IN UINT32  Whence,
    IN VOID    *DevSpecific
    )
{
	return (EFI_UNSUPPORTED);
}

/*++

Routine Description:

    Map stat(2) call to tty operation

Arguments:
    
    StatBuf		- Pointer to stat buffer
	DevSpecific	- Pointer to device specific context.

Returns:

    EFI_UNSUPPORTED - This call is not supported on a tty.

--*/
static EFI_STATUS
ttyFstat(
    IN struct stat *StatBuf,
    IN VOID        *DevSpecific
    )
{
    StatBuf->st_dev               = 0;
    StatBuf->st_ino               = 0;
    StatBuf->st_uid               = 0;
    StatBuf->st_gid               = 0;
    StatBuf->st_mode              = ALLPERMS | S_IFCHR;
    StatBuf->st_rdev              = 0;
    StatBuf->st_atimespec.tv_sec  = 0;
    StatBuf->st_atimespec.tv_nsec = 0;
    StatBuf->st_mtimespec.tv_sec  = 0;
    StatBuf->st_mtimespec.tv_nsec = 0;
    StatBuf->st_ctimespec.tv_sec  = 0;
    StatBuf->st_ctimespec.tv_nsec = 0;
    StatBuf->st_blocks            = 0;
    StatBuf->st_flags             = 0;
    StatBuf->st_gen               = 0;

    return (EFI_SUCCESS);
}

/*++

Routine Description:

    Map ioctl(2) call to tty operation

Arguments:
    
	DevSpecific	- Pointer to device specific context.
    Request		- Ioctl request
	...			- Variable argument list.

Returns:

    EFI status code

--*/
static EFI_STATUS
ttyIoctl(
    IN VOID    *DevSpecific,
    IN UINT32  Request,
	IN va_list pArgs
    )
{
	struct termios			*termios;
	VOID					*Argp;
	SERIAL_IO_MODE			Mode;
	int						Bits;
	ttyInstance_t			*ptty   = DevSpecific;
	SERIAL_IO_INTERFACE		*pIface = ptty->Dev->pIface;

	/*
	 *  Get a ioctl argument pointer
	 */
	Argp = va_arg (pArgs, void*);

	switch ( Request ) {
		case TIOCGETA:
			termios = (struct termios*)Argp;
			*termios = ptty->Dev->termios;
			return (EFI_SUCCESS);

		/*
		 *  Note that we don't deal with flush and drain semantics
		 */
		case TIOCSETA:
		case TIOCSETAW:
		case TIOCSETAF:
			termios = (struct termios*)Argp;

			/*
			 *  Update our copy and convert c_cflags
			 *  to EFI mode vallues.
			 */
			ptty->Dev->termios = *termios;
			cflags2Efi( termios->c_cflag, &Mode );

			/*
			 *  Make the change
			 */
			return pIface->SetAttributes(
						pIface,
						(UINT64)termios->c_ispeed,
						pIface->Mode->ReceiveFifoDepth,
						/*
						 *  The timeout MUST be at least as long as it
						 *  takes to send a single character or we will
						 *  always get a timeout when sending more than
						 *  two characters.  The +10 is a fudge factor.
						 */
						UsecPerChar((UINT64)termios->c_ispeed) + 10,
						Mode.Parity,
						(UINT8)Mode.DataBits,
						Mode.StopBits
						);

		case TIOCGETD: {
			int	*pDisc = Argp;
			*pDisc = _LineDisc;
			return (EFI_SUCCESS);
		}


		case TIOCSDTR:
			ptty->Dev->ModemControl |= EFI_SERIAL_REQUEST_TO_SEND;
			return (pIface->SetControl( pIface, ptty->Dev->ModemControl ));

		case TIOCCDTR:
			ptty->Dev->ModemControl &= ~EFI_SERIAL_DATA_TERMINAL_READY;
			return (pIface->SetControl( pIface, ptty->Dev->ModemControl ));

		case TIOCMBIS:
			Bits = *((int*)Argp);

			if ( Bits & TIOCM_RTS )
				ptty->Dev->ModemControl |= EFI_SERIAL_REQUEST_TO_SEND;

			if ( Bits & TIOCM_DTR )
				ptty->Dev->ModemControl |= EFI_SERIAL_DATA_TERMINAL_READY;

			return (pIface->SetControl( pIface, ptty->Dev->ModemControl ));

		case TIOCMBIC:
			Bits = *((int*)Argp);

			if ( Bits & TIOCM_RTS )
				ptty->Dev->ModemControl &= ~EFI_SERIAL_REQUEST_TO_SEND;

			if ( Bits & TIOCM_DTR )
				ptty->Dev->ModemControl &= ~EFI_SERIAL_DATA_TERMINAL_READY;

			return (pIface->SetControl( pIface, ptty->Dev->ModemControl ));

		case TIOCMGET:
		case TIOCMODG: {
			UINT32		Control;
			EFI_STATUS	Status;

			Status = pIface->GetControl( pIface, &Control );
			if (EFI_ERROR(Status))
				return (Status);

			Bits = 0;
			if (Control & EFI_SERIAL_CLEAR_TO_SEND)		Bits |= TIOCM_CTS;
			if (Control & EFI_SERIAL_DATA_SET_READY)	Bits |= TIOCM_DSR;
			if (Control & EFI_SERIAL_RING_INDICATE)		Bits |= TIOCM_RI;
			if (Control & EFI_SERIAL_CARRIER_DETECT)	Bits |= TIOCM_CD;
			if (ptty->Dev->ModemControl & EFI_SERIAL_REQUEST_TO_SEND)
				Bits |= TIOCM_RTS;
			if (ptty->Dev->ModemControl & EFI_SERIAL_DATA_TERMINAL_READY)
				Bits |= TIOCM_DTR;

			*((int*)Argp) = Bits;
			return (EFI_SUCCESS);
		}

		case TIOCMSET:
		case TIOCMODS: {
			Bits = *((int*)Argp);

			if (Bits & TIOCM_RTS) {
				ptty->Dev->ModemControl |= EFI_SERIAL_REQUEST_TO_SEND;
			} else {
				ptty->Dev->ModemControl &= ~EFI_SERIAL_REQUEST_TO_SEND;
			}
			if (Bits & TIOCM_RTS) {
				ptty->Dev->ModemControl |= EFI_SERIAL_DATA_TERMINAL_READY;
			} else {
				ptty->Dev->ModemControl &= ~EFI_SERIAL_DATA_TERMINAL_READY;
			}

			return (pIface->SetControl( pIface, ptty->Dev->ModemControl ));
		}
	}

	return (EFI_UNSUPPORTED);
}

static EFI_STATUS
ttyPoll(
    IN UINT32  Mask,
    IN VOID    *DevSpecific
    )
/*++

Routine Description:

    Map poll call to tty operation

Arguments:
    
    Mask		- mask of events of interest
	DevSpecific	- Pointer to device specific context.

Returns:

    EFI_NOT_READ	- Nothing ready in mask
    EFI_SUCCESS		- Something ready in mask

--*/
{
	ttyInstance_t	*ptty = DevSpecific;
	EFI_STATUS		Status;
	UINT32			Control;

	Status = ptty->Dev->pIface->GetControl( ptty->Dev->pIface, &Control );
	if ( ! EFI_ERROR( Status ) ) {
		/*
		 *  The EFI interface does not allow us to see if the TX queue is
		 *  empty so we always say we can transmit and hope for the best
		 */
		if (((Mask & POLLRDNORM) && !(Control & EFI_SERIAL_INPUT_BUFFER_EMPTY)) ||
			 (Mask & POLLWRNORM)) {
			return (EFI_SUCCESS);
		}
	}

	return (EFI_NOT_READY);
}

/*++

Routine Description:

    Map open(2) call to tty operations

Arguments:
    
	FilePath	- Path of file to open
	DevName		- Device name to use for mapping
	Flags		- Flags
	Mode		- Mode
	DevPath		- Device path
	Guid		- Guid
	fd			- Pointer to file descriptor returned on success

Returns:

    EFI status code

--*/
EFI_STATUS
ttyOpen(
	char			*FilePath,
	char			*DevName,
	int				Flags,
	mode_t			Mode,
	EFI_DEVICE_PATH	*DevPath,
	EFI_GUID		*Guid,
	INT32			*fd
	)
{
	SERIAL_IO_INTERFACE		*pIface;
	wchar_t					*wcspath;
    EFI_STATUS				Status;
	ttyListEntry_t			*pDev;
	ttyInstance_t			*ptty;
	int						i;

	/*
	 *  Assume the worst
	 */
	*fd = -1;

	/*
	 *  Find the tty in question
	 */
	for ( pDev = ttyList, i = 0; i < _ttyInterfaces; i++, pDev++ ) {
		if (strcmp (DevName, pDev->ttyName) == 0) {
			break;
		}
	}

	if (i == _ttyInterfaces)
		return (EFI_INVALID_PARAMETER);

	/*
	 *  Allocate a tty instance
	 */
	ptty = malloc( sizeof(ttyInstance_t) );
	if (ptty == NULL)
		return (EFI_OUT_OF_RESOURCES);

	pDev->ModemControl = EFI_SERIAL_REQUEST_TO_SEND | EFI_SERIAL_DATA_TERMINAL_READY;
	ptty->Dev = pDev;
	pIface    = pDev->pIface;

	/*
	 *  Convert path to UNICODE
	 */
	if ( FilePath ) {
		wcspath = calloc (strlen(FilePath) + 1, sizeof(wchar_t));
		if (wcspath == NULL) {
			free( ptty );
			return (EFI_OUT_OF_RESOURCES);
		}
		mbstowcs (wcspath, FilePath, strlen(FilePath) + 1);
	} else {
		wcspath = L"";
	}

	/*
	 *  Allocate an initialized file descriptor
	 */
	Status = _LIBC_AllocateNewFileDescriptor( 
				wcspath,     // FileName
				Flags,       // Flags
				Mode,        // Mode
				FALSE,       // IsATTy - TRUE would convert stream to Unicode
				ptty,	     // DevSpecific
				ttyRead,     // read
				ttyWrite,    // write
				ttyClose,    // close
				ttySeek,     // lseek
				ttyFstat,    // fstat
				ttyIoctl,    // ioctl stub
				ttyPoll,     // poll stub
				fd           // New FileDescriptor
				);

	/*
	 *  Reset the port if this is the first open
	 */
	if (pDev->InUse == 0) {
		Status = pIface->Reset( pIface );
	} else {
		Status = EFI_SUCCESS;
	}
	if (!EFI_ERROR(Status)) {
		SERIAL_IO_MODE	*pMode = pIface->Mode;

		Status = pIface->SetAttributes(
						pIface,
						pMode->BaudRate,
						15,
						/*
						 *  The timeout MUST be at least as long as it
						 *  takes to send a single character or we will
						 *  always get a timeout when sending more than
						 *  two characters.  The +10 is a fudge factor.
						 */
						UsecPerChar(pMode->BaudRate) + 10,
						pMode->Parity,
						(UINT8)pMode->DataBits,
						pMode->StopBits
						);

		if (!EFI_ERROR(Status)) {
			pDev->termios.c_cflag     = Efi2cflags(pMode);
			pDev->termios.c_cc[VTIME] = (cc_t)(pMode->Timeout / (1000 * 100));
			pDev->termios.c_cc[VMIN]  = 1;
			pDev->termios.c_ispeed    = (speed_t)pMode->BaudRate;
			pDev->termios.c_ospeed    = (speed_t)pMode->BaudRate;

			/*
			 *  Initialize the termios defaults if first open
			 */
			if (pDev->InUse == 0) {
				pDev->termios.c_iflag = DEFAULT_TERMIOS_IFLAG;
				pDev->termios.c_oflag = DEFAULT_TERMIOS_OFLAG;
				pDev->termios.c_lflag = DEFAULT_TERMIOS_LFLAG;

				Status = pIface->SetControl( pIface, pDev->ModemControl );
			}
		}
	}

	if (FilePath)
		free (wcspath);

	if (!EFI_ERROR(Status)) {
		/*
		 *  If a blocking open, wait for carrier detect
		 */
		if (!(Flags & O_NONBLOCK)) {
			UINT32	Bits;
			int	CallAnswered = FALSE;

			do {
				Status = pIface->GetControl( pIface, &Bits );

				/*
				 *  In lieu of a true modem driver.....
				 */
				if (!CallAnswered && !EFI_ERROR(Status) && (Bits & EFI_SERIAL_RING_INDICATE)) {
					UINTN	DataWritten = 5;
					Status = pIface->Write( pIface, &DataWritten, "ATA\r\n");
					if (!EFI_ERROR(Status)) {
						/*
						 *  Give modem a little time to answer becase. CallAnswered will be cleared
						 *  on next pass of the loop to prevent possible deadlock.
						 */
						usleep( 20000 );
						CallAnswered = TRUE;
					}
				} else {
					/*
					 *  To prevent possible deadlock, we can't leave CallAnswered TRUE.  Deadlock
					 *  could occur where they hang up while we are sending the ATA.
					 */
					CallAnswered = FALSE;
				}
			} while (!EFI_ERROR(Status) && !(Bits & EFI_SERIAL_CARRIER_DETECT));
		}

		if (!EFI_ERROR(Status)) {
			pDev->InUse++;
			ptty->fd = *fd;
		}
	}

	if (EFI_ERROR(Status))
		free (ptty);

	return( Status );
}
/*++

Routine Description:

    Map all serial interfaces

Arguments:
    
    BS		- Pointer Boot Services table.

Returns:

	EFI_SUCCSS		An acceptable interface was found 
	Other EFI status codes as needed

--*/
static int
MapSerialProtocols(
	EFI_BOOT_SERVICES		*BS
	)
{
    EFI_HANDLE      *pHandle;
    UINTN           BufferSize;
    EFI_STATUS      Status;
	UINTN			HandleCount;
	ttyListEntry_t	*pDev;
	int				i;

    /*
     *  Locate all serial protocol interfaces
     */
    BufferSize = sizeof(EFI_HANDLE) * 5;

RetryAllocate:
	pHandle = (EFI_HANDLE*)malloc (BufferSize);
	if (pHandle == NULL) {
		return -1;
	}

    Status = BS->LocateHandle (
						ByProtocol,
						&SerialGuid,
						NULL,
						&BufferSize,
						pHandle
						);
    /*
     *  See if the buffer was big enough
     */
	if (Status == EFI_BUFFER_TOO_SMALL) {
		free (pHandle);
		goto RetryAllocate;
    } else if (EFI_ERROR (Status)) {

		/*
		 *  There was an error - bail
		 */
		free (pHandle);
		return -1;
	}

	/*
	 *  Get a count of the number of handles
	 */
	HandleCount = BufferSize / sizeof(EFI_HANDLE);

	/*
	 *  Allocate memory for the tty list
	 */
	ttyList = calloc( sizeof(ttyListEntry_t), HandleCount );
	if ( ttyList == NULL ) {
		return -1;
	}

	/*
	 *  Map the protocol
	 */
    Status = _LIBC_MapProtocol( &SerialGuid, ttyOpen );
	if (EFI_ERROR (Status)) {
		free (pHandle);
		return -1;
	}

    /*
     *  Now map each device found
     */
    for (pDev = ttyList, i = 0; i < (int)HandleCount; i++, pDev++) {

		/*
		 *  Get the interface for this instance of the protocol
		 */
		Status = BS->HandleProtocol (
						pHandle[ i ],
						&SerialGuid,
						(VOID*)&pDev->pIface
						);
		if (EFI_ERROR (Status)) {
			free (pHandle);
			return -1;
		}

		/*
		 *  Get the device path of the device
		 */
		Status = BS->HandleProtocol (
						pHandle[ i ],
						&DevicePathGuid,
						(VOID*)&pDev->DevicePath
						);

		if (EFI_ERROR (Status)) {
			free (pHandle);
			return -1;
		}

		/*
		 *  Assign a name
		 */
		snprintf(pDev->ttyName, sizeof(pDev->ttyName), "tty%02d:", i);
	
		/*
		 *  Map the device instance
		 */
		Status = _LIBC_MapDevice(&SerialGuid, pDev->DevicePath, pDev->ttyName);
		if (EFI_ERROR (Status)) {
			free (pHandle);
			return -1;
		}
	}

	free (pHandle);
	return (i);
}

/*++

Routine Description:

    Initialize the EFI Serial Protocol Interfaces for use by the
	tty library.  This call can be made explicitly or implicitly.
	Implicit use assumes the caller has already called InitializeLibC.

Arguments:
    
	ImageHandle		- Optional image handle for app using this library
	pSystemTable	- Optional pointer EFI system table

Returns:

	Numer of interfaces on success, -1 on error

--*/
int
ttyInit(
	EFI_HANDLE			ImageHandle,
	EFI_SYSTEM_TABLE	*pSystemTable
	)
{
	/*
	 *  If we've already been here, return success
	 */
	if (_ttyLibIsInitialized == TRUE) {
		return (_ttyInterfaces);
	}

	/*
	 *  Make sure we only do this once
	 */
	_ttyLibIsInitialized = TRUE;
	_ttyInterfaces = -1;

	/*
	 *  Make sure libc has been initialized.
	 */
	if (_LIBC_EFISystemTable == NULL) {

		/*
		 *  We need to do initialization.  Make sure they passed
		 *  valid pointers.
		 */
		if (ImageHandle == NULL || pSystemTable == NULL) {
			return (-1);
		} else  if (InitializeLibC (ImageHandle, pSystemTable)) {
			return (-1);
		}
	}

	/*
	 *  Map all serial protocols into libc's file descriptor subsystem
	 */
	_ttyInterfaces = MapSerialProtocols(_LIBC_EFISystemTable->BootServices);
	return (_ttyInterfaces);
}
