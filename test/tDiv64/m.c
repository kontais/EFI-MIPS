#include "stdio.h"

typedef unsigned char UINT8;
typedef unsigned int UINTN;
typedef unsigned int UINT32;
typedef unsigned long long    UINT64;


extern UINT64 DivU64x32(UINT64 Dividend, UINTN Divisor, UINTN * Rema);
extern UINT64 MultU64x32(UINT64 m1, UINTN m2);
extern UINT8  Log2(UINT64 m1);
extern UINT64  LShiftU64(UINT64 m1, UINTN s2);
extern UINT64  RShiftU64(UINT64 m1, UINTN s2);
extern UINT64  GetPowerOfTwo(UINT64 m1);

#if 0
UINT64 TestDivU64(UINT64 Dividend, UINTN Divisor, UINTN * Rema)
{
  return 0x1234567890;
}
#endif

UINT64 TestMultU64(UINT64 m1, UINTN m2)
{
  return 0x1234567890;
}

int main()
{
  UINT64 ReturnValue;
  UINT32 Rm;

  //ReturnValue = MultU64x32(0x100000004, 2);
  //ReturnValue = Log2(0x100000000);
  //ReturnValue = Log2(0x5);
  //ReturnValue = Log2(0x8000);
  //ReturnValue = LShiftU64(0x9, 32);
  //ReturnValue = RShiftU64(ReturnValue, 32);
  ReturnValue = GetPowerOfTwo(5);

  //ReturnValue = DivU64x32(0x100000007, 0x10000000, 0);

  return ReturnValue;
  //return Rm;
}


