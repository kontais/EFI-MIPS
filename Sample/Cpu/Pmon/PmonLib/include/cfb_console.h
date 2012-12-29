
#define VIDEO_HW_BITBLT             // video hw bitblt 2d acceleration config
#define VIDEO_HW_RECTFILL           // video hw fillrect 2d acceleration config



/*****************************************************************************/
/* some Macros                                   */
/*****************************************************************************/
#define VIDEO_VISIBLE_COLS  (pGD->winSizeX)
#define VIDEO_VISIBLE_ROWS  (pGD->winSizeY)
#define VIDEO_PIXEL_SIZE    (pGD->gdfBytesPP)
#define VIDEO_DATA_FORMAT   (pGD->gdfIndex)
#define VIDEO_FB_ADRS       (pGD->frameAdrs)

#define VIDEO_COLS      VIDEO_VISIBLE_COLS
#define VIDEO_ROWS      VIDEO_VISIBLE_ROWS
#define VIDEO_SIZE      (VIDEO_ROWS*VIDEO_COLS*VIDEO_PIXEL_SIZE)
#define VIDEO_PIX_BLOCKS    (VIDEO_SIZE >> 2)
#define VIDEO_LINE_LEN      (VIDEO_COLS*VIDEO_PIXEL_SIZE)
#define VIDEO_BURST_LEN     (VIDEO_COLS/8)




#define VIDEO_LOGO_WIDTH    0
#define VIDEO_LOGO_HEIGHT   0


#define CONSOLE_ROWS        (VIDEO_ROWS / VIDEO_FONT_HEIGHT)


#define CONSOLE_COLS        (VIDEO_COLS / VIDEO_FONT_WIDTH)
#define CONSOLE_ROW_SIZE    (VIDEO_FONT_HEIGHT * VIDEO_LINE_LEN)
#define CONSOLE_ROW_FIRST   (video_console_address)
#define CONSOLE_ROW_SECOND  (video_console_address + CONSOLE_ROW_SIZE)
#define CONSOLE_SIZE        (CONSOLE_ROW_SIZE * CONSOLE_ROWS)
#define CONSOLE_ROW_LAST    (video_console_address + CONSOLE_SIZE - CONSOLE_ROW_SIZE)
#define CONSOLE_SCROLL_SIZE (CONSOLE_SIZE - CONSOLE_ROW_SIZE)



#define FB_SIZE ((pGD->winSizeX)*(pGD->winSizeY)*(pGD->gdfBytesPP))

/* Macros */
#ifdef  VIDEO_FB_LITTLE_ENDIAN
#define SWAP16(x)    ((((x) & 0x00ff) << 8) | ( (x) >> 8))
#define SWAP32(x)    ((((x) & 0x000000ff) << 24) | (((x) & 0x0000ff00) << 8)|\
              (((x) & 0x00ff0000) >>  8) | (((x) & 0xff000000) >> 24) )
#define SHORTSWAP32(x)   ((((x) & 0x000000ff) <<  8) | (((x) & 0x0000ff00) >> 8)|\
              (((x) & 0x00ff0000) <<  8) | (((x) & 0xff000000) >> 8) )
#else
#define SWAP16(x)    (x)
#define SWAP32(x)    (x)
#define SHORTSWAP32(x)   (x)
#endif





void video_drawchars(int xx, int yy, UINT8 *s, int count);
void video_putchar(int xx, int yy, UINT8 c);
void memsetl(UINT8 *p, int c, int v);
void memcpyl(int *d, int *s, int c);
void video_putc(const char c);
void video_puts(const char *s);
int GetGDFIndex(int BytesPP);
int fb_init(UINTN fbbase, UINTN iobase);	
void vga_init(void);
