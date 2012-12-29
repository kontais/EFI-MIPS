/* 
 * elilo.c - IA-64/IA-32/x86_64 EFI Linux loader
 *
 *  Copyright (C) 1999-2003 Hewlett-Packard Co.
 *	Contributed by David Mosberger <davidm@hpl.hp.com>.
 *	Contributed by Stephane Eranian <eranian@hpl.hp.com>
 *
 *  Copyright (C) 1999-2000 VA Linux Systems
 *       Contributed by Johannes Erdfelt <jerdfelt@valinux.com>.
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
#include "vars.h"
#include "gzip.h"

#include "getopt.h"
#include "fileops.h"
#include "loader.h"
#include "config.h" /* for config_init() */

#define ELILO_SHARED_CMDLINE_OPTS	L"pPMC:aDhd:i:vVc:E"

elilo_config_t elilo_opt;

EFI_SYSTEM_TABLE *systab;	/* pointer to EFI system table */

extern INTN wait_timeout (UINTN);

/*
 * Load the Linux kernel in memory from the boot media
 * Output:
 * 	kd = address of kernel entry point
 * 	   + end address of kernel code+data+bss
 * 	   + kernel entry point
 * Return:
 * 	ELILO_LOAD_ERROR   : if something went wrong
 * 	ELILO_LOAD_ABORTED : user interruption while loading
 * 	ELILO_LOAD_SUCCESS : otherwise
 */
static INTN
do_kernel_load(CHAR16 *kname, kdesc_t *kd)
{
	loader_ops_t	*ldops;
	EFI_STATUS status;
	fops_fd_t fd;

	status = fops_open(kname, &fd);
	if (EFI_ERROR(status)) {
		ERR_PRT((L"Kernel file  not found %s", kname));
		return ELILO_LOAD_ERROR;
	}
	fops_close(fd);

	ldops = loader_probe(kname);
	if (ldops == NULL) {
		ERR_PRT((L"Cannot find a loader for %s", kname));
		return ELILO_LOAD_ERROR;
	}

	VERB_PRT(1,Print(L"Using %s loader\n", ldops->ld_name));
	
	return ldops->ld_load_kernel(kname, kd);
}

