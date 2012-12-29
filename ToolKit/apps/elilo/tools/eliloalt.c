/*
 * eliloalt.c
 *
 * Copyright (C) 2002-2003 Hewlett-Packard Co
 * Contributed by Stephane Eranian <eranian@hpl.hp.com>
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
 */

/*
 * This program is used to set the EliloAlt EFI variable to influence
 * how elilo will behave at the next reboot. This variable is used
 * to boot a certain kernel/configuration only once (debug, for instance).
 *
 * This is is supposed to be installed in /usr/sbin/eliloalt and must only
 * be run by root.
 */
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <fcntl.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <dirent.h>


#define ELILOALT_VERSION	"0.02"

#define ELILO_ALT_NAME	"EliloAlt"
#define EFIVAR_DIR	"/sys/firmware/efi/vars"
#define OFIVAR_DIR	"/proc/efi/vars"
#define ELILO_ALTVAR	EFIVAR_DIR"/"ELILO_ALT_NAME"-00000000-0000-0000-0000-000000000000"
#define OLILO_ALTVAR	OFIVAR_DIR"/"ELILO_ALT_NAME"-00000000-0000-0000-0000-000000000000"

#define EFI_VARIABLE_NON_VOLATILE 0x0000000000000001
#define EFI_VARIABLE_BOOTSERVICE_ACCESS 0x0000000000000002
#define EFI_VARIABLE_RUNTIME_ACCESS 0x0000000000000004

typedef unsigned long efi_status_t;
typedef uint8_t  efi_bool_t;
typedef uint16_t efi_char16_t;		/* UNICODE character */

/*
 * EFI GUID type definition
 */
typedef struct {
	uint32_t data1;
	uint16_t data2;
	uint16_t data3;
	uint8_t  data4[8];
} efi_guid_t;

/*
 * EFI variable structure 
 */
typedef struct _efi_variable_t {
        efi_char16_t	variablename[1024/sizeof(efi_char16_t)];
        efi_guid_t	vendorguid;
        uint64_t	datasize;
        uint8_t         data[1024];
	efi_status_t	status;
        uint32_t        attributes;
} __attribute__((packed)) efi_variable_t;

static char *efivar_dir = EFIVAR_DIR;
static char *elilo_alt_name = ELILO_ALT_NAME;
static char *elilo_altvar = ELILO_ALTVAR;

static struct option cmd_options[]={
	{ "version", 0, 0, 1},
	{ "help", 0, 0, 2},
	{ "delete", 0, 0, 3},
	{ "print", 0, 0, 4},
	{ "set", 1, 0, 5},
	{ 0, 0, 0, 0}
};

static void fatal_error(char *fmt,...) __attribute__((noreturn));

static void
fatal_error(char *fmt, ...) 
{
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);

	exit(1);
}


static void
usage(char **argv)
{
	printf("Usage: %s [OPTIONS] cmdline\n", argv[0]);

	printf(	"-h, --help\t\tdisplay this help and exit\n"
		"--version\t\toutput version information and exit\n"
		"-s, --set cmdline\tset elilo alternate variable to cmdline\n"
		"-p, --print\t\tprint elilo alternate variable\n"
		"-d, --delete\t\tprint elilo alternate variable\n"
	);
}

static char *
check_proc_efi(int find_entry)
{
	DIR *efi_vars;
	struct dirent *entry;
	static char name[1024];

	if (getuid() != 0) {
		fatal_error("This program must be run as root\n");
	}
	efi_vars = opendir(efivar_dir);
	if (efi_vars == NULL) {
		efivar_dir = OFIVAR_DIR;
		elilo_altvar = OLILO_ALTVAR;
		efi_vars = opendir(efivar_dir);
	}
	if (efi_vars == NULL) {
		fatal_error("Can access neither %s nor %s\n",
			    EFIVAR_DIR, efivar_dir);
	}
	if (!find_entry) {
		closedir(efi_vars);
		return NULL;
	}
	/* Find one entry we can open */
	while ((entry = readdir(efi_vars)) != NULL) {
		if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, ".."))
			break;
	}
	if (entry == NULL) {
		fatal_error("Cannot find entry in %s\n", efivar_dir);
	}
	snprintf(name, 1023, "%s/%s", efivar_dir, entry->d_name);
	name[1023] = 0;
	closedir(efi_vars);
	return name;
}

