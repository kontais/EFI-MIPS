/*
 *  Copyright (C) 2001-2003 Hewlett-Packard Co.
 *	Contributed by Stephane Eranian <eranian@hpl.hp.com>
 *	Contributed by Mike Johnston <johnston@intel.com>
 *	Contributed by Chris Ahna <christopher.j.ahna@intel.com>
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

/*
 * This file is used to define all the IA32-specific data structures
 * and constant used by the generic ELILO
 */
#ifndef __ELILO_SYSDEPS_IA32_H__
#define __ELILO_SYSDEPS_IA32_H__

#define ELILO_ARCH	"IA-32" /* ASCII string */
#define PADDR_MASK	0xfffffff

#define INITRD_START	(15*1024*1024)
#define DEFAULT_KERNEL_START	0x100000

/* for now use library versions */
#define Memset(a,v,n)	SetMem((a),(n),(v))
#define Memcpy(a,b,n)	CopyMem((a),(b),(n))

/*
 * This version must match the one in the kernel.
 *
 * This table was put together using information from the
 * following Linux kernel source files:
 *   linux/include/tty.h
 *   linux/arch/i386/kernel/setup.c
 *   linux/arch/i386/boot/bootsect.S
 *   linux/arch/i386/boot/setup.S
 *   linux/arch/i386/boot/video.S
 *
 * New fields in this structure for EFI and ELILO are:
 *   efi_loader_sig
 *   efi_st_addr
 *
 * A new bit, LDRFLAG_BOOT_PARAM_RELOC, in the loader_flags
 * field is also defined in this file.
 */

#pragma pack(1)

/* Definitions for converting EFI memory map to E820 map for Linux
 * These definitions are from include/linux/asm-x86/e820.h
 * The structure ia32_boot_params below is updated to accommodate E820 map
 * EFI memory map is converted to E820 map in this structure and passed
 * to Linux. This way the OS does not need to do the conversion.
 */

#define E820_RAM        1
#define E820_RESERVED   2
#define E820_ACPI       3
#define E820_NVS        4
#define E820_MAX	128

struct e820entry {
	UINT64 addr;	/* start of memory segment */
	UINT64 size;	/* size of memory segment */
	UINT32 type;	/* type of memory segment */
} __attribute__((packed));

