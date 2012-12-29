#include <stdio.h>
#include "EfiBind.h"
#include "EfiTypes.h"

extern int SecMain(void);

#define U64ADDRESS(bus, dev, func, reg)  ( \
      (UINT64) ((((UINTN) bus) << 24) + (((UINTN) dev) << 16) + (((UINTN) func) << 8) + ((UINTN) reg)) \
    ) & 0x00000000ffffffff

#define PCI32ADDRESS(x) \
(0x080000000 | (((UINTN)x & 0x0FF000000) >> 8) | (((UINTN)x & 0x0FF0000)>> 5) | (x & 0x0700) | (x & 0x0FC))

int main()
{
  int ReturnValue;
  UINT64 Address;


  Address = U64ADDRESS(1,2,3,4);

  ReturnValue = SecMain();

  printf("ReturnValue = 0x%x\n", ReturnValue);
  printf("sizeof(UINT64) = %d\n", sizeof(UINT64));
  printf("Address = 0x%llx\n", Address);
  Address = 0x123456789;
  printf("Address = 0x%llx\n", Address);
  Address = U64ADDRESS(1,2,3,4);
  printf("Address = 0x%llx\n", PCI32ADDRESS(Address));
  return ReturnValue;
}
