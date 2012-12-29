/*
 * gcc -o a m.c
 * gcc -march=loongson2e -o a m.c
 */

#include <stdio.h>


typedef unsigned long long UINT64;
typedef unsigned int       UINTN;
typedef unsigned int       UINT32;


int main()
{
  UINT64 ReturnValue;
  
  UINT64 Dividend64 = 101;
  UINTN  Divisor  = 10;
  UINTN  Remainder;

  UINT32 Dividend32;
 
  Dividend32 = (UINT32) Dividend64;

  Remainder = Dividend32 % Divisor;

  ReturnValue = (UINT64) (Dividend32 / Divisor);

  printf("%Ld(/%d) = %Ld * %d + %d\n",
      Dividend64,
      Divisor,
      ReturnValue,
      Divisor,
      Remainder);

  return 0;
}

