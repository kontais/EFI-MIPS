#ifndef _LIBC_H_
#define _LIBC_H_

#include <types.h>






#define EOF  (-1)

/* definitions for fmt parameter of str_fmt(p,width,fmt) */
#define FMT_RJUST 0
#define FMT_LJUST 1
#define FMT_RJUST0 2
#define FMT_CENTER 3



#define	MIN(a,b) (((a)<(b))?(a):(b))
#define	MAX(a,b) (((a)>(b))?(a):(b))




void	 free __P((void *));
void	*malloc __P((size_t));

int	printf	__P((const char *, ...));
int	sprintf	__P((char *, const char *, ...));

int	vfprintf __P((int *, const char *, VA_LIST));
int	vprintf	__P((const char *, VA_LIST));
int	vsprintf __P((char *, const char *, VA_LIST));



void	bzero __P((void *, size_t));
char	*strcat __P((char *, const char *));
char	*strchr __P((const char *, int));
char	*strncpy __P((char *, const char *, size_t));
char	*strcpy __P((char *, const char *));
size_t	strlen __P((const char *));
char	*strichr __P((char *, int));
void	bcopy __P((const char *, char *, size_t));
void	str_fmt __P((char *, int, int));
void	strtoupper __P((char *));
int     toupper(int c);
int strcmp(const char *s1, const char *s2);

void * memset(void *dst0, int c0, size_t length);


int atob(UINT32 *vp, char *p, int base);
char * btoa(char *dst, UINTN value, int base);

int vfprintf (int *fp, const char *fmt, VA_LIST ap);

void *pmalloc __P((size_t ));
void pfree __P((void * ));


extern const char	*_ctype_;


#define	isdigit(c)	((_ctype_ + 1)[(UINT8)(c)] & _N)
#define	islower(c)	((_ctype_ + 1)[(UINT8)(c)] & _L)
#define	isspace(c)	((_ctype_ + 1)[(UINT8)(c)] & _S)
#define	ispunct(c)	((_ctype_ + 1)[(UINT8)(c)] & _P)
#define	isupper(c)	((_ctype_ + 1)[(UINT8)(c)] & _U)
#define	isalpha(c)	((_ctype_ + 1)[(UINT8)(c)] & (_U|_L))
#define	isxdigit(c)	((_ctype_ + 1)[(UINT8)(c)] & (_N|_X))
#define	isalnum(c)	((_ctype_ + 1)[(UINT8)(c)] & (_U|_L|_N))
#define	isprint(c)	((_ctype_ + 1)[(UINT8)(c)] & (_P|_U|_L|_N|_B))
#define	isgraph(c)	((_ctype_ + 1)[(UINT8)(c)] & (_P|_U|_L|_N))
#define	iscntrl(c)	((_ctype_ + 1)[(UINT8)(c)] & _C)



#endif // _LIBC_H_
