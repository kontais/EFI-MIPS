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
#include "bonitopmon.h"
#include "video_font.h"

#define VIDEO_FB_ADRS       (0xb0000000)

#define VIDEO_COLS      (640)
#define VIDEO_ROWS      (400)
#define VIDEO_PIXEL_SIZE    (1)
#define VIDEO_SIZE      (VIDEO_ROWS*VIDEO_COLS*VIDEO_PIXEL_SIZE)


#define VIDEO_LINE_LEN      (VIDEO_COLS*VIDEO_PIXEL_SIZE)






#define CONSOLE_ROWS        (VIDEO_ROWS / VIDEO_FONT_HEIGHT)
#define CONSOLE_COLS        (VIDEO_COLS / VIDEO_FONT_WIDTH)
#define CONSOLE_ROW_SIZE    (VIDEO_FONT_HEIGHT * VIDEO_LINE_LEN)
#define CONSOLE_ROW_FIRST   (VIDEO_FB_ADRS)
#define CONSOLE_ROW_SECOND  (VIDEO_FB_ADRS + CONSOLE_ROW_SIZE)
#define CONSOLE_SIZE        (CONSOLE_ROW_SIZE * CONSOLE_ROWS)
#define CONSOLE_ROW_LAST    (VIDEO_FB_ADRS + CONSOLE_SIZE - CONSOLE_ROW_SIZE)
#define CONSOLE_SCROLL_SIZE (CONSOLE_SIZE - CONSOLE_ROW_SIZE)

extern void _pci_tagprintf (pcitag_t tag, const char *fmt, ...);



int vga_available = 0;

static int console_col = 0;
static int console_row = 0;

unsigned int eorx, fgx, bgx;


static const int video_font_draw_table8[] = {
    0x00000000, 0xff000000, 0x00ff0000, 0xffff0000,
    0x0000ff00, 0xff00ff00, 0x00ffff00, 0xffffff00,
    0x000000ff, 0xff0000ff, 0x00ff00ff, 0xffff00ff,
    0x0000ffff, 0xff00ffff, 0x00ffffff, 0xffffffff
};



/******************************************************************************/
void video_drawchars(int xx, int yy, unsigned char *s, int count)
{
    unsigned char *cdat, *dest, *dest0;
    int rows, offset, c;

    offset = yy * VIDEO_LINE_LEN + xx * VIDEO_PIXEL_SIZE;
    dest0 = VIDEO_FB_ADRS + offset;


        while (count--) {
            c = *s;
            cdat = video_fontdata + c * VIDEO_FONT_HEIGHT;
            for (rows = VIDEO_FONT_HEIGHT, dest = dest0;
                 rows--; dest += VIDEO_LINE_LEN) {
                unsigned char bits = *cdat++;

                ((unsigned int *)dest)[0] =
                    (video_font_draw_table8[bits >> 4] & eorx) ^
                    bgx;
                ((unsigned int *)dest)[1] =
                    (video_font_draw_table8[bits & 15] & eorx) ^
                    bgx;
            }
            dest0 += VIDEO_FONT_WIDTH * VIDEO_PIXEL_SIZE;
            s++;
        }

}



void video_putchar(int xx, int yy, unsigned char c)
{
    video_drawchars(xx, yy, &c, 1);
}



void memsetl(UINT8 *p, int c, int v)
{
    while (c--)
        *(p++) = v;
}


void memcpyl(int *d, int *s, int c)
{
    while (c--)
        *(d++) = *(s++);
}


static void console_scrollup(void)
{
    memcpyl (CONSOLE_ROW_FIRST, CONSOLE_ROW_SECOND,
         CONSOLE_SCROLL_SIZE >> 2);

    /* clear the last one */
    memsetl(CONSOLE_ROW_LAST, CONSOLE_ROW_SIZE, CONSOLE_BG_COL);
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

        break;

    case 8:     /* backspace */

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


void video_puts(const char *s)
{
	if (vga_available == 0) 
		return;

    int count = strlen(s);

    while (count--)
        video_putc(*s++);
}

void _set_font_color(void)
{
    fgx = 0xf4f4f4f4;
    bgx = 0x00000000;
    eorx = fgx ^ bgx;
}


int fb_init(unsigned long fbbase, unsigned long iobase)
{


    _set_font_color();


    memsetl((void *)VIDEO_FB_ADRS, CONSOLE_SIZE,  CONSOLE_BG_COL);

    /* Initialize the console */
    console_col = 0;
    console_row = 0;

			vga_available=1;


    return 0;
}

extern struct pci_device *vga_dev;
	
void vga_init(void)
{

	unsigned long fbaddress,ioaddress;

	unsigned int video_mem_size;
	unsigned int tmp;
int i;
int x,y;


	/* We assume all the framebuffer is required remmapping */

	/* 0x10000000 -> 0x40000000 PCI mapping */
	/* CPU 0x1000000 to uncache is 0xb000000 */
	_pci_conf_write(vga_dev->pa.pa_tag, 0x10, 0x40000000);
	tmp = BONITO_PCIMAP;
	BONITO_PCIMAP = 
	    BONITO_PCIMAP_WIN(0, 0x40000000) |	
		(tmp & ~BONITO_PCIMAP_PCIMAP_LO0);


	fbaddress  =_pci_conf_read(vga_dev->pa.pa_tag,0x10);
	ioaddress  =_pci_conf_read(vga_dev->pa.pa_tag,0x18);

	fbaddress = fbaddress &0xffffff00; //laster 8 bit
	ioaddress = ioaddress &0xfffffff0; //laster 4 bit


	fb_init(fbaddress, ioaddress);
	vga_available = 1;
	printf("after fb_init\n");
	printf("fbaddress 0x%x\n", fbaddress);
	printf("ioaddress 0x%x\n", ioaddress);




	printf("a BEV in SR set to zero.\n");
	printf("a BEV in SR set to zero.\n");
	
	video_puts("sssssssssssss\n");
#if 0

for (y=0;y < 400; y++)
{
  for (x=0; x < 640; x++)
  {
    *(unsigned char*) (0xb0000000 + x+ (y*640)) = (x/40) + (y/25)*16;
    //*(unsigned char*) (0xb0000000 + x+ (y*640)) = 0xf4;
  }
}

#endif
			vga_available=1;

	


}


