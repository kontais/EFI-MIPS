#include <stdio.h>
#include "cp0.h"

int main()
{
  int i;
  
  var32 = 0xabcdef01;
  printf("GetCP0 = 0x%x\n",GetCP0(0));

  return 0;
}