INTN
kernel_load(EFI_HANDLE image, CHAR16 *kname, kdesc_t *kd, memdesc_t *imem, memdesc_t *mmem)
{
	CHAR16 kernel[FILENAME_MAXLEN];

	/*
	 * Do the vm image switch here
	 * if there is "vmm=" then elilo should load image specified
	 * in "vmm=" and then give the "image" to vmm as target kernel image
	 */
	if (elilo_opt.vmcode[0])
		StrCpy(kernel, elilo_opt.vmcode);
	else
		StrCpy(kernel, kname);
	/*
	 * Now let's try to load the kernel !
	 */
	switch(do_kernel_load(kernel, kd)) {
		case ELILO_LOAD_SUCCESS:
			break;

		case ELILO_LOAD_ERROR:
			/* XXX: add fallback support */
			return ELILO_LOAD_ERROR;
		
		case ELILO_LOAD_ABORTED:
			/* we drop initrd in case we aborted the load */
			elilo_opt.initrd[0] = CHAR_NULL;
			elilo_opt.vmcode[0] = CHAR_NULL;

			/* will go back to interactive selection */
			elilo_opt.prompt  = 1; 
			elilo_opt.timeout = ELILO_DEFAULT_TIMEOUT;
			elilo_opt.delay   = 0;

			return ELILO_LOAD_RETRY;	
	}

	VERB_PRT(3, Print(L"kernel loaded in [" PTR_FMT "-" PTR_FMT "] entry=" PTR_FMT "\n", 
			  kd->kstart, kd->kend, kd->kentry));

	if (elilo_opt.initrd[0]) {

		/* ramdisk image is moved to the top of available extended memory later by start_kernel() */
		if (sysdeps_initrd_get_addr(kd, imem) == -1) goto exit_error;

		switch(load_file(elilo_opt.initrd, imem)) {
			case ELILO_LOAD_SUCCESS:
				break;
			case ELILO_LOAD_ERROR:
				goto exit_error;
			case ELILO_LOAD_ABORTED:
				free_kmem();
				/* we drop initrd in case we aborted the load */
				elilo_opt.initrd[0] = CHAR_NULL;
				elilo_opt.vmcode[0] = CHAR_NULL;
				elilo_opt.prompt    = 1; 
				elilo_opt.timeout   = ELILO_DEFAULT_TIMEOUT;
				elilo_opt.delay     = 0;

				return ELILO_LOAD_RETRY;
		}
	}

	if (elilo_opt.vmcode[0]) {

		mmem->start_addr = 0; /* let the allocator decide */

		switch(load_file(kname, mmem)) {
			case ELILO_LOAD_SUCCESS:
				break;
			case ELILO_LOAD_ERROR:
				goto exit_error;
			case ELILO_LOAD_ABORTED:
				if (imem->start_addr)
					free(imem->start_addr);
				free_kmem();
				/* we drop initrd in case we aborted the load */
				elilo_opt.initrd[0] = CHAR_NULL;
				elilo_opt.vmcode[0] = CHAR_NULL;
				elilo_opt.prompt    = 1; 
				elilo_opt.timeout   = ELILO_DEFAULT_TIMEOUT;
				elilo_opt.delay     = 0;

				return ELILO_LOAD_RETRY;
		}

		/* Test for a compressed image and unzip if found */
		if (gzip_probe(mmem->start_addr, mmem->size) == 0 &&
		    gunzip_image(mmem) != ELILO_LOAD_SUCCESS) {
			if (imem->start_addr)
				free(imem->start_addr);
			free(mmem->start_addr);
			free_kmem();
			/* we drop initrd in case we aborted the load */
			elilo_opt.initrd[0] = CHAR_NULL;
			elilo_opt.vmcode[0] = CHAR_NULL;
			elilo_opt.prompt    = 1; 
			elilo_opt.timeout   = ELILO_DEFAULT_TIMEOUT;
			elilo_opt.delay     = 0;

			return ELILO_LOAD_RETRY;
		}
	}
	return ELILO_LOAD_SUCCESS;
exit_error:
	free_kmem();
	if (imem->start_addr) free(imem->start_addr);
	if (mmem->start_addr) free(mmem->start_addr);

	return ELILO_LOAD_ERROR;
}

