#include <stdio.h>
#include <string.h>

extern unsigned int lbu(void *a);
extern unsigned int lb(void *a);

short DataS8[10] = {0x80,  -1, 0, 1, 0x7f};


int main()
{
  unsigned int Value;
  printf("-----------lb-----------\n");
  printf("0x%x\n", lb(&DataS8[0]));
  printf("0x%x\n", lb(&DataS8[1]));
  printf("0x%x\n", lb(&DataS8[2]));
  printf("0x%x\n", lb(&DataS8[3]));
  printf("0x%x\n", lb(&DataS8[4]));

  printf("-----------lbu-----------\n");
  printf("0x%x\n", lbu(&DataS8[0]));
  printf("0x%x\n", lbu(&DataS8[1]));
  printf("0x%x\n", lbu(&DataS8[2]));
  printf("0x%x\n", lbu(&DataS8[3]));
  printf("0x%x\n", lbu(&DataS8[4]));

  printf("-----------char*-----------\n");
  printf("0x%x\n", *(char*)(&DataS8[0]));
  printf("0x%x\n", *(char*)(&DataS8[1]));
  printf("0x%x\n", *(char*)(&DataS8[2]));
  printf("0x%x\n", *(char*)(&DataS8[3]));
  printf("0x%x\n", *(char*)(&DataS8[4]));

  printf("-----------unsigned char*-----------\n");
  printf("0x%x\n", *(unsigned char*)(&DataS8[0]));
  printf("0x%x\n", *(unsigned char*)(&DataS8[1]));
  printf("0x%x\n", *(unsigned char*)(&DataS8[2]));
  printf("0x%x\n", *(unsigned char*)(&DataS8[3]));
  printf("0x%x\n", *(unsigned char*)(&DataS8[4]));


  Value = 0;
  Value = *(char*)(&DataS8[0]);
  printf("Value = %x\n", Value);

  Value = 0;
  Value = *(unsigned char*)(&DataS8[0]);
  printf("Value = %x\n", Value);

  return 0;
}
