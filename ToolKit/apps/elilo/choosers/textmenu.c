/* 
 *  Copyright (C) 2001-2003 Hewlett-Packard Co.
 *	Contributed by Richard Hirst <rhirst@linuxcare.com>
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
#include "console.h"

#define MAX_LABELS	64
#define MSGBUFLEN	4096

static UINT8 msgbuf[MSGBUFLEN];
static CHAR16 *labels[MAX_LABELS];
static CHAR16 *descriptions[MAX_LABELS];
static INTN nlabels;
static INTN CursorRow, CursorCol, PromptRow, PromptCol;
static INTN MenuRow, MenuCol, MenuWidth, MenuHeight;
static INTN DisplayParsed, CurrentAttr, PromptAttr;
static INTN PromptWidth, MenuHiAttr, MenuLoAttr;
static INTN PromptLen, MenuActive, MenuFirst;
static CHAR16 PromptBuf[CMDLINE_MAXLEN];

#define DEF_ATTR	EFI_TEXT_ATTR(EFI_LIGHTGRAY,EFI_BLACK)


#define ClearScreen()	ST->ConOut->ClearScreen (ST->ConOut)
#define SetTextAttr(a)	ST->ConOut->SetAttribute (ST->ConOut, a)

static INTN
tohex(INTN c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	else if (c >= 'A' && c <= 'F')
		return c = c - 'A' + 10;
	else if (c >= 'a' && c <= 'f')
		return c = c - 'a' + 10;
	else
		return 16;
}

static VOID
paint_msg(UINT8 *msg)
{
	INTN c;

	CursorCol = CursorRow = 0;
	CurrentAttr = DEF_ATTR;
	SetTextAttr(CurrentAttr);
	ClearScreen();
	while ((c = *msg++)) {
		/* First map VGA to EFI line drawing chars */
		if      (c == 0xda) c = BOXDRAW_DOWN_RIGHT;
		else if (c == 0xc4) c = BOXDRAW_HORIZONTAL;
		else if (c == 0xbf) c = BOXDRAW_DOWN_LEFT;
		else if (c == 0xb3) c = BOXDRAW_VERTICAL;
		else if (c == 0xd9) c = BOXDRAW_UP_LEFT;
		else if (c == 0xc0) c = BOXDRAW_UP_RIGHT;
		else if (c == 0xb4) c = BOXDRAW_VERTICAL_LEFT;
		else if (c == 0xc3) c = BOXDRAW_VERTICAL_RIGHT;
		else if (c == 0x1e) c = GEOMETRICSHAPE_UP_TRIANGLE;
		else if (c == 0x1f) c = GEOMETRICSHAPE_DOWN_TRIANGLE;
		else if (c > 0x7f)  c = '?';

		/* Now print the printable chars, then process controls */
		if (c >= ' ') {
			Print(L"%c", c);
			CursorCol++;
		}
		else {
			switch (c) {
			case '\r':		/* Ignore CR */
				break;
			case '\n':		/* LF treated as cr/lf */
				CursorRow++;
				CursorCol = 0;
				Print(L"\n");
				break;
			case 0x0c:		/* FF - Clear screen */
				CursorCol = CursorRow = 0;
				ClearScreen();
				break;
			case 0x0f:		/* ^O - Attributes */
				if (msg[0] && msg[1]) {
					INTN bg = tohex(*msg++);
					INTN fg = tohex(*msg++);

					if (bg < 16 || fg < 16) {
						CurrentAttr = EFI_TEXT_ATTR(fg, bg); 
						SetTextAttr(CurrentAttr);
					}
				}
				break;
			case 0x01:		/* ^A - Prompt */
				if (!DisplayParsed) {
					if (!PromptRow) {
						PromptRow = CursorRow;
						PromptCol = CursorCol;
						PromptAttr = CurrentAttr;
					}
					else if (!PromptWidth)
						PromptWidth = CursorCol - PromptCol;
					/* else bad syntax */
				}
				break;
			case 0x02:		/* ^B - Menu */
				if (!DisplayParsed) {
					if (!MenuRow) {
						MenuRow = CursorRow;
						MenuCol = CursorCol;
						MenuLoAttr = CurrentAttr;
					}
					else if (!MenuWidth) {
						MenuWidth = CursorCol - MenuCol;
						MenuHeight = CursorRow - MenuRow + 1;
						MenuHiAttr = CurrentAttr;
					}
					/* else bad syntax */
				}
				break;
			default:
				Print(L"?");
				CursorCol++;
			}
		}
	}
}


static VOID
paint_prompt(VOID)
{
	INTN offset = PromptLen > PromptWidth - 1 ? PromptLen - PromptWidth + 1: 0;
	SetTextAttr(PromptAttr);
	PrintAt(PromptCol, PromptRow, L"%s%s", PromptBuf + offset, L" \b");
	SetTextAttr(CurrentAttr);
}

