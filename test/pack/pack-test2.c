#include <stdio.h>

typedef unsigned int ADDRI;

typedef struct {
  unsigned char Addr[4];
} ADDR4C;

typedef struct {
  unsigned char   c1;
  unsigned char   c2;
  unsigned char   c3;
  unsigned char   c4;
  unsigned char   c5;
  unsigned char   c6;
  ADDR4C          a1;
  ADDR4C          a2;
  unsigned char   c7;
  unsigned char   c8;
  unsigned char   c9;
  unsigned char   c0;
  unsigned int    i1;
  unsigned int    i2;
} ME_DATA;

ME_DATA  myData = {
  1,
  2,
  3,
  4,
  5,
  6,
  {0, 0, 0, 0},
  {0, 0, 0, 0},
  7,
  8,
  9,
  0,
  11,
  22
};

#define MYNTOHL(x) (unsigned int)((((unsigned int) (x) & 0xff)     << 24) | \
                          (((unsigned int) (x) & 0xff00)   << 8)  | \
                          (((unsigned int) (x) & 0xff0000) >> 8)  | \
                          (((unsigned int) (x) & 0xff000000) >> 24))

#define MYEFI_IP4(EfiIpAddr)                (((EfiIpAddr).Addr[0] << 24 | (EfiIpAddr).Addr[1] << 16 | (EfiIpAddr).Addr[2] << 8 | (EfiIpAddr).Addr[3]))
#define MYEFI_NTOHL(EfiIp)                  (MYNTOHL (MYEFI_IP4 ((EfiIp))))



int main()
{
  printf("0x%x\n", (unsigned int)&myData);
  printf("0x%x\n", (unsigned int)&myData.a1);
  printf("0x%x\n", MYEFI_NTOHL(myData.a1));
  return 0;
}




