/*
 *  Copyright (C) 2001-2003 Hewlett-Packard Co.
 *	Contributed by Stephane Eranian <eranian@hpl.hp.com>
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
 *
 * Portions of this file are derived from the  LILO/x86
 * Copyright 1992-1997 Werner Almesberger. 
 */

#include <efi.h>
#include <efilib.h>
#include <efistdarg.h>

#include "elilo.h"
#include "config.h"

/*
 * The first default config file is architecture dependent. This is useful
 * in case of network booting where the server is used for both types of
 * architectures.
 */
#if defined(CONFIG_ia64)
#define ELILO_ARCH_DEFAULT_CONFIG	L"elilo-ia64.conf"
#elif defined (CONFIG_Mips32)
#define ELILO_ARCH_DEFAULT_CONFIG	L"elilo-ia32.conf"
#elif defined (CONFIG_x86_64)
#define ELILO_ARCH_DEFAULT_CONFIG       L"elilo-x86_64.conf"
#else
#error "You need to specfy your default arch config file"
#endif

/* 
 * last resort config file. Common to all architectures
 */
#define ELILO_DEFAULT_CONFIG	L"elilo.conf"

#define MAX_STRING	512
#define CONFIG_BUFSIZE	512	/* input buffer size */

/*
 * maximum number of message files.
 *
 * main message= goes at entry 0, entries [1-12] are used for function keys
 *
 */
#define MAX_MESSAGES	13

typedef struct boot_image {
	struct boot_image *next;
	CHAR16	label[MAX_STRING];
	CHAR16	kname[FILENAME_MAXLEN];
	CHAR16  options[CMDLINE_MAXLEN];
	CHAR16	initrd[FILENAME_MAXLEN];
	CHAR16	vmcode[FILENAME_MAXLEN];
	CHAR16	root[FILENAME_MAXLEN];
	CHAR16	fallback[MAX_STRING];
	CHAR16	description[MAX_STRING];

	UINTN	ramdisk;
	UINTN	readonly;
	UINTN	literal;

	sys_img_options_t sys_img_opts;	/* architecture specific image options */
} boot_image_t;

typedef enum {
	TOK_ERR,
	TOK_EQUAL,
	TOK_STR,
	TOK_EOF
} token_t;

/*
 * global shared options
 * architecture specific global options are private to each architecture
 */
typedef struct {
	CHAR16		root[FILENAME_MAXLEN];	/* globally defined root fs */
	CHAR16		initrd[FILENAME_MAXLEN];/* globally defined initrd  */
	CHAR16		vmcode[FILENAME_MAXLEN];/* globally defined boot-time module  */
	CHAR16		options[CMDLINE_MAXLEN];
	CHAR16		default_image_name[MAX_STRING];
	CHAR16		message_file[MAX_MESSAGES][FILENAME_MAXLEN]; 
	CHAR16		chooser[FILENAME_MAXLEN];/* which image chooser to use */
	CHAR16		config_file[FILENAME_MAXLEN];
	boot_image_t	*default_image;

	UINTN		readonly;

	/* 
	 * options that may affect global elilo options
	 */
	UINTN alt_check;
	UINTN debug;
	UINTN delay;
	UINTN prompt;
	UINTN timeout;
	UINTN verbose;
	UINTN edd30_no_force; /* don't force EDD30 if not set */
} global_config_t;

/*
 * structure used to point to a group of options.
 * Several group for the same category are supported via a linked list.
 */
typedef struct _config_option_group {
	struct _config_option_group	*next;	  /* pointer to next group */
	config_option_t			*options; /* the table of options for this group */
	UINTN				nentries; /* number of entries for this group */
} config_option_group_t;

static option_action_t do_image, do_literal, do_options;
static INTN check_verbosity(VOID *), check_chooser(VOID *);

static global_config_t global_config;	/* options shared by all images */

/*
 * Core global options: shared by all architectures, all modules
 */
