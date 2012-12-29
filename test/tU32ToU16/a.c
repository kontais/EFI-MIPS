#include "stdio.h"

typedef unsigned int u32;
typedef unsigned short u16;
int main()
{
  u32 a = 0x12345678;
  u16 b = 0;

  b = (u16) a;

  printf("0x%x\n", b);
  return 0;
}

