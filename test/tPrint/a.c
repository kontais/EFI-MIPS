#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "EfiCommon.h"

UINT32 getsp(VOID);
UINT32 gets8(VOID);

UINT32 Index;
UINT32 Data[100];


UINTN
Print (
  IN CHAR16     *fmt,
  ...
  )

{
  //CHAR8  *args;
  VA_LIST  args;
  CHAR16 *pc;
  UINT64  uv;
  INT64   sv;

  Data[Index++] =  getsp();
  Data[Index++] =  gets8();

  VA_START (args, fmt);
  //args = (CHAR8 *) & (fmt) + ((sizeof (fmt) + 3) &~(3));
  //args = (CHAR8 *) & (fmt) + 4;

  
  pc = fmt;
  while (*pc) {
    switch (*pc) {
     case 'x':
      //uv = (*(UINTN *) ((args += ((sizeof (UINTN) + 3) &~(3))) - ((sizeof (UINTN) + 3) &~(3))));
      //uv = (*(UINTN *) ((args += 4) - 4));
      uv = VA_ARG (args, UINTN);
      Data[Index++] = (UINT32)uv;
      Data[Index++] = (UINT32)(uv>>32);
      break;

    case 'd':
      //sv = (*(UINT64 *) ((args += ((sizeof (UINT64) + 3) &~(3))) - ((sizeof (UINT64) + 3) &~(3))));
      //sv = (*(UINT64 *) ((args += 8) - 8));

      //args = (void*)((UINT32)(((UINT32) args) + 7) & -8);
      //args = ((void*)((UINT32)(((UINT32) args) + 7) & -8) + 8);
      //sv = (*(UINT64 *) (args - 8));

      //sv = (*(UINT64 *) (((void*)((UINT32)(((UINT32) args) + 7) & -8) + 8) - 8));
      sv = VA_ARG64 (args, UINT64);
     


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