static INTN
main_loop(EFI_HANDLE dev, CHAR16 **argv, INTN argc, INTN index, EFI_HANDLE image)
{
	CHAR16 kname[FILENAME_MAXLEN];
	CHAR16 cmdline_tmp[CMDLINE_MAXLEN];
	CHAR16 cmdline[CMDLINE_MAXLEN];
	VOID *bp;
	UINTN cookie;
	EFI_STATUS status = EFI_SUCCESS;
	kdesc_t kd;
	memdesc_t imem, mmem;
	INTN r, retries=0;

	/*
	 * First place where we potentially do system dependent
	 * operations. It is a one time opportunity before entering
	 * the main loop
	 */
	if (sysdeps_preloop_actions(dev, argv, argc, index, image) == -1) return -1;

	for(;;) {
		kname[0] = cmdline_tmp[0] = cmdline[0] = CHAR_NULL;
		imem.start_addr = 0; imem.pgcnt = 0; imem.size = 0;
		elilo_opt.sys_img_opts = NULL;

		if (kernel_chooser(argv, argc, index, kname, cmdline_tmp) == -1) goto exit_error;

		switch (kernel_load(image, kname, &kd, &imem, &mmem)) {
			case ELILO_LOAD_SUCCESS: 
				goto do_launch;
			case ELILO_LOAD_ERROR:
				goto exit_error;
			/* otherwise we retry ! */
		}
	} 

do_launch:
	r =subst_vars(cmdline_tmp, cmdline, CMDLINE_MAXLEN);

	VERB_PRT(3, Print(L"final cmdline(%d): %s\n", r, cmdline));

	/* Give user time to see the output before launch */
	if (elilo_opt.debug || elilo_opt.verbose) {
		r = wait_timeout(150);
	/* have to turn off all console output(except error output) now before final get_mmemap()
	 * call or it can cause the efi map key to change and the ExitBootSvc call to fail,
	 * forcing debug and verbose options off is the surest way to enforce this.
	 */
		elilo_opt.debug=0;
		elilo_opt.verbose=0; 
	}

	/* free resources associated with file accesses (before ExitBootServices) */
	close_devices();

	/* No console output permitted after create_boot_params()! */
	if ((bp=create_boot_params(cmdline, &imem, &mmem, &cookie)) == 0) goto error;

	/* terminate bootservices 
	* efi ExitBootSvcs spec: *note, get_memmap is called by create_boot_params() 
	* An EFI OS loader must ensure that it has the system's current memory map at the time 
	* it calls ExitBootServices(). This is done by passing in the current memory map's 
	* MapKey value as returned by GetMemoryMap(). Care must be taken to ensure that the 
	* memory map does not change between these two calls. It is suggested that 
	* GetMemoryMap()be called immediately before calling ExitBootServices(). */

retry:
	status = BS->ExitBootServices (image, cookie);
	if (EFI_ERROR(status)) 
	{
		ERR_PRT((L"\nExitBootSvcs: failed, memory map has changed.\n"));
		if (retries < 2)
		{	
			ERR_PRT((L"Main_Loop: Retrying,... have to rebuild boot params"));
			retries++;
			free_boot_params(bp);
			if ((bp=create_boot_params(cmdline, &imem, &mmem, &cookie)) == 0) goto error;			
			goto retry;
		} else {
			ERR_PRT((L"\nMain_Loop: tried ExitBootSvcs 3 times... retries exceeded.... giving up\n"));
			goto bad_exit;
		}
	}


	start_kernel(kd.kentry, bp);
	/* NOT REACHED */

	ERR_PRT((L"start_kernel() return !"));
bad_exit:	
	/*
	 * we are still in BootService mode
	 */
	ERR_PRT((L"ExitBootServices failed %r", status));
error:
	free_kmem();
	if (imem.start_addr) free(imem.start_addr);
	if (mmem.start_addr) free(mmem.start_addr);
	if (bp) free_boot_params(bp);
exit_error:
	return ELILO_LOAD_ERROR;

}

static VOID
elilo_help(VOID)
{
	
	Print(L"-d secs   timeout in 10th of second before booting\n");
	Print(L"-h        this help text\n");
	Print(L"-V        print version\n");
	Print(L"-v        verbose level(can appear multiple times)\n");
	Print(L"-a        always check for alternate kernel image\n");
	Print(L"-i file   load file as the initial ramdisk\n");
	Print(L"-m file   load file as additional boot time vmm module\n");
	Print(L"-C file   indicate the config file to use\n");
	Print(L"-P        parse config file only (verify syntax)\n");
	Print(L"-D        enable debug prints\n");
	Print(L"-p        force interactive mode\n");
	Print(L"-c name   image chooser to use\n");
	Print(L"-E        do not force EDD30 variable\n");

	sysdeps_print_cmdline_opts();

	Print(L"\n");

	print_config_options();
}

/*
 * XXX: hack to work around a problem with EFI command line argument when booted
 * from network. In this case, it looks like LoadOptions/LoadOptionsSize contain
 * garbage
 */
static CHAR16 *default_load_options;
static UINTN default_load_options_size;
static INTN done_fixups;

