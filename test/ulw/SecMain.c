#include <stdio.h>
#include <string.h>

extern int GetWord(void *a);
extern int SetWord(void *a, unsigned int v);

char a[36] = "0123456789abcdefghijklmnopqrstuvwxyz";
char b[36] = "000000000000000000000000000000000000";

int main()
{
  printf("0x%x\n",GetWord(a+0));
  printf("0x%x\n",GetWord(a+1));
  printf("0x%x\n",GetWord(a+2));
  printf("0x%x\n",GetWord(a+3));
  printf("0x%x\n",GetWord(a+4));

  memset(b, 0x30, 36);
  SetWord(b+0, 0x39383736);
  printf("%s\n", b);

  memset(b, 0x30, 36);
  SetWord(b+1, 0x39383736);
  printf("%s\n", b);

  memset(b, 0x30, 36);
  SetWord(b+2, 0x39383736);
  printf("%s\n", b);

  memset(b, 0x30, 36);
  SetWord(b+3, 0x39383736);
  printf("%s\n", b);

  memset(b, 0x30, 36);
  SetWord(b+4, 0x39383736);
  printf("%s\n", b);



  return 0;
}
