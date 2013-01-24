#include <stdio.h>
#include <string.h>

extern unsigned int lhu(void *a);
extern unsigned int lh(void *a);

unsigned short DataU16[10];
short DataS16[10] = {0x8000,  -1, 0, 1, 0x7fff};


int main()
{
  printf("0x%x\n", lh(&DataS16[0]));
  printf("0x%x\n", lh(&DataS16[1]));
  printf("0x%x\n", lh(&DataS16[2]));
  printf("0x%x\n", lh(&DataS16[3]));
  printf("0x%x\n", lh(&DataS16[4]));

  printf("0x%x\n", lhu(&DataS16[0]));
  printf("0x%x\n", lhu(&DataS16[1]));
  printf("0x%x\n", lhu(&DataS16[2]));
  printf("0x%x\n", lhu(&DataS16[3]));
  printf("0x%x\n", lhu(&DataS16[4]));
  return 0;
}