static VOID
fixupargs(EFI_LOADED_IMAGE *info)
{
	EFI_STATUS status;
	EFI_PXE_BASE_CODE *pxe;

#define FAKE_ELILONAME	L"elilo-forced"

	status = BS->HandleProtocol (info->DeviceHandle, &PxeBaseCodeProtocol, (VOID **)&pxe);
	if (EFI_ERROR(status)) return;

	default_load_options      = info->LoadOptions;
	default_load_options_size = info->LoadOptionsSize;

	info->LoadOptions     = FAKE_ELILONAME;
	info->LoadOptionsSize = StrLen(info->LoadOptions)*sizeof(CHAR16);

	done_fixups = 1;
}

/*
 * we restore the arguments in case we modified them just to make sure
 * we don't confuse caller.
 */
static VOID
unfixupargs(EFI_LOADED_IMAGE *info)
{
	if (done_fixups == 0) return;

	info->LoadOptions     = default_load_options;
	info->LoadOptionsSize = default_load_options_size;
}


/*
 * in order to get fully detailed EFI path names to devices, EDD3.0 specification must
 * be turned on. On new versions of EFI, this is the default. An environment variable
 * called EDD30 reflects the current settings. If true, then EDD3.0 is enabled 
 * and device path names show the detailed device types. If false, then a default 
 * generic name is used instead. This has some implications of ELILO's ability to 
 * provide a better naming scheme for detected devices. If EDD3.0 is enabled
 * then much more precise names can be used which makes it easy to use. 
 * If EDD3.0 is nont enabled, ELILO will still work but will use very generic names
 * for devices.
 *
 * ELILO will check the value of the variable. If not true, then it will force it to
 * true. This will require a reboot for EFI to make use of the new value.
 * Return:
 * 	EFI_SUCCESS: if variable is already true or was set to true
 * 	EFI error code: otherwise, like when could not forced variable or unrecognized variable content
 */
#define EDD30_GUID \
{0x964e5b21, 0x6459, 0x11d2, { 0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b}}

#define EDD30_ATTR (EFI_VARIABLE_RUNTIME_ACCESS|EFI_VARIABLE_BOOTSERVICE_ACCESS|EFI_VARIABLE_NON_VOLATILE)

static EFI_GUID edd30_guid = EDD30_GUID;

static INTN
check_edd30(VOID)
{
	EFI_STATUS	status;
	UINTN		l = sizeof(BOOLEAN);
	UINT8		bool = FALSE;
	INTN		ret = -1;

	status = RT->GetVariable (L"EDD30", &edd30_guid, NULL, &l, &bool);
	if (status == EFI_BUFFER_TOO_SMALL || (bool != TRUE && bool != FALSE)) {
		ERR_PRT((L"Warning: EDD30 EFI variable is not boolean value: forcing it to TRUE"));
		return -1;
	}
	if (status == EFI_SUCCESS && bool == TRUE) {
		VERB_PRT(3, Print(L"EDD30 is TRUE\n"));
		elilo_opt.edd30_on = TRUE;
		ret = 0;
	} else {
		VERB_PRT(4, 
			if (status != EFI_SUCCESS) {
				Print(L"EDD30 EFI variable not defined\n");
			} else {
				Print(L"EDD30 EFI variable is false\n");
			}
		);
	}

	return ret;
}

static INTN
force_edd30(VOID)
{
	EFI_STATUS	status;
	UINTN		l = sizeof(BOOLEAN);
	UINT8		bool;

	bool = TRUE;
	status = RT->SetVariable (L"EDD30", &edd30_guid, EDD30_ATTR, l, &bool);
	if (EFI_ERROR(status)) {
		ERR_PRT((L"can't set EDD30 variable: ignoring it"));
		return -1;
	}

	VERB_PRT(3, Print(L"EDD30 variable forced to TRUE. You should reboot to take advantage of EDD3.0.\n"));

	return 0;
}

/*
 * That's equivalent of main(): main entry point
 */
