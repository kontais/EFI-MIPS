#include <stdio.h>

extern int GetWord(void *a);

char a[] = "0123456789abcdefghijklmnopqrstuvwxyz";

int main()
{
  printf("0x%x\n",GetWord(a+0));
  printf("0x%x\n",GetWord(a+1));
  printf("0x%x\n",GetWord(a+2));
  printf("0x%x\n",GetWord(a+3));
  printf("0x%x\n",GetWord(a+4));
  return 0;
}