typedef union ia32_boot_params {
	UINT8 raw[0x2000];
	struct {
/* Cursor position before passing control to kernel. */
/* 0x00 */	UINT8 orig_cursor_col;		/* LDR */
/* 0x01 */	UINT8 orig_cursor_row;		/* LDR */

/* Available contiguous extended memory in KB. */
/* 0x02 */	UINT16 ext_mem_k;		/* LDR */

/* Video page, mode and screen width before passing control to kernel. */
/* 0x04 */	UINT16 orig_video_page;		/* LDR */
/* 0x06 */	UINT8 orig_video_mode;		/* LDR */
/* 0x07 */	UINT8 orig_video_cols;		/* LDR */

/* 0x08 */	UINT16 unused_1;		/* unused */

/* %%TBD */
/* 0x0A */	UINT16 orig_ega_bx;		/* LDR */

/* 0x0C */	UINT16 unused_2;		/* unused */

/* Screen height before passing control to kernel. */
/* 0x0E */	UINT8 orig_video_rows;		/* LDR */

/* %%TBD */
/* 0x0F */	UINT8 is_vga;			/* LDR */
/* 0x10 */	UINT16 orig_video_points;	/* LDR */

/* %%TBD */
/* 0x12 */	UINT16 lfb_width;		/* LDR */
/* 0x14 */	UINT16 lfb_height;		/* LDR */
/* 0x16 */	UINT16 lfb_depth;		/* LDR */
/* 0x18 */	UINT32 lfb_base;		/* LDR */
/* 0x1C */	UINT32 lfb_size;		/* LDR */

/* Offset of command line (from start of ia32_boot_param struct). */
/* The command line magik number must be set for the kernel setup */
/* code to use the command line offset. */
/* 0x20 */	UINT16 cmdline_magik;		/* LDR */
#define CMDLINE_MAGIK		0xA33F
/* 0x22 */	UINT16 cmdline_offset;		/* LDR */

/* %%TBD */
/* 0x24 */	UINT16 lfb_line_len;		/* LDR */

/* %%TBD */
/* 0x26 */	UINT8 lfb_red_size;		/* LDR */
/* 0x27 */	UINT8 lfb_red_pos;		/* LDR */
/* 0x28 */	UINT8 lfb_green_size;		/* LDR */
/* 0x29 */	UINT8 lfb_green_pos;		/* LDR */
/* 0x2A */	UINT8 lfb_blue_size;		/* LDR */
/* 0x2B */	UINT8 lfb_blue_pos;		/* LDR */
/* 0x2C */	UINT8 lfb_rsvd_size;		/* LDR */
/* 0x2D */	UINT8 lfb_rsvd_pos;		/* LDR */

/* %%TBD */
/* 0x2E */	UINT16 vesa_seg;		/* LDR */
/* 0x30 */	UINT16 vesa_off;		/* LDR */

/* %%TBD */
/* 0x32 */	UINT16 lfb_pages;		/* LDR */
/* 0x34 */	UINT8 lfb_reserved[0x0C];	/* reserved */

/* %%TBD */
/* 0x40 */	UINT16 apm_bios_ver;		/* LDR */
#define NO_APM_BIOS		0x0000

/* %%TBD */
/* 0x42 */	UINT16 bios_code_seg;		/* LDR */
/* 0x44 */	UINT32 bios_entry_point;	/* LDR */
/* 0x48 */	UINT16 bios_code_seg16;		/* LDR */
/* 0x4A */	UINT16 bios_data_seg;		/* LDR */

/* %%TBD */
/* 0x4C */	UINT16 apm_bios_flags;		/* LDR */
#define NO_32BIT_APM_MASK	0xFFFD

/* %%TBD */
/* 0x4E */	UINT32 bios_code_len;		/* LDR */
/* 0x52 */	UINT16 bios_data_len;		/* LDR */

/* 0x54 */	UINT8 unused_3[0x2C];		/* unused */

/* %%TBD */
/* 0x80 */	UINT8 hd0_info[0x10];		/* LDR */
/* 0x90 */	UINT8 hd1_info[0x10];		/* LDR */

/* %%TBD */
/* 0xA0 */	UINT16 mca_info_len;		/* LDR */
/* 0xA2 */	UINT8 mca_info_buf[0x10];	/* LDR */

/* 0xB2 */	UINT8 unused_4[0x10E];		/* unused */

/* EFI boot loader signature. */
/* 0x1C0 */	UINT8 efi_loader_sig[4];	/* LDR */
#define EFI_LOADER_SIG_IA32		"EL32"

/* Address of the EFI system table. */
/* 0x1C4 */	UINT32 efi_sys_tbl;		/* LDR */

/* EFI memory descriptor size. */
/* 0x1C8 */	UINT32 efi_mem_desc_size;	/* LDR */

/* EFI memory descriptor version. */
/* 0x1CC */	UINT32 efi_mem_desc_ver;	/* LDR */

/* Address & size of EFI memory map. */
/* 0x1D0 */	UINT32 efi_mem_map;		/* LDR */
/* 0x1D4 */	UINT32 efi_mem_map_size;	/* LDR */

/* Address & size of loader. */
/* 0x1D8 */	UINT32 loader_start;		/* LDR */
/* 0x1DC */	UINT32 loader_size;		/* LDR */

/* Available contiguous extended memory in KB. */
/* 0x1E0 */	UINT32 alt_mem_k;		/* LDR */

/* 0x1E4 */	UINT32 unused_51;		/* unused */
/* 0x1E8 */	UINT8  e820_nrmap;
/* 0x1E9 */	UINT32 unused_52[2];		/* unused */

/* Size of setup code in sectors (1 sector == 512 bytes). */
/* 0x1F1 */	UINT8 setup_sectors;		/* BLD */

/* %%TBD */
/* 0x1F2 */	UINT16 mount_root_rdonly;	/* BLD */

/* %%TBD */
/* 0x1F4 */	UINT16 sys_size;		/* BLD */

/* %%TBD */
/* 0x1F6 */	UINT16 swap_dev;		/* BLD */

/* %%TBD */
/* 0x1F8 */	UINT16 ramdisk_flags;		/* BLD */
#define RAMDISK_PROMPT		0x8000
#define RAMDISK_LOAD		0x4000

/* %%TBD */
/* 0x1FA */	UINT16 video_mode_flag;		/* BLD */

/* %%TBD */
/* 0x1FC */	UINT16 orig_root_dev;		/* BLD */

/* 0x1FE */	UINT8 unused_6;			/* unused */

/* %%TBD */
/* 0x1FF */	UINT8 aux_dev_info;		/* LDR */
#define NO_MOUSE		0x00
#define FOUND_MOUSE		0xAA

/* Jump past setup data (not used in EFI). */
/* 0x200 */	UINT16 jump;			/* BLD */

/* Setup data signature. */
/* 0x202 */	UINT8 setup_sig[4];		/* BLD */
#define SETUP_SIG		"HdrS"

/* %%TBD */
/* 0x206 */	UINT8 hdr_minor;		/* BLD */
/* 0x207 */	UINT8 hdr_major;		/* BLD */

/* %%TBD */
/* 0x208 */	UINT32 rm_switch;		/* LDD */

/* %%TBD */
/* 0x20C */	UINT16 start_sys_seg;		/* BLD */

/* %%TBD */
/* 0x20E */	UINT16 kernel_verstr_offset;	/* BLD */

/* Loader type & version. */
/* 0x210 */	UINT8 loader_type;		/* LDR */
#define LDRTYPE_ELILO			0x50	/* 5?h == elilo */
						/* ?0h == revision */

/* 0x211 */	UINT8 loader_flags;		/* BLD and LDR */
#define LDRFLAG_CAN_USE_HEAP		0x80
#define LDRFLAG_BOOT_PARAM_RELOC	0x40

/* %%TBD */
/* 0x212 */	UINT16 setup_move_size;		/* BLD */

/* %%TBD */
/* 0x214 */	UINT32 kernel_start;		/* LDR */

/* %%TBD */
/* 0x218 */	UINT32 initrd_start;		/* LDR */
/* 0x21C */	UINT32 initrd_size;		/* LDR */

/* %%TBD */
/* 0x220 */	UINT32 bootsect_helper;		/* BLD */

/* %%TBD */
/* 0x224 */	UINT16 heap_end_ptr;		/* LDR */

/* %%TBD */
/* 0x226 */	UINT16 unused_7;		/* LDR */

/* 0x228 */	UINT32 cmdline_addr; 		/* LDR */
/* 0x22C */	UINT32 unused_8[41];
/* 0x2D0 */	UINT8  e820_map[2560];
	} s;
} boot_params_t;
#pragma pack()

