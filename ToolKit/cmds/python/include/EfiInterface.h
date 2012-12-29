/*
 * Copyright (c) 2000
 * Intel Corporation.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 * 
 *    This product includes software developed by Intel Corporation and
 *    its contributors.
 * 
 * 4. Neither the name of Intel Corporation or its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY INTEL CORPORATION AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL INTEL CORPORATION OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */

/*++

Copyright (c) 2000  Intel Corporation

Module Name:

    EfiInterface.h

Abstract:

    This file declares the function table that provides the interface to
    the Python API for dynamically loaded modules.

Revision History

--*/


#ifndef _EFI_INTERFACE_H_
#define _EFI_INTERFACE_H_


#include "Python.h"
#include "compile.h"
#include "node.h"

/*
 * From modsupport.h
 */
typedef int (EFIAPI *PYARG_VA_PARSE) (PyObject *,char *, va_list);
typedef PyObject *(EFIAPI *PY_VA_BUILD_VALUE) (char *, va_list);
typedef int (EFIAPI *PYARG_VA_PARSE_TUPLE_AND_KEYWORDS) (PyObject *, PyObject *, char *, char **, va_list );
typedef PyObject *(EFIAPI *PY_INIT_MODULE4) (char *, PyMethodDef *,	char *,	PyObject *,	int);
typedef char** _PY_PACKAGE_CONTEXT;
typedef int (EFIAPI *PYARG_PARSE_EFI) (PyObject *, char *, va_list);
typedef int (EFIAPI *PYMODULE_ADDOBJECT)(PyObject *m, char *name, PyObject *o);
typedef int (EFIAPI *PYMODULE_ADDINTCONSTANT)(PyObject *m, char *name, long value);
typedef int (EFIAPI *PYMODULE_ADDSTRINGCONSTANT)(PyObject *m, char *name, char *value);

/*
 * From pyerrors.h
 */
typedef void (EFIAPI *PYERR_SET_NONE) (PyObject	*);
typedef void (EFIAPI *PYERR_SET_OBJECT) (PyObject *, PyObject *);
typedef void (EFIAPI *PYERR_SET_STRING) (PyObject *, const char	*);
typedef PyObject *(EFIAPI *PYERR_OCCURRED) (void);
typedef void (EFIAPI *PYERR_CLEAR) (void);
typedef void (EFIAPI *PYERR_FETCH) (PyObject **, PyObject **, PyObject **);
typedef void (EFIAPI *PYERR_RESTORE) (PyObject *, PyObject *, PyObject *);
typedef int (EFIAPI *PYERR_GIVEN_EXCEPTION_MATCHES) (PyObject *, PyObject *);
typedef int (EFIAPI	*PYERR_EXCEPTION_MATCHES) (PyObject *);
typedef void (EFIAPI *PYERR_NORMALIZE_EXCEPTION) (PyObject **, PyObject **, PyObject **);
typedef PyObject**	PYEXC_EXCEPTION;
typedef int (EFIAPI *PYERR_BAD_ARGUMENT) (void);
typedef PyObject *(EFIAPI *PYERR_NO_MEMORY) (void);
typedef PyObject *(EFIAPI *PYERR_SET_FROM_ERRNO) (PyObject *);
typedef PyObject *(EFIAPI *PYERR_SET_FROM_ERRNO_WITH_FILENAME) (PyObject *, char *);
typedef void (EFIAPI *PYERR_BAD_INTERNAL_CALL) (void);
typedef PyObject *(EFIAPI *PYERR_NEW_EXCEPTION) (char *, PyObject *, PyObject *);
typedef int (EFIAPI *PYERR_CHECK_SIGNALS) (void);
typedef void (EFIAPI *PYERR_SET_INTERRUPT) (void);

// From object.h
typedef PyTypeObject *PYTYPE_TYPE;
typedef int (EFIAPI *PYOBJECT_PRINT) (PyObject *, FILE *, int);
typedef PyObject *(EFIAPI *PYOBJECT_REPR) (PyObject *);
typedef PyObject *(EFIAPI *PYOBJECT_STR) (PyObject *);
typedef int (EFIAPI *PYOBJECT_COMPARE) (PyObject *, PyObject *);
typedef PyObject *(EFIAPI *PYOBJECT_GET_ATTR_STRING) (PyObject *, char *);
typedef int (EFIAPI *PYOBJECT_SET_ATTR_STRING) (PyObject *, char *, PyObject *);
typedef int (EFIAPI *PYOBJECT_HAS_ATTR_STRING) (PyObject *, char *);
typedef PyObject *(EFIAPI *PYOBJECT_GET_ATTR) (PyObject *, PyObject *);
typedef int (EFIAPI *PYOBJECT_SET_ATTR) (PyObject *, PyObject *, PyObject *);
typedef int (EFIAPI *PYOBJECT_HAS_ATTR) (PyObject *, PyObject *);
typedef long (EFIAPI *PYOBJECT_HASH) (PyObject *);
typedef int (EFIAPI *PYOBJECT_IS_TRUE) (PyObject *);
typedef int (EFIAPI *PYOBJECT_NOT) (PyObject *);
typedef int (EFIAPI *PYCALLABLE_CHECK) (PyObject *);
typedef int (EFIAPI *PYNUMBER_COERCE) (PyObject **, PyObject **);
typedef int (EFIAPI *PYNUMBER_COERCE_EX) (PyObject **, PyObject **);
typedef int (EFIAPI *PY_REPR_ENTER) (PyObject *);
typedef void (EFIAPI *PY_REPR_LEAVE) (PyObject *);
typedef void (EFIAPI *_PY_DEALLOC) (PyObject *);
typedef void (EFIAPI *_PY_NEW_REFERENCE) (PyObject *);
typedef void (EFIAPI *_PY_FORGET_REFERENCE) (PyObject *);
typedef void (EFIAPI *_PY_PRINT_REFERENCES) (FILE *);
typedef void (EFIAPI *INC_COUNT) (PyTypeObject *);
typedef long *_PY_REF_TOTAL;
typedef PyObject *_PY_NONE_STRUCT;
typedef PyObject *_PY_NOTIMPLEMENTEDSTRUCT;
typedef PyObject *( EFIAPI *PYOBJECT_SELFITER)(PyObject *);
typedef void ( EFIAPI *PYOBJECT_FREE)(void *);
typedef PyObject *(EFIAPI *PYOBJECT_RICHCOMPARE)(PyObject *v, PyObject *w, int op);
typedef int (EFIAPI *PYOBJECT_RICHCOMPAREBOOL)(PyObject *v, PyObject *w, int op);
typedef void *(EFIAPI *_PYMEM_REALLOC)(void *p, size_t nbytes);
typedef int ( EFIAPI *PYOBJECT_GENERICSETATTR)(PyObject *obj, PyObject *name, PyObject *value);
typedef void *(EFIAPI *PYMEM_MALLOC)(size_t nbytes);
typedef void (EFIAPI *PYMEM_FREE)(void *p);
typedef void (EFIAPI *_PYOBJECT_DUMP)(PyObject* op);

// From objimpl.h
typedef PyObject *(EFIAPI *_PYOBJECT_NEW) (PyTypeObject *);
typedef PyVarObject *(EFIAPI *_PYOBJECT_NEW_VAR) (PyTypeObject *, int);

// From dictobject.h
typedef PyTypeObject *PY_DICT_TYPE;
typedef PyObject *(EFIAPI *PYDICT_NEW) (void);
typedef PyObject *(EFIAPI *PYDICT_GET_ITEM) (PyObject *mp, PyObject *key);
typedef int (EFIAPI *PYDICT_SET_ITEM) (PyObject *mp, PyObject *key, PyObject *item);
typedef int (EFIAPI *PYDICT_DEL_ITEM) (PyObject *mp, PyObject *key);
typedef void (EFIAPI *PYDICT_CLEAR) (PyObject *mp);
typedef int (EFIAPI *PYDICT_NEXT) (PyObject *mp, int *pos, PyObject **key, PyObject **value);
typedef PyObject *(EFIAPI *PYDICT_KEYS) (PyObject *mp);
typedef PyObject *(EFIAPI *PYDICT_VALUES) (PyObject *mp);
typedef PyObject *(EFIAPI *PYDICT_ITEMS) (PyObject *mp);
typedef int (EFIAPI *PYDICT_SIZE) (PyObject *mp);
typedef PyObject *(EFIAPI *PYDICT_GET_ITEM_STRING) (PyObject *dp, const char *key);
typedef int (EFIAPI *PYDICT_SET_ITEM_STRING) (PyObject *dp, const char *key, PyObject *item);
typedef int (EFIAPI *PYDICT_DEL_ITEM_STRING) (PyObject *dp, const char *key);

// From methodobject.h
typedef PyTypeObject * PY_CFUNCTION_TYPE;
typedef PyCFunction (EFIAPI *PYCFUNCTION_GET_FUNCTION) (PyObject *);
typedef PyObject * (EFIAPI *PYCFUNCTION_GET_SELF) (PyObject *);
typedef int (EFIAPI *PYCFUNCTION_GET_FLAGS) (PyObject *);
typedef PyObject * (EFIAPI *PY_FIND_METHOD) (PyMethodDef[], PyObject *, char *);
typedef PyObject * (EFIAPI *PYCFUNCTION_NEW) (PyMethodDef *, PyObject *);
typedef PyObject * (EFIAPI *PY_FIND_METHOD_IN_CHAIN) (PyMethodChain *, PyObject *, char *);

