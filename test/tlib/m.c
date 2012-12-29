#include <stdio.h>
#include "SimpleCpuIoLib.h"

extern int SecMain(void);


int main()
{
  int ReturnValue;

  ReturnValue = SecMain();

  printf("ReturnValue = 0x%x\n", ReturnValue);

  // only use CpuIoLib.o
  //printf("ReturnValue = 0x%x\n", CpuIoRead8(0x70));

  // only use SimpleCpuMemLib.o
  //printf("ReturnValue = 0x%x\n", MemRead8(0x70));

  // use CpuIoLib.o and SimpleCpuIoLib.o
  printf("ReturnValue = 0x%x\n", IoRead8(0x70));
  return ReturnValue;
}
