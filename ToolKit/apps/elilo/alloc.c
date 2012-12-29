/*
 *  Copyright (C) 2001-2003 Hewlett-Packard Co.
 *	Contributed by Stephane Eranian <eranian@hpl.hp.com>
 *	Contributed by Fenghua Yu <Fenghua.Yu@intel.com>
 *	Contributed by Bibo Mao <bibo.mao@intel.com>
 *	Contributed by Chandramouli Narayanan <mouli@linux.intel.com>
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

#include <efi.h>
#include <efilib.h>

#include "elilo.h"

#define NALLOC	512

typedef enum { ALLOC_POOL, ALLOC_PAGES } alloc_types_t;

typedef struct _alloc_entry {
	struct _alloc_entry	*next;
	struct _alloc_entry	*prev;
	VOID			*addr;
	UINTN			size; /* bytes for pool, page count for pages */
	alloc_types_t		type;
} alloc_entry_t;

static alloc_entry_t allocs[NALLOC];
static alloc_entry_t *free_allocs, *used_allocs;

static VOID *kmem_addr;
static UINTN kmem_pgcnt;

/*
 * initializes the free list which is singly linked
 */
INTN
alloc_init(VOID)
{
	UINTN i;

	for(i=0; i < NALLOC-1; i++) {
		allocs[i].next = allocs+i+1;
	}
	allocs[i].next = NULL;

	free_allocs = allocs;
	used_allocs = NULL;

	return 0;
}

static VOID
alloc_add(VOID * addr, UINTN size, alloc_types_t type)
{
	alloc_entry_t *alloc;

	/* remove from freelist */
	alloc 	    = free_allocs;
	free_allocs = free_allocs->next;
	
	alloc->prev = NULL;
	alloc->next = used_allocs;
	alloc->addr = addr;
	alloc->type = type;
	alloc->size = size;

	/* add to used list */
	if (used_allocs) used_allocs->prev = alloc;

	used_allocs = alloc;
}

VOID *
alloc(UINTN size, EFI_MEMORY_TYPE type)
{
	EFI_STATUS status;
	VOID *tmp = 0;

	/* no more free slots */
	if (free_allocs == NULL) {
        	ERR_PRT((L"allocator:  no more slots\n"));
		return NULL;
	}

	if (type == 0) type = EfiLoaderData;

	status = BS->AllocatePool(type, size, &tmp);
    	if (EFI_ERROR(status)) {
        	ERR_PRT((L"allocator: AllocatePool(%d, %d) failed (%r)\n", type, size, status));
		return NULL;
    	}
	alloc_add(tmp, size, ALLOC_POOL);
#ifdef DEBUG_MEM
        DBG_PRT((L"alloc: allocated %d bytes @[" PTR_FMT "-" PTR_FMT "]", size, tmp, tmp+size));
#endif
	return tmp;
}

/*
 *  no possibility to partially free an allocated group of pages
 */
VOID *
alloc_pages(UINTN pgcnt, EFI_MEMORY_TYPE type, EFI_ALLOCATE_TYPE where, VOID *addr)
{
	EFI_STATUS status;
	EFI_PHYSICAL_ADDRESS tmp = (EFI_PHYSICAL_ADDRESS)addr;

	/* no more free slots */
	if (free_allocs == NULL) {
        	ERR_PRT((L"allocator:  no more slots\n"));
		return NULL;
	}

	status = BS->AllocatePages (where, type , pgcnt, &tmp);
    	if (EFI_ERROR(status)) {
        	VERB_PRT(1, Print(L"allocator: AllocatePages(%d, %d, %d, 0x%lx) failed (%r)\n", where, type, pgcnt, tmp, status));
		return NULL;
    	}
	/* XXX: will cause warning on IA-32 */
	addr = (VOID *)tmp;

	alloc_add(addr, pgcnt, ALLOC_PAGES);

        DBG_PRT((L"allocator: allocated %d pages @" PTR_FMT, pgcnt, tmp));

	return addr;
}

/*
 * free previously allocated slot
 */
VOID
free(VOID *addr)
{
	alloc_entry_t *p;

	/* find allocation record */
	for(p=used_allocs; p ; p = p->next) {
		if (p->addr == addr) goto found;
	}
	/* not found */
        VERB_PRT(1, Print(L"allocator: invalid free @ " PTR_FMT "\n", addr));
	return;	
found:
#ifdef DEBUG_MEM
        DBG_PRT((L"free: %s @" PTR_FMT " size=%d",
		p->type == ALLOC_POOL ? L"Pool": L"Page", 
		addr, p->size));
#endif
	if (p->type == ALLOC_POOL) 
		BS->FreePool (addr);
	else
		BS->FreePages ((EFI_PHYSICAL_ADDRESS)addr, p->size);

	/* remove from used list */
	if (p->next) 
		p->next->prev = p->prev;
		
	if (p->prev) 
		p->prev->next = p->next;
	else
		used_allocs = p->next;

	/* put back on free list */
	p->next     = free_allocs;
	free_allocs = p;
}

/*
 * garbage collect all used allocations.
 * will put the allocator in initial state 
 */
VOID
free_all(VOID)
{
	alloc_entry_t *tmp;

	while(used_allocs) {
#ifdef DEBUG_MEM
		DBG_PRT((L"free_all %a @ " PTR_FMT, used_allocs->type == ALLOC_POOL ? "pool" : "pages", used_allocs->addr));
#endif	
		if (used_allocs->type == ALLOC_POOL)
			BS->FreePool (used_allocs->addr);
		else
			BS->FreePages ((EFI_PHYSICAL_ADDRESS)used_allocs->addr, used_allocs->size);

		tmp = used_allocs->next;

		/* put back on free list */
		used_allocs->next = free_allocs;
		free_allocs = used_allocs;

		used_allocs = tmp;
	}
}

INTN
alloc_kmem_anywhere(VOID **start_addr, UINTN pgcnt)
{
	void * tmp;
	/*
	 * During "AllocateAnyPages" *start_addr will be ignored.
	 * Therefore we can safely subvert it to reuse this function with
	 * an alloc_kmem_anyhwere_below() semantic...
	 */
	tmp = alloc_pages(pgcnt, EfiLoaderData,
			(*start_addr) ? AllocateMaxAddress : AllocateAnyPages,
			*start_addr);
	if (tmp == NULL) return -1;

	kmem_addr  = tmp;
	kmem_pgcnt = pgcnt;
	*start_addr = tmp;

	return 0;
}

INTN
alloc_kmem(VOID *start_addr, UINTN pgcnt)
{
	if (alloc_pages(pgcnt, EfiLoaderData, AllocateAddress, start_addr) == 0) return -1;

	kmem_addr  = start_addr;
	kmem_pgcnt = pgcnt;

	return 0;
}

VOID
free_kmem(VOID)
{
#ifdef DEBUG_MEM
	DBG_PRT((L"free_kmem before (" PTR_FMT ", %d)", kmem_addr, kmem_pgcnt));
#endif
	if (kmem_addr && kmem_pgcnt != 0) {
		free(kmem_addr);
		kmem_addr  = NULL;
		kmem_pgcnt = 0;
	}
#ifdef DEBUG_MEM
	DBG_PRT((L"free_kmem after (" PTR_FMT ", %d)", kmem_addr, kmem_pgcnt));
#endif
}

VOID
free_all_memory(VOID)
{
	free_all();
	free_kmem();
}
