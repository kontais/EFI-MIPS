#include <stdio.h>

extern int GetCP0(int reg);
int main()
{
  int i;
  printf("SecMain\n");
  for (i = 0; i< 31; i++)
  {
   printf("CP0 $%02d = 0x%x\n",i,GetCP0(i));
  }
  return 0;
}
