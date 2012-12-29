/*
 *  Copyright (C) 2001-2003 Hewlett-Packard Co.
 *	Contributed by Stephane Eranian <eranian@hpl.hp.com>
 *
 *  Copyright (C) 2001 Silicon Graphics, Inc.
 *      Contributed by Brent Casavant <bcasavan@sgi.com>
 *
 * This file is part of the ELILO, the EFI Linux boot loader.
 *
 *  GNU EFI is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  GNU EFI is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GNU EFI; see the file COPYING.  If not, write to the Free
 *  Software Foundation, 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 * Please check out the elilo.txt for complete documentation on how
 * to use this program.
 */

#ifndef __ELILO_H__
#define __ELILO_H__

#define ELILO_VERSION L"3.14"

#include <efi.h>

#ifdef CONFIG_Mips32
#define PTR_FMT L"0x%x"
#else
#define PTR_FMT L"0x%lx"
#endif

#include "elilo_debug.h"

#include "fileops.h"
#include "chooser.h"

#include "strops.h"

#include "sysdeps.h"

#define MB		(1024*1024) /* 1 megabyte */

/* Round X up/down to A, which must be an integer power of two.  */
#define ROUNDUP(x,a)	(((x) + (a) - 1) & ~((a) - 1))
#define ROUNDDOWN(x,a)  ((x) & ~((a) - 1))

#ifndef UINT32_MAX
#define UINT32_MAX	((UINT32)-1)
#endif

/*
 * Elilo Boot modes
 */
#define ELILO_LOAD_SUCCESS	0
#define ELILO_LOAD_ABORTED	1
#define ELILO_LOAD_ERROR	2
#define ELILO_LOAD_RETRY	3

#define ELILO_DEFAULT_TIMEOUT	ELILO_TIMEOUT_INFINITY
#define ELILO_TIMEOUT_INFINITY	(~0UL)

#define CMDLINE_MAXLEN		2048
#define PATHNAME_MAXLEN		512
#define FILENAME_MAXLEN		256
#define MAX_ARGS		256
/* Just pick an arbitrary number that's high enough for now :o) */
#define MAX_DEFAULT_CONFIGS     16

typedef struct {
	UINT8 nothing_yet;
} image_opt_t;

typedef struct config_file {
	CHAR16 fname[FILENAME_MAXLEN];
} config_file_t;

typedef struct {
	/*
	 * list of options controllable from both the command line
	 * and the config file
	 */
	UINTN alt_check;	/* always check for alternate kernel */
	UINTN debug;		/* debug print() on */
	UINTN delay;		/* delay before booting the image */
	UINTN verbose;		/* verbosity level [1-5] */
	CHAR16 initrd[FILENAME_MAXLEN];		/* name of file for initial ramdisk */
	CHAR16 vmcode[FILENAME_MAXLEN];	/* name of file for boot time module*/
	UINT8 delay_set;	/* mark whether or not delay was specified on cmdline */
	UINT8 edd30_on;		/* true is EDD30 variable is TRUE */
	UINT8 edd30_no_force;	/* don't force EDD30 variable to true */
	/*
	 * list of options controllable from either the command line
	 * or the config file
	 */
	UINTN prompt;		/* enter interactive mode */
	UINTN parse_only;	/* only parses the config file */
	UINTN timeout;		/* timeout before leaving interactive mode*/

	image_opt_t img_opt;	/* architecture independent per image options*/

	sys_img_options_t *sys_img_opts;	/* architecture depdendent per image options */

	CHAR16 default_kernel[FILENAME_MAXLEN];
	/* CHAR16 default_config[FILENAME_MAXLEN]; */
        config_file_t default_configs[MAX_DEFAULT_CONFIGS];

	CHAR16 config[FILENAME_MAXLEN];		/* name of config file */
	CHAR16 chooser[FILENAME_MAXLEN];	/* image chooser to use */
} elilo_config_t;


extern elilo_config_t elilo_opt;

extern EFI_SYSTEM_TABLE *systab;

typedef struct {
	VOID 	*start_addr;
	UINTN	pgcnt;
	UINTN	size;
} memdesc_t;

