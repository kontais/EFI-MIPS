#include <stdio.h>

extern int MemCpy(void *d,void *s, unsigned int c);

char a[] = "0123456789abcdefghijklmnopqrstuvwxyz";
char b[] = "zyxwvutsrqponmlkjihgfedcba9876543210";

int main()
{
  printf("RetValue = 0x%x\n",MemCpy(a+5,a,10));
  printf("%s\n",a);
  return 0;
}