static config_option_t global_common_options[]={
{OPT_STR,	OPT_GLOBAL, 	L"default",	NULL,		NULL,			global_config.default_image_name},
{OPT_NUM,	OPT_GLOBAL,	L"timeout",	NULL, 		NULL,			&global_config.timeout},
{OPT_NUM,	OPT_GLOBAL,	L"delay",	NULL,		NULL,			&global_config.delay},
{OPT_BOOL,	OPT_GLOBAL,	L"debug",	NULL,		NULL,			&global_config.debug},
{OPT_BOOL,	OPT_GLOBAL,	L"prompt",	NULL,		NULL,			&global_config.prompt},
{OPT_NUM,	OPT_GLOBAL,	L"verbose",	NULL,		check_verbosity,	&global_config.verbose},
{OPT_FILE,	OPT_GLOBAL,	L"root",	NULL,		NULL,			global_config.root},
{OPT_BOOL,	OPT_GLOBAL,	L"read-only",	NULL,		NULL,			&global_config.readonly},
{OPT_BOOL,	OPT_GLOBAL,	L"noedd30",	NULL,		NULL,			&global_config.edd30_no_force},
{OPT_CMD,	OPT_GLOBAL,	L"append",	NULL,		NULL,			global_config.options},
{OPT_FILE,	OPT_GLOBAL,	L"initrd",	NULL,		NULL,			global_config.initrd},
{OPT_FILE,	OPT_GLOBAL,	L"vmm",		NULL,		NULL,			global_config.vmcode},
{OPT_FILE,	OPT_GLOBAL,	L"image",	do_image,	NULL,			opt_offsetof(kname)},
{OPT_BOOL,	OPT_GLOBAL,	L"checkalt",	NULL,		NULL,			&global_config.alt_check},
{OPT_STR,	OPT_GLOBAL,	L"chooser",	NULL,		check_chooser,		global_config.chooser},
{OPT_FILE,	OPT_GLOBAL,	L"message",	NULL,		NULL,			global_config.message_file[0]},
{OPT_FILE,	OPT_GLOBAL,	L"f1",		NULL,		NULL,			global_config.message_file[1]},
{OPT_FILE,	OPT_GLOBAL,	L"f2",		NULL,		NULL,			global_config.message_file[2]},
{OPT_FILE,	OPT_GLOBAL,	L"f3",		NULL,		NULL,			global_config.message_file[3]},
{OPT_FILE,	OPT_GLOBAL,	L"f4",		NULL,		NULL,			global_config.message_file[4]},
{OPT_FILE,	OPT_GLOBAL,	L"f5",		NULL,		NULL,			global_config.message_file[5]},
{OPT_FILE,	OPT_GLOBAL,	L"f6",		NULL,		NULL,			global_config.message_file[6]},
{OPT_FILE,	OPT_GLOBAL,	L"f7",		NULL,		NULL,			global_config.message_file[7]},
{OPT_FILE,	OPT_GLOBAL,	L"f8",		NULL,		NULL,			global_config.message_file[8]},
{OPT_FILE,	OPT_GLOBAL,	L"f9",		NULL,		NULL,			global_config.message_file[9]},
{OPT_FILE,	OPT_GLOBAL,	L"f10",		NULL,		NULL,			global_config.message_file[10]},
{OPT_FILE,	OPT_GLOBAL,	L"f11",		NULL,		NULL,			global_config.message_file[11]},
{OPT_FILE,	OPT_GLOBAL,	L"f12",		NULL,		NULL,			global_config.message_file[12]}
};

static config_option_t image_common_options[]={
    {OPT_FILE,	OPT_IMAGE,	L"root",	NULL,		NULL,	opt_offsetof(root)},
    {OPT_BOOL,	OPT_IMAGE,	L"read-only",	NULL,		NULL,	opt_offsetof(readonly)},
    {OPT_CMD,	OPT_IMAGE,	L"append",	do_options,	NULL,	opt_offsetof(options)},
    {OPT_CMD,	OPT_IMAGE,	L"literal",	do_literal,	NULL,	NULL},
    {OPT_FILE,	OPT_IMAGE,	L"initrd",	NULL,		NULL,	opt_offsetof(initrd)},
    {OPT_FILE,	OPT_IMAGE,	L"vmm",		NULL,		NULL,	opt_offsetof(vmcode)},
    {OPT_STR,	OPT_IMAGE,	L"label",	NULL,		NULL,	opt_offsetof(label)},
    {OPT_FILE,	OPT_IMAGE,	L"image",	do_image,	NULL,	opt_offsetof(kname)},
    {OPT_STR,	OPT_IMAGE,	L"description",	NULL,		NULL,	opt_offsetof(description)},
};

