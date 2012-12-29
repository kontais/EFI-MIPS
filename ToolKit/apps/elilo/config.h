/*
 *  Copyright (C) 2001-2003 Hewlett-Packard Co.
 *	Contributed by Stephane Eranian <eranian@hpl.hp.com>
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

#ifndef __ELILO_CONFIG_H__
#define __ELILO_CONFIG_H__

#define opt_offsetof(opt)	(&((boot_image_t *)(0x0))->opt)

typedef enum { OPT_NOTUSED, OPT_STR, OPT_CMD, OPT_BOOL, OPT_NUM, OPT_FILE } config_option_type_t;

typedef enum { OPT_GLOBAL, 	/* global and shared among architectures */
	       OPT_IMAGE,	/* per image only and shared among architectures */
	       OPT_IMAGE_SYS	/* per image and architecture specific (offset base in sys_img_opts) */
} config_option_scope_t;

typedef enum { OPTIONS_GROUP_GLOBAL, 	/* group must be added to global set of options */
	       OPTIONS_GROUP_IMAGE,	/* group must be added to per-image set of options */
} config_option_group_scope_t;

struct _config_option_t;
typedef INTN option_action_t(struct _config_option_t *, VOID *);

typedef struct _config_option_t {
    config_option_type_t	type;	/* option type: OPT_CMD, OPT_BOOL, ... */
    config_option_scope_t	scope;	/* option scope: global, per image, per image_sys */
    CHAR16	   		*name;	/* unicode string for the option */
    option_action_t		*action; /* option specific action */
    INTN	   		(*check)(void *);	/* check valid arguments, NULL=don't need */
    VOID	   		*data;	/* which data structure to record the option */
} config_option_t;

extern INTN register_config_options(config_option_t *opt, UINTN nentries, config_option_group_scope_t);

#endif /* __ELILO_CONFIG_H__ */
