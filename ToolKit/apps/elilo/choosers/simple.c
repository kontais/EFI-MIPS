/* 
 *  Copyright (C) 2001-2003 Hewlett-Packard Co.
 *	Contributed by Stephane Eranian <eranian@hpl.hp.com>
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
#include "console.h"

/* static is ugly but does the job here! */
static CHAR16 **alt_argv;

static VOID
display_label_info(CHAR16 *name)
{
	CHAR16 *desc;
	CHAR16 initrd_name[PATHNAME_MAXLEN];
	CHAR16 vmcode_name[PATHNAME_MAXLEN];
	CHAR16 options_tmp[CMDLINE_MAXLEN];
	CHAR16 options[CMDLINE_MAXLEN];
	CHAR16 kname[FILENAME_MAXLEN];

	desc = find_description(name);
	if (desc) {
		Print(L"desc   : %s\n", desc);
	}

	initrd_name[0] = vmcode_name[0] = options_tmp[0] = kname[0] = CHAR_NULL;

	if (find_label(name, kname, options_tmp, initrd_name, vmcode_name) == -1) {
		StrCpy(kname, name);
		Print(L"\n");
	}
	subst_vars(options_tmp, options, CMDLINE_MAXLEN);

	Print(L"cmdline: %s %s\n", kname, options);
	if (initrd_name[0]) Print(L"initrd : %s\n", initrd_name);
	if (vmcode_name[0]) Print(L"vmcode : %s\n", vmcode_name);
}

static VOID
print_infos(int force)
{
	CHAR16 *config_file;
	CHAR16 dpath[FILENAME_MAXLEN];
	CHAR16 *boot_dev_name;
	UINT8 is_abs;

	boot_dev_name = fops_bootdev_name();
	config_file   = get_config_file();

	fops_getdefault_path(dpath, FILENAME_MAXLEN);

	if (force || elilo_opt.verbose > 0)
		Print(L"default file path: %s:%s\n", boot_dev_name, dpath);

	is_abs = config_file && (config_file[0] == CHAR_BACKSLASH || config_file[0] == CHAR_SLASH) ? 1 : 0;

	if (force || elilo_opt.verbose > 0)
			Print(L"config file      : %s%s\n", config_file && is_abs == 0 ? dpath : L"", config_file ? config_file : L"none used");

	if (alt_argv) {
		CHAR16 **p = alt_argv;
		Print(L"found alternate default choice :");
		while (*p) Print(L" %s", *p++);
		Print(L"\n");
	}
}

static VOID
print_help(int force)
{
	if (force || elilo_opt.verbose > 0)
		Print(L"command list (must be first character):\n=:print device list, %%:print variable list, &:print paths, ?:help\nTAB:print label information\n");
}

/*
 * interactively select a kernel image and options.
 * The kernel can be an actual filename or a label in the config file
 * Return:
 * 	-1: if unsucessful
 * 	 0: otherwise
 */
static INTN
select_kernel(CHAR16 *buffer, INTN size)
{
#define CHAR_CTRL_C	L'\003' /* Unicode CTRL-C */
#define CHAR_CTRL_D	L'\004' /* Unicode CTRL-D */
#define CHAR_CTRL_U	L'\025' /* Unicode CTRL-U */
//#define CHAR_TAB	L'\t'
	SIMPLE_INPUT_INTERFACE *ip = systab->ConIn;
	EFI_INPUT_KEY key;
	EFI_STATUS status;
	INTN pos = 0, ret;
	INT8 first_time = 1;

	/* 
	 * let's give some help first
	 */
	print_help(0);

	print_infos(0);

reprint:
	buffer[pos] = CHAR_NULL;

	Print(L"\nELILO boot: %s", buffer);
	/*
	 * autoboot with default choice after timeout expires
	 */
	if (first_time && (ret=wait_timeout(elilo_opt.timeout)) != 1) {
		return ret == -1 ? -1: 0;
	}
	first_time = 0;

	for (;;) {
		while ((status = ip->ReadKeyStroke (ip, &key))
				 == EFI_NOT_READY);
		if (EFI_ERROR(status)) {
			ERR_PRT((L"select_kernel readkey: %r", status));
			return -1;
		} 
		switch (key.UnicodeChar) {
			case CHAR_TAB:
				Print(L"\n");
				if (pos == 0) {
					print_label_list();
					Print(L"(or a kernel file name: [[dev_name:/]path/]kernel_image cmdline options)\n");
				} else {
					buffer[pos] = CHAR_NULL;
					display_label_info(buffer);
				}
				goto reprint;
			case L'%':
				if (pos>0) goto normal_char;
				Print(L"\n");
				print_vars();
				goto reprint;
			case L'?':
				if (pos>0) goto normal_char;
				Print(L"\n");
				print_help(1);
				goto reprint;
			case L'&':
				if (pos>0) goto normal_char;
				Print(L"\n");
				print_infos(1);
				goto reprint;
			case L'=':
				if (pos>0) goto normal_char;
				Print(L"\n");
				print_devices();
				goto reprint;
			case CHAR_BACKSPACE:
				if (pos == 0) break;
				pos--;
				Print(L"\b \b");
				break;
			case CHAR_CTRL_U: /* clear line */
				while (pos) {
					Print(L"\b \b");
					pos--;
				}
				break;
			case CHAR_CTRL_C: /* kill line */
				pos = 0;
				goto reprint;
			case CHAR_LINEFEED:
			case CHAR_CARRIAGE_RETURN:
				buffer[pos]   = CHAR_NULL;
				Print(L"\n");
				return 0;
			default:
normal_char:
				if (key.UnicodeChar == CHAR_CTRL_D || key.ScanCode == 0x17 ) {
					Print(L"\nGiving up then...\n");
					return  -1;
				}
				if (key.UnicodeChar == CHAR_NULL) break;

				if (pos > size-1) break;

				buffer[pos++] = key.UnicodeChar;

				/* Write the character out */
				Print(L"%c", key.UnicodeChar);
		}
	}
	return 0;
}

