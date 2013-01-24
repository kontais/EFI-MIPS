/*
 * (C) Copyright 2002 ELTEC Elektronik AG
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
 * cfb_console.c
 *
 * Color Framebuffer Console driver for 8/15/16/24/32 bits per pixel.
 *
 * At the moment only the 8x16 font is tested and the font fore- and
 * background color is limited to black/white/gray colors. The Linux
 * logo can be placed in the upper left corner and additional board
 * information strings (that normaly goes to serial port) can be drawed.
 *
 * The console driver can use the standard PC keyboard interface (i8042)
 * for character input. Character output goes to a memory mapped video
 * framebuffer with little or big-endian organisation.
 * With environment setting 'console=serial' the console i/o can be
 * forced to serial port.

 The driver uses graphic specific defines/parameters/functions:

 (for SMI LynxE graphic chip)

 CONFIG_VIDEO_SMI_LYNXEM - use graphic driver for SMI 710,712,810
 VIDEO_FB_LITTLE_ENDIAN  - framebuffer organisation default: big endian
 VIDEO_HW_RECTFILL   - graphic driver supports hardware rectangle fill
 VIDEO_HW_BITBLT     - graphic driver supports hardware bit blt

 Console Parameters are set by graphic drivers global struct:

 VIDEO_VISIBLE_COLS      - x resolution
 VIDEO_VISIBLE_ROWS      - y resolution
 VIDEO_PIXEL_SIZE        - storage size in byte per pixel
 VIDEO_DATA_FORMAT       - graphical data format GDF
 VIDEO_FB_ADRS           - start of video memory

 CONFIG_I8042_KBD        - AT Keyboard driver for i8042
 VIDEO_KBD_INIT_FCT      - init function for keyboard
 VIDEO_TSTC_FCT          - keyboard_tstc function
 VIDEO_GETC_FCT          - keyboard_getc function

 CONFIG_CONSOLE_CURSOR       - on/off drawing cursor is done with delay
                   loop in VIDEO_TSTC_FCT (i8042)
 CFG_CONSOLE_BLINK_COUNT     - value for delay loop - blink rate
 CONFIG_CONSOLE_TIME         - display time/date in upper right corner,
                   needs CFG_CMD_DATE and CONFIG_CONSOLE_CURSOR
 CONFIG_VIDEO_LOGO       - display Linux Logo in upper left corner
 CONFIG_VIDEO_BMP_LOGO       - use bmp_logo instead of linux_logo
 CONFIG_CONSOLE_EXTRA_INFO   - display additional board information strings
                   that normaly goes to serial port. This define
                   requires a board specific function:
                   video_drawstring (VIDEO_INFO_X,
                         VIDEO_INFO_Y + i*VIDEO_FONT_HEIGHT,
                         info);
                   that fills a info buffer at i=row.
                   s.a: board/eltec/bab7xx.
CONFIG_VGA_AS_SINGLE_DEVICE  - If set the framebuffer device will be initialised
                   as an output only device. The Keyboard driver
                   will not be set-up. This may be used, if you
                   have none or more than one Keyboard devices
                   (USB Keyboard, AT Keyboard).

CONFIG_VIDEO_SW_CURSOR:      - Draws a cursor after the last character. No
                   blinking is provided. Uses the macros CURSOR_SET
                   and CURSOR_OFF.
CONFIG_VIDEO_HW_CURSOR:      - Uses the hardware cursor capability of the
                   graphic chip. Uses the macro CURSOR_SET.
                   ATTENTION: If booting an OS, the display driver
                   must disable the hardware register of the graphic
                   chip. Otherwise a blinking field is displayed
*/

/************************************************************************

 Copyright (C)
 File name:     cfb_console.c
 Author:  ***      Version:  ***      Date: ***
 Description:   
 Others:        
 Function List:
 
 Revision History:
 
 --------------------------------------------------------------------------
  Date                Author            Activity ID               Activity Headline
  2008-05-17    Zhouhe          PMON20080517    disableoutput() enableoutput() added,cursor bug fixed.
  2009-07-17   QianYuli          PMON20090202     Modified for implement of BIOS-LIKE function
*************************************************************************/