#define OPTION_IS_GLOBAL(p)	((p)->scope == OPT_GLOBAL)
#define OPTION_IS_IMG_SYS(p)	((p)->scope == OPT_IMAGE_SYS)

#define CHAR_EOF	(CHAR16)-1	/* Unicode version of EOF */
#define CHAR_NUM0	L'0'
#define CHAR_NUM9	L'9'

static UINTN line_num;
static INTN back; /* can go back by one char */

static config_option_group_t *global_option_list;
static config_option_group_t *image_option_list;


static config_option_group_t *current_options;

static boot_image_t *image_list, *first_image;
static boot_image_t *current_img;

static INT8 config_buf[CONFIG_BUFSIZE];	/* input buffer: file must be in ASCII! */
static UINTN buf_max, buf_pos;

static fops_fd_t config_fd;

static VOID 
config_error(CHAR16 *msg,...)
{
    Print(L"near line %d: ",line_num);
    IPrint(systab->ConOut, msg);
    Print(L"\n");
}

/*
 * low level read routine
 * Return:
 *
 * Success:
 * 	- the next available unicode character
 * Error:
 * 	- CHAR_EOF : indicating error or EOF
 *
 * XXX: we suppose that the file is in ASCII format!
 */
static CHAR16
getc(VOID)
{
	EFI_STATUS status;

	if (buf_pos == 0 || buf_pos == buf_max) {
		buf_max = CONFIG_BUFSIZE;
		status = fops_read(config_fd, config_buf, &buf_max);
		if (EFI_ERROR(status) || buf_max == 0) return CHAR_EOF;

		buf_pos  = 0;
	}
	return (CHAR16)config_buf[buf_pos++];
}


/*
 * get the next unicode character with a one character
 * rewind buffer.
 */
static CHAR16
next(VOID)
{
    CHAR16 ch;

    if (back) {
	ch = back;
	back = 0;
	return ch;
    }
/*
 * config files served from pxe/tftpboot windows servers can contain
 * extraneous '\r' characters, often the first char in the file, and
 * we need to simply skip over those and continue on
 */
   ch = getc();
   if(ch == '\r')
	ch = getc();

   return ch;
}

/*
 * rewind by one character
 */
static VOID
again(CHAR16 ch)
{
    if (back) { config_error(L"config: again invoked twice"); }
    back = ch;
}

/*
 * Look for matching option in the current group
 *
 * Return:
 * 	- pointer to option if found
 * 	- NULL if not found
 */
static config_option_t *
find_option(config_option_group_t *grp, CHAR16 *str)
{
	config_option_t *p = NULL;
	config_option_t *end;

	while(grp) {
		p = grp->options;
		end = grp->options+grp->nentries;

		while (p != end) {
			if (!StrCmp(str, p->name)) return p;
			p++;
		}
		grp = grp->next;
	}
	return NULL;
}

/*
 * main parser function
 * Return:
 * 	- a token code representing the kind of element found
 * 	- TOK_EOF: end-of-file (or error) detected
 * 	- TOK_STR: if string is found
 * 	- TOK_EQUAL: for the '=' sign
 * 	- TOK_ERR: in case of (parsing) error
 */
