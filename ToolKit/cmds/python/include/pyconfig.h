/* pyconfig.h This is a manually generated version for EFI  */

#ifndef Py_PYCONFIG_H
#define Py_PYCONFIG_H

/* Define if type char is unsigned and you are not using gcc.  */
#ifndef __CHAR_UNSIGNED__
#undef __CHAR_UNSIGNED__
#endif

/* Set the platform ID string */
#ifdef EFI32
# define PLATFORM     "EFI32"
# define COMPILER     "[MSC 32 bit (Intel)]"
#else
# ifdef EFI64
#  define PLATFORM    "EFI64"
#  ifdef _MSC_VER
#   define COMPILER   "[MSC 64 bit (Intel)]"
#  else
#   define COMPILER   "[Intel 64 bit]"
#  endif
# else 
#  ifdef EFIX64
#   define PLATFORM   "EFIX64"
#   define COMPILER   "[MS WINDDK X64 (Intel)]"
#  else
#   ifdef  
#     define PLATFORM "EBC"
#     define COMPILER "[Intel EBC]"
#   else
#     error "Platform other than EFI32, EFI64, EFIX64 and EBC defined"
#   endif
#  endif /* EFIX64 */
# endif /* EFI64 */
#endif /* EFI32 */


#define VERSION "2.4"

#define PREFIX "/TOOLKIT"

#define PYTHONPATH PREFIX "/python" VERSION ";" \
		   PREFIX "/python" VERSION "/lib"

/* Define to empty if the keyword does not work.  */
#undef const

/* Define to `int' if <sys/types.h> doesn't define.  */
#undef gid_t

/* Define if your struct tm has tm_zone.  */
#define HAVE_TM_ZONE 1

/* Define if you don't have tm_zone but do have the external array
   tzname.  */
#undef HAVE_TZNAME

/* Define if on MINIX.  */
#undef _MINIX

/* Define to `int' if <sys/types.h> doesn't define.  */
#undef mode_t

/* Define to `long' if <sys/types.h> doesn't define.  */
#undef off_t

/* Define to `int' if <sys/types.h> doesn't define.  */
#undef pid_t

/* Define if the system does not provide POSIX.1 features except
   with this defined.  */
#undef _POSIX_1_SOURCE

/* Define if you need to in order for stat and other things to work.  */
#undef _POSIX_SOURCE

/* Define as the return type of signal handlers (int or void).  */
#define RETSIGTYPE void

/* Define to `unsigned' if <sys/types.h> doesn't define.  */
#undef size_t

/* Define to `long' if <time.h> doesn't define.  */
#undef clock_t

/* Define if you have the ANSI C header files.  */
#undef STDC_HEADERS

/* Define if you can safely include both <sys/time.h> and <time.h>.  */
#define TIME_WITH_SYS_TIME 1

/* Define if your <sys/time.h> declares struct tm.  */
#undef TM_IN_SYS_TIME

/* Define to `int' if <sys/types.h> doesn't define.  */
#undef uid_t

/* Define if your <unistd.h> contains bad prototypes for exec*()
   (as it does on SGI IRIX 4.x) */
#undef BAD_EXEC_PROTOTYPES

/* Define if your compiler botches static forward declarations
   (as it does on SCI ODT 3.0) */
#define BAD_STATIC_FORWARD 1

/* Define for AIX if your compiler is a genuine IBM xlC/xlC_r and you want
   support for AIX C++ shared extension modules. */
#undef AIX_GENUINE_CPLUSPLUS

/* Define this if you have AtheOS threads. */
#undef ATHEOS_THREADS

/* Define this if you have BeOS threads */
#undef BEOS_THREADS

/* Define if you have the Mach cthreads package */
#undef C_THREADS

/* Define if --enable-ipv6 is specified */
#undef ENABLE_IPV6

/* Define if getpgrp() must be called as getpgrp(0). */
#undef GETPGRP_HAVE_ARG

/* Define if gettimeofday() does not have second (timezone) argument This is
   the case on Motorola V4 (R40V4.2) */
#undef GETTIMEOFDAY_NO_TZ

/* struct addrinfo (netdb.h) */
/* #undef HAVE_ADDRINFO */

/* Define if you have the alarm function.  */
#undef HAVE_ALARM

/* Define this if your time.h defines altzone */
#undef HAVE_ALTZONE

/* Define to 1 if you have the `bind_textdomain_codeset' function. */
/* #undef HAVE_BIND_TEXTDOMAIN_CODESET */

/* Define to 1 if you have the <bluetooth/bluetooth.h> header file. */
/* #undef HAVE_BLUETOOTH_BLUETOOTH_H */

