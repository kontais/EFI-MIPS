/* 
 * console.c - Console screen handling functions
 *
 *  Copyright (C) 2006 Christoph Pfisterer
 *
 * This file is part of the ELILO, the EFI Linux boot loader.
 *
 *  ELILO is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  ELILO is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with ELILO; see the file COPYING.  If not, write to the Free
 *  Software Foundation, 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 * Please check out the elilo.txt for complete documentation on how
 * to use this program.
 */

#include <efi.h>
#include <efilib.h>

#include "elilo.h"

#include <efiConsoleControl.h>

static EFI_GUID console_guid = EFI_CONSOLE_CONTROL_PROTOCOL_GUID;

static BOOLEAN console_inited = FALSE;

static EFI_CONSOLE_CONTROL_PROTOCOL *console_control;

/*
 * Initialize console functions
 */
static VOID console_init(VOID)
{
	EFI_STATUS status;

	if (!console_inited) {
		console_inited = TRUE;

		status = LibLocateProtocol(&console_guid, (VOID **) &console_control);
		if (EFI_ERROR(status))
			console_control = NULL;
	}
}

/*
 * Switch the console to text mode
 */

VOID console_textmode(VOID)
{
	EFI_CONSOLE_CONTROL_SCREEN_MODE console_mode;

	console_init();

	if (console_control != NULL) {
		console_control->GetMode (console_control, &console_mode, NULL, NULL);
		if (console_mode == EfiConsoleControlScreenGraphics)
			console_control->SetMode (console_control, EfiConsoleControlScreenText);
	}
}
