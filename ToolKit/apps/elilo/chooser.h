/*
 *  Copyright (C) 2001-2003 Hewlett-Packard Co.
 *	Contributed by Stephane Eranian <eranian@hpl.hp.com>
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

#ifndef __ELILO_CHOOSER_H__
#define __ELILO_CHOOSER_H__

typedef INTN chooser_func_t(CHAR16 **argv, INTN arc, INTN index, CHAR16 *kname, CHAR16 *cmdline);

/*
 * structure to interface to generic chooser selection code.
 *
 * XXX: must add support for moer than one choice (rating scheme)
 *
 * Interface for probe:
 * 	dev: boot device handle
 * 	return: -1, if not support, 0 if supported
 *
 * Interface for chooser:
 * 	argv,argc: command line argument passed to elilo
 * 	index    : position of first non-option argument (min value=1)
 *	kname    : selected kernel name
 *	cmdline  : requested kernel command line
 * 	return:
 * 		-1: if error
 * 		 0: if successful
 */
typedef struct {
	CHAR16 		*chooser_name;
	INTN		(*chooser_probe)(EFI_HANDLE dev);
	chooser_func_t	*chooser_func;
} chooser_t;

extern INTN init_chooser(EFI_HANDLE);
extern INTN exist_chooser(CHAR16 *name);

extern chooser_func_t *kernel_chooser;

#endif /* __ELILO_CHOOSER_H__ */