static token_t
get_token_core(CHAR16 *str, UINTN maxlen, BOOLEAN rhs)
{
    INTN ch, escaped;
    CHAR16 *here;

    for (;;) {
	while ((ch = next()), ch == ' ' || ch == '\t' || ch == '\n') if (ch == '\n' ) line_num++;

	if (ch == CHAR_EOF) return TOK_EOF;

	/* skip comment line */
	if (ch != '#') break;

	while ((ch = next()), ch != '\n') if (ch == CHAR_EOF) return TOK_EOF;
	line_num++;
    }
    if (ch == '=' && !rhs) return TOK_EQUAL;

    if (ch == '"') {
	here = str;
	while (here-str < maxlen) {
	    if ((ch = next()) == CHAR_EOF) {
		    config_error(L"EOF in quoted string");
		    return TOK_ERR;
	    }
	    if (ch == '"') {
		*here = 0;
		return TOK_STR;
	    }
	    if (ch == '\\') {
		ch = next();
		if (ch != '"' && ch != '\\' && ch != '\n') {
		    config_error(L"Bad use of \\ in quoted string");
		    return TOK_ERR;
		}
		if (ch == '\n') continue;
#if 0
		    while ((ch = next()), ch == ' ' || ch == '\t');
		    if (!ch) continue;
		    again(ch);
		    ch = ' ';
		}
#endif
	    }
	    if (ch == '\n' || ch == '\t') {
		config_error(L"\\n and \\t are not allowed in quoted strings");
		return TOK_ERR;
	    }
	    *here++ = ch;
	}
	config_error(L"Quoted string is too long");
	return TOK_ERR;
    }

    here    = str;
    escaped = 0;

    while (here-str < maxlen) {
	if (escaped) {
	    if (ch == CHAR_EOF) {
		    config_error(L"\\ precedes EOF");
		    return TOK_ERR;
	    }
	    if (ch == '\n') line_num++;
	    else *here++ = ch == '\t' ? ' ' : ch;
	    escaped = 0;
	}
	else {
	    if (ch == ' ' || ch == '\t' || ch == '\n' || ch == '#' ||
	      ch == CHAR_EOF || (ch == '=' && !rhs)) {
		again(ch);
		*here = 0;
		return TOK_STR;
	    }
	    if (!(escaped = (ch == '\\'))) *here++ = ch;
	}
	ch = next();
    }
    config_error(L"Token is too long");
    return TOK_ERR;
}

static token_t
get_token(CHAR16 *str, UINTN maxlen)
{
    return get_token_core(str, maxlen, FALSE);
}

static token_t
get_token_rhs(CHAR16 *str, UINTN maxlen)
{
    return get_token_core(str, maxlen, TRUE);
}

static INTN
image_check(boot_image_t *img)
{
	boot_image_t *b;

	if (img == NULL) return -1;

	/* do the obvious first */
	if (img->label[0] == '\0') {
		config_error(L"image has no label");
		return -1;
	}

	/* image_list points to the */
	for(b=image_list; b; b = b->next) {
		if (img == b) continue;
		if (!StrCmp(img->label, b->label)) {
			config_error(L"image with label %s already defined", img->label);
			return -1;
		}
	}
	return 0;
}

static INTN 
global_check(VOID)
{
	return 0;
}

static INTN
final_check(VOID)
{
	boot_image_t *b;

	if (global_config.default_image_name[0]) {
		for(b=image_list; b; b = b->next) {
			if (!StrCmp(b->label, global_config.default_image_name)) goto found;
		}
		config_error(L"default image '%s' not defined ", global_config.default_image_name);
		return -1;
	}
	global_config.default_image = first_image;
	return 0;
found:
	global_config.default_image = b;
	return 0;
}

/*
 * depending on the active set of options
 * adjust the option data pointer to:
 * if global option set:
 * 	- just the straight value from p->data
 * if image option set:
 * 	- adjust as offset to image
 * Return:
 * 	- the adjusted pointer
 */
static inline VOID *
adjust_pointer(config_option_t *p)
{
	/*
	 * adjust pointer
	 */
	if (OPTION_IS_GLOBAL(p)) return p->data;

	if (OPTION_IS_IMG_SYS(p)) return (VOID *)((UINTN)&current_img->sys_img_opts + p->data);

	return (VOID *)((UINTN)current_img + p->data);
}

/*
 * create a new image entry
 */
static INTN
do_image(config_option_t *p, VOID *str)
{
	boot_image_t *img;
	
	/*
	 * if we move to a new image from the current one
	 * then we need to check for validity of current.
	 *
	 * if this is the first image, then check the global
	 * options.
	 */
	if (current_img) {
		if (image_check(current_img) == -1) return -1;
	} else if (global_check() == -1) return -1;

	img = (boot_image_t *)alloc(sizeof(boot_image_t), EfiLoaderData);
	if (img == NULL) return -1;

	Memset(img, 0, sizeof(boot_image_t));

	DBG_PRT((L"must do image on %s", (CHAR16 *)str));

	/* copy kernel file name */
	StrCpy(img->kname, str);

	/* switch to image mode */
	current_options = image_option_list;

	/* keep track of first image in case no default is defined */
	if (image_list == NULL) first_image = img;

	/* append to end of image list, so when a chooser asks for the list
	 * it gets them in the order they were in in the config file
	 */
	if (image_list == NULL)
		image_list = img;
	else {
		boot_image_t * p = image_list;

		while (p->next)
			p = p->next;
		p->next = img;
	}

	/* update current image */
	current_img = img;

	return 0;
}