/*
 * The stuff below here is for jumping to the kernel.
 */

/*
 * Some macros to copy and set memory after EFI has been
 * stopped.
 */

#define MEMCPY(to, from, cnt) { \
	UINT8 *t = (UINT8 *)(to); \
	UINT8 *f = (UINT8 *)(from); \
	UINTN n = cnt; \
	if (t && f && n && (t<f)) { \
        	while (n--) { \
                        *t++ = *f++; \
                } \
        } else if (t && f && n && (t>f)) { \
                t += n; \
                f += n; \
                while (n--) { \
                        *t-- = *f--; \
                } \
        } \
}

#define MEMSET(ptr, size, val) { \
	UINT8 *p = (UINT8 *)(ptr); \
	UINTN n = (UINTN)(size); \
	UINT8 v = (UINT8)(val); \
	if (p && n) { \
		while (n--) { \
			*p++ = v; \
		} \
	} \
}

/*
 * Descriptor table pointer format.
 */
#pragma pack(1)
typedef struct {
	UINT16 limit;
	UINT32 base;
} dt_addr_t;
#pragma pack()

extern UINTN high_base_mem;
extern UINTN high_ext_mem;

extern boot_params_t *param_start;
extern UINTN param_size;

extern VOID *kernel_start;
extern UINTN kernel_size;