typedef struct {
	VOID *kstart;
	VOID *kend;
	VOID *kentry;
} kdesc_t;

typedef struct {
	EFI_MEMORY_DESCRIPTOR	*md;
	UINTN			map_size;
	UINTN			desc_size;
	UINTN			cookie;
	UINT32			desc_version;
} mmap_desc_t;

#ifndef MAX
#define MAX(a,b)	((a)>(b) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a,b)	((a)>(b) ? (b) : (a))
#endif

#define VERB_PRT(n,cmd) \
	{ if (elilo_opt.verbose >= (n)) { cmd; } }


/* from alloc.c */
extern INTN alloc_init(VOID);
extern VOID *alloc(UINTN, EFI_MEMORY_TYPE);
extern VOID free(VOID *);
extern VOID *alloc_pages(UINTN, EFI_MEMORY_TYPE, EFI_ALLOCATE_TYPE, VOID *);
extern VOID free_pages(VOID *);
extern VOID free_all(VOID);
extern INTN alloc_kmem(VOID *, UINTN);
extern INTN alloc_kmem_anywhere(VOID **, UINTN);
extern VOID free_kmem(VOID);
extern VOID free_all_memory(VOID);

/* from util.c */
extern INTN read_file(UINTN fd, UINTN, CHAR8 *);
extern EFI_STATUS check_abort(VOID);
extern VOID reset_input(VOID);
extern INTN wait_timeout(UINTN);
extern INTN argify(CHAR16 *, UINTN, CHAR16 **);
extern VOID unargify(CHAR16 **, CHAR16 **);
extern void split_args(CHAR16 *, CHAR16 *, CHAR16 *);
extern INTN get_memmap(mmap_desc_t *);
extern VOID free_memmap(mmap_desc_t *);
extern INTN find_kernel_memory(VOID *low_addr, VOID *max_addr, UINTN alignment, VOID ** start);
extern VOID print_memmap(mmap_desc_t *);
extern VOID ascii2U(CHAR8 *, CHAR16 *, UINTN);
extern VOID U2ascii(CHAR16 *, CHAR8 *, UINTN);

/* from config.c (more in config.h) */
extern EFI_STATUS read_config(CHAR16 *);
extern VOID print_config_options(VOID);
extern INTN find_label(CHAR16 *, CHAR16 *, CHAR16 *, CHAR16 *, CHAR16 *);
extern VOID print_label_list(VOID);
extern INTN config_init(VOID);
extern CHAR16 *get_message_filename(INTN which);
extern CHAR16 *find_description(CHAR16 *label);
extern VOID *get_next_description(VOID *prev, CHAR16 **label, CHAR16 **description);
extern CHAR16 *get_config_file(VOID);

/* from initrd.c */
extern INTN load_file(CHAR16 *, memdesc_t *);

/* from alternate.c */
extern INTN alternate_kernel(CHAR16 *, UINTN);

/* from bootparams.c */
extern VOID *create_boot_params (CHAR16 *, memdesc_t *, memdesc_t *, UINTN *);
extern VOID free_boot_params(VOID *bp);

/*
 * architecture-specific API
 */


extern INTN sysdeps_create_boot_params(boot_params_t *, CHAR8 *, memdesc_t *, memdesc_t *, UINTN *);
extern VOID sysdeps_free_boot_params(boot_params_t *);
extern INTN sysdeps_init(EFI_HANDLE dev);
extern INTN sysdeps_initrd_get_addr(kdesc_t *, memdesc_t *);
extern INTN sysdeps_preloop_actions(EFI_HANDLE, CHAR16 **, INTN, INTN, EFI_HANDLE);
extern CHAR16 *sysdeps_get_cmdline_opts(VOID);
extern INTN sysdeps_getopt(INTN, INTN, CHAR16 *);
extern VOID sysdeps_print_cmdline_opts(VOID);
extern INTN sysdeps_register_options(VOID);
extern VOID *sysdeps_checkfix_initrd(VOID *, memdesc_t *);

#define	CHAR_SLASH	L'/'
#define CHAR_BACKSLASH	L'\\'

#endif /* __ELILO_H__ */