/*****************************************************************************/
/* Include video_fb.h after definitions of VIDEO_HW_RECTFILL etc         */
/*****************************************************************************/
#include "video_fb.h"
#include "libc.h"
#include "pci_machdep.h"
#include "sm712.h"
#include "bonitopmon.h"
#include "video_font.h"
#include "cfb_console.h"

int vga_available = 0;

char console_buffer[2][38][128];
/* Locals */
static GraphicDevice *pGD, GD;  /* Pointer to Graphic array */
void *video_fb_address;  /* frame buffer address */
static void *video_console_address; /* console buffer start address */
static int console_col = 0; /* cursor col */
static int console_row = 0; /* cursor row */

static const int video_font_draw_table16[] = {
    /* 0x00000000, 0x42080000, 0x00004208, 0x42084208 */
    0x00000000, 0xffff0000, 0x0000ffff, 0xffffffff
		
};



/******************************************************************************/
void video_drawchars(int xx, int yy, UINT8 *s, int count)
{
    UINT8 *cdat, *dest, *dest0;
    int rows, offset, c;

    offset = yy * VIDEO_LINE_LEN + xx * VIDEO_PIXEL_SIZE;
    dest0 = video_fb_address + offset;

        while (count--) {
            c = *s;
            cdat = video_fontdata + c * VIDEO_FONT_HEIGHT;
            for (rows = VIDEO_FONT_HEIGHT, dest = dest0; rows--; dest += VIDEO_LINE_LEN) {
                UINT8 bits = *cdat++;

                ((UINTN *)dest)[0] = SHORTSWAP32((video_font_draw_table16[bits >> 6]));
                ((UINTN *)dest)[1] = SHORTSWAP32((video_font_draw_table16[bits >> 4 & 3]));
                ((UINTN *)dest)[2] = SHORTSWAP32((video_font_draw_table16[bits >> 2 & 3]));
                ((UINTN *)dest)[3] = SHORTSWAP32((video_font_draw_table16[bits & 3]));
            }
            dest0 += VIDEO_FONT_WIDTH * VIDEO_PIXEL_SIZE;
            s++;
        }
}

/*****************************************************************************/

void video_putchar(int xx, int yy, UINT8 c)
{
    video_drawchars(xx, yy + VIDEO_LOGO_HEIGHT, &c, 1);
}



/*****************************************************************************/

void memsetl(UINT8 *p, int c, int v)
{
    while (c--)
        *(p++) = v;
}

/*****************************************************************************/

void memcpyl(int *d, int *s, int c)
{
    while (c--)
        *(d++) = *(s++);
}

/*****************************************************************************/

static void console_scrollup(void)
{
    /* copy up rows ignoring the first one */
#ifdef VIDEO_HW_BITBLT
    video_hw_bitblt(VIDEO_PIXEL_SIZE,   /* bytes per pixel */
            0,  /* source pos x */
            VIDEO_LOGO_HEIGHT + VIDEO_FONT_HEIGHT,  /* source pos y */
            0,  /* dest pos x */
            VIDEO_LOGO_HEIGHT,  /* dest pos y */
            VIDEO_VISIBLE_COLS, /* frame width */
            VIDEO_VISIBLE_ROWS - VIDEO_LOGO_HEIGHT - VIDEO_FONT_HEIGHT  /* frame height */
        );


#else
    memcpyl (CONSOLE_ROW_FIRST, CONSOLE_ROW_SECOND,
         CONSOLE_SCROLL_SIZE >> 2);
#endif

    /* clear the last one */
#ifdef VIDEO_HW_RECTFILL
    video_hw_rectfill(VIDEO_PIXEL_SIZE, /* bytes per pixel */
              0,    /* dest pos x */
              (VIDEO_VISIBLE_ROWS - VIDEO_FONT_HEIGHT), /* dest pos y */
              VIDEO_VISIBLE_COLS,   /* frame width */
              VIDEO_FONT_HEIGHT,    /* frame height */
              CONSOLE_BG_COL    /* fill color */
        );
#else
    memsetl(CONSOLE_ROW_LAST, CONSOLE_ROW_SIZE, CONSOLE_BG_COL);
#endif
}

/*****************************************************************************/