extern VOID *initrd_start;
extern UINTN initrd_size;
extern VOID *kernel_load_address;

extern dt_addr_t gdt_addr;
extern dt_addr_t idt_addr;

extern UINT16 init_gdt[];
extern UINTN sizeof_init_gdt;

extern UINT8 rmswitch_image[];
extern UINTN rmswitch_size;

extern INTN ia32_use_legacy_free_boot();

/*
 * How to jump to kernel code
 */

static inline void
start_kernel(VOID *kentry, boot_params_t *bp)
{
	UINT32	temp;

	/*
	 * Disable interrupts.
	 */
	//asm volatile ( "cli" : : );

        /*
         * Relocate kernel (if needed), and initrd (if present).
         * Copy kernel first, in case kernel was loaded overlapping where we're
         * planning to copy the initrd.  This assumes that the initrd didn't
         * get loaded overlapping where we're planning to copy the kernel, but
         * that's pretty unlikely since we couldn't alloc that space for the
         * kernel (or the kernel would already be there).
         */
        if (kernel_start != kernel_load_address) {
                MEMCPY(kernel_start, kernel_load_address, kernel_size);
        }

        if (bp->s.initrd_start) {
                temp =  bp->s.initrd_start;
                MEMCPY(INITRD_START, temp , bp->s.initrd_size);
                bp->s.initrd_start = INITRD_START;
        }

	/*
	 * Copy boot sector, setup data and command line
	 * to final resting place.  We need to copy
	 * BOOT_PARAM_MEMSIZE bytes.
	 */

	MEMCPY(high_base_mem, bp, 0x4000);

	bp = (boot_params_t *)high_base_mem;
	bp->s.cmdline_addr = high_base_mem + bp->s.cmdline_offset;

#if 0
	/*
	 * Initialize Linux GDT.
	 */

	MEMSET(gdt_addr.base, gdt_addr.limit, 0);
	MEMCPY(gdt_addr.base, init_gdt, sizeof_init_gdt);

	if (! ia32_use_legacy_free_boot()) {

		/*
		 * Copy our real mode transition code to 0x7C00.
		 */

		MEMCPY(0x7C00, rmswitch_image, rmswitch_size);

		asm volatile ( "movl $0x7C00, %%ebx" : : );
		asm volatile ( "jmp *%%ebx" : : );
	}

	/*
	 * Load descriptor table pointers.
	 */

	asm volatile ( "lidt %0" : : "m" (idt_addr) );
	asm volatile ( "lgdt %0" : : "m" (gdt_addr) );

	/*
	 * ebx := 0  (%%TBD - do not know why, yet)
	 * ecx := kernel entry point
 	 * esi := address of boot sector and setup data
	 */

	asm volatile ( "movl %0, %%esi" : : "m" (high_base_mem) );
	asm volatile ( "movl %0, %%ecx" : : "m" (kentry) );
	asm volatile ( "xorl %%ebx, %%ebx" : : );

	/*
	 * Jump to kernel entry point.
	 */

	asm volatile ( "jmp *%%ecx" : : );
#endif
}

typedef struct sys_img_options {
	UINT8 nothing_yet;
} sys_img_options_t;

#endif /* __ELILO_SYSDEPS_IA32_H__ */