/*
 * by default all boolean options are defined
 * as false. This function sets the boolean
 * to true
 */
static INTN
do_boolean(config_option_t *p)
{
	INT8 *buf;

	buf = adjust_pointer(p);

	if (p->action) return p->action(p, NULL);

	/* set the field to true, overwrite if already defined */
	*buf = 1;

	return 0;
}
	
/*
 * the image option 'literal' requires special handling
 * because it overrides any defined option be it global or
 * local. so we use the option field and record the fact that
 * it should be interpreted as literal
 */
static INTN
do_literal(config_option_t *p, VOID *str)
{
	/*
	 * we know we have a valid current image at this point
	 */
	StrCpy(current_img->options, str);

	current_img->literal = 1;

	return 0;
}

static INTN
do_options(config_option_t *p, VOID *str)
{
	/* we ignore append if literal is already defined */
	if (current_img->literal) return 0;

	/*
	 * we know we have a valid current image at this point
	 */
	StrCpy(current_img->options, str);

	return 0;
}

static INTN
do_numeric(config_option_t *p)
{
	CHAR16 numstr[MAX_STRING];
	CHAR16 *str;
	token_t tok;
	UINTN *buf;
	UINTN tmp;

	/*
	 * match the '=' sign
	 */
	tok = get_token(numstr, MAX_STRING);
	if (tok != TOK_EQUAL) {
		config_error(L"Option %s expects an equal signal + value", p->name);
		return -1;
	}

	/*
	 * now get the value
	 * XXX: = lexer should return TOK_NUM (and we'd be done)
	 */
	tok = get_token(numstr, MAX_STRING);
	if (tok != TOK_STR) {
		config_error(L"Option %s expects a value", p->name);
		return -1;
	}
	str = numstr;
	/*
	 * if there is a customized way of doing the operation
	 * do it and bypass generic
	 */
	if (p->action) return p->action(p, str);

	/*
	 * no field to update
	 */
	if (p->data == NULL) return 0;

	buf = (UINTN *)adjust_pointer(p);

	while (*str && *str >= CHAR_NUM0 && *str <= CHAR_NUM9) str++;
	if (*str) {
		config_error(L"%s is expecting a numeric decimal value", p->name);
		return -1;
	}

	tmp = Atoi(numstr);

	if (p->check && p->check(&tmp) == -1) return -1;

	/*
	 * check for multiple definitions in the same context
	 * XXX: zero must not be a valid number !
	 */
	if (*buf) {
		config_error(L"option %s is already defined in this context", p->name);
		return -1;
	}

	*buf = tmp;

	return 0;
}

static INTN
check_verbosity(VOID *data)
{
	UINTN *val = (UINTN *)data;

	if (*val > 5) {
		config_error(L"Verbosity level must be in [0-5] and not %d", *val);
		return -1;
	}

	return 0;
}

/*
 * here we simply check if chooser is compiled in. At this point, the chooser
 * initialization is not done, so we don't know if that chooser will even be
 * useable.
 */
static INTN
check_chooser(VOID *data)
{
	CHAR16 *chooser = (CHAR16 *)data;

	if (exist_chooser(chooser) == -1) {
		config_error(L"chooser %s is unknown\n", chooser);
		return -1;
	}
	return 0;
}