// From intobject.h
typedef PyTypeObject *PYINT_TYPE;
typedef PyObject * (EFIAPI *PYINT_FROM_LONG) (long);
typedef long (EFIAPI *PYINT_AS_LONG) (PyObject *);
typedef long (EFIAPI *PYINT_GET_MAX) (void);
typedef PyIntObject *PYINTOBJECT;
typedef unsigned long (EFIAPI *PYOS_STROUL) (char *, char **, int);
typedef long (EFIAPI *PYOS_STROL)(char *, char **, int);

// From listobject.h
typedef PyTypeObject *PYLIST_TYPE;
typedef PyObject * (EFIAPI *PYLIST_NEW) (int size);
typedef int (EFIAPI *PYLIST_SIZE) (PyObject *);
typedef PyObject * (EFIAPI *PYLIST_GET_ITEM) (PyObject *, int);
typedef int (EFIAPI *PYLIST_SET_ITEM) (PyObject *, int, PyObject *);
typedef int (EFIAPI *PYLIST_INSERT) (PyObject *, int, PyObject *);
typedef int (EFIAPI *PYLIST_APPEND) (PyObject *, PyObject *);
typedef PyObject * (EFIAPI *PYLIST_GET_SLICE) (PyObject *, int, int);
typedef int (EFIAPI *PYLIST_SET_SLICE) (PyObject *, int, int, PyObject *);
typedef int (EFIAPI *PYLIST_SORT) (PyObject *);
typedef int (EFIAPI *PYLIST_REVERSE) (PyObject *);
typedef PyObject * (EFIAPI *PYLIST_AS_TUPLE) (PyObject *);

// From moduleobject.h
typedef PyTypeObject *PYMODULE_TYPE;
typedef PyObject * (EFIAPI *PYMODULE_NEW) (char *);
typedef PyObject * (EFIAPI *PYMODULE_GET_DICT) (PyObject *);
typedef char * (EFIAPI *PYMODULE_GET_NAME) (PyObject *);
typedef char * (EFIAPI *PYMODULE_GET_FILENAME) (PyObject *);
typedef void (EFIAPI *_PYMODULE_CLEAR) (PyObject *);

// From complexobject.h
typedef Py_complex (EFIAPI *C_SUM) (Py_complex, Py_complex);
typedef Py_complex (EFIAPI *C_DIFF) (Py_complex, Py_complex);
typedef Py_complex (EFIAPI *C_NEG) (Py_complex);
typedef Py_complex (EFIAPI *C_PROD) (Py_complex, Py_complex);
typedef Py_complex (EFIAPI *C_QUOT) (Py_complex, Py_complex);
typedef Py_complex (EFIAPI *C_POW) (Py_complex, Py_complex);
typedef PyTypeObject *PYCOMPLEX_TYPE;
typedef PyObject * (EFIAPI *PYCOMPLEX_FROM_C_COMPLEX) (Py_complex);
typedef PyObject * (EFIAPI *PYCOMPLEX_FROM_DOUBLES) (double real, double imag);
typedef double (EFIAPI *PYCOMPLEX_REAL_AS_DOUBLE) (PyObject *op);
typedef double (EFIAPI *PYCOMPLEX_IMAG_AS_DOUBLE) (PyObject *op);
typedef Py_complex (EFIAPI *PYCOMPLEX_AS_C_COMPLEX) (PyObject *op);

// From floatobject.c
typedef PyTypeObject *PYFLOAT_TYPE;
typedef PyObject * (EFIAPI *PYFLOAT_FROM_DOUBLE) (double);
typedef double (EFIAPI *PYFLOAT_AS_DOUBLE) (PyObject *);
typedef int (EFIAPI *_PYFLOAT_PACK8)(double x, unsigned char *p, int le);
typedef double (EFIAPI *_PYFLOAT_UNPACK8)(const unsigned char *p, int le);
typedef double (EFIAPI *_PYFLOAT_UNPACK4)(const unsigned char *p, int le);
typedef int (EFIAPI *_PYFLOAT_PACK4)(double x, unsigned char *p, int le);

// From stringobject.h
typedef PyTypeObject *PYSTRING_TYPE;
typedef PyObject * (EFIAPI *PYSTRING_FROM_STRING_AND_SIZE) (const char *, int);
typedef PyObject * (EFIAPI *PYSTRING_FROM_STRING) (const char *);
typedef int (EFIAPI *PYSTRING_SIZE) (PyObject *);
typedef char * (EFIAPI *PYSTRING_AS_STRING) (PyObject *);
typedef void (EFIAPI *PYSTRING_CONCAT) (PyObject **, PyObject *);
typedef void (EFIAPI *PYSTRING_CONCAT_AND_DEL) (PyObject **, PyObject *);
typedef int (EFIAPI *PYSTRING_RESIZE) (PyObject **, int);
typedef PyObject * (EFIAPI *PYSTRING_FORMAT) (PyObject *, PyObject *);
typedef void (EFIAPI *PYSTRING_INTERN_IN_PLACE) (PyObject **);
typedef PyObject * (EFIAPI *PYSTRING_INTERN_FROM_STRING) (const char *);
typedef PyObject *(EFIAPI *PYSTRING_DECODEESCAPE)(const char *s, int len, const char *errors, int unicode, const char *recode_encoding);
typedef int (EFIAPI *PYSTRING_ASSTRINGANDSIZE)(register PyObject *obj, register char **s, register int *len);

    // From longobject.h
typedef PyTypeObject *PYLONG_TYPE;
typedef PyObject * (EFIAPI *PYLONG_FROM_LONG) (long);
typedef PyObject * (EFIAPI *PYLONG_FROM_UNSIGNED_LONG) (unsigned long);
typedef PyObject * (EFIAPI *PYLONG_FROM_DOUBLE) (double);
typedef long (EFIAPI *PYLONG_AS_LONG) (PyObject *);
typedef unsigned long (EFIAPI *PYLONG_AS_UNSIGNED_LONG) (PyObject *);
typedef double (EFIAPI *PYLONG_AS_DOUBLE) (PyObject *);
typedef PyObject * (EFIAPI *PYLONG_FROM_VOID_PTR) (void *);
typedef void * (EFIAPI *PYLONG_AS_VOID_PTR) (PyObject *);
#ifdef HAVE_LONG_LONG
typedef PyObject * (EFIAPI *PYLONG_FROM_LONG_LONG) (LONG_LONG);
typedef PyObject * (EFIAPI *PYLONG_FROM_UNSIGNED_LONG_LONG) (unsigned LONG_LONG);
typedef LONG_LONG (EFIAPI *PYLONG_AS_LONG_LONG) (PyObject *);
typedef unsigned LONG_LONG (EFIAPI *PYLONG_AS_UNSIGNED_LONG_LONG) (PyObject *);
#else
typedef void *PYLONG_FROM_LONG_LONG;
typedef void *PYLONG_FROM_UNSIGNED_LONG_LONG;
typedef void *PYLONG_AS_LONG_LONG;
typedef void *PYLONG_AS_UNSIGNED_LONG_LONG;
#endif
typedef PyObject * (EFIAPI *PYLONG_FROM_STRING) (char *, char **, int);
typedef int (EFIAPI *_PYLONG_ASBYTEARRAY)(PyLongObject* v,
		    unsigned char* bytes, size_t n,
		    int little_endian, int is_signed);
typedef size_t (EFIAPI *_PYLONG_NUMBITS)(PyObject *vv);
typedef int (EFIAPI *_PYLONG_SIGN)(PyObject *vv);
typedef PyObject *(EFIAPI *_PYLONG_FROMBYTEARRAY)(const unsigned char* bytes, size_t n,
		      int little_endian, int is_signed);
typedef double (EFIAPI *_PYLONG_ASSCALEDDOUBLE)(PyObject *vv, int *exponent);


    // From abstract.h
