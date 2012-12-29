#include "moduleall.h"

int gVar0 = 0;
int gVar1;

typedef int (*FUNC)(void);
FUNC m2init = module2init;

int module1init()
{
  gVar0 = 0x1234;
  gVar1 = 0xabcdef;
  module2init();
  return 0;
}
