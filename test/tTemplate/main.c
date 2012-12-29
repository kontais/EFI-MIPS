#include "Tiano.h"
#include "stdio.h"
#include "EfiCommonLib.h"

int main()
{
  CHAR8     TempBuffer[30];
  UINT64    Value = 100;
  UINTN     Flags;
  UINTN     Width;

  //Flags |= LONG_TYPE;
  Flags = 0;
  Width = 0;
  ValueToString (TempBuffer, Value, Flags, Width);
  printf("%s\n",TempBuffer);
  return 0;
}