typedef int (EFIAPI *PYOBJECT_CMP) (PyObject *o1, PyObject *o2, int *result);
typedef PyObject * (EFIAPI *PYOBJECT_CALL_OBJECT) (PyObject *callable_object, PyObject *args);
typedef PyObject * (EFIAPI *PYOBJECT_TYPE) (PyObject *o);
typedef int (EFIAPI *PYOBJECT_LENGTH) (PyObject *o);
typedef PyObject * (EFIAPI *PYOBJECT_GET_ITEM) (PyObject *o, PyObject *key);
typedef int (EFIAPI *PYOBJECT_SET_ITEM) (PyObject *o, PyObject *key, PyObject *v);
typedef int (EFIAPI *PYOBJECT_DEL_ITEM) (PyObject *o, PyObject *key);
typedef int (EFIAPI *PYNUMBER_CHECK) (PyObject *o);
typedef PyObject * (EFIAPI *PYNUMBER_ADD) (PyObject *o1, PyObject *o2);
typedef PyObject * (EFIAPI *PYNUMBER_SUBTRACT) (PyObject *o1, PyObject *o2);
typedef PyObject * (EFIAPI *PYNUMBER_MULTIPLY) (PyObject *o1, PyObject *o2);
typedef PyObject * (EFIAPI *PYNUMBER_DIVIDE) (PyObject *o1, PyObject *o2);
typedef PyObject * (EFIAPI *PYNUMBER_REMAINDER) (PyObject *o1, PyObject *o2);
typedef PyObject * (EFIAPI *PYNUMBER_DIVMOD) (PyObject *o1, PyObject *o2);
typedef PyObject * (EFIAPI *PYNUMBER_POWER) (PyObject *o1, PyObject *o2, PyObject *o3);
typedef PyObject * (EFIAPI *PYNUMBER_NEGATIVE) (PyObject *o);
typedef PyObject * (EFIAPI *PYNUMBER_POSITIVE) (PyObject *o);
typedef PyObject * (EFIAPI *PYNUMBER_ABSOLUTE) (PyObject *o);
typedef PyObject * (EFIAPI *PYNUMBER_INVERT) (PyObject *o);
typedef PyObject * (EFIAPI *PYNUMBER_LSHIFT) (PyObject *o1, PyObject *o2);
typedef PyObject * (EFIAPI *PYNUMBER_RSHIFT) (PyObject *o1, PyObject *o2);
typedef PyObject * (EFIAPI *PYNUMBER_AND) (PyObject *o1, PyObject *o2);
typedef PyObject * (EFIAPI *PYNUMBER_XOR) (PyObject *o1, PyObject *o2);
typedef PyObject * (EFIAPI *PYNUMBER_OR) (PyObject *o1, PyObject *o2);
typedef PyObject * (EFIAPI *PYNUMBER_INT) (PyObject *o);
typedef PyObject * (EFIAPI *PYNUMBER_LONG) (PyObject *o);
typedef PyObject * (EFIAPI *PYNUMBER_FLOAT) (PyObject *o);
typedef int (EFIAPI *PYSEQUENCE_CHECK) (PyObject *o);
typedef int (EFIAPI *PYSEQUENCE_LENGTH) (PyObject *o);
typedef PyObject * (EFIAPI *PYSEQUENCE_CONCAT) (PyObject *o1, PyObject *o2);
typedef PyObject * (EFIAPI *PYSEQUENCE_REPEAT) (PyObject *o, int count);
typedef PyObject * (EFIAPI *PYSEQUENCE_GET_ITEM) (PyObject *o, int i);
typedef PyObject * (EFIAPI *PYSEQUENCE_GET_SLICE) (PyObject *o, int i1, int i2);
typedef int (EFIAPI *PYSEQUENCE_SET_ITEM) (PyObject *o, int i, PyObject *v);
typedef int (EFIAPI *PYSEQUENCE_DEL_ITEM) (PyObject *o, int i);
typedef int (EFIAPI *PYSEQUENCE_SET_SLICE) (PyObject *o, int i1, int i2, PyObject *v);
typedef int (EFIAPI *PYSEQUENCE_DEL_SLICE) (PyObject *o, int i1, int i2);
typedef PyObject * (EFIAPI *PYSEQUENCE_TUPLE) (PyObject *o);
typedef PyObject * (EFIAPI *PYSEQUENCE_LIST) (PyObject *o);
typedef int (EFIAPI *PYSEQUENCE_COUNT) (PyObject *o, PyObject *value);
typedef int (EFIAPI *PYSEQUENCE_CONTAINS) (PyObject *o, PyObject *value);
typedef int (EFIAPI *PYSEQUENCE_INDEX) (PyObject *o, PyObject *value);
typedef int (EFIAPI *PYMAPPING_CHECK) (PyObject *o);
typedef int (EFIAPI *PYMAPPING_LENGTH) (PyObject *o);
typedef int (EFIAPI *PYMAPPING_HAS_KEY_STRING) (PyObject *o, char *key);
typedef int (EFIAPI *PYMAPPING_HAS_KEY) (PyObject *o, PyObject *key);
typedef PyObject * (EFIAPI *PYMAPPING_GET_ITEM_STRING) (PyObject *o, char *key);
typedef int (EFIAPI *PYMAPPING_SET_ITEM_STRING) (PyObject *o, char *key, PyObject *value);
typedef PyObject *(EFIAPI *PYITER_NEXT)(PyObject *);
typedef PyObject *(EFIAPI *PYOBJECT_GETITER)(PyObject *);
typedef PyObject *(EFIAPI *PYOBJECT_CALL)(PyObject *func, PyObject *arg, PyObject *kw);
typedef int (EFIAPI *PYOBJECT_ASREADBUFFER)(PyObject *obj, const void **buffer, int *buffer_len);
typedef PyObject *(EFIAPI *PYNUMBER_FLOORDIVIDE)(PyObject *v, PyObject *w);
typedef PyObject *(EFIAPI *PYNUMBER_TRUEDIVIDE)(PyObject *v, PyObject *w);
typedef PyObject *(EFIAPI *PYSEQUENCE_FAST)(PyObject *v, const char *m);

// From tupleobject.h
typedef PyTypeObject *PYTUPLE_TYPE;
typedef PyObject * (EFIAPI *PYTUPLE_NEW) (int size);
typedef int (EFIAPI *PYTUPLE_SIZE) (PyObject *);
typedef PyObject * (EFIAPI *PYTUPLE_GET_ITEM) (PyObject *, int);
typedef int (EFIAPI *PYTUPLE_SET_ITEM) (PyObject *, int, PyObject *);
typedef PyObject * (EFIAPI *PYTUPLE_GET_SLICE) (PyObject *, int, int);
typedef int (EFIAPI *PYTUPLE_RESIZE) (PyObject **, int);
typedef PyObject *(EFIAPI *PYTUPLE_PACK)(int n, ...);

// From classobject.h
typedef PyTypeObject *PYCLASS_TYPE;
typedef PyTypeObject *PYINSTANCE_TYPE;
typedef PyTypeObject *PYMETHOD_TYPE;
typedef PyObject * (EFIAPI *PYCLASS_NEW) (PyObject *, PyObject *, PyObject *);
typedef PyObject * (EFIAPI *PYINSTANCE_NEW) (PyObject *, PyObject *, PyObject *);
typedef PyObject * (EFIAPI *PYMETHOD_NEW) (PyObject *, PyObject *, PyObject *);
typedef PyObject * (EFIAPI *PYMETHOD_FUNCTION) (PyObject *);
typedef PyObject * (EFIAPI *PYMETHOD_SELF) (PyObject *);
typedef PyObject * (EFIAPI *PYMETHOD_CLASS) (PyObject *);
typedef int (EFIAPI *PYCLASS_IS_SUBCLASS) (PyObject *, PyObject *);
typedef PyObject *(EFIAPI *PYINSTANCE_NEWRAW)(PyObject *klass, PyObject *dict);
typedef PyObject *(EFIAPI *_PYINSTANCE_LOOKUP)(PyObject *pinst, PyObject *name);

// From funcobject.h
typedef PyTypeObject *PYFUNCTION_TYPE;
typedef PyObject * (EFIAPI *PYFUNCTION_NEW) (PyObject *, PyObject *);
typedef PyObject * (EFIAPI *PYFUNCTION_GET_CODE) (PyObject *);
typedef PyObject * (EFIAPI *PYFUNCTION_GET_GLOBALS) (PyObject *);
typedef PyObject * (EFIAPI *PYFUNCTION_GET_DEFAULTS) (PyObject *);
typedef int (EFIAPI *PYFUNCTION_SET_DEFAULTS) (PyObject *, PyObject *);

// From fileobject.h
typedef PyTypeObject *PYFILE_TYPE;
typedef PyObject * (EFIAPI *PYFILE_FROM_STRING) (char *, char *);
typedef void (EFIAPI *PYFILE_SET_BUF_SIZE) (PyObject *, int);
typedef PyObject * (EFIAPI *PYFILE_FROM_FILE) (FILE *, char *, char *, int (*)(FILE *));
typedef FILE * (EFIAPI *PYFILE_AS_FILE) (PyObject *);
typedef PyObject * (EFIAPI *PYFILE_NAME) (PyObject *);
typedef PyObject * (EFIAPI *PYFILE_GET_LINE) (PyObject *, int);
typedef int (EFIAPI *PYFILE_WRITE_OBJECT) (PyObject *, PyObject *, int);
typedef int (EFIAPI *PYFILE_SOFT_SPACE) (PyObject *, int);
typedef int (EFIAPI *PYFILE_WRITE_STRING) (const char *, PyObject *);
typedef int (EFIAPI *PYOBJECT_ASFILEDESCRIPTOR)(PyObject *o);

// From sysmodule.h
typedef PyObject * (EFIAPI *PYSYS_GET_OBJECT) (char *);
typedef int (EFIAPI *PYSYS_SET_OBJECT) (char *, PyObject *);
typedef FILE * (EFIAPI *PYSYS_GET_FILE) (char *, FILE *);
typedef void (EFIAPI *PYSYS_SET_ARGV) (int, char **);
typedef void (EFIAPI *PYSYS_SET_PATH) (char *);
typedef void (EFIAPI *PYSYS_WRITE_STDOUT) (const char *format, ...);
typedef void (EFIAPI *PYSYS_WRITESTDERR) (const char *format, ...);