static VOID
paint_menu(VOID)
{
	INTN i, j;

	for (i = 0; i < MenuHeight; i++) {
		INTN attr = (i + MenuFirst == MenuActive) ? MenuHiAttr: MenuLoAttr;
		CHAR16 description[80];

		for (j = 0; j < MenuWidth; j++)
			description[j] = ' ';
		description[MenuWidth] = '\0';
		if (i + MenuFirst < nlabels) {
			for (j = 0; descriptions[i + MenuFirst][j] && j < MenuWidth; j++)
				description[j+1] = descriptions[i + MenuFirst][j];
		}
		SetTextAttr(attr);
		PrintAt(MenuCol, MenuRow + i, L"%-.*s", MenuWidth, description);
		SetTextAttr(CurrentAttr);
	}
	paint_prompt();
}

static INTN
read_message_file(INTN msg, UINT8 *buf, UINTN max)
{
	CHAR16 *filename;
	fops_fd_t message_fd;
	EFI_STATUS status;
	UINTN len = max;

	if (msg > 10) return 0;

	if ((filename = get_message_filename(msg)) == NULL) {
		VERB_PRT(3, Print(L"no message file specified\n"));
		return 0;
	}

	VERB_PRT(3, Print(L"opening message file %s\n", filename));

	status = fops_open(filename, &message_fd);
	if (EFI_ERROR(status)) {
		VERB_PRT(3, Print(L"message file %s not found\n", filename));
		return 0;
	}

	status = fops_read(message_fd, buf, &len);
	if (EFI_ERROR(status)) {
		VERB_PRT(3, Print(L"Error reading message file\n"));
		len = 0;
	}

	fops_close(message_fd);

	VERB_PRT(3, Print(L"done reading message file %s\n", filename));

	return len;
}


/*
 * interactively select a kernel image and options.
 * The kernel can be an actual filename or a label in the config file
 * Return:
 * 	-1: if unsucessful
 * 	 0: otherwise
 */
static INTN
select_kernel(CHAR16 *label, INTN lsize)
{
#define CHAR_CTRL_C	(L'\003') /* Unicode CTRL-C */
#define CHAR_CTRL_D	(L'\004') /* Unicode CTRL-D */
#define CHAR_CTRL_F	(L'\006') /* Unicode CTRL-F */
#define CHAR_DEL	(L'\177') /* Unicode DEL */
	SIMPLE_INPUT_INTERFACE *ip = systab->ConIn;
	EFI_INPUT_KEY key;
	EFI_STATUS status;
	INT8 first_time = 1;
	INTN i;
	INT8 fn = 0;

reprint:
	i = read_message_file(0, msgbuf, MSGBUFLEN-1);
	msgbuf[i] = 0;
	paint_msg(msgbuf);
	DisplayParsed = 1;
	paint_menu();
        CurrentAttr = PromptAttr;
	SetTextAttr(CurrentAttr);

	for (;;) {
		while ((status = ip->ReadKeyStroke (ip, &key)) 
			== EFI_NOT_READY);
		if (EFI_ERROR(status)) {
			SetTextAttr(EFI_TEXT_ATTR(EFI_LIGHTGRAY,EFI_BLACK));
			ClearScreen();
			ERR_PRT((L"select_kernel readkey: %r", status));
			return -1;
		} 
		if (key.UnicodeChar == CHAR_CTRL_F) {
			fn = 1;
			continue;
		}
		if (fn) {
			if (key.UnicodeChar >= '0' && key.UnicodeChar <= '9') {
				if (key.UnicodeChar == '0')
					key.ScanCode = SCAN_F10;
				else
					key.ScanCode = SCAN_F1 + key.UnicodeChar - '1';
				key.UnicodeChar = 0;
			}
			fn = 0;
		}
		if (key.ScanCode == SCAN_UP) {
			if (MenuActive)
				MenuActive--;
			else
				continue;
			if (MenuActive < MenuFirst)
				MenuFirst = MenuActive;
			paint_menu();
			continue;
		}
		else if (key.ScanCode == SCAN_DOWN) {
			if (MenuActive < nlabels - 1)
				MenuActive++;
			else
				continue;
			if (MenuActive >= MenuFirst + MenuHeight)
				MenuFirst = MenuActive - MenuHeight + 1;
			paint_menu();
			continue;
		}
		else if (key.ScanCode >= SCAN_F1 && key.ScanCode <= SCAN_F10) {
			i = read_message_file(key.ScanCode - SCAN_F1 + 1, msgbuf, MSGBUFLEN-1);
			if (i) {
				msgbuf[i] = 0;
				paint_msg(msgbuf);
				while ((status= ip->ReadKeyStroke (ip, &key)) == EFI_NOT_READY);
				goto reprint;
			}
		}

		switch (key.UnicodeChar) {
			/* XXX Do we really want this in textmenual mode? */
			case L'?':
				Print(L"\n");
				print_devices();
				first_time = 0;
				goto reprint;
			case CHAR_BACKSPACE:
			case CHAR_DEL:
				if (PromptLen == 0) break;
				PromptLen--;
				PromptBuf[PromptLen] = 0;
				if (PromptLen >= PromptWidth-2)
					paint_prompt();
				else
					Print(L"\b \b");
				break;

			case CHAR_LINEFEED:
			case CHAR_CARRIAGE_RETURN:
				StrCpy(label, labels[MenuActive]);
				SetTextAttr(EFI_TEXT_ATTR(EFI_LIGHTGRAY,EFI_BLACK));
				ClearScreen();
				return 0;

			default:
				if ( key.UnicodeChar == CHAR_CTRL_D
						|| key.UnicodeChar == CHAR_CTRL_C) {
					SetTextAttr(EFI_TEXT_ATTR(EFI_LIGHTGRAY,EFI_BLACK));
					ClearScreen();
					Print(L"\nGiving up then...\n");
					return  -1;
				}
				if (key.UnicodeChar == CHAR_NULL) break;

				if (PromptLen > CMDLINE_MAXLEN-1) break;

				if (key.UnicodeChar < ' ' || key.UnicodeChar > 0x7e)
					key.UnicodeChar = '?';
				PromptBuf[PromptLen++] = key.UnicodeChar;
				PromptBuf[PromptLen]   = 0;

				/* Write the character out */
				if (PromptLen >= PromptWidth-1)
					paint_prompt();
				else
					Print(L"%c", key.UnicodeChar);
		}
	}
	return 0;
}

