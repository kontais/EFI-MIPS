/***************************************************************************
 * Name:
 *     smi712.h
 * License:
 *     2003-2007, Copyright by BLX IC Design Co., Ltd.
 * Description:
 *     Smi 712 VGA card bios driver for BLX 1A
 *     The code from linux framebuffer drivers.
 *
 ***************************************************************************/

#ifndef  __SMI_712_
#define  __SMI_712_

#define	SM712_2D_DEBUG		1


//
// Mode Setting
//
#define	SM712_FIX_WIDTH		1024
#define	SM712_FIX_HEIGHT	600
#define	SM712_FIX_DEPTH		16
#define	SM712_FIX_HZ		60



#define smi_mmiowb(dat,reg)     pci_mem_write8((UINT8)(dat),(UINTN)(SMIRegs + (reg)))
#define smi_mmioww(dat,reg)     pci_mem_write16((UINT16)(dat),(UINTN)(SMIRegs + (reg)))
#define smi_mmiowl(dat,reg)     pci_mem_write32((UINT32)(dat),(UINTN)(SMIRegs + (reg)))

#define smi_mmiorb(reg)         pci_mem_read8((UINTN)(SMIRegs + (reg)))
#define smi_mmiorw(reg)         pci_mem_read16((UINTN)(SMIRegs + (reg)))
#define smi_mmiorl(reg)         pci_mem_read32((UINTN)(SMIRegs + (reg)))

#define SIZE_SR00_SR04          (0x04 - 0x00 + 1)
#define SIZE_SR10_SR24          (0x24 - 0x10 + 1)
#define SIZE_SR30_SR75          (0x75 - 0x30 + 1)
#define SIZE_SR80_SR93          (0x93 - 0x80 + 1)
#define SIZE_SRA0_SRAF          (0xAF - 0xA0 + 1)
#define SIZE_GR00_GR08          (0x08 - 0x00 + 1)
#define SIZE_AR00_AR14          (0x14 - 0x00 + 1)
#define SIZE_CR00_CR18          (0x18 - 0x00 + 1)
#define SIZE_CR30_CR4D          (0x4D - 0x30 + 1)
#define SIZE_CR90_CRA7          (0xA7 - 0x90 + 1)
#define SIZE_VPR                (0x6C + 1)
#define SIZE_DPR                (0x44 + 1)

#define numVGAModes             7



#ifdef	SM712_2D_DEBUG

#define	REG_2D_SRC_XY			0x00
#define	REG_2D_DST_XY			0x04
#define	REG_2D_DIM_XY			0x08
#define	REG_2D_DE_CTRL			0x0C
#define	REG_2D_PITCH_XY			0x10
#define	REG_2D_COLOR_FG			0x14
#define	REG_2D_COLOR_BG			0x18
#define	REG_2D_DEDF				0x1C
#define	REG_2D_COLOR_COM		0x20
#define	REG_2D_COLOR_COM_MASK	0x24
#define	REG_2D_BB_MASK			0x28
#define	REG_2D_CROP_LEFTTOP		0x2C
#define	REG_2D_CROP_RIGHTBOTTOM	0x30
#define	REG_2D_MONO_PATTERN_LOW	0x34
#define	REG_2D_MONO_PATTERN_HIGH	0x38
#define	REG_2D_SRC_WINDOW		0x3C
#define	REG_2D_SRC_BASE			0x40
#define	REG_2D_DST_BASE			0x44



#endif

struct par_info {
	/* Hardware */
	UINT16             chipID;
	UINT8            *m_pLFB;
	UINT8            *m_pMMIO;
	UINT8            *m_pDPR;
	UINT8            *m_pVPR;

	UINT32             width;
	UINT32             height;
	UINT32             hz;
	/* glame add */
	UINT32             bits_per_pixel;
};

/* The next structure holds all information relevant for a specific video mode. */
struct ModeInit
{
	int             mmSizeX;
	int             mmSizeY;
	int             bpp;
	int             hz;
	UINT8   Init_MISC;
	UINT8   Init_SR00_SR04[SIZE_SR00_SR04];
	UINT8   Init_SR10_SR24[SIZE_SR10_SR24];
	UINT8   Init_SR30_SR75[SIZE_SR30_SR75];
	UINT8   Init_SR80_SR93[SIZE_SR80_SR93];
	UINT8   Init_SRA0_SRAF[SIZE_SRA0_SRAF];
	UINT8   Init_GR00_GR08[SIZE_GR00_GR08];
	UINT8   Init_AR00_AR14[SIZE_AR00_AR14];
	UINT8   Init_CR00_CR18[SIZE_CR00_CR18];
	UINT8   Init_CR30_CR4D[SIZE_CR30_CR4D];
	UINT8   Init_CR90_CRA7[SIZE_CR90_CRA7];
};


void video_hw_bitblt(UINTN bpp,	/* bytes per pixel */
		     UINTN src_x,	/* source pos x */
		     UINTN src_y,	/* source pos y */
		     UINTN dst_x,	/* dest pos x */
		     UINTN dst_y,	/* dest pos y */
		     UINTN dim_x,	/* frame width */
		     UINTN dim_y	/* frame height */
    );

void video_hw_rectfill(UINTN bpp,	/* bytes per pixel */
		       UINTN dst_x,	/* dest pos x */
		       UINTN dst_y,	/* dest pos y */
		       UINTN dim_x,	/* frame width */
		       UINTN dim_y,	/* frame height */
		       UINTN color	/* fill color */
    );


int GetXRes(void);
int GetYRes(void);
int GetBytesPP(void);

int  sm712_init(UINT8 * fbaddress,UINT8 * ioaddress);


#endif /*__SMI_712__*/
