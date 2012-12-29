
/* System module interface */

#ifndef Py_SYSMODULE_H
#define Py_SYSMODULE_H
#ifdef __cplusplus
extern "C" {
#endif

PyAPI_FUNC(PyObject *) PySys_GetObject(char *);
PyAPI_FUNC(int) PySys_SetObject(char *, PyObject *);
PyAPI_FUNC(FILE *) PySys_GetFile(char *, FILE *);
PyAPI_FUNC(void) PySys_SetArgv(int, char **);
PyAPI_FUNC(void) PySys_SetPath(char *);

PyAPI_FUNC(void) PySys_WriteStdout(const char *format, ...)
			Py_GCC_ATTRIBUTE((format(printf, 1, 2)));
PyAPI_FUNC(void) PySys_WriteStderr(const char *format, ...)
			Py_GCC_ATTRIBUTE((format(printf, 1, 2)));

#ifndef EFI_LOADABLE_MODULE
PyAPI_DATA(PyObject *) _PySys_TraceFunc, *_PySys_ProfileFunc;
PyAPI_DATA(int) _PySys_CheckInterval;
#else
PyAPI_DATA(PyObject) **_pPySys_TraceFunc, **_pPySys_ProfileFunc;
PyAPI_DATA(int) *_pPySys_CheckInterval;

#define _PySys_TraceFunc        (*_pPySys_TraceFunc)
#define _PySys_ProfileFunc      (*_pPySys_ProfileFunc)
#define _PySys_CheckInterval    (*_pPySys_CheckInterval)
#endif

PyAPI_FUNC(void) PySys_ResetWarnOptions(void);
PyAPI_FUNC(void) PySys_AddWarnOption(char *);

#ifdef __cplusplus
}
#endif
#endif /* !Py_SYSMODULE_H */
