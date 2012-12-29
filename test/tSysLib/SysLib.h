#ifndef _SYS_LIB_H_
#define _SYS_LIB_H_

typedef unsigned char    CHAR8;
typedef int              INTN;
typedef unsigned int      UINTN;
typedef unsigned char     BOOLEAN;

#define FALSE 0
#define TRUE  1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <sys/types.h>
#include <wait.h>
#include <dirent.h>

//#define _MAX_PATH PATH_MAX
#define _MAX_PATH 1024
#define MAX_PATH _MAX_PATH

#define _strcmpi strcasecmp
#define _stricmp strcasecmp

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
void _strlwr (CHAR8* s);

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


#endif /* _SYS_LIB_H_ */
