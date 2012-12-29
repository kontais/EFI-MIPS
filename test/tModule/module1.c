#include "moduleall.h"
#include "Tiano.h"
#include "SystemLib.h"

int gVar0 = 0;
int gVar1;

typedef int (*FUNC)(void);
FUNC m2init = module2init;

int moduleEntry(unsigned int Ppi[])
{
  int Status;
  EFI_SYSTEM_LIB_PROTOCOL *mSys;

  gVar0 = 0x1234;
  gVar1 = 0xabcdef;
  module2init();

  mSys = ( EFI_SYSTEM_LIB_PROTOCOL *) Ppi[0];
  Status = mSys->printf("Module Test Successfully!\n");
  return Status;
}
