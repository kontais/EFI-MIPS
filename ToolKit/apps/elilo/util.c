/*
 *  Copyright (C) 2001-2003 Hewlett-Packard Co.
 *	Contributed by Stephane Eranian <eranian@hpl.hp.com>
 *
 *  Copyright (C) 2001 Silicon Graphics, Inc.
 *	Contributed by Brent Casavant <bcasavan@sgi.com>
 *
 *  Copyright (C) 2006-2009 Intel Corporation
 *	Contributed by Fenghua Yu <fenghua.yu@intel.com>
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

#define TENTH_SEC		1000000	 	/* 1/10th second in 100ns unit */
#define READ_BLOCK_SIZE		(4*EFI_PAGE_SIZE) /* block size for read_file */

#define is_cr(k)		(((k)==CHAR_LINEFEED)||((k)==CHAR_CARRIAGE_RETURN))
#define CHAR_SPACE 		L' '

static INTN
read_keypress(EFI_INPUT_KEY *key)
{
	return systab->ConIn->ReadKeyStroke (
				systab->ConIn,
				key);
}


EFI_STATUS
check_abort(VOID)
{
	EFI_INPUT_KEY key;

	return read_keypress(&key);
}

inline VOID
reset_input(VOID)
{
	systab->ConIn->Reset (
			systab->ConIn,
			1);
}

#if 0
INTN
wait_keypress_abort(VOID)
{
	SIMPLE_INPUT_INTERFACE *conin = systab->ConIn;
	EFI_INPUT_KEY key;
	EFI_STATUS status;

	reset_input();

	Print(L"Hit ENTER to continue or ANY other key to cancel");

	/* cleanup buffer first */
	while (conin->ReadKeyStroke (conin, &key) == EFI_SUCCESS);

	while ((status=conin->ReadKeyStroke (conin, &key)) == EFI_NOT_READY );

	if (EFI_ERROR(status)) return ELILO_LOAD_ERROR;

	Print(L"\n");

	return is_cr(key.UnicodeChar) ? ELILO_LOAD_SUCCESS: ELILO_BOOT_ABORTED;
}
#endif

/*
 * wait for timeout to expire or keypress
 * Return:
 * 	0 : timeout expired
 * 	1 : a key was pressed (still input stream to process)
 * 	-1: an error occured
 */
INTN
wait_timeout(UINTN timeout)
{
       	EFI_STATUS status;
	EFI_EVENT timer;
	EFI_EVENT list[2];
	UINTN idx;


	if (timeout == 0) return 0;

	/* Create a timeout timer */
	status = BS->CreateEvent (EVT_TIMER, 0, NULL, NULL, &timer);
	if (EFI_ERROR(status)) {
		ERR_PRT((L" waitkey CreateEvent failed %r", status));
		return -1;
	}
	/* In 100ns increments */
	status = BS->SetTimer (timer, TimerPeriodic, TENTH_SEC);
	if (EFI_ERROR(status)) {
		ERR_PRT((L"waitkey SetTimer failed %r", status));
		return -1;
	}

	list[0] = timer;
	list[1] = systab->ConIn->WaitForKey;

	do {
		status = BS->WaitForEvent (2, list, &idx);
		if (EFI_ERROR(status)) {
			ERR_PRT((L"waitkey WaitForEvent failed %r", status));
			return -1;
		}
		if (timeout % 10 == 1) Print(L".");

	} while (timeout-- && idx == 0);	
	Print(L"\n");

	/*
	 * SetTimer(timer, TimerCancel, 0) is causing problems on IA-32 and gcc3
	 * I do not know why it dies with EFI12.35. So let's fake a key stroke.
	 */
	status = BS->SetTimer (timer, TimerCancel, 0);
	if (EFI_ERROR(status)) {
		ERR_PRT((L"waitkey SetTimer(TimerCancel) failed %r", status));
		return -1;
	}

	BS->CloseEvent (timer);

	return idx ? 1 : 0;
}

INTN
argify(CHAR16 *buf, UINTN len, CHAR16 **argv)   
{

        UINTN     i=0, j=0;
        CHAR16   *p = buf;
	
        if (buf == 0) { 
		argv[0] = NULL;
		return 0;
	}
	/* len represents the number of bytes, not the number of 16 bytes chars */
	len = len >> 1;

	/*
	 * Here we use CHAR_NULL as the terminator rather than the length
	 * because it seems like the EFI shell return rather bogus values for it.
	 * Apparently, we are guaranteed to find the '\0' character in the buffer
	 * where the real input arguments stop, so we use it instead.
	 */
	for(;;) {
		while (buf[i] == CHAR_SPACE && buf[i] != CHAR_NULL && i < len) i++;

		if (buf[i] == CHAR_NULL || i == len) goto end;

		p = buf+i;
		i++;

		while (buf[i] != CHAR_SPACE && buf[i] != CHAR_NULL && i < len) i++;

		argv[j++] = p;

		if (buf[i] == CHAR_NULL) goto end;

		buf[i]  = CHAR_NULL;

		if (i == len)  goto end;

		i++;

		if (j == MAX_ARGS-1) {
			ERR_PRT((L"too many arguments (%d) truncating", j));
			goto end;
		}
	}
end:
#if 0
	if (i != len) {
		ERR_PRT((L"ignoring trailing %d characters on command line", len-i));
	}
#endif
        argv[j] = NULL;
	return j;
}

