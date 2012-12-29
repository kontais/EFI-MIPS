/*++

Copyright (c) 1998  Intel Corporation

Module Name:

    math.c

Abstract:

Revision History

--*/

#include "efi_interface.h"

UINT64
LIBC_DivU64x64 (
  IN  UINT64   Dividend,
  IN  UINT64   Divisor,
  OUT UINT64   *Remainder OPTIONAL,
  OUT UINT32   *Error
  );


INT64
LIBC_DivS64x64 (
  IN  INT64   Dividend,
  IN  INT64   Divisor,
  OUT INT64   *Remainder OPTIONAL,
  OUT UINT32  *Error
  )
{
  UINT64   Quotient;
  BOOLEAN  Negative;
  BOOLEAN  RemainderNegative;

  Negative = FALSE;
  RemainderNegative = FALSE;
  if (Dividend < 0) {
    Dividend = -Dividend;
    Negative = (BOOLEAN)!Negative;
    RemainderNegative = TRUE;
  }
  if (Divisor < 0) {
    Divisor = -Divisor;
    Negative = (BOOLEAN)!Negative;
  }
  Quotient = LIBC_DivU64x64 ((UINT64)Dividend, (UINT64)Divisor, (INT64 *)Remainder, Error);
  if (*Error) {
    return (INT64)Quotient;
  }
  if (Negative) {
    Quotient = -((INT64)Quotient);
  }
  if (RemainderNegative) {
    if (Remainder) {
      *Remainder = -*Remainder;
    }
  }
  return (INT64)Quotient;
}