static INTN
do_string_core(config_option_t *p, CHAR16 *str, UINTN maxlen, CHAR16 *msg)
{
	token_t tok;
	CHAR16 *buf;

	/*
	 * match the '=' sign
	 */
	tok = get_token(str, maxlen);
	if (tok != TOK_EQUAL) {
		config_error(L"Option %s expects an equal signal + %s", p->name, msg);
		return -1;
	}

	/*
	 * now get the value
	 */
	tok = get_token_rhs(str, maxlen);
	if (tok != TOK_STR) {
		config_error(L"Option %s expects %s", p->name, msg);
		return -1;
	}

	/*
	 * if there is a customized way of doing the operation
	 * do it and bypass generic
	 */
	if (p->action) return p->action(p, str);

	/*
	 * no field to update
	 */
	if (p->data == NULL) return 0;

	buf = adjust_pointer(p);

	if (*buf != CHAR_NULL) {
		config_error(L"'%s' already defined", p->name);
		return -1;
	}
	if (p->check && p->check(str) == -1) return -1;

	/*
	 * initialize field
	 */
	StrCpy(buf, str);

	return 0;
}
	
static INTN
do_string(config_option_t *p)
{
	CHAR16 str[MAX_STRING];

	return do_string_core(p, str, MAX_STRING, L"string");
}
	
static INTN
do_file(config_option_t *p)
{
	CHAR16 str[FILENAME_MAXLEN];

	return do_string_core(p, str, FILENAME_MAXLEN, L"filename");
}


static INTN
do_cmd(config_option_t *p)
{
	CHAR16 str[CMDLINE_MAXLEN];
	return do_string_core(p, str, CMDLINE_MAXLEN, L"kernel options");
}


INTN
config_parse(VOID)
{
	CHAR16 str[MAX_STRING];
	INTN ret = -1;
	token_t tok;
	config_option_t *p;

	for (;;) {
		tok = get_token(str, MAX_STRING);

		if (tok == TOK_EOF) break;

		if (tok == TOK_ERR) {
			Print(L"Bad Token from elilo config file, parser read: %s\n elilo exiting\n", str);
			return -1;
		}

		if ( (p = find_option(current_options, str)) == NULL) {
			config_error(L"Unkown option %s", str);
			return -1;
		}

		/* make sure we trap in case of error */
		ret = -1;

		switch(p->type) {
			case OPT_BOOL:
				ret = do_boolean(p);
				break;
			case OPT_STR:
				ret = do_string(p);
				break;
			case OPT_NUM:
				ret = do_numeric(p);
				break;
			case OPT_FILE:
				ret = do_file(p);
				break;
			case OPT_CMD:
				ret = do_cmd(p);
				break;
			default:
				config_error(L"Unkown option type %d", p->type);
		}
		if (ret == -1) goto error;
	}
	if (current_img) {
		ret = image_check(current_img);	
	} else {
		config_error(L"No image defined !");
	}
	if (ret == 0) ret = final_check();
error:
	return ret;
}
	
static VOID
update_elilo_opt(VOID)
{
	/*
	 * update boolean options first
	 * Rule: by default not set unless explcitely requested on command line
	 * therefore we can just update from global_config is set there.
	 */
	if (global_config.alt_check) elilo_opt.alt_check = 1;

	if (global_config.debug) elilo_opt.debug   = 1;
	if (global_config.prompt) elilo_opt.prompt = 1;

	/*
	 * update only if not set on command line
	 * XXX: rely on the existence of a non-valid value as a marker than
	 * the option was not specified on the command line
	 */
	if (global_config.verbose && elilo_opt.verbose == 0) 
		elilo_opt.verbose = global_config.verbose;

	if (global_config.chooser[0] && elilo_opt.chooser[0] == 0)
		StrCpy(elilo_opt.chooser, global_config.chooser);

	/*
	 * if edd30_no_force not set by command line option but via config
	 * file, then propagate
	 */
	if (global_config.edd30_no_force && elilo_opt.edd30_no_force == 0)
		elilo_opt.edd30_no_force = 1;

	/*
	 * Difficult case of numeric where 0 can be a valid value
	 * XXX: find a better way of handling these options!
	 */
	if (global_config.delay && elilo_opt.delay_set == 0)
		elilo_opt.delay = global_config.delay;

	/* readjusted by caller if necessary. 0 not a valid value here */
	elilo_opt.timeout = global_config.timeout;

	/* initrd is updated later when we have an image match */
}

/*
 * When called after read_config(), this function returns the config file
 * used by the loader, or NULL if no file was found.
 */
