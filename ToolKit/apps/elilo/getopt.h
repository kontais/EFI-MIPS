/*
 * Simplistic getopt() function header file for EFI
 *
 * Copyright (C) 2000 Hewlett-Packard Co
 * Copyright (C) 2000 Stephane Eranian <eranian@hpl.hp.com>
 *
 * ELILO is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * ELILO is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ELILO; see the file COPYING.  If not, write to the Free
 * Software Foundation, 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */



#ifndef __EFI_GETOPT_H__
#define __EFI_GETOPT_H__

extern CHAR16 *Optarg;
extern INTN Optind, Optopt;

extern INTN Getopt(INTN argc, CHAR16 *const argv[], const CHAR16 *optstring);

#endif /* __EFI_GETOPT_H__ */
