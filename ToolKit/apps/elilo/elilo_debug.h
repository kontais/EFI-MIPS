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

#ifndef __ELILO_DEBUG__
#define __ELILO_DEBUG__

//#define DEBUG_MEM
//#define DEBUG_GZIP
//#define DEBUG_BZ

#define ELILO_DEBUG 1

#define ERR_PRT(a)	do { Print(L"%a(line %d):", __FILE__, __LINE__); Print a; Print(L"\n"); } while (0);

#ifdef ELILO_DEBUG
#define DBG_PRT(a)	do { \
	if (elilo_opt.debug) { \
		Print(L"%a(line %d):", __FILE__, __LINE__); \
		Print a; \
		Print(L"\n"); \
} } while (0);
#else
#define DBG_PRT(a)
#endif

#endif /* __ELILO_DEBUG_H__ */
