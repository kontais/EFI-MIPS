#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "EfiCommon.h"

UINT32 Index;
UINT32 Data[100];


UINTN
Print (
  IN CHAR16     *fmt,
  ...
  )

{
  va_list  args;
  CHAR16 *pc;
  UINT64  uv;
  INT64   sv;

  __builtin_va_start(args,fmt);




  pc = fmt;
  while (*pc) {
    switch (*pc) {
     case 'x':


      uv = __builtin_va_arg(args,UINTN);
      Data[Index++] = (UINT32)uv;
      Data[Index++] = (UINT32)(uv>>32);
      break;

    case 'd':


      sv = __builtin_va_arg(args,UINT64);
      Data[Index++] = (UINT32)sv;
      Data[Index++] = (UINT32)(sv>>32);
      break;
    }

    pc++;
  }



  return 0;
}












int
main (
  )

{


  UINTN  BufSize;
  UINT64 FileSize;
  UINT32 i;



  BufSize = 100;
  FileSize = 4681;

  Print(L"xd", BufSize, FileSize);

  Print(L"d", FileSize);

  Print(L"xd", BufSize, FileSize);

  for (i = 0; i < Index; i++) {
    printf("%d\n", Data[i]);
  }


  return 0;
}

