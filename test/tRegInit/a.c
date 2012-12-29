#include <stdio.h>
#include "RegInit.h"

typedef int UINT32;

UINT32 RegInit(UINT32 RegInitTable);

UINT32 data[1];
UINT32 Table[4][4] = {
  {REG_INIT_OP_WRITE|REG_INIT_MOD_WORD, data, 0x00000003,0x0},
  {REG_INIT_OP_EXIT, 123, 0x0,0x0}
  };

UINT32 TableRead[4][4] = {
  {REG_INIT_OP_READ|REG_INIT_MOD_WORD, data, 0x00000003,0x0},
  {REG_INIT_OP_EXIT, 123, 0x0,0x0}
  };

UINT32 TableRMW[4][4] = {
  {REG_INIT_OP_RMW|REG_INIT_MOD_WORD, data, 0x30,0x03},
  {REG_INIT_OP_EXIT, 123, 0x0,0x0}
  };

UINT32 TableDELAY[4][4] = {
  {REG_INIT_OP_DELAY, 0x10, 0x0,0x0},
  {REG_INIT_OP_EXIT, 3, 0x0,0x0}
  };

UINT32 TableWAIT[4][4] = {
  {REG_INIT_OP_WAIT, data, 0x1,0x1},
  {REG_INIT_OP_EXIT, 1, 0x0,0x0}
  };

int main()
{
  UINT32 retvalue;
  
  // test read
  data[0] = 0x1111;
  retvalue = RegInit((UINT32)Table);

  printf("ret = %d\n", retvalue);
  printf("data = 0x%x\n", data[0]);
  if (0x3 != data[0])
  {
    printf("======ERROR======\n");
  }

  // test read modify write
  data[0] = 0xee;
  retvalue = RegInit((UINT32)TableRead);

  // test write and exit
  data[0] = 0xff;
  retvalue = RegInit((UINT32)TableRMW);

  printf("ret = %d\n", retvalue);
  printf("data = 0x%x\n", data[0]);
  if (0x33 != data[0])
  {
    printf("======ERROR======\n");
  }

  // test delay
  retvalue = RegInit((UINT32)TableDELAY);

  printf("ret = %d\n", retvalue);

  // test wait
  data[0] = 1;
  // data[0] = 0;
  retvalue = RegInit((UINT32)TableWAIT);
  printf("ret = %d\n", retvalue);

  return retvalue;
}

