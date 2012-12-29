#include "stdio.h"

typedef unsigned char UINT8;
typedef unsigned int UINTN;
typedef unsigned int UINT32;
typedef unsigned long long    UINT64;


UINT64 MultU64x64 (UINT64 Value1,  UINT64 Value2,  UINT64 *ResultHigh);
UINT64 DivU64x64 (UINT64 Dividend, UINT64 Divisor, UINT64 *Remainder, UINT32 *Error);

UINT64 DivU64x64C (UINT64 Dividend, UINT64 Divisor, UINT64 *Remainder, UINT32 *Error)
{
  *Error = 0;
  if (Divisor == 0) {
    *Error = 1;
    if (Remainder) {
        *Remainder = 0x80000000;
    }
    return 0x80000000;
  }


  return 0;
}

int main()
{
  UINT64 V1;
  UINT64 V2;
  UINT64 Hi;
  UINT64 RetVal;
  UINT32 Error;

  V1 = 0xffffffff;
  V1 = (V1 << 32) + 0xffffffff;

  V2 = 0x0;
  V2 = (V2 << 32) + 0x2;


  //RetVal= MultU64x64 (V1, V2, &Hi);
  //RetVal= DivU64x64 (V1, V2, &Hi, &Error);
  RetVal= DivU64x64 (V1, V2, &Hi, &Error);


  printf("V1 = 0x%08x", (UINT32)(V1 >> 32));
  printf("%08x\n", (UINT32)V1);

  printf("V2 = 0x%08x", (UINT32)(V2 >> 32));
  printf("%08x\n", (UINT32)V2);

  printf("rt = 0x%08x", (UINT32)(RetVal >> 32));
  printf("%08x\n", (UINT32)RetVal);

  printf("Hi = 0x%08x", (UINT32)(Hi >> 32));
  printf("%08x\n", (UINT32)Hi);

  printf("Er = 0x%08x\n", Error);


  return RetVal;
}