CHAR16 *
get_config_file(VOID)
{
	return global_config.config_file[0] ? global_config.config_file : NULL; 
}

EFI_STATUS
read_config(CHAR16 *filename)
{
	EFI_STATUS status;
	INTN ret;
	
	if (filename == NULL) return EFI_INVALID_PARAMETER;

	VERB_PRT(3, Print(L"trying config file %s\n", filename));

	StrCpy(global_config.config_file, filename);

	status = fops_open(filename, &config_fd);
	if (EFI_ERROR(status)) {
                VERB_PRT(3, Print(L"cannot open config file %s\n", filename));
                return status;
	}
	/*
	 * start numbering at line 1
	 */
	line_num = 1;

	ret = config_parse();

	fops_close(config_fd);

	DBG_PRT((L"done parsing config file\n"));

	if (ret != 0) return EFI_INVALID_PARAMETER;

	update_elilo_opt();

	return EFI_SUCCESS;
}

VOID
print_label_list(VOID)
{
	boot_image_t *img, *dfl = global_config.default_image;

	if (dfl) Print(L"    %s\n", dfl->label);

	for (img = image_list; img; img = img->next) {
		if (img != dfl) Print(L"    %s\n", img->label);
	}
}

/* Make labels and corresponding descriptions available to choosers.  The
 * idea is that the caller passes NULL for 'prev'; the first time, and
 * passes the previously returned value on subsequent calls.  The caller
 * repeats until this fn returns NULL.
 */

VOID *
get_next_description(VOID *prev, CHAR16 **label, CHAR16 **description)
{
	boot_image_t *img = (boot_image_t *)prev;

	if (img == NULL)
		img = image_list;
	else
		img = img->next;

	if (img) {
		*label = img->label;
		*description = img->description;
		return (void *)img;
	}
	else
		return NULL;
}

/*
 * find a description using the label name
 * return NULL if label not found or no description defined
 */
CHAR16 *
find_description(CHAR16 *label)
{
	boot_image_t *img;

	/* Attempt to find the image name now */
	for (img = image_list; img; img = img->next) {
		if (StriCmp(img->label, label) == 0) {
			return img->description;
		}
	}
	return NULL;
}

static void
add_root_to_options(CHAR16 *options, CHAR16 *root, CHAR16 *vmcode)
{
	CHAR16 *o, ko[CMDLINE_MAXLEN];

	if (vmcode[0]) {
		for (o = options; *o; o++) {
			if (*o == '-' && *(o+1) == '-')
				break;
		}
		if (! *o) {
			/* no separator found, add one */
			StrCpy(o, L" -- ");
			o++;
		}

		/* advance past separator and whitespace */
		o += 2;
		while (*o == ' ') o++;
	} else {
		o = options;
	}

	/* insert root param at this point */
	StrCpy(ko, o);
	StrCpy(o, L"root=");
	StrCat(o, root);
	StrCat(o, L" ");
	StrCat(o, ko);
}