/* Define to 1 if you have the <bluetooth.h> header file. */
/* #undef HAVE_BLUETOOTH_H */

/* Define this if you have some version of gethostbyname_r() */
#undef HAVE_GETHOSTBYNAME_R

/* Define if poll() sets errno on invalid file descriptors. */
/* #undef HAVE_BROKEN_POLL */

/* Define if the Posix semaphores do not work on your system */
/* #undef HAVE_BROKEN_POSIX_SEMAPHORES */

/* Define if pthread_sigmask() does not work on your system. */
#define HAVE_BROKEN_PTHREAD_SIGMASK 1

/* Define this if you have the 3-arg version of gethostbyname_r() */
#undef HAVE_GETHOSTBYNAME_R_3_ARG

/* Define this if you have the 5-arg version of gethostbyname_r() */
#undef HAVE_GETHOSTBYNAME_R_5_ARG

/* Define this if you have the 6-arg version of gethostbyname_r() */
#undef HAVE_GETHOSTBYNAME_R_6_ARG

/* Define this if you have the type long long */
#ifdef EFI32 /* check here */
#undef HAVE_LONG_LONG
#else
#define HAVE_LONG_LONG 1
#endif

/* MSC does not have type long long, but does have type __int64 */
#if defined(HAVE_LONG_LONG) && defined(_MSC_VER)
#define LONG_LONG __int64
#endif

/* Define this if you have a K&R style C preprocessor */
#undef HAVE_OLD_CPP

/* Define if your compiler supports function prototypes */
#define HAVE_PROTOTYPES 1

/* Define if your compiler supports variable length function prototypes
   (e.g. void fprintf(FILE *, char *, ...);) *and* <stdarg.h> */
#define HAVE_STDARG_PROTOTYPES 1

/* Define if malloc(0) returns a NULL pointer */
#undef MALLOC_ZERO_RETURNS_NULL

/* Define if you have POSIX threads */
#undef _POSIX_THREADS

/* Define to force use of thread-safe errno, h_errno, and other functions */
#undef _REENTRANT

/* Define if setpgrp() must be called as setpgrp(0, 0). */
#undef SETPGRP_HAVE_ARG

/* Define to empty if the keyword does not work.  */
#undef signed

/* Define if  you can safely include both <sys/select.h> and <sys/time.h>
   (which you can't on SCO ODT 3.0). */
#define SYS_SELECT_WITH_SYS_TIME 1

/* Define if a va_list is an array of some kind */
#undef VA_LIST_IS_ARRAY

/* Define to empty if the keyword does not work.  */
#undef volatile

/* Define if you want SIGFPE handled (see Include/pyfpe.h). */
#undef WANT_SIGFPE_HANDLER

/* Define if you want to use SGI (IRIX 4) dynamic linking.
   This requires the "dl" library by Jack Jansen,
   ftp://ftp.cwi.nl/pub/dynload/dl-1.6.tar.Z.
   Don't bother on IRIX 5, it already has dynamic linking using SunOS
   style shared libraries */ 
#undef WITH_SGI_DL

/* Define if you want to emulate SGI (IRIX 4) dynamic linking.
   This is rumoured to work on VAX (Ultrix), Sun3 (SunOS 3.4),
   Sequent Symmetry (Dynix), and Atari ST.
   This requires the "dl-dld" library,
   ftp://ftp.cwi.nl/pub/dynload/dl-dld-1.1.tar.Z,
   as well as the "GNU dld" library,
   ftp://ftp.cwi.nl/pub/dynload/dld-3.2.3.tar.Z.
   Don't bother on SunOS 4 or 5, they already have dynamic linking using
   shared libraries */ 
#undef WITH_DL_DLD

/* Define if you want to use the new-style (Openstep, Rhapsody, MacOS)
   dynamic linker (dyld) instead of the old-style (NextStep) dynamic
   linker (rld). Dyld is necessary to support frameworks. */
#undef WITH_DYLD

/* Define if you want to compile in rudimentary thread support */
#undef WITH_THREAD

/* Define if you want to produce an OpenStep/Rhapsody framework
   (shared library plus accessory files). */
#undef WITH_NEXT_FRAMEWORK

/* The number of bytes in an off_t. */
//#define SIZEOF_OFF_T	sizeof(off_t)
#define SIZEOF_OFF_T	4

/* Defined to enable large file support when an off_t is bigger than a long
   and long long is available and at least as big as an off_t. You may need
   to add some flags for configuration and compilation to enable this mode.
   E.g, for Solaris 2.7:
   CFLAGS="-D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64" OPT="-O2 $CFLAGS" \
 configure
*/
/* EFI env doesn't support large file */
/* #define HAVE_LARGEFILE_SUPPORT 1 */
#undef HAVE_LARGEFILE_SUPPORT

