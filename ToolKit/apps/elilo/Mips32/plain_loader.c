/*
 *  Copyright (C) 2001-2002 Hewlett-Packard Co.
 *	Contributed by Stephane Eranian <eranian@hpl.hp.com>
 *
 *  Copyright (C) 2001 Silicon Graphics, Inc.
 *      Contributed by Brent Casavant <bcasavan@sgi.com>
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
#include "loader.h"
#include "elf.h"
#include "private.h"

#define LD_NAME L"plain_elf32"

static INTN
is_valid_header(Elf32_Ehdr *ehdr)
{
	UINT16 type, machine;

	type    = ehdr->e_type;
	machine = ehdr->e_machine;

	DBG_PRT((L"class=%d type=%d data=%d machine=%d\n", 
		ehdr->e_ident[EI_CLASS],
		type,
		ehdr->e_ident[EI_DATA],
		machine));

	return    ehdr->e_ident[EI_MAG0]  == 0x7f 
	       && ehdr->e_ident[EI_MAG1]  == 'E'
	       && ehdr->e_ident[EI_MAG2]  == 'L'
	       && ehdr->e_ident[EI_MAG3]  == 'F'
	       && ehdr->e_ident[EI_CLASS] == ELFCLASS32 
	       && type                    == ET_EXEC	/* must be executable */
	       && machine                 == EM_386 ? 0 : -1;
}

static INTN
plain_probe(CHAR16 *kname)
{
	Elf32_Ehdr ehdr;
	EFI_STATUS status;
	INTN ret = -1;
	fops_fd_t fd;
	UINTN size = sizeof(ehdr);

	status = fops_open(kname, &fd);
	if (EFI_ERROR(status)) 
		return -1;

	status = fops_read(fd, &ehdr, &size);
	if (EFI_ERROR(status) || size != sizeof(ehdr)) 
		goto error;

	ret = is_valid_header(&ehdr);
error:
	fops_close(fd);
	return ret;
}


