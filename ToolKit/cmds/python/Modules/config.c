
/* Module configuration */

/* This file contains the table of built-in modules.
   See init_builtin() in import.c. */
#ifdef EFI_LOADABLE_MODULE
#define EFI_DYNAMIC_MODULES
#undef EFI_LOADABLE_MODULE
#endif

#include "Python.h"

extern void init_codecs(void);
extern void init_random(void);
extern void init_sre( void );
extern void init_socket( void );
extern void initarray( void );
extern void initbinascii( void );
extern void initbsddb185( void );
extern void initcmath( void );
extern void initefi( void );
extern void initerrno( void );
extern void initdbm( void );
extern void initmath( void );
extern void initmd5( void );
extern void initoperator( void );
extern void initparser( void );
extern void initregex( void );
extern void initselect( void );
extern void initsha( void );
extern void initstrop( void );
extern void initstruct( void );
extern void inittime( void );
extern void inittiming( void );
extern void initthread( void );
extern void initcStringIO( void );
extern void initcPickle( void );
extern void initzlib( void );
extern void initunicodedata( void );
extern void inititertools( void );

/* -- ADDMODULE MARKER 1 -- */

extern void PyMarshal_Init( void );
extern void initimp( void );

struct _inittab _PyImport_Inittab[] = {

    {"efi",         initefi},
    {"strop",       initstrop},
#ifndef EFI_DYNAMIC_MODULES
    {"_codecs",     init_codecs},
    {"_random",     init_random},
    {"_sre",        init_sre},
    {"_socket",     init_socket},
    {"array",       initarray},
    {"binascii",    initbinascii},
    {"bsddb185",    initbsddb185},
    {"cmath",       initcmath},
    {"dbm",         initdbm},
    {"errno",       initerrno},
    {"math",        initmath},
    {"md5",         initmd5},
    {"operator",    initoperator},
    {"parser",      initparser},
    {"regex",       initregex},
    {"select",      initselect},
    {"sha",         initsha},
    {"struct",      initstruct},
    {"time",        inittime}, 
    {"timing",      inittiming},
#ifdef WITH_THREAD
    {"thread",      initthread},
#endif
    {"cStringIO",   initcStringIO},
    {"cPickle",     initcPickle},
    {"itertools",   inititertools},
    {"unicodedata", initunicodedata},
    {"zlib",        initzlib},
#endif

/* -- ADDMODULE MARKER 2 -- */

	/* This module "lives in" with marshal.c */
	{"marshal", PyMarshal_Init},

	/* This lives it with import.c */
	{"imp", initimp},

	/* These entries are here for sys.builtin_module_names */
	{"__main__", NULL},
	{"__builtin__", NULL},
	{"sys", NULL},

	/* Sentinel */
	{0, 0}
};