static void console_back(void)
{
    console_col--;

    if (console_col < 0) {
        console_col = CONSOLE_COLS - 1;
        console_row--;
        if (console_row < 0)
            console_row = 0;
    }
    video_putchar(console_col * VIDEO_FONT_WIDTH,
              console_row * VIDEO_FONT_HEIGHT, ' ');
}

/*****************************************************************************/

static void console_newline(void)
{
    console_row++;
    console_col = 0;

    /* Check if we need to scroll the terminal */
    if (console_row >= CONSOLE_ROWS) {
        /* Scroll everything up */
        console_scrollup();

        /* Decrement row number */
        console_row--;
    }
}

/*****************************************************************************/

void video_putc(const char c)
{
	if (vga_available == 0) 
		return;
	
    switch (c) {
    case 13:        /* ignore */
        break;

    case '\n':      /* next line */
        console_newline();
        break;

    case 9:     /* tab 8 */
        console_col |= 0x0008;
        console_col &= ~0x0007;

        if (console_col >= CONSOLE_COLS)
            console_newline();
        break;

    case 8:     /* backspace */
        console_back();
        break;

    default:        /* draw the char */
        video_putchar(console_col * VIDEO_FONT_WIDTH,
                  console_row * VIDEO_FONT_HEIGHT, c);
        console_col++;

        /* check for newline */
        if (console_col >= CONSOLE_COLS)
            console_newline();
    }
}

/*****************************************************************************/

void video_puts(const char *s)
{
	if (vga_available == 0) 
		return;

    int count = strlen(s);

    while (count--)
        video_putc(*s++);
}


/*****************************************************************************/
int GetGDFIndex(int BytesPP)
{
    switch (BytesPP) {
    case 1:
        return GDF__8BIT_INDEX;
    case 2:
        return GDF_16BIT_565RGB;
    case 4:
        return GDF_32BIT_X888RGB;
    default:
        return -1;
    }
    return -1;
}

int fb_init(UINTN fbbase, UINTN iobase)
{

    pGD = &GD;
        pGD->winSizeX = GetXRes();
        pGD->winSizeY = GetYRes();

        pGD->gdfBytesPP = GetBytesPP();
        pGD->gdfIndex = GetGDFIndex(GetBytesPP());

    pGD->frameAdrs = 0xb0000000 | fbbase;

    


    video_fb_address = (void *)VIDEO_FB_ADRS;

    memsetl(video_fb_address, CONSOLE_SIZE,  CONSOLE_BG_COL);

    video_console_address = video_fb_address;

    /* Initialize the console */
    console_col = 0;
    console_row = 0;

    memset(console_buffer, ' ', sizeof console_buffer);

    return 0;
	
}



	
void vga_init(void)
{
	UINT8 temp;
	UINTN fbaddress,ioaddress;
	extern struct pci_device *vga_dev;

	//
	//  Turn off LCD
	//
#define	HIGH_PORT	0x0381
#define	LOW_PORT	0x0382
#define	DATA_PORT	0x0383


	pci_io_write8(0xfe, HIGH_PORT);
	pci_io_write8(0x01, LOW_PORT);
	temp = pci_io_read8(DATA_PORT);
	pci_io_write8(0xfe, HIGH_PORT);
	pci_io_write8(0x01, LOW_PORT);
	pci_io_write8(0x00, DATA_PORT);

		fbaddress  =_pci_conf_read(vga_dev->pa.pa_tag,0x10);
		ioaddress  =_pci_conf_read(vga_dev->pa.pa_tag,0x18);

		fbaddress = fbaddress &0xffffff00; //laster 8 bit
		ioaddress = ioaddress &0xfffffff0; //laster 4 bit





		fbaddress |= BONITO_PCILO_BASE_VA;
		ioaddress |= BONITO_PCIIO_BASE_VA;
		


		sm712_init((UINT8 *)fbaddress,(UINT8 *)ioaddress);

		fb_init(fbaddress, ioaddress);

	//
	// Turn on LCD
	//
	pci_io_write8(0xfe, HIGH_PORT);
	pci_io_write8(0x01, LOW_PORT);
	pci_io_write8(temp, DATA_PORT);



			vga_available=1;

	


}

