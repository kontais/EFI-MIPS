/*
 * Copyright (c) 2006
 * Intel Corporation.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 
 * 3. All advertising materials mentioning features or use of this software must
 *    display the following acknowledgement:
 * 
 *    This product includes software developed by Intel Corporation and its
 *    contributors.
 * 
 * 4. Neither the name of Intel Corporation or its contributors may be used to
 *    endorse or promote products derived from this software without specific
 *    prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY INTEL CORPORATION AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL INTEL CORPORATION OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */

/*
 * This file sets up the funtion table that provides the API to
 *  the core of the Python implementation for dynamically
 *  loaded modules under EFI.
 */

#include "EfiInterface.h"


EFI_PYTHON_API EfiPythonApi = {
    EFI_PYTHON_API_REVISION,

    // From modsupport.h
    PyArg_VaParse,
    PyArg_VaParseTupleAndKeywords,
    Py_VaBuildValue,
    Py_InitModule4,
    &_Py_PackageContext,
    PyArg_ParseEfi,
    PyModule_AddObject,
    PyModule_AddIntConstant,
    PyModule_AddStringConstant,

    // From pyerrors.h
    PyErr_SetNone,
    PyErr_SetObject,
    PyErr_SetString,
    PyErr_Occurred,
    PyErr_Clear,
    PyErr_Fetch,
    PyErr_Restore,
    PyErr_GivenExceptionMatches,
    PyErr_ExceptionMatches,
    PyErr_NormalizeException,
    &PyExc_Exception,
    &PyExc_StandardError,
    &PyExc_ArithmeticError,
    &PyExc_LookupError,
    &PyExc_AssertionError,
    &PyExc_AttributeError,
    &PyExc_EOFError,
    &PyExc_FloatingPointError,
    &PyExc_EnvironmentError,
    &PyExc_IOError,
    &PyExc_OSError,
    &PyExc_ImportError,
    &PyExc_IndexError,
    &PyExc_KeyError,
    &PyExc_KeyboardInterrupt,
    &PyExc_MemoryError,
    &PyExc_NameError,
    &PyExc_OverflowError,
    &PyExc_RuntimeError,
    &PyExc_NotImplementedError,
    &PyExc_SyntaxError,
    &PyExc_SystemError,
    &PyExc_SystemExit,
    &PyExc_TypeError,
    &PyExc_ValueError,
    &PyExc_ZeroDivisionError,
    &PyExc_MemoryErrorInst,
    PyErr_BadArgument,
    PyErr_NoMemory,
    PyErr_SetFromErrno,
    PyErr_SetFromErrnoWithFilename,
    PyErr_BadInternalCall,
    PyErr_NewException,
    NULL,   // Place holder for PyErr_CheckSignals
    NULL,   // Place holder for PyErr_SetInterrupt

    // From object.h
    &PyType_Type,
    PyObject_Print,
    PyObject_Repr,
    PyObject_Str,
    PyObject_Compare,
    PyObject_GetAttrString,
    PyObject_SetAttrString,
    PyObject_HasAttrString,
    PyObject_GetAttr,
    PyObject_SetAttr,
    PyObject_HasAttr,
    PyObject_Hash,
    PyObject_IsTrue,
    PyObject_Not,
    PyCallable_Check,
    PyNumber_Coerce,
    PyNumber_CoerceEx,
    Py_ReprEnter,
    Py_ReprLeave,
#ifdef Py_TRACE_REFS
    _Py_Dealloc,
    _Py_NewReference,
    _Py_ForgetReference,
    _Py_PrintReferences,
#else
    NULL,
    NULL,
    NULL,
    NULL,
#endif
#ifdef COUNT_ALLOCS
    inc_count,
#else
    NULL,
#endif
#ifdef Py_REF_DEBUG
    &_Py_RefTotal,
#else
    NULL,
#endif
    &_Py_NoneStruct,
    PyObject_SelfIter,
    PyObject_Free,
    PyObject_RichCompare,
    PyObject_RichCompareBool,
    &_Py_NotImplementedStruct,
    PyMem_Realloc,
    PyObject_GenericSetAttr,
    PyMem_Malloc,
    PyMem_Free,
    _PyObject_Dump,

    // From objimpl.h
    _PyObject_New,
    _PyObject_NewVar,

    // From dictobject.h
    &PyDict_Type,
    PyDict_New,
    PyDict_GetItem,
    PyDict_SetItem,
    PyDict_DelItem,
    PyDict_Clear,
    PyDict_Next,
    PyDict_Keys,
    PyDict_Values,
    PyDict_Items,
    PyDict_Size,
    PyDict_GetItemString,
    PyDict_SetItemString,
    PyDict_DelItemString,

    // From methodobject.h
    &PyCFunction_Type,
    PyCFunction_GetFunction,
    PyCFunction_GetSelf,
    PyCFunction_GetFlags,
    Py_FindMethod,
    PyCFunction_New,
    Py_FindMethodInChain,

    // From intobject.h
    &PyInt_Type,
    PyInt_FromLong,
    PyInt_AsLong,

    PyInt_GetMax,
    &_Py_ZeroStruct,
    &_Py_TrueStruct,
    PyOS_strtoul,
    PyOS_strtol,

    // From listobject.h
    &PyList_Type,
    PyList_New,
    PyList_Size,
    PyList_GetItem,
    PyList_SetItem,
    PyList_Insert,
    PyList_Append,
    PyList_GetSlice,
    PyList_SetSlice,
    PyList_Sort,
    PyList_Reverse,
    PyList_AsTuple,

    // From moduleobject.h
    &PyModule_Type,
    PyModule_New,
    PyModule_GetDict,
    PyModule_GetName,
    PyModule_GetFilename,
    _PyModule_Clear,

    // From complexobject.h
    c_sum,
    c_diff,
    c_neg,
    c_prod,
    c_quot,
    c_pow,
    &PyComplex_Type,
    PyComplex_FromCComplex,
    PyComplex_FromDoubles,
    PyComplex_RealAsDouble,
    PyComplex_ImagAsDouble,
    PyComplex_AsCComplex,

    // From floatobject.c
    &PyFloat_Type,
    PyFloat_FromDouble,
    PyFloat_AsDouble,
    _PyFloat_Pack8,
    _PyFloat_Unpack8,
    _PyFloat_Unpack4,
    _PyFloat_Pack4,

    // From stringobject.h
    &PyString_Type,
    PyString_FromStringAndSize,
    PyString_FromString,
    PyString_Size,
    PyString_AsString,
    PyString_Concat,
    PyString_ConcatAndDel,
    _PyString_Resize,
    PyString_Format,
#ifdef INTERN_STRINGS
    PyString_InternInPlace,
    PyString_InternFromString,
#else
    NULL,
    NULL,
#endif
    PyString_DecodeEscape,
    PyString_AsStringAndSize,

    // From longobject.h
    &PyLong_Type,
    PyLong_FromLong,
    PyLong_FromUnsignedLong,
    PyLong_FromDouble,
    PyLong_AsLong,
    PyLong_AsUnsignedLong,
    PyLong_AsDouble,
    PyLong_FromVoidPtr,
    PyLong_AsVoidPtr,
#ifdef HAVE_LONG_LONG
    PyLong_FromLongLong,
    PyLong_FromUnsignedLongLong,
    PyLong_AsLongLong,
    PyLong_AsUnsignedLongLong,
#else
    NULL,
    NULL,
    NULL,
    NULL,
#endif
    PyLong_FromString,
    _PyLong_AsByteArray,
    _PyLong_NumBits,
    _PyLong_Sign,
    _PyLong_FromByteArray,
    _PyLong_AsScaledDouble,

    // From abstract.h
    PyObject_Cmp,
    PyObject_CallObject,
    PyObject_Type,
    PyObject_Length,
    PyObject_GetItem,
    PyObject_SetItem,
    PyObject_DelItem,
    PyNumber_Check,
    PyNumber_Add,
    PyNumber_Subtract,
    PyNumber_Multiply,
    PyNumber_Divide,
    PyNumber_Remainder,
    PyNumber_Divmod,
    PyNumber_Power,
    PyNumber_Negative,
    PyNumber_Positive,
    PyNumber_Absolute,
    PyNumber_Invert,
    PyNumber_Lshift,
    PyNumber_Rshift,
    PyNumber_And,
    PyNumber_Xor,
    PyNumber_Or,
    PyNumber_Int,
    PyNumber_Long,
    PyNumber_Float,
    PySequence_Check,
    PySequence_Length,
    PySequence_Concat,
    PySequence_Repeat,
    PySequence_GetItem,
    PySequence_GetSlice,
    PySequence_SetItem,
    PySequence_DelItem,
    PySequence_SetSlice,
    PySequence_DelSlice,
    PySequence_Tuple,
    PySequence_List,
    PySequence_Count,
    PySequence_Contains,
    PySequence_Index,
    PyMapping_Check,
    PyMapping_Length,
    PyMapping_HasKeyString,
    PyMapping_HasKey,
    PyMapping_GetItemString,
    PyMapping_SetItemString,
    PyIter_Next,
    PyObject_GetIter,
    PyObject_Call,
    PyObject_AsReadBuffer,
    PyNumber_FloorDivide,
    PyNumber_TrueDivide,
    PySequence_Fast,

    // From tupleobject.h
    &PyTuple_Type,
    PyTuple_New,
    PyTuple_Size,
    PyTuple_GetItem,
    PyTuple_SetItem,
    PyTuple_GetSlice,
    _PyTuple_Resize,
    PyTuple_Pack,

    // From classobject.h
    &PyClass_Type,
    &PyInstance_Type,
    &PyMethod_Type,
    PyClass_New,
    PyInstance_New,
    PyMethod_New,
    PyMethod_Function,
    PyMethod_Self,
    PyMethod_Class,
    PyClass_IsSubclass,
    PyInstance_NewRaw,
    _PyInstance_Lookup,

    // From funcobject.h
    &PyFunction_Type,
    PyFunction_New,
    PyFunction_GetCode,
    PyFunction_GetGlobals,
    PyFunction_GetDefaults,
    PyFunction_SetDefaults,

    // From fileobject.h
    &PyFile_Type,
    PyFile_FromString,
    PyFile_SetBufSize,
    PyFile_FromFile,
    PyFile_AsFile,
    PyFile_Name,
    PyFile_GetLine,
    PyFile_WriteObject,
    PyFile_SoftSpace,
    PyFile_WriteString,
    PyObject_AsFileDescriptor,

    // From sysmodule.h
    PySys_GetObject,
    PySys_SetObject,
    PySys_GetFile,
    PySys_SetArgv,
    PySys_SetPath,
    PySys_WriteStdout,
    PySys_WriteStderr,
//    &_PySys_TraceFunc,
//    &_PySys_ProfileFunc,
//    &_PySys_CheckInterval,

    // From ceval.h
    PyEval_CallObjectWithKeywords,
    PyEval_CallObject,
    PyEval_CallFunction,
    PyEval_CallMethod,
    PyEval_GetBuiltins,
    PyEval_GetGlobals,
    PyEval_GetLocals,
//    PyEval_GetOwner,
    PyEval_GetFrame,
    PyEval_GetRestricted,
    Py_FlushLine,
    Py_AddPendingCall,
    Py_MakePendingCalls,
    PyEval_SaveThread,
    PyEval_RestoreThread,
#ifdef WITH_THREAD
    PyEval_InitThreads,
    PyEval_AcquireLock,
    PyEval_ReleaseLock,
    PyEval_AcquireThread,
    PyEval_ReleaseThread,
#else
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
#endif
    Py_GetRecursionLimit,

    // From import.h
    PyImport_GetMagicNumber,
    PyImport_ExecCodeModule,
    PyImport_ExecCodeModuleEx,
    PyImport_GetModuleDict,
    PyImport_AddModule,
    PyImport_ImportModule,
    PyImport_ImportModuleEx,
    PyImport_Import,
    PyImport_ReloadModule,
    PyImport_Cleanup,
    PyImport_ImportFrozenModule,
    _PyImport_FindExtension,
    _PyImport_FixupExtension,
    &PyImport_Inittab,
    PyImport_AppendInittab,
    PyImport_ExtendInittab,
    &PyImport_FrozenModules,

    // From pythonrun.h
    Py_SetProgramName,
    Py_GetProgramName,
    Py_SetPythonHome,
    Py_GetPythonHome,
    Py_Initialize,
    Py_Finalize,
    Py_IsInitialized,
    Py_NewInterpreter,
    Py_EndInterpreter,
    PyRun_AnyFile,
    PyRun_SimpleString,
    PyRun_SimpleFile,
    PyRun_InteractiveOne,
    PyRun_InteractiveLoop,
    PyParser_SimpleParseString,
    PyParser_SimpleParseFile,
    PyRun_String,
    PyRun_File,
    Py_CompileString,
    PyErr_Print,
    PyErr_PrintEx,
    Py_AtExit,
    Py_Exit,
    Py_FdIsInteractive,
    Py_GetProgramFullPath,
    Py_GetPrefix,
    Py_GetExecPrefix,
    Py_GetPath,
    Py_GetVersion,
    Py_GetPlatform,
    Py_GetCopyright,
    Py_GetCompiler,
    Py_GetBuildInfo,
    _PySys_Init,
    _PyImport_Init,
    _PyImport_Fini,
    PyMethod_Fini,
    PyFrame_Fini,
    PyCFunction_Fini,
    PyTuple_Fini,
    PyString_Fini,
    PyInt_Fini,
    PyFloat_Fini,
    PyOS_FiniInterrupts,
    PyOS_Readline,
    &PyOS_InputHook,
    &PyOS_ReadlineFunctionPointer,

    // From pydebug.h
    &Py_DebugFlag,
    &Py_VerboseFlag,
    &Py_InteractiveFlag,
    &Py_OptimizeFlag,
    &Py_NoSiteFlag,
    &Py_UseClassExceptionsFlag,
    &Py_FrozenFlag,
    &Py_TabcheckFlag,
    &Py_IgnoreEnvironmentFlag,
    Py_FatalError,

    //From cobject.h
    &PyCObject_Type,
    PyCObject_FromVoidPtr,
    PyCObject_FromVoidPtrAndDesc,
    PyCObject_AsVoidPtr,
    PyCObject_GetDesc,
    PyCObject_Import,

    // From compile.h
    &PyCode_Type,
    PyNode_Compile,
    PyCode_New,
    PyCode_Addr2Line,

    // From node.h
    PyNode_New,
    PyNode_AddChild,
    PyNode_Free,
    PyNode_ListTree,

    // From typeobject.h
    PyType_GenericAlloc,
    PyObject_GenericGetAttr,
    PyType_IsSubtype,
    PyType_Ready,
    PyType_GenericNew,

    // From weakrefobject.h
    PyObject_ClearWeakRefs,
    &_PyWeakref_RefType,
    &_PyWeakref_ProxyType,
    &_PyWeakref_CallableProxyType,
    _PyWeakref_ClearRef,

    // mysnprintf
    PyOS_snprintf,

    // sliceobject
    &_Py_EllipsisObject,
    PySlice_GetIndicesEx,
    &PySlice_Type,

    // getargs
    _PyArg_NoKeywords,
    PyArg_UnpackTuple,

    // gcmodule
    PyObject_GC_Track,
    _PyObject_GC_New,
    PyObject_GC_Del,
    PyObject_GC_UnTrack,

    // boolobject
    PyBool_FromLong,

    // pystrtod
    PyOS_ascii_strtod,

    // errors
    PyErr_Warn,
    PyErr_WriteUnraisable,

    // structseq
    PyStructSequence_New,
    PyStructSequence_InitType,
};
