
#ifndef Py_MODSUPPORT_H
#define Py_MODSUPPORT_H
#ifdef __cplusplus
extern "C" {
#endif

/* Module support interface */

#include <stdarg.h>

PyAPI_FUNC(int) PyArg_Parse(PyObject *, char *, ...);
PyAPI_FUNC(int) PyArg_ParseTuple(PyObject *, char *, ...);
PyAPI_FUNC(int) PyArg_ParseTupleAndKeywords(PyObject *, PyObject *,
                                                  char *, char **, ...);
PyAPI_FUNC(int) PyArg_UnpackTuple(PyObject *, char *, int, int, ...);
PyAPI_FUNC(PyObject *) Py_BuildValue(char *, ...);
PyAPI_FUNC(int) _PyArg_NoKeywords(char *funcname, PyObject *kw);

PyAPI_FUNC(int) PyArg_VaParse(PyObject *, char *, va_list);
PyAPI_FUNC(int) PyArg_VaParseTupleAndKeywords(PyObject *, PyObject *,
                                                  char *, char **, va_list);
PyAPI_FUNC(PyObject *) Py_VaBuildValue(char *, va_list);

PyAPI_FUNC(int) PyModule_AddObject(PyObject *, char *, PyObject *);
PyAPI_FUNC(int) PyModule_AddIntConstant(PyObject *, char *, long);
PyAPI_FUNC(int) PyModule_AddStringConstant(PyObject *, char *, char *);

#if defined(EFI32) || defined(EFI64) || defined(EFIX64)
PyAPI_FUNC(int) PyArg_ParseEfi (PyObject *, char *, va_list);
/* PyAPI_FUNC(int) PyArg_VaParseTupleAndKeywords (PyObject *, PyObject *,
						 char *, char **, va_list *); */ /*check here*/
#endif


#define PYTHON_API_VERSION 1012
#define PYTHON_API_STRING "1012"
/* The API version is maintained (independently from the Python version)
   so we can detect mismatches between the interpreter and dynamically
   loaded modules.  These are diagnosed by an error message but
   the module is still loaded (because the mismatch can only be tested
   after loading the module).  The error message is intended to
   explain the core dump a few seconds later.

   The symbol PYTHON_API_STRING defines the same value as a string
   literal.  *** PLEASE MAKE SURE THE DEFINITIONS MATCH. ***

   Please add a line or two to the top of this log for each API
   version change:

   19-Aug-2002  GvR	1012	Changes to string object struct for
   				interning changes, saving 3 bytes.

   17-Jul-2001	GvR	1011	Descr-branch, just to be on the safe side

   25-Jan-2001  FLD     1010    Parameters added to PyCode_New() and
                                PyFrame_New(); Python 2.1a2

   14-Mar-2000  GvR     1009    Unicode API added

   3-Jan-1999	GvR	1007	Decided to change back!  (Don't reuse 1008!)

   3-Dec-1998	GvR	1008	Python 1.5.2b1

   18-Jan-1997	GvR	1007	string interning and other speedups

   11-Oct-1996	GvR	renamed Py_Ellipses to Py_Ellipsis :-(

   30-Jul-1996	GvR	Slice and ellipses syntax added

   23-Jul-1996	GvR	For 1.4 -- better safe than sorry this time :-)

   7-Nov-1995	GvR	Keyword arguments (should've been done at 1.3 :-( )

   10-Jan-1995	GvR	Renamed globals to new naming scheme

   9-Jan-1995	GvR	Initial version (incompatible with older API)
*/

#ifdef MS_WINDOWS
/* Special defines for Windows versions used to live here.  Things
   have changed, and the "Version" is now in a global string variable.
   Reason for this is that this for easier branding of a "custom DLL"
   without actually needing a recompile.  */
#endif /* MS_WINDOWS */

#ifdef Py_TRACE_REFS
/* When we are tracing reference counts, rename Py_InitModule4 so
   modules compiled with incompatible settings will generate a
   link-time error. */
#define Py_InitModule4 Py_InitModule4TraceRefs
#endif

PyAPI_FUNC(PyObject *) Py_InitModule4(char *name, PyMethodDef *methods,
                                            char *doc, PyObject *self,
                                            int apiver);

#define Py_InitModule(name, methods) \
	Py_InitModule4(name, methods, (char *)NULL, (PyObject *)NULL, \
		       PYTHON_API_VERSION)

#define Py_InitModule3(name, methods, doc) \
	Py_InitModule4(name, methods, doc, (PyObject *)NULL, \
		       PYTHON_API_VERSION)

PyAPI_DATA(char *) _Py_PackageContext;

#ifdef __cplusplus
}
#endif
#endif /* !Py_MODSUPPORT_H */