static void
delete_var(void)
{
	efi_variable_t var;
	int fd, r, i;

	check_proc_efi(0);

	fd = open(elilo_altvar, O_WRONLY);
	if (fd == -1) {
		fatal_error("variable not defined\n");
	}

	memset(&var, 0, sizeof(var));

	for (i=0; i < sizeof(elilo_alt_name); i++) {
		var.variablename[i] = (efi_char16_t)elilo_alt_name[i];
	}

	/* 
	 * we use NULL GUID so no need to initialize it now memset() did it
	 * writing with a datasize=0 will effectively delete the variable.
	 */
	
	r = write(fd, &var, sizeof(var));
	if (r != sizeof(var)) {
		fatal_error("Variable %s defined but invalid content\n", elilo_altvar);
	}
	close(fd);
}


static void
print_var(void)
{
	efi_variable_t var;
	int fd, r, i;


	check_proc_efi(0);

	fd = open(elilo_altvar, O_RDONLY);
	if (fd == -1) {
		fatal_error("variable not defined\n");
	}

	memset(&var, 0, sizeof(var));

	r = read(fd, &var, sizeof(var));
	if (r != sizeof(var)) {
		fatal_error("Variable %s defined but invalid content\n", elilo_altvar);
	}
	printf("EliloAlt=\"");
	for(i=0; i < var.datasize; i+=1){
		printf("%c", var.data[i]);
	}
	printf("\"\n");
	close(fd);
}

static void
set_var(char *cmdline)
{
	efi_variable_t var;
	int fd, r, i, j, l;
	char *name;

	name = check_proc_efi(1);

	if (cmdline == NULL) {
		fatal_error("invalid cmdline argument\n");
	}

	l = strlen(cmdline);

	if (l >= 1024) {
		fatal_error("Variable content is too long, must be <= 512 characters\n");
	}

	fd = open(name, O_WRONLY);
	if (fd == -1) {
		fatal_error("can't open %s: %s\n", elilo_altvar, strerror(errno));
	}

	memset(&var, 0, sizeof(var));

	for (i=0; i < sizeof(elilo_alt_name); i++) {
		var.variablename[i] = (efi_char16_t)elilo_alt_name[i];
	}

	for (i=0, j=0; i < l; i++, j+=2) {
		var.data[j] = (efi_char16_t)cmdline[i];
	}
	/* +2 = include char16 for null termination */
	var.datasize   = j+2;

	var.attributes =  EFI_VARIABLE_NON_VOLATILE 
			| EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS;

	/* 
	 * we use NULL GUID so no need to initialize it now memset() did it
	 * writing with a datasize=0 will effectively delete the variable.
	 */
	
	r = write(fd, &var, sizeof(var));
	if (r != sizeof(var)) {
		fatal_error("Variable %s defined but invalid content %d\n", elilo_altvar, r);
	}
	close(fd);

}

int
main(int argc, char **argv)
{
	int c;

	while ((c=getopt_long(argc, argv,"hdps:", cmd_options, 0)) != -1) {
		switch(c) {
			case   0: continue; /* fast path for options */
			case   1:
				printf("Version %s Date: %s\n", ELILOALT_VERSION, __DATE__);
				exit(0);
			case   2:
			case 'h':
				usage(argv);
				exit(0);
			case   3:
			case 'd':
				delete_var();
				exit(0);
			case   4:
			case 'p':
				print_var();
				exit(0);
			case   5:
			case 's':
				set_var(optarg);
				exit(0);
			default:
				fatal_error("Unknown option\n");
		}
	}
	print_var();
	return 0;
}