VOID
unargify(CHAR16 **argv, CHAR16 **args)
{
	if ( *argv == 0 ) {
		*args = L"";
		return;
	}
	*args = *argv;
	while ( argv[1] ) {
		(*argv)[StrLen(*argv)] = CHAR_SPACE;
		argv++;
	}
}

VOID
split_args(CHAR16 *buffer, CHAR16 *kname, CHAR16 *args)
{
	CHAR16 *tmp;

	/* find beginning of kernel name */
	while (*buffer && *buffer == CHAR_SPACE) buffer++;

	tmp = buffer;
	
	/* scan through kernel name */	
	while (*buffer && *buffer != CHAR_SPACE) buffer++;

	if (*buffer) {
		*buffer++ = CHAR_NULL;
		StrCpy(kname, tmp);
	}

	/* skip space between kernel and args */	
	while (*buffer && *buffer == CHAR_SPACE) buffer++;

	StrCpy(args, buffer);
}

INTN
read_file(UINTN fd, UINTN total_size, CHAR8 *buffer)
{
	UINTN size, j=0;
	EFI_STATUS status;
	CHAR16 helicopter[4] = { L'|' , L'/' , L'-' , L'\\' };
	INTN ret = ELILO_LOAD_SUCCESS;
	UINTN sum = 0;
	/*
	 * We load by chunks rather than a single big read because
	 * early versions of EFI had troubles loading files
	 * from floppies in a single big request.  Breaking
	 * the read down into chunks of 4KB fixed that
 	 * problem. While this problem has been fixed, we still prefer
	 * this method because it tells us whether or not we're making
	 * forward progress.
	 */

	while (total_size > 0) {
		size = total_size < READ_BLOCK_SIZE? total_size : READ_BLOCK_SIZE;

		status = fops_read(fd, buffer, &size);
		if (EFI_ERROR(status)) {
			ERR_PRT((L"read_file failed %r", status));
			return ELILO_LOAD_ERROR;
		}
		sum += size;

		Print(L"%c\b",helicopter[j++%4]);

		buffer     += size;
		total_size -= size; 

		if (check_abort() == EFI_SUCCESS) {
			ret = ELILO_LOAD_ABORTED;
			break;
		}
	}
	return ret;
}

INTN
get_memmap(mmap_desc_t *desc)
{
#define	ELILO_MEMMAP_SIZE_DEFAULT	(EFI_PAGE_SIZE*2)
#define	ELILO_MEMMAP_INC		(sizeof(EFI_MEMORY_DESCRIPTOR)<<1)

	EFI_STATUS status;

	desc->map_size = ELILO_MEMMAP_SIZE_DEFAULT;

	for(;;) {
		desc->md = (EFI_MEMORY_DESCRIPTOR *)alloc(desc->map_size, EfiLoaderData);

		if (desc->md == NULL) {
			ERR_PRT((L"failed to allocate memory map buffer"));
			return -1;
		}
		status = BS->GetMemoryMap (&desc->map_size, desc->md, 
					&desc->cookie, &desc->desc_size, &desc->desc_version);
		if (status == EFI_SUCCESS) break;

		free(desc->md);

		if (status != EFI_BUFFER_TOO_SMALL) {
			ERR_PRT((L"failed to obtain memory map %r"));
			return -1;
		}
		desc->map_size += ELILO_MEMMAP_INC;
	}
	DBG_PRT((L"final get_memmap map_size=%d", desc->map_size));

	return 0;
}

#if 0
INTN
get_memmap(mmap_desc_t *desc)
{
	EFI_STATUS status;

	/* will get the right size in return */
	desc->map_size = 0;

	status = BS->GetMemoryMap(&desc->map_size, desc->md, &desc->cookie, &desc->desc_size, &desc->desc_version);
	if (status != EFI_BUFFER_TOO_SMALL) return -1;

	desc->md = (EFI_MEMORY_DESCRIPTOR *)alloc(desc->map_size, EfiLoaderData);
	if (desc->md == NULL) {
		ERR_PRT((L"failed to allocate memory map buffer"));
		return -1;
	}


	status = BS->GetMemoryMap(&desc->map_size, desc->md, &desc->cookie, &desc->desc_size, &desc->desc_version);
	if (EFI_ERROR(status)) {
		ERR_PRT((L"failed to obtain memory map %d: %r", desc->map_size, status));
		free(desc->md);
		return -1;
	}
	DBG_PRT((L"final get_memmap map_size=%d", desc->map_size));

	return 0;
}
#endif


