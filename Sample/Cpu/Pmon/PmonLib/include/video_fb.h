										    /*
										     * (C) Copyright 1997-2002 ELTEC Elektronik AG
										     * Frank Gottschling <fgottschling@eltec.de>
										     *
										     * See file CREDITS for list of people who contributed to this
										     * project.
										     *
										     * This program is free software; you can redistribute it and/or
										     * modify it under the terms of the GNU General Public License as
										     * published by the Free Software Foundation; either version 2 of
										     * the License, or (at your option) any later version.
										     *
										     * This program is distributed in the hope that it will be useful,
										     * but WITHOUT ANY WARRANTY; without even the implied warranty of
										     * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
										     * GNU General Public License for more details.
										     *
										     * You should have received a copy of the GNU General Public License
										     * along with this program; if not, write to the Free Software
										     * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
										     * MA 02111-1307 USA
										     */

/*
 * smiLynxEM.h
 * Silicon Motion graphic interface for sm810/sm710/sm712 accelerator
 *
 *
 *  modification history
 *  --------------------
 *  04-18-2002 Rewritten for U-Boot <fgottschling@eltec.de>.
 */

#ifndef _VIDEO_FB_H_
#define _VIDEO_FB_H_

#include "types.h"

#define CONSOLE_BG_COL            0x00
#define CONSOLE_FG_COL            0xa0

/*
 * Graphic Data Format (GDF) bits for VIDEO_DATA_FORMAT
 */
#define GDF__8BIT_INDEX         0
#define GDF_15BIT_555RGB        1
#define GDF_16BIT_565RGB        2
#define GDF_32BIT_X888RGB       3
#define GDF_24BIT_888RGB        4
#define GDF__8BIT_332RGB        5

/******************************************************************************/
/* Export Graphic Driver Control                                              */
/******************************************************************************/

typedef struct {
	UINTN isaBase;
	UINTN pciBase;
	UINTN dprBase;
	UINTN vprBase;
	UINTN cprBase;
	UINTN frameAdrs;
	UINTN memSize;
	UINTN mode;
	UINTN gdfIndex;
	UINTN gdfBytesPP;
	UINTN fg;
	UINTN bg;
	UINTN plnSizeX;
	UINTN plnSizeY;
	UINTN winSizeX;
	UINTN winSizeY;
	char modeIdent[80];
} GraphicDevice;

/******************************************************************************/
/* Export Graphic Functions                                                   */
/******************************************************************************/

void *video_hw_init(void);	/* returns GraphicDevice struct or NULL */




#endif /*_VIDEO_FB_H_ */
