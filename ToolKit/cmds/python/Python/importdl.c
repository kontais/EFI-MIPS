
/* Support for dynamic loading of extension modules */

#include "Python.h"

/* ./configure sets HAVE_DYNAMIC_LOADING if dynamic loading of modules is
   supported on this platform. configure will then compile and link in one
   of the dynload_*.c files, as appropriate. We will call a function in
   those modules to get a function pointer to the module's init function.
*/
#ifdef HAVE_DYNAMIC_LOADING

#include "importdl.h"

extern dl_funcptr _PyImport_GetDynLoadFunc(const char *name,
					   const char *shortname,
					   const char *pathname, FILE *fp);



PyObject *
_PyImport_LoadDynamicModule(char *name, char *pathname, FILE *fp)
{
	PyObject *m;
	char *lastdot, *shortname, *packagecontext, *oldcontext;
	dl_funcptr p;

	if ((m = _PyImport_FindExtension(name, pathname)) != NULL) {
		Py_INCREF(m);
		return m;
	}
	lastdot = strrchr(name, '.');
	if (lastdot == NULL) {
		packagecontext = NULL;
		shortname = name;
	}
	else {
		packagecontext = name;
		shortname = lastdot+1;
	}

	p = _PyImport_GetDynLoadFunc(name, shortname, pathname, fp);
	if (PyErr_Occurred())
		return NULL;
	if (p == NULL) {
		PyErr_Format(PyExc_ImportError,
		   "dynamic module does not define init function (init%.200s)",
			     shortname);
		return NULL;
	}
        oldcontext = _Py_PackageContext;
	_Py_PackageContext = packagecontext;
	(*p)();
	_Py_PackageContext = oldcontext;
	if (PyErr_Occurred())
		return NULL;

	m = PyDict_GetItemString(PyImport_GetModuleDict(), name);
	if (m == NULL) {
		PyErr_SetString(PyExc_SystemError,
				"dynamic module not initialized properly");
		return NULL;
	}
	/* Remember the filename as the __file__ attribute */
	if (PyModule_AddStringConstant(m, "__file__", pathname) < 0)
		PyErr_Clear(); /* Not important enough to report */

	if (_PyImport_FixupExtension(name, pathname) == NULL)
		return NULL;
	if (Py_VerboseFlag)
		PySys_WriteStderr(
			"import %s # dynamically loaded from %s\n",
			name, pathname);
	Py_INCREF(m);
	return m;
}

#endif /* HAVE_DYNAMIC_LOADING */



#ifdef USE_EFI

static EFI_STATUS
efi_unload_image_fun(
  IN EFI_HANDLE                   ImageHandle
)
{
	return EFI_SUCCESS;
}
/* ----------------------------------------------------------------------
 * EFI dynamic loading support
 *
 * EFI doesn't have shared library support, but it can dynamically load
 *  drivers that can export an API as an interface protocol. 
 *
 * I'm modeling this after the BeOS code above.
 */

/*
 * Initialize our dictionary.
 */
static void efi_init_dyn( void )
{
	if( efi_dyn_images == NULL ) {
		efi_dyn_images = PyDict_New();
		atexit( efi_cleanup_dyn );
	}
}

/* atexit() handler that'll call UnloadImage() for every item in the
 * dictionary.
 */
static void efi_cleanup_dyn( void )
{
	if( efi_dyn_images ) {
		int idx;
		int list_size;
		PyObject *id_list;

		id_list = PyDict_Values( efi_dyn_images );

		list_size = PyList_Size( id_list );
		for( idx = 0; idx < list_size; idx++ ) {
			PyObject *the_item;
			
			the_item = PyList_GetItem( id_list, idx );
			efi_nuke_dyn( the_item );
		}

		PyDict_Clear( efi_dyn_images );
	}
}

/* Whack an item; the item is an EFI Image Handle in disguise, so we'll call
 * UnloadImage() for it.
 */
static void efi_nuke_dyn( PyObject *item )
{
	if( item ) {
		EFI_HANDLE Handle = (EFI_HANDLE)PyLong_AsVoidPtr( item );

		UnloadImage( Handle );
	}
}

/*
 * Add an EFI Image Handle to the dictionary; the module name of the loaded image
 * is the key.  Note that if the key is already in the dict, we unload
 * that image; this should allow reload() to work on dynamically loaded
 * modules (super-keen!).
 */
static void efi_add_dyn( char *name, EFI_HANDLE Handle )
{
	PyObject *py_id;

	if( efi_dyn_images == NULL ) {
		efi_init_dyn();
	}

	/* If there's already an object with this key in the dictionary,
	 * we're doing a reload(), so let's nuke it.
	 */
	py_id = PyDict_GetItemString( efi_dyn_images, name );
	if( py_id ) {
		efi_nuke_dyn( py_id );
		PyDict_DelItemString( efi_dyn_images, name );
	}

	py_id = PyLong_FromVoidPtr( Handle );
	if( py_id ) {
		PyDict_SetItemString( efi_dyn_images, name, py_id );
	}
}

/*
 * The actual module init function in the dynamically loaded module takes
 *  a couple of paramaters. Rather than redefine the init function pointer
 *  I'm using this wrapper.
 */
static void efi_init_wrapperabc( void ) 
{
    if (ModuleInterface) {
        ModuleInterface->InitModule(ModuleInterface, &EfiPythonApi);
        ModuleInterface = NULL;
    }
}

#endif
