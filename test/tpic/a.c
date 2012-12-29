#include "stdio.h"

extern int funcc();
extern int funcb();
int OsPrint(char *Str)
{
  printf("%s\n",Str);
  return 0;
}
int main()
{
  funcc();
  funcb();
  printf("main OK\n");
  return 0;
}
