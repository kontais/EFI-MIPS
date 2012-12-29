/* Boolean object interface */

#ifndef Py_BOOLOBJECT_H
#define Py_BOOLOBJECT_H
#ifdef __cplusplus
extern "C" {
#endif


typedef PyIntObject PyBoolObject;

#ifndef EFI_LOADABLE_MODULE
PyAPI_DATA(PyTypeObject) PyBool_Type;
#else
PyAPI_DATA(PyTypeObject) *pPyBool_Type;
#define PyBool_Type (*pPyBool_Type)
#endif

#define PyBool_Check(x) ((x)->ob_type == &PyBool_Type)

/* Py_False and Py_True are the only two bools in existence.
Don't forget to apply Py_INCREF() when returning either!!! */

#ifndef EFI_LOADABLE_MODULE
/* Don't use these directly */
PyAPI_DATA(PyIntObject) _Py_ZeroStruct, _Py_TrueStruct;

/* Use these macros */
#define Py_False ((PyObject *) &_Py_ZeroStruct)
#define Py_True ((PyObject *) &_Py_TrueStruct)
#else
PyAPI_DATA(PyIntObject) *_pPy_ZeroStruct, *_pPy_TrueStruct;
#define _Py_ZeroStruct (*_pPy_ZeroStruct)
#define _Py_TrueStruct (*_pPy_TrueStruct)

#define Py_False ((PyObject *) _pPy_ZeroStruct)
#define Py_True ((PyObject *)  _pPy_TrueStruct)
#endif



/* Macros for returning Py_True or Py_False, respectively */
#define Py_RETURN_TRUE return Py_INCREF(Py_True), Py_True
#define Py_RETURN_FALSE return Py_INCREF(Py_False), Py_False

/* Function to return a bool from a C long */
PyAPI_FUNC(PyObject *) PyBool_FromLong(long);

#ifdef __cplusplus
}
#endif
#endif /* !Py_BOOLOBJECT_H */
