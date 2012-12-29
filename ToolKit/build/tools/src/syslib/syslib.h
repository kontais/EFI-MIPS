#ifndef _SYS_LIB_H_
#define _SYS_LIB_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>   // PATH_MAX = 4096 too big
#include <unistd.h>
#include <sys/types.h>
#include <wait.h>
#include <dirent.h>
#include <wchar.h>
#include <stdarg.h>

#include "efi.h"

//#define _MAX_PATH PATH_MAX
#define _MAX_PATH 1024
#define MAX_PATH _MAX_PATH

#define _strcmpi strcasecmp
#define _stricmp strcasecmp
#define _strnicmp strncasecmp


typedef struct _LIN32_FIND_DATA {
  __mode_t   dwFileAttributes;
  char       cFileName[ _MAX_PATH ];
  char       DirPath[ _MAX_PATH ];
} LIN32_FIND_DATA, *PLIN32_FIND_DATA, *LPLIN32_FIND_DATA;

//
// implement WIN32 _spawnv
//
#define _P_WAIT 0x00000001

//
// implement _strlwr
//
void _strlwr (char* s);

//
// get pos form fpos_t
//
#define _FPOSOFF(fp) ((fp).__pos)

//
// get FILE * stream file size
//
UINTN GetFileSize(FILE *fp);

//
// implement Find Function
//
DIR *   FindFirstFile(char * Path, LIN32_FIND_DATA *FindData);
BOOLEAN FindNextFile(DIR *DirHandle, LIN32_FIND_DATA *FindData);
void    FindClose(DIR *DirHandle);

//
// libc wcscmp error if use -fshort-wchar and string not 4 byte align
//
int _wcscmp(wchar_t *s1, wchar_t *s2);
int _wcslen(wchar_t *s);
int _wcsncmp (wchar_t *s1, wchar_t *s2, size_t n);
wchar_t *_wcsncpy(wchar_t *dest, const wchar_t *src, size_t n);
wchar_t *_wcscat(wchar_t *dest, const wchar_t *src);
wchar_t *_wcscpy(wchar_t *dest, wchar_t *src);


//
// Print primitives
//
#define LEFT_JUSTIFY  0x01
#define PREFIX_SIGN   0x02
#define PREFIX_BLANK  0x04
#define COMMA_TYPE    0x08
#define LONG_TYPE     0x10
#define PREFIX_ZERO   0x20

//
// Length of temp string buffer to store value string.
//
#define CHARACTER_NUMBER_FOR_VALUE  30

int _swprintf(wchar_t * Buffer, size_t BufferSize, const wchar_t *Format, ...);
int _vswprintf(wchar_t *StartOfBuffer, size_t BufferSize, const wchar_t *FormatString, va_list Marker);
UINTN EfiValueToString (wchar_t *Buffer, INT64  Value, UINTN Flags, UINTN Width);
UINTN EfiValueToHexStr (wchar_t  *Buffer, UINT64 Value, UINTN Flags, UINTN Width);
UINT64 RShiftU64 (UINT64 Operand, UINTN Count);
UINT64 DivU64x32 (UINT64 Dividend, UINTN Divisor, UINTN *Remainder);
wchar_t *AsciiToUnicode (char *Ascii, UINTN *StrLen);
char *UnicodeToAscii (wchar_t *Unicode);

#endif /* _SYS_LIB_H_ */