// From ceval.h
typedef PyObject * (EFIAPI *PYEVAL_CALL_OBJECT_WITH_KEYWORDS) (PyObject *, PyObject *, PyObject *);
typedef PyObject * (EFIAPI *PYEVAL_CALL_OBJECT) (PyObject *, PyObject *);
typedef PyObject * (EFIAPI *PYEVAL_CALL_FUNCTION) (PyObject *obj, char *format, ...);
typedef PyObject * (EFIAPI *PYEVAL_CALL_METHOD) (PyObject *obj, char *methodname, char *format, ...);
typedef PyObject * (EFIAPI *PYEVAL_GET_BUILTINS) (void);
typedef PyObject * (EFIAPI *PYEVAL_GET_GLOBALS) (void);
typedef PyObject * (EFIAPI *PYEVAL_GET_LOCALS) (void);
typedef struct _frame * (EFIAPI *PYEVAL_GET_FRAME) (void);
typedef int (EFIAPI *PYEVAL_GET_RESTRICTED) (void);
typedef int (EFIAPI *PYEVAL_FLUSH_LINE) (void);
typedef int (EFIAPI *PYEVAL_ADD_PENDING_CALL) (int (*func) (void *), void *arg);
typedef int (EFIAPI *PYEVAL_MAKE_PENDING_CALLS) (void);
typedef PyThreadState * (EFIAPI *PYEVAL_SAVE_THREAD) (void);
typedef void (EFIAPI *PYEVAL_RESTORE_THREAD) (PyThreadState *);
typedef void (EFIAPI *PYEVAL_INIT_THREADS) (void);
typedef void (EFIAPI *PYEVAL_ACQUIRE_LOCK) (void);
typedef void (EFIAPI *PYEVAL_RELEASE_LOCK) (void);
typedef void (EFIAPI *PYEVAL_ACQUIRE_THREAD) (PyThreadState *tstate);
typedef void (EFIAPI *PYEVAL_RELEASE_THREAD) (PyThreadState *tstate);
typedef int (EFIAPI *PY_GETRECURSIONLIMIT)(void);

// From import.h
typedef long (EFIAPI *PYIMPORT_GET_MAGIC_NUMBER) (void);
typedef PyObject * (EFIAPI *PYIMPORT_EXEC_CODE_MODULE) (char *name, PyObject *co);
typedef PyObject * (EFIAPI *PYIMPORT_EXEC_CODE_MODULE_EX) (char *name, PyObject *co, char *pathname);
typedef PyObject * (EFIAPI *PYIMPORT_GET_MODULE_DICT) (void);
typedef PyObject * (EFIAPI *PYIMPORT_ADD_MODULE) (char *name);
typedef PyObject * (EFIAPI *PYIMPORT_IMPORT_MODULE) (char *name);
typedef PyObject * (EFIAPI *PYIMPORT_IMPORT_MODULE_EX) (char *name, PyObject *globals, PyObject *locals, PyObject *fromlist);
typedef PyObject * (EFIAPI *PYIMPORT_IMPORT) (PyObject *name);
typedef PyObject * (EFIAPI *PYIMPORT_RELOAD_MODULE) (PyObject *m);
typedef void (EFIAPI *PYIMPORT_CLEANUP) (void);
typedef int (EFIAPI *PYIMPORT_IMPORT_FROZEN_MODULE) (char *);
typedef PyObject * (EFIAPI *PYIMPORT_FIND_EXTENSION) (char *, char *);
typedef PyObject * (EFIAPI *PYIMPORT_FIXUP_EXTENSION) (char *, char *);
typedef struct _inittab **PYIMPORT_INITTAB;
typedef int (EFIAPI *PYIMPORT_APPEND_INITTAB) (char *name, void (*initfunc)(void));
typedef int (EFIAPI *PYIMPORT_EXTEND_INITTAB) (struct _inittab *newtab);
typedef struct _frozen **PYIMPORT_FROZEN_MODULES;

// From pythonrun.h
typedef void (EFIAPI *PY_SET_PROGRAM_NAME) (char *);
typedef char * (EFIAPI *PY_GET_PROGRAM_NAME) (void);
typedef void (EFIAPI *PY_SET_PYTHON_HOME) (char *);
typedef char * (EFIAPI *PY_GET_PYTHON_HOME) (void);
typedef void (EFIAPI *PY_INITIALIZE) (void);
typedef void (EFIAPI *PY_FINALIZE) (void);
typedef int (EFIAPI *PY_IS_INITIALIZED) (void);
typedef PyThreadState * (EFIAPI *PY_NEW_INTERPRETER) (void);
typedef void (EFIAPI *PY_END_INTERPRETER) (PyThreadState *);
typedef int (EFIAPI *PYRUN_ANY_FILE) (FILE *, const char *);
typedef int (EFIAPI *PYRUN_SIMPLE_STRING) (const char *);
typedef int (EFIAPI *PYRUN_SIMPLE_FILE) (FILE *, const char *);
typedef int (EFIAPI *PYRUN_INTERACTIVE_ONE) (FILE *, const char *);
typedef int (EFIAPI *PYRUN_INTERACTIVE_LOOP) (FILE *, const char *);
typedef struct _node * (EFIAPI *PYPARSER_SIMPLE_PARSE_STRING) (const char *, int);
typedef struct _node * (EFIAPI *PYPARSER_SIMPLE_PARSE_FILE) (FILE *, const char *, int);
typedef PyObject * (EFIAPI *PYRUN_STRING) (const char *, int, PyObject *, PyObject *);
typedef PyObject * (EFIAPI *PYRUN_FILE) (FILE *, const char *, int, PyObject *, PyObject *);
typedef PyObject * (EFIAPI *PY_COMPILE_STRING) (const char *, const char *, int);
typedef void (EFIAPI *PYERR_PRINT) (void);
typedef void (EFIAPI *PYERR_PRINT_EX) (int);
typedef int (EFIAPI *PY_AT_EXIT) (void (*func) (void));
typedef void (EFIAPI *PY_EXIT) (int);
typedef int (EFIAPI *PY_FD_IS_INTERACTIVE) (FILE *, const char *);
typedef char * (EFIAPI *PY_GET_PROGRAM_FULL_PATH) (void);
typedef char * (EFIAPI *PY_GET_PREFIX) (void);
typedef char * (EFIAPI *PY_GET_EXEC_PREFIX) (void);
typedef char * (EFIAPI *PY_GET_PATH) (void);
typedef const char * (EFIAPI *PY_GET_VERSION) (void);
typedef const char * (EFIAPI *PY_GET_PLATFORM) (void);
typedef const char * (EFIAPI *PY_GET_COPYRIGHT) (void);
typedef const char * (EFIAPI *PY_GET_COMPILER) (void);
typedef const char * (EFIAPI *PY_GET_BUILD_INFO) (void);
typedef PyObject * (EFIAPI *PYSYS_INIT) (void);
typedef void (EFIAPI *PYIMPORT_INIT) (void);
typedef void (EFIAPI *PYIMPORT_FINI) (void);
typedef void (EFIAPI *PYMETHOD_FINI) (void);
typedef void (EFIAPI *PYFRAME_FINI) (void);
typedef void (EFIAPI *PYCFUNCTION_FINI) (void);
typedef void (EFIAPI *PYTUPLE_FINI) (void);
typedef void (EFIAPI *PYSTRING_FINI) (void);
typedef void (EFIAPI *PYINT_FINI) (void);
typedef void (EFIAPI *PYFLOAT_FINI) (void);
typedef void (EFIAPI *PYOS_FINI_INTERRUPTS) (void);
typedef char * (EFIAPI *PYOS_READLINE) (FILE *, FILE *, char *);
typedef int (EFIAPI **PYOS_INPUT_HOOK) (void);
typedef char * (EFIAPI **PYOS_READLINE_FUNCTION_POINTER) (FILE *, FILE *, char *);

// From pydebug.h
typedef int *PY_FLAG;
typedef void (EFIAPI *PY_FATAL_ERROR) (const char *);

//From cobject.h
typedef PyTypeObject *PYCOBJECT_TYPE;
typedef PyObject * (EFIAPI *PYCOBJECT_FROM_VOID_PTR) (void *cobj, void (*destruct)(void*));
typedef PyObject * (EFIAPI *PYCOBJECT_FROM_VOID_PTR_AND_DESC) (void *cobj, void *desc, void (*destruct)(void*,void*));
typedef void * (EFIAPI *PYCOBJECT_AS_VOID_PTR) (PyObject *);
typedef void * (EFIAPI *PYCOBJECT_GET_DESC) (PyObject *);
typedef void * (EFIAPI *PYCOBJECT_IMPORT) (char *module_name, char *cobject_name);

// From compile.h
typedef PyTypeObject *PYCODE_TYPE;
typedef PyCodeObject * (EFIAPI *PYCODE_COMPILE) (struct _node *, const char *);
typedef PyCodeObject * (EFIAPI *PYCODE_NEW) (
	int, int, int, int, PyObject *, PyObject *, PyObject *, PyObject *,
	PyObject *, PyObject *, PyObject *, PyObject *, int, PyObject *);
typedef int (EFIAPI *PYCODE_ADDR_2_LINE) (PyCodeObject *, int);

// From node.h
typedef node * (EFIAPI *PYNODE_NEW) (int type);
typedef int (EFIAPI *PYNODE_ADD_CHILD) (node *n, int type, char *str, int lineno);
typedef void (EFIAPI *PYNODE_FREE) (node *n);
typedef void (EFIAPI *PYNODE_LIST_TREE) (node *);

