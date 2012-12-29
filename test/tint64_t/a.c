#include <stdio.h>

#ifdef __GNUC__
typedef int __attribute__((__mode__(__DI__)))    __int64_t;
typedef unsigned int __attribute__((__mode__(__DI__)))  __uint64_t;
#else
typedef __int64            __int64_t;
typedef unsigned __int64        __uint64_t;
#endif


int main()
{
  printf("sizeof __int64_t = %d\n", sizeof(__int64_t));
  return 0;
}