static INTN
load_elf(fops_fd_t fd, kdesc_t *kd)
{
	Elf32_Ehdr ehdr;
	Elf32_Phdr *phdrs;
	EFI_STATUS status;
	INTN ret = ELILO_LOAD_ERROR;
	UINTN i, total_size = 0;
	UINTN pages, size, bss_sz, osize;
	VOID *low_addr = (VOID *)~0;
	VOID *max_addr = (VOID *)0;
	UINTN paddr, memsz, filesz;
	UINT16 phnum;

	Print(L"Loading Linux... ");

	size = sizeof(ehdr);

	status = fops_read(fd, &ehdr, &size);
	if (EFI_ERROR(status) || size < sizeof(ehdr)) 
		return ELILO_LOAD_ERROR;

	if (is_valid_header(&ehdr) == -1) {
		ERR_PRT((L"%s : not a 32-bit ELF image\n", LD_NAME));
		return ELILO_LOAD_ERROR;
	}	 
	VERB_PRT(3, {
		Print(L"ELF Header information: \n");
		Print(L"\tEntry point "PTR_FMT"\n", (ehdr.e_entry & PADDR_MASK));
		Print(L"\t%d program headers\n", ehdr.e_phnum);
		Print(L"\t%d segment headers\n", ehdr.e_shnum);
	});

	phnum = ehdr.e_phnum;

	if (fops_seek(fd, ehdr.e_phoff) < 0) {
		ERR_PRT((L"%s : seek to %d for phdrs failed", LD_NAME, ehdr.e_phoff));
		return ELILO_LOAD_ERROR;
	}
	size = osize = (phnum * sizeof(Elf32_Phdr));

	DBG_PRT((L"%s : allocate %d bytes for %d pheaders each of size:%d phentsize=%d\n", 
		LD_NAME, size, phnum, sizeof(Elf32_Phdr), ehdr.e_phentsize));

	phdrs = (Elf32_Phdr *)alloc(size, 0); 
	if (phdrs == NULL) {
		ERR_PRT((L"%s : allocate for phdrs failed", LD_NAME));
		return ELILO_LOAD_ERROR;
	}
	status = fops_read(fd, phdrs, &size);
	if (EFI_ERROR(status) || size != osize) {
		ERR_PRT((L"%s : phdr load failed", LD_NAME, status));
		goto out;
	}
	/*
	 * First pass to figure out total memory footprint
	 */
	for (i = 0; i < phnum; i++) {

		paddr = (phdrs[i].p_paddr & PADDR_MASK);
		memsz = phdrs[i].p_memsz;

		DBG_PRT((L"Phdr %d paddr ["PTR_FMT"-"PTR_FMT"] offset "PTR_FMT""
			" filesz "PTR_FMT" memsz="PTR_FMT" bss_sz="PTR_FMT" p_type="PTR_FMT"\n",
			1+i, paddr, paddr+phdrs[i].p_filesz, phdrs[i].p_offset, 
			phdrs[i].p_filesz, memsz, 
			(memsz - phdrs[i].p_filesz), phdrs[i].p_type));
	
		if (phdrs[i].p_type != PT_LOAD)
		 	continue;
		if (paddr < (UINTN)low_addr) 
		 	low_addr = (VOID *)paddr;
		if (paddr + memsz > (UINTN)max_addr) 
			max_addr = (VOID *)paddr + memsz;
	}

	if ((UINTN)low_addr & (EFI_PAGE_SIZE - 1)) {
		ERR_PRT((L"%s : kernel low address "PTR_FMT" not page aligned\n", 
			LD_NAME, low_addr));
		goto out;
	}
	/* how many bytes are needed to hold the kernel? */
	total_size = (UINTN)max_addr - (UINTN)low_addr;

	/* round up to get required number of pages */
	pages = EFI_SIZE_TO_PAGES(total_size);

	/* keep track of location where kernel starts and ends */
	kd->kstart = low_addr;
	kd->kend   = (low_addr + (pages << EFI_PAGE_SHIFT));
	kd->kentry = (VOID *)(ehdr.e_entry & PADDR_MASK);
	
	VERB_PRT(3, {
		Print(L"Lowest PhysAddr: "PTR_FMT"\nTotalMemSize:%d bytes (%d pages)\n",
      			low_addr, total_size, pages);
		Print(L"Kernel entry @ "PTR_FMT"\n", kd->kentry);
	});
	
	/* now allocate memory for the kernel at the exact requested spot */
	if (alloc_kmem(low_addr, pages) == -1) {
		ERR_PRT((L"%s : AllocatePages(%d, "PTR_FMT") for kernel failed\n", 
			LD_NAME, pages, low_addr));
		ERR_PRT((L"%s : Could not alloc %d pages for the kernel at "PTR_FMT""
			" and relocation is not not been implemented!\n", 
			LD_NAME, pages, low_addr));
		goto load_abort;
	}
	/* Pure paranoia.  Clear the memory first.  Just in case... */
	Memset(low_addr, 0, (pages << EFI_PAGE_SHIFT));

	VERB_PRT(1, Print(L"Press any key to interrupt\n"));

	/* 
	 * Walk through the program headers
	 * and actually load data into physical memory
	 */

	for (i = 0; i < phnum; i++) {

		/* Check for pure loadable segment; ignore if not loadable */
		if (phdrs[i].p_type != PT_LOAD)
			continue;

		VERB_PRT(3, Print(L"poffs: "PTR_FMT" (phdrs[%d].p_offset)\n", 
			phdrs[i].p_offset, i));

		filesz = phdrs[i].p_filesz;
		low_addr = (VOID *)((UINTN) phdrs[i].p_paddr & PADDR_MASK);

		 /* Move to the right position */
		if (fops_seek(fd, phdrs[i].p_offset) < 0)
			goto out_kernel;

		 /* How many BSS bytes to clear */
		bss_sz = phdrs[i].p_memsz - filesz;

		VERB_PRT(4, {
			Print(L"\nHeader #%d\n", i);
			Print(L"Offset in file "PTR_FMT"\n", phdrs[i].p_offset);
			Print(L"Physical addr "PTR_FMT"\n", low_addr);
			Print(L"BSS size %d bytes\n", bss_sz);
		});

		/*
		 * Read actual segment into memory
		 */
		ret = fops_read(fd, low_addr, &filesz);
		if (ret == ELILO_LOAD_ABORTED) goto load_abort;
		if (ret == ELILO_LOAD_ERROR) goto out;

		/*
		 * Clear bss section
		 */
		if (bss_sz) 
			Memset((VOID *)low_addr+filesz, 0, bss_sz);
	}

	free(phdrs);

	Print(L"..Done\n");
	return ELILO_LOAD_SUCCESS;

load_abort:
	Print(L"..Aborted\n");
	ret = ELILO_LOAD_ABORTED;
out_kernel:
	/* free kernel memory */
	free_kmem();
out:
	free(phdrs);
	return ret;
}

static INTN
plain_load_kernel(CHAR16 *kname, kdesc_t *kd)
{	
	INTN ret;
	fops_fd_t fd;
	EFI_STATUS status;

	/*
	 * Moving the open here simplifies the load_elf() error handling
	 */
	status = fops_open(kname, &fd);
	if (EFI_ERROR(status)) return ELILO_LOAD_ERROR;

	Print(L"Loading %s...", kname);

	ret = load_elf(fd, kd);
	
	fops_close(fd);
	return ret;
}

loader_ops_t plain_loader={
	NULL,
	LD_NAME,
	plain_probe,
	plain_load_kernel
};