/* The number of bytes in a int.  */
/* The number of bytes in a long.  */
/* check here */
#if defined(EFI32)
    #define SIZEOF_INT  4
    #define SIZEOF_LONG 4
#else
    #if defined(EFI64)
        #define SIZEOF_INT  4
        #define SIZEOF_LONG 4
    #else
        #if defined EFIX64
            #define SIZEOF_INT  4
            #define SIZEOF_LONG 4
        #else
            #undef SIZEOF_INT
            #undef SIZEOF_LONG
        #endif
    #endif
#endif

/* The number of bytes in a long long.  */
#define SIZEOF_LONG_LONG 8

/* The number of bytes in a void *.  */
#ifdef EFI32
  #define SIZEOF_VOID_P 4
#else
  #ifdef EFI64
    #define SIZEOF_VOID_P 8
  #else
    #ifdef EFIX64
      #define SIZEOF_VOID_P 8
    #else
      #undef SIZEOF_VOID_P
    #endif
  #endif
#endif



/* Define if you have the chown function.  */
#undef HAVE_CHOWN

/* Define if you have the clock function.  */
#undef HAVE_CLOCK

/* Define if you have the dlopen function.  */
#undef HAVE_DLOPEN

/* Define if you have the dup2 function.  */
#define HAVE_DUP2 1

/* Define if you have the execv function.  */
#undef HAVE_EXECV

/* Define if you have the fdatasync function.  */
#undef HAVE_FDATASYNC

/* Define if you have the flock function.  */
#undef HAVE_FLOCK

/* Define if you have the fork function.  */
#undef HAVE_FORK

/* Define if you have the fseek64 function.  */
#undef HAVE_FSEEK64

/* Define if you have the fseeko function.  */
#define HAVE_FSEEKO 1

/* Define if you have the fstatvfs function.  */
#undef HAVE_FSTATVFS

/* Define if you have the fsync function.  */
#undef HAVE_FSYNC

/* Define if you have the ftell64 function.  */
#undef HAVE_FTELL64

/* Define if you have the ftello function.  */
#define HAVE_FTELLO 1

/* Define if you have the ftime function.  */
#undef HAVE_FTIME

/* Define if you have the ftruncate function.  */
#undef HAVE_FTRUNCATE

/* Define if you have the getcwd function.  */
#define HAVE_GETCWD 1

/* Define if you have the getpeername function.  */
#define HAVE_GETPEERNAME 1

/* Define if you have the getpgrp function.  */
#undef HAVE_GETPGRP

/* Define if you have the getpid function.  */
#define HAVE_GETPID 1

/* Define if you have the getpwent function.  */
#undef HAVE_GETPWENT

/* Define if you have the gettimeofday function.  */
#define HAVE_GETTIMEOFDAY 1

/* Define if you have the getwd function.  */
#undef HAVE_GETWD

/* Define if you have the hypot function.  */
#undef HAVE_HYPOT

/* Define if you have the kill function.  */
#undef HAVE_KILL

/* Define if you have the link function.  */
#undef HAVE_LINK

/* Define if you have the lstat function.  */
#define HAVE_LSTAT 1

/* Define if you have the memmove function.  */
#define HAVE_MEMMOVE 1

/* Define if you have the mkfifo function.  */
#undef HAVE_MKFIFO

/* Define if you have the mktime function.  */
#define HAVE_MKTIME 1

/* Define if you have the nice function.  */
#undef HAVE_NICE

/* Define if you have the pause function.  */
#undef HAVE_PAUSE

/* Define if you have the plock function.  */
#undef HAVE_PLOCK

/* Define if you have the pthread_init function.  */
#undef HAVE_PTHREAD_INIT

/* Define if you have the putenv function.  */
#define HAVE_PUTENV 1

/* Define if you have the readlink function.  */
#undef HAVE_READLINK

/* Define if you have the select function.  */
#define HAVE_SELECT 1

/* Define if you have the setgid function.  */
#undef HAVE_SETGID

/* Define if you have the setlocale function.  */
#define HAVE_SETLOCALE 1

/* Define if you have the setpgid function.  */
#undef HAVE_SETPGID

/* Define if you have the setpgrp function.  */
#undef HAVE_SETPGRP

/* Define if you have the setsid function.  */
#undef HAVE_SETSID

/* Define if you have the setuid function.  */
#undef HAVE_SETUID