EFI_STATUS
efi_main (EFI_HANDLE image, EFI_SYSTEM_TABLE *system_tab)
{
	CHAR16 *argv[MAX_ARGS];
	CHAR16 optstring[MAX_ARGS];
	EFI_LOADED_IMAGE *info;
	EFI_STATUS status, ret = EFI_LOAD_ERROR;
	INTN argc = 0, c;
	INTN edd30_status, retry;
	CHAR16 *ptr, *arglist = NULL;
	BOOLEAN devices_initialized = FALSE;
	CHAR16 dpath[FILENAME_MAXLEN];
	CHAR16 *devpath;

	elilo_opt.verbose=0;
	elilo_opt.debug=0;

	/* initialize global variable */
	systab = system_tab;
	
	/* initialize EFI library */
	InitializeLib(image, systab);
	/*
	 * disable the platform watchdog timer if we go interactive
	 * XXX: we should reinstate it once we're done
	 * It seems like with PXE, the timer is set to a few minutes
	 * and sometimes triggers if we stay too long in interactive
	 * mode.
	 * XXX: clean this up !
	 */
       BS->SetWatchdogTimer (0, 0x0, 0, NULL);

	/* initialize memory allocator */
	if (alloc_init() == -1) return EFI_LOAD_ERROR;

	status = BS->HandleProtocol (image, &LoadedImageProtocol, (VOID **) &info);
	if (EFI_ERROR(status)) {
		ERR_PRT((L"image handle does not support LOADED_IMAGE protocol"));
		return EFI_LOAD_ERROR;
	}

	VERB_PRT(5,Print(L"Loaded at " PTR_FMT " size=%ld bytes code=%d data=%d\n", info->ImageBase, info->ImageSize, info->ImageCodeType, info->ImageDataType));
	/*
	 * verify EDD3.0 status. Users may have to reboot 
	 */
	edd30_status = check_edd30();

	/*
	 * initialize configuration empire
	 */
	if (config_init() == -1) goto do_exit;

	/* 
	 * architecture-specific initializations
	 */
	if (sysdeps_init(info->DeviceHandle) == -1) goto do_exit;
	if (sysdeps_register_options() == -1) goto do_exit;

	/*
	 * This may be required in case Elilo was booted with absolutely no arguments
	 * Elilo's logic is that just like normal Linux programs at least one argument
	 * (argv[0]) exists at all times and that it usually gives the name of the program
	 * (the command used to start it).
	ERR_PRT((L"LoadOptions=%x OpenSize=%d", info->LoadOptions, info->LoadOptionsSize));
	 */

	/*
	 * in case there is something wrong with the default boot_device
	 * we default to basic fixups and we need to force interactive
	 * mode to make sure the user has a chance of specifying a kernel
	 */
	fixupargs(info);

#if 0
	Print(L"LoadOptions=%x OpenSize=%d\n", info->LoadOptions, info->LoadOptionsSize);
	{ INTN i; for (i=0; i< info->LoadOptionsSize>>1; i++) Print(L"options[%d]=%d (%c)\n", i, ((CHAR16 *)info->LoadOptions)[i], ((CHAR16 *)info->LoadOptions)[i]); }
#endif

	/*
	 * we must copy argument because argify modifies the string.
	 * This caused problems when arguments are coming from NVRAM
	 * as passed by the EFI boot manager
	 *
	 * We add an extra character to the buffer in case the LoadOptions is not 
	 * NULL terminated. The extra space will be used to ADD the extra terminator.
	 */
	arglist = alloc(info->LoadOptionsSize+sizeof(CHAR16), EfiLoaderData);
	if (arglist == NULL) {
		ERR_PRT((L"cannot copy argument list"));
		return EFI_OUT_OF_RESOURCES;
	}
	Memcpy(arglist, info->LoadOptions, info->LoadOptionsSize);

	argc = argify(arglist,info->LoadOptionsSize, argv); 

	StrCpy(optstring, ELILO_SHARED_CMDLINE_OPTS);
	StrCat(optstring, sysdeps_get_cmdline_opts());

	while ((c=Getopt(argc, argv, optstring)) != -1 ) {
		switch(c) {
			case 'a':
				elilo_opt.alt_check = 1;
				break;
			case 'D':
				elilo_opt.debug = 1;
				break;
			case 'p':
				elilo_opt.prompt = 1;
				break;
			case 'v':
				elilo_opt.verbose++;
				if (elilo_opt.verbose > 5) elilo_opt.verbose = 5;
				break;
			case 'h':
				elilo_help();
				ret = EFI_SUCCESS;
				goto do_exit;
			case 'd':
				/*
				 * zero is a valid value here, so we use the delay-set to mark the
				 * fact that the user specified a value on cmdline. See config.c
				 */
				elilo_opt.delay = Atoi(Optarg);
				elilo_opt.delay_set = 1;
				break;
			case 'E':
				/* don't force EDD30 EFI variable if not already set */
				elilo_opt.edd30_no_force = 1;
				break;
			case 'i':
				if (StrLen(Optarg) >= FILENAME_MAXLEN-1) {
					Print(L"initrd filename is limited to %d characters\n", FILENAME_MAXLEN);
					goto do_exit;
				}
				StrCpy(elilo_opt.initrd, Optarg);
				break;
			case 'm':
				if (StrLen(Optarg) >= FILENAME_MAXLEN-1) {
					Print(L"vmm module filename is limited to %d characters\n", FILENAME_MAXLEN);
					goto do_exit;
				}
				StrCpy(elilo_opt.vmcode, Optarg);
				break;
			case 'C':
				if (StrLen(Optarg) >= FILENAME_MAXLEN-1) {
					Print(L"config filename is limited to %d characters\n", FILENAME_MAXLEN);
					goto do_exit;
				}
				StrCpy(elilo_opt.config, Optarg);
				break;
			case 'M': /* builtin debug tool */
				{ mmap_desc_t mdesc; 
					if (get_memmap(&mdesc) == -1) {
						Print(L"Cannot get memory map\n");
						return EFI_LOAD_ERROR;
					}
					print_memmap(&mdesc);
					ret = EFI_SUCCESS;
					goto do_exit;
				}
			case 'V':
				Print(L"ELILO v%s for EFI/%a\n", ELILO_VERSION, ELILO_ARCH);
				ret = EFI_SUCCESS;
				goto do_exit;
			case 'P':
				/* cmdline only option */
				elilo_opt.parse_only =  1;
				break;
			case 'c':
				if (StrLen(Optarg) >= FILENAME_MAXLEN-1) {
					Print(L"chooser name is limited to %d characters\n", FILENAME_MAXLEN);
					goto do_exit;
				}
				StrCpy(elilo_opt.chooser, Optarg);
				break;
			default:
				/*
				 * try system dependent options before declaring error
				 */
				if (sysdeps_getopt(c, Optind, Optarg) == 0) continue;

				Print(L"Unknown option -%c\n", (CHAR16)c);
				goto do_exit;
		}
	}
	DBG_PRT((L"Optind=%d optarg=" PTR_FMT " argc=%d", Optind, Optarg, argc));

	/*
	 * we can't defer this phase any longer...
	 * Must be done after the elilo_opt are initialized (at least partially)
	 */
	if (init_devices(info->DeviceHandle) == -1) goto do_exit;

	devices_initialized = TRUE;

	devpath = DevicePathToStr(info->FilePath);

	/*
	 * set per fileops defaults files for configuration and kernel
	 */
	fops_setdefaults(elilo_opt.default_configs, elilo_opt.default_kernel, FILENAME_MAXLEN, devpath);

	/*
	 * XXX: won't be visible if verbose not required from command line
	 */
	VERB_PRT(2,Print(L"Default config: %s\nDefault_kernel: %s\n",
			elilo_opt.default_configs[0].fname, elilo_opt.default_kernel));
	/*
	 * use default config file if not specified by user
	 */
	ptr = elilo_opt.config[0] == CHAR_NULL ?  (retry=1,elilo_opt.default_configs[0].fname) : (retry=0,elilo_opt.config);

	/*
	 * parse config file (verbose becomes visible if set)
	 */
	ret = read_config(ptr);
	VERB_PRT(1,Print(L"read_config=%r\n", ret));

        /* Only try the default config filenames if user did not specify a
         * config filename on the command line */
        if (elilo_opt.config[0] == CHAR_NULL) {
                while ((ret != EFI_SUCCESS) &&
                       (retry < MAX_DEFAULT_CONFIGS) &&
                       (elilo_opt.default_configs[retry].fname[0] != CHAR_NULL)) {

                        ptr = elilo_opt.default_configs[retry].fname;
                        ret = read_config(ptr);
                        VERB_PRT(1,Print(L"read_config=%r\n", ret));
                        retry += 1;
                }
        }
	/*
	 * when the config file is not found, we fail only if:
	 * 	- the user did not specified interactive mode
	 * 	- the user did not explicitely specify the config file
	 */
	if (ret == EFI_NOT_FOUND || ret == EFI_TFTP_ERROR) {
		if (elilo_opt.prompt == 0 && elilo_opt.config[0] != CHAR_NULL) {
			Print(L"config file %s not found\n", ptr);
			goto do_exit;
		}
		fops_getdefault_path(dpath, FILENAME_MAXLEN);
		if (ret == EFI_TFTP_ERROR)
			Print(L"no config file found on TFTP server in %s\n", dpath);
		else
			Print(L"no config file found in %s\n", dpath);
	}
	/*
	 * stop if just doing parsing
	 */
	if (elilo_opt.parse_only) {
		if (ret == EFI_SUCCESS)
			Print(L"Config file %s parsed successfully\n", ptr);
		goto do_exit;
	}
	/*
	 * if there was an error when parsing the config file, then
	 * we force interactive mode to give a chance to the user.
	 * We also clear the error. 
         */
	if (ret != EFI_SUCCESS) {
		Print(L"forcing interactive mode due to config file error(s)\n");
		elilo_opt.prompt = 1;
	}
        /*
	 * However, if the user specified a kernel on the command line
	 * then we don't go to interactive mode, even if there was an option in
         * the config file telling us to do so.
	 */
	if (argc > Optind) {
            elilo_opt.prompt = 0;
        }

	/*
	 * If EDD30 EFI variable was not set to TRUE (or not defined), we
	 * we try to force it. This will take effect at the next reboot.
	 *
	 * Some controllers don't have EDD30 support, in this case forcing it 
	 * may cause problems, therefore we check the edd_no_force option
	 * before making the call.
	 */
	if (edd30_status == -1 && elilo_opt.edd30_no_force == 0) {
		force_edd30();
	}

	ret = EFI_LOAD_ERROR;



	/* set default timeout if going interactive */
	if ((elilo_opt.prompt && elilo_opt.timeout == 0)) {
		elilo_opt.timeout = ELILO_DEFAULT_TIMEOUT;
	}

	/*
	 * which chooser we will use 
	 */
	if (init_chooser(info->DeviceHandle) == -1) {
		ERR_PRT((L"Cannot find a decent chooser\n"));
		goto do_exit;
	}

	//if (elilo_opt.prompt == 0) VERB_PRT(1, print_devices());

	main_loop(info->DeviceHandle, argv, argc, Optind, image);
	/* should not return */
do_exit:
	unfixupargs(info);

	//if (arglist) free(arglist);

	/* free all resources assiocated with file accesses */
	if (devices_initialized) close_devices();

	/* garbage collect all remaining allocations */
	free_all_memory();

	return ret;
}
