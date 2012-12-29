/*	$Id: pcivar.h,v 1.1.1.1 2006/06/29 06:43:25 cpu Exp $	*/
/*	$OpenBSD: pcivar.h,v 1.16 1999/07/18 03:20:18 csapuntz Exp $	*/
/*	$NetBSD: pcivar.h,v 1.23 1997/06/06 23:48:05 thorpej Exp $	*/

/*
 * Copyright (c) 1996 Christopher G. Demetriou.  All rights reserved.
 * Copyright (c) 1994 Charles Hannum.  All rights reserved.
 * Copyright (c) 2001 Patrik Lindergren. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by Charles Hannum.
 *      This product includes software developed by Patrik Lindergren.
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _DEV_PCI_PCIVAR_H_
#define	_DEV_PCI_PCIVAR_H_

/*
 * Definitions for PCI autoconfiguration.
 *
 * This file describes types and functions which are used for PCI
 * configuration.  Some of this information is machine-specific, and is
 * provided by pci_machdep.h.
 */

#include <bus.h>
#include <pcireg.h>

/*
 * Structures and definitions needed by the machine-dependent header.
 */
typedef UINT32 pcireg_t;		/* configuration space register XXX */
/*
 * Types provided to machine-independent PCI code
 */
typedef struct arch_pci_chipset *pci_chipset_tag_t;
typedef UINTN pcitag_t;
typedef UINTN pci_intr_handle_t;

#define	PCI_CACHE_LINE_SIZE 8		/* expressed in 32 bit words */


/*
 *  Any physical to virtual conversion CPU
 */
#define VA_TO_PA(x)     UNCACHED_TO_PHYS(x)
#define PA_TO_VA(x)     PHYS_TO_CACHED(x)


/*
 * PCI device attach arguments.
 */
struct pci_attach_args {
	bus_space_tag_t pa_iot;		/* pci i/o space tag */
	bus_space_tag_t pa_memt;	/* pci mem space tag */
	bus_dma_tag_t pa_dmat;		/* DMA tag */
	pci_chipset_tag_t pa_pc;
	int		pa_flags;	/* flags; see below */
	int		pa_bus;

	UINTN		pa_device;
	UINTN		pa_function;
	pcitag_t	pa_tag;
	pcireg_t	pa_id, pa_class;

	/*
	 * Interrupt information.
	 *
	 * "Intrline" is used on systems whose firmware puts
	 * the right routing data into the line register in
	 * configuration space.  The rest are used on systems
	 * that do not.
	 */
	UINTN		pa_intrswiz;	/* how to swizzle pins if ppb */
	pcitag_t	pa_intrtag;	/* intr. appears to come from here */
	pci_intr_pin_t	pa_intrpin;	/* intr. appears on this pin */
	pci_intr_line_t	pa_intrline;	/* intr. routing information */
};

/*
 * Flags given in the bus and device attachment args.
 *
 * OpenBSD doesn't actually use them yet -- csapuntz@cvs.openbsd.org
 */
#define	PCI_FLAGS_IO_ENABLED	0x01		/* I/O space is enabled */
#define	PCI_FLAGS_MEM_ENABLED	0x02		/* memory space is enabled */


struct pci_bus;
struct pci_device;

/*
 *  Structure describing a PCI BUS. An entire PCI BUS
 *  topology will be described by several structures
 *  linked together on a linked list.
 */
struct pci_bus {
	struct pci_bus *next;		/* next bus pointer */
	UINT8	min_gnt;        /* largest min grant */
	UINT8	max_lat;        /* smallest max latency */
	UINT8	devsel;         /* slowest devsel */
	UINT8	fast_b2b;       /* support fast b2b */
	UINT8	prefetch;       /* support prefetch */
	UINT8	freq66;         /* support 66MHz */
	UINT8	width64;        /* 64 bit bus */
	UINT8	bus;
	UINT8	ndev;           /* # devices on bus */
	UINT8	def_ltim;       /* default ltim counter */
	UINT8	max_ltim;       /* maximum ltim counter */
	INT32		bandwidth;      /* # of .25us ticks/sec @ 33MHz */
	paddr_t		minpcimemaddr;	/* PCI allocation min mem for bus */
	paddr_t		nextpcimemaddr;	/* PCI allocation max mem for bus */
	paddr_t		minpciioaddr;	/* PCI allocation min i/o for bus */
	paddr_t		nextpciioaddr;	/* PCI allocation max i/o for bus */
	paddr_t		pci_mem_base;
	paddr_t		pci_io_base;
};

struct pci_intline_routing;

/* Index values */
#define PCI_INTLINE_A	0
#define PCI_INTLINE_B	1
#define PCI_INTLINE_C	2
#define PCI_INTLINE_D	3

struct pci_intline_routing {
    UINT8	bus;
    UINT8	device;
    UINT8	function;
    UINT8	intline[32][4];	/* interrupt line mapping */
    struct pci_intline_routing *next;
};

/*
 * PCI Memory/IO Space
 */
struct pci_win {
	struct pci_win	*next;
	int		reg;
	int		flags;
	vm_size_t	size;
	pcireg_t	address;
	struct pci_device *device;
};

/*
 * PCI Bridge parameters
 */
struct pci_bridge {
	UINT8		pribus_num;	/* Primary Bus number */
	UINT8		secbus_num;	/* Secondary Bus number */
	UINT8		subbus_num;	/* Sub. Bus number */
	struct pci_bus		*secbus;	/* Secondary PCI bus pointer */
	struct pci_device	*child;
	struct pci_win		*memspace;
	struct pci_win		*iospace;
};

/*
 * PCI Device parameters
 */
struct pci_device {
	struct pci_attach_args	pa;
	UINT8		min_gnt;
	UINT8		max_lat;
	UINT8		int_line;
	pcireg_t		stat;
	UINT8		intr_routing[4];
	struct pci_bridge  	bridge;
	struct pci_bus		*pcibus;
	struct pci_device	*next;
	struct pci_device	*parent;
};

extern struct pci_device *_pci_head;
extern struct pci_bus *_pci_bushead;
extern struct pci_intline_routing *_pci_inthead;

#endif /* _DEV_PCI_PCIVAR_H_ */