// From typeobject.h
typedef PyObject *(EFIAPI *PYTYPE_GENERICALLOC)(PyTypeObject *type, int nitems);
typedef PyObject *(EFIAPI *PYOBJECT_GENERICGETATTR)(PyObject *obj, PyObject *name);
typedef int (EFIAPI *PYTYPE_ISSUBTYPE)(PyTypeObject *a, PyTypeObject *b);
typedef int (EFIAPI *PYTYPE_READY)(PyTypeObject *type);
typedef PyObject *(EFIAPI *PYTYPE_GENERICNEW)(PyTypeObject *type, PyObject *args, PyObject *kwds);

// From weakrefobject.h
typedef void (EFIAPI *PYOBJECT_CLEARWEAKREFS)(PyObject *);
typedef PyTypeObject *PYWEAKREF_REFTYPE;
typedef PyTypeObject *PYWEAKREF_PROXYTYPE;
typedef PyTypeObject *PYWEAKREF_CALLABLEPROXYTYPE;
typedef void (EFIAPI *_PYWEAKREF_CLEARREF)(PyWeakReference *self);

// mysnprintf
typedef int (EFIAPI *PYOS_SNPRINTF)(char *str, size_t size, const  char  *format, ...);

// sliceobject
typedef PyObject * PY_ELLIPSISOBJECT;
typedef int (EFIAPI *PYSLICE_GETINDICESEX)(PySliceObject *r, int length,
		     int *start, int *stop, int *step, int *slicelength);
typedef PyTypeObject *PYSLICE_TYPE;

// getargs
typedef int (EFIAPI *PYARG_NOKEYWORDS)(char *funcname, PyObject *kw);
typedef int (EFIAPI *PYARG_UNPACKTUPLE)(PyObject *args, char *name, int min, int max, ...);

// gcmodule
typedef void (EFIAPI *PYOBJECT_GC_TRACK)(void *op);
typedef PyObject *(EFIAPI *PYOBJECT_GC_NEW)(PyTypeObject *tp);
typedef void (EFIAPI *PYOBJECT_GC_DEL)(void *op);
typedef void (EFIAPI *PYOBJECT_GC_UNTRACK)(void *op);

// boolobject
typedef PyObject *(EFIAPI *PYBOOL_FROMLONG)(long ok);

// pystrtod
typedef double (EFIAPI *PYOS_ASCII_STRTOD)(const char  *nptr, char **endptr);

// errors
typedef int (EFIAPI *PYERR_WARN)(PyObject *category, char *message);
typedef void (EFIAPI *PYERR_WRITEUNRAISABLE)(PyObject *obj);

// structseq
typedef PyObject *(EFIAPI *PYSTRUCTSEQUENCE_NEW)(PyTypeObject *type);
typedef void (EFIAPI *PYSTRUCTSEQUENCE_INITTYPE)(PyTypeObject *type, PyStructSequence_Desc *desc);

// ****************************************************************************
#define EFI_PYTHON_API_REVISION		((UINT64) 0x000010000)