static VOID
display_message(VOID)
{
	fops_fd_t fd;
	EFI_STATUS status;
	UINTN len, i;
	CHAR16 *filename;
	CHAR8 buf[256];

	if ((filename = get_message_filename(0)) == NULL) return;

	if (*filename == CHAR_NULL) return;

	VERB_PRT(3, Print(L"opening message file %s\n", filename));

	status = fops_open(filename, &fd);
	if (EFI_ERROR(status)) {
		VERB_PRT(3, Print(L"message file %s not found\n", filename));
		return;
	}
	len = 256;
	Print(L"\n");
	while ((status = fops_read(fd, buf, &len)) == EFI_SUCCESS) {
		/* XXX: ugly ! */
		for (i=0; i < len; i++) {
			Print(L"%c", (CHAR16)buf[i]);
		}
		if (len < 256) break;
	}
	fops_close(fd);
}

static INTN
simple_choose(CHAR16 **argv, INTN argc, INTN index, CHAR16 *kname, CHAR16 *cmdline)
{	
#	define BOOT_IMG_STR	L"BOOT_IMAGE="
	CHAR16 buffer[CMDLINE_MAXLEN];
	CHAR16 alt_buffer[CMDLINE_MAXLEN];
	CHAR16 initrd_name[PATHNAME_MAXLEN];
	CHAR16 vmcode_name[PATHNAME_MAXLEN];
	CHAR16 args[CMDLINE_MAXLEN];
	CHAR16 devname[PATHNAME_MAXLEN];
	CHAR16 dpath[FILENAME_MAXLEN];
	CHAR16 *slash_pos, *colon_pos, *backslash_pos;
	UINTN len;
	INTN ret;

	buffer[0] = alt_buffer[0] = CHAR_NULL;

	display_message();

restart:
	initrd_name[0] = vmcode_name[0] = kname[0] = cmdline[0] = args[0] = CHAR_NULL;

	/* reset per image loader options */
	Memset(&elilo_opt.img_opt, 0, sizeof(elilo_opt.img_opt));

	/*
	 * check for alternate kernel image and params in EFI variable
	 */
	if (elilo_opt.alt_check && alternate_kernel(alt_buffer, sizeof(alt_buffer)) == 0) {
		argc     = argify(alt_buffer,sizeof(alt_buffer), argv); 
		alt_argv = argv;
		index    = 0;
		args[0]  = initrd_name[0] = vmcode_name[0] = 0;
		/* 
		 * don't check twice because the variable is deleted after
		 * first access
		 */
		elilo_opt.alt_check = 0; 
	}

	if (elilo_opt.prompt) {
		console_textmode();
		ret = select_kernel(buffer, CMDLINE_MAXLEN);
		if (ret == -1) return -1;
		/* this function takes really the number of bytes ... */
		argc    = argify(buffer,sizeof(buffer), argv); 
		index   = 0;
	}

	/*
	 * if we found an alternate choice and the user
	 * did not force it manually, then use the alternate
	 * option.
	 */
	if (alt_buffer[0] && buffer[0] == CHAR_NULL) {
		StrCpy(buffer, alt_buffer);
	}

	/*
	 * First search for matching label in the config file
	 * if options were specified on command line, they take
	 * precedence over the ones in the config file
	 *
	 * if no match is found, the args and initrd arguments may
	 * still be modified by global options in the config file.
	 */
	ret = find_label((index < argc) ? argv[index] : NULL, kname, args, initrd_name, vmcode_name);

	/*
	 * not found, so assume first argument is kernel name and
	 * not label name 
	 */
	if (ret == -1) {
		if ((index < argc) && argv[index]) 
			StrCpy(kname, argv[index]);
		else
			StrCpy(kname, elilo_opt.default_kernel);
	}
	/*
	 * no matter what happened for kname, if user specified
	 * additional options, they override the ones in the
	 * config file 
	 */
	if (argc > 1+index) {
		/*StrCpy(args, argv[++index]);*/
		while (++index < argc) {
			StrCat(args, L" ");
			StrCat(args, argv[index]);
		}
	}
	/*
	 * if initrd specified on command line, it overrides
	 * the one defined in config file, if any
	 */
	if (elilo_opt.initrd[0] == CHAR_NULL && initrd_name[0] != CHAR_NULL) {
		StrCpy(elilo_opt.initrd, initrd_name);
	}

	if (elilo_opt.vmcode[0] == CHAR_NULL && vmcode_name[0] != CHAR_NULL) {
		StrCpy(elilo_opt.vmcode, vmcode_name);
	}

	VERB_PRT(1,  { Print(L"kernel     is  '%s'\n", kname);
		       Print(L"arguments  are '%s'\n", args);
			if (elilo_opt.initrd[0]) Print(L"initrd      is '%s'\n", elilo_opt.initrd);
			if (elilo_opt.vmcode[0]) Print(L"vmm         is '%s'\n", elilo_opt.vmcode);
		      });

	if (elilo_opt.prompt == 0) {
		/* minimal printing */
		Print(L"ELILO v%s for EFI/%a\n", ELILO_VERSION, ELILO_ARCH);
		ret = wait_timeout(elilo_opt.delay);
		if (ret != 0) {
			elilo_opt.prompt = 1;
			elilo_opt.initrd[0] = elilo_opt.vmcode[0] = CHAR_NULL;
			elilo_opt.timeout =  ELILO_TIMEOUT_INFINITY;
			goto restart;
		}
	}
	/*
	 * add the device name, if not already specified, 
	 * so that we know where we came from
	 */
	slash_pos     = StrChr(kname, L'/');
	backslash_pos = StrChr(kname, L'\\');
	colon_pos     = StrChr(kname, L':');

	if (backslash_pos && backslash_pos < slash_pos) slash_pos = backslash_pos;

	if (colon_pos == NULL || (slash_pos && (slash_pos < colon_pos))) {
		StrCpy(devname, fops_bootdev_name());
		StrCat(devname, L":");

		/* the default path is always terminated with a separator */
		if (kname[0] != L'/' && kname[0] != L'\\') {
			fops_getdefault_path(dpath,FILENAME_MAXLEN); 
			StrCat(devname, dpath);
		}
	} else {
		devname[0] = CHAR_NULL;
	}

	/*
	 * create final argument list to the kernel
	 */
	len = StrLen(BOOT_IMG_STR)	/* BOOT_IMAGE= */
	     +StrLen(devname)		/* device name */
	     				/* kernel name */
	     +elilo_opt.vmcode[0] ? StrLen(elilo_opt.vmcode) : StrLen(kname)
	     +1				/* space */
	     +StrLen(args);		/* args length */

	if (len >= CMDLINE_MAXLEN-1) {
		ERR_PRT((L" arguments list too long cannot fit BOOT_IMAGE\n"));
		return -1;
	}
	StrCpy(cmdline, L"BOOT_IMAGE=");
	StrCat(cmdline, devname);
	if (elilo_opt.vmcode[0])
		StrCat(cmdline, elilo_opt.vmcode);
	else
		StrCat(cmdline, kname);
	StrCat(cmdline, L" ");
	StrCat(cmdline, args);

	return 0;
}

static INTN
simple_probe(EFI_HANDLE dev)
{
	/* this chooser always work */
	return 0;
}

chooser_t simple_chooser={
	L"simple",
	simple_probe,
	simple_choose
};