/* Define if you have the setvbuf function.  */
#define HAVE_SETVBUF 1

/* Define if you have the sigaction function.  */
#undef HAVE_SIGACTION

/* Define if you have the siginterrupt function.  */
#undef HAVE_SIGINTERRUPT

/* Define if you have the sigrelse function.  */
#undef HAVE_SIGRELSE

/* Define if you have the statvfs function.  */
#undef HAVE_STATVFS

/* Define if you have the strdup function.  */
#define HAVE_STRDUP 1

/* Define if you have the strerror function.  */
#define HAVE_STRERROR 1

/* Define if you have the strftime function.  */
#define HAVE_STRFTIME 1

/* Define if you have the strptime function.  */
#define HAVE_STRPTIME 1

/* Define if you have the symlink function.  */
#undef HAVE_SYMLINK

/* Define if you have the tcgetpgrp function.  */
#undef HAVE_TCGETPGRP

/* Define if you have the tcsetpgrp function.  */
#undef HAVE_TCSETPGRP

/* Define if you have the timegm function.  */
#define HAVE_TIMEGM 1

/* Define if you have the times function.  */
#undef HAVE_TIMES

/* Define if you have the truncate function.  */
#undef HAVE_TRUNCATE

/* Define if you have the uname function.  */
#undef HAVE_UNAME

/* Define if you have the waitpid function.  */
#undef HAVE_WAITPID

/* Define if you have the <dirent.h> header file.  */
#define HAVE_DIRENT_H 1

/* Define if you have the <dlfcn.h> header file.  */
#undef HAVE_DLFCN_H

/* Define if you have the <fcntl.h> header file.  */
#define HAVE_FCNTL_H 1

/* Define if you have the <limits.h> header file.  */
#define HAVE_LIMITS_H 1

/* Define if you have the <locale.h> header file.  */
#define HAVE_LOCALE_H 1

/* Define if you have the <ncurses.h> header file.  */
#undef HAVE_NCURSES_H

/* Define if you have the <ndir.h> header file.  */
#undef HAVE_NDIR_H

/* Define if you have the <pthread.h> header file.  */
#undef HAVE_PTHREAD_H

/* Define if you have the <signal.h> header file.  */
#undef HAVE_SIGNAL_H

/* Define if you have the <stdarg.h> header file.  */
#define HAVE_STDARG_H 1

/* Define if you have the <stddef.h> header file.  */
#define HAVE_STDDEF_H 1

/* Define if you have the <stdlib.h> header file.  */
#define HAVE_STDLIB_H 1

/* Define if you have the <sys/audioio.h> header file.  */
#undef HAVE_SYS_AUDIOIO_H

/* Define if you have the <sys/dir.h> header file.  */
#undef HAVE_SYS_DIR_H

/* Define if you have the <sys/file.h> header file.  */
#define HAVE_SYS_FILE_H 1

/* Define if you have the <sys/lock.h> header file.  */
#define HAVE_SYS_LOCK_H 1

/* Define if you have the <sys/ndir.h> header file.  */
#undef HAVE_SYS_NDIR_H

/* Define if you have the <sys/param.h> header file.  */
#define HAVE_SYS_PARAM_H 1

/* Define if you have the <sys/select.h> header file.  */
#define HAVE_SYS_SELECT_H 1

/* Define if you have the <sys/time.h> header file.  */
#define HAVE_SYS_TIME_H 1

/* Define if you have the <sys/times.h> header file.  */
#undef HAVE_SYS_TIMES_H

/* Define if you have the <sys/un.h> header file.  */
#undef HAVE_SYS_UN_H

/* Define if you have the <sys/utsname.h> header file.  */
#undef HAVE_SYS_UTSNAME_H

/* Define if you have the <sys/wait.h> header file.  */
#undef HAVE_SYS_WAIT_H

/* Define if you have the <thread.h> header file.  */
#undef HAVE_THREAD_H

/* Define if you have the <unistd.h> header file.  */
#define HAVE_UNISTD_H 1

/* Define if you have the <utime.h> header file.  */
#define HAVE_UTIME_H 1

/* Define if you have the dl library (-ldl).  */
#undef HAVE_LIBDL

/* Define if you have the dld library (-ldld).  */
#undef HAVE_LIBDLD

/* Define if you have the ieee library (-lieee).  */
#undef HAVE_LIBIEEE

/* Define as the integral type used for Unicode representation. */
#define PY_UNICODE_TYPE unsigned short

/* Define as the size of the unicode type. */
#define Py_UNICODE_SIZE 2

/* Define if you want to have a Unicode type. */
#define Py_USING_UNICODE 1

#endif