VOID
free_memmap(mmap_desc_t *desc)
{
	if (desc->md) {
		free(desc->md);
		desc->md = NULL;
	}
}

VOID
print_memmap(mmap_desc_t *desc)
{
	EFI_MEMORY_DESCRIPTOR *md;
	UINTN desc_size;
	VOID *p;
	VOID *md_end;
	INT8 printed;
	UINTN ntypes;
	CHAR16* str;

	static CHAR16 *memtypes[]={
		L"ReservedMemoryType",
		L"LoaderCode",
		L"LoaderData",
		L"BootServicesCode",
		L"BootServicesData",
		L"RuntimeServicesCode",
		L"RuntimeServicesData",
		L"ConventionalMemory",
		L"UnusableMemory",
		L"ACPIReclaimMemory",
		L"ACPIMemoryNVS",
		L"MemoryMappedIO",
		L"MemoryMappedIOPortSpace",
		L"PalCode"
	};


	md_end = ((VOID *)desc->md)+desc->map_size;
	desc_size = desc->desc_size;

	ntypes = sizeof(memtypes)/sizeof(CHAR16 *);

	for(p = desc->md; p < md_end; p += desc_size) {
		md = p;

		str = md->Type < ntypes ? memtypes[md->Type] : L"Unknown";

		Print(L"%24s %lx-%lx %8lx", str, md->PhysicalStart,
				md->PhysicalStart+(md->NumberOfPages<<EFI_PAGE_SHIFT),
				md->NumberOfPages);

		printed=0;
#define P_FLG(f)	{ \
	Print(L" %s %s", printed ? L"|":L"", f); \
	printed=1; \
}

		if (md->Attribute & EFI_MEMORY_UC) {
			P_FLG(L"UC");
		}
		if (md->Attribute & EFI_MEMORY_WC) {
			P_FLG(L"WC");
		}
		if (md->Attribute & EFI_MEMORY_WT) {
			P_FLG(L"WT");
		}
		if (md->Attribute & EFI_MEMORY_WB) {
			P_FLG(L"WB");
		}
		if (md->Attribute & EFI_MEMORY_UCE) {
			P_FLG(L"UCE");
		}
		if (md->Attribute & EFI_MEMORY_WP) {
			P_FLG(L"WP");
		}
		if (md->Attribute & EFI_MEMORY_RP) {
			P_FLG(L"RP");
		}
		if (md->Attribute & EFI_MEMORY_XP) {
			P_FLG(L"XP");
		}
		if (md->Attribute & EFI_MEMORY_RUNTIME) {
			P_FLG(L"RT");
		}
		Print(L"\n");
	}
}

INTN
find_kernel_memory(VOID* low_addr, VOID* max_addr, UINTN alignment, VOID** start)
{       
#define HIGHEST_ADDR (VOID*)(~0)
        mmap_desc_t mdesc;
        EFI_MEMORY_DESCRIPTOR *md;
	UINT64 size;
	VOID *p, *addr;
	VOID *desc_end, *md_end, *best_addr = HIGHEST_ADDR;

	/*
	 * first get up-to-date memory map
	 *
	 * XXX: is there a danger of not seeing the latest version if interrupted
	 * during our scan ?
	 *
	 */
        if (get_memmap(&mdesc) == -1) {
                ERR_PRT((L"find_kernel_memory :GetMemoryMap() failed"));
                return -1;
        }

	desc_end = ((VOID *)mdesc.md) + mdesc.map_size;
	size     = max_addr - low_addr;
        /*
	 * Find memory which covers the desired range
	 */
	for(p = mdesc.md; p < desc_end; p += mdesc.desc_size) {
		md = p;

		/*
		 * restrict to decent memory types. 
		 *
		 * the EFI memory map report where memory is and how it is currently used
	 	 * using types.
		 *
		 * EfiLoaderData which is used by the AllocatePages() cannot be used
		 * here because it may hold some valid information. Same thing for most
		 * of the memory types with the exception of EfiConventional which 
		 * can be assumed as being free to use. 
		 */
		if (md->Type != EfiConventionalMemory) continue;

		/* 
		 * compute aligned address and upper boundary for range
		 */
		md_end = (VOID*)(md->PhysicalStart + md->NumberOfPages * EFI_PAGE_SIZE);	
		addr   = (VOID*)ROUNDUP(md->PhysicalStart, alignment);

		/*
		 * need to check if:
		 * - aligned address still in the range
		 * - the range [addr-addr+size) still fits into memory range
		 * if so we have a match. We do not assume that the memory ranges
		 * are sorted by EFI, therefore we must record the match and only
		 * keep the lowest possible one.
		 */
		if (addr < best_addr && addr < md_end && addr+size <= md_end) best_addr = addr;
        }
        if (best_addr == HIGHEST_ADDR) {
                free_memmap(&mdesc);
                ERR_PRT((L"Could not find memory suitable for loading image"));
                return -1;
        }

        *start = best_addr;

        free_memmap(&mdesc);

        return 0;
}

