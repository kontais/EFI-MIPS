#include <stdio.h>
typedef unsigned int u32;
typedef int  i32;
int main()
{

  u32 base = 100;
  u32 image = 6;
  u32 adjust ;
  //i32 adjust ;
  u32 nbase = 120;
  u32 nimage;
  adjust  = image - base;
  nimage = nbase + adjust;

  printf("ni=%d\n",nimage);

  return 0;
}