INTN
find_label(CHAR16 *label, CHAR16 *kname, CHAR16 *options, CHAR16 *initrd, CHAR16 *vmcode)
{
	boot_image_t *img;

	if (label == NULL) {
		if (global_config.default_image == NULL) return -1;
		img = global_config.default_image;
		goto found;
	}

	options[0] = 0;

	/* Attempt to find the image name now */
	for (img = image_list; img; img = img->next) {
		if (StriCmp(img->label, label) == 0) {
			goto found;
		}
	}
	/*
	 * when the label does not exist, we still propagate the global options
	 */
	if (global_config.options[0]) {
		StrCat(options, L" ");
		StrCat(options, global_config.options);
	}
	if (global_config.root[0])
		add_root_to_options(options, global_config.root, global_config.vmcode);
	if (global_config.readonly) StrCat(options, L" ro");

	if (global_config.initrd[0]) StrCpy(initrd, global_config.initrd);
	if (global_config.vmcode[0]) StrCpy(vmcode, global_config.vmcode);

	/* make sure we don't get garbage here */
	elilo_opt.sys_img_opts = NULL;

	return -1;
found:
	StrCpy(kname, img->kname);

	/* per image initrd has precedence over global initrd */
	if (img->initrd[0]) 
		StrCpy(initrd, img->initrd);
	else
		StrCpy(initrd, global_config.initrd);

	/* per image vmcode has precedence over global vmcode */
	if (img->vmcode[0]) 
		StrCpy(vmcode, img->vmcode);
	else
		StrCpy(vmcode, global_config.vmcode);

	/*
	 * literal option overrides any other image-based or global option
	 *
	 * In any case, the image option has priority over the global option
	 */
	if (img->literal == 0) {
		/* XXX: check max length */
		if (img->options[0] || global_config.options[0]) {
			StrCat(options, L" ");
			StrCat(options, img->options[0] ? img->options: global_config.options);
		}
		if (img->root[0] || global_config.root[0]) {
			add_root_to_options(options, img->root[0] 
					    ? img->root : global_config.root, vmcode);
		}
		if (img->readonly || global_config.readonly) {
			StrCat(options, L" ro");
		}
	} else {
		/* literal options replace everything */
		StrCpy(options, img->options);
	}

	/*
	 * point to architecture dependent options for this image
	 */
	elilo_opt.sys_img_opts = &img->sys_img_opts;

	DBG_PRT((L"label %s: kname=%s options=%s initrd=%s vmcode=%s", img->label, kname, options, initrd, vmcode));

	return 0;
}

static VOID
print_options(config_option_group_t *grp, BOOLEAN first)
{
	config_option_t *end, *p;
	CHAR16 *str;

	while (grp) {
		p = grp->options;
		end = grp->options+grp->nentries;
		while (p != end) {
			str = NULL;
			switch(p->type) {
				case OPT_BOOL:
					str = L"%s";
					break;
				case OPT_STR :
					str = L"%s=string";
					break;
				case OPT_FILE :
					str = L"%s=filename";
					break;
				case OPT_CMD :
					str = L"%s=kernel_options";
					break;
				case OPT_NUM :
					str = L"%s=number";
					break;
				default: 
					break;
			}
			if (str && first == FALSE) Print(L", ");
			if (str) Print(str, p->name);
			first = FALSE;
			p++;
		}
		grp = grp->next;
	}
}


VOID
print_config_options(VOID)
{
	Print(L"Global options supported:\n");

	print_options(global_option_list, TRUE);
	Print(L"\n\n");

	Print(L"Image options supported:\n");
	print_options(image_option_list, TRUE);
	Print(L"\n");
}


/*
 * returns a pointer to filename used for message N (which). NULL if it does
 * not exist.
 */
CHAR16 *
get_message_filename(INTN which)
{
	if (which < 0 || which >= MAX_MESSAGES) return NULL;
	return global_config.message_file[which];
}

INTN
register_config_options(config_option_t *opt, UINTN n, config_option_group_scope_t group)
{
	config_option_group_t *newgrp, **grp;

	if (opt == NULL || n == 0 || (group != OPTIONS_GROUP_GLOBAL && group != OPTIONS_GROUP_IMAGE)) return -1;

	VERB_PRT(3, Print(L"registering %d options for group %s\n", n, group == OPTIONS_GROUP_GLOBAL ? L"global" : L"image"));

	newgrp = alloc(sizeof(config_option_group_t), EfiLoaderData);
	if (newgrp == NULL) return -1;

	grp = group == OPTIONS_GROUP_GLOBAL ? &global_option_list : &image_option_list;

	if (*grp) while ((*grp)->next) grp = &(*grp)->next;

	newgrp->options  = opt;
	newgrp->next     = NULL;
	newgrp->nentries = n;

	if (*grp) { 
		(*grp)->next = newgrp;
	} else {
		*grp = newgrp;	
	}
	return 0;
}

/*
 * initialize config options: register global and per image options
 */
INTN
config_init(VOID)
{
	INTN ret;

	ret = register_config_options(global_common_options, 
		  	              sizeof(global_common_options)/sizeof(config_option_t),
				      OPTIONS_GROUP_GLOBAL);
	if (ret == -1) return -1;

	ret = register_config_options(image_common_options, 
				      sizeof(image_common_options)/sizeof(config_option_t),
				      OPTIONS_GROUP_IMAGE);

	current_options = global_option_list;

	return ret;
}

