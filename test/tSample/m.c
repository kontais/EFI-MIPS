#include <stdio.h>
extern int SecMain(void);


int main()
{
  int ReturnValue;

  ReturnValue = SecMain();

  printf("ReturnValue = 0x%x\n", ReturnValue);
  return ReturnValue;
}