typedef struct _EFI_PYTHON_API {
    UINT64		                        EPA_Revision;

    // From modsupport.h
    PYARG_VA_PARSE	                    EPA_PyArg_VaParse;
    PYARG_VA_PARSE_TUPLE_AND_KEYWORDS   EPA_PyArg_VaParseTupleAndKeywords;
    PY_VA_BUILD_VALUE	                EPA_Py_VaBuildValue;
    PY_INIT_MODULE4	                    EPA_Py_InitModule4;
    _PY_PACKAGE_CONTEXT                 EPA__Py_PackageContext;
    PYARG_PARSE_EFI                     EPA_PyArg_ParseEfi;
    PYMODULE_ADDOBJECT                  EPA_PyModule_AddObject;
    PYMODULE_ADDINTCONSTANT             EPA_PyModule_AddIntConstant;
    PYMODULE_ADDSTRINGCONSTANT          EPA_PyModule_AddStringConstant;

    // From pyerrors.h
    PYERR_SET_NONE			        EPA_PyErr_SetNone;
    PYERR_SET_OBJECT		        EPA_PyErr_SetObject;
    PYERR_SET_STRING		        EPA_PyErr_SetString;
    PYERR_OCCURRED			        EPA_PyErr_Occurred;
    PYERR_CLEAR				        EPA_PyErr_Clear;
    PYERR_FETCH				        EPA_PyErr_Fetch;
    PYERR_RESTORE			        EPA_PyErr_Restore;
    PYERR_GIVEN_EXCEPTION_MATCHES	EPA_PyErr_GivenExceptionMatches;
    PYERR_EXCEPTION_MATCHES		    EPA_PyErr_ExceptionMatches;
    PYERR_NORMALIZE_EXCEPTION		EPA_PyErr_NormalizeException;
    PYEXC_EXCEPTION 			    EPA_PyExc_Exception;
    PYEXC_EXCEPTION                 EPA_PyExc_StandardError;
    PYEXC_EXCEPTION 			    EPA_PyExc_ArithmeticError;
    PYEXC_EXCEPTION 			    EPA_PyExc_LookupError;
    PYEXC_EXCEPTION			        EPA_PyExc_AssertionError;
    PYEXC_EXCEPTION 			    EPA_PyExc_AttributeError;
    PYEXC_EXCEPTION 			    EPA_PyExc_EOFError;
    PYEXC_EXCEPTION 			    EPA_PyExc_FloatingPointError;
    PYEXC_EXCEPTION 			    EPA_PyExc_EnvironmentError;
    PYEXC_EXCEPTION 			    EPA_PyExc_IOError;
    PYEXC_EXCEPTION 			    EPA_PyExc_OSError;
    PYEXC_EXCEPTION 			    EPA_PyExc_ImportError;
    PYEXC_EXCEPTION 			    EPA_PyExc_IndexError;
    PYEXC_EXCEPTION 			    EPA_PyExc_KeyError;
    PYEXC_EXCEPTION 			    EPA_PyExc_KeyboardInterrupt;
    PYEXC_EXCEPTION 			    EPA_PyExc_MemoryError;
    PYEXC_EXCEPTION 			    EPA_PyExc_NameError;
    PYEXC_EXCEPTION 			    EPA_PyExc_OverflowError;
    PYEXC_EXCEPTION 			    EPA_PyExc_RuntimeError;
    PYEXC_EXCEPTION 			    EPA_PyExc_NotImplementedError;
    PYEXC_EXCEPTION 			    EPA_PyExc_SyntaxError;
    PYEXC_EXCEPTION 			    EPA_PyExc_SystemError;
    PYEXC_EXCEPTION 			    EPA_PyExc_SystemExit;
    PYEXC_EXCEPTION 			    EPA_PyExc_TypeError;
    PYEXC_EXCEPTION 			    EPA_PyExc_ValueError;
    PYEXC_EXCEPTION 			    EPA_PyExc_ZeroDivisionError;
    PYEXC_EXCEPTION 			    EPA_PyExc_MemoryErrorInst;
    PYERR_BAD_ARGUMENT			    EPA_PyErr_BadArgument;
    PYERR_NO_MEMORY			        EPA_PyErr_NoMemory;
    PYERR_SET_FROM_ERRNO		    EPA_PyErr_SetFromErrno;
    PYERR_SET_FROM_ERRNO_WITH_FILENAME	EPA_PyErr_SetFromErrnoWithFilename;
    PYERR_BAD_INTERNAL_CALL		    EPA_PyErr_BadInternalCall;
    PYERR_NEW_EXCEPTION			    EPA_PyErr_NewException;
    PYERR_CHECK_SIGNALS             EPA_PyErr_CheckSignals;
    PYERR_SET_INTERRUPT             EPA_PyErr_SetInterrupt;

    // From object.h
    PYTYPE_TYPE                     EPA_PyType_Type;
    PYOBJECT_PRINT                  EPA_PyObject_Print;
    PYOBJECT_REPR                   EPA_PyObject_Repr;
    PYOBJECT_STR                    EPA_PyObject_Str;
    PYOBJECT_COMPARE                EPA_PyObject_Compare;
    PYOBJECT_GET_ATTR_STRING        EPA_PyObject_GetAttrString;
    PYOBJECT_SET_ATTR_STRING        EPA_PyObject_SetAttrString;
    PYOBJECT_HAS_ATTR_STRING        EPA_PyObject_HasAttrString;
    PYOBJECT_GET_ATTR               EPA_PyObject_GetAttr;
    PYOBJECT_SET_ATTR               EPA_PyObject_SetAttr;
    PYOBJECT_HAS_ATTR               EPA_PyObject_HasAttr;
    PYOBJECT_HASH                   EPA_PyObject_Hash;
    PYOBJECT_IS_TRUE                EPA_PyObject_IsTrue;
    PYOBJECT_NOT                    EPA_PyObject_Not;
    PYCALLABLE_CHECK                EPA_PyCallable_Check;
    PYNUMBER_COERCE                 EPA_PyNumber_Coerce;
    PYNUMBER_COERCE_EX              EPA_PyNumber_CoerceEx;
    PY_REPR_ENTER                   EPA_Py_ReprEnter;
    PY_REPR_LEAVE                   EPA_Py_ReprLeave;
    _PY_DEALLOC                     EPA__Py_Dealloc;
    _PY_NEW_REFERENCE               EPA__Py_NewReference;
    _PY_FORGET_REFERENCE            EPA__Py_ForgetReference;
    _PY_PRINT_REFERENCES            EPA__Py_PrintReferences;
    INC_COUNT                       EPA_inc_count;
    _PY_REF_TOTAL                   EPA__Py_RefTotal;
    _PY_NONE_STRUCT                 EPA__Py_NoneStruct;
    PYOBJECT_SELFITER               EPA_PyObject_SelfIter;
    PYOBJECT_FREE                   EPA_PyObject_Free;
    PYOBJECT_RICHCOMPARE            EPA_PyObject_RichCompare;
    PYOBJECT_RICHCOMPAREBOOL        EPA_PyObject_RichCompareBool;
    _PY_NOTIMPLEMENTEDSTRUCT        EPA__NotImplementedStruct;
    _PYMEM_REALLOC                  EPA_PyMem_Realloc;
    PYOBJECT_GENERICSETATTR         EPA_PyObject_GenericSetAttr;
    PYMEM_MALLOC                    EPA_PyMem_Malloc;
    PYMEM_FREE                      EPA_PyMem_Free;
    _PYOBJECT_DUMP                  EPA__PyObject_Dump;

    // From objimpl.h
    _PYOBJECT_NEW                   EPA__PyObject_New;
    _PYOBJECT_NEW_VAR               EPA__PyObject_NewVar;

    // From dictobject.h
    PY_DICT_TYPE                    EPA_PyDict_Type;
    PYDICT_NEW                      EPA_PyDict_New;
    PYDICT_GET_ITEM                 EPA_PyDict_GetItem;
    PYDICT_SET_ITEM                 EPA_PyDict_SetItem;
    PYDICT_DEL_ITEM                 EPA_PyDict_DelItem;
    PYDICT_CLEAR                    EPA_PyDict_Clear;
    PYDICT_NEXT                     EPA_PyDict_Next;
    PYDICT_KEYS                     EPA_PyDict_Keys;
    PYDICT_VALUES                   EPA_PyDict_Values;
    PYDICT_ITEMS                    EPA_PyDict_Items;
    PYDICT_SIZE                     EPA_PyDict_Size;
    PYDICT_GET_ITEM_STRING          EPA_PyDict_GetItemString;
    PYDICT_SET_ITEM_STRING          EPA_PyDict_SetItemString;
    PYDICT_DEL_ITEM_STRING          EPA_PyDict_DelItemString;

    // From methodobject.h
    PY_CFUNCTION_TYPE               EPA_PyCFunction_Type;
    PYCFUNCTION_GET_FUNCTION        EPA_PyCFunction_GetFunction;
    PYCFUNCTION_GET_SELF            EPA_PyCFunction_GetSelf;
    PYCFUNCTION_GET_FLAGS           EPA_PyCFunction_GetFlags;
    PY_FIND_METHOD                  EPA_Py_FindMethod;
    PYCFUNCTION_NEW                 EPA_PyCFunction_New;
    PY_FIND_METHOD_IN_CHAIN         EPA_Py_FindMethodInChain;

    // From intobject.h
    PYINT_TYPE                      EPA_PyInt_Type;
    PYINT_FROM_LONG                 EPA_PyInt_FromLong;
    PYINT_AS_LONG                   EPA_PyInt_AsLong;
    PYINT_GET_MAX                   EPA_PyInt_GetMax;
    PYINTOBJECT                     EPA__Py_ZeroStruct;
    PYINTOBJECT                     EPA__Py_TrueStruct;
    PYOS_STROUL                     EPA_PyOS_strtoul;
    PYOS_STROL                      EPA_PyOS_strtol;

    // From listobject.h
    PYLIST_TYPE                     EPA_PyList_Type;
    PYLIST_NEW                      EPA_PyList_New;
    PYLIST_SIZE                     EPA_PyList_Size;
    PYLIST_GET_ITEM                 EPA_PyList_GetItem;
    PYLIST_SET_ITEM                 EPA_PyList_SetItem;
    PYLIST_INSERT                   EPA_PyList_Insert;
    PYLIST_APPEND                   EPA_PyList_Append;
    PYLIST_GET_SLICE                EPA_PyList_GetSlice;
    PYLIST_SET_SLICE                EPA_PyList_SetSlice;
    PYLIST_SORT                     EPA_PyList_Sort;
    PYLIST_REVERSE                  EPA_PyList_Reverse;
    PYLIST_AS_TUPLE                 EPA_PyList_AsTuple;

    // From moduleobject.h
    PYMODULE_TYPE                   EPA_PyModule_Type;
    PYMODULE_NEW                    EPA_PyModule_New;
    PYMODULE_GET_DICT               EPA_PyModule_GetDict;
    PYMODULE_GET_NAME               EPA_PyModule_GetName;
    PYMODULE_GET_FILENAME           EPA_PyModule_GetFilename;
    _PYMODULE_CLEAR                 EPA__PyModule_Clear;

    // From complexobject.h
    C_SUM                           EPA_c_sum;
    C_DIFF                          EPA_c_diff;
    C_NEG                           EPA_c_neg;
    C_PROD                          EPA_c_prod;
    C_QUOT                          EPA_c_quot;
    C_POW                           EPA_c_pow;
    PYCOMPLEX_TYPE                  EPA_PyComplex_Type;
    PYCOMPLEX_FROM_C_COMPLEX        EPA_PyComplex_FromCComplex;
    PYCOMPLEX_FROM_DOUBLES          EPA_PyComplex_FromDoubles;
    PYCOMPLEX_REAL_AS_DOUBLE        EPA_PyComplex_RealAsDouble;
    PYCOMPLEX_IMAG_AS_DOUBLE        EPA_PyComplex_ImagAsDouble;
    PYCOMPLEX_AS_C_COMPLEX          EPA_PyComplex_AsCComplex;

    // From floatobject.c
    PYFLOAT_TYPE                    EPA_PyFloat_Type;
    PYFLOAT_FROM_DOUBLE             EPA_PyFloat_FromDouble;
    PYFLOAT_AS_DOUBLE               EPA_PyFloat_AsDouble;
    _PYFLOAT_PACK8                  EPA__PyFloat_Pack8;
    _PYFLOAT_UNPACK8                EPA__PyFloat_Unpack8;
    _PYFLOAT_UNPACK4                EPA__PyFloat_Unpack4;
    _PYFLOAT_PACK4                  EPA__PyFloat_Pack4;

    // From stringobject.h
    PYSTRING_TYPE                   EPA_PyString_Type;
    PYSTRING_FROM_STRING_AND_SIZE   EPA_PyString_FromStringAndSize;
    PYSTRING_FROM_STRING            EPA_PyString_FromString;
    PYSTRING_SIZE                   EPA_PyString_Size;
    PYSTRING_AS_STRING              EPA_PyString_AsString;
    PYSTRING_CONCAT                 EPA_PyString_Concat;
    PYSTRING_CONCAT_AND_DEL         EPA_PyString_ConcatAndDel;
    PYSTRING_RESIZE                 EPA__PyString_Resize;
    PYSTRING_FORMAT                 EPA_PyString_Format;
    PYSTRING_INTERN_IN_PLACE        EPA_PyString_InternInPlace;
    PYSTRING_INTERN_FROM_STRING     EPA_PyString_InternFromString;
    PYSTRING_DECODEESCAPE           EPA_PyString_DecodeEscape;
    PYSTRING_ASSTRINGANDSIZE        EPA_PyString_AsStringAndSize;

    // From longobject.h
    PYLONG_TYPE                     EPA_PyLong_Type;
    PYLONG_FROM_LONG                EPA_PyLong_FromLong;
    PYLONG_FROM_UNSIGNED_LONG       EPA_PyLong_FromUnsignedLong;
    PYLONG_FROM_DOUBLE              EPA_PyLong_FromDouble;
    PYLONG_AS_LONG                  EPA_PyLong_AsLong;
    PYLONG_AS_UNSIGNED_LONG         EPA_PyLong_AsUnsignedLong;
    PYLONG_AS_DOUBLE                EPA_PyLong_AsDouble;
    PYLONG_FROM_VOID_PTR            EPA_PyLong_FromVoidPtr;
    PYLONG_AS_VOID_PTR              EPA_PyLong_AsVoidPtr;
    PYLONG_FROM_LONG_LONG           EPA_PyLong_FromLongLong;
    PYLONG_FROM_UNSIGNED_LONG_LONG  EPA_PyLong_FromUnsignedLongLong;
    PYLONG_AS_LONG_LONG             EPA_PyLong_AsLongLong;
    PYLONG_AS_UNSIGNED_LONG_LONG    EPA_PyLong_AsUnsignedLongLong;
    PYLONG_FROM_STRING              EPA_PyLong_FromString;
    _PYLONG_ASBYTEARRAY             EPA__PyLong_AsByteArray;
    _PYLONG_NUMBITS                 EPA__PyLong_NumBits;
    _PYLONG_SIGN                    EPA__PyLong_Sign;
    _PYLONG_FROMBYTEARRAY           EPA__PyLong_FromByteArray;
    _PYLONG_ASSCALEDDOUBLE          EPA__PyLong_AsScaledDouble;

    // From abstract.h
    PYOBJECT_CMP                    EPA_PyObject_Cmp;
    PYOBJECT_CALL_OBJECT            EPA_PyObject_CallObject;
    PYOBJECT_TYPE                   EPA_PyObject_Type;
    PYOBJECT_LENGTH                 EPA_PyObject_Length;
    PYOBJECT_GET_ITEM               EPA_PyObject_GetItem;
    PYOBJECT_SET_ITEM               EPA_PyObject_SetItem;
    PYOBJECT_DEL_ITEM               EPA_PyObject_DelItem;
    PYNUMBER_CHECK                  EPA_PyNumber_Check;
    PYNUMBER_ADD                    EPA_PyNumber_Add;
    PYNUMBER_SUBTRACT               EPA_PyNumber_Subtract;
    PYNUMBER_MULTIPLY               EPA_PyNumber_Multiply;
    PYNUMBER_DIVIDE                 EPA_PyNumber_Divide;
    PYNUMBER_REMAINDER              EPA_PyNumber_Remainder;
    PYNUMBER_DIVMOD                 EPA_PyNumber_Divmod;
    PYNUMBER_POWER                  EPA_PyNumber_Power;
    PYNUMBER_NEGATIVE               EPA_PyNumber_Negative;
    PYNUMBER_POSITIVE               EPA_PyNumber_Positive;
    PYNUMBER_ABSOLUTE               EPA_PyNumber_Absolute;
    PYNUMBER_INVERT                 EPA_PyNumber_Invert;
    PYNUMBER_LSHIFT                 EPA_PyNumber_Lshift;
    PYNUMBER_RSHIFT                 EPA_PyNumber_Rshift;
    PYNUMBER_AND                    EPA_PyNumber_And;
    PYNUMBER_XOR                    EPA_PyNumber_Xor;
    PYNUMBER_OR                     EPA_PyNumber_Or;
    PYNUMBER_INT                    EPA_PyNumber_Int;
    PYNUMBER_LONG                   EPA_PyNumber_Long;
    PYNUMBER_FLOAT                  EPA_PyNumber_Float;
    PYSEQUENCE_CHECK                EPA_PySequence_Check;
    PYSEQUENCE_LENGTH               EPA_PySequence_Length;
    PYSEQUENCE_CONCAT               EPA_PySequence_Concat;
    PYSEQUENCE_REPEAT               EPA_PySequence_Repeat;
    PYSEQUENCE_GET_ITEM             EPA_PySequence_GetItem;
    PYSEQUENCE_GET_SLICE            EPA_PySequence_GetSlice;
    PYSEQUENCE_SET_ITEM             EPA_PySequence_SetItem;
    PYSEQUENCE_DEL_ITEM             EPA_PySequence_DelItem;
    PYSEQUENCE_SET_SLICE            EPA_PySequence_SetSlice;
    PYSEQUENCE_DEL_SLICE            EPA_PySequence_DelSlice;
    PYSEQUENCE_TUPLE                EPA_PySequence_Tuple;
    PYSEQUENCE_LIST                 EPA_PySequence_List;
    PYSEQUENCE_COUNT                EPA_PySequence_Count;
    PYSEQUENCE_CONTAINS             EPA_PySequence_Contains;
    PYSEQUENCE_INDEX                EPA_PySequence_Index;
    PYMAPPING_CHECK                 EPA_PyMapping_Check;
    PYMAPPING_LENGTH                EPA_PyMapping_Length;
    PYMAPPING_HAS_KEY_STRING        EPA_PyMapping_HasKeyString;
    PYMAPPING_HAS_KEY               EPA_PyMapping_HasKey;
    PYMAPPING_GET_ITEM_STRING       EPA_PyMapping_GetItemString;
    PYMAPPING_SET_ITEM_STRING       EPA_PyMapping_SetItemString;
    PYITER_NEXT                     EPA_PyIter_Next;
    PYOBJECT_GETITER                EPA_PyObject_GetIter;
    PYOBJECT_CALL                   EPA_PyObject_Call;
    PYOBJECT_ASREADBUFFER           EPA_PyObject_AsReadBuffer;
    PYNUMBER_FLOORDIVIDE            EPA_PyNumber_FloorDivide;
    PYNUMBER_TRUEDIVIDE             EPA_PyNumber_TrueDivide;
    PYSEQUENCE_FAST                 EPA_PySequence_Fast;

    // From tupleobject.h
    PYTUPLE_TYPE                    EPA_PyTuple_Type;
    PYTUPLE_NEW                     EPA_PyTuple_New;
    PYTUPLE_SIZE                    EPA_PyTuple_Size;
    PYTUPLE_GET_ITEM                EPA_PyTuple_GetItem;
    PYTUPLE_SET_ITEM                EPA_PyTuple_SetItem;
    PYTUPLE_GET_SLICE               EPA_PyTuple_GetSlice;
    PYTUPLE_RESIZE                  EPA_PyTuple_Resize;
    PYTUPLE_PACK                    EPA_PyTuple_Pack;

    // From classobject.h
    PYCLASS_TYPE                    EPA_PyClass_Type;
    PYINSTANCE_TYPE                 EPA_PyInstance_Type;
    PYMETHOD_TYPE                   EPA_PyMethod_Type;
    PYCLASS_NEW                     EPA_PyClass_New;
    PYINSTANCE_NEW                  EPA_PyInstance_New;
    PYMETHOD_NEW                    EPA_PyMethod_New;
    PYMETHOD_FUNCTION               EPA_PyMethod_Function;
    PYMETHOD_SELF                   EPA_PyMethod_Self;
    PYMETHOD_CLASS                  EPA_PyMethod_Class;
    PYCLASS_IS_SUBCLASS             EPA_PyClass_IsSubclass;
    PYINSTANCE_NEWRAW               EPA_PyInstance_NewRaw;
    _PYINSTANCE_LOOKUP              EPA__PyInstance_Lookup;

    // From funcobject.h
    PYFUNCTION_TYPE                 EPA_PyFunction_Type;
    PYFUNCTION_NEW                  EPA_PyFunction_New;
    PYFUNCTION_GET_CODE             EPA_PyFunction_GetCode;
    PYFUNCTION_GET_GLOBALS          EPA_PyFunction_GetGlobals;
    PYFUNCTION_GET_DEFAULTS         EPA_PyFunction_GetDefaults;
    PYFUNCTION_SET_DEFAULTS         EPA_PyFunction_SetDefaults;

    // From fileobject.h
    PYFILE_TYPE                     EPA_PyFile_Type;
    PYFILE_FROM_STRING              EPA_PyFile_FromString;
    PYFILE_SET_BUF_SIZE             EPA_PyFile_SetBufSize;
    PYFILE_FROM_FILE                EPA_PyFile_FromFile;
    PYFILE_AS_FILE                  EPA_PyFile_AsFile;
    PYFILE_NAME                     EPA_PyFile_Name;
    PYFILE_GET_LINE                 EPA_PyFile_GetLine;
    PYFILE_WRITE_OBJECT             EPA_PyFile_WriteObject;
    PYFILE_SOFT_SPACE               EPA_PyFile_SoftSpace;
    PYFILE_WRITE_STRING             EPA_PyFile_WriteString;
    PYOBJECT_ASFILEDESCRIPTOR       EPA_PyObject_AsFileDescriptor;

    // From sysmodule.h
    PYSYS_GET_OBJECT                EPA_PySys_GetObject;
    PYSYS_SET_OBJECT                EPA_PySys_SetObject;
    PYSYS_GET_FILE                  EPA_PySys_GetFile;
    PYSYS_SET_ARGV                  EPA_PySys_SetArgv;
    PYSYS_SET_PATH                  EPA_PySys_SetPath;
    PYSYS_WRITE_STDOUT              EPA_PySys_WriteStdout;
    PYSYS_WRITESTDERR               EPA_PySys_WriteStderr;

    // From ceval.h
    PYEVAL_CALL_OBJECT_WITH_KEYWORDS EPA_PyEval_CallObjectWithKeywords;
    PYEVAL_CALL_OBJECT              EPA_PyEval_CallObject;
    PYEVAL_CALL_FUNCTION            EPA_PyEval_CallFunction;
    PYEVAL_CALL_METHOD              EPA_PyEval_CallMethod;
    PYEVAL_GET_BUILTINS             EPA_PyEval_GetBuiltins;
    PYEVAL_GET_GLOBALS              EPA_PyEval_GetGlobals;
    PYEVAL_GET_LOCALS               EPA_PyEval_GetLocals;
    PYEVAL_GET_FRAME                EPA_PyEval_GetFrame;
    PYEVAL_GET_RESTRICTED           EPA_PyEval_GetRestricted;
    PYEVAL_FLUSH_LINE               EPA_Py_FlushLine;
    PYEVAL_ADD_PENDING_CALL         EPA_Py_AddPendingCall;
    PYEVAL_MAKE_PENDING_CALLS       EPA_Py_MakePendingCalls;
    PYEVAL_SAVE_THREAD              EPA_PyEval_SaveThread;
    PYEVAL_RESTORE_THREAD           EPA_PyEval_RestoreThread;
    PYEVAL_INIT_THREADS             EPA_PyEval_InitThreads;
    PYEVAL_ACQUIRE_LOCK             EPA_PyEval_AcquireLock;
    PYEVAL_RELEASE_LOCK             EPA_PyEval_ReleaseLock;
    PYEVAL_ACQUIRE_THREAD           EPA_PyEval_AcquireThread;
    PYEVAL_RELEASE_THREAD           EPA_PyEval_ReleaseThread;
    PY_GETRECURSIONLIMIT            EPA_Py_GetRecursionLimit;    

    // From import.h
    PYIMPORT_GET_MAGIC_NUMBER       EPA_PyImport_GetMagicNumber;
    PYIMPORT_EXEC_CODE_MODULE       EPA_PyImport_ExecCodeModule;
    PYIMPORT_EXEC_CODE_MODULE_EX    EPA_PyImport_ExecCodeModuleEx;
    PYIMPORT_GET_MODULE_DICT        EPA_PyImport_GetModuleDict;
    PYIMPORT_ADD_MODULE             EPA_PyImport_AddModule;
    PYIMPORT_IMPORT_MODULE          EPA_PyImport_ImportModule;
    PYIMPORT_IMPORT_MODULE_EX       EPA_PyImport_ImportModuleEx;
    PYIMPORT_IMPORT                 EPA_PyImport_Import;
    PYIMPORT_RELOAD_MODULE          EPA_PyImport_ReloadModule;
    PYIMPORT_CLEANUP                EPA_PyImport_Cleanup;
    PYIMPORT_IMPORT_FROZEN_MODULE   EPA_PyImport_ImportFrozenModule;
    PYIMPORT_FIND_EXTENSION         EPA__PyImport_FindExtension;
    PYIMPORT_FIXUP_EXTENSION        EPA__PyImport_FixupExtension;
    PYIMPORT_INITTAB                EPA_PyImport_Inittab;
    PYIMPORT_APPEND_INITTAB         EPA_PyImport_AppendInittab;
    PYIMPORT_EXTEND_INITTAB         EPA_PyImport_ExtendInittab;
    PYIMPORT_FROZEN_MODULES         EPA_PyImport_FrozenModules;

    // From pythonrun.h
    PY_SET_PROGRAM_NAME             EPA_Py_SetProgramName;
    PY_GET_PROGRAM_NAME             EPA_Py_GetProgramName;
    PY_SET_PYTHON_HOME              EPA_Py_SetPythonHome;
    PY_GET_PYTHON_HOME              EPA_Py_GetPythonHome;
    PY_INITIALIZE                   EPA_Py_Initialize;
    PY_FINALIZE                     EPA_Py_Finalize;
    PY_IS_INITIALIZED               EPA_Py_IsInitialized;
    PY_NEW_INTERPRETER              EPA_Py_NewInterpreter;
    PY_END_INTERPRETER              EPA_Py_EndInterpreter;
    PYRUN_ANY_FILE                  EPA_PyRun_AnyFile;
    PYRUN_SIMPLE_STRING             EPA_PyRun_SimpleString;
    PYRUN_SIMPLE_FILE               EPA_PyRun_SimpleFile;
    PYRUN_INTERACTIVE_ONE           EPA_PyRun_InteractiveOne;
    PYRUN_INTERACTIVE_LOOP          EPA_PyRun_InteractiveLoop;
    PYPARSER_SIMPLE_PARSE_STRING    EPA_PyParser_SimpleParseString;
    PYPARSER_SIMPLE_PARSE_FILE      EPA_PyParser_SimpleParseFile;
    PYRUN_STRING                    EPA_PyRun_String;
    PYRUN_FILE                      EPA_PyRun_File;
    PY_COMPILE_STRING               EPA_Py_CompileString;
    PYERR_PRINT                     EPA_PyErr_Print;
    PYERR_PRINT_EX                  EPA_PyErr_PrintEx;
    PY_AT_EXIT                      EPA_Py_AtExit;
    PY_EXIT                         EPA_Py_Exit;
    PY_FD_IS_INTERACTIVE            EPA_Py_FdIsInteractive;
    PY_GET_PROGRAM_FULL_PATH        EPA_Py_GetProgramFullPath;
    PY_GET_PREFIX                   EPA_Py_GetPrefix;
    PY_GET_EXEC_PREFIX              EPA_Py_GetExecPrefix;
    PY_GET_PATH                     EPA_Py_GetPath;
    PY_GET_VERSION                  EPA_Py_GetVersion;
    PY_GET_PLATFORM                 EPA_Py_GetPlatform;
    PY_GET_COPYRIGHT                EPA_Py_GetCopyright;
    PY_GET_COMPILER                 EPA_Py_GetCompiler;
    PY_GET_BUILD_INFO               EPA_Py_GetBuildInfo;
    PYSYS_INIT                      EPA__PySys_Init;
    PYIMPORT_INIT                   EPA__PyImport_Init;
    PYIMPORT_FINI                   EPA__PyImport_Fini;
    PYMETHOD_FINI                   EPA_PyMethod_Fini;
    PYFRAME_FINI                    EPA_PyFrame_Fini;
    PYCFUNCTION_FINI                EPA_PyCFunction_Fini;
    PYTUPLE_FINI                    EPA_PyTuple_Fini;
    PYSTRING_FINI                   EPA_PyString_Fini;
    PYINT_FINI                      EPA_PyInt_Fini;
    PYFLOAT_FINI                    EPA_PyFloat_Fini;
    PYOS_FINI_INTERRUPTS            EPA_PyOS_FiniInterrupts;
    PYOS_READLINE                   EPA_PyOS_Readline;
    PYOS_INPUT_HOOK                 EPA_PyOS_InputHook;
    PYOS_READLINE_FUNCTION_POINTER  EPA_PyOS_ReadlineFunctionPointer;

    // From pydebug.h
    PY_FLAG                         EPA_Py_DebugFlag;
    PY_FLAG                         EPA_Py_VerboseFlag;
    PY_FLAG                         EPA_Py_InteractiveFlag;
    PY_FLAG                         EPA_Py_OptimizeFlag;
    PY_FLAG                         EPA_Py_NoSiteFlag;
    PY_FLAG                         EPA_Py_UseClassExceptionsFlag;
    PY_FLAG                         EPA_Py_FrozenFlag;
    PY_FLAG                         EPA_Py_TabcheckFlag;
    PY_FLAG                         EPA_Py_IgnoreEnvironmentFlag;
    PY_FATAL_ERROR                  EPA_Py_FatalError;

    //From cobject.h
    PYCOBJECT_TYPE                  EPA_PyCObject_Type;
    PYCOBJECT_FROM_VOID_PTR         EPA_PyCObject_FromVoidPtr;
    PYCOBJECT_FROM_VOID_PTR_AND_DESC EPA_PyCObject_FromVoidPtrAndDesc;
    PYCOBJECT_AS_VOID_PTR           EPA_PyCObject_AsVoidPtr;
    PYCOBJECT_GET_DESC              EPA_PyCObject_GetDesc;
    PYCOBJECT_IMPORT                EPA_PyCObject_Import;

    // From compile.h
    PYCODE_TYPE                     EPA_PyCode_Type;
    PYCODE_COMPILE                  EPA_PyNode_Compile;
    PYCODE_NEW                      EPA_PyCode_New; 
    PYCODE_ADDR_2_LINE              EPA_PyCode_Addr2Line;

    // From node.h
    PYNODE_NEW                      EPA_PyNode_New;
    PYNODE_ADD_CHILD                EPA_PyNode_AddChild;
    PYNODE_FREE                     EPA_PyNode_Free;
    PYNODE_LIST_TREE                EPA_PyNode_ListTree;

    // From typeobject.h
    PYTYPE_GENERICALLOC             EPA_PyType_GenericAlloc;
    PYOBJECT_GENERICGETATTR         EPA_PyObject_GenericGetAttr;
    PYTYPE_ISSUBTYPE                EPA_PyType_IsSubtype;
    PYTYPE_READY                    EPA_PyType_Ready;
    PYTYPE_GENERICNEW               EPA_PyType_GenericNew;

    // From weakrefobject.h
    PYOBJECT_CLEARWEAKREFS          EPA_PyObject_ClearWeakRefs;
    PYWEAKREF_REFTYPE               EPA_PyWeakref_RefType;
    PYWEAKREF_PROXYTYPE             EPA_PyWeakref_ProxyType;
    PYWEAKREF_CALLABLEPROXYTYPE     EPA_PyWeakref_CallableProxyType;
    _PYWEAKREF_CLEARREF             EPA__PyWeakref_ClearRef;


    // mysnprintf
    PYOS_SNPRINTF                   EPA_PyOS_snprintf;

    // sliceobject
    PY_ELLIPSISOBJECT               EPA__Py_EllipsisObject;
    PYSLICE_GETINDICESEX            EPA_PySlice_GetIndicesEx;
    PYSLICE_TYPE                    EPA_PySlice_Type;

    // getargs
    PYARG_NOKEYWORDS                EPA_PyArg_NoKeywords;
    PYARG_UNPACKTUPLE               EPA_PyArg_UnpackTuple;

    // gcmodule
    PYOBJECT_GC_TRACK               EPA_PyObject_GC_Track;
    PYOBJECT_GC_NEW                 EPA_PyObject_GC_New;
    PYOBJECT_GC_DEL                 EPA_PyObject_GC_Del;
    PYOBJECT_GC_UNTRACK             EPA_PyObject_GC_UnTrack;

    // boolobject
    PYBOOL_FROMLONG                 EPA_PyBool_FromLong;

    // pystrtod
    PYOS_ASCII_STRTOD               EPA_PyOS_ascii_strtod;

    // errors
    PYERR_WARN                      EPA_PyErr_Warn;
    PYERR_WRITEUNRAISABLE           EPA_PyErr_WriteUnraisable;

    // structseq
    PYSTRUCTSEQUENCE_NEW            EPA_PyStructSequence_New;
    PYSTRUCTSEQUENCE_INITTYPE       EPA_PyStructSequence_InitType;

} EFI_PYTHON_API;

#endif