INTN
textmenu_choose(CHAR16 **argv, INTN argc, INTN index, CHAR16 *kname, CHAR16 *cmdline)
{	
#	define BOOT_IMG_STR	L"BOOT_IMAGE="
	CHAR16 label[CMDLINE_MAXLEN];
	CHAR16 initrd_name[PATHNAME_MAXLEN];
	CHAR16 vmcode_name[PATHNAME_MAXLEN];
	CHAR16 args[CMDLINE_MAXLEN];
	CHAR16 devname[PATHNAME_MAXLEN];
	CHAR16 dpath[FILENAME_MAXLEN];
	CHAR16 *slash_pos, *colon_pos, *backslash_pos;
	UINTN len;
	INTN ret;
	VOID *handle = NULL;

	/* Clear all static variables, as we might be called more than once */

	CursorRow = CursorCol = PromptRow = PromptCol = 0;
	MenuRow = MenuCol = MenuWidth = MenuHeight = 0;
	DisplayParsed = CurrentAttr = PromptAttr = 0;
	PromptWidth = MenuHiAttr = MenuLoAttr = 0;
	PromptLen = MenuActive = MenuFirst = 0;
	PromptBuf[0] = CHAR_NULL;

	nlabels = 0;
	while (nlabels < MAX_LABELS && (handle = get_next_description(handle, labels + nlabels, descriptions + nlabels))) {
		if (descriptions[nlabels][0] == 0)
			descriptions[nlabels] = labels[nlabels];
		nlabels++;
	}
restart:
	vmcode_name[0] = initrd_name[0] = kname[0] = cmdline[0] = args[0] = CHAR_NULL;

	/* reset per image loader options */
	Memset(&elilo_opt.img_opt, 0, sizeof(elilo_opt.img_opt));

	if (elilo_opt.prompt) {
		console_textmode();
		ret = select_kernel(label, sizeof(label));
		if (ret == -1) return -1;
		argc    = argify(PromptBuf,sizeof(PromptBuf), argv); 
		index   = 0;
	}

	/*
	 * check for alternate kernel image and params in EFI variable
	 */
	if (elilo_opt.alt_check && alternate_kernel(PromptBuf, sizeof(PromptBuf)) == 0) {
		argc    = argify(PromptBuf,sizeof(PromptBuf), argv); 
		index   = 0;
		label[0] = args[0] = initrd_name[0] = vmcode_name[0] = 0;
	}

	/*
	 * First search for matching label in the config file
	 * if options were specified on command line, they take
	 * precedence over the ones in the config file
	 *
	 * if no match is found, the args and initrd arguments may
	 * still be modified by global options in the config file.
	 */
	if (label[0])
		ret = find_label(label, kname, args, initrd_name, vmcode_name);
	else
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
	if (label[0])
		index--;
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
	     +StrLen(kname)		/* kernel name */
	     +elilo_opt.vmcode[0] ? StrLen(elilo_opt.vmcode) : StrLen(kname)
	     +1				/* space */
	     +StrLen(args);		/* args length */

	if (len >= CMDLINE_MAXLEN-1) {
		SetTextAttr(EFI_TEXT_ATTR(EFI_LIGHTGRAY,EFI_BLACK));
		ClearScreen();
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

	VERB_PRT(3, Print(L"final command line is '%s'\n", cmdline));

	return 0;
}

static INTN
textmenu_probe(EFI_HANDLE dev)
{
	/* this chooser always works */
	return 0;
}

chooser_t textmenu_chooser={
	L"textmenu",
	textmenu_probe,
	textmenu_choose
};

