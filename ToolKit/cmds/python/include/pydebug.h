
#ifndef Py_PYDEBUG_H
#define Py_PYDEBUG_H
#ifdef __cplusplus
extern "C" {
#endif

#ifndef EFI_LOADABLE_MODULE
PyAPI_DATA(int) Py_DebugFlag;
PyAPI_DATA(int) Py_VerboseFlag;
PyAPI_DATA(int) Py_InteractiveFlag;
PyAPI_DATA(int) Py_OptimizeFlag;
PyAPI_DATA(int) Py_NoSiteFlag;
PyAPI_DATA(int) Py_UseClassExceptionsFlag;
PyAPI_DATA(int) Py_FrozenFlag;
PyAPI_DATA(int) Py_TabcheckFlag;
PyAPI_DATA(int) Py_UnicodeFlag;
PyAPI_DATA(int) Py_IgnoreEnvironmentFlag;
PyAPI_DATA(int) Py_DivisionWarningFlag;
#else
PyAPI_DATA(int) *pPy_DebugFlag;
PyAPI_DATA(int) *pPy_VerboseFlag;
PyAPI_DATA(int) *pPy_InteractiveFlag;
PyAPI_DATA(int) *pPy_OptimizeFlag;
PyAPI_DATA(int) *pPy_NoSiteFlag;
PyAPI_DATA(int) *pPy_UseClassExceptionsFlag;
PyAPI_DATA(int) *pPy_FrozenFlag;
PyAPI_DATA(int) *pPy_TabcheckFlag;
PyAPI_DATA(int) *pPy_UnicodeFlag;
PyAPI_DATA(int) *pPy_IgnoreEnvironmentFlag;
PyAPI_DATA(int) *pPy_DivisionWarningFlag;

#define Py_DebugFlag        (*pPy_DebugFlag)
#define Py_VerboseFlag      (*pPy_VerboseFlag)
#define Py_InteractiveFlag  (*pPy_InteractiveFlag)
#define Py_OptimizeFlag     (*pPy_OptimizeFlag)
#define Py_NoSiteFlag       (*pPy_NoSiteFlag)
#define Py_UseClassExceptionsFlag   (*pPy_UseClassExceptionsFlag)
#define Py_FrozenFlag       (*pPy_FrozenFlag)
#define Py_TabcheckFlag     (*pPy_TabcheckFlag)
#define Py_UnicodeFlag      (*pPy_UnicodeFlag)
#define Py_IgnoreEnvironmentFlag    (*pPy_IgnoreEnvironmentFlag)
#define Py_DivisionWarningFlag      (*pPy_DivisionWarningFlag)

#endif

/* _XXX Py_QnewFlag should go away in 3.0.  It's true iff -Qnew is passed,
  on the command line, and is used in 2.2 by ceval.c to make all "/" divisions
  true divisions (which they will be in 3.0). */
PyAPI_DATA(int) _Py_QnewFlag;

/* this is a wrapper around getenv() that pays attention to
   Py_IgnoreEnvironmentFlag.  It should be used for getting variables like
   PYTHONPATH and PYTHONHOME from the environment */
#define Py_GETENV(s) (Py_IgnoreEnvironmentFlag ? NULL : getenv(s))

PyAPI_FUNC(void) Py_FatalError(const char *message);

#ifdef __cplusplus
}
#endif
#endif /* !Py_PYDEBUG_H */
